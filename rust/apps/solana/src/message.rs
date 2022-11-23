use crate::compact::Compact;
use crate::errors::{Result, SolanaError};
use crate::instruction::Instruction;
use crate::parser::detail::{CommonDetail, ProgramDetail, ProgramDetailUnknown, SolanaDetail};
use crate::read::Read;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use third_party::base58;

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
    pub header: MessageHeader,
    pub accounts: Vec<Account>,
    pub block_hash: BlockHash,
    pub instructions: Vec<Instruction>,
}

impl Read<Message> for Message {
    fn read(raw: &mut Vec<u8>) -> Result<Message> {
        let header = MessageHeader::read(raw)?;
        let accounts = Compact::read(raw)?.data;
        let block_hash = BlockHash::read(raw)?;
        let instructions = Compact::read(raw)?.data;
        Ok(Message {
            header,
            accounts,
            block_hash,
            instructions,
        })
    }
}

impl Message {
    pub fn to_program_details(&self) -> Result<Vec<SolanaDetail>> {
        self.instructions
            .iter()
            .map(|instruction| {
                let accounts = instruction
                    .account_indexes
                    .iter()
                    .map(|account_index| {
                        base58::encode(&self.accounts[usize::from(*account_index)].value)
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
