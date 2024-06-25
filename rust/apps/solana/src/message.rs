use crate::compact::Compact;
use crate::errors::{Result, SolanaError};
use crate::instruction::Instruction;
use crate::parser::detail::{CommonDetail, ProgramDetail, ProgramDetailUnknown, SolanaDetail};
use crate::read::Read;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;
use third_party::base58;
use third_party::sha1::digest::consts::True;

struct Signature {
    value: Vec<u8>,
}

impl Read<Signature> for Signature {
    fn read(raw: &mut Vec<u8>) -> Result<Signature> {
        if raw.len() < 64 {
            return Err(SolanaError::InvalidData(format!("signature")));
        }
        Ok(Signature {
            value: raw.splice(0..64, []).collect(),
        })
    }
}

#[derive(Clone)]
pub struct Account {
    pub value: Vec<u8>,
}

impl Read<Account> for Account {
    fn read(raw: &mut Vec<u8>) -> Result<Account> {
        if raw.len() < 32 {
            return Err(SolanaError::InvalidData(format!("account")));
        }
        Ok(Account {
            value: raw.splice(0..32, []).collect(),
        })
    }
}

pub struct BlockHash {
    pub value: Vec<u8>,
}

impl Read<BlockHash> for BlockHash {
    fn read(raw: &mut Vec<u8>) -> Result<BlockHash> {
        if raw.len() < 32 {
            return Err(SolanaError::InvalidData(format!("blockhash")));
        }
        Ok(BlockHash {
            value: raw.splice(0..32, []).collect(),
        })
    }
}

pub struct Message {
    pub is_versioned: bool,
    pub header: MessageHeader,
    pub accounts: Vec<Account>,
    pub block_hash: BlockHash,
    pub instructions: Vec<Instruction>,
    pub address_table_lookups: Option<Vec<MessageAddressTableLookup>>,
}

impl Read<Message> for Message {
    fn read(raw: &mut Vec<u8>) -> Result<Message> {
        let first_byte = raw.get(0);
        let is_versioned = match first_byte {
            Some(0x80) => true,
            Some(_) => false,
            None => return Err(SolanaError::InvalidData("empty message".to_string())),
        };
        if (is_versioned) {
            raw.remove(0);
        }
        let header = MessageHeader::read(raw)?;
        let accounts = Compact::read(raw)?.data;
        let block_hash = BlockHash::read(raw)?;
        let instructions = Compact::read(raw)?.data;
        let address_table_lookups = match is_versioned {
            true => Some(Compact::read(raw)?.data),
            false => None,
        };
        Ok(Message {
            is_versioned,
            header,
            accounts,
            block_hash,
            instructions,
            address_table_lookups,
        })
    }
}

impl Message {
    pub fn to_program_details(&self) -> Result<Vec<SolanaDetail>> {
        let accounts = self.prepare_accounts();
        self.instructions
            .iter()
            .map(|instruction| {
                let accounts = instruction
                    .account_indexes
                    .iter()
                    .map(|account_index| {
                        accounts
                            .get(*account_index as usize)
                            .map(|v| v.to_string())
                            .unwrap_or("Unknown Account".to_string())
                    })
                    .collect::<Vec<String>>();
                let program_account =
                    base58::encode(&self.accounts[usize::from(instruction.program_index)].value);
                let accounts_string = accounts.join(",");
                match instruction.parse(&program_account, accounts) {
                    Ok(value) => Ok(value),
                    Err(e) => Ok(SolanaDetail {
                        common: CommonDetail {
                            program: "Unknown".to_string(),
                            method: "".to_string(),
                        },
                        kind: ProgramDetail::Unknown(ProgramDetailUnknown {
                            program_index: instruction.program_index,
                            account_indexes: instruction.account_indexes.to_vec(),
                            data: base58::encode(&instruction.data),
                            reason: format!("Unable to parse instruction, {}", e.to_string()),
                            accounts: accounts_string,
                            program_account,
                        }),
                    }),
                }
            })
            .collect::<Result<Vec<SolanaDetail>>>()
    }

    pub fn validate(raw: &mut Vec<u8>) -> bool {
        Self::read(raw).is_ok()
    }

    fn prepare_accounts(&self) -> Vec<String> {
        let mut accounts: Vec<String> = self
            .accounts
            .clone()
            .iter()
            .map(|v| base58::encode(&v.value))
            .collect();
        if let Some(table) = &self.address_table_lookups {
            let mut child_accounts =
                table
                    .iter()
                    .fold(vec![], |acc, cur: &MessageAddressTableLookup| {
                        let mut indexes = cur.readonly_indexes.clone();
                        indexes.append(&mut cur.writable_indexes.clone());
                        indexes.sort();
                        let parent_address = base58::encode(&cur.account_key.value);
                        let child_accounts: Vec<String> = indexes
                            .iter()
                            .map(|v| format!("{}#{}", &parent_address, v))
                            .collect();
                        vec![acc, child_accounts].concat()
                    });
            accounts.append(&mut child_accounts);
        }
        accounts
    }
}

pub struct MessageHeader {
    pub num_required_signatures: u8,
    pub num_readonly_signed_accounts: u8,
    pub num_readonly_unsigned_accounts: u8,
}

impl Read<MessageHeader> for MessageHeader {
    fn read(raw: &mut Vec<u8>) -> Result<MessageHeader> {
        if raw.len() < 3 {
            return Err(SolanaError::InvalidData("message header".to_string()));
        }
        let n1 = raw.remove(0);
        let n2 = raw.remove(0);
        let n3 = raw.remove(0);
        Ok(MessageHeader {
            num_required_signatures: n1,
            num_readonly_signed_accounts: n2,
            num_readonly_unsigned_accounts: n3,
        })
    }
}

impl Read<u8> for u8 {
    fn read(raw: &mut Vec<u8>) -> Result<u8> {
        if raw.is_empty() {
            return Err(SolanaError::InvalidData("u8".to_string()));
        }
        Ok(raw.remove(0))
    }
}

pub struct MessageAddressTableLookup {
    pub account_key: Account,
    pub writable_indexes: Vec<u8>,
    pub readonly_indexes: Vec<u8>,
}

impl Read<MessageAddressTableLookup> for MessageAddressTableLookup {
    fn read(raw: &mut Vec<u8>) -> Result<MessageAddressTableLookup> {
        let account_key = Account::read(raw)?;
        let writable_indexes = Compact::read(raw)?.data;
        let readonly_indexes = Compact::read(raw)?.data;
        Ok(Self {
            account_key,
            writable_indexes,
            readonly_indexes,
        })
    }
}
