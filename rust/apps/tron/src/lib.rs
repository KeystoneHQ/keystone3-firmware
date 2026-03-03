#![no_std]
extern crate alloc;
extern crate core;

#[cfg(test)]
#[macro_use]
extern crate std;

use crate::errors::{Result, TronError};
use crate::utils::base58check_to_u8_slice;
use alloc::string::String;
use alloc::vec;

use ur_registry::pb::protoc;

mod address;
pub mod errors;
mod pb;
mod transaction;
mod utils;

pub use crate::address::get_address;
pub use crate::transaction::parser::{DetailTx, OverviewTx, ParsedTx, TxParser};
use crate::transaction::wrapped_tron::WrappedTron;
use crate::utils::keccak256;
use alloc::string::ToString;
use app_utils::keystone;
use core::str::FromStr;
use keystore::algorithms::secp256k1;
use transaction::checker::TxChecker;
use transaction::signer::Signer;

pub fn sign_raw_tx(
    raw_tx: protoc::Payload,
    context: keystone::ParseContext,
    seed: &[u8],
) -> Result<(String, String)> {
    let tx = WrappedTron::from_payload(raw_tx, &context)?;
    tx.sign(seed)
}

pub fn parse_raw_tx(raw_tx: protoc::Payload, context: keystone::ParseContext) -> Result<ParsedTx> {
    let tx_data = WrappedTron::from_payload(raw_tx, &context)?;
    tx_data.parse()
}

pub fn check_raw_tx(raw_tx: protoc::Payload, context: keystone::ParseContext) -> Result<()> {
    let tx_data = WrappedTron::from_payload(raw_tx, &context)?;
    tx_data.check(&context)
}

pub fn sign_tx_request(json_bytes: &[u8], hd_path: &String, seed: &[u8]) -> errors::Result<String> {
    let tx = WrappedTron::from_json_bytes(json_bytes, hd_path.clone())?;

    let pubkey = secp256k1::get_public_key_by_seed(seed, hd_path)
        .map_err(|e| TronError::KeystoreError(e.to_string()))?;

    let derived_raw_address = {
        // Get uncompressed public key (65 bytes), remove 0x04 prefix, keep 64 bytes
        let uncompressed = pubkey.serialize_uncompressed();
        let pubkey_hash_input = &uncompressed[1..65];

        let digest = keccak256(pubkey_hash_input);

        // Construct TRON address bytes: 0x41 + last 20 bytes of Keccak256 hash
        let mut raw = vec![0x41u8];
        raw.extend_from_slice(&digest[12..]);
        raw
    };
    let json_from_bytes = base58check_to_u8_slice(tx.from.clone())?;

    if derived_raw_address != json_from_bytes {
        return Err(TronError::NoMyInputs);
    }

    let (signed_hex, _) = tx.sign(seed)?;
    Ok(signed_hex)
}

pub fn parse_tx_request(json_bytes: &[u8], path: &String) -> errors::Result<ParsedTx> {
    let tx = WrappedTron::from_json_bytes(json_bytes, path.clone())?;
    let parsed_tx = tx.parse()?;
    Ok(parsed_tx)
}

pub fn check_tx_request(
    sign_data: &[u8],
    path: &str,
    master_fingerprint: bitcoin::bip32::Fingerprint,
    xpub: &str,
) -> Result<()> {
    let mut tx_data = WrappedTron::from_json_bytes(sign_data, path.to_string())?;
    tx_data.extended_pubkey = xpub.to_string();

    let extended_pubkey = bitcoin::bip32::Xpub::from_str(xpub)
        .map_err(|_| errors::TronError::InvalidParseContext(String::from("invalid xpub")))?;
    let context = keystone::ParseContext::new(master_fingerprint, extended_pubkey);
    tx_data.check(&context)
}

#[cfg(test)]
mod test {
    use super::*;
    use alloc::vec::Vec;
    use core::str::FromStr;
    use hex::FromHex;
    use ur_registry::pb::protobuf_parser::{parse_protobuf, unzip};
    use ur_registry::pb::protoc::{Base, Payload};
    pub fn prepare_parse_context(pubkey_str: &str) -> keystone::ParseContext {
        let master_fingerprint = bitcoin::bip32::Fingerprint::from_str("73c5da0a").unwrap();
        let extended_pubkey = bitcoin::bip32::Xpub::from_str(pubkey_str).unwrap();
        keystone::ParseContext::new(master_fingerprint, extended_pubkey)
    }

    pub fn prepare_payload(hex: &str) -> Payload {
        let bytes = Vec::from_hex(hex).unwrap();
        let unzip_data = unzip(bytes.clone()).unwrap();
        let base: Base = parse_protobuf(unzip_data).unwrap();
        base.data.unwrap()
    }

    #[test]
    fn test_sign_raw_tx() {
        let hex = "1f8b08000000000000030dcebb4ac3501c807153444a17b553e95482501142ce39f99f4b8b83362d74696cd38897eddca205db60ac22d95db49b4fe0e6e8e4e65bf80a6e3e80a083816ffb965fb552df9ce461666c6b9c67cb4c6757cd5fa75aa95739e283419f85eeb7535b4fe2a3a8bea3710a42748cc729621e60a43cc1c17801a2c03166444bd4dc1ef9006d1f7768db47653e6aad5e578f7f6877a3f7e234dd2499f2331adc4c212ff27eb83c97b3dbb8b8be3798c96568c971ecbac99416e32c84d3416f3ebceb47878bc509b91806a3c9a58e23368be77b6bc4c1dda8760046714cb148b154982949b400ad4a8f92960bc23ae5e20a510bda12c9d2006b0bd2d8148ca582045b0f9fcf8dc6cfd753291cbebdefb73ec6ff42b912d514010000";
        let pubkey_str = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let result = sign_raw_tx(payload, context, &seed);
        assert!(result.is_ok());
        let (tx_hex, tx_id) = result.unwrap();
        assert!(!tx_hex.is_empty());
        assert!(!tx_id.is_empty());
    }

    #[test]
    fn test_parse_raw_tx() {
        let hex = "1f8b08000000000000030dcebb4ac3501c807153444a17b553e95482501142ce39f99f4b8b83362d74696cd38897eddca205db60ac22d95db49b4fe0e6e8e4e65bf80a6e3e80a083816ffb965fb552df9ce461666c6b9c67cb4c6757cd5fa75aa95739e283419f85eeb7535b4fe2a3a8bea3710a42748cc729621e60a43cc1c17801a2c03166444bd4dc1ef9006d1f7768db47653e6aad5e578f7f6877a3f7e234dd2499f2331adc4c212ff27eb83c97b3dbb8b8be3798c96568c971ecbac99416e32c84d3416f3ebceb47878bc509b91806a3c9a58e23368be77b6bc4c1dda8760046714cb148b154982949b400ad4a8f92960bc23ae5e20a510bda12c9d2006b0bd2d8148ca582045b0f9fcf8dc6cfd753291cbebdefb73ec6ff42b912d514010000";
        let pubkey_str = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let result = parse_raw_tx(payload, context);
        assert!(result.is_ok());
        let parsed = result.unwrap();
        assert!(!parsed.overview.value.is_empty());
    }

    #[test]
    fn test_check_raw_tx() {
        // Use a valid test case from checker.rs
        let hex = "1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000";
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let result = check_raw_tx(payload, context);
        assert!(result.is_ok());
    }

    // Helper function: compute TRON address from seed and path, used for test data construction
    fn compute_address_from_seed(seed: &[u8], path: &String) -> String {
        let pubkey = secp256k1::get_public_key_by_seed(seed, path).unwrap();
        let uncompressed = pubkey.serialize_uncompressed();
        let hash = keccak256(&uncompressed[1..65]);
        let mut address_bytes = [0u8; 21];
        address_bytes[0] = 0x41;
        address_bytes[1..].copy_from_slice(&hash[12..]);
        bitcoin::base58::encode_check(&address_bytes)
    }

    #[test]
    fn test_sign_tx_request_success() {
        let path = "m/44'/195'/0'/0/0".to_string();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();

        let pubkey = secp256k1::get_public_key_by_seed(&seed, &path).unwrap();
        let uncompressed = pubkey.serialize_uncompressed();
        let digest = keccak256(&uncompressed[1..65]);
        let mut raw = vec![0x41u8];
        raw.extend_from_slice(&digest[12..]);
        let correct_address = compute_address_from_seed(&seed, &path);

        let json_str = format!(
            r#"{{
              "token": "TRX",
              "contract_address": "",
              "from": "{}",
              "to": "{}",
              "memo": "Test Transaction",
              "value": "1000000",
              "latest_block": {{
                "hash": "000000000001e240dec2860d5e1687299b8f269d09ceea82e7b96408dab58bd2",
                "number": 123456,
                "timestamp": 1670000000
              }},
              "override": {{
                "token_short_name": "TRX",
                "token_full_name": "tron",
                "decimals": 6
              }},
              "fee": 1000000
            }}"#,
            correct_address, correct_address
        );

        let result = sign_tx_request(json_str.as_bytes(), &path, &seed);
        if let Err(ref e) = result {
            std::println!("Sign Error Details: {:?}", e);
        }

        assert!(
            result.is_ok(),
            "Sign should succeed when address matches: {:?}",
            result.err()
        );
    }

    #[test]
    fn test_sign_tx_request_address_mismatch() {
        let path = "m/44'/195'/0'/0/0".to_string();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();

        let json_str = r#"{"to":"TKCsXtfKfH2d6aEaQCctybDC9uaA3MSj2h","from":"TWrongAddressHash12345678901234567890","value":"1000000"}"#;

        let result = sign_tx_request(json_str.as_bytes(), &path, &seed);

        assert!(result.is_err());
        assert!(matches!(result.unwrap_err(), TronError::NoMyInputs));
    }

    #[test]
    fn test_parse_tx_request() {
        let path = "m/44'/195'/0'/0/0".to_string();
        let json_str = r#"{
    "token": "TRX",
    "contract_address": "",
    "from": "TXhtYr8nmgiSp3dY3cSfiKBjed3zN8teHS",
    "to": "TKCsXtfKfH2d6aEaQCctybDC9uaA3MSj2h",
    "memo": "Test Transaction",
    "value": "1000000",
    "latest_block": {
        "hash": "000000000001e240dec2860d5e1687299b8f269d09ceea82e7b96408dab58bd2",
        "number": 123456,
        "timestamp": 1670000000
    },
    "override": {
        "token_short_name": "TRX",
        "token_full_name": "tron",
        "decimals": 6
    },
    "fee": 1000000
}"#;

        let result = parse_tx_request(json_str.as_bytes(), &path);
        match result {
            Ok(_) => (),
            Err(e) => panic!("Transaction parse Failed with error: {:?}", e),
        }
        assert!(result.is_ok());
        let parsed = result.unwrap();
        assert_eq!(parsed.overview.value, "1 TRX");
    }

    #[test]
    fn test_check_tx_request() {
        let path = "m/44'/195'/0'/0/0";
        let xpub = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
        let master_fingerprint = bitcoin::bip32::Fingerprint::from_str("73c5da0a").unwrap();

        let correct_address = get_address(path.to_string(), &xpub.to_string()).unwrap();

        let json_str = format!(
            r#"{{
            "token": "TRX",
            "contract_address": "",
            "xfp": "73c5da0a",
            "memo": "Test Transaction",
            "from": "{}",
            "to": "TKCsXtfKfH2d6aEaQCctybDC9uaA3MSj2h",
            "value": "1000000",
            "latest_block": {{
                "hash": "000000000001e240dec2860d5e1687299b8f269d09ceea82e7b96408dab58bd2",
                "number": 123456,
                "timestamp": 1670000000
              }},
              "override": {{
                "token_short_name": "TRX",
                "token_full_name": "tron",
                "decimals": 6
              }},
              "fee": 1000000
        }}"#,
            correct_address
        );

        let result = check_tx_request(json_str.as_bytes(), path, master_fingerprint, xpub);

        match result {
            Ok(_) => (),
            Err(e) => panic!("Transaction Check Failed with error: {:?}", e),
        }
    }
}
