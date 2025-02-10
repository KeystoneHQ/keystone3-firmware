#[macro_export]
macro_rules! impl_c_ptr {
    ($name:ident) => {
        impl $name {
            #[cfg(feature = "debug-memory")]
            #[track_caller]
            pub fn c_ptr(self) -> *mut Self {
                let x = alloc::boxed::Box::into_raw(alloc::boxed::Box::new(self));
                rust_tools::debug!(alloc::format!(
                    "Rust Ptr: {}#{:p}, called from: {}",
                    core::stringify!($name),
                    x,
                    core::panic::Location::caller()
                ));
                x
            }
            #[cfg(not(feature = "debug-memory"))]
            pub fn c_ptr(self) -> *mut Self {
                alloc::boxed::Box::into_raw(alloc::boxed::Box::new(self))
            }
        }
    };
    ($name:ident<$t: ident>) => {
        impl<$t> $name<$t> {
            #[cfg(feature = "debug-memory")]
            #[track_caller]
            pub fn c_ptr(self) -> *mut Self {
                let x = alloc::boxed::Box::into_raw(alloc::boxed::Box::new(self));
                rust_tools::debug!(alloc::format!(
                    "Rust Ptr: {}#{:p}, called from: {}",
                    core::stringify!($name),
                    x,
                    core::panic::Location::caller()
                ));
                x
            }
            #[cfg(not(feature = "debug-memory"))]
            pub fn c_ptr(self) -> *mut Self {
                alloc::boxed::Box::into_raw(alloc::boxed::Box::new(self))
            }
        }
    };
}

#[macro_export]
macro_rules! impl_simple_c_ptr {
    ($name:ident<$t: ident>) => {
        impl<$t> $name<$t> {
            #[cfg(feature = "debug-memory")]
            #[track_caller]
            pub fn simple_c_ptr(self) -> *mut Self {
                let x = alloc::boxed::Box::into_raw(alloc::boxed::Box::new(self));
                rust_tools::debug!(alloc::format!(
                    "Rust Ptr: {}#{:p}, called from: {}",
                    core::stringify!($name),
                    x,
                    core::panic::Location::caller()
                ));
                x
            }
            #[cfg(not(feature = "debug-memory"))]
            pub fn simple_c_ptr(self) -> *mut Self {
                alloc::boxed::Box::into_raw(alloc::boxed::Box::new(self))
            }
        }
    };
}

#[macro_export]
macro_rules! impl_new_error {
    ($name:ident) => {
        impl $name {
            pub fn error(error_code: ErrorCodes, error_message: String) -> Self {
                let result = Self::new();
                Self {
                    error_code: error_code as u32,
                    error_message: CString::new(error_message).unwrap().into_raw(),
                    ..result
                }
            }
        }
        impl From<URError> for $name {
            fn from(value: URError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }

        impl From<RustCError> for $name {
            fn from(value: RustCError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        impl From<KeystoreError> for $name {
            fn from(value: KeystoreError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        impl From<super::errors::KeystoneError> for $name {
            fn from(value: super::errors::KeystoneError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "bitcoin")]
        impl From<app_bitcoin::errors::BitcoinError> for $name {
            fn from(value: app_bitcoin::errors::BitcoinError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "ethereum")]
        impl From<app_ethereum::errors::EthereumError> for $name {
            fn from(value: app_ethereum::errors::EthereumError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "tron")]
        impl From<app_tron::errors::TronError> for $name {
            fn from(value: app_tron::errors::TronError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "avalanche")]
        impl From<app_avalanche::errors::AvaxError> for $name {
            fn from(value: app_avalanche::errors::AvaxError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "solana")]
        impl From<app_solana::errors::SolanaError> for $name {
            fn from(value: app_solana::errors::SolanaError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "near")]
        impl From<app_near::errors::NearError> for $name {
            fn from(value: app_near::errors::NearError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "aptos")]
        impl From<app_aptos::errors::AptosError> for $name {
            fn from(value: app_aptos::errors::AptosError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "cosmos")]
        impl From<app_cosmos::errors::CosmosError> for $name {
            fn from(value: app_cosmos::errors::CosmosError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "xrp")]
        impl From<app_xrp::errors::XRPError> for $name {
            fn from(value: app_xrp::errors::XRPError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "sui")]
        impl From<app_sui::errors::SuiError> for $name {
            fn from(value: app_sui::errors::SuiError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "cardano")]
        impl From<app_cardano::errors::CardanoError> for $name {
            fn from(value: app_cardano::errors::CardanoError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "arweave")]
        impl From<app_arweave::errors::ArweaveError> for $name {
            fn from(value: app_arweave::errors::ArweaveError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "stellar")]
        impl From<app_stellar::errors::StellarError> for $name {
            fn from(value: app_stellar::errors::StellarError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "ton")]
        impl From<app_ton::errors::TonError> for $name {
            fn from(value: app_ton::errors::TonError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "zcash")]
        impl From<app_zcash::errors::ZcashError> for $name {
            fn from(value: app_zcash::errors::ZcashError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }

        #[cfg(feature = "monero")]
        impl From<app_monero::errors::MoneroError> for $name {
            fn from(value: app_monero::errors::MoneroError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "ergo")]
        impl From<app_ergo::errors::ErgoError> for $name {
            fn from(value: app_ergo::errors::ErgoError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
    };

    ($name:ident<$t:ident>) => {
        impl<$t: Free> $name<$t> {
            pub fn error(error_code: ErrorCodes, error_message: String) -> Self {
                let result = Self::new();
                Self {
                    error_code: error_code as u32,
                    error_message: CString::new(error_message).unwrap().into_raw(),
                    ..result
                }
            }
        }

        impl<$t: Free> From<URError> for $name<$t> {
            fn from(value: URError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }

        impl<$t: Free> From<RustCError> for $name<$t> {
            fn from(value: RustCError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }

        impl<$t: Free> From<KeystoreError> for $name<$t> {
            fn from(value: KeystoreError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        impl<$t: Free> From<super::errors::KeystoneError> for $name<$t> {
            fn from(value: super::errors::KeystoneError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "bitcoin")]
        impl<$t: Free> From<app_bitcoin::errors::BitcoinError> for $name<$t> {
            fn from(value: app_bitcoin::errors::BitcoinError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "ethereum")]
        impl<$t: Free> From<app_ethereum::errors::EthereumError> for $name<$t> {
            fn from(value: app_ethereum::errors::EthereumError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }

        #[cfg(feature = "tron")]
        impl<$t: Free> From<app_tron::errors::TronError> for $name<$t> {
            fn from(value: app_tron::errors::TronError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "solana")]
        impl<$t: Free> From<app_solana::errors::SolanaError> for $name<$t> {
            fn from(value: app_solana::errors::SolanaError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "near")]
        impl<$t: Free> From<app_near::errors::NearError> for $name<$t> {
            fn from(value: app_near::errors::NearError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "cosmos")]
        impl<$t: Free> From<app_cosmos::errors::CosmosError> for $name<$t> {
            fn from(value: app_cosmos::errors::CosmosError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "xrp")]
        impl<$t: Free> From<app_xrp::errors::XRPError> for $name<$t> {
            fn from(value: app_xrp::errors::XRPError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "aptos")]
        impl<$t: Free> From<app_aptos::errors::AptosError> for $name<$t> {
            fn from(value: app_aptos::errors::AptosError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "sui")]
        impl<$t: Free> From<app_sui::errors::SuiError> for $name<$t> {
            fn from(value: app_sui::errors::SuiError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "cardano")]
        impl<$t: Free> From<app_cardano::errors::CardanoError> for $name<$t> {
            fn from(value: app_cardano::errors::CardanoError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "arweave")]
        impl<$t: Free> From<app_arweave::errors::ArweaveError> for $name<$t> {
            fn from(value: app_arweave::errors::ArweaveError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "stellar")]
        impl<$t: Free> From<app_stellar::errors::StellarError> for $name<$t> {
            fn from(value: app_stellar::errors::StellarError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "ton")]
        impl<$t: Free> From<app_ton::errors::TonError> for $name<$t> {
            fn from(value: app_ton::errors::TonError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "zcash")]
        impl<$t: Free> From<app_zcash::errors::ZcashError> for $name<$t> {
            fn from(value: app_zcash::errors::ZcashError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "monero")]
        impl<$t: Free> From<app_monero::errors::MoneroError> for $name<$t> {
            fn from(value: app_monero::errors::MoneroError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "ergo")]
        impl<$t: Free> From<app_ergo::errors::ErgoError> for $name<$t> {
            fn from(value: app_ergo::errors::ErgoError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
    };
}

#[macro_export]
macro_rules! impl_simple_new_error {
    ($name:ident<$t:ident>) => {
        impl<$t> $name<$t> {
            pub fn error(error_code: ErrorCodes, error_message: String) -> Self {
                let result = Self::new();
                Self {
                    error_code: error_code as u32,
                    error_message: CString::new(error_message).unwrap().into_raw(),
                    ..result
                }
            }
        }
        impl<$t> From<KeystoreError> for $name<$t> {
            fn from(value: KeystoreError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        impl<$t> From<RustCError> for $name<$t> {
            fn from(value: RustCError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "bitcoin")]
        impl<$t> From<app_bitcoin::errors::BitcoinError> for $name<$t> {
            fn from(value: app_bitcoin::errors::BitcoinError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "ethereum")]
        impl<$t> From<app_ethereum::errors::EthereumError> for $name<$t> {
            fn from(value: app_ethereum::errors::EthereumError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "tron")]
        impl<$t> From<app_tron::errors::TronError> for $name<$t> {
            fn from(value: app_tron::errors::TronError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "avalanche")]
        impl<$t> From<app_avalanche::errors::AvaxError> for $name<$t> {
            fn from(value: app_avalanche::errors::AvaxError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "solana")]
        impl<$t> From<app_solana::errors::SolanaError> for $name<$t> {
            fn from(value: app_solana::errors::SolanaError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "near")]
        impl<$t> From<app_near::errors::NearError> for $name<$t> {
            fn from(value: app_near::errors::NearError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "aptos")]
        impl<$t> From<app_aptos::errors::AptosError> for $name<$t> {
            fn from(value: app_aptos::errors::AptosError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "xrp")]
        impl<$t> From<app_xrp::errors::XRPError> for $name<$t> {
            fn from(value: app_xrp::errors::XRPError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "cosmos")]
        impl<$t> From<app_cosmos::errors::CosmosError> for $name<$t> {
            fn from(value: app_cosmos::errors::CosmosError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "sui")]
        impl<$t> From<app_sui::errors::SuiError> for $name<$t> {
            fn from(value: app_sui::errors::SuiError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "cardano")]
        impl<$t> From<app_cardano::errors::CardanoError> for $name<$t> {
            fn from(value: app_cardano::errors::CardanoError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "arweave")]
        impl<$t> From<app_arweave::errors::ArweaveError> for $name<$t> {
            fn from(value: app_arweave::errors::ArweaveError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "stellar")]
        impl<$t> From<app_stellar::errors::StellarError> for $name<$t> {
            fn from(value: app_stellar::errors::StellarError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "ton")]
        impl<$t> From<app_ton::errors::TonError> for $name<$t> {
            fn from(value: app_ton::errors::TonError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "zcash")]
        impl<$t> From<app_zcash::errors::ZcashError> for $name<$t> {
            fn from(value: app_zcash::errors::ZcashError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "monero")]
        impl<$t> From<app_monero::errors::MoneroError> for $name<$t> {
            fn from(value: app_monero::errors::MoneroError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
        #[cfg(feature = "ergo")]
        impl<$t> From<app_ergo::errors::ErgoError> for $name<$t> {
            fn from(value: app_ergo::errors::ErgoError) -> Self {
                Self::error(ErrorCodes::from(&value), value.to_string())
            }
        }
    };
}

#[macro_export]
macro_rules! extract_ptr_with_type {
    ($x: expr, $name: ident) => {{
        let ptr = $x as *mut $name;
        let result: &mut $name = unsafe { &mut *ptr };
        result
    }};
    ($x: expr, $name: ident<$t: ident>) => {{
        let ptr = $x as *mut $name<$t>;
        let result: &mut $name<$t> = unsafe { &mut *ptr };
        result
    }};
}

#[macro_export]
macro_rules! extract_array {
    ($x: expr, $name: ident, $length: expr) => {{
        let ptr = $x as *mut $name;
        let result: &[$name] = unsafe { core::slice::from_raw_parts(ptr, $length as usize) };
        result
    }};
}

#[macro_export]
macro_rules! impl_response {
    ($name:ident) => {
        impl_c_ptr!($name);
        impl_new_error!($name);
    };
    ($name:ident<$t:ident>) => {
        impl_c_ptr!($name<$t>);
        impl_new_error!($name<$t>);
    };
}

#[macro_export]
macro_rules! impl_c_ptrs {
    ($($name: ident), *) => {
        $(
            impl_c_ptr!($name);
        )*
    };
    ($($name:ident<$t: ident>), *) => {
        $(
            impl_c_ptr!($name<$t>);
        )*
    }
}

#[macro_export]
macro_rules! impl_simple_free {
    ($($name: ident), *) => {
        $(
            impl SimpleFree for $name {fn free(&self){}}
        )*
    };
}

#[macro_export]
macro_rules! make_free_method {
    ($t: ident) => {
        app_utils::paste::item! {
            #[no_mangle]
            pub extern "C" fn [<free_ $t>](ptr: PtrT<$t>) {
                $crate::check_and_free_ptr!(ptr)
            }
        }
    };
    ($t1:ident<$t2:ident>) => {
        app_utils::paste::item! {
            #[no_mangle]
            pub extern "C" fn [<free_ $t1 _ $t2>](ptr: PtrT<$t1<$t2>>) {
                $crate::check_and_free_ptr!(ptr)
            }
        }
    };
}
