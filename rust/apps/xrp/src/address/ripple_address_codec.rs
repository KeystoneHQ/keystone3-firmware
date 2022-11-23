use crate::address::ripple_keypair::sha256_digest;
use alloc::string::String;
use base_x;

const CHECKSUM_LENGTH: usize = 4;
const ALPHABET: &str = "rpshnaf39wBUDNEGHJKLM4PQRST7VWXYZ2bcdeCg65jkm8oFqi1tuvAxyz";

struct Address;

trait Settings {
    const PAYLOAD_LEN: usize;
    const PREFIX: &'static [u8] = &[];

    fn prefix(&self) -> &'static [u8] {
        Self::PREFIX
    }

    fn prefix_len(&self) -> usize {
        Self::PREFIX.len()
    }

    fn payload_len(&self) -> usize {
        Self::PAYLOAD_LEN
    }
}

impl Settings for Address {
    const PAYLOAD_LEN: usize = 20;
    const PREFIX: &'static [u8] = &[0x00];
}

fn calc_checksum(bytes: &[u8]) -> [u8; CHECKSUM_LENGTH] {
    sha256_digest(&sha256_digest(bytes))[..CHECKSUM_LENGTH]
        .try_into()
        .unwrap()
}

fn encode_bytes(bytes: &[u8]) -> String {
    let checked_bytes = [bytes, &calc_checksum(bytes)].concat();
    base_x::encode(ALPHABET, &checked_bytes)
}

fn encode_bytes_with_prefix(prefix: &[u8], bytes: &[u8]) -> String {
    encode_bytes(&[prefix, bytes].concat())
}

pub fn encode_account_id(bytes: &[u8; Address::PAYLOAD_LEN]) -> String {
    encode_bytes_with_prefix(Address.prefix(), bytes)
}
