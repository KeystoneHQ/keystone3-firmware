use crate::errors::{IotaError, Result};
use alloc::{
    format,
    string::{String, ToString},
};
use cryptoxide::hashing::blake2b_256;
use hex;

pub fn get_address_from_pubkey(pubkey: String) -> Result<String> {
    let pubkey_bytes = hex::decode(pubkey)?;
    if pubkey_bytes.len() != 32 {
        return Err(IotaError::InvalidData("pubkey is not 32 bytes".to_string()));
    }

    Ok(format!("0x{}", hex::encode(blake2b_256(&pubkey_bytes))))
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;
    use cryptoxide::hashing::blake2b_256;

    #[test]
    fn test_get_address_from_pubkey() {
        let pubkey = "bfa73107effa14b21ff1b9ae2e6b2e770232b7c29018abbf76475b25395369c0";
        let address = get_address_from_pubkey(pubkey.to_string());
        assert_eq!(
            address.unwrap(),
            "0x193a4811b7207ac7a861f840552f9c718172400f4c46bdef5935008a7977fb04"
        );
    }

    #[test]
    fn test_get_address_from_pubkey_invalid_hex() {
        let pubkey = "zz"; // invalid hex
        let address = get_address_from_pubkey(pubkey.to_string());
        assert!(matches!(address, Err(IotaError::InvalidData(_))));
    }

    #[test]
    fn test_get_address_from_pubkey_with_prefix() {
        let pubkey = "0xbfa73107effa14b21ff1b9ae2e6b2e770232b7c29018abbf76475b25395369c0";
        let address = get_address_from_pubkey(pubkey.to_string());
        assert!(matches!(address, Err(IotaError::InvalidData(_))));
    }

    #[test]
    fn test_get_address_from_pubkey_empty() {
        let pubkey = "";
        let address = get_address_from_pubkey(pubkey.to_string());
        assert!(
            matches!(address, Err(IotaError::InvalidData(msg)) if msg.contains("pubkey is not 32 bytes"))
        );
    }

    #[test]
    fn test_get_address_from_pubkey_len_31() {
        // 31 bytes (62 hex chars)
        let pubkey = "aa".repeat(31);
        let address = get_address_from_pubkey(pubkey);
        assert!(
            matches!(address, Err(IotaError::InvalidData(msg)) if msg.contains("pubkey is not 32 bytes"))
        );
    }

    #[test]
    fn test_get_address_from_pubkey_case_insensitive() {
        let lower = "bfa73107effa14b21ff1b9ae2e6b2e770232b7c29018abbf76475b25395369c0";
        let upper = lower.to_uppercase();
        let addr_lower = get_address_from_pubkey(lower.to_string()).unwrap();
        let addr_upper = get_address_from_pubkey(upper);
        assert_eq!(addr_lower, addr_upper.unwrap());
    }

    #[test]
    fn test_get_address_from_pubkey_output_format() {
        let pubkey = "bfa73107effa14b21ff1b9ae2e6b2e770232b7c29018abbf76475b25395369c0";
        let addr = get_address_from_pubkey(pubkey.to_string()).unwrap();
        assert!(addr.starts_with("0x"));
        assert_eq!(addr.len(), 66);
    }
}
