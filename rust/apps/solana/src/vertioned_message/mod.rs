use crate::errors::Result;
use crate::message::Message as LegacyMessage;
use crate::parser::detail::{CommonDetail, ProgramDetail, ProgramDetailUnknown, SolanaDetail};
use crate::read::Read;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use third_party::base58;

pub mod v0;

/// Bit mask that indicates whether a serialized message is versioned.
pub const MESSAGE_VERSION_PREFIX: u8 = 0x80;

/// Either a legacy message or a v0 message.
///
/// # Serialization
///
/// If the first bit is set, the remaining 7 bits will be used to determine
/// which message version is serialized starting from version `0`. If the first
/// is bit is not set, all bytes are used to encode the legacy `Message`
/// format.
pub enum VersionedMessage {
    Legacy(LegacyMessage),
    V0(v0::Message),
}

impl VersionedMessage {
    pub fn read(raw: &mut Vec<u8>) -> Result<VersionedMessage> {
        if MESSAGE_VERSION_PREFIX == raw[0] {
            raw.remove(0);
            Ok(VersionedMessage::V0(v0::Message::read(raw)?))
        } else {
            Ok(VersionedMessage::Legacy(LegacyMessage::read(raw)?))
        }
    }

    pub fn to_program_details(&self) -> Result<Vec<SolanaDetail>> {
        let instructions = match self {
            VersionedMessage::Legacy(m) => &m.instructions,
            VersionedMessage::V0(m) => &m.instructions,
        };
        let accounts = match self {
            VersionedMessage::Legacy(m) => &m.accounts,
            VersionedMessage::V0(m) => &m.account_keys,
        };
        instructions
            .iter()
            .map(|instruction| {
                let accounts_str_vec = instruction
                    .account_indexes
                    .iter()
                    .map(|account_index| {
                        let i = usize::from(*account_index);
                        if i >= accounts.len() {
                            "Unknown".to_string()
                        } else {
                            base58::encode(&accounts[i].value)
                        }
                    })
                    .collect::<Vec<String>>();
                let program_account =
                    base58::encode(&accounts[usize::from(instruction.program_index)].value);
                let accounts_string = accounts_str_vec.join(",");
                match instruction.parse(&program_account, accounts_str_vec) {
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
}
