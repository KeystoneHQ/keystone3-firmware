use crate::common::{
    structs::TransactionParseResult,
    types::{PtrT, PtrUR},
};
use crate::extract_ptr_with_type;
use app_arweave::{ao_transaction::AOTransferTransaction, parse_data_item};
use ur_registry::arweave::arweave_sign_request::ArweaveSignRequest;

use super::structs::{DisplayArweaveAOTransfer, DisplayArweaveDataItem};

#[no_mangle]
pub unsafe extern "C" fn ar_is_ao_transfer(ptr: PtrUR) -> bool {
    let sign_request = extract_ptr_with_type!(ptr, ArweaveSignRequest);
    let sign_data = sign_request.get_sign_data();
    let data_item = parse_data_item(&sign_data);
    match data_item {
        Ok(item) => match AOTransferTransaction::try_from(item) {
            Ok(_ao_transfer) => true,
            Err(_e) => false,
        },
        Err(_e) => false,
    }
}

#[no_mangle]
pub unsafe extern "C" fn ar_parse_data_item(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayArweaveDataItem>> {
    let sign_request = extract_ptr_with_type!(ptr, ArweaveSignRequest);
    let sign_data = sign_request.get_sign_data();
    let data_item = parse_data_item(&sign_data);
    match data_item {
        Ok(item) => {
            TransactionParseResult::success(DisplayArweaveDataItem::from(item).c_ptr()).c_ptr()
        }
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn ar_parse_ao_transfer(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayArweaveAOTransfer>> {
    let sign_request = extract_ptr_with_type!(ptr, ArweaveSignRequest);
    let sign_data = sign_request.get_sign_data();
    let data_item = parse_data_item(&sign_data);
    match data_item {
        Ok(item) => match AOTransferTransaction::try_from(item) {
            Ok(ao_transfer) => {
                TransactionParseResult::success(DisplayArweaveAOTransfer::from(ao_transfer).c_ptr())
                    .c_ptr()
            }
            Err(e) => TransactionParseResult::from(e).c_ptr(),
        },
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}
