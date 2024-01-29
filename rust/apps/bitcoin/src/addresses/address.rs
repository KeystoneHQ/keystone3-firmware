extern crate alloc;

use crate::addresses::cashaddr::CashAddrCodec;
use crate::addresses::constants::{
    PUBKEY_ADDRESS_PREFIX_BCH, PUBKEY_ADDRESS_PREFIX_BTC, PUBKEY_ADDRESS_PREFIX_DASH,
    PUBKEY_ADDRESS_PREFIX_DASH_P2SH, PUBKEY_ADDRESS_PREFIX_TEST, SCRIPT_ADDRESS_PREFIX_BTC,
    SCRIPT_ADDRESS_PREFIX_LTC, SCRIPT_ADDRESS_PREFIX_LTC_P2PKH, SCRIPT_ADDRESS_PREFIX_TEST,
};
use crate::addresses::encoding::{
    BCHAddressEncoding, BTCAddressEncoding, DASHAddressEncoding, LTCAddressEncoding,
};
use crate::errors::BitcoinError;
use crate::network::Network;
use alloc::string::ToString;
use alloc::vec::Vec;
use core::fmt;
use core::str::FromStr;
use third_party::bitcoin::address::Payload;
use third_party::bitcoin::address::{WitnessProgram, WitnessVersion};
use third_party::bitcoin::base58;
use third_party::bitcoin::blockdata::script;
use third_party::bitcoin::hashes::Hash;
use third_party::bitcoin::PublicKey;
use third_party::bitcoin::{bech32, PubkeyHash, ScriptHash};

#[derive(Debug)]
pub struct Address {
    pub payload: Payload,
    pub network: Network,
}

impl Address {
    #[inline]
    pub fn p2pkh(pk: &PublicKey, network: Network) -> Result<Address, BitcoinError> {
        match network {
            Network::Bitcoin
            | Network::BitcoinTestnet
            | Network::Litecoin
            | Network::BitcoinCash
            | Network::Dash => Ok(Address {
                network,
                payload: Payload::p2pkh(pk),
            }),
        }
    }

    pub fn p2wpkh(pk: &PublicKey, network: Network) -> Result<Address, BitcoinError> {
        match network {
            Network::Bitcoin | Network::BitcoinTestnet => {
                let payload = Payload::p2wpkh(pk).map_err(|_e| {
                    BitcoinError::AddressError(format!("invalid payload for p2wpkh"))
                })?;
                Ok(Address { network, payload })
            }
            _ => Err(BitcoinError::AddressError(format!(
                "Invalid network for p2wpkh {:?}",
                network
            ))),
        }
    }

    pub fn p2shp2wpkh(pk: &PublicKey, network: Network) -> Result<Address, BitcoinError> {
        match network {
            Network::Bitcoin | Network::BitcoinTestnet | Network::Litecoin => {
                let payload = Payload::p2shwpkh(pk).map_err(|_e| {
                    BitcoinError::AddressError(format!("invalid payload for p2shwpkh"))
                })?;
                Ok(Address { network, payload })
            }
            _ => Err(BitcoinError::AddressError(format!(
                "Invalid network for p2wpkh"
            ))),
        }
    }

    pub fn from_script(script: &script::Script, network: Network) -> Result<Address, BitcoinError> {
        Ok(Address {
            payload: Payload::from_script(script)
                .map_err(|e| BitcoinError::AddressError(format!("invalid payload, {}", e)))?,
            network,
        })
    }
}

impl fmt::Display for Address {
    fn fmt(&self, fmt: &mut fmt::Formatter) -> fmt::Result {
        match self.network {
            Network::Bitcoin => {
                let encoding = BTCAddressEncoding {
                    payload: &self.payload,
                    p2pkh_prefix: PUBKEY_ADDRESS_PREFIX_BTC,
                    p2sh_prefix: SCRIPT_ADDRESS_PREFIX_BTC,
                    bech32_hrp: "bc",
                };
                encoding.fmt(fmt)
            }
            Network::BitcoinTestnet => {
                let encoding = BTCAddressEncoding {
                    payload: &self.payload,
                    p2pkh_prefix: PUBKEY_ADDRESS_PREFIX_TEST,
                    p2sh_prefix: SCRIPT_ADDRESS_PREFIX_TEST,
                    bech32_hrp: "tb",
                };
                encoding.fmt(fmt)
            }
            Network::Dash => {
                let encoding = DASHAddressEncoding {
                    payload: &self.payload,
                    p2pkh_prefix: PUBKEY_ADDRESS_PREFIX_DASH,
                };
                encoding.fmt(fmt)
            }
            Network::Litecoin => {
                let encoding = LTCAddressEncoding {
                    payload: &self.payload,
                    p2sh_prefix: SCRIPT_ADDRESS_PREFIX_LTC,
                    p2pkh_prefix: SCRIPT_ADDRESS_PREFIX_LTC_P2PKH,
                    bech32_hrp: "ltc",
                };
                encoding.fmt(fmt)
            }
            Network::BitcoinCash => {
                let encoding = BCHAddressEncoding {
                    payload: &self.payload,
                    p2pkh_prefix: PUBKEY_ADDRESS_PREFIX_BCH,
                };
                encoding.fmt(fmt)
            }
        }
    }
}

fn find_bech32_prefix(bech32: &str) -> &str {
    match bech32.rfind('1') {
        None => bech32,
        Some(sep) => bech32.split_at(sep).0,
    }
}

impl FromStr for Address {
    type Err = BitcoinError;

    fn from_str(s: &str) -> Result<Address, Self::Err> {
        let bech32_network = match find_bech32_prefix(s) {
            "bc" | "BC" => Some(Network::Bitcoin),
            "ltc" | "LTC" => Some(Network::Litecoin),
            _ => None,
        };
        let cash_addr = CashAddrCodec::decode(s);
        if cash_addr.is_ok() {
            return cash_addr;
        }
        if let Some(network) = bech32_network {
            let (_, payload, variant) = bech32::decode(s)
                .map_err(|_e_| Self::Err::AddressError(format!("bech32 decode failed")))?;
            if payload.is_empty() {
                return Err(Self::Err::AddressError(format!("empty bech32 payload")));
            }

            // Get the script version and program (converted from 5-bit to 8-bit)
            let (version, program): (WitnessVersion, Vec<u8>) = {
                let (v, p5) = payload.split_at(1);
                let witness_version = WitnessVersion::try_from(v[0])
                    .map_err(|_e| Self::Err::AddressError(format!("invalid witness version")))?;
                let program = bech32::FromBase32::from_base32(p5)
                    .map_err(|_e| Self::Err::AddressError(format!("invalid base32")))?;
                (witness_version, program)
            };

            if program.len() < 2 || program.len() > 40 {
                return Err(Self::Err::AddressError(format!(
                    "invalid witness program length {}",
                    (program.len() as u8).to_string()
                )));
            }

            // Specific segwit v0 check.
            if version == WitnessVersion::V0 && (program.len() != 20 && program.len() != 32) {
                return Err(Self::Err::AddressError(format!(
                    "invalid segwit v0 program length"
                )));
            }

            // Encoding check
            let expected = version.bech32_variant();
            if expected != variant {
                return Err(Self::Err::AddressError(format!("invalid bech32 variant")));
            }

            return Ok(Address {
                payload: Payload::WitnessProgram(
                    WitnessProgram::new(version, program)
                        .map_err(|e| BitcoinError::AddressError(format!("{}", e)))?,
                ),
                network,
            });
        }

        // Base58
        if s.len() > 50 {
            return Err(Self::Err::AddressError(format!(
                "invalid base58 length {}",
                (s.len() as u8).to_string()
            )));
        }
        let data = base58::decode_check(s)
            .map_err(|_e| Self::Err::AddressError(format!("invalid base58 check")))?;
        if data.len() != 21 {
            return Err(Self::Err::AddressError(format!(
                "invalid base58 length {}",
                (data.len() as u8).to_string()
            )));
        }

        let (network, payload) = match data[0].clone() {
            PUBKEY_ADDRESS_PREFIX_BTC => {
                let pubkey_hash = PubkeyHash::from_slice(&data[1..])
                    .map_err(|_| Self::Err::AddressError(format!("failed to get pubkey hash")))?;
                (Network::Bitcoin, Payload::PubkeyHash(pubkey_hash))
            }
            PUBKEY_ADDRESS_PREFIX_DASH => {
                let pubkey_hash = PubkeyHash::from_slice(&data[1..])
                    .map_err(|_| Self::Err::AddressError(format!("failed to get pubkey hash")))?;
                (Network::Dash, Payload::PubkeyHash(pubkey_hash))
            }
            PUBKEY_ADDRESS_PREFIX_DASH_P2SH => {
                let script_hash = ScriptHash::from_slice(&data[1..])
                    .map_err(|_| Self::Err::AddressError(format!("failed to get script hash")))?;
                (Network::Dash, Payload::ScriptHash(script_hash))
            }
            SCRIPT_ADDRESS_PREFIX_LTC_P2PKH => {
                let pubkey_hash = PubkeyHash::from_slice(&data[1..])
                    .map_err(|_| Self::Err::AddressError(format!("failed to get pubkey hash")))?;
                (Network::Litecoin, Payload::PubkeyHash(pubkey_hash))
            }
            SCRIPT_ADDRESS_PREFIX_LTC => {
                let script_hash = ScriptHash::from_slice(&data[1..])
                    .map_err(|_| Self::Err::AddressError(format!("failed to get script hash")))?;
                (Network::Litecoin, Payload::ScriptHash(script_hash))
            }
            SCRIPT_ADDRESS_PREFIX_BTC => {
                let script_hash = ScriptHash::from_slice(&data[1..])
                    .map_err(|_| Self::Err::AddressError(format!("failed to get script hash")))?;
                (Network::Bitcoin, Payload::ScriptHash(script_hash))
            }
            _x => return Err(Self::Err::AddressError(format!("invalid address version"))),
        };

        Ok(Address { network, payload })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_address_btc_p2pkh() {
        let addr = Address::from_str("1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2").unwrap();
        assert_eq!(addr.network.get_unit(), "BTC");
        assert_eq!(addr.to_string(), "1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2");
    }

    #[test]
    fn test_address_btc_p2sh() {
        let addr = Address::from_str("3J98t1WpEZ73CNmQviecrnyiWrnqRhWNLy").unwrap();
        assert_eq!(addr.network.get_unit(), "BTC");
        assert_eq!(addr.to_string(), "3J98t1WpEZ73CNmQviecrnyiWrnqRhWNLy");
    }

    #[test]
    fn test_address_btc_p2wpkh() {
        let addr = Address::from_str("bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4").unwrap();
        assert_eq!(addr.network.get_unit(), "BTC");
        assert_eq!(
            addr.to_string(),
            "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4"
        );
    }

    #[test]
    fn test_address_btc_p2tr() {
        let addr =
            Address::from_str("bc1p5d7rjq7g6rdk2yhzks9smlaqtedr4dekq08ge8ztwac72sfr9rusxg3297")
                .unwrap();
        assert_eq!(addr.network.get_unit(), "BTC");
        assert_eq!(
            addr.to_string(),
            "bc1p5d7rjq7g6rdk2yhzks9smlaqtedr4dekq08ge8ztwac72sfr9rusxg3297"
        );
    }

    #[test]
    fn test_address_ltc_p2wpkh() {
        let addr = Address::from_str("ltc1qum864wd9nwsc0u9ytkctz6wzrw6g7zdn08yddf").unwrap();
        assert_eq!(addr.network.get_unit(), "LTC");
        assert_eq!(
            addr.to_string(),
            "ltc1qum864wd9nwsc0u9ytkctz6wzrw6g7zdn08yddf"
        );
    }

    #[test]
    fn test_address_ltc_p2sh() {
        let addr = Address::from_str("MR5Hu9zXPX3o9QuYNJGft1VMpRP418QDfW").unwrap();
        assert_eq!(addr.network.get_unit(), "LTC");
        assert_eq!(addr.to_string(), "MR5Hu9zXPX3o9QuYNJGft1VMpRP418QDfW");
    }

    #[test]
    fn test_address_ltc_p2pkh() {
        let addr = Address::from_str("LhyLNfBkoKshT7R8Pce6vkB9T2cP2o84hx").unwrap();
        assert_eq!(addr.network.get_unit(), "LTC");
        assert_eq!(addr.to_string(), "LhyLNfBkoKshT7R8Pce6vkB9T2cP2o84hx");
    }

    #[test]
    fn test_address_bch() {
        let addr = Address::from_str("qpm2qsznhks23z7629mms6s4cwef74vcwvy22gdx6a").unwrap();
        assert_eq!(addr.network.get_unit(), "BCH");
        assert_eq!(
            addr.to_string(),
            "qpm2qsznhks23z7629mms6s4cwef74vcwvy22gdx6a"
        );
    }
}
