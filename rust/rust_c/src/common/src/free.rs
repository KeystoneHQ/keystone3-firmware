use crate::ffi::VecFFI;
use crate::make_free_method;
use crate::structs::SimpleResponse;
use crate::types::{PtrString, PtrT};
use crate::ur::{UREncodeMultiResult, UREncodeResult, URParseMultiResult, URParseResult};
use alloc::boxed::Box;
use cty::{c_char, c_void};

pub trait Free {
    fn free(&self);
}

pub trait SimpleFree {
    fn free(&self);
}

#[macro_export]
macro_rules! check_and_free_ptr {
    ($p: expr) => {
        if $p.is_null() {
            return;
        } else {
            unsafe {
                let x = alloc::boxed::Box::from_raw($p);
                x.free()
            }
        }
    };
}

#[macro_export]
macro_rules! free_str_ptr {
    ($p: expr) => {
        if !$p.is_null() {
            unsafe {
                cstr_core::CString::from_raw($p);
            }
        }
    };
}

#[macro_export]
macro_rules! free_vec {
    ($p: expr) => {
        if !$p.is_null() {
            unsafe {
                let x = alloc::boxed::Box::from_raw($p);
                let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
                ve.iter().for_each(|v| v.free())
            }
        }
    };
}

#[macro_export]
macro_rules! free_ptr_with_type {
    ($x: expr, $name: ident) => {
        if (!$x.is_null()) {
            unsafe {
                let x = extract_ptr_with_type!($x, $name);
                let _b = alloc::boxed::Box::from_raw(x);
                // drop(b);
            }
        }
    };
}

// #[no_mangle]
// pub extern "C" fn free_transaction_parse_result_display_tx(
//     ptr: PtrT<TransactionParseResult<DisplayTx>>,
// ) {
//     check_and_free_ptr!(ptr);
// }
//
// #[no_mangle]
// pub extern "C" fn free_transaction_parse_result_display_eth(
//     ptr: PtrT<TransactionParseResult<DisplayETH>>,
// ) {
//     check_and_free_ptr!(ptr);
// }
//
// #[no_mangle]
// pub extern "C" fn free_transaction_parse_result_display_tron(
//     ptr: PtrT<TransactionParseResult<DisplayTron>>,
// ) {
//     check_and_free_ptr!(ptr);
// }
//
// #[no_mangle]
// pub extern "C" fn free_transaction_parse_result_display_cardano_tx(
//     ptr: PtrT<TransactionParseResult<DisplayCardanoTx>>,
// ) {
//     check_and_free_ptr!(ptr);
// }

#[no_mangle]
pub extern "C" fn free_ur_parse_result(ur_parse_result: PtrT<URParseResult>) {
    check_and_free_ptr!(ur_parse_result);
}

#[no_mangle]
pub extern "C" fn free_ur_parse_multi_result(ptr: PtrT<URParseMultiResult>) {
    check_and_free_ptr!(ptr)
}

#[no_mangle]
pub extern "C" fn free_ur_encode_result(ptr: PtrT<UREncodeResult>) {
    check_and_free_ptr!(ptr);
}

#[no_mangle]
pub extern "C" fn free_ur_encode_muilt_result(ptr: PtrT<UREncodeMultiResult>) {
    check_and_free_ptr!(ptr);
}

#[no_mangle]
pub extern "C" fn free_simple_response_u8(ptr: PtrT<SimpleResponse<u8>>) {
    check_and_free_ptr!(ptr);
}

#[no_mangle]
pub extern "C" fn free_simple_response_c_char(ptr: PtrT<SimpleResponse<c_char>>) {
    check_and_free_ptr!(ptr);
}

#[no_mangle]
pub extern "C" fn free_ptr_string(ptr: PtrString) {
    free_str_ptr!(ptr);
}

#[no_mangle]
pub extern "C" fn free_rust_value(any_ptr: *mut c_void) {
    if any_ptr.is_null() {
        return;
    }
    unsafe {
        drop(Box::from_raw(any_ptr));
    }
}

// make_free_method!(Response<DisplayContractData>);
// make_free_method!(TransactionParseResult<DisplayCardanoTx>);
// make_free_method!(TransactionParseResult<DisplaySolanaTx>);
// make_free_method!(TransactionParseResult<DisplayCosmosTx>);
// make_free_method!(TransactionParseResult<DisplayNearTx>);
// make_free_method!(TransactionParseResult<DisplayXrpTx>);
// make_free_method!(TransactionParseResult<DisplayETHPersonalMessage>);
// make_free_method!(TransactionParseResult<DisplaySolanaMessage>);
// make_free_method!(TransactionParseResult<DisplayETHTypedData>);
// make_free_method!(TransactionParseResult<DisplaySuiIntentMessage>);
// make_free_method!(TransactionParseResult<DisplayAptosTx>);
// make_free_method!(TransactionCheckResult);
make_free_method!(VecFFI<u8>);
