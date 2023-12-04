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
use keystore::algorithms::ed25519;
use stellar_xdr::curr::{Limits, PublicKey, TypeVariant, Uint256};
use third_party::{cryptoxide::hashing, hex};

pub mod errors;

pub fn generate_address(pub_key: &str) -> Result<String> {
    let buf: Vec<u8> = hex::decode(pub_key)?;
    let pk = PublicKey::PublicKeyTypeEd25519(Uint256(buf.as_slice().try_into()?));
    Ok(pk.to_string())
}

pub fn parse_tx(payload: &[u8]) -> Result<stellar_xdr::curr::Type> {
    let limits = Limits::none();
    let t = stellar_xdr::curr::Type::from_xdr(
        TypeVariant::TransactionSignaturePayload,
        payload,
        limits,
    )?;
    Ok(t)
}

pub fn sign_tx(seed: &[u8], path: &String, payload: &[u8]) -> Result<[u8; 64]> {
    let hash = hashing::sha256(payload);
    ed25519::slip10_ed25519::sign_message_by_seed(seed, path, &hash)
        .map_err(|e| errors::StellarError::SignFailure(e.to_string()))
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
    fn test_parse_tx() {
        let data = hex::decode("cee0302d59844d32bdca915c8203dd44b33fbb7edc19051ea37abedf28ecd4720000000200000000b4152f0e761e32152a5ab1e2b5b1830c55d4e9542266ca5189a4c798bbd2ce28000000c80001c7c60000009c00000001000000000000000000000000656d617d000000000000000200000000000000070000000011144aea7add6c85858be9dbc4d4a5f756037925941675926c69b11ebe7f1f8c00000001414243000000000100000000000000010000000011144aea7add6c85858be9dbc4d4a5f756037925941675926c69b11ebe7f1f8c0000000000000000d0b0998700000000").unwrap();
        let t = parse_tx(&data).unwrap();
        let tx_json = json!(t).to_string();
        assert_eq!(
            tx_json,
            "{\"network_id\":\"cee0302d59844d32bdca915c8203dd44b33fbb7edc19051ea37abedf28ecd472\",\"tagged_transaction\":{\"tx\":{\"cond\":{\"time\":{\"max_time\":1701667197,\"min_time\":0}},\"ext\":\"v0\",\"fee\":200,\"memo\":\"none\",\"operations\":[{\"body\":{\"allow_trust\":{\"asset\":{\"credit_alphanum4\":\"41424300\"},\"authorize\":1,\"trustor\":\"GAIRISXKPLOWZBMFRPU5XRGUUX3VMA3ZEWKBM5MSNRU3CHV6P4PYZ74D\"}},\"source_account\":null},{\"body\":{\"payment\":{\"amount\":3501234567,\"asset\":\"native\",\"destination\":\"GAIRISXKPLOWZBMFRPU5XRGUUX3VMA3ZEWKBM5MSNRU3CHV6P4PYZ74D\"}},\"source_account\":null}],\"seq_num\":501128194162844,\"source_account\":\"GC2BKLYOOYPDEFJKLKY6FNNRQMGFLVHJKQRGNSSRRGSMPGF32LHCQVGF\"}}}"
        );
    }

    #[test]
    fn test_sign_tx() {
        let seed = hex::decode("a33b2bdc2dc3c53d24081e5ed2273e6e8e0e43f8b26c746fbd1db2b8f1d4d8faa033545d3ec9303d36e743a4574b80b124353d380535532bb69455dc0ee442c4").unwrap();
        let path = "m/44'/148'/0'".to_string();
        let payload = hex::decode("cee0302d59844d32bdca915c8203dd44b33fbb7edc19051ea37abedf28ecd47200000002000000001996c8e39d8065e00f6c848a457e7d521204c617c7255fff6974831bd2294ccc00000064002b5b8b0000000100000001000000000000000000000000656d6f9a00000000000000010000000000000001000000001996c8e39d8065e00f6c848a457e7d521204c617c7255fff6974831bd2294ccc00000000000000000098968000000000").unwrap();
        let signature = sign_tx(&seed, &path, &payload).unwrap();
        assert_eq!(
            hex::encode(&signature),
            "e5fdbde7bf0bd81f93e31386a2de6793f1fb88ae91185a213c2ad22b2c8a2d00c4b252254e0a41024d6e5c859868411caa6a8f6c199f1525fe5f769d51b4b807"
        )
    }
}
