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
use sui_types::transaction::{TransactionData, TransactionDataV1, TransactionKind, CallArg};

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
    owner: PtrString,
    price: PtrString,
    gas_budget: PtrString,
    details: PtrString,
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

                Self {
                    amount: convert_c_char(amount),
                    recipient: convert_c_char(recipient),
                    network: convert_c_char("IOTA Mainnet".to_string()),
                    sender: convert_c_char(data.sender.to_string()),
                    owner: convert_c_char(data.gas_data.owner.to_string()),
                    price: convert_c_char(data.gas_data.price.to_string()),
                    gas_budget: convert_c_char(data.gas_data.budget.to_string()),
                    details: convert_c_char(details),
                }
            }
            _ => todo!("Other Intent types not implemented"),
        }
    }
}

fn extract_transaction_params(args: &Vec<CallArg>) -> (String, String) {
    let amount = args.get(0)
        .and_then(|arg| if let CallArg::Pure(bytes) = arg {
            Some(format!("{}", u64::from_le_bytes(
                bytes.as_slice().try_into().unwrap_or([0; 8])
            )))
        } else {
            None
        })
        .unwrap_or_default();

    let recipient = args.get(1)
        .and_then(|arg| if let CallArg::Pure(bytes) = arg {
            Some(format!("0x{}", hex::encode(bytes)))
        } else {
            None
        })
        .unwrap_or_default();

    (amount, recipient)
}

impl Free for DisplayIotaIntentData {
    fn free(&self) {
        free_str_ptr!(self.network);
        free_str_ptr!(self.sender);
        free_str_ptr!(self.recipient);
        free_str_ptr!(self.owner);
        free_str_ptr!(self.price);
        free_str_ptr!(self.gas_budget);
        free_str_ptr!(self.details);
    }
}

make_free_method!(TransactionParseResult<DisplayIotaIntentData>);
