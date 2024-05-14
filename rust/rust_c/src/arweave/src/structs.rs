use common_rust_c::free::Free;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{Ptr, PtrString, PtrT};
use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};

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
