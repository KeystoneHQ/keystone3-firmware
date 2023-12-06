#![allow(dead_code)]
use alloc::boxed::Box;

use alloc::string::String;
use alloc::vec::Vec;
use app_ethereum::abi::{ContractData, ContractMethodParam};
use app_ethereum::structs::{ParsedEthereumTransaction, PersonalMessage, TypedData};
use common_rust_c::ffi::VecFFI;
use common_rust_c::free::Free;
use common_rust_c::structs::{Response, TransactionParseResult};
use common_rust_c::types::{PtrString, PtrT};
use common_rust_c::utils::convert_c_char;
use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};
use core::ptr::null_mut;
use third_party::itertools::Itertools;
use third_party::ur_registry::ethereum::eth_sign_request::DataType;

#[repr(C)]
pub struct DisplayETH {
    tx_type: PtrString,
    chain_id: u64,
    overview: PtrT<DisplayETHOverview>,
    detail: PtrT<DisplayETHDetail>,
}

impl_c_ptr!(DisplayETH);

#[repr(C)]
pub struct DisplayETHOverview {
    value: PtrString,

    max_txn_fee: PtrString,
    gas_price: PtrString,
    gas_limit: PtrString,

    from: PtrString,
    to: PtrString,
}

impl_c_ptr!(DisplayETHOverview);

impl Free for DisplayETHOverview {
    fn free(&self) {
        free_str_ptr!(self.value);
        free_str_ptr!(self.max_txn_fee);
        free_str_ptr!(self.gas_price);
        free_str_ptr!(self.gas_limit);
        free_str_ptr!(self.from);
        free_str_ptr!(self.to);
    }
}

#[repr(C)]
pub struct DisplayETHDetail {
    value: PtrString,

    max_txn_fee: PtrString,

    max_fee: PtrString,
    max_priority: PtrString,

    max_fee_price: PtrString,
    max_priority_price: PtrString,

    gas_price: PtrString,
    gas_limit: PtrString,

    from: PtrString,
    to: PtrString,

    input: PtrString,
}

impl_c_ptr!(DisplayETHDetail);

impl Free for DisplayETHDetail {
    fn free(&self) {
        free_str_ptr!(self.value);
        free_str_ptr!(self.max_txn_fee);
        free_str_ptr!(self.gas_price);
        free_str_ptr!(self.gas_limit);
        free_str_ptr!(self.from);
        free_str_ptr!(self.to);
        free_str_ptr!(self.max_fee);
        free_str_ptr!(self.max_priority);
        free_str_ptr!(self.max_fee_price);
        free_str_ptr!(self.max_priority_price);
        free_str_ptr!(self.input);
    }
}

impl Free for DisplayETH {
    fn free(&self) {
        unsafe {
            let x = Box::from_raw(self.overview);
            x.free();
            free_str_ptr!(self.tx_type);
            let y = Box::from_raw(self.detail);
            y.free();
        }
    }
}

impl From<ParsedEthereumTransaction> for DisplayETH {
    fn from(value: ParsedEthereumTransaction) -> Self {
        Self {
            tx_type: match value.max_priority_fee_per_gas {
                Some(_) => convert_c_char(String::from("FeeMarket")),
                None => convert_c_char(String::from("Legacy")),
            },
            chain_id: value.chain_id,
            overview: DisplayETHOverview::from(value.clone()).c_ptr(),
            detail: DisplayETHDetail::from(value.clone()).c_ptr(),
        }
    }
}

impl From<ParsedEthereumTransaction> for DisplayETHOverview {
    fn from(tx: ParsedEthereumTransaction) -> Self {
        Self {
            value: convert_c_char(tx.value),
            max_txn_fee: convert_c_char(tx.max_txn_fee),
            gas_price: match tx.gas_price {
                None => null_mut(),
                Some(s) => convert_c_char(s),
            },
            gas_limit: convert_c_char(tx.gas_limit),
            from: convert_c_char(tx.from),
            to: convert_c_char(tx.to),
        }
    }
}

impl From<ParsedEthereumTransaction> for DisplayETHDetail {
    fn from(tx: ParsedEthereumTransaction) -> Self {
        Self {
            value: convert_c_char(tx.value),
            max_txn_fee: convert_c_char(tx.max_txn_fee),
            max_fee: tx.max_fee.map(convert_c_char).unwrap_or(null_mut()),
            max_priority: tx.max_priority.map(convert_c_char).unwrap_or(null_mut()),
            max_fee_price: tx.max_fee_per_gas.map(convert_c_char).unwrap_or(null_mut()),
            max_priority_price: tx
                .max_priority_fee_per_gas
                .map(convert_c_char)
                .unwrap_or(null_mut()),
            gas_price: tx.gas_price.map(convert_c_char).unwrap_or(null_mut()),
            gas_limit: convert_c_char(tx.gas_limit),
            from: convert_c_char(tx.from),
            to: convert_c_char(tx.to),
            input: convert_c_char(tx.input),
        }
    }
}

#[repr(C)]
pub struct DisplayETHPersonalMessage {
    raw_message: PtrString,
    utf8_message: PtrString,
    from: PtrString,
}

impl From<PersonalMessage> for DisplayETHPersonalMessage {
    fn from(message: PersonalMessage) -> Self {
        Self {
            raw_message: convert_c_char(message.raw_message),
            utf8_message: if message.utf8_message.is_empty() {
                null_mut()
            } else {
                convert_c_char(message.utf8_message)
            },
            from: convert_c_char(message.from),
        }
    }
}

impl_c_ptr!(DisplayETHPersonalMessage);

impl Free for DisplayETHPersonalMessage {
    fn free(&self) {
        free_str_ptr!(self.raw_message);
        free_str_ptr!(self.utf8_message);
        free_str_ptr!(self.from);
    }
}

#[repr(C)]
pub struct DisplayETHTypedData {
    name: PtrString,
    version: PtrString,
    chain_id: PtrString,
    verifying_contract: PtrString,
    salt: PtrString,
    primary_type: PtrString,
    message: PtrString,
    from: PtrString,
}

impl From<TypedData> for DisplayETHTypedData {
    fn from(message: TypedData) -> Self {
        fn to_ptr_string(string: String) -> PtrString {
            if string.is_empty() {
                null_mut()
            } else {
                convert_c_char(string)
            }
        }

        Self {
            name: to_ptr_string(message.name),
            version: to_ptr_string(message.version),
            chain_id: to_ptr_string(message.chain_id),
            verifying_contract: to_ptr_string(message.verifying_contract),
            salt: to_ptr_string(message.salt),
            primary_type: to_ptr_string(message.primary_type),
            message: to_ptr_string(message.message),
            from: to_ptr_string(message.from),
        }
    }
}

impl_c_ptr!(DisplayETHTypedData);

impl Free for DisplayETHTypedData {
    fn free(&self) {
        free_str_ptr!(self.name);
        free_str_ptr!(self.version);
        free_str_ptr!(self.chain_id);
        free_str_ptr!(self.verifying_contract);
        free_str_ptr!(self.salt);
        free_str_ptr!(self.primary_type);
        free_str_ptr!(self.message);
        free_str_ptr!(self.from);
    }
}

pub enum TransactionType {
    Legacy,
    TypedTransaction,
    PersonalMessage,
    TypedData,
}

impl From<DataType> for TransactionType {
    fn from(value: DataType) -> Self {
        match value {
            DataType::Transaction => TransactionType::Legacy,
            DataType::TypedTransaction => TransactionType::TypedTransaction,
            DataType::TypedData => TransactionType::TypedData,
            DataType::PersonalMessage => TransactionType::PersonalMessage,
        }
    }
}

#[repr(C)]
pub struct DisplayContractData {
    pub contract_name: PtrString,
    pub method_name: PtrString,
    pub params: PtrT<VecFFI<DisplayContractParam>>,
}

impl_c_ptr!(DisplayContractData);

impl From<ContractData> for DisplayContractData {
    fn from(value: ContractData) -> Self {
        Self {
            contract_name: convert_c_char(value.get_contract_name()),
            method_name: convert_c_char(value.get_method_name()),
            params: VecFFI::from(
                value
                    .get_params()
                    .iter()
                    .map(|v| DisplayContractParam::from(v))
                    .collect_vec(),
            )
            .c_ptr(),
        }
    }
}

impl Free for DisplayContractData {
    fn free(&self) {
        free_str_ptr!(self.method_name);
        free_str_ptr!(self.contract_name);
        unsafe {
            let x = Box::from_raw(self.params);
            let v = Vec::from_raw_parts(x.data, x.size, x.cap);
            for x in v {
                x.free()
            }
        }
    }
}

#[repr(C)]
pub struct DisplayContractParam {
    pub name: PtrString,
    pub value: PtrString,
}

impl From<&ContractMethodParam> for DisplayContractParam {
    fn from(value: &ContractMethodParam) -> Self {
        Self {
            name: convert_c_char(value.get_name()),
            value: convert_c_char(value.get_value()),
        }
    }
}

impl Free for DisplayContractParam {
    fn free(&self) {
        free_str_ptr!(self.name);
        free_str_ptr!(self.value);
    }
}

impl_c_ptr!(DisplayContractParam);

#[repr(C)]
pub struct EthParsedErc20Transaction {
    pub to: PtrString,
    pub value: PtrString,
}

impl From<app_ethereum::erc20::ParsedErc20Transaction> for EthParsedErc20Transaction {
    fn from(value: app_ethereum::erc20::ParsedErc20Transaction) -> Self {
        Self {
            to: convert_c_char(value.to),
            value: convert_c_char(value.value),
        }
    }
}

impl Free for EthParsedErc20Transaction {
    fn free(&self) {
        free_str_ptr!(self.to);
        free_str_ptr!(self.value);
    }
}

impl_c_ptr!(EthParsedErc20Transaction);

make_free_method!(TransactionParseResult<DisplayETH>);
make_free_method!(TransactionParseResult<DisplayETHPersonalMessage>);
make_free_method!(TransactionParseResult<DisplayETHTypedData>);
make_free_method!(Response<DisplayContractData>);
make_free_method!(TransactionParseResult<EthParsedErc20Transaction>);
