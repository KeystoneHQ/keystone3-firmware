use crate::errors::{Result, StellarError};
use crate::strkeys::{calculate_crc16_checksum, encode_base32, StrKeyType};
use alloc::string::String;
use alloc::vec::Vec;
use core::str::FromStr;
use keystore::algorithms::ed25519::slip10_ed25519::get_public_key_by_seed;
use third_party::hex;

pub fn get_address(pub_key: &String) -> Result<String> {
    match hex::decode(pub_key) {
        Ok(pub_key) => {
            let key_type = StrKeyType::STRKEY_PUBKEY;
            let key = [key_type as u8]
                .iter()
                .chain(pub_key.iter())
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
        Err(e) => Err(StellarError::AddressError(format!(
            "hex decode error {}",
            e
        ))),
    }
}

pub fn generate_stellar_address(
    seed: &[u8],
    path: &String,
) -> Result<String> {
    let public_key = get_public_key_by_seed(seed, path)?;
    get_address(&hex::encode(public_key))
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
        let address = generate_stellar_address(&seed, &path).unwrap();
        assert_eq!(
            "GDKLQMRO2LFHLJ5I67VVOBLUOGYXXV6V72SPTKFCSNRWWTLFH7HT33AU",
            address
        );
    }

    #[test]
    fn test_stellar_xpub_address() {
        let xpub = "5f5a723f0f3ef785387b016a8b61eb2713b02f429edadfacd96082d7da029594";
        let address_from_xpub = get_address(&xpub.to_string()).unwrap();
        assert_eq!(
            "GBPVU4R7B47PPBJYPMAWVC3B5MTRHMBPIKPNVX5M3FQIFV62AKKZIPNL",
            address_from_xpub
        );
    }

    #[test]
    fn test_stellar_xpub_address_error() {
        let address_error = StellarError::AddressError(
            "hex decode error Invalid character 'g' at position 63".to_string(),
        );
        let error: StellarError = address_error.into();
        let xpub = "5f5a723f0f3ef785387b016a8b61eb2713b02f429edadfacd96082d7da02959g";
        let address_from_xpub = get_address(&xpub.to_string());
        assert_eq!(
            error.to_string(),
            address_from_xpub.unwrap_err().to_string()
        );
    }
}
