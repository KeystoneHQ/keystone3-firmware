use alloc::vec;
use alloc::vec::Vec;
use alloc::string::String;
use curve25519_dalek::scalar::Scalar;
use crate::key::PublicKey;
use chacha20::{ChaCha20Legacy, ChaCha20LegacyCore, ChaChaCore};
use chacha20::cipher::{KeyIvInit, StreamCipher, StreamCipherSeek, generic_array::GenericArray};
use cuprate_cryptonight::cryptonight_hash_v0;
use third_party::hex;
use third_party::cryptoxide::digest::Digest;
use third_party::cryptoxide::sha3::Keccak256;
use third_party::cryptoxide::hashing;
use third_party::cryptoxide::ripemd160::Ripemd160;

pub const OUTPUT_EXPORT_MAGIC: &str = "Monero output export\x04";
pub const KEY_IMAGE_EXPORT_MAGIC: &str = "Monero key image export\x03";
pub const PUBKEY_LEH: usize = 32;

pub fn decrypt_data_with_pvk(
    pvk: [u8; PUBKEY_LEH],
    data: Vec<u8>,
    magic: &str,
) -> (PublicKey, PublicKey, Vec<u8>) {
    let pvk_hash = cryptonight_hash_v0(&pvk);
    let key = GenericArray::from_slice(&pvk_hash);

    let magic_bytes = magic.as_bytes();

    let mut data = data.clone();
    let mut magic_bytes_found = true;
    for i in 0..magic_bytes.len() {
        if data[i] != magic_bytes[i] {
            magic_bytes_found = false;
            break;
        }
    }

    if magic_bytes_found {
        data = data[magic_bytes.len()..].to_vec();
    }

    let nonce = GenericArray::from_slice(&data[0..8]);

    let data = data[8..].to_vec();

    let mut cipher = ChaCha20Legacy::new(key, nonce);

    let mut buffer = data.clone();

    cipher.apply_keystream(&mut buffer);

    let start = match magic {
        OUTPUT_EXPORT_MAGIC => 0,
        KEY_IMAGE_EXPORT_MAGIC => 4,
        _ => 0,
    };

    let pk1 = PublicKey::from_bytes(&buffer[start..start + PUBKEY_LEH]).unwrap();
    let pk2 = PublicKey::from_bytes(&buffer[start + PUBKEY_LEH..start + PUBKEY_LEH * 2]).unwrap();

    (pk1, pk2, buffer[(start + 64)..].to_vec())
}

pub fn keccak256(data: &[u8]) -> [u8; PUBKEY_LEH] {
    let mut hasher = Keccak256::new();
    hasher.input(data);
    let mut result = [0u8; PUBKEY_LEH];
    hasher.result(&mut result);
    result
}

pub fn calc_subaddress_m(secret_view_key: &[u8], major: u32, minor: u32) -> [u8; PUBKEY_LEH] {
    let prefix = "SubAddr".as_bytes().to_vec();
    let mut data = prefix.clone();
    data.push(0);
    data.extend_from_slice(secret_view_key);
    data.extend_from_slice(&major.to_le_bytes());
    data.extend_from_slice(&minor.to_le_bytes());
    hash_to_scalar(&data).to_bytes()
}

pub fn hash_to_scalar(data: &[u8]) -> Scalar {
    Scalar::from_bytes_mod_order(keccak256(data))
}

pub fn hash160(data: &[u8]) -> [u8; 20] {
    ripemd160_digest(&sha256_digest(data))
}

pub(crate) fn sha256_digest(data: &[u8]) -> Vec<u8> {
    hashing::sha256(&data).to_vec()
}

fn ripemd160_digest(data: &[u8]) -> [u8; 20] {
    let mut hasher = Ripemd160::new();
    hasher.input(data);
    let mut output = [0u8; 20];
    hasher.result(&mut output);
    output
}
