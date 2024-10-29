use core::{convert::TryInto, error::Error, fmt, str::FromStr};

use crate::zcash_address::AddressKind;
use crate::zcash_protocol::consensus::{NetworkConstants, NetworkType};
use crate::zcash_protocol::constants::{mainnet, regtest, testnet};
use alloc::string::String;
use alloc::vec::Vec;
use third_party::bech32::{self, Bech32, Bech32m, Checksum, Hrp};

use super::unified::{self, Encoding};
use super::ZcashAddress;

/// An error while attempting to parse a string as a Zcash address.
#[derive(Debug, PartialEq, Eq)]
pub enum ParseError {
    /// The string is an invalid encoding.
    InvalidEncoding,
    /// The string is not a Zcash address.
    NotZcash,
    /// Errors specific to unified addresses.
    Unified(unified::ParseError),
}

impl From<unified::ParseError> for ParseError {
    fn from(e: unified::ParseError) -> Self {
        match e {
            unified::ParseError::InvalidEncoding(_) => Self::InvalidEncoding,
            unified::ParseError::UnknownPrefix(_) => Self::NotZcash,
            _ => Self::Unified(e),
        }
    }
}

impl fmt::Display for ParseError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            ParseError::InvalidEncoding => write!(f, "Invalid encoding"),
            ParseError::NotZcash => write!(f, "Not a Zcash address"),
            ParseError::Unified(e) => e.fmt(f),
        }
    }
}

impl Error for ParseError {}

impl FromStr for ZcashAddress {
    type Err = ParseError;

    /// Attempts to parse the given string as a Zcash address.
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        // Remove leading and trailing whitespace, to handle copy-paste errors.
        let s = s.trim();

        // Try decoding as a unified address
        match unified::Address::decode(s) {
            Ok((net, data)) => {
                return Ok(ZcashAddress {
                    net,
                    kind: AddressKind::Unified(data),
                });
            }
            Err(unified::ParseError::NotUnified | unified::ParseError::UnknownPrefix(_)) => {
                // allow decoding to fall through to Sapling/TEX/Transparent
            }
            Err(e) => {
                return Err(ParseError::from(e));
            }
        }

        // Try decoding as a Sapling or TEX address (Bech32/Bech32m)
        if let Ok((hrp, data)) = bech32::decode(s) {
            // If we reached this point, the encoding is found to be valid Bech32 or Bech32m.
            // let data = Vec::<u8>::from_base32(&data).map_err(|_| ParseError::InvalidEncoding)?;

            let is_sapling = match hrp.to_lowercase().as_str() {
                mainnet::HRP_SAPLING_PAYMENT_ADDRESS
                | testnet::HRP_SAPLING_PAYMENT_ADDRESS
                | regtest::HRP_SAPLING_PAYMENT_ADDRESS => true,
                // We will not define new Bech32 address encodings.
                _ => false,
            };

            if is_sapling {
                let net = match hrp.to_lowercase().as_str() {
                    mainnet::HRP_SAPLING_PAYMENT_ADDRESS => NetworkType::Main,
                    testnet::HRP_SAPLING_PAYMENT_ADDRESS => NetworkType::Test,
                    regtest::HRP_SAPLING_PAYMENT_ADDRESS => NetworkType::Regtest,
                    // We will not define new Bech32 address encodings.
                    _ => {
                        return Err(ParseError::NotZcash);
                    }
                };

                return data[..]
                    .try_into()
                    .map(AddressKind::Sapling)
                    .map_err(|_| ParseError::InvalidEncoding)
                    .map(|kind| ZcashAddress { net, kind });
            } else {
                let net = match hrp.to_lowercase().as_str() {
                    mainnet::HRP_TEX_ADDRESS => NetworkType::Main,
                    testnet::HRP_TEX_ADDRESS => NetworkType::Test,
                    regtest::HRP_TEX_ADDRESS => NetworkType::Regtest,
                    // Not recognized as a Zcash address type
                    _ => {
                        return Err(ParseError::NotZcash);
                    }
                };

                return data[..]
                    .try_into()
                    .map(AddressKind::Tex)
                    .map_err(|_| ParseError::InvalidEncoding)
                    .map(|kind| ZcashAddress { net, kind });
            }
        }

        // The rest use Base58Check.
        if let Ok(decoded) = bs58::decode(s).with_check(None).into_vec() {
            if decoded.len() >= 2 {
                let (prefix, net) = match decoded[..2].try_into().unwrap() {
                    prefix @ (mainnet::B58_PUBKEY_ADDRESS_PREFIX
                    | mainnet::B58_SCRIPT_ADDRESS_PREFIX
                    | mainnet::B58_SPROUT_ADDRESS_PREFIX) => (prefix, NetworkType::Main),
                    prefix @ (testnet::B58_PUBKEY_ADDRESS_PREFIX
                    | testnet::B58_SCRIPT_ADDRESS_PREFIX
                    | testnet::B58_SPROUT_ADDRESS_PREFIX) => (prefix, NetworkType::Test),
                    // We will not define new Base58Check address encodings.
                    _ => return Err(ParseError::NotZcash),
                };

                return match prefix {
                    mainnet::B58_SPROUT_ADDRESS_PREFIX | testnet::B58_SPROUT_ADDRESS_PREFIX => {
                        decoded[2..].try_into().map(AddressKind::Sprout)
                    }
                    mainnet::B58_PUBKEY_ADDRESS_PREFIX | testnet::B58_PUBKEY_ADDRESS_PREFIX => {
                        decoded[2..].try_into().map(AddressKind::P2pkh)
                    }
                    mainnet::B58_SCRIPT_ADDRESS_PREFIX | testnet::B58_SCRIPT_ADDRESS_PREFIX => {
                        decoded[2..].try_into().map(AddressKind::P2sh)
                    }
                    _ => unreachable!(),
                }
                .map_err(|_| ParseError::InvalidEncoding)
                .map(|kind| ZcashAddress { kind, net });
            }
        };

        // If it's not valid Bech32, Bech32m, or Base58Check, it's not a Zcash address.
        Err(ParseError::NotZcash)
    }
}

fn encode_bech32<ck: Checksum>(hrp: &str, data: &[u8]) -> String {
    bech32::encode::<ck>(Hrp::parse_unchecked(hrp), data).expect("hrp is invalid")
}

fn encode_b58(prefix: [u8; 2], data: &[u8]) -> String {
    let mut bytes = Vec::with_capacity(2 + data.len());
    bytes.extend_from_slice(&prefix);
    bytes.extend_from_slice(data);
    bs58::encode(bytes).with_check().into_string()
}

impl fmt::Display for ZcashAddress {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let encoded = match &self.kind {
            AddressKind::Sprout(data) => encode_b58(self.net.b58_sprout_address_prefix(), data),
            AddressKind::Sapling(data) => {
                encode_bech32::<Bech32>(self.net.hrp_sapling_payment_address(), data)
            }
            AddressKind::Unified(addr) => addr.encode(&self.net),
            AddressKind::P2pkh(data) => encode_b58(self.net.b58_pubkey_address_prefix(), data),
            AddressKind::P2sh(data) => encode_b58(self.net.b58_script_address_prefix(), data),
            AddressKind::Tex(data) => encode_bech32::<Bech32m>(self.net.hrp_tex_address(), data),
        };
        write!(f, "{}", encoded)
    }
}
