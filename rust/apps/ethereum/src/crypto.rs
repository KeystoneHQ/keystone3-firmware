use cryptoxide::digest::Digest;
use cryptoxide::sha3::Keccak256;

pub fn keccak256(input: &[u8]) -> [u8; 32] {
    let mut hasher = Keccak256::new();
    hasher.input(input);
    let mut output = [0u8; 32];
    hasher.result(&mut output);
    output
}
