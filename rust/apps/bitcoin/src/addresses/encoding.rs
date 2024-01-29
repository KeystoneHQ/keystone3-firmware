use core::fmt;
use third_party::bitcoin::address::Payload;
use third_party::bitcoin::base58;
use third_party::bitcoin::bech32;

use crate::addresses::cashaddr::{Base58Codec, CashAddrCodec};

pub struct BTCAddressEncoding<'a> {
    pub payload: &'a Payload,
    pub p2pkh_prefix: u8,
    pub p2sh_prefix: u8,
    pub bech32_hrp: &'a str,
}

pub struct LTCAddressEncoding<'a> {
    pub payload: &'a Payload,
    pub p2pkh_prefix: u8,
    pub p2sh_prefix: u8,
    pub bech32_hrp: &'a str,
}

pub struct DASHAddressEncoding<'a> {
    pub payload: &'a Payload,
    pub p2pkh_prefix: u8,
}

pub struct BCHAddressEncoding<'a> {
    pub payload: &'a Payload,
    pub p2pkh_prefix: u8,
}

struct UpperWriter<W: fmt::Write>(W);

impl<W: fmt::Write> fmt::Write for UpperWriter<W> {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        for c in s.chars() {
            self.0.write_char(c.to_ascii_uppercase())?;
        }
        Ok(())
    }
}

impl<'a> fmt::Display for BTCAddressEncoding<'a> {
    fn fmt(&self, fmt: &mut fmt::Formatter) -> fmt::Result {
        match self.payload {
            Payload::PubkeyHash(hash) => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2pkh_prefix;
                prefixed[1..].copy_from_slice(&hash[..]);
                base58::encode_check_to_fmt(fmt, &prefixed[..])
            }
            Payload::ScriptHash(hash) => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2sh_prefix;
                prefixed[1..].copy_from_slice(&hash[..]);
                base58::encode_check_to_fmt(fmt, &prefixed[..])
            }
            Payload::WitnessProgram(witness) => {
                let mut upper_writer;
                let writer = if fmt.alternate() {
                    upper_writer = UpperWriter(fmt);
                    &mut upper_writer as &mut dyn fmt::Write
                } else {
                    fmt as &mut dyn fmt::Write
                };
                let mut bech32_writer = bech32::Bech32Writer::new(
                    self.bech32_hrp,
                    witness.version().bech32_variant(),
                    writer,
                )?;
                bech32::WriteBase32::write_u5(&mut bech32_writer, (witness.version()).into())?;
                bech32::ToBase32::write_base32(&witness.program(), &mut bech32_writer)
            }
            _ => {
                write!(fmt, "invalid payload")
            }
        }
    }
}

impl<'a> fmt::Display for BCHAddressEncoding<'a> {
    fn fmt(&self, fmt: &mut fmt::Formatter) -> fmt::Result {
        match self.payload {
            Payload::PubkeyHash(hash) => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2pkh_prefix;
                prefixed[1..].copy_from_slice(&hash[..]);
                let base58_addr = base58::encode_check(&prefixed[..]);
                let decoded = Base58Codec::decode(base58_addr.as_str());
                match decoded {
                    Ok(address) => CashAddrCodec::encode_to_fmt(fmt, address),
                    Err(_) => write!(fmt, "invalid addresses"),
                }
            }
            _ => {
                write!(fmt, "invalid payload")
            }
        }
    }
}

impl<'a> fmt::Display for LTCAddressEncoding<'a> {
    fn fmt(&self, fmt: &mut fmt::Formatter) -> fmt::Result {
        match self.payload {
            Payload::PubkeyHash(hash) => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2pkh_prefix;
                prefixed[1..].copy_from_slice(&hash[..]);
                base58::encode_check_to_fmt(fmt, &prefixed[..])
            }
            Payload::ScriptHash(hash) => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2sh_prefix;
                prefixed[1..].copy_from_slice(&hash[..]);
                base58::encode_check_to_fmt(fmt, &prefixed[..])
            }
            Payload::WitnessProgram(witness) => {
                let mut upper_writer;
                let writer = if fmt.alternate() {
                    upper_writer = UpperWriter(fmt);
                    &mut upper_writer as &mut dyn fmt::Write
                } else {
                    fmt as &mut dyn fmt::Write
                };
                let mut bech32_writer = bech32::Bech32Writer::new(
                    self.bech32_hrp,
                    witness.version().bech32_variant(),
                    writer,
                )?;
                bech32::WriteBase32::write_u5(&mut bech32_writer, (witness.version()).into())?;
                bech32::ToBase32::write_base32(&witness.program(), &mut bech32_writer)
            }
            _ => {
                write!(fmt, "invalid payload")
            }
        }
    }
}

impl<'a> fmt::Display for DASHAddressEncoding<'a> {
    fn fmt(&self, fmt: &mut fmt::Formatter) -> fmt::Result {
        match self.payload {
            Payload::PubkeyHash(hash) => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2pkh_prefix;
                prefixed[1..].copy_from_slice(&hash[..]);
                base58::encode_check_to_fmt(fmt, &prefixed[..])
            }
            _ => {
                write!(fmt, "invalid payload")
            }
        }
    }
}
