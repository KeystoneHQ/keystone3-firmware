use alloc::boxed::Box;
use alloc::vec::Vec;
use alloc::string::{String, ToString};
use common_rust_c::types::Ptr;
use core::ptr::null_mut;

// use app_bitcoin;
// use app_bitcoin::parsed_tx::{DetailTx, OverviewTx, ParsedInput, ParsedOutput, ParsedTx};
use app_avalanche::transactions::{base_tx::BaseTx};
use common_rust_c::ffi::VecFFI;
use common_rust_c::free::Free;
use common_rust_c::structs::Response;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{PtrString, PtrT};
use common_rust_c::ur::UREncodeResult;
use common_rust_c::utils::convert_c_char;
use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};

// #[repr(C)]
// pub struct PsbtSignResult {
//     base_str: PtrString,
//     hex_str: PtrString,
//     ur_result: PtrT<UREncodeResult>,
// }

// impl PsbtSignResult {
//     pub fn new(psbt_bytes: &Vec<u8>, ur_result: PtrT<UREncodeResult>) -> Self {
//         PsbtSignResult {
//             base_str: convert_c_char(base64::encode(psbt_bytes)),
//             hex_str: convert_c_char(hex::encode(psbt_bytes)),
//             ur_result,
//         }
//     }
// }

// impl Free for PsbtSignResult {
//     fn free(&self) {
//         free_str_ptr!(self.base_str);
//         free_str_ptr!(self.hex_str);
//         unsafe {
//             let x = Box::from_raw(self.ur_result);
//             x.free();
//         }
//     }
// }

// impl_c_ptr!(PsbtSignResult);

// make_free_method!(Response<PsbtSignResult>);

#[repr(C)]
pub struct DisplayAvaxTx {
    overview: *mut DisplayTxAvaxOverview,
    // detail: DisplayTxDetail,
}

impl_c_ptr!(DisplayAvaxTx);

#[repr(C)]
pub struct DisplayTxAvaxOverview {
    total_output_amount: PtrString,
    fee_amount: PtrString,
    network: PtrString,
    method: PtrString,
    // from: PtrT<VecFFI<DisplayAddress>>,
    // to: PtrT<VecFFI<DisplayAvaxOverview>>,
}

impl_c_ptr!(DisplayTxAvaxOverview);

// #[repr(C)]
// pub struct DisplayTxDetail {
//     network: PtrString,
//     method: PtrString,
//     total_input_amount: PtrString,
//     total_output_amount: PtrString,
//     fee_amount: PtrString,
//     from: PtrT<VecFFI<DisplayAvaxDetailInput>>,
//     to: PtrT<VecFFI<DisplayAvaxDetailInput>>,
// }

// impl_c_ptr!(DisplayTxDetail);

#[repr(C)]
pub struct DisplayAddress {
    address: PtrString,
}

#[repr(C)]
pub struct DisplayAvaxDetailInput {
    // has_address: bool,
    address: PtrString,
    amount: PtrString,
    // is_mine: bool,
    path: PtrString,
    // is_external: bool,
}

#[repr(C)]
pub struct DisplayAvaxOverview {
    address: PtrString,
}

// #[repr(C)]
// pub struct DisplayAvaxDetailInput {
//     address: PtrString,
//     amount: PtrString,
//     // is_mine: bool,
//     path: PtrString,
//     // is_external: bool,
// }

impl From<BaseTx> for DisplayAvaxTx {
    fn from(value: BaseTx) -> Self {
        DisplayAvaxTx {
            overview: DisplayTxAvaxOverview::from(value).c_ptr(),
            // detail: DisplayTxDetail::from(value.detail).c_ptr(),
        }
    }
}

impl From<BaseTx> for DisplayTxAvaxOverview {
    fn from(value: BaseTx) -> Self {
        DisplayTxAvaxOverview {
            total_output_amount: convert_c_char(value.get_total_output_amount().to_string()),
            fee_amount: convert_c_char(value.get_fee_amount().to_string()),
            // from: VecFFI::from(
            //     value
            //         .from
            //         .iter()
            //         .map(|v| DisplayAddress {
            //             address: convert_c_char(v.clone()),
            //         })
            //         .collect::<Vec<DisplayAddress>>(),
            // )
            // .c_ptr(),
            // to: VecFFI::from(
            //     value
            //         .to
            //         .iter()
            //         .map(|v| DisplayAvaxOverview {
            //             address: convert_c_char(v.clone()),
            //         })
            //         .collect::<Vec<DisplayAvaxOverview>>(),
            // )
            // .c_ptr(),
            network: convert_c_char(String::from("main")),
            // network: convert_c_char(value.network),
            method: convert_c_char(String::from("Send")),
        }
    }
}

// impl From<DetailTx> for DisplayTxDetail {
//     fn from(value: DetailTx) -> Self {
//         DisplayTxDetail {
//             total_input_amount: convert_c_char(value.total_input_amount),
//             total_output_amount: convert_c_char(value.total_output_amount),
//             fee_amount: convert_c_char(value.fee_amount),
//             from: VecFFI::from(
//                 value
//                     .from
//                     .iter()
//                     .map(|v| DisplayAvaxDetailInput::from(v.clone()))
//                     .collect::<Vec<DisplayAvaxDetailInput>>(),
//             )
//             .c_ptr(),
//             to: VecFFI::from(
//                 value
//                     .to
//                     .iter()
//                     .map(|v| DisplayAvaxDetailInput::from(v.clone()))
//                     .collect::<Vec<DisplayAvaxDetailInput>>(),
//             )
//             .c_ptr(),
//             network: convert_c_char(value.network),
//             total_output_sat: convert_c_char(value.total_output_sat),
//             total_input_sat: convert_c_char(value.total_input_sat),
//             fee_sat: convert_c_char(value.fee_sat),
//             sign_status: if let Some(sign_status) = value.sign_status {
//                 convert_c_char(sign_status)
//             } else {
//                 null_mut()
//             },
//         }
//     }
// }

// impl From<ParsedInput> for DisplayAvaxDetailInput {
//     fn from(value: ParsedInput) -> Self {
//         DisplayAvaxDetailInput {
//             has_address: value.address.is_some(),
//             address: value
//                 .address
//                 .map(|v| convert_c_char(v))
//                 .unwrap_or(null_mut()),
//             amount: convert_c_char(value.amount),
//             is_mine: value.path.is_some(),
//             path: value.path.map(|v| convert_c_char(v)).unwrap_or(null_mut()),
//             is_external: value.is_external,
//         }
//     }
// }

// impl From<ParsedOutput> for DisplayAvaxDetailInput {
//     fn from(value: ParsedOutput) -> Self {
//         DisplayAvaxDetailInput {
//             address: convert_c_char(value.address),
//             amount: convert_c_char(value.amount),
//             is_mine: value.path.is_some(),
//             path: value.path.map(|v| convert_c_char(v)).unwrap_or(null_mut()),
//             is_external: value.is_external,
//         }
//     }
// }

// impl Free for DisplayTx {
//     fn free(&self) {
//         unsafe {
//             let x = Box::from_raw(self.overview);
//             let y = Box::from_raw(self.detail);
//             x.free();
//             y.free();
//         }
//     }
// }

// make_free_method!(TransactionParseResult<DisplayTx>);

// impl Free for DisplayTxOverview {
//     fn free(&self) {
//         unsafe {
//             let x = Box::from_raw(self.from);
//             let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
//             ve.iter().for_each(|v| {
//                 v.free();
//             });
//             let x = Box::from_raw(self.to);
//             let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
//             ve.iter().for_each(|v| {
//                 v.free();
//             });

//             let _ = Box::from_raw(self.total_output_amount);
//             let _ = Box::from_raw(self.fee_amount);
//             let _ = Box::from_raw(self.total_output_sat);
//             let _ = Box::from_raw(self.fee_sat);
//             let _ = Box::from_raw(self.network);
//         }
//     }
// }

// impl Free for DisplayTxDetail {
//     fn free(&self) {
//         unsafe {
//             let x = Box::from_raw(self.from);
//             let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
//             ve.iter().for_each(|v| {
//                 v.free();
//             });
//             let x = Box::from_raw(self.to);
//             let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
//             ve.iter().for_each(|v| {
//                 v.free();
//             });

//             let _ = Box::from_raw(self.total_input_amount);
//             let _ = Box::from_raw(self.total_output_amount);
//             let _ = Box::from_raw(self.fee_amount);
//             let _ = Box::from_raw(self.network);
//             let _ = Box::from_raw(self.total_input_sat);
//             let _ = Box::from_raw(self.total_output_sat);
//             let _ = Box::from_raw(self.fee_sat);
//         }
//     }
// }

// impl Free for DisplayAddress {
//     fn free(&self) {
//         unsafe {
//             let _ = Box::from_raw(self.address);
//         }
//     }
// }

// impl Free for DisplayAvaxDetailInput {
//     fn free(&self) {
//         unsafe {
//             let _ = Box::from_raw(self.address);
//             let _ = Box::from_raw(self.amount);
//             let _ = Box::from_raw(self.path);
//         }
//     }
// }

// impl Free for DisplayAvaxOverview {
//     fn free(&self) {
//         unsafe {
//             let _ = Box::from_raw(self.address);
//         }
//     }
// }

// impl Free for DisplayAvaxDetailInput {
//     fn free(&self) {
//         unsafe {
//             let _ = Box::from_raw(self.address);
//             let _ = Box::from_raw(self.amount);
//             let _ = Box::from_raw(self.path);
//         }
//     }
// }

// #[repr(C)]
// pub struct DisplayBtcMsg {
//     pub detail: PtrString,
//     pub address: PtrString,
// }

// impl_c_ptr!(DisplayBtcMsg);

// impl Free for DisplayBtcMsg {
//     fn free(&self) {
//         free_str_ptr!(self.detail);
//         free_str_ptr!(self.address);
//     }
// }

// make_free_method!(TransactionParseResult<DisplayBtcMsg>);
