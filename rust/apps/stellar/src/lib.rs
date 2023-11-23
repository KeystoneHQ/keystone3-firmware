#![no_std]
#![feature(error_in_core)]

#[allow(unused_imports)]
#[macro_use]
extern crate alloc;
extern crate core;

#[cfg(test)]
#[macro_use]
extern crate std;

use core::str::FromStr;

use crate::{errors::Result, types::intent::IntentScope};
use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use errors::SuiError;
use serde_derive::{Deserialize, Serialize};
use third_party::{bcs, hex};
use third_party::{
    blake2::{
        digest::{Update, VariableOutput},
        Blake2bVar,
    },
    serde_json,
};
use types::{intent::IntentMessage, msg::PersonalMessageUtf8};

pub mod errors;
pub mod types;

pub fn generate_address(pub_key: &str) -> Result<String> {
    let mut hasher = Blake2bVar::new(32).unwrap();
    let mut buf: Vec<u8> = hex::decode(pub_key)?;
    // insert flag, ed25519 is 0, secp256k1 is 1, secp256r1 is 2, multi sign is 3.
    buf.insert(0, 0);
    hasher.update(&buf);
    let mut addr = [0u8; 32];
    hasher.finalize_variable(&mut addr).unwrap();
    Ok(format!("0x{}", hex::encode(addr)))
}

pub fn parse_tx() -> Result<stellar_xdr::curr::SignerKeyEd25519SignedPayload> {
    let payload = stellar_xdr::curr::SignerKeyEd25519SignedPayload::from_str("PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAQACAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUPB6IBZGM");
    Ok(payload.unwrap())
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;
    use third_party::serde_json::json;

    #[test]
    fn test_generate_address() {
        let pub_key = "edbe1b9b3b040ff88fbfa4ccda6f5f8d404ae7ffe35f9b220dec08679d5c336f";
        let address = generate_address(pub_key).unwrap();
        assert_eq!(
            "0x504886c9ec43bff70af37f55865094cc3a799cb54479f252d30cd3717f15ecdc",
            address
        );
    }

    #[test]
    fn test_parse_tx() {
        let payload = parse_tx().unwrap();
        println!("{:?}", payload);
        assert_eq!(
            "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAQACAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUPB6IBZGM",
            payload.to_string()
        );
    }

}
