use crate::errors::{AvaxError, Result};
#[cfg(feature = "testnet")]
use crate::network::TESTNET_ID;
use crate::network::{Network, MAINNET_ID};
use crate::ripple_keypair::hash160;
use alloc::string::String;
use bech32::{self, Bech32};
use bitcoin::bip32::{ChildNumber, Xpub};
use core::str::FromStr;

pub fn get_address(network: Network, extended_pub_key: &String) -> Result<String> {
    let mut prefix = "avax";
    match network {
        Network::AvaxMainNet => {}
        #[cfg(feature = "testnet")]
        Network::AvaxTestNet => {
            prefix = "fuji";
        }
        _ => {
            return Err(AvaxError::UnsupportedNetwork(format!("{:?}", network)));
        }
    }
    let xpub = Xpub::from_str(extended_pub_key.as_str())
        .map_err(|_e| AvaxError::InvalidHex(format!("invalid xpub")))?;

    if let ChildNumber::Normal { index } = xpub.child_number {
        if xpub.depth != 5 || index != 0 {
            return Err(AvaxError::InvalidHex(format!("invalid xpub path")));
        }
    } else {
        return Err(AvaxError::InvalidHex(format!("invalid xpub path")));
    }

    bech32::encode::<Bech32>(
        bech32::Hrp::parse_unchecked(prefix),
        &hash160(&xpub.public_key.serialize()),
    )
    .map_err(|e| AvaxError::InvalidHex(format!("bech32 encode error: {}", e)))
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;

    #[test]
    fn get_avax_address_test() {
        {
            let extended_pub_key = "xpub6FhuXFXbBLaLykvMfme4YCM4mwq8mP9HxW8Pww2DdmK18GkZnDjhGCgtU8m9oTz2HeLrdqJaWQp2WxQQprpu2icAWVndHWYDXT8pnMJ46SZ";
            let address = get_address(Network::AvaxMainNet, &extended_pub_key.to_string()).unwrap();
            assert_eq!(
                "avax1fmlmwakmgkezcg95lk97m8p3tgc9anuxemenwh".to_string(),
                address
            );
        }
        {
            let extended_pub_key = "xpub6CtGrkSu2vBVfzzN6fn19rH9GWpbVckvoDJL4GtHGyU7a8f6oHsdz4WLtzAkGcfSipG5evZqwqaT1fnmHkeX73jGSjzVwyA9joeijJMdR6R";
            let address = get_address(Network::AvaxMainNet, &extended_pub_key.to_string());
            if let Err(AvaxError::InvalidHex(msg)) = address {
                assert_eq!(msg, "invalid xpub path");
            }
        }
    }
}
