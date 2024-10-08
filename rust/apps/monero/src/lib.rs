#![no_std]
#![feature(error_in_core)]
extern crate alloc;

use third_party::cryptoxide::sha3::Keccak256;
use alloc::string::ToString;
use alloc::format;
use third_party::hex;
use keystore::algorithms::crypto::hmac_sha512;
use third_party::ed25519_bip32_core::{XPrv, DerivationScheme, XPub, XPRV_SIZE};
use keystore::algorithms::ed25519::slip10_ed25519::{get_private_key_by_seed, get_public_key_by_seed};
use keystore::algorithms::ed25519::bip32_ed25519;
use third_party::cryptoxide::hashing::sha2::Sha512;
use third_party::bitcoin::{script, Network, PrivateKey, Script, base58};
use third_party::cryptoxide::digest::Digest;
use alloc::vec::Vec;
use curve25519_dalek::edwards::{CompressedEdwardsY, EdwardsPoint};
use curve25519_dalek::scalar::Scalar;
use base58_monero::{encode, decode, Error};

fn keccak256(data: &[u8]) -> [u8; 32] {
    let mut hasher = Keccak256::new();
    hasher.input(data);
    let mut result = [0u8; 32];
    hasher.result(&mut result);
    result
}

fn calc_subaddress_m(secret_view_key: &[u8], major: u32, minor: u32) -> [u8; 32] {
    let prefix = "SubAddr".as_bytes().to_vec();
    let mut data = prefix.clone();
    data.push(0);
    data.extend_from_slice(secret_view_key);
    data.extend_from_slice(&major.to_le_bytes());
    data.extend_from_slice(&minor.to_le_bytes());
    hash_to_scalar(&data)
}

fn hash_to_scalar(data: &[u8]) -> [u8; 32] {
    let mut hasher = Keccak256::new();
    hasher.input(data);
    let mut result = [0u8; 32];
    hasher.result(&mut result);
    let scalar = Scalar::from_bytes_mod_order(result);
    scalar.to_bytes()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_monero_add_generator() {
        // BIP39 Mnemonic: key stone key stone key stone key stone key stone key stone key stone success
        let seed = hex::decode("45a5056acbe881d7a5f2996558b303e08b4ad1daffacf6ffb757ff2a9705e6b9f806cffe3bd90ff8e3f8e8b629d9af78bcd2ed23e8c711f238308e65b62aa5f0").unwrap();
        let path = "m/44'/128'/0'/0/0".to_string();

        // raw_private_key
        let key = keystore::algorithms::secp256k1::get_private_key_by_seed(
            &seed,
            &path.to_string(),
        ).unwrap();
        let raw_private_key = PrivateKey::new(key, Network::Bitcoin);

        // raw_secret_spend_key
        let mut raw_secret_spend_key = keccak256(&raw_private_key.to_bytes());

        // secret_spend_key
        let secret_spend_key = Scalar::from_bytes_mod_order(raw_secret_spend_key).to_bytes();

        // secret_view_key
        let mut secret_view_key = hash_to_scalar(&secret_spend_key);

        // publicSpendKey
        let scalar = Scalar::from_bytes_mod_order(secret_spend_key);
        let public_spend_key: CompressedEdwardsY = EdwardsPoint::mul_base(&scalar).compress();

        // publicViewKey
        let scalar = Scalar::from_bytes_mod_order(secret_view_key);
        let public_view_key: CompressedEdwardsY = EdwardsPoint::mul_base(&scalar).compress();

        // address MONERO_MAINNET
        let prefix = "12";
        let res_hex = format!("{}{}{}", prefix, hex::encode(public_spend_key.as_bytes()), hex::encode(public_view_key.as_bytes()));
        let mut hasher = Keccak256::new();
        hasher.input(&hex::decode(res_hex.clone()).unwrap());
        let mut checksum = [0u8; 32];
        hasher.result(&mut checksum);
        let res_hex = format!("{}{}", res_hex, hex::encode(&checksum[0..4]));
        let address = encode(&hex::decode(res_hex).unwrap()).unwrap();

        // result => primary address
        assert_eq!(hex::encode(raw_private_key.to_bytes()), "66ec3ba491849c927c9be0bd8387b0a7215c61c69854d53f6585630d4557e752");
        assert_eq!(hex::encode(raw_secret_spend_key), "62cf06d75043c5bedb51d3070297b164e4d8ded84f0a3d65a542915475ca6fee");
        assert_eq!(hex::encode(secret_spend_key), "6c3895c1dfd7c3ed22be481ed5ec7f40e3d8ded84f0a3d65a542915475ca6f0e");
        assert_eq!(hex::encode(secret_view_key), "17921dbd51b4a1af0b4049bc13dc7048ace1dcd8be9b8669de95b8430924ea09");
        assert_eq!(hex::encode(public_spend_key.as_bytes()), "12f38162635cf3aecf081d96158022b2a1517993100e54d62b17057f2443e749");
        assert_eq!(hex::encode(public_view_key.as_bytes()), "e18a5360ae4b2ff71bf91c5a626e14fc2395608375b750526bc0962ed27237a1");
        assert_eq!(address, "42LmACF1Ce6WEs5w1nNsoPWswJQzcRdZucphf75q1bzvDMjq1vJ2iJziLGdTn1JbcPjB4iiEagCMuEnazTfaryuQKG7sw7S");
    }

    #[test]
    fn test_monero_subadd_generator() {
        // BIP39 Mnemonic: key stone key stone key stone key stone key stone key stone key stone success
        let seed = hex::decode("45a5056acbe881d7a5f2996558b303e08b4ad1daffacf6ffb757ff2a9705e6b9f806cffe3bd90ff8e3f8e8b629d9af78bcd2ed23e8c711f238308e65b62aa5f0").unwrap();
        let path = "m/44'/128'/0'/0/0".to_string();
        let key = keystore::algorithms::secp256k1::get_private_key_by_seed(
            &seed,
            &path.to_string(),
        ).unwrap();
        let raw_private_key = PrivateKey::new(key, Network::Bitcoin);
        let mut raw_secret_spend_key = keccak256(&raw_private_key.to_bytes());
        let secret_spend_key = Scalar::from_bytes_mod_order(raw_secret_spend_key).to_bytes();
        let mut secret_view_key = hash_to_scalar(&secret_spend_key.clone());

        // m
        let mut m = calc_subaddress_m(&secret_view_key, 0, 1);

        // publicSpendKey
        let scalar = Scalar::from_bytes_mod_order(secret_spend_key);
        let public_spend_key: CompressedEdwardsY = EdwardsPoint::mul_base(&scalar).compress();

        // subaddress secretSpendKey
        let subaddress_secret_spend_key = Scalar::from_bytes_mod_order(secret_spend_key) + Scalar::from_bytes_mod_order(m);

        // subaddress publicSpendKey
        let scalar = Scalar::from_bytes_mod_order(subaddress_secret_spend_key.to_bytes());
        let subaddress_public_spend_key: CompressedEdwardsY = EdwardsPoint::mul_base(&scalar).compress();

        // subaddress publicViewKey
        let scalar = Scalar::from_bytes_mod_order(secret_view_key);
        let point = subaddress_public_spend_key.decompress().unwrap();
        let subaddress_public_view_key = (point * scalar).compress();

        // subaddress
        let prefix = "2A";
        let res_hex = format!("{}{}{}", prefix, hex::encode(subaddress_public_spend_key.as_bytes()), hex::encode(subaddress_public_view_key.as_bytes()));
        let mut hasher = Keccak256::new();
        hasher.input(&hex::decode(res_hex.clone()).unwrap());
        let mut checksum = [0u8; 32];
        hasher.result(&mut checksum);
        let res_hex = format!("{}{}", res_hex, hex::encode(&checksum[0..4]));
        let address = encode(&hex::decode(res_hex).unwrap()).unwrap();

        // result => subaddress
        assert_eq!(hex::encode(m), "426543494cfc94803177f4ccffaee54275d9accb3f54a2caafa753ff62e8b400");
        assert_eq!(hex::encode(subaddress_secret_spend_key.as_bytes()), "ae9dd80a2cd4586e54353debd49b658358b28ba48f5edf2f55eae453d8b2240f");
        assert_eq!(hex::encode(subaddress_public_spend_key.as_bytes()), "3dca752621e394b068c3bde78951d029778d822aee481a2b08dc21589a3c6693");
        assert_eq!(hex::encode(subaddress_public_view_key.as_bytes()), "33f3f7b3628e0587f23abec549a071fb420783de74858a1fba0d9e49f3c193f7");
        assert_eq!(address, "84o4iSLUprPWWPeu4ZZPFm7wHMDkwCm9b8CVQ4YUko9PRd453PvhZ8YPjrDRJ4VPrGj2Wxx7KJgFT6JnnbEfapZGUvPSFuM");
    }
}