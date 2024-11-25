use alloc::string::String;
use alloc::vec::Vec;
use cryptoxide::digest::Digest;
use cryptoxide::hashing;
use cryptoxide::ripemd160::Ripemd160;

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

pub fn hash160(data: &[u8]) -> [u8; 20] {
    ripemd160_digest(&sha256_digest(data))
}