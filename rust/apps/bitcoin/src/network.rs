use crate::errors::BitcoinError;
use alloc::string::{String, ToString};
use core::str::FromStr;

pub trait NetworkT {
    fn get_unit(&self) -> String;
    fn normalize(&self) -> String;
}

#[derive(Debug, Clone)]
pub enum Network {
    Bitcoin,
    BitcoinTestnet,
    Litecoin,
    Dash,
    BitcoinCash,
}

impl NetworkT for Network {
    fn get_unit(&self) -> String {
        match self {
            Network::Bitcoin => "BTC",
            Network::BitcoinTestnet => "tBTC",
            Network::Litecoin => "LTC",
            Network::Dash => "DASH",
            Network::BitcoinCash => "BCH",
        }
        .to_string()
    }

    fn normalize(&self) -> String {
        match self {
            Network::Bitcoin => "Bitcoin Mainnet",
            Network::BitcoinTestnet => "Bitcoin Testnet",
            Network::Litecoin => "Litecoin",
            Network::Dash => "Dash",
            Network::BitcoinCash => "Bitcoin Cash",
        }
        .to_string()
    }
}

impl Network {
    
    pub fn bip44_coin_type(&self) -> String {
        match self {
            Network::Bitcoin => 0,
            Network::BitcoinTestnet => 1,
            Network::Litecoin => 2,
            Network::Dash => 5,
            Network::BitcoinCash => 145,
        }
        .to_string()
    }
}

impl FromStr for Network {
    type Err = BitcoinError;
    fn from_str(network: &str) -> Result<Self, BitcoinError> {
        match network {
            "BTC" => Ok(Self::Bitcoin),
            "tBTC" => Ok(Self::BitcoinTestnet),
            "LTC" => Ok(Self::Litecoin),
            "DASH" => Ok(Self::Dash),
            "BCH" => Ok(Self::BitcoinCash),
            "BTC_NATIVE_SEGWIT" => Ok(Self::Bitcoin),
            "BTC_SEGWIT" => Ok(Self::Bitcoin),
            "BTC_LEGACY" => Ok(Self::Bitcoin),
            _ => Err(BitcoinError::UnsupportedNetwork(format!("{:?}", network))),
        }
    }
}

#[derive(Debug, Clone, PartialEq)]
pub enum CustomNewNetwork {
    FractalBitcoin,
    FractalBitcoinTest
}

impl NetworkT for CustomNewNetwork {
    fn get_unit(&self) -> String {
        match self {
            CustomNewNetwork::FractalBitcoin => "FB",
            CustomNewNetwork::FractalBitcoinTest => "tFB",
        }
        .to_string()
    }

    fn normalize(&self) -> String {
        match self {
            CustomNewNetwork::FractalBitcoin => "Fractal Bitcoin",
            CustomNewNetwork::FractalBitcoinTest => "Fractal Bitcoin Testnet",
        }
        .to_string()
    }
}
