#![no_std]
#![feature(error_in_core)]
extern crate alloc;
extern crate core;

#[cfg(test)]
#[macro_use]
extern crate std;

pub mod address;
pub mod errors;
pub mod parser;
mod transaction;

use crate::errors::{XRPError, R};
use crate::transaction::WrappedTxData;
use alloc::format;
use alloc::string::ToString;
use alloc::vec::Vec;
use third_party::bitcoin::bip32::DerivationPath;
use third_party::hex;
use third_party::secp256k1::Message;
use third_party::serde_json::{from_slice, Value};

pub fn sign_tx(raw_hex: &[u8], hd_paths: Vec<DerivationPath>, seed: &[u8]) -> R<Vec<u8>> {
    let v: Value = from_slice(raw_hex)?;
    let mut wrapped_tx = WrappedTxData::from_raw(v.to_string().into_bytes().as_slice())?;
    let message = Message::from_slice(&wrapped_tx.tx_hex).map_err(|_e| {
        XRPError::SignFailure(format!(
            "invalid message to sign {:?}",
            hex::encode(wrapped_tx.tx_hex)
        ))
    })?;
    let hd_path = wrapped_tx.detect_hd_path(seed, hd_paths)?;
    let (_, signature) = keystore::algorithms::secp256k1::sign_message_by_seed(
        &seed,
        &hd_path.to_string(),
        &message,
    )
    .map_err(|e| XRPError::KeystoreError(format!("sign failed {:?}", e.to_string())))?;
    let signed_tx_str = wrapped_tx.generate_signed_tx(&signature)?;
    Ok(hex::decode(signed_tx_str)?.to_vec())
}

pub fn parse(raw_hex: &[u8]) -> R<parser::structs::ParsedXrpTx> {
    let v: Value = from_slice(raw_hex)?;
    let wrapped_tx = WrappedTxData::from_raw(v.to_string().into_bytes().as_slice())?;
    parser::structs::ParsedXrpTx::build(wrapped_tx.tx_data)
}

#[cfg(test)]
mod tests {
    use super::*;
    use core::str::FromStr;

    #[test]
    fn test_xrp_sign() {
        let hd_paths = vec![DerivationPath::from_str("m/44'/144'/0'/0/0").unwrap()];
        let raw_hex = "7B2253657175656E6365223A312C22466565223A223230222C224163636F756E74223A22724C354259534C643839757A6A3469344A3437694C51673948776D65584537654374222C2244657374696E6174696F6E223A227248666F6631784E6245744A5973584E384D55626E66396946697843455938346B66222C2244657374696E6174696F6E546167223A313730303337333336342C22416D6F756E74223A2231303030303030222C225472616E73616374696F6E54797065223A225061796D656E74222C22466C616773223A323134373438333634382C225369676E696E675075624B6579223A22303331443638424331413134324536373636423242444642303036434346453133354546324530453245393441424235434635433941423631303437373646424145227D";
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let signed_tx = sign_tx(
            &hex::decode(raw_hex).unwrap().as_slice(),
            hd_paths,
            seed.as_slice(),
        )
        .unwrap();
        assert_eq!("120000228000000024000000012e6559a3746140000000000f42406840000000000000147321031d68bc1a142e6766b2bdfb006ccfe135ef2e0e2e94abb5cf5c9ab6104776fbae7446304402207c29d6746cde16e42a0e63c7f6815329cdc73a6ed50a2cc75b208acd066e553602200e0458183beb7f8ea34f0bd9477b672787f66942f231962f3c4a6055acba69f68114d8343e8e1f27b467b651748b8396a52c9185d9f98314b0cb0194b32f22136d1ff5a01e45fb2fed2c3f75", hex::encode(signed_tx));
    }
}
