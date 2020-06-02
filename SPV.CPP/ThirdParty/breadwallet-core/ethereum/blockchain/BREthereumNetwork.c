//
//  BREthereumNetwork
//  breadwallet-core Ethereum
//
//  Created by Ed Gamble on 3/13/18.
//  Copyright © 2018 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <stdlib.h>
#include "BREthereumNetwork.h"

#define NUMBER_OF_SEEDS_LIMIT       (5)
#define NUMBER_OF_ENODES_LIMIT      (10)

static void
networkInitilizeAllIfAppropriate (void);

//
// Network
//
struct BREthereumNetworkRecord {
    const char *name;
    int chainId;
    BREthereumHash genesisBlockHeaderHash;
    BREthereumHash trustedCheckpointBlockHeaderHash;
    const char *seeds[NUMBER_OF_SEEDS_LIMIT + 1];
    const char *enodesBRD[NUMBER_OF_ENODES_LIMIT + 1];
    const char *enodesCOM[NUMBER_OF_ENODES_LIMIT + 1];

    const char *enodesLCLParity[4];
    const char *enodesLCLGeth[4];
};

extern BREthereumChainId
networkGetChainId (BREthereumNetwork network) {
    networkInitilizeAllIfAppropriate();
    return network->chainId;
}

extern BREthereumHash
networkGetGenesisBlockHeaderHash (BREthereumNetwork network) {
    networkInitilizeAllIfAppropriate();
    return network->genesisBlockHeaderHash;
}

extern BREthereumHash
networkGetTrustedCheckpointBlockHeaderHash (BREthereumNetwork network) {
    networkInitilizeAllIfAppropriate();
    return network->trustedCheckpointBlockHeaderHash;
}

extern const char *
networkGetName (BREthereumNetwork network) {
    return network->name;
}

extern const char**
networkGetSeeds (BREthereumNetwork network) {
    return network->seeds;
}

extern size_t
networkGetSeedsCount (BREthereumNetwork network) {
    size_t i = 0;
    while (NULL != network->seeds[i]) i++;
    return i;
}

extern const char**
networkGetEnodesBRD (BREthereumNetwork network) {
    return network->enodesBRD;
}

extern const char**
networkGetEnodesCommunity (BREthereumNetwork network) {
    return network->enodesCOM;
}

extern const char**
networkGetEnodesLocal (BREthereumNetwork network, int parity) {
    return parity ?  network->enodesLCLParity : network->enodesLCLGeth;
}

/// MARK: - Static Network Definitions

//
// Mainnet
//
static struct BREthereumNetworkRecord ethereumMainnetRecord = {
    "Mainnet",
    1,
    EMPTY_HASH_INIT,
    EMPTY_HASH_INIT,
    // Seeds
    { "seed.mainnet.eth.brd.breadwallet.com",
        "seed.mainnet.eth.community.breadwallet.com",
        NULL },

    // Enodes

    // BRD
    { "enode://757782a7331bf92e648db97779bf10341ffde5db3e28603c863462ebcb20016d0565469dbb4b20925e1c6e59a01b04b51e80273f844e066b8ebec746e2a2a833@178.128.62.142:8888",
        "enode://ae1e2d1f4c17203e17a9cc8bffd5a2f9ad4cf081fa966caa643e32bdbd31f483d5ecb515113df4c9e9a6673eed25033d3031836260053bbd2f00c0d5a00cc319@206.189.78.132:8888",
        "enode://b9040af88f88a5b5e2864b2e98630d58579aab0649a90fff5b5b544f0aaf97a2a084651ca5a2b2f358abd215bda4494e7a350ab126915abd559d6da7b539b6ca@138.68.12.85:8888",
        "enode://16e59b1305340bf33546b218dcdb393c7ff8791a6c1cd059ece918fd6b57877e053d26b58bf5f6daa67e5de201057c6297bcab76fa8ec4bc1af15b4642892fd9@159.203.9.180:8888",
        "enode://c07c687e2d402593a29cd8a6e711c37b517d8ccefa1b8de14aede99dd9b0c697796aa00b6693ec8270c5ecae0780b71c8717a19e3504cf6576a3a2699e0d5032@142.93.184.70:8888",
        NULL },

    // Community
    { "enode://0f740f471e876020566c2ce331c81b4128b9a18f636b1d4757c4eaea7f077f4b15597a743f163280293b0a7e35092064be11c4ec199b9905541852a36be9004b@206.221.178.149:30303",
        "enode://16d92fc94f4ec4386aca44d255853c27cbe97a4274c0df98d2b642b0cc4b2f2330e99b00b46db8a031da1a631c85e2b4742d52f5eaeca46612cd28db41fb1d7f@91.223.175.173:30303",
        NULL },

    // Local - Parity
    { "enode://74b31b97f646b206dd01d8f20d080b97e502483a55ee64ea02cbf0c6df4263ff33bee61ba940113db36a4cfd1e1e8f2fe66cf91e6a1925f63860fb6bc5671c87@192.168.1.200:8888",  // SSD Archive - Sam
        "enode://6ff469b687ad551b105226ea5d84c5137e8cbba0e12c134fa53b620b6fa90bbb2ee0fe1f590d05eec79f70c21399946be6c87d2ff7b698c77a775807917114d4@127.0.0.1:30303",   // SSD Archive - Ed
        NULL },

    // Local - Geth
    { "enode://654580048e9de8f7743ca38035c7ab7fbf2d59b6acd5b92cc031e4571b2c441fe9fc5bb261ada112fb39ca32c1ac7716d91a211b992693c9472ad6af42c5302a@127.0.0.1:30304",
        NULL }
};
const BREthereumNetwork ethereumMainnet = &ethereumMainnetRecord;

/*
// MainnetChainConfig is the chain parameters to run a node on the main network.
MainnetChainConfig = &ChainConfig{
  ChainId:        big.NewInt(1),
  HomesteadBlock: big.NewInt(1150000),
  DAOForkBlock:   big.NewInt(1920000),
  DAOForkSupport: true,
  EIP150Block:    big.NewInt(2463000),
  EIP150Hash:     common.HexToHash("0x2086799aeebeae135c246c65021c82b4e15a2c451340993aacfd2751886514f0"),
  EIP155Block:    big.NewInt(2675000),
  EIP158Block:    big.NewInt(2675000),
  ByzantiumBlock: big.NewInt(4370000),
  Ethash: new(EthashConfig),
}
*/

//
// Testnet
//
static struct BREthereumNetworkRecord ethereumTestnetRecord = {
    "Testnet",
    21,
    EMPTY_HASH_INIT,
    EMPTY_HASH_INIT,
    // Seeds
    {   //"seed.ropsten.eth.brd.breadwallet.com",
        //"seed.ropsten.eth.community.breadwallet.com",
        NULL },

    // Enodes

    // BRD
    {   //"enode://87ef58b88a9c7574eb870097675e26f78dcd958834bd768b678aa01eabd316c74df1ff01bfbe030c5b75878646df4108554434df61de591a2c6859e329bbacde@138.68.6.252:8888",
		"enode://5e1d6f9f74e33b2d1e2fda87efaf60a788b338c08eefd3a435e9c7de98645bc041421c27d9ed3927c7b5195febd691aff30de881842749f3030089df0e135232@3.208.184.54:20630",
		"enode://30dc2b7986e2ec5902498ec26fad6fcecece617aa1652f227f684ede6a0939bb7a205ada1c91420d30b427c86bbdcc31fdfd6d955dd8f5854370f583025a0708@3.209.35.13:20630",
		"enode://b0357d45e9070c1660f63f077e0e3b0054a18d93785589d498586b6e0b7ec7c5b39ef608e82e7280ca95019db7c36455275d98a3e8684916ba8f3a7aab4ad38b@3.210.227.193:20630",
        NULL },
    { NULL },
    { NULL },
    { NULL }
};
const BREthereumNetwork ethereumTestnet = &ethereumTestnetRecord;

/*
// TestnetChainConfig contains the chain parameters to run a node on the Ropsten test network.
TestnetChainConfig = &ChainConfig{
  ChainId:        big.NewInt(3),
  HomesteadBlock: big.NewInt(0),
  DAOForkBlock:   nil,
  DAOForkSupport: true,
  EIP150Block:    big.NewInt(0),
  EIP150Hash:     common.HexToHash("0x41941023680923e0fe4d74a34bdac8141f2540e3ae90623718e47d66d1ca4a2d"),
  EIP155Block:    big.NewInt(10),
  EIP158Block:    big.NewInt(10),
  ByzantiumBlock: big.NewInt(1700000),
  Ethash: new(EthashConfig),
}
*/
//
// Rinkeby
//
static struct BREthereumNetworkRecord ethereumRinkebyRecord = {
    "Rinkeby",
    22,
    EMPTY_HASH_INIT,
    EMPTY_HASH_INIT,
    // Seeds
    { NULL },

    // Enodes
    
    { "enode://fe44bc423f210805daad60cc5d308f449e9282c28a9aba91040d7c727cf5751d1ae9e85d32a430f4a6fe15c8eb52833a1747e8b28e6ed5ae291fdae32e6b9181@18.217.15.245:20630",
	  "enode://777e2a86687d675c05344acc6e24cefbd3e233759e8b89d7b3d101aeffc89e6292f66a115c5bfc30f250c120e6a2354a7a6ea304439cfded706de1c9ade61abf@18.217.15.245:20630",
	  "enode://deb84117dada6c2c8f9c5d9d44f749b6fbbefdc987a1611b683ead6e4e2ce8e0d05a196591a713376eee5d9c165d3888d2e175e8eb842e5a381f273c0268edca@18.217.15.245:20630",
	  NULL },
    { NULL },
    { NULL },
    { NULL }
};
const BREthereumNetwork ethereumRinkeby = &ethereumRinkebyRecord;

/*
// RinkebyChainConfig contains the chain parameters to run a node on the Rinkeby test network.
RinkebyChainConfig = &ChainConfig{
  ChainId:        big.NewInt(4),
  HomesteadBlock: big.NewInt(1),
  DAOForkBlock:   nil,
  DAOForkSupport: true,
  EIP150Block:    big.NewInt(2),
  EIP150Hash:     common.HexToHash("0x9b095b36c15eaf13044373aef8ee0bd3a382a5abb92e402afa44b8249c3a90e9"),
  EIP155Block:    big.NewInt(3),
  EIP158Block:    big.NewInt(3),
  ByzantiumBlock: big.NewInt(1035301),
  Clique: &CliqueConfig{
    Period: 15,
    Epoch:  30000,
  },
}
*/

/// MARK: - Trusted Checkpoints

/*
// trustedCheckpoint represents a set of post-processed trie roots (CHT and BloomTrie) associated with
// the appropriate section index and head hash. It is used to start light syncing from this checkpoint
// and avoid downloading the entire header chain while still being able to securely access old headers/logs.
type trustedCheckpoint struct {
    name                                string
    sectionIdx                          uint64
    sectionHead, chtRoot, bloomTrieRoot common.Hash
}

var (
     mainnetCheckpoint = trustedCheckpoint{
     name:          "mainnet",
     sectionIdx:    153,
     sectionHead:   common.HexToHash("04c2114a8cbe49ba5c37a03cc4b4b8d3adfc0bd2c78e0e726405dd84afca1d63"),
     chtRoot:       common.HexToHash("d7ec603e5d30b567a6e894ee7704e4603232f206d3e5a589794cec0c57bf318e"),
     bloomTrieRoot: common.HexToHash("0b139b8fb692e21f663ff200da287192201c28ef5813c1ac6ba02a0a4799eef9"),
     }

     ropstenCheckpoint = trustedCheckpoint{
     name:          "ropsten",
     sectionIdx:    79,
     sectionHead:   common.HexToHash("1b1ba890510e06411fdee9bb64ca7705c56a1a4ce3559ddb34b3680c526cb419"),
     chtRoot:       common.HexToHash("71d60207af74e5a22a3e1cfbfc89f9944f91b49aa980c86fba94d568369eaf44"),
     bloomTrieRoot: common.HexToHash("70aca4b3b6d08dde8704c95cedb1420394453c1aec390947751e69ff8c436360"),
     }
     )

// trustedCheckpoints associates each known checkpoint with the genesis hash of the chain it belongs to
var trustedCheckpoints = map[common.Hash]trustedCheckpoint{
    params.MainnetGenesisHash: mainnetCheckpoint,
    params.TestnetGenesisHash: ropstenCheckpoint,
}

 // Rinkeby: genesis for all intents and purposes.
 // > INFO [06-06|11:34:07] Block synchronisation started
 // INFO [06-06|11:34:08] Imported new block headers               count=192 elapsed=76.267ms number=192 hash=8c570c…ba360c ignored=0

*/
static void
networkInitilizeAllIfAppropriate (void) {
    static int needsInitialization = 1;

    if (needsInitialization) {

        // Mainnet

        ethereumMainnetRecord.genesisBlockHeaderHash =
        hashCreate ("0xd4e56740f876aef8c010b86a40d5f56745a118d0906a34e69aec8c0db1cb8fa3");

        ethereumMainnetRecord.trustedCheckpointBlockHeaderHash =
        hashCreate("0x04c2114a8cbe49ba5c37a03cc4b4b8d3adfc0bd2c78e0e726405dd84afca1d63");

        // Testnet / 'Ropsten'

        ethereumTestnetRecord.genesisBlockHeaderHash =
        hashCreate("0x41941023680923e0fe4d74a34bdac8141f2540e3ae90623718e47d66d1ca4a2d");

        ethereumTestnetRecord.trustedCheckpointBlockHeaderHash =
        hashCreate("0x1b1ba890510e06411fdee9bb64ca7705c56a1a4ce3559ddb34b3680c526cb419");

        // Rinkeby

        ethereumRinkebyRecord.genesisBlockHeaderHash =
        hashCreate("0x6341fd3daf94b748c72ced5a5b26028f2474f5f00d824504e4fa37a75767e177");
        
        ethereumRinkebyRecord.trustedCheckpointBlockHeaderHash =
        hashCreate("0x6341fd3daf94b748c72ced5a5b26028f2474f5f00d824504e4fa37a75767e177");

        // Notable RACE
        needsInitialization = 0;

    }
}
