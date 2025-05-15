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
use app_iota::transaction::TransactionData;

#[repr(C)]
pub struct DisplayIotaSignData {
    pub payload: PtrString,
    pub derivation_path: PtrString,
    pub message_hash: PtrString,
    pub xpub: PtrString,
}

// #[repr(C)]
// pub struct DisplayIotaIntent {
//     data: *mut DisplayIotaIntentData,
// }

// impl_c_ptr!(DisplayIotaIntent);

#[repr(C)]
pub struct DisplayIotaIntentData {
    kind: PtrString,
    inputs: PtrT<VecFFI<PtrString>>,
    sender: PtrString,
    owner: PtrString,
    price: PtrString,
    gas_budget: PtrString,
}

impl_c_ptr!(DisplayIotaIntentData);

impl From<TransactionData> for DisplayIotaIntentData {
    fn from(value: TransactionData) -> Self {
        match value {
            TransactionData::V1(data) => {
                Self {
                    kind: convert_c_char(String::from("ProgrammableTransaction")),
                    inputs: VecFFI::from(data.get_kind().get_inputs_string().iter().map(|v| convert_c_char(v.to_string())).collect_vec()).c_ptr(),
                    sender: convert_c_char(hex::encode(data.get_sender())),
                    owner: convert_c_char(hex::encode(data.get_gas_data().get_owner())),
                    price: convert_c_char(data.get_gas_data().get_price().to_string()),
                    gas_budget: convert_c_char(data.get_gas_data().get_budget().to_string()),
                }
            }
        }
    }
}
