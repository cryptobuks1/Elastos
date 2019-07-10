package spv

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/event"
	"golang.org/x/net/context"
	"math/big"
	"path/filepath"
	"strings"
	"sync"
	"sync/atomic"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/bloom"
	spv "github.com/elastos/Elastos.ELA.SPV/interface"
	"github.com/elastos/Elastos.ELA.SideChain.ETH"
	ethCommon "github.com/elastos/Elastos.ELA.SideChain.ETH/common"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/core/events"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/crypto"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/ethclient"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/ethdb"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/log"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/node"
	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	core "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/utils/signal"
)

var (
	dataDir          = "./"
	ipcClient        *ethclient.Client
	stack            *node.Node
	SpvService       *Service
	spvTxhash        string //Spv notification main chain hash
	spvTransactiondb *ethdb.LDBDatabase
	musend           sync.RWMutex
	mufind           sync.RWMutex
	muupti           sync.RWMutex
	candSend         int32     //1 can send recharge transactions, 0 can not send recharge transactions
	candIterator     int32 = 1 //1 Iteratively send recharge transactions, 0 can't iteratively send recharge transactions
	MinedBlockSub    *event.TypeMuxSubscription
	Signers          map[ethCommon.Address]struct{} // Set of authorized signers at this moment
)

const (
	databaseCache int = 768

	handles = 16

	//Unprocessed refill transaction index prefix
	UnTransaction string = "UnT-"

	// missingNumber is returned by GetBlockNumber if no header with the
	// given block hash has been stored in the database
	missingNumber = uint64(0xffffffffffffffff)

	//Cross-chain exchange rate
	rate int64 = 10000000000

	//Cross-chain recharge unprocessed transaction index
	UnTransactionIndex = "UnTI"

	//Cross-chain recharge unprocessed transaction seek
	UnTransactionSeek = "UnTS"

	// Fixed number of extra-data prefix bytes reserved for signer vanity
	extraVanity = 32

	// Fixed number of extra-data suffix bytes reserved for signer seal
	extraSeal = 65
)

//type MinedBlockEvent struct{}

type Config struct {
	// DataDir is the data path to store db files peer addresses etc.
	DataDir string

	// ActiveNet indicates the ELA network to connect with.
	ActiveNet string

	// GenesisAddress is the address generated by the side chain genesis block.
	GenesisAddress string
}

type Service struct {
	spv.DPOSSPVService
}

//Spv database initialization
func SpvDbInit(spvdataDir string) {
	db, err := ethdb.NewLDBDatabase(filepath.Join(spvdataDir, "spv_transaction_info.db"), databaseCache, handles)
	if err != nil {
		log.Error("spv Open db: %v", err)
		return
	}
	db.Meter("eth/db/ela/")
	spvTransactiondb = db
}

//Spv service initialization
func NewService(cfg *Config, s *node.Node) (*Service, error) {
	stack = s
	var chainParams *config.Params
	switch strings.ToLower(cfg.ActiveNet) {
	case "testnet", "test", "t":
		chainParams = config.DefaultParams.TestNet()
	case "regnet", "reg", "r":
		chainParams = config.DefaultParams.RegNet()
	default:

		chainParams = config.DefaultParams.TestNet()

	}
	spvCfg := spv.DPOSConfig{Config: spv.Config{
		DataDir:     cfg.DataDir,
		ChainParams: chainParams,
	},
	}
	dataDir = cfg.DataDir
	initLog(cfg.DataDir)

	service, err := spv.NewDPOSSPVService(&spvCfg, signal.NewInterrupt().C)
	if err != nil {
		log.Error(fmt.Sprintf("Spv New DPOS SPVService: %v", err))
		return nil, err
	}

	SpvService = &Service{DPOSSPVService: service}
	err = service.RegisterTransactionListener(&listener{
		address: cfg.GenesisAddress,
		service: service,
	})
	if err != nil {
		log.Error(fmt.Sprintf("Spv Register Transaction Listener: %v", err))
		return nil, err
	}
	client, err := stack.Attach()
	if err != nil {
		log.Error("Attach client: %v", err)
	}
	ipcClient = ethclient.NewClient(client)
	genesis, err := ipcClient.HeaderByNumber(context.Background(), new(big.Int).SetInt64(0))
	if err != nil {
		log.Error(fmt.Sprintf("IpcClient: %v", err))
	}
	signers := make([]ethCommon.Address, (len(genesis.Extra)-extraVanity-extraSeal)/ethCommon.AddressLength)
	for i := 0; i < len(signers); i++ {
		copy(signers[i][:], genesis.Extra[extraVanity+i*ethCommon.AddressLength:])
	}
	Signers = make(map[ethCommon.Address]struct{})
	for _, signer := range signers {
		Signers[signer] = struct{}{}
	}
	MinedBlockSub = s.EventMux().Subscribe(events.MinedBlockEvent{})
	go minedBroadcastLoop(MinedBlockSub)
	return SpvService, nil
}

//minedBroadcastLoop Mining awareness, eth can initiate a recharge transaction after the block
func minedBroadcastLoop(minedBlockSub *event.TypeMuxSubscription) {
	var i = 0

	for {
		select {
		case <-minedBlockSub.Chan():
			i++
			if i >= 2 {
				atomic.StoreInt32(&candSend, 1)
			}

		case <-time.After(1 * time.Minute):
			i = 0
			atomic.StoreInt32(&candSend, 0)

		}
	}

}

func (s *Service) GetDatabase() ethdb.Database {
	return spvTransactiondb
}

func (s *Service) VerifyTransaction(tx *types.Transaction) error {
	payload, ok := tx.Payload.(*types.PayloadRechargeToSideChain)
	if !ok {
		return errors.New("[VerifyTransaction] Invalid payload core.PayloadRechargeToSideChain")
	}

	switch tx.PayloadVersion {
	case types.RechargeToSideChainPayloadVersion0:

		proof := new(bloom.MerkleProof)
		mainChainTransaction := new(core.Transaction)

		reader := bytes.NewReader(payload.MerkleProof)
		if err := proof.Deserialize(reader); err != nil {
			return errors.New("[VerifyTransaction] RechargeToSideChain payload deserialize failed")
		}

		reader = bytes.NewReader(payload.MainChainTransaction)
		if err := mainChainTransaction.Deserialize(reader); err != nil {
			return errors.New("[VerifyTransaction] RechargeToSideChain mainChainTransaction deserialize failed")
		}

		if err := s.DPOSSPVService.VerifyTransaction(*proof, *mainChainTransaction); err != nil {
			return errors.New("[VerifyTransaction] SPV module verify transaction failed.")
		}

	case types.RechargeToSideChainPayloadVersion1:

		_, err := s.GetTransaction(&payload.MainChainTransactionHash)
		if err != nil {
			return errors.New("[VerifyTransaction] Main chain transaction not found")
		}

	default:
		return errors.New("[VerifyTransaction] invalid payload version.")
	}

	return nil
}

func (s *Service) VerifyElaHeader(hash *common.Uint256) error {
	blockChain := s.HeaderStore()
	_, err := blockChain.Get(hash)
	if err != nil {
		return errors.New("[VerifyElaHeader] Verify ela header failed.")
	}
	return nil
}

type listener struct {
	address string
	service spv.DPOSSPVService
}

func (l *listener) Address() string {
	return l.address
}

func (l *listener) Type() core.TxType {
	return core.TransferCrossChainAsset
}

func (l *listener) Flags() uint64 {
	return spv.FlagNotifyInSyncing
}

func (l *listener) Notify(id common.Uint256, proof bloom.MerkleProof, tx core.Transaction) {
	// Submit transaction receipt
	log.Info("========================================================================================")
	log.Info("mainchain transaction info")
	log.Info("----------------------------------------------------------------------------------------")
	log.Info(string(tx.String()))
	log.Info("----------------------------------------------------------------------------------------")
	savePayloadInfo(tx)
	l.service.SubmitTransactionReceipt(id, tx.Hash())
}

//savePayloadInfo save and send spv perception
func savePayloadInfo(elaTx core.Transaction) {
	nr := bytes.NewReader(elaTx.Payload.Data(elaTx.PayloadVersion))
	p := new(payload.TransferCrossChainAsset)
	p.Deserialize(nr, elaTx.PayloadVersion)
	var fees []string
	var address []string
	var outputs []string
	for i, amount := range p.CrossChainAmounts {
		fees = append(fees, (elaTx.Outputs[i].Value - amount).String())
		outputs = append(outputs, elaTx.Outputs[i].Value.String())
		address = append(address, p.CrossChainAddresses[i])
	}
	addr := strings.Join(address, ",")
	fee := strings.Join(fees, ",")
	output := strings.Join(outputs, ",")
	if spvTxhash == elaTx.Hash().String() {
		return
	}
	spvTxhash = elaTx.Hash().String()
	err := spvTransactiondb.Put([]byte(elaTx.Hash().String()+"Fee"), []byte(fee))

	if err != nil {
		log.Error(fmt.Sprintf("SpvServicedb Put Fee: %v", err), "elaHash", elaTx.Hash().String())
	}

	err = spvTransactiondb.Put([]byte(elaTx.Hash().String()+"Address"), []byte(addr))

	if err != nil {
		log.Error(fmt.Sprintf("SpvServicedb Put Address: %v", err), "elaHash", elaTx.Hash().String())
	}
	err = spvTransactiondb.Put([]byte(elaTx.Hash().String()+"Output"), []byte(output))

	if err != nil {
		log.Error(fmt.Sprintf("SpvServicedb Put Output: %v", err), "elaHash", elaTx.Hash().String())
	}
	if atomic.LoadInt32(&candSend) == 1 {

		var from ethCommon.Address
		if wallets := stack.AccountManager().Wallets(); len(wallets) > 0 {
			if accounts := wallets[0].Accounts(); len(accounts) > 0 {
				from = accounts[0].Address
			}
		}
		if _, ok := Signers[from]; ok {
			if atomic.LoadInt32(&candIterator) == 1 {
				atomic.StoreInt32(&candIterator, 0)
				go IteratorUnTransaction(from)
			}
			f, err := common.StringToFixed64(fees[0])
			if err != nil {
				log.Error(fmt.Sprintf("SpvSendTransaction Fee StringToFixed64: %v", err), "elaHash", elaTx)
				return

			}
			fe := new(big.Int).SetInt64(f.IntValue())
			y := new(big.Int).SetInt64(rate)
			fee := new(big.Int).Mul(fe, y)
			SendTransaction(from, elaTx.Hash().String(), fee)
		}
	} else {
		UpTransactionIndex(elaTx.Hash().String())
	}
	return
}

//UpTransactionIndex records spv-aware refill transaction index
func UpTransactionIndex(elaTx string) {
	muupti.Lock()
	defer muupti.Unlock()
	index := GetUnTransactionNum(spvTransactiondb, UnTransactionIndex)
	if index == missingNumber {
		index = 1
	}
	err := spvTransactiondb.Put(append([]byte(UnTransaction), encodeUnTransactionNumber(index)...), []byte(elaTx))
	if err != nil {
		log.Error(fmt.Sprintf("SpvServicedb Put UnTransaction: %v", err), "elaHash", elaTx)
	}
	log.Info(UnTransaction+"put", "index", index, "elaTx", elaTx)
	err = spvTransactiondb.Put([]byte(UnTransactionIndex), encodeUnTransactionNumber(index+1))
	if err != nil {
		log.Error("UnTransactionIndexPut", err, index+1)
		return
	}
	log.Info(UnTransactionIndex+"put", "index", index+1)

}

//IteratorUnTransaction iterates before mining and processes existing spv refill transactions
func IteratorUnTransaction(from ethCommon.Address) {
	for {
		index := GetUnTransactionNum(spvTransactiondb, UnTransactionIndex)
		if index == missingNumber {
			atomic.StoreInt32(&candIterator, 1)
			return
		}
		seek := GetUnTransactionNum(spvTransactiondb, UnTransactionSeek)
		if seek == missingNumber {
			seek = 1
		}
		if seek == index {
			atomic.StoreInt32(&candIterator, 1)
			return
		}
		txhash, err := spvTransactiondb.Get(append([]byte(UnTransaction), encodeUnTransactionNumber(seek)...))
		if err != nil {
			log.Error("get UnTransaction ", err, seek)
			atomic.StoreInt32(&candIterator, 1)
			return
		}

		fee, _, _ := FindOutputFeeAndaddressByTxHash(string(txhash))
		SendTransaction(from, string(txhash), fee)
		err = spvTransactiondb.Put([]byte(UnTransactionSeek), encodeUnTransactionNumber(seek+1))
		log.Info(UnTransactionSeek+"put", seek+1)
		if err != nil {
			log.Error("UnTransactionIndexPutSeek ", err, seek+1)
			atomic.StoreInt32(&candIterator, 1)
			return
		}
		err = spvTransactiondb.Delete(append([]byte(UnTransaction), encodeUnTransactionNumber(seek)...))
		log.Info(UnTransaction+"delete", "seek", seek)
		if err != nil {
			log.Error("UnTransactionIndexDeleteSeek ", err, seek)
			atomic.StoreInt32(&candIterator, 1)
			return
		}
	}
}

//SendTransaction sends a reload transaction to txpool
func SendTransaction(from ethCommon.Address, elaTx string, fee *big.Int) {
	musend.Lock()
	defer musend.Unlock()
	ethTx, err := ipcClient.StorageAt(context.Background(), ethCommon.Address{}, ethCommon.HexToHash("0x"+elaTx), nil)
	if err != nil {
		log.Error(fmt.Sprintf("IpcClient StorageAt: %v", err))
		return
	}
	h := ethCommon.Hash{}
	if ethCommon.BytesToHash(ethTx) != h {
		log.Warn("Cross-chain transactions have been processed", "elaHash", elaTx)
		return
	}
	data, err := common.HexStringToBytes(elaTx)
	if err != nil {
		log.Error(fmt.Sprintf("elaTx HexStringToBytes: %v"+elaTx, err))
		return
	}
	msg := ethereum.CallMsg{From: from, To: &ethCommon.Address{}, Data: data}
	gasLimit, err := ipcClient.EstimateGas(context.Background(), msg)
	if err != nil {
		log.Error(fmt.Sprintf("IpcClient EstimateGas: %v", err))
		return
	}
	if gasLimit == 0 {
		return
	}

	price := new(big.Int).Quo(fee, new(big.Int).SetUint64(gasLimit))
	callmsg := ethereum.TXMsg{From: from, To: &ethCommon.Address{}, Gas: gasLimit, Data: data, GasPrice: price}
	hash, err := ipcClient.SendPublicTransaction(context.Background(), callmsg)
	if err != nil {
		log.Error(fmt.Sprintf("IpcClient SendPublicTransaction: %v", err))
		return
	}
	log.Info("Cross chain Transaction", "elaTx", elaTx, "ethTh", hash.String())
}

func encodeUnTransactionNumber(number uint64) []byte {
	enc := make([]byte, 8)
	binary.BigEndian.PutUint64(enc, number)
	return enc
}

func GetUnTransactionNum(db DatabaseReader, Prefix string) uint64 {
	data, _ := db.Get([]byte(Prefix))
	if len(data) != 8 {
		return missingNumber
	}
	return binary.BigEndian.Uint64(data)
}

// DatabaseReader wraps the Get method of a backing data store.
type DatabaseReader interface {
	Get(key []byte) (value []byte, err error)
}

//FindOutputFeeAndaddressByTxHash Finds the eth recharge address, recharge amount, and transaction fee based on the main chain hash.
func FindOutputFeeAndaddressByTxHash(transactionHash string) (*big.Int, ethCommon.Address, *big.Int) {
	mufind.Lock()
	defer mufind.Unlock()
	var emptyaddr ethCommon.Address
	transactionHash = strings.Replace(transactionHash, "0x", "", 1)
	if spvTransactiondb == nil {
		return new(big.Int), emptyaddr, new(big.Int)
	}
	v, err := spvTransactiondb.Get([]byte(transactionHash + "Fee"))
	if err != nil {
		log.Error(fmt.Sprintf("SpvServicedb Get Fee: %v"), err, "elaHash", transactionHash)
		return new(big.Int), emptyaddr, new(big.Int)
	}
	fees := strings.Split(string(v), ",")
	f, err := common.StringToFixed64(fees[0])
	if err != nil {
		log.Error(fmt.Sprintf("SpvServicedb Get Fee StringToFixed64: %v", err), "elaHash", transactionHash)
		return new(big.Int), emptyaddr, new(big.Int)

	}
	fe := new(big.Int).SetInt64(f.IntValue())
	y := new(big.Int).SetInt64(rate)

	addrss, err := spvTransactiondb.Get([]byte(transactionHash + "Address"))
	if err != nil {
		log.Error(fmt.Sprintf("SpvServicedb Get Address: %v", err), "elaHash", transactionHash)
		return new(big.Int), emptyaddr, new(big.Int)

	}
	addrs := strings.Split(string(addrss), ",")
	if !ethCommon.IsHexAddress(addrs[0]) {
		return new(big.Int), emptyaddr, new(big.Int)
	}
	outputs, err := spvTransactiondb.Get([]byte(transactionHash + "Output"))
	if err != nil {
		log.Error(fmt.Sprintf("SpvServicedb Get elaHash: %v", err), "elaHash", transactionHash)
		return new(big.Int), emptyaddr, new(big.Int)

	}
	output := strings.Split(string(outputs), ",")
	o, err := common.StringToFixed64(output[0])
	if err != nil {
		log.Error(fmt.Sprintf("SpvServicedb Get elaHash StringToFixed64: %v", err), "elaHash", transactionHash)
		return new(big.Int), emptyaddr, new(big.Int)

	}
	op := new(big.Int).SetInt64(o.IntValue())
	return new(big.Int).Mul(fe, y), ethCommon.HexToAddress(addrs[0]), new(big.Int).Mul(op, y)
}

// Get Ela Chain Height
func GetElaChainHeight() uint32 {
	var elaHeight uint32 = 0
	if SpvService == nil || SpvService.DPOSSPVService == nil {
		log.Info("spv service initiation does not finish yet !")
	} else {
		elaHeight = SpvService.DPOSSPVService.GetHeight()
	}
	return elaHeight
}

// Until Get Ela Chain Height
func UntilGetElaChainHeight() uint32 {
	for {
		if elaHeight := GetElaChainHeight(); elaHeight != 0 {
			return elaHeight
		}
		log.Info("can not get elas height, because ela height interface has no any response !")
		time.Sleep(time.Millisecond * 1000)
	}
}

// Determine whether an address is an arbiter
func AddrIsArbiter(address ethCommon.Address) int8 {
	if SpvService == nil || SpvService.DPOSSPVService == nil {
		log.Info("spv service initiation does not finish yet !")
	} else {
		arbiters := SpvService.DPOSSPVService.GetProducersByHeight(SpvService.DPOSSPVService.GetHeight())
		for _, arbiter := range arbiters {
			publicKey, convertErr := crypto.UnmarshalPubkey(arbiter)
			if convertErr == nil {
				if abiterAddress := crypto.PubkeyToAddress(*publicKey); abiterAddress == address {
					return 1
				}
			}
		}
	}
	return 0
}

// Determine whether an address is an arbiter
func AddrIsArbiterWithElaHeight(address ethCommon.Address, elaHeight uint32) int8 {
	if SpvService == nil || SpvService.DPOSSPVService == nil {
		log.Info("spv service initiation does not finish yet !")
	} else {
		arbiters := SpvService.DPOSSPVService.GetProducersByHeight(elaHeight)
		for _, arbiter := range arbiters {
			publicKey, convertErr := crypto.UnmarshalPubkey(arbiter)
			if convertErr == nil {
				if abiterAddress := crypto.PubkeyToAddress(*publicKey); abiterAddress == address {
					return 1
				}
			}
		}
	}
	return 0
}

func GetCurrentProducers() [][]byte {
	var arbiters [][]byte
	if SpvService == nil || SpvService.DPOSSPVService == nil {
		log.Info("spv service initiation does not finish yet !")
	} else {
		arbiters := SpvService.DPOSSPVService.GetProducersByHeight(GetCurrentElaHeight())
		log.Info("---------------------------------------------------------------")
		log.Info("arbiters", arbiters)
	}

	return arbiters
}

func GetCurrentElaHeight() uint32 {
	var height uint32
	if SpvService == nil || SpvService.DPOSSPVService == nil {
		log.Info("spv service initiation does not finish yet !")
	} else {
		height = SpvService.DPOSSPVService.GetHeight()
		log.Info("----GetCurrentElaHeight-----------------------------------------------------------")
		log.Info("ElaHeight", height)
	}

	return height
}

func GetProducersByHeight(height uint32) [][]byte {
	var arbiters [][]byte

	if SpvService == nil || SpvService.DPOSSPVService == nil {
		log.Info("spv service initiation does not finish yet !")
	} else {
		arbiters := SpvService.DPOSSPVService.GetProducersByHeight(height)
		log.Info("---------------------------------------------------------------")
		log.Info("arbiters", arbiters)
	}
	return arbiters
}
