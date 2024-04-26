#![feature(exclusive_range_pattern)]
#![no_std]
#![feature(error_in_core)]
extern crate alloc;

pub mod abi;
pub mod address;
mod crypto;
mod eip1559_transaction;
pub mod eip712;
pub mod erc20;
pub mod errors;
mod legacy_transaction;
mod normalizer;
pub mod structs;
mod traits;

pub type Bytes = Vec<u8>;

use crate::crypto::keccak256;
use crate::eip1559_transaction::{EIP1559Transaction, ParsedEIP1559Transaction};
use crate::errors::{EthereumError, Result};
use crate::structs::{EthereumSignature, ParsedEthereumTransaction, PersonalMessage, TypedData};
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;

pub use legacy_transaction::*;

use crate::eip712::eip712::{Eip712, TypedData as Eip712TypedData};
use third_party::hex;
use third_party::secp256k1::{Message, PublicKey};
use third_party::serde_json;

pub fn parse_legacy_tx(tx_hex: Vec<u8>, from_key: PublicKey) -> Result<ParsedEthereumTransaction> {
    ParsedEthereumTransaction::from_legacy(
        ParsedLegacyTransaction::from(LegacyTransaction::decode_raw(tx_hex.as_slice())?),
        from_key,
    )
}

pub fn parse_fee_market_tx(
    tx_hex: Vec<u8>,
    from_key: PublicKey,
) -> Result<ParsedEthereumTransaction> {
    ParsedEthereumTransaction::from_eip1559(
        ParsedEIP1559Transaction::from(EIP1559Transaction::decode_raw(tx_hex.as_slice())?),
        from_key,
    )
}

pub fn parse_personal_message(tx_hex: Vec<u8>, from_key: PublicKey) -> Result<PersonalMessage> {
    let raw_messge = hex::encode(tx_hex.clone());
    let utf8_message = match String::from_utf8(tx_hex) {
        Ok(utf8_message) => {
            if app_utils::is_cjk(&utf8_message) {
                "".to_string()
            } else {
                utf8_message
            }
        }
        Err(_e) => "".to_string(),
    };
    PersonalMessage::from(raw_messge, utf8_message, from_key)
}

pub fn parse_typed_data_message(tx_hex: Vec<u8>, from_key: PublicKey) -> Result<TypedData> {
    let utf8_message =
        String::from_utf8(tx_hex).map_err(|e| EthereumError::InvalidUtf8Error(e.to_string()))?;
    let typed_data: Eip712TypedData = serde_json::from_str(&utf8_message)
        .map_err(|e| EthereumError::InvalidTypedData(e.to_string(), utf8_message))?;
    TypedData::from(Into::into(typed_data), from_key)
}

pub fn sign_legacy_tx(sign_data: Vec<u8>, seed: &[u8], path: &String) -> Result<EthereumSignature> {
    let tx = LegacyTransaction::decode_raw(sign_data.as_slice())?;
    let hash = keccak256(sign_data.as_slice());
    let message = Message::from_digest_slice(&hash).unwrap();
    keystore::algorithms::secp256k1::sign_message_by_seed(seed, path, &message)
        .map_err(|e| EthereumError::SignFailure(e.to_string()))
        .map(|(rec_id, rs)| {
            let v = rec_id as u64 + 27;
            if tx.is_eip155_compatible() {
                EthereumSignature(v + tx.chain_id() * 2 + 8, rs)
            } else {
                EthereumSignature(v, rs)
            }
        })
}

pub fn sign_fee_markey_tx(
    sign_data: Vec<u8>,
    seed: &[u8],
    path: &String,
) -> Result<EthereumSignature> {
    // sign_data should starts with 0x02
    let first_byte = *sign_data.first().ok_or(EthereumError::InvalidTransaction)?;
    if first_byte != 0x02 {
        return Err(EthereumError::InvalidTransaction);
    }

    let hash = keccak256(sign_data.as_slice());
    let message = Message::from_digest_slice(&hash).unwrap();
    keystore::algorithms::secp256k1::sign_message_by_seed(seed, path, &message)
        .map_err(|e| EthereumError::SignFailure(e.to_string()))
        .map(|(rec_id, rs)| EthereumSignature(rec_id as u64, rs))
}

const PREFIX_PERSONAL_MESSAGE: &str = "\u{0019}Ethereum Signed Message:\n";

pub fn sign_personal_message(
    sign_data: Vec<u8>,
    seed: &[u8],
    path: &String,
) -> Result<EthereumSignature> {
    let mut message = format!("{}{}", PREFIX_PERSONAL_MESSAGE, sign_data.len())
        .as_bytes()
        .to_vec();
    message.extend(sign_data);
    let hash = keccak256(message.as_slice());
    let message =
        Message::from_digest_slice(&hash).map_err(|e| EthereumError::SignFailure(e.to_string()))?;
    keystore::algorithms::secp256k1::sign_message_by_seed(seed, path, &message)
        .map_err(|e| EthereumError::SignFailure(e.to_string()))
        .map(|(rec_id, rs)| {
            let v = rec_id as u64 + 27;
            EthereumSignature(v, rs)
        })
}

pub fn sign_typed_data_message(
    sign_data: Vec<u8>,
    seed: &[u8],
    path: &String,
) -> Result<EthereumSignature> {
    let utf8_message =
        String::from_utf8(sign_data).map_err(|e| EthereumError::InvalidUtf8Error(e.to_string()))?;
    let typed_data: Eip712TypedData = serde_json::from_str(&utf8_message)
        .map_err(|e| EthereumError::InvalidTypedData(e.to_string(), utf8_message))?;

    let hash = typed_data
        .encode_eip712()
        .map_err(|e| EthereumError::HashTypedDataError(e.to_string()))?;
    let message =
        Message::from_digest_slice(&hash).map_err(|e| EthereumError::SignFailure(e.to_string()))?;

    keystore::algorithms::secp256k1::sign_message_by_seed(seed, path, &message)
        .map_err(|e| EthereumError::SignFailure(e.to_string()))
        .map(|(rec_id, rs)| {
            let v = rec_id as u64 + 27;
            EthereumSignature(v, rs)
        })
}

#[cfg(test)]
mod tests {

    extern crate std;

    use crate::alloc::string::ToString;
    use crate::eip712::eip712::{Eip712, TypedData as Eip712TypedData};
    use crate::{
        parse_fee_market_tx, parse_personal_message, parse_typed_data_message,
        sign_personal_message, sign_typed_data_message,
    };
    use keystore::algorithms::secp256k1::get_public_key_by_seed;
    use third_party::hex;
    use third_party::serde_json;

    #[test]
    fn test_parse_personal_message() {
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let path = "m/44'/60'/0'/0/0".to_string();

        let pubkey = get_public_key_by_seed(&seed, &path).unwrap();
        let sign_data =
            hex::decode("4578616d706c652060706572736f6e616c5f7369676e60206d657373616765").unwrap();
        let result = parse_personal_message(sign_data, pubkey).unwrap();
        assert_eq!(
            "4578616d706c652060706572736f6e616c5f7369676e60206d657373616765",
            result.raw_message
        );
        assert_eq!("Example `personal_sign` message", result.utf8_message);
        assert_eq!("0x9858EfFD232B4033E47d90003D41EC34EcaEda94", result.from);
    }

    #[test]
    fn test_sign_personal_message() {
        let sign_data =
            hex::decode("4578616d706c652060706572736f6e616c5f7369676e60206d657373616765").unwrap();
        let path = "m/44'/60'/0'/0/0".to_string();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let message = sign_personal_message(sign_data, &seed, &path).unwrap();
        assert_eq!("b836ae2bac525ae9d2799928cf6f52919cb2ed5e5e52ca26e3b3cdbeb136ca2f618da0e6413a6aa3aaa722fbc2bcc87f591b8b427ee6915916f257de8125810e1b",
                   hex::encode(message.serialize()));
    }

    #[test]
    fn test_parse_tx() {
        let sign_data = hex::decode("f902b6011f8405f5e1008503a5fe2f0883026b08943fc91a3afd70395cd496c647d5a6cc9d4b2b7fad872386f26fc10000b902843593564c000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000064996e5f00000000000000000000000000000000000000000000000000000000000000020b000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000004000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000002386f26fc1000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000002386f26fc10000000000000000000000000000000000000000000000000000f84605ccc515414000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002bc02aaa39b223fe8d0a0e5c4f27ead9083c756cc20001f46b175474e89094c44da98b954eedeac495271d0f000000000000000000000000000000000000000000c0").unwrap();
        let path = "m/44'/60'/0'/0/0".to_string();
        //to: 3fc91a3afd70395cd496c647d5a6cc9d4b2b7fad
        //data: 3593564c000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000064996e5f00000000000000000000000000000000000000000000000000000000000000020b000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000004000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000002386f26fc1000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000002386f26fc10000000000000000000000000000000000000000000000000000f84605ccc515414000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002bc02aaa39b223fe8d0a0e5c4f27ead9083c756cc20001f46b175474e89094c44da98b954eedeac495271d0f000000000000000000000000000000000000000000
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let pubkey = get_public_key_by_seed(&seed, &path).unwrap();
        let result = parse_fee_market_tx(sign_data, pubkey).unwrap();
        assert_eq!(31, result.nonce);
        assert_eq!(1, result.chain_id);
        assert_eq!("0x9858EfFD232B4033E47d90003D41EC34EcaEda94", result.from);
        assert_eq!("0x3fc91a3afd70395cd496c647d5a6cc9d4b2b7fad", result.to);
        assert_eq!("0.01", result.value);
        assert_eq!(None, result.gas_price);
        assert_eq!("15.669800712 Gwei", result.max_fee_per_gas.unwrap());
        assert_eq!("0.1 Gwei", result.max_priority_fee_per_gas.unwrap());
        assert_eq!("0.002483224", result.max_fee.unwrap());
        assert_eq!("0.000015847", result.max_priority.unwrap());
        assert_eq!("158472", result.gas_limit);
        assert_eq!("0.002483224", result.max_txn_fee);
        assert_eq!(result.input, "3593564c000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000064996e5f00000000000000000000000000000000000000000000000000000000000000020b000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000004000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000002386f26fc1000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000002386f26fc10000000000000000000000000000000000000000000000000000f84605ccc515414000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002bc02aaa39b223fe8d0a0e5c4f27ead9083c756cc20001f46b175474e89094c44da98b954eedeac495271d0f000000000000000000000000000000000000000000");
    }

    #[test]
    fn test_parse_typed_data() {
        let sign_data = "7b227479706573223a7b22454950373132446f6d61696e223a5b7b226e616d65223a226e616d65222c2274797065223a22737472696e67227d2c7b226e616d65223a2276657273696f6e222c2274797065223a22737472696e67227d2c7b226e616d65223a22636861696e4964222c2274797065223a2275696e74323536227d2c7b226e616d65223a22766572696679696e67436f6e7472616374222c2274797065223a2261646472657373227d5d2c224f72646572436f6d706f6e656e7473223a5b7b226e616d65223a226f666665726572222c2274797065223a2261646472657373227d2c7b226e616d65223a227a6f6e65222c2274797065223a2261646472657373227d2c7b226e616d65223a226f66666572222c2274797065223a224f666665724974656d5b5d227d2c7b226e616d65223a22737461727454696d65222c2274797065223a2275696e74323536227d2c7b226e616d65223a22656e6454696d65222c2274797065223a2275696e74323536227d2c7b226e616d65223a227a6f6e6548617368222c2274797065223a2262797465733332227d2c7b226e616d65223a2273616c74222c2274797065223a2275696e74323536227d2c7b226e616d65223a22636f6e647569744b6579222c2274797065223a2262797465733332227d2c7b226e616d65223a22636f756e746572222c2274797065223a2275696e74323536227d5d2c224f666665724974656d223a5b7b226e616d65223a22746f6b656e222c2274797065223a2261646472657373227d5d2c22436f6e73696465726174696f6e4974656d223a5b7b226e616d65223a22746f6b656e222c2274797065223a2261646472657373227d2c7b226e616d65223a226964656e7469666965724f724372697465726961222c2274797065223a2275696e74323536227d2c7b226e616d65223a227374617274416d6f756e74222c2274797065223a2275696e74323536227d2c7b226e616d65223a22656e64416d6f756e74222c2274797065223a2275696e74323536227d2c7b226e616d65223a22726563697069656e74222c2274797065223a2261646472657373227d5d7d2c227072696d61727954797065223a224f72646572436f6d706f6e656e7473222c22646f6d61696e223a7b226e616d65223a22536561706f7274222c2276657273696f6e223a22312e31222c22636861696e4964223a2231222c22766572696679696e67436f6e7472616374223a22307830303030303030303030366333383532636245663365303845386446323839313639456445353831227d2c226d657373616765223a7b226f666665726572223a22307866333946643665353161616438384636463463653661423838323732373963666646623932323636222c226f66666572223a5b7b22746f6b656e223a22307841363034303630383930393233466634303065386336663532393034363141383341454441436563227d5d2c22737461727454696d65223a2231363538363435353931222c22656e6454696d65223a2231363539323530333836222c227a6f6e65223a22307830303443303035303030303061443130344437444264303065336165304135433030353630433030222c227a6f6e6548617368223a22307830303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030222c2273616c74223a223136313738323038383937313336363138222c22636f6e647569744b6579223a22307830303030303037623032323330303931613765643031323330303732663730303661303034643630613864346537316435393962383130343235306630303030222c22746f74616c4f726967696e616c436f6e73696465726174696f6e4974656d73223a2232222c22636f756e746572223a2230227d7d";
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let path = "m/44'/60'/0'/0/0".to_string();

        let pubkey = get_public_key_by_seed(&seed, &path).unwrap();
        let sign_data = hex::decode(sign_data).unwrap();
        let result = parse_typed_data_message(sign_data, pubkey).unwrap();
        assert_eq!("Seaport", &result.name);
        assert_eq!("1.1", &result.version);
        assert_eq!("1", &result.chain_id);
        assert_eq!(
            "0x00000000006c3852cbef3e08e8df289169ede581",
            &result.verifying_contract
        );
    }

    #[test]

    fn test_sign_typed_data() {
        let sign_data =
            hex::decode("7b227479706573223a7b22454950373132446f6d61696e223a5b7b226e616d65223a226e616d65222c2274797065223a22737472696e67227d2c7b226e616d65223a2276657273696f6e222c2274797065223a22737472696e67227d2c7b226e616d65223a22636861696e4964222c2274797065223a2275696e74323536227d2c7b226e616d65223a22766572696679696e67436f6e7472616374222c2274797065223a2261646472657373227d5d2c224f72646572436f6d706f6e656e7473223a5b7b226e616d65223a226f666665726572222c2274797065223a2261646472657373227d2c7b226e616d65223a227a6f6e65222c2274797065223a2261646472657373227d2c7b226e616d65223a226f66666572222c2274797065223a224f666665724974656d5b5d227d2c7b226e616d65223a22737461727454696d65222c2274797065223a2275696e74323536227d2c7b226e616d65223a22656e6454696d65222c2274797065223a2275696e74323536227d2c7b226e616d65223a227a6f6e6548617368222c2274797065223a2262797465733332227d2c7b226e616d65223a2273616c74222c2274797065223a2275696e74323536227d2c7b226e616d65223a22636f6e647569744b6579222c2274797065223a2262797465733332227d2c7b226e616d65223a22636f756e746572222c2274797065223a2275696e74323536227d5d2c224f666665724974656d223a5b7b226e616d65223a22746f6b656e222c2274797065223a2261646472657373227d5d2c22436f6e73696465726174696f6e4974656d223a5b7b226e616d65223a22746f6b656e222c2274797065223a2261646472657373227d2c7b226e616d65223a226964656e7469666965724f724372697465726961222c2274797065223a2275696e74323536227d2c7b226e616d65223a227374617274416d6f756e74222c2274797065223a2275696e74323536227d2c7b226e616d65223a22656e64416d6f756e74222c2274797065223a2275696e74323536227d2c7b226e616d65223a22726563697069656e74222c2274797065223a2261646472657373227d5d7d2c227072696d61727954797065223a224f72646572436f6d706f6e656e7473222c22646f6d61696e223a7b226e616d65223a22536561706f7274222c2276657273696f6e223a22312e31222c22636861696e4964223a2231222c22766572696679696e67436f6e7472616374223a22307830303030303030303030366333383532636245663365303845386446323839313639456445353831227d2c226d657373616765223a7b226f666665726572223a22307866333946643665353161616438384636463463653661423838323732373963666646623932323636222c226f66666572223a5b7b22746f6b656e223a22307841363034303630383930393233466634303065386336663532393034363141383341454441436563227d5d2c22737461727454696d65223a2231363538363435353931222c22656e6454696d65223a2231363539323530333836222c227a6f6e65223a22307830303443303035303030303061443130344437444264303065336165304135433030353630433030222c227a6f6e6548617368223a22307830303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030303030222c2273616c74223a223136313738323038383937313336363138222c22636f6e647569744b6579223a22307830303030303037623032323330303931613765643031323330303732663730303661303034643630613864346537316435393962383130343235306630303030222c22746f74616c4f726967696e616c436f6e73696465726174696f6e4974656d73223a2232222c22636f756e746572223a2230227d7d").unwrap();
        let path = "m/44'/60'/0'/0/0".to_string();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let message = sign_typed_data_message(sign_data, &seed, &path).unwrap();
        assert_eq!("042fd02150738ede751c43803d6d7bbbcf32c9afce40c861df87357639862c6a653d3307aa16aff363e3444cb418b72e9d715a6e8e479cb56f4ce3012eed87531b",
                   hex::encode(message.serialize()));
    }

    #[test]
    /// EIP-712 standard ambiguity: ethermint treats "string" type as standard, which may differ from the spec (source: https://eips.ethereum.org/EIPS/eip-712).
    fn test_non_standard_eip712_typed_data_parsing() {
        let utf8_message = r#"
        {
          "types": {
            "EIP712Domain": [
              {
                "name": "name",
                "type": "string"
              },
              {
                "name": "version",
                "type": "string"
              },
              {
                "name": "chainId",
                "type": "uint256"
              },
              {
                "name": "verifyingContract",
                "type": "string"
              },
              {
                "name": "salt",
                "type": "string"
              }
            ],
            "Tx": [
              {
                "name": "account_number",
                "type": "string"
              },
              {
                "name": "chain_id",
                "type": "string"
              },
              {
                "name": "fee",
                "type": "Fee"
              },
              {
                "name": "memo",
                "type": "string"
              },
              {
                "name": "msgs",
                "type": "Msg[]"
              },
              {
                "name": "sequence",
                "type": "string"
              }
            ],
            "Fee": [
              {
                "name": "feePayer",
                "type": "string"
              },
              {
                "name": "amount",
                "type": "Coin[]"
              },
              {
                "name": "gas",
                "type": "string"
              }
            ],
            "Coin": [
              {
                "name": "denom",
                "type": "string"
              },
              {
                "name": "amount",
                "type": "string"
              }
            ],
            "Msg": [
              {
                "name": "type",
                "type": "string"
              },
              {
                "name": "value",
                "type": "MsgValue"
              }
            ],
            "MsgValue": [
              {
                "name": "sender",
                "type": "string"
              },
              {
                "name": "routes",
                "type": "TypeRoutes[]"
              },
              {
                "name": "token_in",
                "type": "TypeTokenIn"
              },
              {
                "name": "token_out_min_amount",
                "type": "string"
              }
            ],
            "TypeRoutes": [
              {
                "name": "pool_id",
                "type": "uint64"
              },
              {
                "name": "token_out_denom",
                "type": "string"
              }
            ],
            "TypeTokenIn": [
              {
                "name": "denom",
                "type": "string"
              },
              {
                "name": "amount",
                "type": "string"
              }
            ]
          },
          "primaryType": "Tx",
          "domain": {
            "name": "Cosmos Web3",
            "version": "1.0.0",
            "chainId": 1100,
            "verifyingContract": "cosmos",
            "salt": "0"
          },
          "message": {
            "account_number": "832193",
            "chain_id": "dymension_1100-1",
            "fee": {
              "amount": [
                {
                  "amount": "4172140000000000",
                  "denom": "adym"
                }
              ],
              "gas": "208607",
              "feePayer": "dym1tqsdz785sqjnlggee0lwxjwfk6dl36ae6q5dx9"
            },
            "memo": "",
            "msgs": [
              {
                "type": "dymensionxyz/dymension/gamm/SwapExactAmountIn",
                "value": {
                  "sender": "dym1tqsdz785sqjnlggee0lwxjwfk6dl36ae6q5dx9",
                  "token_out_min_amount": "4747",
                  "token_in": {
                    "amount": "10000000000000000",
                    "denom": "adym"
                  },
                  "routes": [
                    {
                      "pool_id": "4",
                      "token_out_denom": "ibc/C4CFF46FD6DE35CA4CF4CE031E643C8FDC9BA4B99AE598E9B0ED98FE3A2319F9"
                    }
                  ]
                }
              }
            ],
            "sequence": "14"
          }
        }
        "#.to_string();

        let typed_data: Eip712TypedData = serde_json::from_str(&utf8_message).unwrap();
        let hash = typed_data.encode_eip712().unwrap();
        assert_eq!(
            Some("cosmos".to_string()),
            typed_data.domain.verifying_contract
        );
        assert_eq!(
            "61aca3c3989a82c6b606cdcbe6fc7e7d786dad2608b3f1806586261386154b68",
            hex::encode(&hash[..])
        );
    }

    #[test]
    fn test_non_standard_eip712_typed_data_sign() {
        let sign_data = r#"
        {
            "types": {
                "EIP712Domain": [
                    {
                        "name": "name",
                        "type": "string"
                    },
                    {
                        "name": "version",
                        "type": "string"
                    },
                    {
                        "name": "chainId",
                        "type": "uint256"
                    },
                    {
                        "name": "verifyingContract",
                        "type": "address"
                    },
                    {
                        "name": "salt",
                        "type": "string"
                    }
                ],
                "Tx": [
                    {
                        "name": "context",
                        "type": "string"
                    },
                    {
                        "name": "msgs",
                        "type": "string"
                    }
                ]
            },
            "primaryType": "Tx",
            "domain": {
                "name": "Injective Web3",
                "version": "1.0.0",
                "chainId": "0x1",
                "verifyingContract": "0xCcCCccccCCCCcCCCCCCcCcCccCcCCCcCcccccccC",
                "salt": "0"
            },
            "message": {
                "context": "{\"account_number\":37370,\"chain_id\":\"injective-1\",\"fee\":{\"amount\":[{\"denom\":\"inj\",\"amount\":\"50180000000000\"}],\"gas\":100360,\"payer\":\"inj1065f86fh88ptyrg8h5048zu0vyx7ex8ymwgr6h\"},\"memo\":\"\",\"sequence\":15,\"timeout_height\":63590762}",
                "msgs": "[{\"@type\":\"/cosmos.bank.v1beta1.MsgSend\",\"from_address\":\"inj1tqsdz785sqjnlggee0lwxjwfk6dl36aez5003n\",\"to_address\":\"inj1tqsdz785sqjnlggee0lwxjwfk6dl36aez5003n\",\"amount\":[{\"denom\":\"inj\",\"amount\":\"100000000000000\"}]}]"
            }
        }
        "#;
        let path = "m/44'/60'/0'/0/0".to_string();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let message = sign_typed_data_message(sign_data.as_bytes().to_vec(), &seed, &path).unwrap();
        assert_eq!(
            "cbf0b0d6ef4b47e1624267fb41e00de27f5812d5ff324f1817e73791905554844a80df5ead72fec8ac2be5fa9eebbfddb953577ea6f6f9df3c9dbf490035dd3f1c",
            hex::encode(message.serialize())
        );
    }

    #[test]
    fn test_parse_dym_vote_typed_data_msg() {
        let utf8_msg = r#"
        {
            "domain": {
                "chainId": 1100,
                "name": "Cosmos Web3",
                "salt": "0",
                "verifyingContract": "cosmos",
                "version": "1.0.0"
            },
            "message": {
                "account_number": "855752",
                "chain_id": "dymension_1100-1",
                "fee": {
                    "amount": [
                        {
                            "amount": "2527440000000000",
                            "denom": "adym"
                        }
                    ],
                    "feePayer": "dym1g65rdfk4sqxa82u6dwg5eyzwlqqhkjxggf4u0y",
                    "gas": "126372"
                },
                "memo": "",
                "msgs": [
                    {
                        "type": "cosmos-sdk/MsgVote",
                        "value": {
                            "option": 3,
                            "proposal_id": 12,
                            "voter": "dym1g65rdfk4sqxa82u6dwg5eyzwlqqhkjxggf4u0y"
                        }
                    }
                ],
                "sequence": "4"
            },
            "primaryType": "Tx",
            "types": {
                "Coin": [
                    {
                        "name": "denom",
                        "type": "string"
                    },
                    {
                        "name": "amount",
                        "type": "string"
                    }
                ],
                "EIP712Domain": [
                    {
                        "name": "name",
                        "type": "string"
                    },
                    {
                        "name": "version",
                        "type": "string"
                    },
                    {
                        "name": "chainId",
                        "type": "uint256"
                    },
                    {
                        "name": "verifyingContract",
                        "type": "string"
                    },
                    {
                        "name": "salt",
                        "type": "string"
                    }
                ],
                "Fee": [
                    {
                        "name": "feePayer",
                        "type": "string"
                    },
                    {
                        "name": "amount",
                        "type": "Coin[]"
                    },
                    {
                        "name": "gas",
                        "type": "string"
                    }
                ],
                "Msg": [
                    {
                        "name": "type",
                        "type": "string"
                    },
                    {
                        "name": "value",
                        "type": "MsgValue"
                    }
                ],
                "MsgValue": [
                    {
                        "name": "proposal_id",
                        "type": "uint64"
                    },
                    {
                        "name": "voter",
                        "type": "string"
                    },
                    {
                        "name": "option",
                        "type": "int32"
                    }
                ],
                "Tx": [
                    {
                        "name": "account_number",
                        "type": "string"
                    },
                    {
                        "name": "chain_id",
                        "type": "string"
                    },
                    {
                        "name": "fee",
                        "type": "Fee"
                    },
                    {
                        "name": "memo",
                        "type": "string"
                    },
                    {
                        "name": "msgs",
                        "type": "Msg[]"
                    },
                    {
                        "name": "sequence",
                        "type": "string"
                    }
                ]
            }
        }        
        "#;
        let typed_data: Eip712TypedData = serde_json::from_str(&utf8_msg).unwrap();
        let hash = typed_data.encode_eip712().unwrap();
        assert_eq!(
            "cosmos",
            typed_data.domain.verifying_contract.unwrap().as_str()
        );
        assert_eq!("Cosmos Web3", typed_data.domain.name.unwrap().as_str());
        assert_eq!(1100, typed_data.domain.chain_id.unwrap().as_u64());

        assert_eq!(
            "37fe5c140a9d70d91f786f300ce87be665e9551469e5747de4dce35edf129cf6",
            hex::encode(&hash[..])
        );
    }
}
