#![no_std]
extern crate alloc;
extern crate core;
#[cfg(test)]
#[macro_use]
extern crate std;

use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::str::FromStr;

use bitcoin::bip32::{DerivationPath, Xpub};
use bitcoin::secp256k1;
use bitcoin::secp256k1::Message;

use serde_json::{from_slice, Value};

use crate::errors::{XRPError, R};
use crate::transaction::WrappedTxData;

pub mod address;
pub mod errors;
pub mod parser;
mod transaction;

pub fn get_tx_hash(raw_hex: &[u8]) -> R<String> {
    let v: Value = from_slice(raw_hex)?;
    let wrapped_tx = WrappedTxData::from_raw(v.to_string().into_bytes().as_slice())?;
    Ok(hex::encode(wrapped_tx.tx_hex))
}

pub fn sign_tx(raw_hex: &[u8], hd_path: &String, seed: &[u8]) -> R<Vec<u8>> {
    let v: Value = from_slice(raw_hex)?;
    let mut wrapped_tx = WrappedTxData::from_raw(v.to_string().into_bytes().as_slice())?;
    let message = Message::from_digest_slice(&wrapped_tx.tx_hex).map_err(|_e| {
        XRPError::SignFailure(format!(
            "invalid message to sign {:?}",
            hex::encode(wrapped_tx.tx_hex)
        ))
    })?;
    let (_, signature) =
        keystore::algorithms::secp256k1::sign_message_by_seed(seed, hd_path, &message)
            .map_err(|e| XRPError::KeystoreError(format!("sign failed {:?}", e.to_string())))?;
    let signed_tx_str = wrapped_tx.generate_signed_tx(&signature)?;
    Ok(hex::decode(signed_tx_str)?.to_vec())
}

pub fn parse(raw_hex: &[u8]) -> R<parser::structs::ParsedXrpTx> {
    let v: Value = from_slice(raw_hex)?;
    parser::structs::ParsedXrpTx::build(v)
}

pub fn parse_batch(raw_hex: &[u8], service_fee: &[u8]) -> R<parser::structs::ParsedXrpTx> {
    parser::structs::ParsedXrpTx::build_batch(&from_slice(raw_hex)?, &from_slice(service_fee)?)
}

pub fn get_pubkey_path(root_xpub: &str, pubkey: &str, max_i: u32) -> R<String> {
    let xpub = Xpub::from_str(root_xpub)?;
    let k1 = secp256k1::Secp256k1::new();
    let a_xpub = xpub.derive_pub(&k1, &DerivationPath::from_str("m/0")?)?;
    let pubkey_arr = hex::decode(pubkey)?;
    let pubkey_bytes = pubkey_arr.as_slice();
    for i in 0..max_i {
        let pk = a_xpub.derive_pub(&k1, &DerivationPath::from_str(&format!("m/{i}"))?)?;
        let key = pk.public_key.serialize();
        if key.eq(pubkey_bytes) {
            return Ok(format!("{pubkey}:m/0/{i}"));
        }
    }
    Err(XRPError::InvalidData("pubkey not found".to_string()))
}

pub fn check_tx(raw: &[u8], root_xpub: &str, cached_pubkey: &str) -> R<String> {
    let v: Value = from_slice(raw)?;
    let wrapped_tx = WrappedTxData::from_raw(v.to_string().into_bytes().as_slice())?;
    if wrapped_tx.signing_pubkey == cached_pubkey {
        return Ok("".to_string());
    }
    get_pubkey_path(root_xpub, &wrapped_tx.signing_pubkey, 200)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_xrp_sign() {
        let hd_paths = String::from("m/44'/144'/0'/0/0");
        let raw_hex = "7B2253657175656E6365223A312C22466565223A223230222C224163636F756E74223A22724C354259534C643839757A6A3469344A3437694C51673948776D65584537654374222C2244657374696E6174696F6E223A227248666F6631784E6245744A5973584E384D55626E66396946697843455938346B66222C2244657374696E6174696F6E546167223A313730303337333336342C22416D6F756E74223A2231303030303030222C225472616E73616374696F6E54797065223A225061796D656E74222C22466C616773223A323134373438333634382C225369676E696E675075624B6579223A22303331443638424331413134324536373636423242444642303036434346453133354546324530453245393441424235434635433941423631303437373646424145227D";
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let signed_tx = sign_tx(
            hex::decode(raw_hex).unwrap().as_slice(),
            &hd_paths,
            seed.as_slice(),
        )
        .unwrap();
        assert_eq!("120000228000000024000000012e6559a3746140000000000f42406840000000000000147321031d68bc1a142e6766b2bdfb006ccfe135ef2e0e2e94abb5cf5c9ab6104776fbae7446304402207c29d6746cde16e42a0e63c7f6815329cdc73a6ed50a2cc75b208acd066e553602200e0458183beb7f8ea34f0bd9477b672787f66942f231962f3c4a6055acba69f68114d8343e8e1f27b467b651748b8396a52c9185d9f98314b0cb0194b32f22136d1ff5a01e45fb2fed2c3f75", hex::encode(signed_tx));
    }

    #[test]
    fn test_get_pubkey_path() {
        let pubkey = "03F5C5BB1D19EC710D3D7FAD199AF10CF8BC1D11348E5B3765C0B0B9C0BEC32879";
        let root_xpub = "xpub6Czwh4mKUQryD6dbe9e9299Gjn4EnSP641rACmQeAhCXYjW4Hnj8tqiCMir3VSWoNnjimtzy6qtnjD1GSf8FtEdXUFcGeXXezXyEMXtMmo1";
        let result = get_pubkey_path(root_xpub, pubkey, 200).unwrap();
        assert_eq!(
            result,
            "03F5C5BB1D19EC710D3D7FAD199AF10CF8BC1D11348E5B3765C0B0B9C0BEC32879:m/0/0"
        );
    }

    #[test]
    fn test_check_tx() {
        let raw = hex::decode("7B225472616E73616374696F6E54797065223A225061796D656E74222C22416D6F756E74223A223130303030303030222C2244657374696E6174696F6E223A22724A6436416D48485A7250756852683377536637696B724D4A516639373646516462222C22466C616773223A323134373438333634382C224163636F756E74223A227247556D6B794C627671474633687758347177474864727A4C6459325170736B756D222C22466565223A223132222C2253657175656E6365223A34323532393130372C224C6173744C656467657253657175656E6365223A34323532393137332C225369676E696E675075624B6579223A22303346354335424231443139454337313044334437464144313939414631304346384243314431313334384535423337363543304230423943304245433332383739227D").unwrap();
        let root_xpub = "xpub6Czwh4mKUQryD6dbe9e9299Gjn4EnSP641rACmQeAhCXYjW4Hnj8tqiCMir3VSWoNnjimtzy6qtnjD1GSf8FtEdXUFcGeXXezXyEMXtMmo1";
        let result = check_tx(&raw, root_xpub, "");
        assert!(result.is_ok());
    }
}
