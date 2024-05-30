use alloc::vec::Vec;
use third_party::cryptoxide::{digest::Digest, hmac::Hmac, pbkdf2, sha2::{Sha256, Sha512}};

pub(crate) fn pbkdf2_sha512(key: &[u8], salt: &[u8], iterations: u32, output: &mut [u8]) {
    let mut hmac = Hmac::new(Sha512::new(), key);
    pbkdf2::pbkdf2(&mut hmac, salt, iterations, output)
}

pub(crate) fn sha256(data: &[u8]) -> Vec<u8> {
    let mut hasher = Sha256::new();
    let mut out = [0u8; 32];
    hasher.input(data);
    hasher.result(&mut out);
    out.to_vec()
}