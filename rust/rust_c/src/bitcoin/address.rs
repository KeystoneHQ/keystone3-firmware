use alloc::string::{String, ToString};

use crate::common::structs::SimpleResponse;
use crate::common::types::PtrString;
use crate::common::utils::{convert_c_char, recover_c_char};
use app_bitcoin;
use app_bitcoin::addresses::xyzpub;
use app_bitcoin::network::Network;
use bitcoin::secp256k1::ffi::types::c_char;
use core::str::FromStr;

#[no_mangle]
pub unsafe extern "C" fn utxo_get_address(
    hd_path: PtrString,
    x_pub: PtrString,
) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(x_pub);
    let hd_path = recover_c_char(hd_path);
    let address = app_bitcoin::get_address(hd_path, &x_pub);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn btcoin_get_address_with_network(
    hd_path: PtrString,
    x_pub: PtrString,
    network: PtrString,
) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(x_pub);
    let hd_path = recover_c_char(hd_path);
    let network: String = recover_c_char(network);
    let network = match bitcoin_address_network(network.as_str()) {
        Ok(v) => v,
        Err(e) => return SimpleResponse::from(e).simple_c_ptr(),
    };
    let address = app_bitcoin::get_address_with_network(hd_path, &x_pub, network);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn xpub_convert_version(
    x_pub: PtrString,
    target: PtrString,
) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(x_pub);
    let target = recover_c_char(target);
    let ver = xyzpub::Version::from_str(target.as_str());
    match ver {
        Ok(v) => match xyzpub::convert_version(x_pub, &v) {
            Ok(result) => {
                SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr()
            }
            Err(e) => SimpleResponse::from(e).simple_c_ptr(),
        },
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

fn bitcoin_address_network(network: &str) -> Result<Network, app_bitcoin::errors::BitcoinError> {
    match network {
        "bitcoin-mainnet" | "BTC" | "mainnet" => Ok(Network::Bitcoin),
        "bitcoin-testnet" | "bitcoin-signet" | "tBTC" | "testnet" | "signet" => {
            Ok(Network::BitcoinTestnet)
        }
        other => Err(app_bitcoin::errors::BitcoinError::UnsupportedNetwork(
            other.to_string(),
        )),
    }
}
