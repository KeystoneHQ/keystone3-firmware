#![allow(dead_code)]
use crate::parser::utils::format_option_u128_amount;
use crate::primitives_core::hash::CryptoHash;
use crate::primitives_core::serialize::dec_format;
use crate::primitives_core::types::{Balance, Nonce, StorageUsage};
use alloc::string::String;
use alloc::vec::Vec;
use borsh::maybestd::io;
use borsh::{BorshDeserialize, BorshSerialize};
use serde::{Deserialize, Serialize};

#[derive(
    BorshSerialize,
    BorshDeserialize,
    Serialize,
    Deserialize,
    PartialEq,
    Eq,
    Debug,
    Clone,
    Copy,
    Default,
)]
pub enum AccountVersion {
    #[default]
    V1,
}

#[derive(Serialize, Deserialize, PartialEq, Eq, Debug, Clone)]
pub struct Account {
    #[serde(with = "dec_format")]
    amount: Balance,

    #[serde(with = "dec_format")]
    locked: Balance,

    code_hash: CryptoHash,

    storage_usage: StorageUsage,

    #[serde(default)]
    version: AccountVersion,
}

impl Account {
    pub const MAX_ACCOUNT_DELETION_STORAGE_USAGE: u64 = 10_000;

    pub fn new(
        amount: Balance,
        locked: Balance,
        code_hash: CryptoHash,
        storage_usage: StorageUsage,
    ) -> Self {
        Account {
            amount,
            locked,
            code_hash,
            storage_usage,
            version: AccountVersion::V1,
        }
    }

    #[inline]
    pub fn amount(&self) -> Balance {
        self.amount
    }

    #[inline]
    pub fn locked(&self) -> Balance {
        self.locked
    }

    #[inline]
    pub fn code_hash(&self) -> CryptoHash {
        self.code_hash
    }

    #[inline]
    pub fn storage_usage(&self) -> StorageUsage {
        self.storage_usage
    }

    #[inline]
    pub fn version(&self) -> AccountVersion {
        self.version
    }

    #[inline]
    pub fn set_amount(&mut self, amount: Balance) {
        self.amount = amount;
    }

    #[inline]
    pub fn set_locked(&mut self, locked: Balance) {
        self.locked = locked;
    }

    #[inline]
    pub fn set_code_hash(&mut self, code_hash: CryptoHash) {
        self.code_hash = code_hash;
    }

    #[inline]
    pub fn set_storage_usage(&mut self, storage_usage: StorageUsage) {
        self.storage_usage = storage_usage;
    }

    pub fn set_version(&mut self, version: AccountVersion) {
        self.version = version;
    }
}

#[derive(BorshSerialize, BorshDeserialize)]
struct LegacyAccount {
    amount: Balance,
    locked: Balance,
    code_hash: CryptoHash,
    storage_usage: StorageUsage,
}

impl BorshDeserialize for Account {
    fn deserialize(buf: &mut &[u8]) -> Result<Self, io::Error> {
        // This should only ever happen if we have pre-transition account serialized in state
        // See test_account_size
        let deserialized_account = LegacyAccount::deserialize(buf)?;
        if buf.len() != 0 {
            panic!("Tried deserializing a buffer that is not exactly the size of an account");
        }
        Ok(Account {
            amount: deserialized_account.amount,
            locked: deserialized_account.locked,
            code_hash: deserialized_account.code_hash,
            storage_usage: deserialized_account.storage_usage,
            version: AccountVersion::V1,
        })
    }
}

impl BorshSerialize for Account {
    fn serialize<W: io::Write>(&self, writer: &mut W) -> io::Result<()> {
        match self.version {
            AccountVersion::V1 => LegacyAccount {
                amount: self.amount,
                locked: self.locked,
                code_hash: self.code_hash,
                storage_usage: self.storage_usage,
            }
            .serialize(writer),
        }
    }
}

#[derive(
    BorshSerialize, BorshDeserialize, Serialize, Deserialize, PartialEq, Eq, Hash, Clone, Debug,
)]
pub struct AccessKey {
    #[serde(rename = "Access Key Nonce")]
    pub nonce: Nonce,
    #[serde(flatten)]
    pub permission: AccessKeyPermission,
}

impl AccessKey {
    pub const ACCESS_KEY_NONCE_RANGE_MULTIPLIER: u64 = 1_000_000;

    pub fn full_access() -> Self {
        Self {
            nonce: 0,
            permission: AccessKeyPermission::FullAccess,
        }
    }
}

#[derive(
    BorshSerialize, BorshDeserialize, Serialize, Deserialize, PartialEq, Eq, Hash, Clone, Debug,
)]
#[serde(tag = "Access Key Permission")]
pub enum AccessKeyPermission {
    FunctionCall(FunctionCallPermission),
    FullAccess,
}

#[derive(
    BorshSerialize, BorshDeserialize, Serialize, Deserialize, PartialEq, Eq, Hash, Clone, Debug,
)]
pub struct FunctionCallPermission {
    #[serde(
        rename = "Access Key Allowance",
        serialize_with = "format_option_u128_amount"
    )]
    pub allowance: Option<Balance>,

    // This isn't an AccountId because already existing records in testnet genesis have invalid
    // values for this field (see: https://github.com/near/nearcore/pull/4621#issuecomment-892099860)
    // we accomodate those by using a string, allowing us to read and parse genesis.
    #[serde(rename = "Access Key Receiver ID")]
    pub receiver_id: String,
    #[serde(rename = "Access Key Method Names")]
    pub method_names: Vec<String>,
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::primitives_core::hash::hash;
    use crate::primitives_core::serialize::to_base;
    use borsh::BorshSerialize;

    #[test]
    fn test_account_serialization() {
        let acc = Account::new(1_000_000, 1_000_000, CryptoHash::default(), 100);
        let bytes = acc.try_to_vec().unwrap();
        let hash = hash(bytes.as_slice());
        assert_eq!(
            to_base(&hash.0.to_vec()),
            "EVk5UaxBe8LQ8r8iD5EAxVBs6TJcMDKqyH7PBuho6bBJ"
        );
    }

    #[test]
    fn test_account_deserialization() {
        let old_account = LegacyAccount {
            amount: 100,
            locked: 200,
            code_hash: CryptoHash::default(),
            storage_usage: 300,
        };
        let mut old_bytes = &old_account.try_to_vec().unwrap()[..];
        let new_account = <Account as BorshDeserialize>::deserialize(&mut old_bytes).unwrap();
        assert_eq!(new_account.amount, old_account.amount);
        assert_eq!(new_account.locked, old_account.locked);
        assert_eq!(new_account.code_hash, old_account.code_hash);
        assert_eq!(new_account.storage_usage, old_account.storage_usage);
        assert_eq!(new_account.version, AccountVersion::V1);
        let mut new_bytes = &new_account.try_to_vec().unwrap()[..];
        let deserialized_account =
            <Account as BorshDeserialize>::deserialize(&mut new_bytes).unwrap();
        assert_eq!(deserialized_account, new_account);
    }
}
