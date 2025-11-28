#![no_std]
extern crate alloc;
extern crate core;

#[cfg(test)]
#[macro_use]
extern crate std;

use crate::errors::Result;
use alloc::string::String;

use ur_registry::pb::protoc;

mod address;
pub mod errors;
mod pb;
mod transaction;
mod utils;

pub use crate::address::get_address;
pub use crate::transaction::parser::{DetailTx, OverviewTx, ParsedTx, TxParser};
use crate::transaction::wrapped_tron::WrappedTron;
use app_utils::keystone;
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
}
