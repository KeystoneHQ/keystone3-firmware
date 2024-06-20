#![no_std]
#![feature(error_in_core)]

use core::str::FromStr;

use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use errors::Result;

extern crate alloc;

use vendor::{address::TonAddress, wallet::TonWallet};

pub mod errors;
mod jettons;
mod messages;
pub mod mnemonic;
pub mod structs;
pub mod transaction;
mod utils;
mod vendor;

pub fn ton_public_key_to_address(pk: Vec<u8>) -> Result<String> {
    TonWallet::derive_default(vendor::wallet::WalletVersion::V4R2, pk)
        .map(|v| v.address.to_base64_url_flags(true, false))
        .map_err(|e| errors::TonError::AddressError(e.to_string()))
}

pub fn ton_compare_address_and_public_key(pk: Vec<u8>, address: String) -> bool {
    match TonWallet::derive_default(vendor::wallet::WalletVersion::V4R2, pk) {
        Ok(wallet) => match TonAddress::from_str(&address) {
            Ok(address) => return wallet.address.eq(&address),
            Err(e) => false,
        },
        Err(e) => false,
    }
}

#[cfg(test)]
mod tests {
    use third_party::hex;

    #[test]
    fn test_generate_address() {
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = super::ton_public_key_to_address(pk).unwrap();
        assert_eq!(address, "UQC4FC01K66rElokeYTPeEnWStITQDxGiK8RhkMXpT88FY5b")
    }
}
