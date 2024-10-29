use alloc::string::{String, ToString};
use alloc::{format, vec};
use core::ops::{Deref, Mul};
use core::str::FromStr;

use bytes::BytesMut;
use ethereum_types::{H256, U256};
use hex;
use rlp::{Decodable, DecoderError, Encodable, Rlp};
use ur_registry::pb::protoc::EthTx;

use crate::erc20::encode_erc20_transfer_calldata;
use crate::normalizer::{normalize_price, normalize_value};
use crate::structs::{EthereumSignature, TransactionAction};
use crate::traits::BaseTransaction;
use crate::{impl_base_transaction, H160};

type Bytes = vec::Vec<u8>;

#[derive(Clone)]
pub struct TransactionRecoveryId(pub u64);

impl Deref for TransactionRecoveryId {
    type Target = u64;

    fn deref(&self) -> &u64 {
        &self.0
    }
}

impl TransactionRecoveryId {
    pub fn standard(self) -> u8 {
        if self.0 == 27 || self.0 == 28 || self.0 > 36 {
            ((self.0 - 1) % 2) as u8
        } else {
            4
        }
    }

    pub fn chain_id(self) -> Option<u64> {
        if self.0 > 36 {
            Some((self.0 - 35) / 2)
        } else {
            None
        }
    }
}

pub struct TransactionSignature {
    v: TransactionRecoveryId,
    r: H256,
    s: H256,
}

impl Encodable for TransactionSignature {
    fn rlp_append(&self, s: &mut rlp::RlpStream) {
        s.append(&self.v.0);
        s.append(&self.r);
        s.append(&self.s);
    }
}

impl TryFrom<EthereumSignature> for TransactionSignature {
    type Error = ();

    fn try_from(value: EthereumSignature) -> Result<Self, Self::Error> {
        let v = value.0;
        let rs_vec = value.1;
        // 0;32 is r   32;64 is s
        let r = H256::from_slice(&rs_vec[0..32]);
        let s = H256::from_slice(&rs_vec[32..64]);
        Ok(Self {
            v: TransactionRecoveryId(v),
            r,
            s,
        })
    }
}

impl TransactionSignature {
    #[must_use]
    pub fn new(v: u64, r: H256, s: H256) -> Self {
        Self {
            v: TransactionRecoveryId(v),
            r,
            s,
        }
    }

    pub fn v(&self) -> u64 {
        self.v.0
    }

    #[allow(dead_code)]
    pub fn r(&self) -> &H256 {
        &self.r
    }

    pub fn s(&self) -> &H256 {
        &self.s
    }

    #[allow(dead_code)]
    pub fn is_low_s(&self) -> bool {
        const LOWER: H256 = H256([
            0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0x5d, 0x57, 0x6e, 0x73, 0x57, 0xa4, 0x50, 0x1d, 0xdf, 0xe9, 0x2f, 0x46,
            0x68, 0x1b, 0x20, 0xa0,
        ]);

        self.s <= LOWER
    }
}

pub struct LegacyTransaction {
    nonce: U256,
    gas_price: U256,
    gas_limit: U256,
    action: TransactionAction,
    value: U256,
    input: Bytes,
    signature: Option<TransactionSignature>,
}

impl LegacyTransaction {
    pub fn new(
        nonce: u64,
        gas_price: u64,
        gas_limit: u64,
        action: TransactionAction,
        value: u64,
        input: String,
    ) -> Self {
        Self {
            nonce: U256::from(nonce),
            gas_price: U256::from(gas_price),
            gas_limit: U256::from(gas_limit),
            action,
            value: U256::from(value),
            input: Bytes::from(hex::decode(input).unwrap()),
            signature: None,
        }
    }

    pub fn decode_raw(bytes: &[u8]) -> Result<LegacyTransaction, DecoderError> {
        rlp::decode(bytes)
    }

    pub fn encode_raw(&self) -> BytesMut {
        rlp::encode(self)
    }

    pub fn set_signature(mut self, signature: TransactionSignature) -> Self {
        self.signature = Some(signature);
        self
    }
    pub fn get_gas_price(&self) -> U256 {
        self.gas_price
    }

    pub fn get_gas_limit(&self) -> U256 {
        self.gas_limit
    }

    pub fn get_value(&self) -> U256 {
        self.value
    }

    pub fn get_data(&self) -> Bytes {
        self.input.clone()
    }

    pub fn decode_rsv(rlp: &Rlp) -> Result<Option<TransactionSignature>, DecoderError> {
        if rlp.item_count()? == 6 {
            return Ok(None);
        } else if rlp.item_count()? == 9 {
            let v = rlp.val_at(6)?;
            let r = {
                let mut rarr = [0_u8; 32];
                rlp.val_at::<U256>(7)?.to_big_endian(&mut rarr);
                H256::from(rarr)
            };
            let s = {
                let mut sarr = [0_u8; 32];
                rlp.val_at::<U256>(8)?.to_big_endian(&mut sarr);
                H256::from(sarr)
            };
            return Ok(Some(TransactionSignature::new(v, r, s)));
        }
        Err(DecoderError::RlpIncorrectListLen)
    }

    pub fn chain_id(&self) -> u64 {
        match &self.signature {
            Some(signature) => {
                if signature.s().is_zero() {
                    // unsigned eip155 compatible transaction
                    signature.v()
                } else {
                    // signed eip 155 compatible transaction
                    // v = {0, 1} + chain_id * 2 + 27 + 8
                    (signature.v() - 35) / 2
                }
            }
            None => 1,
        }
    }

    pub fn is_eip155_compatible(&self) -> bool {
        match &self.signature {
            Some(signature) => {
                if signature.s().is_zero() {
                    // unsigned eip155 compatible transaction
                    true
                } else {
                    // signed eip 155 compatible transaction
                    // v = {0, 1} + chain_id * 2 + 27 + 8
                    signature.v() > 35
                }
            }
            None => false,
        }
    }
}

impl_base_transaction!(LegacyTransaction);

impl Decodable for LegacyTransaction {
    fn decode(rlp: &Rlp) -> Result<Self, DecoderError> {
        if rlp.item_count()? < 6 {
            return Err(DecoderError::RlpIncorrectListLen);
        }
        let signature = LegacyTransaction::decode_rsv(rlp)?;
        Ok(Self {
            nonce: rlp.val_at(0)?,
            gas_price: rlp.val_at(1)?,
            gas_limit: rlp.val_at(2)?,
            action: rlp.val_at(3)?,
            value: rlp.val_at(4)?,
            input: rlp.val_at(5)?,
            signature,
        })
    }
}

impl Encodable for LegacyTransaction {
    // Legacy Transaction Fields must order as below
    fn rlp_append(&self, s: &mut rlp::RlpStream) {
        s.begin_unbounded_list();
        s.append(&self.nonce);
        s.append(&self.gas_price);
        s.append(&self.gas_limit);
        s.append(&self.action);
        s.append(&self.value);
        s.append(&self.input);
        // chain_id will remove when the signature is added
        if let None = &self.signature {
            s.append(&self.chain_id());
            s.append(&vec![]);
            s.append(&vec![]);
        }
        if let Some(signature) = &self.signature {
            signature.rlp_append(s);
        }
        s.finalize_unbounded_list();
    }
}

pub struct ParsedLegacyTransaction {
    pub(crate) nonce: u32,
    pub(crate) gas_price: String,
    pub(crate) gas_limit: String,
    pub(crate) to: String,
    pub(crate) value: String,
    pub(crate) input: String,
    pub(crate) chain_id: u64,
    pub(crate) max_txn_fee: String,
}

impl From<LegacyTransaction> for ParsedLegacyTransaction {
    fn from(value: LegacyTransaction) -> Self {
        Self {
            nonce: value.nonce.as_u32(),
            gas_price: normalize_price(value.gas_price.as_u64()),
            gas_limit: value.gas_limit.to_string(),
            to: format!("0x{}", hex::encode(value.get_to())),
            value: normalize_value(value.get_value()),
            input: hex::encode(value.get_data()),
            chain_id: value.chain_id(),
            max_txn_fee: normalize_value(value.gas_price.mul(value.gas_limit)),
        }
    }
}

impl TryFrom<EthTx> for LegacyTransaction {
    type Error = ();
    fn try_from(eth_tx: EthTx) -> Result<Self, Self::Error> {
        // check this transaction is erc20 transaction or not
        if let Some(erc20_override) = eth_tx.r#override {
            let _contract_address = erc20_override.contract_address;
            // generate  erc20 transfer inputdata
            let to = crate::H160::from_str(&eth_tx.to).unwrap();
            let amount = crate::U256::from(eth_tx.value.parse::<u64>().unwrap());
            let input_data = encode_erc20_transfer_calldata(to, amount);
            let legacy_transaction = LegacyTransaction::new(
                eth_tx.nonce as u64,
                eth_tx.gas_price.parse::<u64>().unwrap(),
                eth_tx.gas_limit.parse::<u64>().unwrap(),
                TransactionAction::Call(to),
                0,
                input_data,
            );
            Ok(legacy_transaction)
        } else {
            let legacy_transaction = LegacyTransaction::new(
                eth_tx.nonce as u64,
                eth_tx.gas_price.parse::<u64>().unwrap(),
                eth_tx.gas_limit.parse::<u64>().unwrap(),
                TransactionAction::Call(H160::from_str(eth_tx.to.as_str()).unwrap()),
                eth_tx.value.parse::<u64>().unwrap(),
                hex::encode(eth_tx.memo.as_bytes()),
            );
            Ok(legacy_transaction)
        }
    }
}

#[cfg(test)]
mod tests {
    use core::str::FromStr;

    use crate::crypto::keccak256;
    use crate::H160;

    use super::*;

    extern crate std;
    #[test]
    fn test_transfer_erc20_legacy_transaction() {
        let tx = LegacyTransaction::new(
            33,
            15198060006,
            46000,
            TransactionAction::Call(H160::from_str("0xfe2c232adDF66539BFd5d1Bd4B2cc91D358022a2").unwrap()),
            200000000000000,
            "a9059cbb00000000000000000000000049ab56b91fc982fd6ec1ec7bb87d74efa6da30ab00000000000000000000000000000000000000000000000001480ff69d129e2d".to_string(),
        );
        let unsigned_serialize_tx = tx.encode_raw();
        let unsigned_tx_hex = hex::encode(&unsigned_serialize_tx);

        assert_eq!(
            "f86f21850389dffde682b3b094fe2c232addf66539bfd5d1bd4b2cc91d358022a286b5e620f48000b844a9059cbb00000000000000000000000049ab56b91fc982fd6ec1ec7bb87d74efa6da30ab00000000000000000000000000000000000000000000000001480ff69d129e2d018080",
            unsigned_tx_hex
        );
        // sign tx
        let r = "0x35df2b615912b8be79a13c9b0a1540ade55434ab68778a49943442a9e6d3141a".to_string();
        let s = "0x0a6e33134ba47c1f1cda59ec3ef62a59d4da6a9d111eb4e447828574c1c94f66".to_string();
        let v = 38;
        let r_vec_32: [u8; 32] = hex::decode(&r[2..]).unwrap().try_into().unwrap();
        let s_vec_32: [u8; 32] = hex::decode(&s[2..]).unwrap().try_into().unwrap();

        let signature = TransactionSignature::new(v, H256::from(r_vec_32), H256::from(s_vec_32));
        let tx = tx.set_signature(signature);

        // raw tx
        let signed_tx = tx.encode_raw();
        let signed_tx_hex = hex::encode(&signed_tx);
        // tx id === tx hash
        let signed_tx_hash = keccak256(&signed_tx);
        let signed_tx_hash_hex = hex::encode(&signed_tx_hash);

        assert_eq!(
            "fec8bfea5ec13ad726de928654cd1733b1d81d2d2916ac638e6b9a245f034ace".to_string(),
            signed_tx_hash_hex
        );

        assert_eq!(
            "f8af21850389dffde682b3b094fe2c232addf66539bfd5d1bd4b2cc91d358022a286b5e620f48000b844a9059cbb00000000000000000000000049ab56b91fc982fd6ec1ec7bb87d74efa6da30ab00000000000000000000000000000000000000000000000001480ff69d129e2d26a035df2b615912b8be79a13c9b0a1540ade55434ab68778a49943442a9e6d3141aa00a6e33134ba47c1f1cda59ec3ef62a59d4da6a9d111eb4e447828574c1c94f66",
            signed_tx_hex
        );

        let decoded_tx = LegacyTransaction::decode_raw(&signed_tx).unwrap();
        assert_eq!(decoded_tx.nonce, U256::from(33));
        let hex_data = hex::encode(decoded_tx.input);
        assert_eq!(decoded_tx.gas_price, U256::from(15198060006u64));
        assert_eq!(decoded_tx.gas_limit, U256::from(46000));
        assert_eq!(200000000000000, decoded_tx.value.as_u64());
        assert_eq!(hex_data, "a9059cbb00000000000000000000000049ab56b91fc982fd6ec1ec7bb87d74efa6da30ab00000000000000000000000000000000000000000000000001480ff69d129e2d");
    }

    #[test]
    fn test_eth_transfer_legacy_transaction() {
        let tx = LegacyTransaction::new(
            33,
            15198060006,
            46000,
            TransactionAction::Call(
                H160::from_str("0xfe2c232adDF66539BFd5d1Bd4B2cc91D358022a2").unwrap(),
            ),
            200000000000000,
            "".to_string(),
        );
        assert_eq!(0, tx.input.len());
        let unsigned_tx = tx.encode_raw();
        let unsigned_tx_hex = hex::encode(&unsigned_tx);
        assert_eq!(
            unsigned_tx_hex,
            "ea21850389dffde682b3b094fe2c232addf66539bfd5d1bd4b2cc91d358022a286b5e620f4800080018080"
                .to_string()
        );
        let unsigned_tx_hash = keccak256(&unsigned_tx);
        let unsigned_tx_hash_hex = hex::encode(&unsigned_tx_hash);
        // sign tx
        let r = "0x35df2b615912b8be79a13c9b0a1540ade55434ab68778a49943442a9e6d3141a".to_string();
        let s = "0x0a6e33134ba47c1f1cda59ec3ef62a59d4da6a9d111eb4e447828574c1c94f66".to_string();
        let v = 38;
        let r_vec_32: [u8; 32] = hex::decode(&r[2..]).unwrap().try_into().unwrap();
        let s_vec_32: [u8; 32] = hex::decode(&s[2..]).unwrap().try_into().unwrap();

        let signature = TransactionSignature::new(v, H256::from(r_vec_32), H256::from(s_vec_32));
        let tx = tx.set_signature(signature);

        // raw tx
        let signed_tx = tx.encode_raw();
        let signed_tx_hex = hex::encode(&signed_tx);
        assert_eq!(
            "f86a21850389dffde682b3b094fe2c232addf66539bfd5d1bd4b2cc91d358022a286b5e620f480008026a035df2b615912b8be79a13c9b0a1540ade55434ab68778a49943442a9e6d3141aa00a6e33134ba47c1f1cda59ec3ef62a59d4da6a9d111eb4e447828574c1c94f66".to_string(),
            signed_tx_hex
        );

        // tx id === tx hash
        let signed_tx_hash = keccak256(&signed_tx);
        let signed_tx_hash_hex = hex::encode(&signed_tx_hash);
        assert_eq!(
            "c20d7398343b00d1bc5aaf8f6d9a879217003d6cc726053cd41fb960977cd066".to_string(),
            signed_tx_hash_hex
        );

        let decoded_tx = LegacyTransaction::decode_raw(&signed_tx).unwrap();
        assert_eq!(decoded_tx.nonce, U256::from(33));

        assert_eq!(decoded_tx.gas_price, U256::from(15198060006u64));
        assert_eq!(decoded_tx.gas_limit, U256::from(46000));
        assert_eq!(decoded_tx.chain_id(), 1);
        assert_eq!(200000000000000, decoded_tx.value.as_u64());
        assert_eq!(decoded_tx.input, Bytes::from("".as_bytes()));
    }

    #[test]
    fn test_legacy_transaction2() {
        let tx = LegacyTransaction::new(
            36,
            5807408099,
            46000,
            TransactionAction::Call(
                H160::from_str("0xfe2c232adDF66539BFd5d1Bd4B2cc91D358022a2").unwrap(),
            ),
            200000000000000,
            "".to_string(),
        );

        // sign tx
        let r = "0xb1ac33d7688ac7e94e7867269774a919191b6f3a02de4a0c88c2f61b06715cab".to_string();
        let s = "0x22432d033f8dbb6de527fa3fa6025ebf9a9980ab2f9ba757f259dd7844aafbea".to_string();
        let v = 37;
        let r_vec_32: [u8; 32] = hex::decode(&r[2..]).unwrap().try_into().unwrap();
        let s_vec_32: [u8; 32] = hex::decode(&s[2..]).unwrap().try_into().unwrap();

        let signature = TransactionSignature::new(v, H256::from(r_vec_32), H256::from(s_vec_32));
        let tx = tx.set_signature(signature);

        // raw tx
        let signed_tx = tx.encode_raw();
        let signed_tx_hex = hex::encode(&signed_tx);
        // tx id === tx hash
        let signed_tx_hash = keccak256(&signed_tx);
        let signed_tx_hash_hex = hex::encode(&signed_tx_hash);
        assert_eq!(
            "e01dd745d8cc0983f288da28ab288f7d1be809164c83ae477bdb927d31f49a7c".to_string(),
            signed_tx_hash_hex
        )
    }
}
