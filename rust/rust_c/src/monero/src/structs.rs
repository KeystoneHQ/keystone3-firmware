use alloc::string::{String, ToString};
use alloc::vec::Vec;
use alloc::boxed::Box;
use common_rust_c::free::Free;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{PtrString, PtrT};
use app_monero::transfer::DisplayTransactionInfo;
use app_monero::outputs::DisplayMoneroOutput as InnerDisplayMoneroOutput;
use common_rust_c::ffi::VecFFI;

use common_rust_c::utils::convert_c_char;
use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};

#[repr(C)]
pub struct DisplayMoneroOutput {
    pub txos_num: PtrString,
    pub total_amount: PtrString,
}

#[repr(C)]
pub struct DisplayMoneroUnsignedTxOutput {
    address: PtrString,
    amount: PtrString,
}

impl Free for DisplayMoneroUnsignedTxOutput {
    fn free(&self) {
        free_str_ptr!(self.address);
        free_str_ptr!(self.amount);
    }
}

#[repr(C)]
pub struct DisplayMoneroUnsignedTxInput {
    pub key: PtrString,
    pub amount: PtrString,
}

impl Free for DisplayMoneroUnsignedTxInput {
    fn free(&self) {
        free_str_ptr!(self.key);
        free_str_ptr!(self.amount);
    }
}

#[repr(C)]
pub struct DisplayMoneroUnsignedTx {
    pub outputs: PtrT<VecFFI<DisplayMoneroUnsignedTxOutput>>,
    pub inputs: PtrT<VecFFI<DisplayMoneroUnsignedTxInput>>,
    pub input_amount: PtrString,
    pub output_amount: PtrString,
    pub fee: PtrString,
}

impl From<DisplayTransactionInfo> for DisplayMoneroUnsignedTx {
    fn from(value: DisplayTransactionInfo) -> Self {
        Self {
            outputs: VecFFI::from(
                value.outputs.iter().map(|output| DisplayMoneroUnsignedTxOutput {
                    address: convert_c_char(output.0.to_string()),
                    amount: convert_c_char(output.1.to_string()),
                })
                .collect::<Vec<DisplayMoneroUnsignedTxOutput>>(),
            )
            .c_ptr(),
            inputs: VecFFI::from(
                value.inputs.iter().map(|input| DisplayMoneroUnsignedTxInput {
                    key: convert_c_char(input.0.to_string()),
                    amount: convert_c_char(input.1.to_string()),
                })
                .collect::<Vec<DisplayMoneroUnsignedTxInput>>(),
            )
            .c_ptr(),
            input_amount: convert_c_char(value.input_amount.to_string()),
            output_amount: convert_c_char(value.output_amount.to_string()),
            fee: convert_c_char(value.fee.to_string()),
        }
    }
}

impl_c_ptr!(DisplayMoneroOutput);
impl_c_ptr!(DisplayMoneroUnsignedTx);

impl Free for DisplayMoneroOutput {
    fn free(&self) {
        free_str_ptr!(self.txos_num);
        free_str_ptr!(self.total_amount);
    }
}

impl Free for DisplayMoneroUnsignedTx {
    fn free(&self) {
        unsafe {
            let x = Box::from_raw(self.outputs);
            let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
            ve.iter().for_each(|v| {
                v.free();
            });
            let x = Box::from_raw(self.inputs);
            let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
            ve.iter().for_each(|v| {
                v.free();
            });

            free_str_ptr!(self.input_amount);
            free_str_ptr!(self.output_amount);
            free_str_ptr!(self.fee);
        }
    }
}

impl From<InnerDisplayMoneroOutput> for DisplayMoneroOutput {
    fn from(value: InnerDisplayMoneroOutput) -> Self {
        Self {
            txos_num: convert_c_char(value.txos_num.to_string()),
            total_amount: convert_c_char(value.total_amount.to_string()),
        }
    }
}

make_free_method!(TransactionParseResult<DisplayMoneroOutput>);
make_free_method!(TransactionParseResult<DisplayMoneroUnsignedTx>);
