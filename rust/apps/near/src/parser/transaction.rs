use crate::account_id::AccountId;
use crate::crypto::PublicKey;
use crate::parser::utils::{format_gas_amount, format_u128_amount};
use crate::primitives_core::account::AccessKey;
use crate::primitives_core::hash::{hash, CryptoHash};

use crate::primitives_core::types::{Balance, Gas, Nonce};
use alloc::fmt;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use borsh::{BorshDeserialize, BorshSerialize};
use hex;
use serde::{Deserialize, Serialize};

#[derive(BorshSerialize, BorshDeserialize, Serialize, Deserialize, PartialEq, Eq, Debug, Clone)]
pub struct Transaction {
    /// An account on which behalf transaction is signed
    pub signer_id: AccountId,
    /// A public key of the access key which was used to sign an account.
    /// Access key holds permissions for calling certain kinds of actions.
    pub public_key: PublicKey,
    /// Nonce is used to determine order of transaction in the pool.
    /// It increments for a combination of `signer_id` and `public_key`
    pub nonce: Nonce,
    /// Receiver account for this transaction
    pub receiver_id: AccountId,
    /// The hash of the block in the blockchain on top of which the given transaction is valid
    pub block_hash: CryptoHash,
    /// A list of actions to be applied
    pub actions: Vec<Action>,
}

#[allow(dead_code)]
impl Transaction {
    /// Computes a hash of the transaction for signing and size of serialized transaction
    pub fn get_hash_and_size(&self) -> (CryptoHash, u64) {
        let bytes = self.try_to_vec().expect("Failed to deserialize");
        (hash(&bytes), bytes.len() as u64)
    }
}

#[derive(BorshSerialize, BorshDeserialize, Serialize, Deserialize, PartialEq, Eq, Debug, Clone)]
#[serde(tag = "Action")]
pub enum Action {
    /// Create an (sub)account using a transaction `receiver_id` as an ID for
    /// a new account ID must pass validation rules described here
    /// <http://nomicon.io/Primitives/Account.html>.
    #[serde(rename = "Create Account")]
    CreateAccount(CreateAccountAction),
    /// Sets a Wasm code to a receiver_id
    #[serde(rename = "Deploy Contract")]
    DeployContract(DeployContractAction),
    #[serde(rename = "Function Call")]
    FunctionCall(FunctionCallAction),
    #[serde(rename = "Transfer")]
    Transfer(TransferAction),
    #[serde(rename = "Stake")]
    Stake(StakeAction),
    #[serde(rename = "Add Key")]
    AddKey(AddKeyAction),
    #[serde(rename = "Delete Key")]
    DeleteKey(DeleteKeyAction),
    #[serde(rename = "Delete Account")]
    DeleteAccount(DeleteAccountAction),
}

pub trait Normalizer {
    fn normalize(&self) -> String;
}

impl Normalizer for Action {
    fn normalize(&self) -> String {
        match self {
            Action::CreateAccount(v) => v.normalize(),
            Action::DeployContract(v) => v.normalize(),
            Action::FunctionCall(v) => v.normalize(),
            Action::Transfer(v) => v.normalize(),
            Action::Stake(v) => v.normalize(),
            Action::AddKey(v) => v.normalize(),
            Action::DeleteKey(v) => v.normalize(),
            Action::DeleteAccount(v) => v.normalize(),
        }
    }
}

/// Create account action
#[derive(BorshSerialize, BorshDeserialize, Serialize, Deserialize, PartialEq, Eq, Clone, Debug)]
pub struct CreateAccountAction {}

impl Normalizer for CreateAccountAction {
    fn normalize(&self) -> String {
        "Create Account".to_string()
    }
}

impl From<CreateAccountAction> for Action {
    fn from(create_account_action: CreateAccountAction) -> Self {
        Self::CreateAccount(create_account_action)
    }
}

/// Deploy contract action
#[derive(BorshSerialize, BorshDeserialize, Serialize, Deserialize, PartialEq, Eq, Clone)]
pub struct DeployContractAction {
    /// WebAssembly binary
    #[serde(skip_serializing)]
    pub code: Vec<u8>,
}

impl Normalizer for DeployContractAction {
    fn normalize(&self) -> String {
        "Deploy Contract".to_string()
    }
}

impl From<DeployContractAction> for Action {
    fn from(deploy_contract_action: DeployContractAction) -> Self {
        Self::DeployContract(deploy_contract_action)
    }
}

impl fmt::Debug for DeployContractAction {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("DeployContractAction")
            .field("code", &format_args!("{}", hex::encode(&self.code)))
            .finish()
    }
}

#[derive(BorshSerialize, BorshDeserialize, Serialize, Deserialize, PartialEq, Eq, Clone)]
pub struct FunctionCallAction {
    #[serde(rename = "Method")]
    pub method_name: String,
    #[serde(skip_serializing)]
    pub args: Vec<u8>,
    #[serde(serialize_with = "format_gas_amount", rename = "Prepaid Gas")]
    pub gas: Gas,
    #[serde(serialize_with = "format_u128_amount", rename = "Deposit Value")]
    pub deposit: Balance,
}

impl Normalizer for FunctionCallAction {
    fn normalize(&self) -> String {
        "Function Call".to_string()
    }
}

impl From<FunctionCallAction> for Action {
    fn from(function_call_action: FunctionCallAction) -> Self {
        Self::FunctionCall(function_call_action)
    }
}

impl fmt::Debug for FunctionCallAction {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("FunctionCallAction")
            .field("method_name", &format_args!("{}", &self.method_name))
            .field("args", &format_args!("{}", hex::encode(&self.args)))
            .field("gas", &format_args!("{}", &self.gas))
            .field("deposit", &format_args!("{}", &self.deposit))
            .finish()
    }
}

#[derive(BorshSerialize, BorshDeserialize, Serialize, Deserialize, PartialEq, Eq, Clone, Debug)]
pub struct TransferAction {
    #[serde(rename = "Value", serialize_with = "format_u128_amount")]
    pub deposit: Balance,
}

impl From<TransferAction> for Action {
    fn from(transfer_action: TransferAction) -> Self {
        Self::Transfer(transfer_action)
    }
}

impl Normalizer for TransferAction {
    fn normalize(&self) -> String {
        "Transfer".to_string()
    }
}

/// An action which stakes singer_id tokens and setup's validator public key
#[derive(BorshSerialize, BorshDeserialize, Serialize, Deserialize, PartialEq, Eq, Clone, Debug)]
pub struct StakeAction {
    /// Amount of tokens to stake.
    #[serde(serialize_with = "format_u128_amount", rename = "Stake Amount")]
    pub stake: Balance,
    /// Validator key which will be used to sign transactions on behalf of singer_id
    #[serde(rename = "Public Key")]
    pub public_key: PublicKey,
}

impl Normalizer for StakeAction {
    fn normalize(&self) -> String {
        "Stake".to_string()
    }
}

impl From<StakeAction> for Action {
    fn from(stake_action: StakeAction) -> Self {
        Self::Stake(stake_action)
    }
}

#[derive(BorshSerialize, BorshDeserialize, Serialize, Deserialize, PartialEq, Eq, Clone, Debug)]
pub struct AddKeyAction {
    /// A public key which will be associated with an access_key
    #[serde(rename = "Public Key")]
    pub public_key: PublicKey,
    /// An access key with the permission
    #[serde(flatten)]
    pub access_key: AccessKey,
}

impl Normalizer for AddKeyAction {
    fn normalize(&self) -> String {
        "Add Key".to_string()
    }
}

impl From<AddKeyAction> for Action {
    fn from(add_key_action: AddKeyAction) -> Self {
        Self::AddKey(add_key_action)
    }
}

#[derive(BorshSerialize, BorshDeserialize, Serialize, Deserialize, PartialEq, Eq, Clone, Debug)]
pub struct DeleteKeyAction {
    /// A public key associated with the access_key to be deleted.
    #[serde(rename = "Public Key")]
    pub public_key: PublicKey,
}

impl Normalizer for DeleteKeyAction {
    fn normalize(&self) -> String {
        "Delete Key".to_string()
    }
}

impl From<DeleteKeyAction> for Action {
    fn from(delete_key_action: DeleteKeyAction) -> Self {
        Self::DeleteKey(delete_key_action)
    }
}

#[derive(BorshSerialize, BorshDeserialize, Serialize, Deserialize, PartialEq, Eq, Clone, Debug)]
pub struct DeleteAccountAction {
    #[serde(rename = "Beneficiary ID")]
    pub beneficiary_id: AccountId,
}

impl Normalizer for DeleteAccountAction {
    fn normalize(&self) -> String {
        "Delete Account".to_string()
    }
}

impl From<DeleteAccountAction> for Action {
    fn from(delete_account_action: DeleteAccountAction) -> Self {
        Self::DeleteAccount(delete_account_action)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::crypto::PublicKey;
    use crate::primitives_core::account::{AccessKeyPermission, FunctionCallPermission};
    use alloc::string::ToString;
    use hex;

    /// This test is change checker for a reason - we don't expect transaction format to change.
    /// If it does - you MUST update all of the dependencies: like nearlib and other clients.
    #[test]
    fn test_serialize_transaction() {
        let public_key: PublicKey = "22skMptHjFWNyuEWY22ftn2AbLPSYpmYwGJRGwpNHbTV"
            .parse()
            .unwrap();
        let transaction = Transaction {
            signer_id: "test.near".parse().unwrap(),
            public_key: public_key.clone(),
            nonce: 1,
            receiver_id: "123".parse().unwrap(),
            block_hash: Default::default(),
            actions: vec![
                Action::CreateAccount(CreateAccountAction {}),
                Action::DeployContract(DeployContractAction {
                    code: vec![1, 2, 3],
                }),
                Action::FunctionCall(FunctionCallAction {
                    method_name: "qqq".to_string(),
                    args: vec![1, 2, 3],
                    gas: 1_000,
                    deposit: 1_000_000,
                }),
                Action::Transfer(TransferAction { deposit: 123 }),
                Action::Stake(StakeAction {
                    public_key: public_key.clone(),
                    stake: 1_000_000,
                }),
                Action::AddKey(AddKeyAction {
                    public_key: public_key.clone(),
                    access_key: AccessKey {
                        nonce: 0,
                        permission: AccessKeyPermission::FunctionCall(FunctionCallPermission {
                            allowance: None,
                            receiver_id: "zzz".parse().unwrap(),
                            method_names: vec!["www".to_string()],
                        }),
                    },
                }),
                Action::DeleteKey(DeleteKeyAction { public_key }),
                Action::DeleteAccount(DeleteAccountAction {
                    beneficiary_id: "123".parse().unwrap(),
                }),
            ],
        };
        let result = BorshSerialize::try_to_vec(&transaction).unwrap();
        assert_eq!(
            "09000000746573742e6e656172000f56a5f028dfc089ec7c39c1183b321b4d8f89ba5bec9e1762803cc2491f6ef8010000000000000003000000313233000000000000000000000000000000000000000000000000000000000000000008000000000103000000010203020300000071717103000000010203e80300000000000040420f00000000000000000000000000037b0000000000000000000000000000000440420f00000000000000000000000000000f56a5f028dfc089ec7c39c1183b321b4d8f89ba5bec9e1762803cc2491f6ef805000f56a5f028dfc089ec7c39c1183b321b4d8f89ba5bec9e1762803cc2491f6ef800000000000000000000030000007a7a7a010000000300000077777706000f56a5f028dfc089ec7c39c1183b321b4d8f89ba5bec9e1762803cc2491f6ef80703000000313233",
            hex::encode(result),
        );
    }
}
