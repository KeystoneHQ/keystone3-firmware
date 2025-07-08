use alloc::format;
use alloc::string::ToString;
use alloc::vec::Vec;
use alloc::{boxed::Box, string::String};
use app_cardano::structs::{
    CardanoCertificate, CardanoFrom, CardanoTo, CardanoWithdrawal, ParsedCardanoSignCip8Data,
    ParsedCardanoSignData, ParsedCardanoTx, VotingProcedure, VotingProposal,
};
use core::ptr::null_mut;
use hex;
use itertools::Itertools;

use crate::common::ffi::VecFFI;
use crate::common::free::{free_ptr_string, Free};
use crate::common::structs::TransactionParseResult;
use crate::common::types::{Ptr, PtrString, PtrT};
use crate::common::utils::convert_c_char;
use crate::{
    check_and_free_ptr, free_str_ptr, free_vec, impl_c_ptr, impl_c_ptrs, make_free_method,
};
use app_ethereum::address::checksum_address;
use app_sui::Intent;
use sui_types::{
    message::PersonalMessage,
    transaction::{CallArg, Command, TransactionData, TransactionDataV1, TransactionKind},
};

#[repr(C)]
pub struct DisplayIotaSignData {
    pub payload: PtrString,
    pub derivation_path: PtrString,
    pub message_hash: PtrString,
    pub xpub: PtrString,
}

#[repr(C)]
pub struct DisplayIotaIntentData {
    amount: PtrString,
    network: PtrString,
    sender: PtrString,
    recipient: PtrString,
    details: PtrString,
    transaction_type: PtrString,
    method: PtrString,
    message: PtrString,
    to: PtrString,
}

impl_c_ptr!(DisplayIotaIntentData);

impl DisplayIotaIntentData {
    pub fn with_address(mut self, address: String) -> Self {
        self.sender = convert_c_char(address);
        self
    }
}

impl From<Intent> for DisplayIotaIntentData {
    fn from(value: Intent) -> Self {
        match value {
            Intent::TransactionData(transaction_data) => {
                let TransactionData::V1(data) = transaction_data.value else {
                    unreachable!("Only V1 transactions are supported")
                };
                let details = serde_json::to_string(&data)
                    .unwrap_or_else(|_| "Failed to serialize transaction data".to_string());

                let TransactionKind::ProgrammableTransaction(kind_data) = data.kind else {
                    unreachable!("Only ProgrammableTransaction is supported")
                };

                let method_string = get_method(&kind_data.commands);
                let method = match method_string.as_str() {
                    "stake" => convert_c_char("Stake".to_string()),
                    "bridge" => convert_c_char("Bridge".to_string()),
                    _ => null_mut(),
                };
                let (amount, recipient, to) =
                    extract_transaction_params(&kind_data.inputs, &method_string);

                Self {
                    amount,
                    recipient,
                    network: convert_c_char("IOTA Mainnet".to_string()),
                    sender: convert_c_char(data.sender.to_string()),
                    details: convert_c_char(details),
                    transaction_type: convert_c_char("Programmable Transaction".to_string()),
                    method,
                    message: null_mut(),
                    to,
                }
            }
            Intent::PersonalMessage(personal_message) => Self {
                amount: null_mut(),
                recipient: null_mut(),
                network: null_mut(),
                sender: null_mut(),
                details: null_mut(),
                transaction_type: convert_c_char("Message".to_string()),
                method: null_mut(),
                message: convert_c_char(
                    base64::decode(&personal_message.value.message)
                        .ok()
                        .and_then(|bytes| String::from_utf8(bytes).ok())
                        .unwrap_or_else(|| personal_message.value.message.clone()),
                ),
                to: null_mut(),
            },
            _ => todo!("Other Intent types not implemented"),
        }
    }
}

fn extract_transaction_params(
    args: &Vec<CallArg>,
    method_string: &String,
) -> (PtrString, PtrString, PtrString) {
    let pure_args = args
        .iter()
        .filter_map(|arg| {
            if let CallArg::Pure(data) = arg {
                Some(data)
            } else {
                None
            }
        })
        .collect::<Vec<_>>();

    let amounts: Vec<u64> = pure_args
        .iter()
        .filter_map(|bytes| {
            if bytes.len() == 8 {
                Some(u64::from_le_bytes(
                    bytes.as_slice().try_into().unwrap_or([0; 8]),
                ))
            } else {
                None
            }
        })
        .collect();

    let amount = if amounts.len() >= 2 {
        let max_amount = amounts.iter().max().unwrap_or(&0);
        let min_amount = amounts.iter().min().unwrap_or(&0);
        let net_amount = max_amount - min_amount;
        convert_c_char(format!("{} IOTA", net_amount as f64 / 1000_000_000.0))
    } else if amounts.len() == 1 {
        convert_c_char(format!("{} IOTA", amounts[0] as f64 / 1000_000_000.0))
    } else {
        null_mut()
    };

    let (recipient, to) = if method_string.contains("bridge") {
        let to = pure_args
            .iter()
            .find(|bytes| bytes.len() == (20 + 1 + 1 + 1))
            .map(|bytes| {
                convert_c_char(
                    checksum_address(&format!("0x{}", hex::encode(&bytes[3..]))).unwrap(),
                )
            })
            .unwrap_or(null_mut());
        (null_mut(), to)
    } else {
        let recipient = pure_args
            .iter()
            .find(|bytes| bytes.len() == 32)
            .map(|bytes| format!("0x{}", hex::encode(bytes)))
            .unwrap_or_default();
        (convert_c_char(recipient), null_mut())
    };

    (amount, recipient, to)
}

fn get_method(commands: &Vec<Command>) -> String {
    let has_assets_bag_new = commands.iter().any(|command| {
        if let Command::MoveCall(kind_data) = command {
            kind_data.module.to_string() == "assets_bag" && kind_data.function.to_string() == "new"
        } else {
            false
        }
    });

    let has_request_create = commands.iter().any(|command| {
        if let Command::MoveCall(kind_data) = command {
            kind_data.module.to_string() == "request"
                && kind_data.function.to_string() == "create_and_send_request"
        } else {
            false
        }
    });

    if has_assets_bag_new && has_request_create {
        return "bridge".to_string();
    }

    let has_stake = commands.iter().any(|command| {
        if let Command::MoveCall(kind_data) = command {
            kind_data.function.to_string().contains("stake")
        } else {
            false
        }
    });

    if has_stake {
        return "stake".to_string();
    }

    commands
        .iter()
        .find_map(|command| {
            if let Command::MoveCall(kind_data) = command {
                Some(kind_data.function.to_string())
            } else {
                None
            }
        })
        .unwrap_or_default()
}

fn check_stake(commands: &Vec<Command>) -> bool {
    commands.iter().any(|command| {
        if let Command::MoveCall(kind_data) = command {
            kind_data.function.to_string().contains("stake")
        } else {
            false
        }
    })
}

#[repr(C)]
pub struct DisplayIotaSignMessageHash {
    pub network: PtrString,
    pub path: PtrString,
    pub from_address: PtrString,
    pub message: PtrString,
}

impl DisplayIotaSignMessageHash {
    pub fn new(network: String, path: String, message: String, from_address: String) -> Self {
        Self {
            network: convert_c_char(network),
            path: convert_c_char(path),
            message: convert_c_char(message),
            from_address: convert_c_char(from_address),
        }
    }
}

impl_c_ptr!(DisplayIotaSignMessageHash);

impl Free for DisplayIotaSignMessageHash {
    fn free(&self) {
        free_str_ptr!(self.network);
        free_str_ptr!(self.path);
        free_str_ptr!(self.message);
        free_str_ptr!(self.from_address);
    }
}

make_free_method!(DisplayIotaSignMessageHash);

impl Free for DisplayIotaIntentData {
    fn free(&self) {
        free_str_ptr!(self.network);
        free_str_ptr!(self.sender);
        free_str_ptr!(self.recipient);
        free_str_ptr!(self.details);
        free_str_ptr!(self.transaction_type);
        free_str_ptr!(self.method);
        free_str_ptr!(self.amount);
        free_str_ptr!(self.message);
    }
}

make_free_method!(TransactionParseResult<DisplayIotaIntentData>);
