use crate::common::errors::{RustCError, R};
use crate::common::structs::{TransactionCheckResult, TransactionParseResult};
use crate::common::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::recover_c_char;
use crate::ergo::structs::DisplayErgoTx;
use crate::extract_ptr_with_type;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use alloc::{format, slice};
use app_ergo::errors::ErgoError;
use app_ergo::structs::{ErgoUnspentBox, ParseContext, UnparsedErgoAsset};
use itertools::Itertools;
use ur_registry::ergo::ergo_sign_request::ErgoSignRequest;
use ur_registry::ergo::ergo_signed_tx::ErgoSignedTx;
use ur_registry::registry_types::ERGO_SIGNED_TX;

#[no_mangle]
pub extern "C" fn ergo_parse_tx(
    ptr: PtrUR,
    x_pub: PtrString,
) -> PtrT<TransactionParseResult<DisplayErgoTx>> {
    let ergo_sign_request = extract_ptr_with_type!(ptr, ErgoSignRequest);
    let x_pub = recover_c_char(x_pub);
    let tx_data = ergo_sign_request.get_sign_data();
    let parse_context = prepare_parse_context(&ergo_sign_request, x_pub);
    match parse_context {
        Ok(parse_context) => match app_ergo::transaction::parse_tx(tx_data, parse_context) {
            Ok(v) => TransactionParseResult::success(DisplayErgoTx::from(v).c_ptr()).c_ptr(),
            Err(e) => TransactionParseResult::from(e).c_ptr(),
        },
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn ergo_check_tx(ptr: PtrUR, x_pub: PtrString) -> PtrT<TransactionCheckResult> {
    let ergo_sign_request = extract_ptr_with_type!(ptr, ErgoSignRequest);
    let x_pub = recover_c_char(x_pub);
    let tx_data = ergo_sign_request.get_sign_data();
    let parse_context = prepare_parse_context(&ergo_sign_request, x_pub);
    match parse_context {
        Ok(parse_context) => match app_ergo::transaction::check_tx(tx_data, parse_context) {
            Ok(_) => TransactionCheckResult::new().c_ptr(),
            Err(e) => TransactionCheckResult::from(e).c_ptr(),
        },
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}

fn prepare_parse_context(ergo_sign_request: &ErgoSignRequest, x_pub: String) -> R<ParseContext> {
    let utxo: Result<Vec<ErgoUnspentBox>, ErgoError> = ergo_sign_request
        .get_boxes()
        .iter()
        .map(
            |ergo_box| match app_ergo::address::ergo_tree_to_address(ergo_box.get_ergo_tree()) {
                Ok(address) => Ok(ErgoUnspentBox::new(
                    ergo_box.get_box_id(),
                    address,
                    ergo_box.get_value(),
                    match ergo_box.get_assets() {
                        None => None,
                        Some(v) => Some(
                            v.iter()
                                .map(|a| UnparsedErgoAsset::new(a.get_token_id(), a.get_amount()))
                                .collect_vec(),
                        ),
                    },
                )),
                Err(e) => Err(e),
            },
        )
        .try_collect();

    let addresses = ergo_sign_request
        .get_derivation_paths()
        .iter()
        .map(|derivation_path| derivation_path.get_path())
        .flatten()
        .map(|derivation_path| {
            app_ergo::address::get_address(format!("m/{}", derivation_path), &x_pub)
        })
        .filter_map(|address| address.ok())
        .collect_vec();

    match utxo {
        Ok(u) => Ok(ParseContext::new(u, addresses)),
        Err(e) => Err(RustCError::InvalidData(e.to_string())),
    }
}

#[no_mangle]
pub extern "C" fn ergo_sign_tx(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    entropy: PtrBytes,
    entropy_len: u32,
) -> PtrT<UREncodeResult> {
    let ergo_sign_request = extract_ptr_with_type!(ptr, ErgoSignRequest);
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let entropy = unsafe { slice::from_raw_parts(entropy, entropy_len as usize) };

    let derivation_paths = ergo_sign_request
        .get_derivation_paths()
        .iter()
        .map(|derivation_path| derivation_path.get_path())
        .flatten()
        .map(|derivation_path| format!("m/{}", derivation_path))
        .collect_vec();

    let tx_data = ergo_sign_request.get_sign_data();

    let sign_result = app_ergo::transaction::sign_tx(tx_data, seed, entropy, &derivation_paths);

    match sign_result.map(|v| ErgoSignedTx::new(ergo_sign_request.get_request_id(), v).try_into()) {
        Ok(v) => match v {
            Ok(data) => UREncodeResult::encode(
                data,
                ERGO_SIGNED_TX.get_type(),
                FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
            )
            .c_ptr(),
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}
