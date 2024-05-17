use alloc::string::String;
use alloc::vec::Vec;
use core::str::FromStr;
use keystore::algorithms::ed25519::slip10_ed25519::get_public_key_by_seed;
use keystore::errors::Result;
use crate::strkeys::{StrKeyType, calculate_crc16_checksum, encode_base32};

pub fn generate_stellar_address(seed: &[u8], path: &String, key_type: StrKeyType) -> Result<String> {
    let public_key = get_public_key_by_seed(seed, path)?;
    let key = [key_type as u8].iter().chain(public_key.iter()).cloned().collect::<Vec<u8>>();
    let checksum = calculate_crc16_checksum(&key);
    let data = key.iter().chain(checksum.iter()).cloned().collect::<Vec<u8>>();
    Ok(encode_base32(&data))
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;
    use third_party::hex;

    #[test]
    fn test_stellar_address() {
        let seed = hex::decode("e4a5a632e70943ae7f07659df1332160937fad82587216a4c64315a0fb39497ee4a01f76ddab4cba68147977f3a147b6ad584c41808e8238a07f6cc4b582f186").unwrap();
        let path = "m/44'/148'/0'".to_string();
        let address = generate_stellar_address(&seed, &path, StrKeyType::STRKEY_PUBKEY).unwrap();
        assert_eq!(
            "GDRXE2BQUC3AZNPVFSCEZ76NJ3WWL25FYFK6RGZGIEKWE4SOOHSUJUJ6",
            address
        );
    }
}