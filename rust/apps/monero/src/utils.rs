use curve25519_dalek::scalar::Scalar;
use third_party::cryptoxide::digest::Digest;
use third_party::cryptoxide::sha3::Keccak256;

pub fn keccak256(data: &[u8]) -> [u8; 32] {
    let mut hasher = Keccak256::new();
    hasher.input(data);
    let mut result = [0u8; 32];
    hasher.result(&mut result);
    result
}

pub fn calc_subaddress_m(secret_view_key: &[u8], major: u32, minor: u32) -> [u8; 32] {
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
