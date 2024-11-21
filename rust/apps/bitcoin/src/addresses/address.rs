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
use bech32;
use bitcoin::address::AddressData as Payload;
use bitcoin::blockdata::script;
use bitcoin::script::PushBytesBuf;
use bitcoin::secp256k1::{Secp256k1, XOnlyPublicKey};
use bitcoin::{base58, Script, TapNodeHash};
use bitcoin::{CompressedPublicKey, PublicKey};
use bitcoin::{PubkeyHash, ScriptHash};
use bitcoin::{WitnessProgram, WitnessVersion};
use bitcoin_hashes::Hash;
use core::fmt;
use core::str::FromStr;

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
                payload: Payload::P2pkh {
                    pubkey_hash: pk.pubkey_hash(),
                },
            }),
        }
    }

    pub fn p2wpkh(pk: &PublicKey, network: Network) -> Result<Address, BitcoinError> {
        match network {
            Network::Bitcoin | Network::BitcoinTestnet => {
                let payload = Payload::Segwit {
                    witness_program: WitnessProgram::p2wpkh(
                        &CompressedPublicKey::try_from(pk.clone()).map_err(|e| {
                            BitcoinError::AddressError(format!("invalid payload for p2wpkh: {}", e))
                        })?,
                    ),
                };
                Ok(Address { network, payload })
            }
            _ => Err(BitcoinError::AddressError(format!(
                "Invalid network for p2wpkh {:?}",
                network
            ))),
        }
    }

    pub fn p2tr_no_script(pk: &PublicKey, network: Network) -> Result<Address, BitcoinError> {
        match network {
            Network::Bitcoin | Network::BitcoinTestnet => {
                let secp = Secp256k1::verification_only();
                let payload = Payload::Segwit {
                    witness_program: WitnessProgram::p2tr(
                        &secp,
                        XOnlyPublicKey::from(pk.inner),
                        None,
                    ),
                };
                Ok(Address { network, payload })
            }
            _ => Err(BitcoinError::AddressError(format!(
                "Invalid network for p2tr {:?}",
                network
            ))),
        }
    }

    pub fn p2tr(
        x_only_pubkey: &XOnlyPublicKey,
        merkle_root: Option<TapNodeHash>,
        network: Network,
    ) -> Result<Address, BitcoinError> {
        match network {
            Network::Bitcoin | Network::BitcoinTestnet => {
                let secp = Secp256k1::verification_only();
                let payload = Payload::Segwit {
                    witness_program: WitnessProgram::p2tr(&secp, *x_only_pubkey, merkle_root),
                };
                Ok(Address { network, payload })
            }
            _ => Err(BitcoinError::AddressError(format!(
                "Invalid network for p2tr {:?}",
                network
            ))),
        }
    }

    pub fn p2shp2wpkh(pk: &PublicKey, network: Network) -> Result<Address, BitcoinError> {
        match network {
            Network::Bitcoin | Network::BitcoinTestnet | Network::Litecoin => {
                let builder = script::Builder::new()
                    .push_int(0)
                    .push_slice(pk.wpubkey_hash().map_err(|e| {
                        BitcoinError::AddressError(format!("invalid payload for p2shwpkh: {}", e))
                    })?);
                let script_hash = builder.as_script().script_hash();
                let payload = Payload::P2sh {
                    script_hash,
                };
                Ok(Address { network, payload })
            }
            _ => Err(BitcoinError::AddressError(format!(
                "Invalid network for p2wpkh"
            ))),
        }
    }

    pub fn from_script(script: &script::Script, network: Network) -> Result<Address, BitcoinError> {
        Ok(Address {
            payload: Payload::P2sh {
                script_hash: script.script_hash(),
            },
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
                    p2sh_prefix: PUBKEY_ADDRESS_PREFIX_DASH_P2SH,
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
            "tb" | "TB" => Some(Network::BitcoinTestnet),
            "ltc" | "LTC" => Some(Network::Litecoin),
            _ => None,
        };
        let cash_addr = CashAddrCodec::decode(s);
        if cash_addr.is_ok() {
            return cash_addr;
        }
        if let Some(network) = bech32_network {
            let (_hrp, version, data) = bech32::segwit::decode(s)?;
            let version = WitnessVersion::try_from(version).expect("we know this is in range 0-16");
            let program = PushBytesBuf::try_from(data).expect("decode() guarantees valid length");
            let witness_program = WitnessProgram::new(version, program.as_bytes())?;

            return Ok(Address {
                network,
                payload: Payload::Segwit { witness_program },
            });
            // let (_, payload, variant) = bech32::decode(s)
            //     .map_err(|_e_| Self::Err::AddressError(format!("bech32 decode failed")))?;
            // if payload.is_empty() {
            //     return Err(Self::Err::AddressError(format!("empty bech32 payload")));
            // }

            // // Get the script version and program (converted from 5-bit to 8-bit)
            // let (version, program): (WitnessVersion, Vec<u8>) = {
            //     let (v, p5) = payload.split_at(1);
            //     let witness_version = WitnessVersion::try_from(v[0])
            //         .map_err(|_e| Self::Err::AddressError(format!("invalid witness version")))?;
            //     let program = bech32::FromBase32::from_base32(p5)
            //         .map_err(|_e| Self::Err::AddressError(format!("invalid base32")))?;
            //     (witness_version, program)
            // };

            // if program.len() < 2 || program.len() > 40 {
            //     return Err(Self::Err::AddressError(format!(
            //         "invalid witness program length {}",
            //         (program.len() as u8).to_string()
            //     )));
            // }

            // // Specific segwit v0 check.
            // if version == WitnessVersion::V0 && (program.len() != 20 && program.len() != 32) {
            //     return Err(Self::Err::AddressError(format!(
            //         "invalid segwit v0 program length"
            //     )));
            // }

            // // Encoding check
            // let expected = version.bech32_variant();
            // if expected != variant {
            //     return Err(Self::Err::AddressError(format!("invalid bech32 variant")));
            // }

            // return Ok(Address {
            //     payload: Payload::WitnessProgram(
            //         WitnessProgram::new(version, program)
            //             .map_err(|e| BitcoinError::AddressError(format!("{}", e)))?,
            //     ),
            //     network,
            // });
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
                (Network::Bitcoin, Payload::P2pkh { pubkey_hash })
            }
            PUBKEY_ADDRESS_PREFIX_TEST => {
                let pubkey_hash = PubkeyHash::from_slice(&data[1..])
                    .map_err(|_| Self::Err::AddressError(format!("failed to get pubkey hash")))?;
                (Network::BitcoinTestnet, Payload::P2pkh { pubkey_hash })
            }
            PUBKEY_ADDRESS_PREFIX_DASH => {
                let pubkey_hash = PubkeyHash::from_slice(&data[1..])
                    .map_err(|_| Self::Err::AddressError(format!("failed to get pubkey hash")))?;
                (Network::Dash, Payload::P2pkh { pubkey_hash })
            }
            PUBKEY_ADDRESS_PREFIX_DASH_P2SH => {
                let script_hash = ScriptHash::from_slice(&data[1..])
                    .map_err(|_| Self::Err::AddressError(format!("failed to get script hash")))?;
                (Network::Dash, Payload::P2sh { script_hash })
            }
            SCRIPT_ADDRESS_PREFIX_LTC_P2PKH => {
                let pubkey_hash = PubkeyHash::from_slice(&data[1..])
                    .map_err(|_| Self::Err::AddressError(format!("failed to get pubkey hash")))?;
                (Network::Litecoin, Payload::P2pkh { pubkey_hash })
            }
            SCRIPT_ADDRESS_PREFIX_LTC => {
                let script_hash = ScriptHash::from_slice(&data[1..])
                    .map_err(|_| Self::Err::AddressError(format!("failed to get script hash")))?;
                (Network::Litecoin, Payload::P2sh { script_hash })
            }
            SCRIPT_ADDRESS_PREFIX_BTC => {
                let script_hash = ScriptHash::from_slice(&data[1..])
                    .map_err(|_| Self::Err::AddressError(format!("failed to get script hash")))?;
                (Network::Bitcoin, Payload::P2sh { script_hash })
            }
            SCRIPT_ADDRESS_PREFIX_TEST => {
                let script_hash = ScriptHash::from_slice(&data[1..])
                    .map_err(|_| Self::Err::AddressError(format!("failed to get script hash")))?;
                (Network::BitcoinTestnet, Payload::P2sh { script_hash })
            }
            _x => return Err(Self::Err::AddressError(format!("invalid address version"))),
        };

        Ok(Address { network, payload })
    }
}

#[cfg(test)]
mod tests {
    use crate::network::NetworkT;

    use super::*;

    #[test]
    fn test_address_btc_p2pkh() {
        let addr = Address::from_str("1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2").unwrap();
        assert_eq!(addr.network.get_unit(), "BTC");
        assert_eq!(addr.to_string(), "1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2");
    }

    #[test]
    fn test_address_btc_p2pkh_testnet() {
        let addr = Address::from_str("mszm85TQkAhvAigVfraWicXNnCypp1TTbH").unwrap();
        assert_eq!(addr.network.get_unit(), "tBTC");
        assert_eq!(addr.to_string(), "mszm85TQkAhvAigVfraWicXNnCypp1TTbH");
    }

    #[test]
    fn test_address_btc_p2sh() {
        let addr = Address::from_str("3J98t1WpEZ73CNmQviecrnyiWrnqRhWNLy").unwrap();
        assert_eq!(addr.network.get_unit(), "BTC");
        assert_eq!(addr.to_string(), "3J98t1WpEZ73CNmQviecrnyiWrnqRhWNLy");
    }

    #[test]
    fn test_address_btc_p2sh_testnet() {
        let addr = Address::from_str("2NCuF1UQSRXn4WTCKQRGBdUhuFtTg1VpjtK").unwrap();
        assert_eq!(addr.network.get_unit(), "tBTC");
        assert_eq!(addr.to_string(), "2NCuF1UQSRXn4WTCKQRGBdUhuFtTg1VpjtK");
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
    fn test_address_btc_p2wpkh_testnet() {
        let addr = Address::from_str("tb1q6sjunnh9w9epn9z7he2dxmklgfg7x38yefmld7").unwrap();
        assert_eq!(addr.network.get_unit(), "tBTC");
        assert_eq!(
            addr.to_string(),
            "tb1q6sjunnh9w9epn9z7he2dxmklgfg7x38yefmld7"
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
    fn test_address_btc_p2tr_testnet() {
        let addr =
            Address::from_str("tb1p8wpt9v4frpf3tkn0srd97pksgsxc5hs52lafxwru9kgeephvs7rqlqt9zj")
                .unwrap();
        assert_eq!(addr.network.get_unit(), "tBTC");
        assert_eq!(
            addr.to_string(),
            "tb1p8wpt9v4frpf3tkn0srd97pksgsxc5hs52lafxwru9kgeephvs7rqlqt9zj"
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

    #[test]
    fn test_address_dash_p2wpkh() {
        let addr = Address::from_str("XdAUmwtig27HBG6WfYyHAzP8n6XC9jESEw").unwrap();
        assert_eq!(addr.network.get_unit(), "DASH");
        assert_eq!(addr.to_string(), "XdAUmwtig27HBG6WfYyHAzP8n6XC9jESEw");
    }

    #[test]
    fn test_address_dash_p2sh() {
        let addr = Address::from_str("7qd1hqQqZzMRaJA5drqkpEZL41s3JktRuZ").unwrap();
        assert_eq!(addr.network.get_unit(), "DASH");
        assert_eq!(addr.to_string(), "7qd1hqQqZzMRaJA5drqkpEZL41s3JktRuZ");
    }
}
