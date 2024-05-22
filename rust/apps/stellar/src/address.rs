use crate::strkeys::{calculate_crc16_checksum, encode_base32, StrKeyType};
use alloc::string::String;
use alloc::vec::Vec;
use core::str::FromStr;
use keystore::algorithms::ed25519::slip10_ed25519::get_public_key_by_seed;
use keystore::errors::Result;

pub fn generate_stellar_address(
    seed: &[u8],
    path: &String,
    key_type: StrKeyType,
) -> Result<String> {
    let public_key = get_public_key_by_seed(seed, path)?;
    let key = [key_type as u8]
        .iter()
        .chain(public_key.iter())
        .cloned()
        .collect::<Vec<u8>>();
    let checksum = calculate_crc16_checksum(&key);
    let data = key
        .iter()
        .chain(checksum.iter())
        .cloned()
        .collect::<Vec<u8>>();
    Ok(encode_base32(&data))
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;
    use third_party::hex;

    #[test]
    fn test_stellar_address() {
        let seed = hex::decode("96063c45132c840f7e1665a3b97814d8eb2586f34bd945f06fa15b9327eebe355f654e81c6233a52149d7a95ea7486eb8d699166f5677e507529482599624cdc").unwrap();
        let path = "m/44'/148'/0'".to_string();
        let address = generate_stellar_address(&seed, &path, StrKeyType::STRKEY_PUBKEY).unwrap();
        assert_eq!(
            "GDKLQMRO2LFHLJ5I67VVOBLUOGYXXV6V72SPTKFCSNRWWTLFH7HT33AU",
            address
        );
    }
}
