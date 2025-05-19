use alloc::string::ToString;
use alloc::vec::Vec;
use alloc::format;
use alloc::{boxed::Box, string::String};
use app_cardano::structs::{
    CardanoCertificate, CardanoFrom, CardanoTo, CardanoWithdrawal, ParsedCardanoSignCip8Data,
    ParsedCardanoSignData, ParsedCardanoTx, VotingProcedure, VotingProposal,
};
use core::ptr::null_mut;
use hex;
use itertools::Itertools;
use ur_registry::cardano::cardano_catalyst_voting_registration::CardanoCatalystVotingRegistrationRequest;

use crate::common::ffi::VecFFI;
use crate::common::free::{free_ptr_string, Free};
use crate::common::structs::TransactionParseResult;
use crate::common::types::{Ptr, PtrString, PtrT};
use crate::common::utils::convert_c_char;
use crate::{
    check_and_free_ptr, free_str_ptr, free_vec, impl_c_ptr, impl_c_ptrs, make_free_method,
};
use app_sui::Intent;
use sui_types::transaction::{
    CallArg, Command, TransactionData, TransactionDataV1, TransactionKind,
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
}

impl_c_ptr!(DisplayIotaIntentData);

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

                let (amount, recipient) = extract_transaction_params(&kind_data.inputs);
                let (transaction_type, method, network) = if check_stake(&kind_data.commands) {
                    (
                        convert_c_char("Programmable Transaction".to_string()),
                        convert_c_char("Stake".to_string()),
                        null_mut(),
                    )
                } else {
                    (
                        null_mut(),
                        null_mut(),
                        convert_c_char("IOTA Mainnet".to_string()),
                    )
                };

                Self {
                    amount: convert_c_char(amount),
                    recipient: convert_c_char(recipient),
                    network,
                    sender: convert_c_char(data.sender.to_string()),
                    details: convert_c_char(details),
                    transaction_type,
                    method,
                }
            }
            _ => todo!("Other Intent types not implemented"),
        }
    }
}

fn extract_transaction_params(args: &Vec<CallArg>) -> (String, String) {
    let mut pure_args = args
        .iter()
        .filter_map(|arg| {
            if let CallArg::Pure(data) = arg {
                Some(data)
            } else {
                None
            }
        })
        .collect::<Vec<_>>();

    let amount = pure_args
        .get(0)
        .map(|bytes| {
            if bytes.len() == 8 {
                format!(
                    "{} IOTA",
                    u64::from_le_bytes(bytes.as_slice().try_into().unwrap_or([0; 8])) as f64
                        / 1000_000_000.0
                )
            } else {
                "0".to_string()
            }
        })
        .unwrap_or_default();

    let recipient = pure_args
        .last()
        .map(|bytes| format!("0x{}", hex::encode(bytes)))
        .unwrap_or_default();

    (amount, recipient)
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
    }
}

make_free_method!(TransactionParseResult<DisplayIotaIntentData>);
