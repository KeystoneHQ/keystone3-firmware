use alloc::string::{String, ToString};
use alloc::{format, vec};
use bytes::BytesMut;
use core::ops::{Deref, Mul};
use core::str::FromStr;

use ethereum_types::{H160, H256, U256};
use rlp::{Decodable, DecoderError, Encodable, Rlp};
use third_party::hex;

type Bytes = vec::Vec<u8>;
use crate::erc20::encode_erc20_transfer_calldata;
use crate::structs::EthereumSignature;
use serde::{Deserialize, Deserializer, Serialize, Serializer};
use third_party::ur_registry::pb::protoc::EthTx;

const CHAIN_ID: u64 = 1u64;
const LEGACY_TRANSACTION_TYPE: u64 = 0u64;
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

impl Encodable for TransactionSignature {
    fn rlp_append(&self, s: &mut rlp::RlpStream) {
        s.append(&self.v.0);
        s.append(&self.r);
        s.append(&self.s);
    }
}

pub struct LegacyTransactionV2 {
    nonce: U256,
    gas_price: U256,
    gas_limit: U256,
    chain_id: u64,
    to: Bytes,
    value: U256,
    data: Bytes,
    r#type: U256,
    access_list: Bytes,
    signature: Option<TransactionSignature>,
}

fn add_hex_prefix(hex: &str) -> String {
    if hex.starts_with("0x") {
        hex.to_string()
    } else {
        format!("0x{}", hex)
    }
}

impl LegacyTransactionV2 {
    pub fn new(
        nonce: u32,
        gas_price: u64,
        gas_limit: u64,
        chain_id: u64,
        to: String,
        value: u64,
        data: String,
    ) -> Self {
        let to: H160 = to.parse().unwrap();
        Self {
            nonce: U256::from(nonce),
            gas_price: U256::from(gas_price),
            gas_limit: U256::from(gas_limit),
            chain_id,
            to: Bytes::from(to.as_bytes()),
            value: U256::from(value),
            data: Bytes::from(hex::decode(data).unwrap()),
            signature: None,
            r#type: U256::from(0),
            access_list: Bytes::from(vec![]),
        }
    }

    pub fn set_signature(mut self, signature: TransactionSignature) -> Self {
        self.signature = Some(signature);
        self
    }

    pub fn decode_raw(bytes: &[u8]) -> Result<LegacyTransactionV2, DecoderError> {
        rlp::decode(bytes)
    }
    pub fn encode_raw(&self) -> BytesMut {
        rlp::encode(self)
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

    fn decode_rsv(rlp: &Rlp) -> Result<Option<TransactionSignature>, DecoderError> {
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

impl TryFrom<EthTx> for LegacyTransactionV2 {
    type Error = ();
    fn try_from(eth_tx: EthTx) -> Result<Self, Self::Error> {
        // check this transaction is erc20 transaction or not
        if let Some(erc20_override) = eth_tx.r#override {
            let contract_address = erc20_override.contract_address;
            // generate  erc20 transfer inputdata
            let to = crate::H160::from_str(&eth_tx.to).unwrap();
            let amount = crate::U256::from(eth_tx.value.parse::<u64>().unwrap());
            let input_data = encode_erc20_transfer_calldata(to, amount);
            let legacy_transaction = LegacyTransactionV2::new(
                eth_tx.nonce as u32,
                eth_tx.gas_price.parse::<u64>().unwrap(),
                eth_tx.gas_limit.parse::<u64>().unwrap(),
                CHAIN_ID,
                contract_address,
                0,
                input_data,
            );
            Ok(legacy_transaction)
        } else {
            let legacy_transaction = LegacyTransactionV2::new(
                eth_tx.nonce as u32,
                eth_tx.gas_price.parse::<u64>().unwrap(),
                eth_tx.gas_limit.parse::<u64>().unwrap(),
                CHAIN_ID,
                eth_tx.to,
                eth_tx.value.parse::<u64>().unwrap(),
                eth_tx.memo,
            );
            Ok(legacy_transaction)
        }
    }
}

impl Decodable for LegacyTransactionV2 {
    fn decode(rlp: &Rlp) -> Result<Self, DecoderError> {
        if rlp.item_count()? < 6 {
            return Err(DecoderError::RlpIncorrectListLen);
        }
        let signature = LegacyTransactionV2::decode_rsv(rlp)?;
        Ok(Self {
            nonce: rlp.val_at(0)?,
            gas_price: rlp.val_at(1)?,
            gas_limit: rlp.val_at(2)?,
            to: rlp.val_at(3)?,
            value: rlp.val_at(4)?,
            r#type: U256::from(LEGACY_TRANSACTION_TYPE),
            chain_id: CHAIN_ID,
            data: rlp.val_at(5)?,
            access_list: vec![],
            signature,
        })
    }
}

impl Encodable for LegacyTransactionV2 {
    // Legacy Transaction Fields must order as below
    fn rlp_append(&self, s: &mut rlp::RlpStream) {
        s.begin_unbounded_list();
        s.append(&self.nonce);
        s.append(&self.gas_price);
        s.append(&self.gas_limit);
        s.append(&self.to);
        s.append(&self.value);
        s.append(&self.data);
        // chain_id will remove when the signature is added
        if let None = &self.signature {
            s.append(&self.chain_id);
            s.append(&vec![]);
            s.append(&vec![]);
        }
        if let Some(signature) = &self.signature {
            signature.rlp_append(s);
        }
        s.finalize_unbounded_list();
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::crypto::keccak256;
    extern crate std;
    #[test]
    fn test_convet_hex_test() {
        let nonce = 33;
        let hex_nonce = add_hex_prefix(&format!("{:x}", nonce));
        assert_eq!(hex_nonce, "0x21");
    }

    #[test]
    fn test_transfer_erc20_legacy_transaction() {
        let tx = LegacyTransactionV2::new(
            33,
            15198060006,
            46000,
            1,
            "0xfe2c232adDF66539BFd5d1Bd4B2cc91D358022a2".to_string(),
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

        let decoded_tx = LegacyTransactionV2::decode_raw(&signed_tx).unwrap();
        assert_eq!(decoded_tx.nonce, U256::from(33));
        let hex_data = hex::encode(decoded_tx.data);
        assert_eq!(decoded_tx.gas_price, U256::from(15198060006u64));
        assert_eq!(decoded_tx.gas_limit, U256::from(46000));
        assert_eq!(decoded_tx.chain_id, 1);
        assert_eq!(200000000000000, decoded_tx.value.as_u64());
        assert_eq!(hex_data, "a9059cbb00000000000000000000000049ab56b91fc982fd6ec1ec7bb87d74efa6da30ab00000000000000000000000000000000000000000000000001480ff69d129e2d");
        assert_eq!(decoded_tx.r#type, U256::from(0));
    }

    #[test]
    fn test_eth_transfer_legacy_transaction() {
        let tx = LegacyTransactionV2::new(
            33,
            15198060006,
            46000,
            1,
            "0xfe2c232adDF66539BFd5d1Bd4B2cc91D358022a2".to_string(),
            200000000000000,
            "".to_string(),
        );
        assert_eq!(0, tx.data.len());
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

        let decoded_tx = LegacyTransactionV2::decode_raw(&signed_tx).unwrap();
        assert_eq!(decoded_tx.nonce, U256::from(33));

        assert_eq!(decoded_tx.gas_price, U256::from(15198060006u64));
        assert_eq!(decoded_tx.gas_limit, U256::from(46000));
        assert_eq!(decoded_tx.chain_id, 1);
        assert_eq!(200000000000000, decoded_tx.value.as_u64());
        assert_eq!(decoded_tx.data, Bytes::from("".as_bytes()));
        assert_eq!(decoded_tx.r#type, U256::from(0));
    }

    #[test]
    fn test_legacy_transaction2() {
        let tx = LegacyTransactionV2::new(
            36,
            5807408099,
            46000,
            1,
            "0xfe2c232adDF66539BFd5d1Bd4B2cc91D358022a2".to_string(),
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
