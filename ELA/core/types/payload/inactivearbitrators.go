package payload

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const PayloadInactiveArbitratorsVersion byte = 0x00

type InactiveArbitrators struct {
	Sponsor     []byte
	Arbitrators [][]byte

	hash *common.Uint256
}

func (i *InactiveArbitrators) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := i.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (i *InactiveArbitrators) Serialize(w io.Writer,
	version byte) error {
	if err := common.WriteVarBytes(w, i.Sponsor); err != nil {
		return err
	}

	if err := common.WriteVarUint(w, uint64(len(i.Arbitrators))); err != nil {
		return err
	}

	for _, v := range i.Arbitrators {
		if err := common.WriteVarBytes(w, v); err != nil {
			return err
		}
	}

	return nil
}

func (i *InactiveArbitrators) Deserialize(r io.Reader,
	version byte) (err error) {
	if i.Sponsor, err = common.ReadVarBytes(r, crypto.NegativeBigLength,
		"public key"); err != nil {
		return err
	}

	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return err
	}

	i.Arbitrators = make([][]byte, count)
	for u := uint64(0); u < count; u++ {
		if i.Arbitrators[u], err = common.ReadVarBytes(r,
			crypto.NegativeBigLength, "public key"); err != nil {
			return err
		}
	}

	return err
}

func (i *InactiveArbitrators) Hash() common.Uint256 {
	if i.hash == nil {
		buf := new(bytes.Buffer)
		i.Serialize(buf, PayloadInactiveArbitratorsVersion)
		hash := common.Uint256(common.Sha256D(buf.Bytes()))
		i.hash = &hash
	}
	return *i.hash
}
