/// AccountId, (name, address) use name, (null, address) use address, (name, null) use name,
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct AccountId {
    #[prost(bytes = "vec", tag = "1")]
    pub name: ::prost::alloc::vec::Vec<u8>,
    #[prost(bytes = "vec", tag = "2")]
    pub address: ::prost::alloc::vec::Vec<u8>,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct Authority {
    #[prost(message, optional, tag = "1")]
    pub account: ::core::option::Option<AccountId>,
    #[prost(bytes = "vec", tag = "2")]
    pub permission_name: ::prost::alloc::vec::Vec<u8>,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct Transaction {
    #[prost(message, optional, tag = "1")]
    pub raw_data: ::core::option::Option<transaction::Raw>,
    /// only support size = 1,  repeated list here for muti-sig extension
    #[prost(bytes = "vec", repeated, tag = "2")]
    pub signature: ::prost::alloc::vec::Vec<::prost::alloc::vec::Vec<u8>>,
}
/// Nested message and enum types in `Transaction`.
pub mod transaction {
    #[allow(clippy::derive_partial_eq_without_eq)]
    #[derive(Clone, PartialEq, ::prost::Message)]
    pub struct Contract {
        #[prost(enumeration = "contract::ContractType", tag = "1")]
        pub r#type: i32,
        #[prost(message, optional, tag = "2")]
        pub parameter: ::core::option::Option<::prost_types::Any>,
        #[prost(bytes = "vec", tag = "3")]
        pub provider: ::prost::alloc::vec::Vec<u8>,
        #[prost(bytes = "vec", tag = "4")]
        pub contract_name: ::prost::alloc::vec::Vec<u8>,
        #[prost(int32, tag = "5")]
        pub permission_id: i32,
    }
    /// Nested message and enum types in `Contract`.
    pub mod contract {
        #[derive(
            Clone, Copy, Debug, PartialEq, Eq, Hash, PartialOrd, Ord, ::prost::Enumeration,
        )]
        #[repr(i32)]
        pub enum ContractType {
            AccountCreateContract = 0,
            TransferContract = 1,
            TransferAssetContract = 2,
            TriggerSmartContract = 31,
        }
        impl ContractType {
            /// String value of the enum field names used in the ProtoBuf definition.
            ///
            /// The values are not transformed in any way and thus are considered stable
            /// (if the ProtoBuf definition does not change) and safe for programmatic use.
            pub fn as_str_name(&self) -> &'static str {
                match self {
                    ContractType::AccountCreateContract => "AccountCreateContract",
                    ContractType::TransferContract => "TransferContract",
                    ContractType::TransferAssetContract => "TransferAssetContract",
                    ContractType::TriggerSmartContract => "TriggerSmartContract",
                }
            }
            /// Creates an enum from field names used in the ProtoBuf definition.
            pub fn from_str_name(value: &str) -> ::core::option::Option<Self> {
                match value {
                    "AccountCreateContract" => Some(Self::AccountCreateContract),
                    "TransferContract" => Some(Self::TransferContract),
                    "TransferAssetContract" => Some(Self::TransferAssetContract),
                    "TriggerSmartContract" => Some(Self::TriggerSmartContract),
                    _ => None,
                }
            }
        }
    }
    #[allow(clippy::derive_partial_eq_without_eq)]
    #[derive(Clone, PartialEq, ::prost::Message)]
    pub struct Raw {
        #[prost(bytes = "vec", tag = "1")]
        pub ref_block_bytes: ::prost::alloc::vec::Vec<u8>,
        #[prost(int64, tag = "3")]
        pub ref_block_num: i64,
        #[prost(bytes = "vec", tag = "4")]
        pub ref_block_hash: ::prost::alloc::vec::Vec<u8>,
        #[prost(int64, tag = "8")]
        pub expiration: i64,
        #[prost(message, repeated, tag = "9")]
        pub auths: ::prost::alloc::vec::Vec<super::Authority>,
        /// data not used
        #[prost(bytes = "vec", tag = "10")]
        pub data: ::prost::alloc::vec::Vec<u8>,
        /// only support size = 1,  repeated list here for extension
        #[prost(message, repeated, tag = "11")]
        pub contract: ::prost::alloc::vec::Vec<Contract>,
        /// scripts not used
        #[prost(bytes = "vec", tag = "12")]
        pub scripts: ::prost::alloc::vec::Vec<u8>,
        #[prost(int64, tag = "14")]
        pub timestamp: i64,
        #[prost(int64, tag = "18")]
        pub fee_limit: i64,
    }
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct TransferContract {
    #[prost(bytes = "vec", tag = "1")]
    pub owner_address: ::prost::alloc::vec::Vec<u8>,
    #[prost(bytes = "vec", tag = "2")]
    pub to_address: ::prost::alloc::vec::Vec<u8>,
    #[prost(int64, tag = "3")]
    pub amount: i64,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct TransferAssetContract {
    /// this field is token name before the proposal ALLOW_SAME_TOKEN_NAME is active, otherwise it is token id and token is should be in string format.
    #[prost(bytes = "vec", tag = "1")]
    pub asset_name: ::prost::alloc::vec::Vec<u8>,
    #[prost(bytes = "vec", tag = "2")]
    pub owner_address: ::prost::alloc::vec::Vec<u8>,
    #[prost(bytes = "vec", tag = "3")]
    pub to_address: ::prost::alloc::vec::Vec<u8>,
    #[prost(int64, tag = "4")]
    pub amount: i64,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct TriggerSmartContract {
    #[prost(bytes = "vec", tag = "1")]
    pub owner_address: ::prost::alloc::vec::Vec<u8>,
    #[prost(bytes = "vec", tag = "2")]
    pub contract_address: ::prost::alloc::vec::Vec<u8>,
    #[prost(int64, tag = "3")]
    pub call_value: i64,
    #[prost(bytes = "vec", tag = "4")]
    pub data: ::prost::alloc::vec::Vec<u8>,
    #[prost(int64, tag = "5")]
    pub call_token_value: i64,
    #[prost(int64, tag = "6")]
    pub token_id: i64,
}
