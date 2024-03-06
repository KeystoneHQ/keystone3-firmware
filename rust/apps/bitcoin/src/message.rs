use alloc::vec::Vec;
use third_party::cryptoxide::hashing::sha256;

const PREFIX_MESSAGE: &str = "\u{0018}Bitcoin Signed Message:\n";

pub fn hash(sign_data: Vec<u8>) -> Vec<u8> {
    let mut message = format!("{}{}", PREFIX_MESSAGE, sign_data.len())
        .as_bytes()
        .to_vec();
    message.extend(sign_data);
    sha256(&message).to_vec()
}
