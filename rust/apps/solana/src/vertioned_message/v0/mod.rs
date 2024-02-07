use crate::compact::Compact;
use crate::errors::Result;
use crate::instruction::Instruction as CompiledInstruction;
use crate::message::MessageHeader;
use crate::message::{Account, BlockHash};
use crate::read::Read;
use alloc::vec::Vec;

/// Address table lookups describe an on-chain address lookup table to use
/// for loading more readonly and writable accounts in a single tx.
pub struct MessageAddressTableLookup {
    /// Address lookup table account key
    pub account_key: Account,
    /// List of indexes used to load writable account addresses
    // #[serde(with = "short_vec")]
    pub writable_indexes: Vec<u8>,
    /// List of indexes used to load readonly account addresses
    // #[serde(with = "short_vec")]
    pub readonly_indexes: Vec<u8>,
}

impl Read<MessageAddressTableLookup> for MessageAddressTableLookup {
    fn read(raw: &mut Vec<u8>) -> Result<MessageAddressTableLookup> {
        let account_key = Account::read(raw)?;
        let writable_indexes = Compact::read(raw)?.data;
        let readonly_indexes = Compact::read(raw)?.data;
        Ok(MessageAddressTableLookup {
            account_key,
            writable_indexes,
            readonly_indexes,
        })
    }
}

pub struct Message {
    /// The message header, identifying signed and read-only `account_keys`.
    /// Header values only describe static `account_keys`, they do not describe
    /// any additional account keys loaded via address table lookups.
    pub header: MessageHeader,

    /// List of accounts loaded by this transaction.
    // #[serde(with = "short_vec")]
    pub account_keys: Vec<Account>,

    /// The blockhash of a recent block.
    pub recent_blockhash: BlockHash,

    /// Instructions that invoke a designated program, are executed in sequence,
    /// and committed in one atomic transaction if all succeed.
    ///
    /// # Notes
    ///
    /// Program indexes must index into the list of message `account_keys` because
    /// program id's cannot be dynamically loaded from a lookup table.
    ///
    /// Account indexes must index into the list of addresses
    /// constructed from the concatenation of three key lists:
    ///   1) message `account_keys`
    ///   2) ordered list of keys loaded from `writable` lookup table indexes
    ///   3) ordered list of keys loaded from `readable` lookup table indexes
    // #[serde(with = "short_vec")]
    pub instructions: Vec<CompiledInstruction>,

    /// List of address table lookups used to load additional accounts
    /// for this transaction.
    // #[serde(with = "short_vec")]
    pub address_table_lookups: Vec<MessageAddressTableLookup>,
}

impl Read<Message> for Message {
    fn read(raw: &mut Vec<u8>) -> Result<Message> {
        let header = MessageHeader::read(raw)?;
        let account_keys = Compact::read(raw)?.data;
        let recent_blockhash = BlockHash::read(raw)?;
        let instructions = Compact::read(raw)?.data;
        let address_table_lookups = Compact::read(raw)?.data;
        Ok(Message {
            header,
            account_keys,
            recent_blockhash,
            instructions,
            address_table_lookups,
        })
    }
}
