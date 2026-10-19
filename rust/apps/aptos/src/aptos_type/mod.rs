#![allow(dead_code)] //we dont check 3rd party code warnings

mod account_address;
mod chain_id;
mod identifier;
mod language_storage;
mod module;
mod parser;
mod safe_serialize;
mod script;
mod serde_helper;
mod transaction_argument;
mod value;

use account_address::AccountAddress;
use chain_id::ChainId;
use module::ModuleBundle;
use script::EntryFunction;
use script::Script;
use serde::{Deserialize, Serialize};

#[derive(Debug, Hash, Eq, PartialEq, Serialize, Deserialize)]
pub struct RawTransaction {
    /// Sender's address.
    sender: AccountAddress,

    /// Sequence number of this transaction. This must match the sequence number
    /// stored in the sender's account at the time the transaction executes.
    sequence_number: u64,

    /// The transaction payload, e.g., a script to execute.
    payload: TransactionPayload,

    /// Maximal total gas to spend for this transaction.
    max_gas_amount: u64,

    /// Price to be paid per gas unit.
    gas_unit_price: u64,

    /// Expiration timestamp for this transaction, represented
    /// as seconds from the Unix Epoch. If the current blockchain timestamp
    /// is greater than or equal to this time, then the transaction has
    /// expired and will be discarded. This can be set to a large value far
    /// in the future to indicate that a transaction does not expire.
    expiration_timestamp_secs: u64,

    /// Chain ID of the Aptos network this transaction is intended for.
    chain_id: ChainId,
}

/// Different kinds of transactions.
#[derive(Clone, Debug, Hash, Eq, PartialEq, Serialize, Deserialize)]
pub enum TransactionPayload {
    /// A transaction that executes code.
    Script(Script),
    /// A transaction that publishes multiple modules at the same time.
    ModuleBundle(ModuleBundle),
    /// A transaction that executes an existing entry function published on-chain.
    EntryFunction(EntryFunction),
    // BCS variant 3: append only, preserving existing transaction discriminants.
    Multisig(Multisig),
}

/// Execution of a pre-approved proposal by one owner of an Aptos multisig account.
/// This is a transaction payload, not a multi-agent/multi-key signing envelope.
#[derive(Clone, Debug, Hash, Eq, PartialEq, Serialize, Deserialize)]
pub struct Multisig {
    multisig_address: AccountAddress,
    transaction_payload: Option<MultisigTransactionPayload>,
}

#[derive(Clone, Debug, Hash, Eq, PartialEq, Serialize, Deserialize)]
pub enum MultisigTransactionPayload {
    EntryFunction(EntryFunction),
    Script(Script),
}

impl RawTransaction {
    pub(crate) fn validate_for_review(&self) -> crate::errors::Result<()> {
        if let TransactionPayload::Multisig(multisig) = &self.payload {
            // The device cannot fetch an omitted on-chain proposal. Accept only
            // an inline entry function so the existing JSON review contains both
            // the vault address and the action. Script execution is out of scope.
            if !matches!(
                &multisig.transaction_payload,
                Some(MultisigTransactionPayload::EntryFunction(_))
            ) {
                return Err(crate::errors::AptosError::ParseTxError(
                    alloc::string::String::from(
                        "multisig execution requires an inline entry function",
                    ),
                ));
            }
        }
        Ok(())
    }
}
