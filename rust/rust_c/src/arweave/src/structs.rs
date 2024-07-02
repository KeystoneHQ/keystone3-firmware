use core::ptr::null_mut;

use alloc::vec::Vec;
use app_arweave::{
    ao_transaction::AOTransferTransaction,
    data_item::{DataItem, Tag},
};
use common_rust_c::free::Free;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{Ptr, PtrString, PtrT};
use common_rust_c::utils::convert_c_char;
use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};
use common_rust_c::{ffi::VecFFI, free_vec};

#[repr(C)]
pub enum ArweaveRequestType {
    ArweaveRequestTypeTransaction = 1,
    ArweaveRequestTypeDataItem,
    ArweaveRequestTypeMessage,
    ArweaveRequestTypeUnknown,
}

#[repr(C)]
pub struct DisplayArweaveTx {
    pub value: PtrString,
    pub fee: PtrString,
    pub from: PtrString,
    pub to: PtrString,
    pub detail: PtrString,
}

#[repr(C)]
pub struct DisplayArweaveMessage {
    pub message: PtrString,
    pub raw_message: PtrString,
}

impl Free for DisplayArweaveTx {
    fn free(&self) {
        free_str_ptr!(self.value);
        free_str_ptr!(self.fee);
        free_str_ptr!(self.from);
        free_str_ptr!(self.to);
    }
}

impl Free for DisplayArweaveMessage {
    fn free(&self) {
        free_str_ptr!(self.message);
        free_str_ptr!(self.raw_message);
    }
}

make_free_method!(TransactionParseResult<DisplayArweaveTx>);
make_free_method!(TransactionParseResult<DisplayArweaveMessage>);

#[repr(C)]
pub enum DisplayDataItemType {
    DataItem,
    AOTransfer,
}

#[repr(C)]
pub struct DisplayTag {
    name: PtrString,
    value: PtrString,
}

impl From<Tag> for DisplayTag {
    fn from(value: Tag) -> Self {
        Self {
            name: convert_c_char(value.get_name()),
            value: convert_c_char(value.get_value()),
        }
    }
}

impl Free for DisplayTag {
    fn free(&self) {
        free_str_ptr!(self.name);
        free_str_ptr!(self.value);
    }
}

#[repr(C)]
pub struct DisplayArweaveDataItem {
    owner: PtrString,
    target: PtrString,
    anchor: PtrString,
    tags: Ptr<VecFFI<DisplayTag>>,
    data: PtrString,
}

impl From<DataItem> for DisplayArweaveDataItem {
    fn from(value: DataItem) -> Self {
        Self {
            owner: convert_c_char(value.get_owner()),
            target: value.get_target().map(convert_c_char).unwrap_or(null_mut()),
            anchor: value.get_anchor().map(convert_c_char).unwrap_or(null_mut()),
            tags: VecFFI::from(
                value
                    .get_tags()
                    .get_data()
                    .iter()
                    .map(|v| DisplayTag::from(v.clone()))
                    .collect::<Vec<DisplayTag>>(),
            )
            .c_ptr(),
            data: convert_c_char(value.get_data()),
        }
    }
}

impl Free for DisplayArweaveDataItem {
    fn free(&self) {
        free_str_ptr!(self.owner);
        free_str_ptr!(self.target);
        free_str_ptr!(self.anchor);
        free_str_ptr!(self.data);
        free_vec!(self.tags)
    }
}

impl_c_ptr!(DisplayArweaveDataItem);

#[repr(C)]
pub struct DisplayArweaveAOTransfer {
    from: PtrString,
    to: PtrString,
    quantity: PtrString,
    token_id: PtrString,
    other_info: Ptr<VecFFI<DisplayTag>>,
}

impl From<AOTransferTransaction> for DisplayArweaveAOTransfer {
    fn from(value: AOTransferTransaction) -> Self {
        Self {
            from: convert_c_char(value.get_from()),
            to: convert_c_char(value.get_to()),
            quantity: convert_c_char(value.get_quantity()),
            other_info: VecFFI::from(
                value
                    .get_other_info()
                    .iter()
                    .map(|v| DisplayTag::from(v.clone()))
                    .collect::<Vec<DisplayTag>>(),
            )
            .c_ptr(),
            token_id: convert_c_char(value.get_token_id()),
        }
    }
}

impl Free for DisplayArweaveAOTransfer {
    fn free(&self) {
        free_str_ptr!(self.from);
        free_str_ptr!(self.to);
        free_str_ptr!(self.token_id);
        free_str_ptr!(self.quantity);
        free_vec!(self.other_info)
    }
}

impl_c_ptr!(DisplayArweaveAOTransfer);
