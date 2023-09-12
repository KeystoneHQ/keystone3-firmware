#![no_std]
#![feature(error_in_core)]

#[allow(unused_imports)] //report unused_import in test
#[macro_use]
extern crate alloc;
extern crate core;

#[cfg(test)]
#[macro_use]
extern crate std;

use crate::errors::{CosmosError, Result};
use crate::transaction::structs::{ParsedCosmosTx, SignMode};
use crate::utils::{hash160, keccak256, sha256_digest};
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use keystore::algorithms::secp256k1::derive_public_key;
use third_party::bitcoin::bech32;
use third_party::bitcoin::bech32::{ToBase32, Variant};
use third_party::secp256k1::{Message, PublicKey};

mod cosmos_sdk_proto;
pub mod errors;
mod proto_wrapper;
pub mod transaction;
pub mod utils;

fn generate_evmos_address(key: PublicKey, prefix: &str) -> Result<String> {
    let keccak: [u8; 32] = keccak256(&key.serialize_uncompressed()[1..]);
    let keccak160: [u8; 20] = keccak[keccak.len() - 20..keccak.len()]
        .try_into()
        .map_err(|_e| CosmosError::InvalidAddressError("keccak160 failed failed".to_string()))?;
    let address = bech32::encode(prefix, &keccak160.to_base32(), Variant::Bech32)?;
    Ok(address)
}

fn generate_general_address(key: PublicKey, prefix: &str) -> Result<String> {
    let hash160: [u8; 20] = hash160(&key.serialize());
    let address = bech32::encode(prefix, hash160.to_base32(), Variant::Bech32)?;
    Ok(address)
}

fn generate_address(key: PublicKey, prefix: &str) -> Result<String> {
    if prefix.to_lowercase().eq("evmos") || prefix.to_lowercase().eq("inj") {
        return generate_evmos_address(key, prefix);
    }
    return generate_general_address(key, prefix);
}

// pub fn parse_raw_tx(raw_tx: &Vec<u8>) -> Result<String> {
//     SignDoc::parse(raw_tx).map(|doc| {
//         serde_json::to_string(&doc).map_err(|err| CosmosError::ParseTxError(err.to_string()))
//     })?
// }

pub fn parse(
    raw_tx: &Vec<u8>,
    data_type: transaction::structs::DataType,
) -> Result<ParsedCosmosTx> {
    ParsedCosmosTx::build(raw_tx, data_type)
}

pub fn sign_tx(
    message: Vec<u8>,
    path: &String,
    sign_mode: SignMode,
    seed: &[u8],
) -> Result<[u8; 64]> {
    let hash = match sign_mode {
        SignMode::COSMOS => sha256_digest(message.as_slice()),
        SignMode::EVM => keccak256(message.as_slice()).to_vec(),
    };

    if let Ok(message) = Message::from_slice(&hash) {
        let (_, signature) = keystore::algorithms::secp256k1::sign_message_by_seed(
            &seed, path, &message,
        )
        .map_err(|e| CosmosError::KeystoreError(format!("sign failed {:?}", e.to_string())))?;
        return Ok(signature);
    }
    Err(CosmosError::SignFailure("invalid message".to_string()))
}

pub fn derive_address(
    hd_path: &str,
    root_x_pub: &str,
    root_path: &str,
    prefix: &str,
) -> Result<String> {
    let root_path = if !root_path.ends_with("/") {
        root_path.to_string() + "/"
    } else {
        root_path.to_string()
    };
    let sub_path = hd_path
        .strip_prefix(&root_path)
        .ok_or(CosmosError::InvalidHDPath(hd_path.to_string()))?;
    derive_public_key(&root_x_pub.to_string(), &format!("m/{}", sub_path))
        .map(|public_key| generate_address(public_key, prefix))
        .map_err(|e| CosmosError::from(e))?
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_derive_address() {
        {
            //general address ATOM-0
            let root_path = "44'/118'/0'";
            let root_xpub = "xpub6DGzViq8bmgMLYdVZ3xnLVEdKwzBnGdzzJZ4suG8kVb9TTLAbrwv8YdKBb8FWKdBNinaHKmBv7JpQvqBYx4rxch7WnHzNFzSVrMf8hQepTP";
            let hd_path = "44'/118'/0'/0/0";
            let address = derive_address(hd_path, root_xpub, root_path, "cosmos").unwrap();
            assert_eq!("cosmos19rl4cm2hmr8afy4kldpxz3fka4jguq0auqdal4", address);
        }
        {
            //general address OSMO-0
            let root_path = "44'/118'/0'";
            let root_xpub = "xpub6DGzViq8bmgMLYdVZ3xnLVEdKwzBnGdzzJZ4suG8kVb9TTLAbrwv8YdKBb8FWKdBNinaHKmBv7JpQvqBYx4rxch7WnHzNFzSVrMf8hQepTP";
            let hd_path = "44'/118'/0'/0/0";
            let address = derive_address(hd_path, root_xpub, root_path, "osmo").unwrap();
            assert_eq!("osmo19rl4cm2hmr8afy4kldpxz3fka4jguq0a5m7df8", address);
        }
        {
            //general address ROWAN-0
            let root_path = "44'/118'/0'";
            let root_xpub = "xpub6DGzViq8bmgMLYdVZ3xnLVEdKwzBnGdzzJZ4suG8kVb9TTLAbrwv8YdKBb8FWKdBNinaHKmBv7JpQvqBYx4rxch7WnHzNFzSVrMf8hQepTP";
            let hd_path = "44'/118'/0'/0/0";
            let address = derive_address(hd_path, root_xpub, root_path, "sif").unwrap();
            assert_eq!("sif19rl4cm2hmr8afy4kldpxz3fka4jguq0aeazts7", address);
        }
        {
            //general address CTK-0
            let root_path = "44'/118'/0'";
            let root_xpub = "xpub6DGzViq8bmgMLYdVZ3xnLVEdKwzBnGdzzJZ4suG8kVb9TTLAbrwv8YdKBb8FWKdBNinaHKmBv7JpQvqBYx4rxch7WnHzNFzSVrMf8hQepTP";
            let hd_path = "44'/118'/0'/0/0";
            let address = derive_address(hd_path, root_xpub, root_path, "certik").unwrap();
            assert_eq!("certik19rl4cm2hmr8afy4kldpxz3fka4jguq0amg3277", address);
        }
        {
            //general address CRO-0
            let root_path = "44'/394'/0'";
            let root_xpub = "xpub6CWZrBZhmNj2Gy3kWq6ZRagV68MJ89PqE9Y4m8jLP7Wvc2KRrZ83n7Y1rokGEHz8qj7xiRN7x5m6b5uAhCghuNd74Qbe5HrrWw1Dsb9NTUQ";
            let hd_path = "44'/394'/0'/0/0";
            let address = derive_address(hd_path, root_xpub, root_path, "cro").unwrap();
            assert_eq!("cro1r3ywhs4ng96dnm9zkc5y3etl7tps5cvvz26lr4", address);
        }
        {
            //general address IOV-0
            let root_path = "44'/234'/0'";
            let root_xpub = "xpub6BxV2Rig2nzNdhzPZvLMfedG9UpBvoS6XoSCZPVeyojFhasgE9Hwhq56HxVoYQMr8W2Pcrzn7FJ7pLqXXDq5eG5NKoCzxWAsmt2P5KfXgfj";
            let hd_path = "44'/234'/0'/0/0";
            let address = derive_address(hd_path, root_xpub, root_path, "star").unwrap();
            assert_eq!("star19nwv008krnl9w56ayu9yztq2265fq9q7lsyuzh", address);
        }
        {
            //general address AKT-0
            let root_path = "44'/118'/0'";
            let root_xpub = "xpub6DGzViq8bmgMLYdVZ3xnLVEdKwzBnGdzzJZ4suG8kVb9TTLAbrwv8YdKBb8FWKdBNinaHKmBv7JpQvqBYx4rxch7WnHzNFzSVrMf8hQepTP";
            let hd_path = "44'/118'/0'/0/0";
            let address = derive_address(hd_path, root_xpub, root_path, "akash").unwrap();
            assert_eq!("akash19rl4cm2hmr8afy4kldpxz3fka4jguq0a3mq6x0", address);
        }
        {
            //general address SCRT-0
            let root_path = "44'/529'/0'";
            let root_xpub = "xpub6CHxzGiCX6ZdYkQEu8Ys2znp2PDdQA6Q9AxB6ty1YP7Y5mzxaqBqg9fJu1UySQBiPx1CE7niLrXi9ZSVcLZFxZJvVjnVadEJkX5JGToEVXa";
            let hd_path = "44'/529'/0'/0/0";
            let address = derive_address(hd_path, root_xpub, root_path, "secret").unwrap();
            assert_eq!("secret1gkle2qetd47g4qlruxu8kx4m97875t66qsgr0p", address);
        }
        {
            //general address BLD-0
            let root_path = "44'/564'/0'";
            let root_xpub = "xpub6Cy2KXVssRUr6QK68mXztCSpmxK7yd68tCLHniaMVnRBxU8XYoFBSjeZ8hfg3VJDPAh3i8z1uhfpxPMy7Ub3FxyKzWu9RJdcmEkz6PL2cLu";
            let hd_path = "44'/564'/0'/0/0";
            let address = derive_address(hd_path, root_xpub, root_path, "agoric").unwrap();
            assert_eq!("agoric1r0q3ltgz67ldu86l9c6hvmwq5qke3af5h489vm", address);
        }
        {
            //general address EVMOS-0
            let root_path = "44'/60'/0'";
            let root_xpub = "xpub6DCoCpSuQZB2jawqnGMEPS63ePKWkwWPH4TU45Q7LPXWuNd8TMtVxRrgjtEshuqpK3mdhaWHPFsBngh5GFZaM6si3yZdUsT8ddYM3PwnATt";
            let hd_path = "44'/60'/0'/0/0";
            let address = derive_address(hd_path, root_xpub, root_path, "evmos").unwrap();
            assert_eq!("evmos1npvwllfr9dqr8erajqqr6s0vxnk2ak55t3r99j", address);
        }
    }
}
