#![no_std]

extern crate alloc;
use alloc::string::ToString;
use app_monero::address::Address;
use app_monero::key::PublicKey;
use app_monero::structs::{AddressType, Network};
use common_rust_c::structs::SimpleResponse;
use common_rust_c::types::PtrString;
use common_rust_c::utils::{convert_c_char, recover_c_char};
use cty::c_char;

#[no_mangle]
pub extern "C" fn monero_get_address(
    pub_spend_key: PtrString,
    pub_view_key: PtrString,
) -> *mut SimpleResponse<c_char> {
    let spend_key = recover_c_char(pub_spend_key);
    let view_key = recover_c_char(pub_view_key);

    let address = Address::new(
        Network::Mainnet,
        AddressType::Standard,
        PublicKey::from_str(spend_key.as_str()).unwrap(),
        PublicKey::from_str(view_key.as_str()).unwrap(),
    );

    SimpleResponse::success(convert_c_char(address.to_string()) as *mut c_char).simple_c_ptr()
}

pub extern "C" fn monero_test() -> *mut SimpleResponse<c_char> {
    app_monero::transfer::test_sign_performance();

    SimpleResponse::success(convert_c_char("Hello from Monero".to_string()) as *mut c_char).simple_c_ptr()
}
