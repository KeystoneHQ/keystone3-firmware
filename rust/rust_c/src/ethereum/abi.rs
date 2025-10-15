use super::structs::{DisplayContractData, DisplaySwapkitContractData};
use crate::common::errors::RustCError;
use crate::common::structs::Response;
use crate::common::types::{Ptr, PtrString};
use crate::common::utils::recover_c_char;

#[no_mangle]
pub unsafe extern "C" fn eth_parse_contract_data(
    input_data: PtrString,
    contract_json: PtrString,
) -> Ptr<Response<DisplayContractData>> {
    let input_data = recover_c_char(input_data);
    let contract_json = recover_c_char(contract_json);
    let input_data =
        hex::decode(input_data.clone()).map_err(|_e| RustCError::InvalidHex(input_data.clone()));
    match input_data {
        Ok(_input_data) => {
            let result = app_ethereum::abi::parse_contract_data(_input_data, contract_json);
            match result {
                Ok(v) => Response::success_ptr(DisplayContractData::from(v).c_ptr()).c_ptr(),
                Err(e) => Response::from(e).c_ptr(),
            }
        }
        Err(e) => Response::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn eth_parse_swapkit_contract(
    input_data: PtrString,
    contract_json: PtrString,
) -> Ptr<Response<DisplaySwapkitContractData>> {
    let input_data = recover_c_char(input_data);
    let contract_json = recover_c_char(contract_json);
    let input_data =
        hex::decode(input_data.clone()).map_err(|_e| RustCError::InvalidHex(input_data.clone()));
    match input_data {
        Ok(_input_data) => {
            let result = app_ethereum::swap::parse_swapkit_contract(_input_data, contract_json);
            match result {
                Ok(v) => Response::success_ptr(DisplaySwapkitContractData::from(v).c_ptr()).c_ptr(),
                Err(e) => Response::from(e).c_ptr(),
            }
        }
        Err(e) => Response::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn eth_parse_contract_data_by_method(
    input_data: PtrString,
    contract_name: PtrString,
    contract_method_json: PtrString,
) -> Ptr<Response<DisplayContractData>> {
    let input_data = recover_c_char(input_data);
    let contract_name = recover_c_char(contract_name);
    let contract_method_json = recover_c_char(contract_method_json);
    let input_data =
        hex::decode(input_data.clone()).map_err(|_e| RustCError::InvalidHex(input_data.clone()));
    match input_data {
        Ok(_input_data) => {
            let result = app_ethereum::abi::parse_method_data(
                _input_data,
                contract_name,
                contract_method_json,
            );
            match result {
                Ok(v) => Response::success(DisplayContractData::from(v)).c_ptr(),
                Err(e) => Response::from(e).c_ptr(),
            }
        }
        Err(e) => Response::from(e).c_ptr(),
    }
}
