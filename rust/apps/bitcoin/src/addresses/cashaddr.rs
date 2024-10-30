extern crate alloc;

use crate::addresses::address::Address;
use crate::addresses::constants::PUBKEY_ADDRESS_PREFIX_BCH;
use crate::errors::BitcoinError;
use crate::errors::Result;
use crate::network::Network;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use bitcoin::address::Payload;
use bitcoin::PubkeyHash;
use bitcoin_hashes::Hash;
use core::{fmt, str};

// Prefixes
const DASH_PREFIX: &str = "bitcoincash";

const CHARSET: &[u8; 32] = b"qpzry9x8gf2tvdw0s3jn54khce6mua7l";
const BASE58_CHARS: &[u8] = b"123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

#[rustfmt::skip]
const BASE58_DIGITS: [Option<u8>; 128] = [
    None, None, None, None, None, None, None, None,     // 0-7
    None, None, None, None, None, None, None, None,     // 8-15
    None, None, None, None, None, None, None, None,     // 16-23
    None, None, None, None, None, None, None, None,     // 24-31
    None, None, None, None, None, None, None, None,     // 32-39
    None, None, None, None, None, None, None, None,     // 40-47
    None, Some(0), Some(1), Some(2), Some(3), Some(4), Some(5), Some(6),  // 48-55
    Some(7), Some(8), None, None, None, None, None, None,     // 56-63
    None, Some(9), Some(10), Some(11), Some(12), Some(13), Some(14), Some(15), // 64-71
    Some(16), None, Some(17), Some(18), Some(19), Some(20), Some(21), None,     // 72-79
    Some(22), Some(23), Some(24), Some(25), Some(26), Some(27), Some(28), Some(29), // 80-87
    Some(30), Some(31), Some(32), None, None, None, None, None,     // 88-95
    None, Some(33), Some(34), Some(35), Some(36), Some(37), Some(38), Some(39), // 96-103
    Some(40), Some(41), Some(42), Some(43), None, Some(44), Some(45), Some(46), // 104-111
    Some(47), Some(48), Some(49), Some(50), Some(51), Some(52), Some(53), Some(54), // 112-119
    Some(55), Some(56), Some(57), None, None, None, None, None,     // 120-127
];

#[rustfmt::skip]
const CHARSET_REV: [Option<u8>; 128] = [
    None,     None,     None,     None,     None,     None,     None,     None,
    None,     None,     None,     None,     None,     None,     None,     None,
    None,     None,     None,     None,     None,     None,     None,     None,
    None,     None,     None,     None,     None,     None,     None,     None,
    None,     None,     None,     None,     None,     None,     None,     None,
    None,     None,     None,     None,     None,     None,     None,     None,
    Some(15), None,     Some(10), Some(17), Some(21), Some(20), Some(26), Some(30),
    Some(7),  Some(5),  None,     None,     None,     None,     None,     None,
    None,     Some(29), None,     Some(24), Some(13), Some(25), Some(9),  Some(8),
    Some(23), None,     Some(18), Some(22), Some(31), Some(27), Some(19), None,
    Some(1),  Some(0),  Some(3),  Some(16), Some(11), Some(28), Some(12), Some(14),
    Some(6),  Some(4),  Some(2),  None,     None,     None,     None,     None,
    None,     Some(29),  None,    Some(24), Some(13), Some(25), Some(9),  Some(8),
    Some(23), None,     Some(18), Some(22), Some(31), Some(27), Some(19), None,
    Some(1),  Some(0),  Some(3),  Some(16), Some(11), Some(28), Some(12), Some(14),
    Some(6),  Some(4),  Some(2),  None,     None,     None,     None,     None,
];

// Version byte flags
#[allow(dead_code)]
mod version_byte_flags {
    pub const TYPE_MASK: u8 = 0x78;
    pub const TYPE_P2PKH: u8 = 0x00;
    pub const TYPE_P2SH: u8 = 0x08;

    pub const SIZE_MASK: u8 = 0x07;
    pub const SIZE_160: u8 = 0x00;
    pub const SIZE_192: u8 = 0x01;
    pub const SIZE_224: u8 = 0x02;
    pub const SIZE_256: u8 = 0x03;
    pub const SIZE_320: u8 = 0x04;
    pub const SIZE_384: u8 = 0x05;
    pub const SIZE_448: u8 = 0x06;
    pub const SIZE_512: u8 = 0x07;
}

// https://github.com/Bitcoin-ABC/bitcoin-abc/blob/2804a49bfc0764ba02ce2999809c52b3b9bb501e/src/cashaddr.cpp#L42
fn polymod(v: &[u8]) -> u64 {
    let mut c: u64 = 1;
    for d in v.iter() {
        let c0: u8 = (c >> 35) as u8;
        c = ((c & 0x0007_ffff_ffff) << 5) ^ u64::from(*d);
        if c0 & 0x01 != 0 {
            c ^= 0x0098_f2bc_8e61;
        }
        if c0 & 0x02 != 0 {
            c ^= 0x0079_b76d_99e2;
        }
        if c0 & 0x04 != 0 {
            c ^= 0x00f3_3e5f_b3c4;
        }
        if c0 & 0x08 != 0 {
            c ^= 0x00ae_2eab_e2a8;
        }
        if c0 & 0x10 != 0 {
            c ^= 0x001e_4f43_e470;
        }
    }
    c ^ 1
}

// Expand the addresses prefix for the checksum operation.
fn expand_prefix(prefix: &str) -> Vec<u8> {
    let mut ret: Vec<u8> = prefix.chars().map(|c| (c as u8) & 0x1f).collect();
    ret.push(0);
    ret
}

fn convert_bits(data: &[u8], inbits: u8, outbits: u8, pad: bool) -> Vec<u8> {
    assert!(inbits <= 8 && outbits <= 8);
    let num_bytes = (data.len() * inbits as usize + outbits as usize - 1) / outbits as usize;
    let mut ret = Vec::with_capacity(num_bytes);
    let mut acc: u16 = 0; // accumulator of bits
    let mut num: u8 = 0; // num bits in acc
    let groupmask = (1 << outbits) - 1;
    for d in data.iter() {
        // We push each input chunk into a 16-bit accumulator
        acc = (acc << inbits) | u16::from(*d);
        num += inbits;
        // Then we extract all the output groups we can
        while num > outbits {
            ret.push((acc >> (num - outbits)) as u8);
            acc &= !(groupmask << (num - outbits));
            num -= outbits;
        }
    }
    if pad {
        // If there's some bits left, pad and add it
        if num > 0 {
            ret.push((acc << (outbits - num)) as u8);
        }
    } else {
        // If there's some bits left, figure out if we need to remove padding and add it
        let padding = (data.len() * inbits as usize) % outbits as usize;
        if num as usize > padding {
            ret.push((acc >> padding) as u8);
        }
    }
    ret
}

fn from_base58_str(data: &str) -> Result<Vec<u8>> {
    // 11/15 is just over log_256(58)
    let mut scratch = vec![0u8; 1 + data.len() * 11 / 15];
    // Build in base 256
    for d58 in data.bytes() {
        // Compute "X = X * 58 + next_digit" in base 256
        if d58 as usize > BASE58_DIGITS.len() {
            return Err(BitcoinError::AddressError(format!("invalid char")));
        }
        let mut carry = match BASE58_DIGITS[d58 as usize] {
            Some(d58) => u32::from(d58),
            None => {
                return Err(BitcoinError::AddressError(format!("invalid char")));
            }
        };
        for d256 in scratch.iter_mut().rev() {
            carry += u32::from(*d256) * 58;
            *d256 = carry as u8;
            carry /= 256;
        }
        assert_eq!(carry, 0);
    }

    // Copy leading zeroes directly
    let mut ret: Vec<u8> = data
        .bytes()
        .take_while(|&x| x == BASE58_CHARS[0])
        .map(|_| 0)
        .collect();
    // Copy rest of string
    ret.extend(scratch.into_iter().skip_while(|&x| x == 0));
    Ok(ret)
}

pub struct CashAddrCodec;

pub struct Base58Codec;

impl Base58Codec {
    pub fn decode(addr_str: &str) -> Result<Vec<u8>> {
        // Convert from base58
        let raw = from_base58_str(addr_str)?;
        let length = raw.len();
        if length != 25 {
            return Err(BitcoinError::AddressError(format!(
                "BCH addresses decode error: invalid length {:?}",
                length
            )));
        }
        let version_byte = raw[0];
        if version_byte != PUBKEY_ADDRESS_PREFIX_BCH {
            return Err(BitcoinError::AddressError(format!(
                "invalid version {:?}",
                version_byte
            )));
        };
        // Verify checksum
        let payload = &raw[0..raw.len() - 4];
        let checksum_actual = &raw[raw.len() - 4..];
        let checksum_expected = &bitcoin_hashes::sha256d::Hash::hash(payload)[0..4];
        if checksum_expected != checksum_actual {
            return Err(BitcoinError::AddressError(format!("checksum failed")));
        }

        let body = payload[1..].to_vec();
        Ok(body)
    }
}

impl CashAddrCodec {
    pub fn encode_to_fmt(fmt: &mut fmt::Formatter, raw: Vec<u8>) -> fmt::Result {
        let encoded = CashAddrCodec::encode(raw).map_err(|_| fmt::Error)?;
        write!(fmt, "{}", encoded)
    }

    pub fn encode(raw: Vec<u8>) -> Result<String> {
        // Calculate version byte
        let hash_flag = version_byte_flags::TYPE_P2PKH;
        let length = raw.len();
        let version_byte = match length {
            20 => version_byte_flags::SIZE_160,
            24 => version_byte_flags::SIZE_192,
            28 => version_byte_flags::SIZE_224,
            32 => version_byte_flags::SIZE_256,
            40 => version_byte_flags::SIZE_320,
            48 => version_byte_flags::SIZE_384,
            56 => version_byte_flags::SIZE_448,
            64 => version_byte_flags::SIZE_512,
            _ => {
                return Err(BitcoinError::AddressError(
                    "invalid version bytes".to_string(),
                ))
            }
        } | hash_flag;

        // Get prefix
        let prefix = DASH_PREFIX;

        // Convert payload to 5 bit array
        let mut payload = Vec::with_capacity(1 + raw.len());
        payload.push(version_byte);
        payload.extend(raw);
        let payload_5_bits = convert_bits(&payload, 8, 5, true);

        // Construct payload string using CHARSET
        let mut payload_str: String = payload_5_bits
            .iter()
            .map(|b| CHARSET[*b as usize] as char)
            .collect();

        // Create checksum
        let expanded_prefix = expand_prefix(prefix);
        let checksum_input = [&expanded_prefix[..], &payload_5_bits, &[0; 8][..]].concat();
        let checksum = polymod(&checksum_input);

        // Convert checksum to string
        let checksum_str: String = (0..8)
            .rev()
            .map(|i| CHARSET[((checksum >> (i * 5)) & 31) as usize] as char)
            .collect();

        payload_str.push_str(&checksum_str);
        Ok(payload_str)
    }
    pub fn decode(addr_str: &str) -> Result<Address> {
        // Do some sanity checks on the string
        let prefix = DASH_PREFIX;
        let mut payload_chars = addr_str.chars();
        if let Some(first_char) = payload_chars.next() {
            if first_char.is_lowercase() {
                if payload_chars.any(|c| c.is_uppercase()) {
                    return Err(BitcoinError::AddressError(format!("mixed case")));
                }
            } else if payload_chars.any(|c| c.is_lowercase()) {
                return Err(BitcoinError::AddressError(format!("mixed case")));
            }
        } else {
            return Err(BitcoinError::AddressError(format!("invalid length")));
        }

        // Decode payload to 5 bit array
        let payload_chars = addr_str.chars(); // Reintialize iterator here
        let payload_5_bits: Result<Vec<u8>> = payload_chars
            .map(|c| {
                let i = c as usize;
                if let Some(Some(d)) = CHARSET_REV.get(i) {
                    Ok(*d as u8)
                } else {
                    return Err(BitcoinError::AddressError(format!("invalid char")));
                }
            })
            .collect();
        let payload_5_bits = payload_5_bits?;

        // Verify the checksum
        let checksum = polymod(&[&expand_prefix(prefix), &payload_5_bits[..]].concat());
        if checksum != 0 {
            return Err(BitcoinError::AddressError(format!("invalid checksum")));
        }

        // Convert from 5 bit array to byte array
        let len_5_bit = payload_5_bits.len();
        let payload = convert_bits(&payload_5_bits[..(len_5_bit - 8)], 5, 8, false);

        // Verify the version byte
        let version = payload[0];

        // Check length
        let body = &payload[1..];
        let body_len = body.len();
        let version_size = version & version_byte_flags::SIZE_MASK;
        if (version_size == version_byte_flags::SIZE_160 && body_len != 20)
            || (version_size == version_byte_flags::SIZE_192 && body_len != 24)
            || (version_size == version_byte_flags::SIZE_224 && body_len != 28)
            || (version_size == version_byte_flags::SIZE_256 && body_len != 32)
            || (version_size == version_byte_flags::SIZE_320 && body_len != 40)
            || (version_size == version_byte_flags::SIZE_384 && body_len != 48)
            || (version_size == version_byte_flags::SIZE_448 && body_len != 56)
            || (version_size == version_byte_flags::SIZE_512 && body_len != 64)
        {
            return Err(BitcoinError::AddressError(format!(
                "invalid length {:?}",
                body_len
            )));
        }

        // Extract the hash type and return
        let version_type = version & version_byte_flags::TYPE_MASK;
        if version_type != version_byte_flags::TYPE_P2PKH {
            return Err(BitcoinError::AddressError(format!(
                "invalid version {:?}",
                version_type
            )));
        };
        let publickey_hash = PubkeyHash::from_slice(&body.to_vec())
            .map_err(|_| BitcoinError::AddressError(format!("invalid public key hash")))?;
        Ok(Address {
            payload: Payload::PubkeyHash(publickey_hash),
            network: Network::BitcoinCash,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use hex::ToHex;

    #[test]
    fn test_decode_cash_addr() {
        let addr_str = "qz65ywjm92m27wshfnew2w3us5vsgxqkxc55t9lqcw";
        let address = CashAddrCodec::decode(addr_str).unwrap();
        assert_eq!(
            address.payload.script_pubkey().encode_hex::<String>(),
            "76a914b5423a5b2ab6af3a174cf2e53a3c85190418163688ac"
        );
    }
}
