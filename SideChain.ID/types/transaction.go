package types

import (
	"github.com/elastos/Elastos.ELA.SideChain.ID/pact"
	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/vm/interfaces"
	"github.com/elastos/Elastos.ELA/common"
)

const (
	RegisterDID types.TxType = 0x0a
	UpdateDID   types.TxType = 0x0b
)

func IsRegisterIdentificationTx(tx *types.Transaction) bool {
	return tx.TxType == RegisterIdentification
}

func IsRegisterDIDTx(tx *types.Transaction) bool {
	return tx.TxType == RegisterDID
}

func IsUpdateDIDTx(tx *types.Transaction) bool {
	return tx.TxType == UpdateDID
}

func init() {

	txTypeStr := types.TxTypeStr
	types.TxTypeStr = func(txType types.TxType) string {
		if txType == RegisterIdentification {
			return "RegisterIdentification"
		}
		return txTypeStr(txType)
	}

	getDataContainer := types.GetDataContainer
	types.GetDataContainer = func(programHash *common.Uint168, tx *types.Transaction) interfaces.IDataContainer {
		switch tx.TxType {
		case RegisterIdentification:
			for _, output := range tx.Outputs {
				if programHash[0] == pact.PrefixRegisterId && programHash.IsEqual(output.ProgramHash) {
					return tx.Payload.(*PayloadRegisterIdentification)
				}
			}
		case RegisterDID:
			// todo comlete me
		case UpdateDID:
			// todo complete me
		}
		return getDataContainer(programHash, tx)
	}

	getPayloadByTxType := types.GetPayloadByTxType
	types.GetPayloadByTxType = func(txType types.TxType) (types.Payload, error) {
		switch txType {
		case RegisterIdentification:
			return &PayloadRegisterIdentification{}, nil
		case RegisterDID:
			return &PayloadDIDInfo{}, nil
		case UpdateDID:
			return &PayloadDIDInfo{}, nil
		}
		return getPayloadByTxType(txType)
	}
}
