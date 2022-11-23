use crate::errors::{BitcoinError, Result};
use crate::network::Network;
use alloc::string::{String, ToString};
use core::fmt;
use core::str::FromStr;
use third_party::bitcoin::bip32::DerivationPath;

#[derive(PartialEq, Debug)]
pub enum ScriptType {
    P2PKH,
    P2SHP2WPKH,
    P2WPKH,
    P2SHP2WSH,
    P2WSH,
    P2SH,
    RAW,
}

impl ScriptType {
    pub fn from_coin_code(coin_code: String) -> Result<ScriptType> {
        match coin_code.as_str() {
            "BTC" => Ok(ScriptType::P2SHP2WPKH),
            "BTC_LEGACY" => Ok(ScriptType::P2PKH),
            "BTC_SEGWIT" => Ok(ScriptType::P2SHP2WPKH),
            "BTC_NATIVE_SEGWIT" => Ok(ScriptType::P2WPKH),
            "LTC" => Ok(ScriptType::P2SHP2WPKH),
            "DASH" => Ok(ScriptType::P2PKH),
            "BCH" => Ok(ScriptType::P2PKH),
            _ => Err(BitcoinError::UnsupportedScriptType(coin_code)),
        }
    }
    pub fn to_derivation_path(&self, network: &Network) -> Result<DerivationPath> {
        let coin_type = network.bip44_coin_type();
        let path_str = match self {
            ScriptType::P2PKH => Ok(format!("m/44'/{}'/0'", coin_type)),
            ScriptType::P2SHP2WPKH | ScriptType::P2SH | ScriptType::P2SHP2WSH => {
                Ok(format!("m/49'/{}'/0'", coin_type))
            }
            ScriptType::P2WPKH | ScriptType::P2WSH => Ok(format!("m/84'/{}'/0'", coin_type)),
            ScriptType::RAW => Err(BitcoinError::UnsupportedScriptType("raw".to_string())),
        }?;
        DerivationPath::from_str(path_str.as_str())
            .map_err(|_| BitcoinError::UnsupportedScriptType("invalid derivation path".to_string()))
    }
}

impl FromStr for ScriptType {
    type Err = BitcoinError;
    fn from_str(script_type: &str) -> Result<Self> {
        match script_type {
            "P2PKH" => Ok(Self::P2PKH),
            "P2SHP2WPKH" => Ok(Self::P2SHP2WPKH),
            "P2WPKH" => Ok(Self::P2WPKH),
            "P2SHP2WSH" => Ok(Self::P2SHP2WSH),
            "P2WSH" => Ok(Self::P2WSH),
            "P2SH" => Ok(Self::P2SH),
            "RAW" => Ok(Self::RAW),
            _ => Err(BitcoinError::UnsupportedScriptType(format!(
                "{:?}",
                script_type
            ))),
        }
    }
}

impl fmt::Display for ScriptType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let script_type_str = match self {
            ScriptType::P2PKH => "P2PKH",
            ScriptType::P2SHP2WPKH => "P2SHP2WPKH",
            ScriptType::P2WPKH => "P2WPKH",
            ScriptType::P2SHP2WSH => "P2SHP2WSH",
            ScriptType::P2WSH => "P2WSH",
            ScriptType::P2SH => "P2SH",
            ScriptType::RAW => "RAW",
        }
        .to_string();
        write!(f, "{}", script_type_str)
    }
}
