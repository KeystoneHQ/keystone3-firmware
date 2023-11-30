#![no_std]
#![feature(error_in_core)]

#[allow(unused_imports)]
#[macro_use]
extern crate alloc;
extern crate core;

#[cfg(test)]
#[macro_use]
extern crate std;

use crate::errors::Result;
use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use stellar_xdr::curr::{Limits, PublicKey, TypeVariant, Uint256};
use third_party::hex;

pub mod errors;

pub fn generate_address(pub_key: &str) -> Result<String> {
    let buf: Vec<u8> = hex::decode(pub_key)?;
    let pk = PublicKey::PublicKeyTypeEd25519(Uint256(buf.as_slice().try_into()?));
    Ok(pk.to_string())
}

pub fn parse_tx(data: Vec<u8>) -> Result<stellar_xdr::curr::Type> {
    let limits = Limits::none();
    let t = stellar_xdr::curr::Type::from_xdr(TypeVariant::TransactionEnvelope, data, limits)?;
    Ok(t)
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;
    use third_party::serde_json::json;

    #[test]
    fn test_generate_address() {
        let pub_key = "1996c8e39d8065e00f6c848a457e7d521204c617c7255fff6974831bd2294ccc";
        let address = generate_address(pub_key).unwrap();
        assert_eq!(
            "GAMZNSHDTWAGLYAPNSCIURL6PVJBEBGGC7DSKX77NF2IGG6SFFGMZIY7",
            address
        );
    }

    #[test]
    fn test_parse_envelope() {
        let data = hex::decode("0000000200000000b4152f0e761e32152a5ab1e2b5b1830c55d4e9542266ca5189a4c798bbd2ce28000000c80001c7c6000000860000000100000000000000000000000065601b8b000000000000000200000000000000070000000011144aea7add6c85858be9dbc4d4a5f756037925941675926c69b11ebe7f1f8c00000001414243000000000100000000000000010000000011144aea7add6c85858be9dbc4d4a5f756037925941675926c69b11ebe7f1f8c0000000000000000d0b099870000000000000000").unwrap();
        let t = parse_tx(data).unwrap();
        let tx_json = json!(t).to_string();
        assert_eq!(
            tx_json,
            "{\"tx\":{\"signatures\":[],\"tx\":{\"cond\":{\"time\":{\"max_time\":1700797323,\"min_time\":0}},\"ext\":\"v0\",\"fee\":200,\"memo\":\"none\",\"operations\":[{\"body\":{\"allow_trust\":{\"asset\":{\"credit_alphanum4\":\"41424300\"},\"authorize\":1,\"trustor\":\"GAIRISXKPLOWZBMFRPU5XRGUUX3VMA3ZEWKBM5MSNRU3CHV6P4PYZ74D\"}},\"source_account\":null},{\"body\":{\"payment\":{\"amount\":3501234567,\"asset\":\"native\",\"destination\":\"GAIRISXKPLOWZBMFRPU5XRGUUX3VMA3ZEWKBM5MSNRU3CHV6P4PYZ74D\"}},\"source_account\":null}],\"seq_num\":501128194162822,\"source_account\":\"GC2BKLYOOYPDEFJKLKY6FNNRQMGFLVHJKQRGNSSRRGSMPGF32LHCQVGF\"}}}"
        );
    }
}
