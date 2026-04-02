#![no_std]
#![cfg_attr(coverage_nightly, feature(coverage_attribute))]

#[allow(unused_imports)] //report unused_import in test
#[macro_use]
extern crate alloc;
extern crate core;
#[cfg(test)]
#[macro_use]
extern crate std;

use alloc::string::{String, ToString};

use crate::errors::{CosmosError, Result};
use crate::transaction::structs::{ParsedCosmosTx, SignMode};
use crate::utils::{hash160, keccak256, sha256_digest};
use bech32::{Bech32, Hrp};
use bitcoin::secp256k1::{Message, PublicKey};

use keystore::algorithms::secp256k1::derive_public_key;

#[cfg_attr(coverage_nightly, coverage(off))]
#[allow(warnings)]
mod cosmos_sdk_proto;
pub mod errors;
mod proto_wrapper;
pub mod transaction;
pub mod utils;

fn generate_evmos_address(key: PublicKey, prefix: &str) -> Result<String> {
    let keccak: [u8; 32] = keccak256(&key.serialize_uncompressed()[1..]);
    let keccak160: [u8; 20] = keccak[keccak.len() - 20..keccak.len()]
        .try_into()
        .map_err(|_e| {
            CosmosError::InvalidAddressError("keccak160 conversion failed".to_string())
        })?;
    let hrp = Hrp::parse_unchecked(prefix);
    let address = bech32::encode::<Bech32>(hrp, &keccak160)?;
    Ok(address)
}

fn generate_general_address(key: PublicKey, prefix: &str) -> Result<String> {
    let hash160: [u8; 20] = hash160(&key.serialize());
    let hrp = Hrp::parse_unchecked(prefix);
    let address = bech32::encode::<Bech32>(hrp, &hash160)?;
    Ok(address)
}

fn generate_address(key: PublicKey, prefix: &str) -> Result<String> {
    match prefix.to_lowercase().as_str() {
        "evmos" | "inj" | "dym" => generate_evmos_address(key, prefix),
        _ => generate_general_address(key, prefix),
    }
}

pub fn parse(raw_tx: &[u8], data_type: transaction::structs::DataType) -> Result<ParsedCosmosTx> {
    ParsedCosmosTx::build(raw_tx, data_type)
}

pub fn sign_tx(
    message: &[u8],
    path: &String,
    sign_mode: SignMode,
    seed: &[u8],
) -> Result<[u8; 64]> {
    let hash = match sign_mode {
        SignMode::COSMOS => sha256_digest(message),
        SignMode::EVM => keccak256(message).to_vec(),
    };

    if let Ok(message) = Message::from_digest_slice(hash.as_slice()) {
        let (_, signature) =
            keystore::algorithms::secp256k1::sign_message_by_seed(seed, path, &message)
                .map_err(|e| CosmosError::KeystoreError(format!("sign failed {e}")))?;
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
    let root_path = if !root_path.ends_with('/') {
        format!("{root_path}/")
    } else {
        root_path.to_string()
    };
    let sub_path = hd_path
        .strip_prefix(&root_path)
        .ok_or(CosmosError::InvalidHDPath(hd_path.to_string()))?;
    derive_public_key(&root_x_pub.to_string(), &format!("m/{sub_path}"))
        .map(|public_key| generate_address(public_key, prefix))
        .map_err(CosmosError::from)?
}

#[cfg(test)]
mod tests {

    use super::*;

    #[test]
    fn test_parse_tx() {
        // transfer rune old case
        let raw_tx = "0a570a500a0e2f74797065732e4d736753656e64123e0a14d2a392d2d0e98f64dd0f9aa422da9b37b21944501214ead5b280c71c6ae156ee581cff8c9147a64cca1f1a100a0472756e65120831303030303030301203626d79125a0a500a460a1f2f636f736d6f732e63727970746f2e736563703235366b312e5075624b657912230a2103b1f26f209231b1ee90a8c52a981038794195fd7e08c47df02ff8bb7c1ce2a43512040a020801180012061080cab5ee011a1474686f72636861696e2d6d61696e6e65742d763120bdb106";
        let data_type = transaction::structs::DataType::Direct;
        let parsed_tx = parse(&hex::decode(raw_tx).unwrap(), data_type).unwrap();

        let overview = parsed_tx.overview;
        let network = overview.common.network;
        assert_ne!("THORChain", network.as_str(),)
    }

    #[test]
    fn test_derive_thorchain_address_by_seed() {
        let seed = [
            150, 6, 60, 69, 19, 44, 132, 15, 126, 22, 101, 163, 185, 120, 20, 216, 235, 37, 134,
            243, 75, 217, 69, 240, 111, 161, 91, 147, 39, 238, 190, 53, 95, 101, 78, 129, 198, 35,
            58, 82, 20, 157, 122, 149, 234, 116, 134, 235, 141, 105, 145, 102, 245, 103, 126, 80,
            117, 41, 72, 37, 153, 98, 76, 220,
        ];
        let path = "M/44'/931'/0'/0/0";
        let pub_key =
            keystore::algorithms::secp256k1::get_public_key_by_seed(&seed, &path.to_string())
                .unwrap();
        let address = generate_address(pub_key, "thor").unwrap();
        assert_eq!("thor14vc9484wvt66f7upncl7hq8kcvdd7qm80ld002", address);
    }

    #[test]
    fn test_derive_thorchain_address_by_xpub() {
        {
            let root_path = "44'/931'/0'";
            let root_xpub = "xpub6CexGUAW8CXpTAZ19JxEGRxt2g4W7YNc3XSopBxw27jjBWDF67KShM7JqUibfQpHTsjzBdEwAw9X7QsBTVxjRpgK3bUbhS4e3y6kVhUfkek";
            let hd_path = "44'/931'/0'/0/0";
            let address = derive_address(hd_path, root_xpub, root_path, "thor").unwrap();
            assert_eq!("thor14vc9484wvt66f7upncl7hq8kcvdd7qm80ld002", address);
        }
    }

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
            let address = derive_address(hd_path, root_xpub, root_path, "shentu").unwrap();
            assert_eq!("shentu19rl4cm2hmr8afy4kldpxz3fka4jguq0a55fydg", address);
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
