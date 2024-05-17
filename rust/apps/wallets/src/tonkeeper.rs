use alloc::string::String;
use alloc::vec::Vec;
use third_party::ur_registry::crypto_hd_key::CryptoHDKey;
use third_party::ur_registry::crypto_key_path::CryptoKeyPath;
use third_party::ur_registry::error::{URError, URResult};

pub struct PathInfo {
    pub path: String,
    pub mfp: [u8; 4],
}

pub fn generate_sync_ur(
    pubkey: Vec<u8>,
    wallet_name: String,
    path_info: Option<PathInfo>,
) -> URResult<CryptoHDKey> {
    match path_info {
        Some(info) => {
            let key_path = info.path;
            let master_fingerprint = info.mfp;
            let crypto_key_path = CryptoKeyPath::from_path(key_path, Some(master_fingerprint))
                .map_err(|e| URError::UrEncodeError(e))?;
            Ok(CryptoHDKey::new_extended_key(
                None,
                pubkey,
                None,
                None,
                Some(crypto_key_path),
                None,
                None,
                None,
                Some(wallet_name),
            ))
        }
        None => Ok(CryptoHDKey::new_extended_key(
            None,
            pubkey,
            None,
            None,
            None,
            None,
            None,
            Some(wallet_name),
            None,
        )),
    }
}
#[cfg(test)]
mod tests {
    use alloc::string::ToString;
    use third_party::hex;

    use super::*;

    #[test]
    fn test_generate_sync_ur() {
        let pubkey =
            hex::decode("8fb494cc03adcba0b7c287ad066117a1b66809feaf2591a8415a261c9738da85")
                .unwrap();
        let result = generate_sync_ur(pubkey, "Keystone".to_string(), None).unwrap();
        let bytes: Vec<u8> = result.try_into().unwrap();
        assert_eq!(hex::encode(bytes), "a20358208fb494cc03adcba0b7c287ad066117a1b66809feaf2591a8415a261c9738da8509684b657973746f6e65");
    }
}
