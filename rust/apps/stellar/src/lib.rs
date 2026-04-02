#![no_std]

pub mod address;
pub mod errors;
pub mod strkeys;
pub mod structs;

#[macro_use]
extern crate alloc;

use crate::errors::{Result, StellarError};
use crate::structs::Network;
use alloc::string::String;
use alloc::vec::Vec;

fn ensure_signature_base_len(base: &[u8]) -> Result<()> {
    if base.len() < 32 {
        return Err(StellarError::InvalidData(
            "signature base is shorter than 32 bytes".into(),
        ));
    }
    Ok(())
}

pub fn get_network_from_base(base: &[u8]) -> Result<Network> {
    ensure_signature_base_len(base)?;
    let network_id = &base[0..32];
    Ok(Network::from_hash(network_id))
}

fn strip_network_prefix(base: &[u8]) -> Result<Vec<u8>> {
    ensure_signature_base_len(base)?;
    Ok(base[32..].to_vec())
}

pub fn base_to_xdr(base: &[u8]) -> Result<String> {
    let stripped_base = strip_network_prefix(base)?;
    Ok(base64::encode(&stripped_base))
}

#[cfg(test)]
mod tests {

    extern crate std;

    use super::*;

    #[test]
    fn test_network() {
        let public = Network::Public;
        assert_eq!(
            "Public Global Stellar Network ; September 2015",
            public.get()
        );
    }

    #[test]
    fn test_network_hash() {
        let public = Network::Public;
        assert_eq!(
            "7ac33997544e3175d266bd022439b22cdb16508c01163f26e5cb2a3e1045a979",
            public.hash()
        );
    }

    #[test]
    fn test_hash_from() {
        let signature_base = "7ac33997544e3175d266bd022439b22cdb16508c01163f26e5cb2a3e1045a979000000020000000096e8c54780e871fabf106cb5b047149e72b04aa5e069a158b2d0e7a68ab50d4f00002710031494870000000a00000001000000000000000000000000664ed303000000000000000100000000000000060000000155534443000000003b9911380efe988ba0a8900eb1cfe44f366f7dbe946bed077240f7f624df15c57fffffffffffffff00000000";
        let network = get_network_from_base(&hex::decode(signature_base).unwrap()).unwrap();
        assert_eq!(
            network.get(),
            "Public Global Stellar Network ; September 2015"
        );
    }

    #[test]
    fn test_strip_network_prefix() {
        let signature_base = "7ac33997544e3175d266bd022439b22cdb16508c01163f26e5cb2a3e1045a979000000020000000096e8c54780e871fabf106cb5b047149e72b04aa5e069a158b2d0e7a68ab50d4f00002710031494870000000a00000001000000000000000000000000664ed303000000000000000100000000000000060000000155534443000000003b9911380efe988ba0a8900eb1cfe44f366f7dbe946bed077240f7f624df15c57fffffffffffffff00000000";
        let data = strip_network_prefix(&hex::decode(signature_base).unwrap()).unwrap();
        assert_eq!(data.len(), 144);
        assert_eq!(hex::encode(data), "000000020000000096e8c54780e871fabf106cb5b047149e72b04aa5e069a158b2d0e7a68ab50d4f00002710031494870000000a00000001000000000000000000000000664ed303000000000000000100000000000000060000000155534443000000003b9911380efe988ba0a8900eb1cfe44f366f7dbe946bed077240f7f624df15c57fffffffffffffff00000000");
    }

    // #[test]
    // fn test_base_to_xdr() {
    //   let signature_base = "7ac33997544e3175d266bd022439b22cdb16508c01163f26e5cb2a3e1045a979000000020000000096e8c54780e871fabf106cb5b047149e72b04aa5e069a158b2d0e7a68ab50d4f00002710031494870000000a00000001000000000000000000000000664ed303000000000000000100000000000000060000000155534443000000003b9911380efe988ba0a8900eb1cfe44f366f7dbe946bed077240f7f624df15c57fffffffffffffff00000000";
    //   let target_xdr = "AAAAAgAAAACW6MVHgOhx+r8QbLWwRxSecrBKpeBpoViy0OemirUNTwAAJxADFJSHAAAACgAAAAEAAAAAAAAAAAAAAABmTtMDAAAAAAAAAAEAAAAAAAAABgAAAAFVU0RDAAAAADuZETgO/piLoKiQDrHP5E82b32+lGvtB3JA9/Yk3xXFf/////////8AAAAAAAAAAA==";
    //   let xdr = base_to_xdr(&hex::decode(signature_base).unwrap());
    //   assert_eq!(target_xdr, xdr);
    // }
}
