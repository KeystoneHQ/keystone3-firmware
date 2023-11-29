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
use serde_derive::{Deserialize, Serialize};
use stellar_xdr::curr::{TransactionEnvelope, Limits, TypeVariant, PublicKey, Uint256};
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
    let mut buf: Vec<u8> = hex::decode(pub_key)?;
    let pk = PublicKey::PublicKeyTypeEd25519(Uint256(buf.as_slice().try_into()?));
    Ok(pk.to_string())
}

pub fn parse_tx() -> Result<stellar_xdr::curr::Type> {
    let data = hex::decode("0000000200000000b4152f0e761e32152a5ab1e2b5b1830c55d4e9542266ca5189a4c798bbd2ce28000000c80001c7c6000000860000000100000000000000000000000065601b8b000000000000000200000000000000070000000011144aea7add6c85858be9dbc4d4a5f756037925941675926c69b11ebe7f1f8c00000001414243000000000100000000000000010000000011144aea7add6c85858be9dbc4d4a5f756037925941675926c69b11ebe7f1f8c0000000000000000d0b099870000000000000000").unwrap();
    let limits = Limits::none();
    let payload = stellar_xdr::curr::Type::from_xdr(TypeVariant::TransactionEnvelope, data, limits);
    let t = stellar_xdr::curr::MessageType::Auth;
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
        println!("---------------json: {:?}", json!(payload).to_string());
        assert_eq!(
            "PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJUAAAAAQACAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUPB6IBZGM",
            ""
        );
    }

}
