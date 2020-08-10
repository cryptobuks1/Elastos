/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "CRCProposal.h"

#include <Common/hash.h>
#include <Common/Log.h>
#include <WalletCore/Base58.h>
#include <WalletCore/Key.h>

namespace Elastos {
	namespace ElaWallet {

#define JsonKeyType "Type"
#define JsonKeyStage "Stage"
#define JsonKeyAmount "Amount"

#define JsonKeyType "Type"
#define JsonKeyCategoryData "CategoryData"
#define JsonKeyOwnerPublicKey "OwnerPublicKey"
#define JsonKeyDraftHash "DraftHash"
#define JsonKeyBudgets "Budgets"
#define JsonKeyRecipient "Recipient"
#define JsonKeyTargetProposalHash "TargetProposalHash"
#define JsonKeyNewRecipient "NewRecipient"
#define JsonKeyNewOwnerPublicKey "NewOwnerPublicKey"
#define JsonKeySecretaryPublicKey "SecretaryGeneralPublicKey"
#define JsonKeySecretaryDID "SecretaryGeneralDID"
#define JsonKeySignature "Signature"
#define JsonKeyNewOwnerSignature "NewOwnerSignature"
#define JsonKeySecretarySignature "SecretaryGeneralSignature"
#define JsonKeyCRCouncilMemberDID "CRCouncilMemberDID"
#define JsonKeyCRCouncilMemberSignature "CRCouncilMemberSignature"

		Budget::Budget() {

		}

		Budget::Budget(Budget::Type type, uint8_t stage, const BigInt &amount) :
			_type(type), _stage(stage), _amount(amount) {

		}

		Budget::~Budget() {

		}

		Budget::Type Budget::GetType() const {
			return _type;
		}

		uint8_t Budget::GetStage() const {
			return _stage;
		}

		BigInt Budget::GetAmount() const {
			return _amount;
		}

		void Budget::Serialize(ByteStream &ostream) const {
			ostream.WriteUint8(_type);
			ostream.WriteUint8(_stage);
			ostream.WriteUint64(_amount.getUint64());
		}

		bool Budget::Deserialize(const ByteStream &istream) {
			uint8_t type;
			if (!istream.ReadUint8(type)) {
				SPVLOG_ERROR("Budget::Deserialize: read type key");
				return false;
			}
			_type = Budget::Type(type);

			if (!istream.ReadUint8(_stage)) {
				SPVLOG_ERROR("Budget::Deserialize: read stage key");
				return false;
			}

			uint64_t amount;
			if (!istream.ReadUint64(amount)) {
				SPVLOG_ERROR("Budget::Deserialize: read amount key");
				return false;
			}
			_amount.setUint64(amount);

			return true;
		}

		bool Budget::IsValid() const {
			if (_type >= Budget::Type::maxType) {
				SPVLOG_ERROR("invalid budget type: {}", _type);
				return false;
			}

			if (_stage > 127) {
				SPVLOG_ERROR("invalid budget stage", _stage);
				return false;
			}

			return true;
		}

		nlohmann::json Budget::ToJson() const {
			nlohmann::json j;
			j[JsonKeyType] = _type;
			j[JsonKeyStage] = _stage;
			j[JsonKeyAmount] = _amount.getDec();
			return j;
		}

		void Budget::FromJson(const nlohmann::json &j) {
			_type = Budget::Type(j[JsonKeyType].get<uint8_t>());
			_stage = j[JsonKeyStage].get<uint8_t>();
			_amount.setDec(j[JsonKeyAmount].get<std::string>());
		}

		CRCProposal::CRCProposal() {

		}

		CRCProposal::~CRCProposal() {

		}

		void CRCProposal::SetTpye(CRCProposal::Type type) {
			_type = type;
		}

		CRCProposal::Type CRCProposal::GetType() const {
			return _type;
		}

		void CRCProposal::SetCategoryData(const std::string &categoryData) {
			_categoryData = categoryData;
		}

		const std::string &CRCProposal::GetCategoryData() const {
			return _categoryData;
		}

		void CRCProposal::SetOwnerPublicKey(const bytes_t &publicKey) {
			_ownerPublicKey = publicKey;
		}

		const bytes_t &CRCProposal::GetOwnerPublicKey() const {
			return _ownerPublicKey;
		}

		void CRCProposal::SetCRCouncilMemberDID(const Address &crSponsorDID) {
			_crCouncilMemberDID = crSponsorDID;
		}

		const Address &CRCProposal::GetCRCouncilMemberDID() const {
			return _crCouncilMemberDID;
		}

		void CRCProposal::SetDraftHash(const uint256 &draftHash) {
			_draftHash = draftHash;
		}

		const uint256 &CRCProposal::GetDraftHash() const {
			return _draftHash;
		}

		void CRCProposal::SetBudgets(const std::vector<Budget> &budgets) {
			_budgets = budgets;
		}

		const std::vector<Budget> &CRCProposal::GetBudgets() const {
			return _budgets;
		}

		void CRCProposal::SetRecipient(const Address &recipient) {
			_recipient = recipient;
		}

		const Address &CRCProposal::GetRecipient() const {
			return _recipient;
		}

		void CRCProposal::SetSignature(const bytes_t &signature) {
			_signature = signature;
		}

		const bytes_t &CRCProposal::GetSignature() const {
			return _signature;
		}

		void CRCProposal::SetCRCouncilMemberSignature(const bytes_t &signature) {
			_crCouncilMemberSignature = signature;
		}

		const bytes_t &CRCProposal::GetCRCouncilMemberSignature() const {
			return _crCouncilMemberSignature;
		}

		const uint256 &CRCProposal::DigestNormalOwnerUnsigned(uint8_t version) const {
			if (_digestOwnerUnsigned == 0) {
				ByteStream stream;
				SerializeOwnerUnsigned(stream, version);
				_digestOwnerUnsigned = sha256(stream.GetBytes());
			}

			return _digestOwnerUnsigned;
		}

		const uint256 &CRCProposal::DigestNormalCRCouncilMemberUnsigned(uint8_t version) const {
			if (_digestCRCouncilMemberUnsigned == 0) {
				ByteStream stream;
				SerializeCRCouncilMemberUnsigned(stream, version);
				_digestCRCouncilMemberUnsigned = sha256(stream.GetBytes());
			}

			return _digestCRCouncilMemberUnsigned;
		}

		size_t CRCProposal::EstimateSize(uint8_t version) const {
			ByteStream stream;
			size_t size = 0;

			size += sizeof(uint16_t);

			size += stream.WriteVarUint(_categoryData.size());
			size += _categoryData.size();

			size += stream.WriteVarUint(_ownerPublicKey.size());
			size += _ownerPublicKey.size();

			size += _draftHash.size();

			size += stream.WriteVarUint(_budgets.size());

			ByteStream byteStream;
			for (size_t i = 0; i < _budgets.size(); ++i) {
				_budgets[i].Serialize(byteStream);
			}
			size += byteStream.GetBytes().size();

			size += _recipient.ProgramHash().size();

			size += stream.WriteVarUint(_signature.size());
			size += _signature.size();

			size += _crCouncilMemberDID.ProgramHash().size();

			size += stream.WriteVarUint(_crCouncilMemberSignature.size());
			size += _crCouncilMemberSignature.size();

			return size;
		}

		// normal or elip
		void CRCProposal::SerializeOwnerUnsigned(ByteStream &ostream, uint8_t version) const {
			ostream.WriteUint16(_type);
			ostream.WriteVarString(_categoryData);
			ostream.WriteVarBytes(_ownerPublicKey);
			ostream.WriteBytes(_draftHash);
			ostream.WriteVarUint(_budgets.size());
			for (size_t i = 0; i < _budgets.size(); ++i) {
				_budgets[i].Serialize(ostream);
			}
			ostream.WriteBytes(_recipient.ProgramHash());
		}

		bool CRCProposal::DeserializeOwnerUnsigned(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadVarString(_categoryData)) {
				SPVLOG_ERROR("deserialize categoryData");
				return false;
			}

			if (!stream.ReadVarBytes(_ownerPublicKey)) {
				SPVLOG_ERROR("deserialize owner PublicKey");
				return false;
			}

			if (!stream.ReadBytes(_draftHash)) {
				SPVLOG_ERROR("deserialize draftHash");
				return false;
			}

			uint64_t count = 0;
			if (!stream.ReadVarUint(count)) {
				SPVLOG_ERROR("deserialize budgets size");
				return false;
			}
			_budgets.resize(count);
			for (size_t i = 0; i < count; ++i) {
				if (!_budgets[i].Deserialize(stream)) {
					SPVLOG_ERROR("deserialize bugets");
					return false;
				}
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize recipient");
				return false;
			}
			_recipient = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeCRCouncilMemberUnsigned(ByteStream &ostream, uint8_t version) const {
			SerializeOwnerUnsigned(ostream, version);

			ostream.WriteVarBytes(_signature);

			ostream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

		bool CRCProposal::DeserializeCRCouncilMemberUnsigned(const ByteStream &istream, uint8_t version) {
			if (!DeserializeOwnerUnsigned(istream, version)) {
				SPVLOG_ERROR("deserialize unsigned");
				return false;
			}

			if (!istream.ReadVarBytes(_signature)) {
				SPVLOG_ERROR("deserialize signature");
				return false;
			}

			uint168 programHash;
			if (!istream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize sponsor did");
				return false;
			}
			_crCouncilMemberDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeNormalOrELIP(ByteStream &stream, uint8_t version) const {
			SerializeCRCouncilMemberUnsigned(stream, version);

			stream.WriteVarBytes(_crCouncilMemberSignature);
		}

		bool CRCProposal::DeserializeNormalOrELIP(const ByteStream &stream, uint8_t version) {
			if (!DeserializeCRCouncilMemberUnsigned(stream, version)) {
				SPVLOG_ERROR("CRCProposal deserialize crc unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
				SPVLOG_ERROR("CRCProposal deserialize crc signature");
				return false;
			}

			return true;
		}

		// change owner
		void CRCProposal::SerializeChangeOwnerUnsigned(ByteStream &stream, uint8_t version) const {
			uint16_t type = _type;
			stream.WriteUint16(type);

			stream.WriteVarBytes(_categoryData);
			stream.WriteVarBytes(_ownerPublicKey);
			stream.WriteBytes(_draftHash);
			stream.WriteBytes(_targetProposalHash);
			stream.WriteBytes(_newRecipient.ProgramHash());
			stream.WriteVarBytes(_newOwnerPublicKey);
		}

		bool CRCProposal::DeserializeChangeOwnerUnsigned(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadVarString(_categoryData)) {
				SPVLOG_ERROR("deserialize categoryData");
				return false;
			}

			if (!stream.ReadVarBytes(_ownerPublicKey)) {
				SPVLOG_ERROR("deserialize owner PublicKey");
				return false;
			}

			if (!stream.ReadBytes(_draftHash)) {
				SPVLOG_ERROR("deserialize draftHash");
				return false;
			}

			if (!stream.ReadBytes(_targetProposalHash)) {
				SPVLOG_ERROR("deserialize target proposal hash");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize new recipient");
				return false;
			}
			_newRecipient = Address(programHash);

			if (!stream.ReadVarBytes(_newOwnerPublicKey)) {
				SPVLOG_ERROR("deserialize new owner PublicKey");
				return false;
			}

			return true;
		}

		void CRCProposal::SerializeChangeOwnerCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
			SerializeChangeOwnerUnsigned(stream, version);

			stream.WriteVarBytes(_signature);
			stream.WriteVarBytes(_newOwnerSignature);
			stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

		bool CRCProposal::DeserializeChangeOwnerCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
			if (!DeserializeChangeOwnerUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize change owner unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_signature)) {
				SPVLOG_ERROR("deserialize change owner signature");
				return false;
			}

			if (!stream.ReadVarBytes(_newOwnerSignature)) {
				SPVLOG_ERROR("deserialize change owner new owner signature");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize sponsor did");
				return false;
			}
			_crCouncilMemberDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeChangeOwner(ByteStream &stream, uint8_t version) const {
			SerializeChangeOwnerCRCouncilMemberUnsigned(stream, version);

			stream.WriteVarBytes(_crCouncilMemberSignature);
		}

		bool CRCProposal::DeserializeChangeOwner(const ByteStream &stream, uint8_t version) {
			if (!DeserializeChangeOwnerCRCouncilMemberUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize change owner cr council member unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
				SPVLOG_ERROR("deserialize change owner cr council member signature");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposal::ToJsonChangeOwnerUnsigned(uint8_t version) const {
			nlohmann::json j;

			j[JsonKeyType] = _type;
			j[JsonKeyCategoryData] = _categoryData;
			j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
			j[JsonKeyDraftHash] = _draftHash.GetHex();
			j[JsonKeyTargetProposalHash] = _targetProposalHash.GetHex();
			j[JsonKeyNewRecipient] = _newRecipient.String();
			j[JsonKeyNewOwnerPublicKey] = _newOwnerPublicKey.getHex();

			return j;
		}

		void CRCProposal::FromJsonChangeOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
			_categoryData = j[JsonKeyCategoryData].get<std::string>();
			_ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
			_draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
			_targetProposalHash.SetHex(j[JsonKeyTargetProposalHash].get<std::string>());
			_newRecipient = Address(j[JsonKeyNewRecipient].get<std::string>());
			_newOwnerPublicKey.setHex(j[JsonKeyNewOwnerPublicKey].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJsonChangeOwnerCRCouncilMemberUnsigned(uint8_t version) const {
			nlohmann::json j = ToJsonChangeOwnerUnsigned(version);

			j[JsonKeySignature] = _signature.getHex();
			j[JsonKeyNewOwnerSignature] = _newOwnerSignature.getHex();
			j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();

			return j;
		}

		void CRCProposal::FromJsonChangeOwnerCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonChangeOwnerUnsigned(j, version);

			_signature.setHex(j[JsonKeySignature].get<std::string>());
			_newOwnerSignature.setHex(j[JsonKeyNewOwnerSignature].get<std::string>());
			_crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

		bool CRCProposal::IsValidChangeOwnerUnsigned(uint8_t version) const {
			if (_type != changeProposalOwner) {
				SPVLOG_ERROR("invalid type: {}", _type);
				return false;
			}

			if (_categoryData.size() > 4096) {
				SPVLOG_ERROR("category data exceed 4096 bytes");
				return false;
			}

			try {
				Key key(_ownerPublicKey);
				Key key1(_newOwnerPublicKey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid pubkey");
				return false;
			}

			if (_draftHash == 0 || _targetProposalHash == 0) {
				SPVLOG_ERROR("invalid hash");
				return false;
			}

			if (!_newRecipient.Valid()) {
				SPVLOG_ERROR("invalid new recipient");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValidChangeOwnerCRCouncilMemberUnsigned(uint8_t version) const {
			if (!IsValidChangeOwnerUnsigned(version)) {
				return false;
			}

			try {
				if (!Key(_ownerPublicKey).Verify(DigestChangeOwnerUnsigned(version), _signature)) {
					SPVLOG_ERROR("verify signature fail");
					return false;
				}

				if (!Key(_newOwnerPublicKey).Verify(DigestChangeOwnerUnsigned(version), _newOwnerSignature)) {
					SPVLOG_ERROR("verify new owner signature fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("verify signature exception: {}", e.what());
				return false;
			}

			if (!_crCouncilMemberDID.Valid()) {
				SPVLOG_ERROR("invalid cr council member did");
				return false;
			}

			return true;
		}

		const uint256 &CRCProposal::DigestChangeOwnerUnsigned(uint8_t version) const {
			if (_digestChangeOwnerUnsigned == 0) {
				ByteStream stream;
				SerializeChangeOwnerUnsigned(stream, version);
				_digestChangeOwnerUnsigned = sha256(stream.GetBytes());
			}

			return _digestChangeOwnerUnsigned;
		}

		const uint256 &CRCProposal::DigestChangeOwnerCRCouncilMemberUnsigned(uint8_t version) const {
			if (_digestChangeOwnerCRCouncilMemberUnsigned == 0) {
				ByteStream stream;
				SerializeChangeOwnerCRCouncilMemberUnsigned(stream, version);
				_digestChangeOwnerCRCouncilMemberUnsigned = sha256(stream.GetBytes());
			}

			return _digestChangeOwnerCRCouncilMemberUnsigned;
		}

		// terminate proposal
		void CRCProposal::SerializeTerminateProposalUnsigned(ByteStream &stream, uint8_t version) const {
			uint16_t type = _type;
			stream.WriteUint16(type);
			stream.WriteVarString(_categoryData);
			stream.WriteVarBytes(_ownerPublicKey);
			stream.WriteBytes(_draftHash);
			stream.WriteBytes(_targetProposalHash);
		}

		bool CRCProposal::DeserializeTerminateProposalUnsigned(const ByteStream &stream, uint8_t version) {
			if (!stream.ReadVarString(_categoryData)) {
				SPVLOG_ERROR("deserialize terminate proposal category data");
				return false;
			}

			if (!stream.ReadVarBytes(_ownerPublicKey)) {
				SPVLOG_ERROR("deserialize terminate proposal owner pubkey");
				return false;
			}

			if (!stream.ReadBytes(_draftHash)) {
				SPVLOG_ERROR("deserialize terminate proposal draft hash");
				return false;
			}

			if (!stream.ReadBytes(_targetProposalHash)) {
				SPVLOG_ERROR("deserialize terminate proposal target proposal hash");
				return false;
			}

			return true;
		}

		void CRCProposal::SerializeTerminateProposalCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
			SerializeTerminateProposalUnsigned(stream, version);

			stream.WriteVarBytes(_signature);
			stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

		bool CRCProposal::DeserializeTerminateProposalCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
			if (!DeserializeTerminateProposalUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize terminate proposal unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_signature)) {
				SPVLOG_ERROR("deserialize terminate proposal signature");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize sponsor did");
				return false;
			}
			_crCouncilMemberDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeTerminateProposal(ByteStream &stream, uint8_t version) const {
			SerializeTerminateProposalCRCouncilMemberUnsigned(stream, version);

			stream.WriteVarBytes(_crCouncilMemberSignature);
		}

		bool CRCProposal::DeserializeTerminateProposal(const ByteStream &stream, uint8_t version) {
			if (!DeserializeTerminateProposalCRCouncilMemberUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize terminate proposal cr council member unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
				SPVLOG_ERROR("deserialize change owner cr council member signature");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposal::ToJsonTerminateProposalOwnerUnsigned(uint8_t version) const {
			nlohmann::json j;

			j[JsonKeyType] = _type;
			j[JsonKeyCategoryData] = _categoryData;
			j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
			j[JsonKeyDraftHash] = _draftHash.GetHex();
			j[JsonKeyTargetProposalHash] = _targetProposalHash.GetHex();

			return j;
		}

		void CRCProposal::FromJsonTerminateProposalOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
			_categoryData = j[JsonKeyCategoryData].get<std::string>();
			_ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
			_draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
			_targetProposalHash.SetHex(j[JsonKeyTargetProposalHash].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJsonTerminateProposalCRCouncilMemberUnsigned(uint8_t version) const {
			nlohmann::json j = ToJsonTerminateProposalOwnerUnsigned(version);

			j[JsonKeySignature] = _signature.getHex();
			j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();

			return j;
		}

		void CRCProposal::FromJsonTerminateProposalCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonTerminateProposalOwnerUnsigned(j, version);

			_signature.setHex(j[JsonKeySignature].get<std::string>());
			_crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

		bool CRCProposal::IsValidTerminateProposalOwnerUnsigned(uint8_t version) const {
			if (_type != terminateProposal) {
				SPVLOG_ERROR("invalid type: {}", _type);
				return false;
			}

			if (_categoryData.size() > 4096) {
				SPVLOG_ERROR("category data exceed 4096 bytes");
				return false;
			}

			try {
				Key key(_ownerPublicKey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid pubkey");
				return false;
			}

			if (_draftHash == 0 || _targetProposalHash == 0) {
				SPVLOG_ERROR("invalid hash");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValidTerminateProposalCRCouncilMemberUnsigned(uint8_t version) const {
			if (!IsValidTerminateProposalOwnerUnsigned(version)) {
				SPVLOG_ERROR("terminate proposal unsigned is not valid");
				return false;
			}

			try {
				if (!Key(_ownerPublicKey).Verify(DigestChangeOwnerUnsigned(version), _signature)) {
					SPVLOG_ERROR("verify signature fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("verify signature exception: {}", e.what());
				return false;
			}

			if (!_crCouncilMemberDID.Valid()) {
				SPVLOG_ERROR("invalid cr council member did");
				return false;
			}

			return true;
		}

		const uint256 &CRCProposal::DigestTerminateProposalOwnerUnsigned(uint8_t version) const {
			if (_digestTerminateProposalOwnerUnsigned == 0) {
				ByteStream stream;
				SerializeTerminateProposalUnsigned(stream, version);
				_digestTerminateProposalOwnerUnsigned = sha256(stream.GetBytes());
			}

			return _digestTerminateProposalOwnerUnsigned;
		}

		const uint256 &CRCProposal::DigestTerminateProposalCRCouncilMemberUnsigned(uint8_t version) const {
			if (_digestTerminateProposalCRCouncilMemberUnsigned == 0) {
				ByteStream stream;
				SerializeTerminateProposalCRCouncilMemberUnsigned(stream, version);
				_digestTerminateProposalCRCouncilMemberUnsigned = sha256(stream.GetBytes());
			}

			return _digestTerminateProposalCRCouncilMemberUnsigned;
		}

		// change secretary general
		void CRCProposal::SerializeSecretaryElectionOwnerUnsigned(ByteStream &stream, uint8_t version) const {
			uint16_t type = _type;
			stream.WriteUint16(type);
			stream.WriteVarString(_categoryData);
			stream.WriteVarBytes(_ownerPublicKey);
			stream.WriteBytes(_draftHash);
			stream.WriteVarBytes(_secretaryPublicKey);
			stream.WriteBytes(_secretaryDID.ProgramHash());
		}

		bool CRCProposal::DeserializeSecretaryElectionOwnerUnsigned(const ByteStream &stream, uint8_t verion) {
			if (!stream.ReadVarString(_categoryData)) {
				SPVLOG_ERROR("deserialize category data");
				return false;
			}

			if (!stream.ReadVarBytes(_ownerPublicKey)) {
				SPVLOG_ERROR("deserialize owner pubkey");
				return false;
			}

			if (!stream.ReadBytes(_draftHash)) {
				SPVLOG_ERROR("deserialize draft hash");
				return false;
			}

			if (!stream.ReadVarBytes(_secretaryPublicKey)) {
				SPVLOG_ERROR("deserialize secretary pubkey");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize sponsor did");
				return false;
			}
			_secretaryDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeSecretaryElectionSecretaryUnsigned(ByteStream &stream, uint8_t version) const {
			SerializeSecretaryElectionOwnerUnsigned(stream, version);

			stream.WriteVarBytes(_signature);
		}

		bool CRCProposal::DeserializeSecretaryElectionSecretaryUnsigned(const ByteStream &stream, uint8_t version) {
			if (!DeserializeSecretaryElectionOwnerUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize change secretary unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_signature)) {
				SPVLOG_ERROR("deserialize signature");
				return false;
			}

			return true;
		}

		void CRCProposal::SerializeSecretaryElectionCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const {
			SerializeSecretaryElectionSecretaryUnsigned(stream, version);

			stream.WriteVarBytes(_secretarySignature);
			stream.WriteBytes(_crCouncilMemberDID.ProgramHash());
		}

		bool CRCProposal::DeserializeSecretaryElectionCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version) {
			if (!DeserializeSecretaryElectionSecretaryUnsigned(stream, version)) {
				SPVLOG_ERROR("deserialize change secretary secretary unsigned");
				return false;
			}

			if (!stream.ReadVarBytes(_secretarySignature)) {
				SPVLOG_ERROR("deserialize secretary signature");
				return false;
			}

			uint168 programHash;
			if (!stream.ReadBytes(programHash)) {
				SPVLOG_ERROR("deserialize cr council mem did");
				return false;
			}
			_crCouncilMemberDID = Address(programHash);

			return true;
		}

		void CRCProposal::SerializeSecretaryElection(ByteStream &stream, uint8_t version) const {
			SerializeSecretaryElectionCRCouncilMemberUnsigned(stream, version);

			stream.WriteVarBytes(_crCouncilMemberSignature);
		}

		bool CRCProposal::DeserializeSecretaryElection(const ByteStream &stream, uint8_t version) {
			if (!DeserializeSecretaryElectionCRCouncilMemberUnsigned(stream, version)) {
				return false;
			}

			if (!stream.ReadVarBytes(_crCouncilMemberSignature)) {
				SPVLOG_ERROR("deserialize change secretary cr council member signature");
				return false;
			}

			return true;
		}

		nlohmann::json CRCProposal::ToJsonSecretaryElectionOwnerUnsigned(uint8_t version) const {
			nlohmann::json j;

			j[JsonKeyType] = _type;
			j[JsonKeyCategoryData] = _categoryData;
			j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
			j[JsonKeyDraftHash] = _draftHash.GetHex();
			j[JsonKeySecretaryPublicKey] = _secretaryPublicKey.getHex();
			j[JsonKeySecretaryDID] = _secretaryDID.String();

			return j;
		}

		void CRCProposal::FromJsonSecretaryElectionOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint16_t>());
			_categoryData = j[JsonKeyCategoryData].get<std::string>();
			_ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
			_draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
			_secretaryPublicKey.setHex(j[JsonKeySecretaryPublicKey].get<std::string>());
			_secretaryDID = Address(j[JsonKeySecretaryDID].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJsonSecretaryElectionSecretaryUnsigned(uint8_t version) const {
			nlohmann::json j;

			j = ToJsonSecretaryElectionOwnerUnsigned(version);
			j[JsonKeySignature] = _signature.getHex();

			return j;
		}

		void CRCProposal::FromJsonSecretaryElectionSecretaryUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonSecretaryElectionOwnerUnsigned(j, version);
			_signature.setHex(j[JsonKeySignature].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJsonSecretaryElectionCRCouncilMemberUnsigned(uint8_t version) const {
			nlohmann::json j;

			j = ToJsonSecretaryElectionSecretaryUnsigned(version);
			j[JsonKeySecretarySignature] = _secretarySignature.getHex();
			j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();

			return j;
		}

		void CRCProposal::FromJsonSecretaryElectionCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonSecretaryElectionSecretaryUnsigned(j, version);
			_secretarySignature.setHex(j[JsonKeySecretarySignature].get<std::string>());
			_crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

		bool CRCProposal::IsValidSecretaryElectionOwnerUnsiged(uint8_t version) const {
			if (_type != secretaryGeneralElection) {
				SPVLOG_ERROR("invalid type: {}", _type);
				return false;
			}

			if (_categoryData.size() > 4096) {
				SPVLOG_ERROR("category data exceed 4096 bytes");
				return false;
			}

			try {
				Key key(_ownerPublicKey);
				Key key1(_secretaryPublicKey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid ...");
				return false;
			}

			if (!_secretaryDID.Valid()) {
				SPVLOG_ERROR("invalid secretary did");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValidSecretaryElectionSecretaryUnsigned(uint8_t version) const {
			if (!IsValidSecretaryElectionOwnerUnsiged(version)) {
				SPVLOG_ERROR("secretary election owner unsigned not valid");
				return false;
			}

			try {
				if (!Key(_ownerPublicKey).Verify(DigestSecretaryElectionOwnerUnsigned(version), _signature)) {
					SPVLOG_ERROR("verify owner signature fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("verify signature exception: {}", e.what());
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValidSecretaryElectionCRCouncilMemberUnsigned(uint8_t version) const {
			if (!IsValidSecretaryElectionSecretaryUnsigned(version)) {
				SPVLOG_ERROR("secretary election secretary unsigned not valid");
				return false;
			}

			try {
				if (!Key(_secretaryPublicKey).Verify(DigestSecretaryElectionSecretaryUnsigned(version), _secretarySignature)) {
					SPVLOG_ERROR("verify secretary signature fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("verify signature exception: {}", e.what());
				return false;
			}

			if (!_crCouncilMemberDID.Valid()) {
				SPVLOG_ERROR("invalid cr committee did");
				return false;
			}

			return true;
		}

		const uint256 &CRCProposal::DigestSecretaryElectionOwnerUnsigned(uint8_t version) const {
			if (_digestSecretaryElectionOwnerUnsigned == 0) {
				ByteStream stream;
				SerializeSecretaryElectionOwnerUnsigned(stream, version);
				_digestSecretaryElectionOwnerUnsigned = sha256(stream.GetBytes());
			}

			return _digestOwnerUnsigned;
		}

		const uint256 &CRCProposal::DigestSecretaryElectionSecretaryUnsigned(uint8_t version) const {
			if (_digestSecretaryElectionSecretaryUnsigned == 0) {
				ByteStream stream;
				SerializeSecretaryElectionSecretaryUnsigned(stream, version);
				_digestSecretaryElectionSecretaryUnsigned = sha256(stream.GetBytes());
			}

			return _digestSecretaryElectionSecretaryUnsigned;
		}

		const uint256 &CRCProposal::DigestSecretaryElectionCRCouncilMemberUnsigned(uint8_t version) const {
			if (_digestSecretaryElectionCRCouncilMemberUnsigned == 0) {
				ByteStream stream;
				SerializeSecretaryElectionCRCouncilMemberUnsigned(stream, version);
				_digestSecretaryElectionCRCouncilMemberUnsigned = sha256(stream.GetBytes());
			}

			return _digestSecretaryElectionCRCouncilMemberUnsigned;
		}

		// top serialize or deserialize
		void CRCProposal::Serialize(ByteStream &stream, uint8_t version) const {
			switch (_type) {
				case changeProposalOwner:
					SerializeChangeOwner(stream, version);
					break;

				case terminateProposal:
					SerializeTerminateProposal(stream, version);
					break;

				case secretaryGeneralElection:
					SerializeSecretaryElection(stream, version);
					break;

				case elip:
				case normal:
					SerializeNormalOrELIP(stream, version);
					break;

				default:
					SPVLOG_ERROR("serialize cr proposal unknown type");
					break;
			}
		}

		bool CRCProposal::Deserialize(const ByteStream &stream, uint8_t version) {
			uint16_t type = 0;
			if (!stream.ReadUint16(type)) {
				SPVLOG_ERROR("deserialize type");
				return false;
			}
			_type = CRCProposal::Type(type);

			bool r = false;
			switch (_type) {
				case changeProposalOwner:
					r = DeserializeChangeOwner(stream, version);
					break;

				case terminateProposal:
					r = DeserializeTerminateProposal(stream, version);
					break;

				case secretaryGeneralElection:
					r = DeserializeSecretaryElection(stream, version);
					break;

				case elip:
				case normal:
					r = DeserializeNormalOrELIP(stream, version);
					break;

				default:
					SPVLOG_ERROR("unknow type: {}", _type);
					r = false;
					break;
			}

			return r;
		}

		nlohmann::json CRCProposal::ToJsonNormalOwnerUnsigned(uint8_t version) const {
			nlohmann::json j;
			j[JsonKeyType] = _type;
			j[JsonKeyCategoryData] = _categoryData;
			j[JsonKeyOwnerPublicKey] = _ownerPublicKey.getHex();
			j[JsonKeyDraftHash] = _draftHash.GetHex();
			j[JsonKeyBudgets] = _budgets;
			j[JsonKeyRecipient] = _recipient.String();
			return j;
		}

		void CRCProposal::FromJsonNormalOwnerUnsigned(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint8_t>());
			_categoryData = j[JsonKeyCategoryData].get<std::string>();
			_ownerPublicKey.setHex(j[JsonKeyOwnerPublicKey].get<std::string>());
			_draftHash.SetHex(j[JsonKeyDraftHash].get<std::string>());
			_budgets = j[JsonKeyBudgets].get<std::vector<Budget>>();
			_recipient = Address(j[JsonKeyRecipient].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJsonNormalCRCouncilMemberUnsigned(uint8_t version) const {
			nlohmann::json j = ToJsonNormalOwnerUnsigned(version);
			j[JsonKeySignature] = _signature.getHex();
			j[JsonKeyCRCouncilMemberDID] = _crCouncilMemberDID.String();
			return j;
		}

		void CRCProposal::FromJsonNormalCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version) {
			FromJsonNormalOwnerUnsigned(j, version);
			_signature.setHex(j[JsonKeySignature].get<std::string>());
			_crCouncilMemberDID = Address(j[JsonKeyCRCouncilMemberDID].get<std::string>());
		}

		nlohmann::json CRCProposal::ToJson(uint8_t version) const {
			nlohmann::json j;

			switch (_type) {
				case normal:
				case elip:
					j = ToJsonNormalCRCouncilMemberUnsigned(version);
					j[JsonKeyCRCouncilMemberSignature] = _crCouncilMemberSignature.getHex();
					break;

				case secretaryGeneralElection:
					j = ToJsonSecretaryElectionCRCouncilMemberUnsigned(version);
					j[JsonKeyCRCouncilMemberSignature] = _crCouncilMemberSignature.getHex();
					break;

				case changeProposalOwner:
					j = ToJsonChangeOwnerCRCouncilMemberUnsigned(version);
					j[JsonKeyCRCouncilMemberSignature] = _crCouncilMemberSignature.getHex();
					break;

				case terminateProposal:
					j = ToJsonTerminateProposalCRCouncilMemberUnsigned(version);
					j[JsonKeyCRCouncilMemberSignature] = _crCouncilMemberSignature.getHex();
					break;

				default:
					SPVLOG_ERROR("unknow type: {}", _type);
					break;
			}
			return j;
		}

		void CRCProposal::FromJson(const nlohmann::json &j, uint8_t version) {
			_type = CRCProposal::Type(j[JsonKeyType].get<uint8_t>());

			switch (_type) {
				case normal:
				case elip:
					FromJsonNormalCRCouncilMemberUnsigned(j, version);
					_crCouncilMemberSignature.setHex(j[JsonKeyCRCouncilMemberSignature].get<std::string>());
					break;

				case secretaryGeneralElection:
					FromJsonSecretaryElectionCRCouncilMemberUnsigned(j, version);
					_crCouncilMemberSignature.setHex(j[JsonKeyCRCouncilMemberSignature].get<std::string>());
					break;

				case changeProposalOwner:
					FromJsonChangeOwnerCRCouncilMemberUnsigned(j, version);
					_crCouncilMemberSignature.setHex(j[JsonKeyCRCouncilMemberSignature].get<std::string>());
					break;

				case terminateProposal:
					FromJsonTerminateProposalCRCouncilMemberUnsigned(j, version);
					_crCouncilMemberSignature.setHex(j[JsonKeyCRCouncilMemberSignature].get<std::string>());
					break;

				default:
					SPVLOG_ERROR("unknow type: {}", _type);
					break;
			}
		}

		bool CRCProposal::IsValidNormalOwnerUnsigned(uint8_t version) const {
			if (_type >= CRCProposal::maxType) {
				SPVLOG_ERROR("invalid proposal type: {}", _type);
				return false;
			}

			if (_categoryData.size() > 4096) {
				SPVLOG_ERROR("category data exceed 4096 bytes");
				return false;
			}

			try {
				Key key(_ownerPublicKey);
			} catch (const std::exception &e) {
				SPVLOG_ERROR("invalid proposal owner pubkey");
				return false;
			}

			for (const Budget &budget : _budgets) {
				if (!budget.IsValid()) {
					SPVLOG_ERROR("invalid budget");
					return false;
				}
			}

			if (!_recipient.Valid()) {
				SPVLOG_ERROR("invalid recipient");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValidNormalCRCouncilMemberUnsigned(uint8_t version) const {
			if (!IsValidNormalOwnerUnsigned(version))
				return false;

			try {
				if (!Key(_ownerPublicKey).Verify(DigestNormalOwnerUnsigned(version), _signature)) {
					SPVLOG_ERROR("verify owner signature fail");
					return false;
				}
			} catch (const std::exception &e) {
				SPVLOG_ERROR("verify signature exception: {}", e.what());
				return false;
			}

			if (!_crCouncilMemberDID.Valid()) {
				SPVLOG_ERROR("invalid cr committee did");
				return false;
			}

			return true;
		}

		bool CRCProposal::IsValid(uint8_t version) const {
			bool isValid = false;
			switch (_type) {
				case normal:
				case elip:
					isValid = IsValidNormalCRCouncilMemberUnsigned(version);
					break;

				case secretaryGeneralElection:
					isValid = IsValidSecretaryElectionCRCouncilMemberUnsigned(version);
					break;

				case changeProposalOwner:
					isValid = IsValidChangeOwnerCRCouncilMemberUnsigned(version);
					break;

				case terminateProposal:
					isValid = IsValidTerminateProposalCRCouncilMemberUnsigned(version);
					break;

				default:
					break;
			}

			if (_crCouncilMemberSignature.empty()) {
				SPVLOG_ERROR("cr committee not signed");
				isValid = false;
			}

			return isValid;
		}

		CRCProposal &CRCProposal::operator=(const CRCProposal &payload) {
			_type = payload._type;
			_categoryData = payload._categoryData;
			_ownerPublicKey = payload._ownerPublicKey;
			_draftHash = payload._draftHash;
			_budgets = payload._budgets;
			_recipient = payload._recipient;
			_targetProposalHash = payload._targetProposalHash;
			_newRecipient = payload._newRecipient;
			_newOwnerPublicKey = payload._newOwnerPublicKey;
			_secretaryPublicKey = payload._secretaryPublicKey;
			_secretaryDID = payload._secretaryDID;
			_signature = payload._signature;
			_newOwnerSignature = payload._newOwnerSignature;
			_secretarySignature = payload._secretarySignature;

			_crCouncilMemberDID = payload._crCouncilMemberDID;
			_crCouncilMemberSignature = payload._crCouncilMemberSignature;
			return *this;
		}

		IPayload &CRCProposal::operator=(const IPayload &payload) {
			try {
				const CRCProposal &crcProposal = dynamic_cast<const CRCProposal &>(payload);
				operator=(crcProposal);
			} catch (const std::bad_cast &e) {
				SPVLOG_ERROR("payload is not instance of CRCProposal");
			}
			return *this;
		}
	}
}