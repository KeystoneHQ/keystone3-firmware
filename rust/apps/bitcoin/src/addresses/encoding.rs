use bech32::Hrp;
use bitcoin::address::AddressData as Payload;
use bitcoin::base58;
use core::fmt;

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
    pub p2sh_prefix: u8,
}

pub struct BCHAddressEncoding<'a> {
    pub payload: &'a Payload,
    pub p2pkh_prefix: u8,
}

pub struct ZECAddressEncoding<'a> {
    pub payload: &'a Payload,
    pub p2pkh_prefix_byte0: u8,
    pub p2pkh_prefix_byte1: u8,
}

pub struct DOGEAddressEncoding<'a> {
    pub payload: &'a Payload,
    pub p2pkh_prefix: u8,
    pub p2sh_prefix: u8,
}

#[allow(unused)]
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
            Payload::P2pkh { pubkey_hash } => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2pkh_prefix;
                prefixed[1..].copy_from_slice(&pubkey_hash[..]);
                base58::encode_check_to_fmt(fmt, &prefixed[..])
            }
            Payload::P2sh { script_hash } => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2sh_prefix;
                prefixed[1..].copy_from_slice(&script_hash[..]);
                base58::encode_check_to_fmt(fmt, &prefixed[..])
            }
            Payload::Segwit { witness_program } => {
                let hrp = Hrp::parse_unchecked(self.bech32_hrp);
                let version = witness_program.version().to_fe();
                let program = witness_program.program().as_bytes();

                if fmt.alternate() {
                    bech32::segwit::encode_upper_to_fmt_unchecked(fmt, hrp, version, program)
                } else {
                    bech32::segwit::encode_lower_to_fmt_unchecked(fmt, hrp, version, program)
                }
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
            Payload::P2pkh { pubkey_hash } => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2pkh_prefix;
                prefixed[1..].copy_from_slice(&pubkey_hash[..]);
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
            Payload::P2pkh { pubkey_hash } => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2pkh_prefix;
                prefixed[1..].copy_from_slice(&pubkey_hash[..]);
                base58::encode_check_to_fmt(fmt, &prefixed[..])
            }
            Payload::P2sh { script_hash } => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2sh_prefix;
                prefixed[1..].copy_from_slice(&script_hash[..]);
                base58::encode_check_to_fmt(fmt, &prefixed[..])
            }
            Payload::Segwit { witness_program } => {
                let hrp = Hrp::parse_unchecked(self.bech32_hrp);
                let version = witness_program.version().to_fe();
                let program = witness_program.program().as_bytes();

                if fmt.alternate() {
                    bech32::segwit::encode_upper_to_fmt_unchecked(fmt, hrp, version, program)
                } else {
                    bech32::segwit::encode_lower_to_fmt_unchecked(fmt, hrp, version, program)
                }
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
            Payload::P2pkh { pubkey_hash } => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2pkh_prefix;
                prefixed[1..].copy_from_slice(&pubkey_hash[..]);
                base58::encode_check_to_fmt(fmt, &prefixed[..])
            }
            Payload::P2sh { script_hash } => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2sh_prefix;
                prefixed[1..].copy_from_slice(&script_hash[..]);
                base58::encode_check_to_fmt(fmt, &prefixed[..])
            }
            _ => {
                write!(fmt, "invalid payload")
            }
        }
    }
}

impl<'a> fmt::Display for ZECAddressEncoding<'a> {
    fn fmt(&self, fmt: &mut fmt::Formatter) -> fmt::Result {
        match self.payload {
            Payload::P2pkh { pubkey_hash } => {
                let mut prefixed = [0; 22];
                prefixed[0] = self.p2pkh_prefix_byte0;
                prefixed[1] = self.p2pkh_prefix_byte1;
                prefixed[2..].copy_from_slice(&pubkey_hash[..]);
                base58::encode_check_to_fmt(fmt, &prefixed[..])
            }
            _ => {
                write!(fmt, "invalid payload")
            }
        }
    }
}

impl<'a> fmt::Display for DOGEAddressEncoding<'a> {
    fn fmt(&self, fmt: &mut fmt::Formatter) -> fmt::Result {
        match self.payload {
            Payload::P2pkh { pubkey_hash } => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2pkh_prefix;
                prefixed[1..].copy_from_slice(&pubkey_hash[..]);
                base58::encode_check_to_fmt(fmt, &prefixed[..])
            }
            Payload::P2sh { script_hash } => {
                let mut prefixed = [0; 21];
                prefixed[0] = self.p2sh_prefix;
                prefixed[1..].copy_from_slice(&script_hash[..]);
                base58::encode_check_to_fmt(fmt, &prefixed[..])
            }
            _ => {
                write!(fmt, "invalid payload")
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;
    use bitcoin::hashes::{hash160, Hash};
    use bitcoin::{PubkeyHash, ScriptHash, WitnessProgram, WitnessVersion};

    fn p2pkh_payload(hex_path: &str) -> Payload {
        let bytes = hex::decode(hex_path).expect("Invalid hex");
        let hash = hash160::Hash::from_slice(&bytes).expect("Invalid hash");
        Payload::P2pkh {
            pubkey_hash: PubkeyHash::from(hash),
        }
    }

    fn p2sh_payload(hex_path: &str) -> Payload {
        let bytes = hex::decode(hex_path).expect("Invalid hex");
        let hash = hash160::Hash::from_slice(&bytes).expect("Invalid hash");
        Payload::P2sh {
            script_hash: ScriptHash::from(hash),
        }
    }

    fn segwit_payload(version: WitnessVersion, hex_path: &str) -> Payload {
        let bytes = hex::decode(hex_path).expect("Invalid hex");
        let witness_program =
            WitnessProgram::new(version, &bytes).expect("Invalid witness program");
        Payload::Segwit { witness_program }
    }

    #[test]
    fn test_btc_encoding() {
        // BTC P2PKH: 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa
        let p2pkh = p2pkh_payload("62e907b15cbf27d5425399ebf6f0fb50ebb88f18");
        let btc_encoding = BTCAddressEncoding {
            payload: &p2pkh,
            p2pkh_prefix: 0x00,
            p2sh_prefix: 0x05,
            bech32_hrp: "bc",
        };
        assert_eq!(
            btc_encoding.to_string(),
            "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa"
        );

        // BTC P2SH: 3J98t1WpEZ73CNmQviecrnyiWrnqRhWNLy
        let p2sh = p2sh_payload("b472a266d0bd89c13706a4132cc6a64f852e5519");
        let btc_encoding = BTCAddressEncoding {
            payload: &p2sh,
            p2pkh_prefix: 0x00,
            p2sh_prefix: 0x05,
            bech32_hrp: "bc",
        };
        // Generated address from the given hash
        assert_eq!(
            btc_encoding.to_string(),
            "3J98t1WpEZ73CNmQvieacj3En2FD1ds8xP"
        );

        // BTC Segwit: bc1qar0srrr7xfkvy5l643lydnw9re59gtzzwf5mdq
        let segwit = segwit_payload(
            WitnessVersion::V0,
            "751e76e8199196d454941c45d1b3a323f1433bd6",
        );
        let btc_encoding = BTCAddressEncoding {
            payload: &segwit,
            p2pkh_prefix: 0x00,
            p2sh_prefix: 0x05,
            bech32_hrp: "bc",
        };
        // Lowercase by default (Display)
        assert_eq!(
            btc_encoding.to_string(),
            "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4"
        );
        // Check alternate (uppercase)
        assert_eq!(
            format!("{:#}", btc_encoding),
            "BC1QW508D6QEJXTDG4Y5R3ZARVARY0C5XW7KV8F3T4"
        );
    }

    #[test]
    fn test_ltc_encoding() {
        let p2pkh = p2pkh_payload("62e907b15cbf27d5425399ebf6f0fb50ebb88f18");
        let ltc_encoding = LTCAddressEncoding {
            payload: &p2pkh,
            p2pkh_prefix: 0x30,
            p2sh_prefix: 0x32,
            bech32_hrp: "ltc",
        };
        assert_eq!(
            ltc_encoding.to_string(),
            "LUEweDxDA4WhvWiNXXSxjM9CYzHPJv4QQF"
        );
    }

    #[test]
    fn test_doge_encoding() {
        let p2pkh = p2pkh_payload("62e907b15cbf27d5425399ebf6f0fb50ebb88f18");
        let doge_encoding = DOGEAddressEncoding {
            payload: &p2pkh,
            p2pkh_prefix: 0x1E,
            p2sh_prefix: 0x16,
        };
        assert_eq!(
            doge_encoding.to_string(),
            "DEA5vGb2NpAwCiCp5yTE16F3DueQUVivQp"
        );
    }

    #[test]
    fn test_zec_encoding() {
        let p2pkh = p2pkh_payload("62e907b15cbf27d5425399ebf6f0fb50ebb88f18");
        let zec_encoding = ZECAddressEncoding {
            payload: &p2pkh,
            p2pkh_prefix_byte0: 0x1C,
            p2pkh_prefix_byte1: 0xB8,
        };
        assert_eq!(
            zec_encoding.to_string(),
            "t1StbPM4X3j4FGM57HpGnb9BMbS7C1nFW1r"
        );
    }

    #[test]
    fn test_bch_encoding() {
        let p2pkh = p2pkh_payload("62e907b15cbf27d5425399ebf6f0fb50ebb88f18");
        let bch_encoding = BCHAddressEncoding {
            payload: &p2pkh,
            p2pkh_prefix: 0x00,
        };
        assert_eq!(
            bch_encoding.to_string(),
            "qp3wjpa3tjlj042z2wv7hahsldgwhwy0rq9sywjpyy"
        );
    }
}
