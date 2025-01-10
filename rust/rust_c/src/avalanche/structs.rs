use alloc::boxed::Box;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use app_avalanche::constants::NAVAX_TO_AVAX_RATIO;
use core::ptr::null_mut;

use app_avalanche::transactions::{
    base_tx::BaseTx,
    structs::{AvaxFromToInfo, AvaxMethodInfo, AvaxTxInfo},
};

use crate::common::{
    errors::RustCError,
    ffi::{CSliceFFI, VecFFI},
    free::{free_ptr_string, Free},
    structs::{ExtendedPublicKey, SimpleResponse, TransactionCheckResult, TransactionParseResult},
    types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR},
    ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT, FRAGMENT_UNLIMITED_LENGTH},
    utils::{convert_c_char, recover_c_char},
};

use crate::{
    check_and_free_ptr, extract_ptr_with_type, free_str_ptr, impl_c_ptr, impl_new_error,
    impl_response, impl_simple_c_ptr, impl_simple_new_error, make_free_method,
};

#[repr(C)]
pub struct DisplayAvaxTx {
    data: *mut DisplayTxAvaxData,
}

impl_c_ptr!(DisplayAvaxTx);

#[repr(C)]
pub struct DisplayTxAvaxData {
    network: PtrString,
    network_key: PtrString,
    subnet_id: PtrString,
    total_output_amount: PtrString,
    total_input_amount: PtrString,
    fee_amount: PtrString,
    reward_address: PtrString,
    method: PtrT<DisplayAvaxMethodInfo>,
    to: PtrT<VecFFI<DisplayAvaxFromToInfo>>,
    from: PtrT<DisplayAvaxFromToInfo>,
}

impl_c_ptr!(DisplayTxAvaxData);

#[repr(C)]
pub struct DisplayAvaxFromToInfo {
    address: PtrString,
    amount: PtrString,
    path: PtrString,
}

impl Free for DisplayAvaxFromToInfo {
    fn free(&self) {
        unsafe {
            free_ptr_string(self.address);
            free_ptr_string(self.amount);
            free_ptr_string(self.path);
        }
    }
}

impl_c_ptr!(DisplayAvaxFromToInfo);

impl From<&AvaxFromToInfo> for DisplayAvaxFromToInfo {
    fn from(value: &AvaxFromToInfo) -> Self {
        DisplayAvaxFromToInfo {
            address: convert_c_char(value.address.get(0).unwrap().clone()),
            amount: convert_c_char(value.amount.clone()),
            path: convert_c_char("".to_string()),
        }
    }
}

#[repr(C)]
pub struct DisplayAvaxMethodInfo {
    method_key: PtrString,
    method: PtrString,
    start_time: i64,
    end_time: i64,
}

impl_c_ptr!(DisplayAvaxMethodInfo);

impl Free for DisplayAvaxMethodInfo {
    fn free(&self) {
        unsafe {
            free_ptr_string(self.method_key);
            free_ptr_string(self.method);
        }
    }
}

impl From<AvaxMethodInfo> for DisplayAvaxMethodInfo {
    fn from(value: AvaxMethodInfo) -> Self {
        DisplayAvaxMethodInfo {
            method_key: convert_c_char(value.method_key),
            method: convert_c_char(value.method),
            start_time: value.start_time,
            end_time: value.end_time,
        }
    }
}

impl<T: AvaxTxInfo> From<T> for DisplayAvaxTx {
    fn from(value: T) -> Self {
        DisplayAvaxTx {
            data: DisplayTxAvaxData::from(value).c_ptr(),
        }
    }
}

impl DisplayAvaxTx {
    pub fn with_from_address(mut self, address: String, path: String) -> Self {
        unsafe {
            let data = &mut *self.data;
            let from = &mut *data.from;
            from.address = convert_c_char(address);
            from.path = convert_c_char(path);
        }
        self
    }
}

impl<T: AvaxTxInfo> From<T> for DisplayTxAvaxData {
    fn from(value: T) -> Self {
        DisplayTxAvaxData {
            from: DisplayAvaxFromToInfo {
                address: convert_c_char("".to_string()),
                amount: convert_c_char(format!(
                    "{} AVAX",
                    value.get_total_input_amount() as f64 / NAVAX_TO_AVAX_RATIO
                )),
                path: convert_c_char("".to_string()),
            }
            .c_ptr(),
            total_input_amount: convert_c_char(format!(
                "{} AVAX",
                value.get_total_input_amount() as f64 / NAVAX_TO_AVAX_RATIO
            )),
            total_output_amount: convert_c_char(format!(
                "{} AVAX",
                value.get_total_output_amount() as f64 / NAVAX_TO_AVAX_RATIO
            )),
            fee_amount: convert_c_char(format!(
                "{} AVAX",
                value.get_fee_amount() as f64 / NAVAX_TO_AVAX_RATIO
            )),
            to: VecFFI::from(
                value
                    .get_outputs_addresses()
                    .iter()
                    .map(|v| DisplayAvaxFromToInfo::from(v))
                    .collect::<Vec<DisplayAvaxFromToInfo>>(),
            )
            .c_ptr(),
            network_key: convert_c_char(value.get_network_key()),
            subnet_id: value
                .get_subnet_id()
                .map_or(core::ptr::null_mut(), convert_c_char),
            network: value
                .get_network()
                .map_or(core::ptr::null_mut(), convert_c_char),
            reward_address: value
                .get_reward_address()
                .map_or(core::ptr::null_mut(), convert_c_char),
            method: value.get_method_info().map_or(core::ptr::null_mut(), |v| {
                DisplayAvaxMethodInfo::from(v).c_ptr()
            }),
        }
    }
}

impl Free for DisplayTxAvaxData {
    fn free(&self) {
        unsafe {
            // let x = Box::from_raw(self.from);
            // let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
            // ve.iter().for_each(|v| {
            //     v.free();
            // });
            let x = Box::from_raw(self.to);
            let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
            ve.iter().for_each(|v| {
                v.free();
            });

            free_ptr_string(self.network);
            free_ptr_string(self.network_key);
            free_ptr_string(self.subnet_id);
            free_ptr_string(self.total_output_amount);
            free_ptr_string(self.total_input_amount);
            free_ptr_string(self.fee_amount);
            free_ptr_string(self.reward_address);
            Box::from_raw(self.method);
        }
    }
}

impl Free for DisplayAvaxTx {
    fn free(&self) {
        unsafe {
            Box::from_raw(self.data).free();
        }
    }
}

make_free_method!(TransactionParseResult<DisplayAvaxTx>);
