use crate::errors::AvaxError;
use alloc::string::{String, ToString};
use core::str::FromStr;

pub trait NetworkT {
    fn get_unit(&self) -> String;
    fn normalize(&self) -> String;
}

#[derive(Debug, Clone)]
pub enum Network {
    AvaxMainNet,

    #[cfg(feature = "testnet")]
    AvaxTestNet,

    // @todo: Add Local and CustomNewNetwork
}

pub const MAINNET_ID: u32 = 1;
#[cfg(feature = "testnet")]
pub const FUJI_ID: u32 = 5;

pub const AVAX_COIN_TYPE: u32 = 9000;

pub const MAINNET_HRP: &str = "avax";
pub const FUJI_HRP: &str = "fuji";
pub const LOCAL_HRP: &str = "local";

impl NetworkT for Network {
    fn get_unit(&self) -> String {
        match self {
            Network::AvaxMainNet => "AVAX",
            #[cfg(feature = "testnet")]
            Network::AvaxTestNet => "tAVAX",
        }
        .to_string()
    }

    fn normalize(&self) -> String {
        match self {
            Network::AvaxMainNet => "Avax Mainnet",
            #[cfg(feature = "testnet")]
            Network::AvaxTestNet => "Avax Testnet",
        }
        .to_string()
    }
}

impl Network {
    pub fn bip44_coin_type(&self) -> String {
        AVAX_COIN_TYPE.to_string()
    }
}

impl FromStr for Network {
    type Err = AvaxError;
    fn from_str(network: &str) -> Result<Self, AvaxError> {
        match network {
            "avax" => Ok(Network::AvaxMainNet),
            #[cfg(feature = "testnet")]
            "fuji" => Ok(Network::AvaxTestNet),
            _ => Err(AvaxError::UnsupportedNetwork(format!("{:?}", network))),
        }
    }
}