use crate::constants::*;
use crate::errors::{AvaxError, Result};
#[cfg(feature = "testnet")]
use crate::network::TESTNET_ID;
use crate::network::{Network, MAINNET_ID};
use crate::ripple_keypair::hash160;
use crate::transactions::structs::ParsedSizeAble;
use alloc::string::{String, ToString};
use bech32::{self, Bech32};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;
use keystore::algorithms::secp256k1::derive_public_key;

#[derive(Debug, Clone)]
pub struct Address {
    address: [u8; ADDRESS_LEN],
}

impl ParsedSizeAble for Address {
    fn parsed_size(&self) -> usize {
        ADDRESS_LEN as usize
    }
}

impl Address {
    pub fn encode(&self) -> String {
        bech32::encode::<Bech32>(bech32::Hrp::parse_unchecked("fuji"), &self.address).unwrap()
        // bech32::encode::<Bech32>(bech32::Hrp::parse_unchecked("avax"), &self.address).unwrap()
    }

    pub fn to_evm_address(&self) -> String {
        format!("0x{}", hex::encode(&self.address))
    }
}

impl TryFrom<Bytes> for Address {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let mut address = [0u8; ADDRESS_LEN];
        bytes.copy_to_slice(&mut address);
        Ok(Address { address })
    }
}

pub fn get_address(
    network: Network,
    hd_path: &str,
    root_x_pub: &str,
    root_path: &str,
) -> Result<String> {
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

    let root_path = if !root_path.ends_with('/') {
        root_path.to_string() + "/"
    } else {
        root_path.to_string()
    };

    let public_key = derive_public_key(
        &root_x_pub.to_string(),
        &format!(
            "m/{}",
            hd_path
                .strip_prefix(&root_path)
                .ok_or(AvaxError::InvalidHDPath(hd_path.to_string()))?
        ),
    )
    .unwrap();
    bech32::encode::<Bech32>(
        bech32::Hrp::parse_unchecked(prefix),
        &hash160(&public_key.serialize()),
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
            let hd_path = "m/44'/9000'/0'/0/0";
            let root_x_pub = "xpub6CPE4bhTujy9CeJJbyskjJsp8FGgyWBsWV2W9GfZwuP9aeDBEoPRBsLk3agq32Gp5gkb9nJSjCn9fgZmuvmV3nPLk5Bc2wfKUQZREp4eG13";
            let root_path = "m/44'/9000'/0'";
            let address = get_address(Network::AvaxMainNet, &hd_path, &root_x_pub, &root_path);
            println!("address = {}", address.unwrap());
            // assert!(false);
        }
        {
            let prefix = "fuji";
            // let data = [
            //     9, 105, 234, 98, 226, 187, 48, 230, 109, 130, 232, 47, 226, 103, 237, 246, 135, 30,
            //     165, 247,
            // ];
            let data = hex::decode("69bc9b5b6cbbbd490abbd79a37ad6cd643be87ab").unwrap();
            assert_eq!(
                "fuji1p9575chzhvcwvmvzaqh7yeld76r3af0h3x77mq",
                bech32::encode::<Bech32>(bech32::Hrp::parse_unchecked(prefix), &data).unwrap()
            );
        }
    }
}
