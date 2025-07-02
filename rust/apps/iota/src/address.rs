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

    #[test]
    fn test_get_address_from_pubkey() {
        let pubkey = "bfa73107effa14b21ff1b9ae2e6b2e770232b7c29018abbf76475b25395369c0";
        let address = get_address_from_pubkey(pubkey.to_string());
        assert_eq!(
            address.unwrap(),
            "0x193a4811b7207ac7a861f840552f9c718172400f4c46bdef5935008a7977fb04"
        );
    }
}
