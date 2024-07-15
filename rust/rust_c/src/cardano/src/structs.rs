use alloc::boxed::Box;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use app_cardano::structs::{
    CardanoCertificate, CardanoFrom, CardanoTo, CardanoWithdrawal, ParsedCardanoSignData,
    ParsedCardanoTx,
};
use core::ptr::null_mut;
use third_party::hex;
use third_party::itertools::Itertools;
use third_party::ur_registry::cardano::cardano_catalyst_voting_registration::CardanoCatalystVotingRegistrationRequest;

use common_rust_c::ffi::VecFFI;
use common_rust_c::free::{free_ptr_string, Free};
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{Ptr, PtrString, PtrT};
use common_rust_c::utils::convert_c_char;
use common_rust_c::{
    check_and_free_ptr, free_str_ptr, free_vec, impl_c_ptr, impl_c_ptrs, make_free_method,
};

#[repr(C)]
pub struct DisplayCardanoSignData {
    pub payload: PtrString,
    pub derivation_path: PtrString,
}

#[repr(C)]
pub struct DisplayCardanoCatalyst {
    pub nonce: PtrString,
    pub stake_key: PtrString,
    pub rewards: PtrString,
    pub vote_keys: Ptr<VecFFI<PtrString>>,
}

impl From<CardanoCatalystVotingRegistrationRequest> for DisplayCardanoCatalyst {
    fn from(value: CardanoCatalystVotingRegistrationRequest) -> Self {
        Self {
            nonce: convert_c_char(value.get_nonce().to_string()),
            stake_key: convert_c_char(app_cardano::governance::parse_stake_address(
                value.get_stake_pub(),
            ).unwrap()),
            rewards: convert_c_char(app_cardano::governance::parse_payment_address(
                value.get_payment_address(),
            ).unwrap()),
            vote_keys: VecFFI::from(
                value
                    .get_delegations()
                    .iter()
                    .map(|v| convert_c_char(v.get_path().get_path().unwrap()))
                    .collect_vec(),
            )
            .c_ptr(),
        }
    }
}

#[repr(C)]
pub struct DisplayCardanoTx {
    pub from: PtrT<VecFFI<DisplayCardanoFrom>>,
    pub to: PtrT<VecFFI<DisplayCardanoTo>>,
    pub fee: PtrString,
    pub network: PtrString,
    pub total_input: PtrString,
    pub total_output: PtrString,
    pub certificates: Ptr<VecFFI<DisplayCardanoCertificate>>,
    pub withdrawals: Ptr<VecFFI<DisplayCardanoWithdrawal>>,
    pub auxiliary_data: PtrString,
}

#[repr(C)]
pub struct DisplayCardanoFrom {
    address: PtrString,
    amount: PtrString,
    has_path: bool,
    path: PtrString,
}

#[repr(C)]
pub struct DisplayCardanoTo {
    address: PtrString,
    amount: PtrString,
    has_assets: bool,
    assets_text: PtrString,
}

#[repr(C)]
pub struct DisplayCardanoCertificate {
    cert_type: PtrString,
    variant1: PtrString,
    variant2: PtrString,
    variant1_label: PtrString,
    variant2_label: PtrString,
}

#[repr(C)]
pub struct DisplayCardanoWithdrawal {
    address: PtrString,
    amount: PtrString,
}

impl_c_ptrs!(DisplayCardanoTx);

impl_c_ptrs!(DisplayCardanoCatalyst);

impl_c_ptrs!(DisplayCardanoSignData);

impl Free for DisplayCardanoSignData {
    fn free(&self) {
        free_str_ptr!(self.payload);
        free_str_ptr!(self.derivation_path);
    }
}

impl Free for DisplayCardanoCatalyst {
    fn free(&self) {
        free_str_ptr!(self.nonce);
        free_str_ptr!(self.stake_key);
        free_str_ptr!(self.rewards);
        free_vec!(self.vote_keys);
    }
}

impl Free for DisplayCardanoTx {
    fn free(&self) {
        unsafe {
            let x = Box::from_raw(self.from);
            let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
            ve.iter().for_each(|v| {
                v.free();
            });
            let x = Box::from_raw(self.to);
            let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
            ve.iter().for_each(|v| {
                v.free();
            });
            free_vec!(self.withdrawals);
            free_vec!(self.certificates);

            free_ptr_string(self.total_input);
            free_ptr_string(self.total_output);
            free_ptr_string(self.fee);
            free_ptr_string(self.network);
        }
    }
}

impl From<ParsedCardanoSignData> for DisplayCardanoSignData {
    fn from(value: ParsedCardanoSignData) -> Self {
        Self {
            payload: convert_c_char(value.get_payload()),
            derivation_path: convert_c_char(value.get_derivation_path()),
        }
    }
}

impl From<ParsedCardanoTx> for DisplayCardanoTx {
    fn from(value: ParsedCardanoTx) -> Self {
        Self {
            from: VecFFI::from(
                value
                    .get_from()
                    .iter()
                    .map(DisplayCardanoFrom::from)
                    .collect_vec(),
            )
            .c_ptr(),
            to: VecFFI::from(
                value
                    .get_to()
                    .iter()
                    .map(DisplayCardanoTo::from)
                    .collect_vec(),
            )
            .c_ptr(),
            fee: convert_c_char(value.get_fee()),
            network: convert_c_char(value.get_network()),
            total_input: convert_c_char(value.get_total_input()),
            total_output: convert_c_char(value.get_total_output()),
            certificates: VecFFI::from(
                value
                    .get_certificates()
                    .iter()
                    .map(DisplayCardanoCertificate::from)
                    .collect_vec(),
            )
            .c_ptr(),
            withdrawals: VecFFI::from(
                value
                    .get_withdrawals()
                    .iter()
                    .map(DisplayCardanoWithdrawal::from)
                    .collect_vec(),
            )
            .c_ptr(),
            auxiliary_data: value
                .get_auxiliary_data()
                .map(|v| convert_c_char(v))
                .unwrap_or(null_mut()),
        }
    }
}

impl From<&CardanoFrom> for DisplayCardanoFrom {
    fn from(value: &CardanoFrom) -> Self {
        Self {
            address: convert_c_char(value.get_address()),
            amount: convert_c_char(value.get_amount()),
            has_path: value.get_path().is_some(),
            path: value.get_path().map(convert_c_char).unwrap_or(null_mut()),
        }
    }
}

impl Free for DisplayCardanoTo {
    fn free(&self) {
        free_str_ptr!(self.assets_text);
        free_str_ptr!(self.address);
        free_str_ptr!(self.amount);
    }
}

impl Free for DisplayCardanoFrom {
    fn free(&self) {
        free_str_ptr!(self.path);
        free_str_ptr!(self.amount);
        free_str_ptr!(self.address);
    }
}

impl From<&CardanoTo> for DisplayCardanoTo {
    fn from(value: &CardanoTo) -> Self {
        Self {
            address: convert_c_char(value.get_address()),
            amount: convert_c_char(value.get_amount()),
            has_assets: value.get_assets_text().is_some(),
            assets_text: value
                .get_assets_text()
                .map(convert_c_char)
                .unwrap_or(null_mut()),
        }
    }
}

impl From<&CardanoCertificate> for DisplayCardanoCertificate {
    fn from(value: &CardanoCertificate) -> Self {
        Self {
            cert_type: convert_c_char(value.get_cert_type()),
            variant1: convert_c_char(value.get_variant1()),
            variant2: value
                .get_variant2()
                .map(|v| convert_c_char(v))
                .unwrap_or(null_mut()),
            variant1_label: convert_c_char(value.get_variant1_label()),
            variant2_label: value
                .get_variant2_label()
                .map(|v| convert_c_char(v))
                .unwrap_or(null_mut()),
        }
    }
}

impl Free for DisplayCardanoCertificate {
    fn free(&self) {
        free_str_ptr!(self.cert_type);
        free_str_ptr!(self.variant1);
        free_str_ptr!(self.variant2);
        free_str_ptr!(self.variant1_label);
        free_str_ptr!(self.variant2_label);
    }
}

impl From<&CardanoWithdrawal> for DisplayCardanoWithdrawal {
    fn from(value: &CardanoWithdrawal) -> Self {
        Self {
            address: convert_c_char(value.get_address()),
            amount: convert_c_char(value.get_amount()),
        }
    }
}

impl Free for DisplayCardanoWithdrawal {
    fn free(&self) {
        free_str_ptr!(self.address);
        free_str_ptr!(self.amount);
    }
}

make_free_method!(TransactionParseResult<DisplayCardanoTx>);
make_free_method!(TransactionParseResult<DisplayCardanoCatalyst>);
make_free_method!(TransactionParseResult<DisplayCardanoSignData>);
