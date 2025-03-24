#![allow(dead_code)]
use alloc::boxed::Box;

use super::util::{calculate_max_txn_fee, convert_wei_to_eth};
use crate::common::ffi::VecFFI;
use crate::common::free::Free;
use crate::common::structs::{Response, TransactionParseResult};
use crate::common::types::{PtrString, PtrT};
use crate::common::utils::convert_c_char;
use crate::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use app_ethereum::abi::{ContractData, ContractMethodParam};
use app_ethereum::erc20::encode_erc20_transfer_calldata;
use app_ethereum::structs::{ParsedEthereumTransaction, PersonalMessage, TypedData};
use core::ptr::null_mut;
use core::str::FromStr;
use itertools::Itertools;
use ur_registry::ethereum::eth_sign_request::DataType;
use ur_registry::pb::protoc::EthTx;

#[repr(C)]
pub struct DisplayETH {
    pub(crate) tx_type: PtrString,
    pub(crate) chain_id: u64,
    pub(crate) overview: PtrT<DisplayETHOverview>,
    pub(crate) detail: PtrT<DisplayETHDetail>,
}

impl_c_ptr!(DisplayETH);

impl DisplayETH {
    pub fn set_from_address(self, from_address: String) -> DisplayETH {
        unsafe {
            let overview = &mut *self.overview;
            overview.from = convert_c_char(from_address.clone());

            let detail = &mut *self.detail;
            detail.from = convert_c_char(from_address);
        }
        self
    }
}

impl TryFrom<EthTx> for DisplayETH {
    type Error = ();
    fn try_from(eth_tx: EthTx) -> Result<Self, Self::Error> {
        let temp_from_address = "".to_string();
        // check this transaction is erc20 transaction or not
        if let Some(erc20_override) = eth_tx.r#override {
            let contract_address = erc20_override.contract_address;
            let display_tx_overview = DisplayETHOverview {
                value: convert_c_char(convert_wei_to_eth("0")),
                max_txn_fee: convert_c_char(convert_wei_to_eth(&calculate_max_txn_fee(
                    &eth_tx.gas_price,
                    &eth_tx.gas_limit,
                ))),
                gas_price: convert_c_char(eth_tx.gas_price.clone()),
                gas_limit: convert_c_char(eth_tx.gas_limit.clone()),
                from: convert_c_char(temp_from_address.clone()),
                to: convert_c_char(contract_address.clone()),
            };

            // calculate erc20 transfer inputdata
            let to = app_ethereum::H160::from_str(&eth_tx.to).unwrap();
            let amount = app_ethereum::U256::from(eth_tx.value.parse::<u64>().unwrap());
            let input_data = encode_erc20_transfer_calldata(to, amount);

            let display_tx_detail = DisplayETHDetail {
                value: convert_c_char(convert_wei_to_eth("0")),
                max_txn_fee: convert_c_char(convert_wei_to_eth(&calculate_max_txn_fee(
                    &eth_tx.gas_price,
                    &eth_tx.gas_limit,
                ))),
                max_fee: convert_c_char(eth_tx.gas_price.clone()),
                max_priority: convert_c_char(eth_tx.gas_price.clone()),
                max_fee_price: convert_c_char(eth_tx.gas_price.clone()),
                max_priority_price: convert_c_char(eth_tx.gas_price.clone()),
                gas_price: convert_c_char(eth_tx.gas_price),
                gas_limit: convert_c_char(eth_tx.gas_limit),
                from: convert_c_char(temp_from_address.clone()),
                to: convert_c_char(contract_address),
                input: convert_c_char(input_data),
            };
            let display_eth = DisplayETH {
                tx_type: convert_c_char("Legacy".to_string()),
                chain_id: 1,
                overview: display_tx_overview.c_ptr(),
                detail: display_tx_detail.c_ptr(),
            };
            Ok(display_eth)
        } else {
            let display_tx_overview = DisplayETHOverview {
                value: convert_c_char(convert_wei_to_eth(&eth_tx.value)),
                max_txn_fee: convert_c_char(convert_wei_to_eth(&calculate_max_txn_fee(
                    &eth_tx.gas_price,
                    &eth_tx.gas_limit,
                ))),
                gas_price: convert_c_char(eth_tx.gas_price.clone()),
                gas_limit: convert_c_char(eth_tx.gas_limit.clone()),
                from: convert_c_char(temp_from_address.clone()),
                to: convert_c_char(eth_tx.to.clone()),
            };

            let display_tx_detail = DisplayETHDetail {
                value: convert_c_char(convert_wei_to_eth(&eth_tx.value)),
                max_txn_fee: convert_c_char(convert_wei_to_eth(&calculate_max_txn_fee(
                    &eth_tx.gas_price,
                    &eth_tx.gas_limit,
                ))),
                max_fee: convert_c_char(eth_tx.gas_price.clone()),
                max_priority: convert_c_char(eth_tx.gas_price.clone()),
                max_fee_price: convert_c_char(eth_tx.gas_price.clone()),
                max_priority_price: convert_c_char(eth_tx.gas_price.clone()),
                gas_price: convert_c_char(eth_tx.gas_price),
                gas_limit: convert_c_char(eth_tx.gas_limit),
                from: convert_c_char(temp_from_address.clone()),
                to: convert_c_char(eth_tx.to),
                input: convert_c_char(eth_tx.memo),
            };
            let display_eth = DisplayETH {
                tx_type: convert_c_char("Legacy".to_string()),
                chain_id: 1,
                overview: display_tx_overview.c_ptr(),
                detail: display_tx_detail.c_ptr(),
            };
            Ok(display_eth)
        }
    }
}

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
// parse evm transaction detail
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
                    .map(DisplayContractParam::from)
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
