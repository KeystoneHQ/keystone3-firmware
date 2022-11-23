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
}
