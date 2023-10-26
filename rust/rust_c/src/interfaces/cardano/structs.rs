use crate::interfaces::ffi::VecFFI;
use crate::interfaces::free::{free_ptr_string, Free};
use crate::interfaces::types::{Ptr, PtrString, PtrT};
use crate::interfaces::utils::convert_c_char;
use crate::{free_str_ptr, free_vec, impl_c_ptr, impl_c_ptrs};
use alloc::boxed::Box;
use alloc::vec::Vec;
use app_cardano::structs::{
    CardanoCertificate, CardanoFrom, CardanoTo, CardanoWithdrawal, ParsedCardanoTx,
};
use core::ptr::null_mut;
use third_party::itertools::Itertools;

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
    address: PtrString,
    pool: PtrString,
}

#[repr(C)]
pub struct DisplayCardanoWithdrawal {
    address: PtrString,
    amount: PtrString,
}

impl_c_ptrs!(DisplayCardanoTx);

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

            free_ptr_string(self.fee);
            free_ptr_string(self.network);
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
            address: convert_c_char(value.get_address()),
            pool: value
                .get_pool()
                .map(|v| convert_c_char(v))
                .unwrap_or(null_mut()),
        }
    }
}

impl Free for DisplayCardanoCertificate {
    fn free(&self) {
        free_str_ptr!(self.cert_type);
        free_str_ptr!(self.address);
        free_str_ptr!(self.pool);
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
