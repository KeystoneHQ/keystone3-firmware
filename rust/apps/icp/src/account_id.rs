use alloc::string::String;
use alloc::vec::Vec;
use serde::Serialize;
use sha2::{Digest, Sha224};

use crate::principal::Principal;

const SUB_ACCOUNT_ZERO: Subaccount = Subaccount([0; 32]);
const ACCOUNT_DOMAIN_SEPERATOR: &[u8] = b"\x0Aaccount-id";

#[derive(Clone, Copy, Hash, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct AccountIdentifier {
    pub hash: [u8; 28],
}

impl AccountIdentifier {
    pub fn new(account: Principal, sub_account: Option<Subaccount>) -> AccountIdentifier {
        let mut hash = Sha224::new();
        hash.update(ACCOUNT_DOMAIN_SEPERATOR);
        hash.update(account.as_slice());

        let sub_account = sub_account.unwrap_or(SUB_ACCOUNT_ZERO);
        hash.update(&sub_account.0[..]);

        AccountIdentifier {
            hash: hash.finalize().into(),
        }
    }

    /// Converts this account identifier into a binary "address".
    /// The address is CRC32(identifier) . identifier.
    pub fn to_address(self) -> [u8; 32] {
        let mut result = [0u8; 32];
        result[0..4].copy_from_slice(&self.generate_checksum());
        result[4..32].copy_from_slice(&self.hash);
        result
    }

    pub fn to_hex(self) -> String {
        hex::encode(self.to_vec())
    }

    pub fn to_vec(self) -> Vec<u8> {
        [&self.generate_checksum()[..], &self.hash[..]].concat()
    }

    pub fn generate_checksum(&self) -> [u8; 4] {
        let mut hasher = crc32fast::Hasher::new();
        hasher.update(&self.hash);
        hasher.finalize().to_be_bytes()
    }
}

impl Serialize for AccountIdentifier {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        self.to_hex().serialize(serializer)
    }
}

/// Subaccounts are arbitrary 32-byte values.
#[derive(Clone, Hash, Debug, PartialEq, Eq, Copy)]
pub struct Subaccount(pub [u8; 32]);

impl Subaccount {
    #[allow(dead_code)]
    pub fn to_vec(self) -> Vec<u8> {
        self.0.to_vec()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::principal_id::PrincipalId;
    use alloc::str::FromStr;
    use alloc::string::ToString;
    #[test]
    fn test_from_principal_id_str_to_default_account_id() {
        let principal_id_str = "7rtqo-ah3ki-saurz-utzxq-o4yhl-so2yx-iardd-mktej-x4k24-ijen6-dae";
        let principal_id = PrincipalId::from_str(&principal_id_str).unwrap();
        let account_id: AccountIdentifier = AccountIdentifier::new(principal_id.0, None);
        assert_eq!(
            "33a807e6078195d2bbe1904b0ed0fc65b8a3a437b43831ccebba2b7b6d393bd6".to_string(),
            account_id.to_hex()
        )
    }
}
