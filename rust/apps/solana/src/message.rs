use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;

use bitcoin;
use bitcoin::base58;

use crate::compact::Compact;
use crate::errors::{Result, SolanaError};
use crate::instruction::Instruction;
pub use crate::message_v1::TransactionConfig;
use crate::parser::detail::{CommonDetail, ProgramDetail, ProgramDetailInstruction, SolanaDetail};
use crate::read::Read;

struct Signature {
    value: Vec<u8>,
}

impl Read<Signature> for Signature {
    fn read(raw: &mut Vec<u8>) -> Result<Signature> {
        if raw.len() < 64 {
            return Err(SolanaError::InvalidData("signature".to_string()));
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
            return Err(SolanaError::InvalidData("account".to_string()));
        }
        Ok(Account {
            value: raw.splice(0..32, []).collect(),
        })
    }
}

#[derive(Clone)]
pub struct BlockHash {
    pub value: Vec<u8>,
}

impl Read<BlockHash> for BlockHash {
    fn read(raw: &mut Vec<u8>) -> Result<BlockHash> {
        if raw.len() < 32 {
            return Err(SolanaError::InvalidData("blockhash".to_string()));
        }
        Ok(BlockHash {
            value: raw.splice(0..32, []).collect(),
        })
    }
}

#[derive(Clone)]
pub struct Message {
    pub transaction_config: Option<TransactionConfig>,
    pub is_versioned: bool,
    pub header: MessageHeader,
    pub accounts: Vec<Account>,
    pub block_hash: BlockHash,
    pub instructions: Vec<Instruction>,
    pub address_table_lookups: Option<Vec<MessageAddressTableLookup>>,
}

impl Read<Message> for Message {
    fn read(raw: &mut Vec<u8>) -> Result<Message> {
        if raw.first() == Some(&0x81) {
            return Self::read_v1(raw);
        }
        let first_byte = raw.first().copied();
        let is_versioned = match first_byte {
            Some(0x80) => true,
            Some(value) if value & 0x80 != 0 => {
                return Err(SolanaError::InvalidData(
                    "unsupported message version".to_string(),
                ))
            }
            Some(_) => false,
            None => return Err(SolanaError::InvalidData("empty message".to_string())),
        };
        if is_versioned {
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
        let message = Message {
            transaction_config: None,
            is_versioned,
            header,
            accounts,
            block_hash,
            instructions,
            address_table_lookups,
        };
        message.validate_structure()?;
        Ok(message)
    }
}

impl Message {
    pub fn read_exact(raw: &mut Vec<u8>) -> Result<Message> {
        let message = Self::read(raw)?;
        if !raw.is_empty() {
            return Err(SolanaError::InvalidData(
                "trailing bytes after message".to_string(),
            ));
        }
        Ok(message)
    }

    pub fn validate_signer(&self, signer: &[u8; 32]) -> Result<()> {
        let required_signatures = self.header.num_required_signatures as usize;
        if required_signatures == 0 {
            return Err(SolanaError::InvalidData(
                "transaction does not require a signer".to_string(),
            ));
        }
        if self.accounts[..required_signatures]
            .iter()
            .any(|account| account.value.as_slice() == signer)
        {
            return Ok(());
        }
        Err(SolanaError::InvalidData(
            "derived key is not a required transaction signer".to_string(),
        ))
    }

    pub(crate) fn validate_structure(&self) -> Result<()> {
        let static_account_count = self.accounts.len();
        let required_signatures = self.header.num_required_signatures as usize;
        let readonly_signed = self.header.num_readonly_signed_accounts as usize;
        let readonly_unsigned = self.header.num_readonly_unsigned_accounts as usize;

        if required_signatures > static_account_count {
            return Err(SolanaError::InvalidData(
                "required signatures exceed static accounts".to_string(),
            ));
        }
        if readonly_signed > required_signatures {
            return Err(SolanaError::InvalidData(
                "readonly signed accounts exceed required signatures".to_string(),
            ));
        }
        if readonly_unsigned > static_account_count.saturating_sub(required_signatures) {
            return Err(SolanaError::InvalidData(
                "readonly unsigned accounts exceed unsigned accounts".to_string(),
            ));
        }

        let loaded_account_count = self
            .address_table_lookups
            .as_ref()
            .map(|lookups| {
                lookups.iter().fold(0usize, |count, lookup| {
                    count
                        .saturating_add(lookup.writable_indexes.len())
                        .saturating_add(lookup.readonly_indexes.len())
                })
            })
            .unwrap_or(0);
        let account_count = static_account_count.saturating_add(loaded_account_count);

        for instruction in &self.instructions {
            if instruction.program_index as usize >= account_count {
                return Err(SolanaError::InvalidData(
                    "program index exceeds account list".to_string(),
                ));
            }
            if instruction
                .account_indexes
                .iter()
                .any(|index| *index as usize >= account_count)
            {
                return Err(SolanaError::InvalidData(
                    "instruction account index exceeds account list".to_string(),
                ));
            }
        }
        Ok(())
    }

    pub fn to_program_details(&self) -> Result<Vec<SolanaDetail>> {
        let resolved_accounts = self.prepare_accounts();
        let mut details = self
            .instructions
            .iter()
            .map(|instruction| {
                let instruction_accounts = instruction
                    .account_indexes
                    .iter()
                    .map(|account_index| {
                        resolved_accounts
                            .get(*account_index as usize)
                            .map(|v| v.to_string())
                            .unwrap_or("Unknown Account".to_string())
                    })
                    .collect::<Vec<String>>();
                let program_account = resolved_accounts
                    .get(usize::from(instruction.program_index))
                    .ok_or_else(|| {
                        SolanaError::InvalidData(
                            "program index exceeds resolved account list".to_string(),
                        )
                    })?
                    .to_string();
                // V1 compute-budget instructions are successful no-ops, even
                // when their data is invalid. Never display them as active fees.
                if self.transaction_config.is_some()
                    && program_account == "ComputeBudget111111111111111111111111111111"
                {
                    return Ok(SolanaDetail {
                        common: CommonDetail {
                            program: "ComputeBudget".to_string(),
                            method: "IgnoredInV1".to_string(),
                        },
                        kind: ProgramDetail::Instruction(ProgramDetailInstruction {
                            data: base58::encode(&instruction.data),
                            accounts: instruction_accounts,
                            program_account,
                        }),
                    });
                }
                // parse instruction data
                match instruction.parse(&program_account, instruction_accounts.clone()) {
                    Ok(value) => Ok(value),
                    Err(_) => Ok(SolanaDetail {
                        common: CommonDetail {
                            program: "Unknown".to_string(),
                            method: "".to_string(),
                        },
                        kind: ProgramDetail::Instruction(ProgramDetailInstruction {
                            data: base58::encode(&instruction.data),
                            accounts: instruction_accounts,
                            program_account,
                        }),
                    }),
                }
            })
            .collect::<Result<Vec<SolanaDetail>>>()?;
        if let Some(config) = &self.transaction_config {
            details.push(config.to_detail());
        }
        Ok(details)
    }

    pub fn validate(raw: &mut Vec<u8>) -> bool {
        Self::read_exact(raw).is_ok()
    }

    pub fn has_valid_prefix(raw: &mut Vec<u8>) -> bool {
        Self::read(raw).is_ok()
    }

    fn prepare_accounts(&self) -> Vec<String> {
        // encode convert accounts bytes to base58
        let mut accounts: Vec<String> = self
            .accounts
            .clone()
            .iter()
            .map(|v| bitcoin::base58::encode(&v.value))
            .collect();
        // construct address table lookup account
        let mut writable_lookup_accounts: Vec<String> = vec![];
        let mut readonly_lookup_accounts: Vec<String> = vec![];
        if let Some(table) = &self.address_table_lookups {
            table.iter().for_each(|cur: &MessageAddressTableLookup| {
                // global account vec = account_keys + writable_indexes + readonly_indexes
                let parent_address = bitcoin::base58::encode(&cur.account_key.value);
                let mut writable_child_accounts: Vec<String> = cur
                    .writable_indexes
                    .iter()
                    .map(|v| format!("{}#{}", &parent_address, v))
                    .collect();
                writable_lookup_accounts.append(&mut writable_child_accounts);
                let mut readonly_child_accounts: Vec<String> = cur
                    .readonly_indexes
                    .iter()
                    .map(|v| format!("{}#{}", &parent_address, v))
                    .collect();
                readonly_lookup_accounts.append(&mut readonly_child_accounts);
            });
        }
        accounts.append(&mut writable_lookup_accounts);
        accounts.append(&mut readonly_lookup_accounts);
        accounts
    }
}

#[derive(Clone)]
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
#[derive(Clone)]
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

#[cfg(test)]
mod tests {
    use alloc::vec;
    use alloc::vec::Vec;

    use super::Message;

    fn minimal_legacy_message() -> Vec<u8> {
        let mut message = vec![1, 0, 0, 1];
        message.extend_from_slice(&[0u8; 32]);
        message.extend_from_slice(&[0u8; 32]);
        message.push(0);
        message
    }

    fn system_transfer_message() -> Vec<u8> {
        let mut message = vec![1, 0, 1, 3];
        message.extend_from_slice(&[0x11u8; 32]);
        message.extend_from_slice(&[0x22u8; 32]);
        message.extend_from_slice(&[0u8; 32]);
        message.extend_from_slice(&[0x77u8; 32]);
        message.extend_from_slice(&[
            1, // instruction count
            2, // System Program index in the complete account list
            2, 0, 1, // account indexes
            12, 2, 0, 0, 0, // SystemInstruction::Transfer
            0, 202, 154, 59, 0, 0, 0, 0, // 1 SOL
        ]);
        message
    }

    #[test]
    fn exact_parser_rejects_trailing_bytes() {
        let mut message = minimal_legacy_message();
        assert!(Message::read_exact(&mut message).is_ok());

        let mut message_with_suffix = minimal_legacy_message();
        message_with_suffix.push(0xaa);
        assert!(Message::read_exact(&mut message_with_suffix).is_err());

        let mut message_with_suffix = minimal_legacy_message();
        message_with_suffix.push(0xaa);
        assert!(Message::has_valid_prefix(&mut message_with_suffix));
    }

    #[test]
    fn rejects_unsupported_versions_and_invalid_headers() {
        assert!(Message::read_exact(&mut vec![0x81]).is_err());

        let mut invalid_header = minimal_legacy_message();
        invalid_header[0] = 2;
        assert!(Message::read_exact(&mut invalid_header).is_err());
    }

    #[test]
    fn resolves_program_index_from_complete_account_list() {
        let message = Message::read_exact(&mut system_transfer_message()).unwrap();
        let details = message.to_program_details().unwrap();
        assert_eq!(details.len(), 1);
        assert_eq!(details[0].common.program, "System");
        assert_eq!(details[0].common.method, "Transfer");
    }
}
