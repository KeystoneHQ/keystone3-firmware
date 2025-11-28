use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::{convert::TryFrom, fmt, str::FromStr};

use hex::FromHex;
use serde::{de::Error as _, Deserialize, Deserializer, Serialize, Serializer};

/// A struct that represents an account address.
#[derive(Ord, PartialOrd, Eq, PartialEq, Hash, Clone, Copy)]
pub struct AccountAddress([u8; AccountAddress::LENGTH]);

impl AccountAddress {
    pub const fn new(address: [u8; Self::LENGTH]) -> Self {
        Self(address)
    }

    /// The number of bytes in an address.
    /// Default to 16 bytes, can be set to 20 bytes with address20 feature.
    pub const LENGTH: usize = 32;

    /// Hex address: 0x0
    pub const ZERO: Self = Self([0u8; Self::LENGTH]);

    /// Hex address: 0x1
    pub const ONE: Self = Self::get_hex_address_one();

    const fn get_hex_address_one() -> Self {
        let mut addr = [0u8; AccountAddress::LENGTH];
        addr[AccountAddress::LENGTH - 1] = 1u8;
        Self(addr)
    }

    pub fn short_str_lossless(&self) -> String {
        let hex_str = hex::encode(self.0).trim_start_matches('0').to_string();
        if hex_str.is_empty() {
            "0".to_string()
        } else {
            hex_str
        }
    }

    pub fn to_vec(&self) -> Vec<u8> {
        self.0.to_vec()
    }

    pub fn into_bytes(self) -> [u8; Self::LENGTH] {
        self.0
    }

    pub fn from_hex_literal(literal: &str) -> crate::errors::Result<Self> {
        if !literal.starts_with("0x") {
            return Err(crate::errors::AptosError::InvalidData(format!(
                "{literal} not start with 0x"
            )));
        }

        let hex_len = literal.len() - 2;

        // If the string is too short, pad it
        if hex_len < Self::LENGTH * 2 {
            let mut hex_str = String::with_capacity(Self::LENGTH * 2);
            for _ in 0..Self::LENGTH * 2 - hex_len {
                hex_str.push('0');
            }
            hex_str.push_str(&literal[2..]);
            AccountAddress::from_hex(hex_str)
        } else {
            AccountAddress::from_hex(&literal[2..])
        }
    }

    pub fn to_hex_literal(&self) -> String {
        format!("0x{}", self.short_str_lossless())
    }

    pub fn from_hex<T: AsRef<[u8]>>(hex: T) -> crate::errors::Result<Self> {
        <[u8; Self::LENGTH]>::from_hex(hex)
            .map_err(|e| crate::errors::AptosError::ParseTxError(e.to_string()))
            .map(Self)
    }

    pub fn to_hex(&self) -> String {
        format!("{self:x}")
    }

    pub fn from_bytes<T: AsRef<[u8]>>(bytes: T) -> crate::errors::Result<Self> {
        <[u8; Self::LENGTH]>::try_from(bytes.as_ref())
            .map_err(|_e| crate::errors::AptosError::InvalidData("from_bytes error".to_string()))
            .map(Self)
    }
}

impl AsRef<[u8]> for AccountAddress {
    fn as_ref(&self) -> &[u8] {
        &self.0
    }
}

impl core::ops::Deref for AccountAddress {
    type Target = [u8; Self::LENGTH];

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl fmt::Display for AccountAddress {
    fn fmt(&self, f: &mut fmt::Formatter) -> core::fmt::Result {
        write!(f, "{self:x}")
    }
}

impl fmt::Debug for AccountAddress {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{self:x}")
    }
}

impl fmt::LowerHex for AccountAddress {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if f.alternate() {
            write!(f, "0x")?;
        }

        for byte in &self.0 {
            write!(f, "{byte:02x}")?;
        }

        Ok(())
    }
}

impl fmt::UpperHex for AccountAddress {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if f.alternate() {
            write!(f, "0x")?;
        }

        for byte in &self.0 {
            write!(f, "{byte:02X}")?;
        }

        Ok(())
    }
}

impl From<[u8; AccountAddress::LENGTH]> for AccountAddress {
    fn from(bytes: [u8; AccountAddress::LENGTH]) -> Self {
        Self::new(bytes)
    }
}

impl TryFrom<&[u8]> for AccountAddress {
    type Error = crate::errors::AptosError;

    /// Tries to convert the provided byte array into Address.
    fn try_from(bytes: &[u8]) -> crate::errors::Result<AccountAddress> {
        Self::from_bytes(bytes)
    }
}

impl TryFrom<Vec<u8>> for AccountAddress {
    type Error = crate::errors::AptosError;

    /// Tries to convert the provided byte buffer into Address.
    fn try_from(bytes: Vec<u8>) -> crate::errors::Result<AccountAddress> {
        Self::from_bytes(bytes)
    }
}

impl From<AccountAddress> for Vec<u8> {
    fn from(addr: AccountAddress) -> Vec<u8> {
        addr.0.to_vec()
    }
}

impl From<&AccountAddress> for Vec<u8> {
    fn from(addr: &AccountAddress) -> Vec<u8> {
        addr.0.to_vec()
    }
}

impl From<AccountAddress> for [u8; AccountAddress::LENGTH] {
    fn from(addr: AccountAddress) -> Self {
        addr.0
    }
}

impl From<&AccountAddress> for [u8; AccountAddress::LENGTH] {
    fn from(addr: &AccountAddress) -> Self {
        addr.0
    }
}

impl From<&AccountAddress> for String {
    fn from(addr: &AccountAddress) -> String {
        hex::encode(addr.as_ref())
    }
}

impl TryFrom<String> for AccountAddress {
    type Error = crate::errors::AptosError;

    fn try_from(s: String) -> crate::errors::Result<AccountAddress> {
        Self::from_hex(s)
    }
}

impl FromStr for AccountAddress {
    type Err = crate::errors::AptosError;

    fn from_str(s: &str) -> crate::errors::Result<Self> {
        // Accept 0xADDRESS or ADDRESS
        if let Ok(address) = AccountAddress::from_hex_literal(s) {
            Ok(address)
        } else {
            Self::from_hex(s)
        }
    }
}

impl<'de> Deserialize<'de> for AccountAddress {
    fn deserialize<D>(deserializer: D) -> core::result::Result<Self, D::Error>
    where
        D: Deserializer<'de>,
    {
        if deserializer.is_human_readable() {
            let s = <String>::deserialize(deserializer)?;
            AccountAddress::from_str(&s).map_err(D::Error::custom)
        } else {
            // In order to preserve the Serde data model and help analysis tools,
            // make sure to wrap our value in a container with the same name
            // as the original type.
            #[derive(::serde::Deserialize)]
            #[serde(rename = "AccountAddress")]
            struct Value([u8; AccountAddress::LENGTH]);

            let value = Value::deserialize(deserializer)?;
            Ok(AccountAddress::new(value.0))
        }
    }
}

impl Serialize for AccountAddress {
    fn serialize<S>(&self, serializer: S) -> core::result::Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        if serializer.is_human_readable() {
            self.to_hex().serialize(serializer)
        } else {
            // See comment in deserialize.
            serializer.serialize_newtype_struct("AccountAddress", &self.0)
        }
    }
}

#[cfg(test)]
mod tests {
    extern crate std;
    use super::*;

    #[test]
    fn test_account_address_new() {
        let bytes = [0u8; AccountAddress::LENGTH];
        let addr = AccountAddress::new(bytes);
        assert_eq!(addr.into_bytes(), bytes);
    }

    #[test]
    fn test_account_address_zero() {
        let zero = AccountAddress::ZERO;
        assert_eq!(zero.into_bytes(), [0u8; AccountAddress::LENGTH]);
    }

    #[test]
    fn test_account_address_one() {
        let one = AccountAddress::ONE;
        let bytes = one.into_bytes();
        assert_eq!(bytes[AccountAddress::LENGTH - 1], 1);
        for i in 0..AccountAddress::LENGTH - 1 {
            assert_eq!(bytes[i], 0);
        }
    }

    #[test]
    fn test_account_address_short_str_lossless() {
        let addr = AccountAddress::ZERO;
        assert_eq!(addr.short_str_lossless(), "0");

        let mut bytes = [0u8; AccountAddress::LENGTH];
        bytes[AccountAddress::LENGTH - 1] = 0x42;
        let addr = AccountAddress::new(bytes);
        assert_eq!(addr.short_str_lossless(), "42");
    }

    #[test]
    fn test_account_address_to_vec() {
        let addr = AccountAddress::ONE;
        let vec = addr.to_vec();
        assert_eq!(vec.len(), AccountAddress::LENGTH);
    }

    #[test]
    fn test_account_address_from_hex_literal() {
        let addr = AccountAddress::from_hex_literal("0x1").unwrap();
        assert_eq!(addr, AccountAddress::ONE);

        let addr = AccountAddress::from_hex_literal("0x0").unwrap();
        assert_eq!(addr, AccountAddress::ZERO);
    }

    #[test]
    fn test_account_address_from_hex_literal_no_prefix() {
        let result = AccountAddress::from_hex_literal("1");
        assert!(result.is_err());
    }

    #[test]
    fn test_account_address_from_hex_literal_short() {
        let addr = AccountAddress::from_hex_literal("0xa").unwrap();
        let bytes = addr.into_bytes();
        assert_eq!(bytes[AccountAddress::LENGTH - 1], 0xa);
    }

    #[test]
    fn test_account_address_to_hex_literal() {
        let addr = AccountAddress::ONE;
        let hex = addr.to_hex_literal();
        assert!(hex.starts_with("0x"));
    }

    #[test]
    fn test_account_address_from_hex() {
        let hex_str = "00".repeat(AccountAddress::LENGTH);
        let addr = AccountAddress::from_hex(hex_str).unwrap();
        assert_eq!(addr, AccountAddress::ZERO);
    }

    #[test]
    fn test_account_address_from_hex_invalid() {
        let result = AccountAddress::from_hex("zz");
        assert!(result.is_err());
    }

    #[test]
    fn test_account_address_to_hex() {
        let addr = AccountAddress::ONE;
        let hex = addr.to_hex();
        assert_eq!(hex.len(), AccountAddress::LENGTH * 2);
    }

    #[test]
    fn test_account_address_from_bytes() {
        let bytes = [0u8; AccountAddress::LENGTH];
        let addr = AccountAddress::from_bytes(bytes).unwrap();
        assert_eq!(addr.into_bytes(), bytes);
    }

    #[test]
    fn test_account_address_from_bytes_invalid_length() {
        let bytes = vec![0u8; 31];
        let result = AccountAddress::from_bytes(bytes);
        assert!(result.is_err());
    }

    #[test]
    fn test_account_address_as_ref() {
        let addr = AccountAddress::ONE;
        let slice: &[u8] = addr.as_ref();
        assert_eq!(slice.len(), AccountAddress::LENGTH);
    }

    #[test]
    fn test_account_address_deref() {
        let addr = AccountAddress::ONE;
        let bytes: &[u8; AccountAddress::LENGTH] = &*addr;
        assert_eq!(bytes[AccountAddress::LENGTH - 1], 1);
    }

    #[test]
    fn test_account_address_display() {
        let addr = AccountAddress::ONE;
        let s = format!("{}", addr);
        assert!(!s.is_empty());
    }

    #[test]
    fn test_account_address_debug() {
        let addr = AccountAddress::ONE;
        let s = format!("{:?}", addr);
        assert!(!s.is_empty());
    }

    #[test]
    fn test_account_address_lower_hex() {
        let addr = AccountAddress::ONE;
        let s = format!("{:x}", addr);
        assert_eq!(s.len(), AccountAddress::LENGTH * 2);
    }

    #[test]
    fn test_account_address_lower_hex_alternate() {
        let addr = AccountAddress::ONE;
        let s = format!("{:#x}", addr);
        assert!(s.starts_with("0x"));
    }

    #[test]
    fn test_account_address_upper_hex() {
        let addr = AccountAddress::ONE;
        let s = format!("{:X}", addr);
        assert_eq!(s.len(), AccountAddress::LENGTH * 2);
    }

    #[test]
    fn test_account_address_upper_hex_alternate() {
        let addr = AccountAddress::ONE;
        let s = format!("{:#X}", addr);
        assert!(s.starts_with("0x"));
    }

    #[test]
    fn test_account_address_from_array() {
        let bytes = [0u8; AccountAddress::LENGTH];
        let addr: AccountAddress = bytes.into();
        assert_eq!(addr.into_bytes(), bytes);
    }

    #[test]
    fn test_account_address_try_from_slice() {
        let bytes = vec![0u8; AccountAddress::LENGTH];
        let addr = AccountAddress::try_from(bytes.as_slice()).unwrap();
        let expected: [u8; AccountAddress::LENGTH] = bytes.try_into().unwrap();
        assert_eq!(addr.into_bytes(), expected);
    }

    #[test]
    fn test_account_address_try_from_vec() {
        let bytes = vec![0u8; AccountAddress::LENGTH];
        let addr = AccountAddress::try_from(bytes).unwrap();
        assert_eq!(addr.into_bytes(), [0u8; AccountAddress::LENGTH]);
    }

    #[test]
    fn test_account_address_into_vec() {
        let addr = AccountAddress::ONE;
        let vec: Vec<u8> = addr.into();
        assert_eq!(vec.len(), AccountAddress::LENGTH);
    }

    #[test]
    fn test_account_address_ref_into_vec() {
        let addr = AccountAddress::ONE;
        let vec: Vec<u8> = (&addr).into();
        assert_eq!(vec.len(), AccountAddress::LENGTH);
    }

    #[test]
    fn test_account_address_into_array() {
        let addr = AccountAddress::ONE;
        let arr: [u8; AccountAddress::LENGTH] = addr.into();
        assert_eq!(arr[AccountAddress::LENGTH - 1], 1);
    }

    #[test]
    fn test_account_address_ref_into_array() {
        let addr = AccountAddress::ONE;
        let arr: [u8; AccountAddress::LENGTH] = (&addr).into();
        assert_eq!(arr[AccountAddress::LENGTH - 1], 1);
    }

    #[test]
    fn test_account_address_ref_into_string() {
        let addr = AccountAddress::ONE;
        let s: String = (&addr).into();
        assert_eq!(s.len(), AccountAddress::LENGTH * 2);
    }

    #[test]
    fn test_account_address_try_from_string() {
        let hex_str = "00".repeat(AccountAddress::LENGTH);
        let addr = AccountAddress::try_from(hex_str).unwrap();
        assert_eq!(addr, AccountAddress::ZERO);
    }

    #[test]
    fn test_account_address_from_str() {
        let addr = AccountAddress::from_str("0x1").unwrap();
        assert_eq!(addr, AccountAddress::ONE);

        let hex_str = "00".repeat(AccountAddress::LENGTH);
        let addr = AccountAddress::from_str(&hex_str).unwrap();
        assert_eq!(addr, AccountAddress::ZERO);
    }

    #[test]
    fn test_account_address_ord() {
        let addr1 = AccountAddress::ZERO;
        let addr2 = AccountAddress::ONE;
        assert!(addr1 < addr2);
    }

    #[test]
    fn test_account_address_eq() {
        let addr1 = AccountAddress::ONE;
        let addr2 = AccountAddress::ONE;
        assert_eq!(addr1, addr2);
    }

    #[test]
    fn test_account_address_clone() {
        let addr1 = AccountAddress::ONE;
        let addr2 = addr1.clone();
        assert_eq!(addr1, addr2);
    }
}
