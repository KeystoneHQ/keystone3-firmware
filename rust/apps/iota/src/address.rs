use crate::errors::{Result, IotaError};
use alloc::{format, string::{String, ToString}};
use cryptoxide::hashing::blake2b_256;
use hex;

pub fn get_address_from_xpub(xpub: String) -> Result<String> {
    let xpub_bytes = hex::decode(xpub)?;
    if xpub_bytes.len() != 32 {
        return Err(IotaError::InvalidData("xpub is not 32 bytes".to_string()));
    }

    Ok(format!("0x{}", hex::encode(blake2b_256(&xpub_bytes))))
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;

    #[test]
    fn test_get_address_from_xpub() {
        let xpub = "bfa73107effa14b21ff1b9ae2e6b2e770232b7c29018abbf76475b25395369c0";
        let address = get_address_from_xpub(xpub.to_string());
        assert_eq!(address.unwrap(), "0x193a4811b7207ac7a861f840552f9c718172400f4c46bdef5935008a7977fb04");
    }
}
