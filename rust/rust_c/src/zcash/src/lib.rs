#![no_std]
extern crate alloc;

pub mod structs;

use core::{ptr::null_mut, slice};

use alloc::{boxed::Box, string::ToString, vec};
use app_zcash::{
    get_address,
    pczt::structs::{ParsedFrom, ParsedOrchard, ParsedPczt, ParsedTo, ParsedTransparent},
};
use common_rust_c::{
    structs::{Response, SimpleResponse, TransactionParseResult},
    types::{Ptr, PtrBytes, PtrString, PtrUR},
    utils::{convert_c_char, recover_c_char},
};
use cty::c_char;
use keystore::algorithms::zcash::{self, calculate_seed_fingerprint, derive_ufvk};
use structs::DisplayPczt;

#[no_mangle]
pub extern "C" fn derive_zcash_ufvk(seed: PtrBytes, seed_len: u32) -> *mut SimpleResponse<c_char> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let ufvk_text = derive_ufvk(seed);
    match ufvk_text {
        Ok(text) => SimpleResponse::success(convert_c_char(text)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn calculate_zcash_seed_fingerprint(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<u8> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let sfp = calculate_seed_fingerprint(seed);
    match sfp {
        Ok(bytes) => {
            SimpleResponse::success(Box::into_raw(Box::new(bytes)) as *mut u8).simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn generate_zcash_default_address(
    ufvk_text: PtrString,
) -> *mut SimpleResponse<c_char> {
    let ufvk_text = recover_c_char(ufvk_text);
    let address = get_address(&ufvk_text);
    match address {
        Ok(text) => SimpleResponse::success(convert_c_char(text)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn parse_zcash_tx(
    tx: PtrUR,
    ufvk: PtrString,
) -> Ptr<TransactionParseResult<DisplayPczt>> {
    TransactionParseResult::success(mock_parsed_pczt().c_ptr()).c_ptr()
}

fn mock_parsed_pczt() -> DisplayPczt {
    let parsed = ParsedPczt::new(
        Some(ParsedTransparent::new(
            vec![ParsedFrom::new(
                "t1QN3Kxh5wPFDi9ZPrTCgMVY5X4Rz96LkVC".to_string(),
                "1 ZEC".to_string(),
                1,
                true,
            )],
            vec![ParsedTo::new(
                "t1QN3Kxh5wPFDi9ZPrTCgMVY5X4Rz96LkVC".to_string(),
                "1 ZEC".to_string(),
                1,
                false,
                true,
                None,
            )],
        )),
        Some(ParsedOrchard::new(
            vec![ParsedFrom::new(
                "u13axqdhqadqf3aua82uvnp8wle5vf8fgvnxhzr8nd2kpc23d3d06r25cgzsx4gz8gastt8lcqz4v2kyfdj0zvlkhv4vjudlxsrvprx48y".to_string(),
                "1 ZEC".to_string(),
                1,
                true,
            ),ParsedFrom::new(
                "u13axqdhqadqf3aua82uvnp8wle5vf8fgvnxhzr8nd2kpc23d3d06r25cgzsx4gz8gastt8lcqz4v2kyfdj0zvlkhv4vjudlxsrvprx48y".to_string(),
                "1 ZEC".to_string(),
                1,
                true,
            ),ParsedFrom::new(
                "u13axqdhqadqf3aua82uvnp8wle5vf8fgvnxhzr8nd2kpc23d3d06r25cgzsx4gz8gastt8lcqz4v2kyfdj0zvlkhv4vjudlxsrvprx48y".to_string(),
                "1 ZEC".to_string(),
                1,
                true,
            )],
            vec![
                ParsedTo::new(
                    "u13axqdhqadqf3aua82uvnp8wle5vf8fgvnxhzr8nd2kpc23d3d06r25cgzsx4gz8gastt8lcqz4v2kyfdj0zvlkhv4vjudlxsrvprx48y".to_string(),
                    "1 ZEC".to_string(),
                    1,
                    false,
                    true,
                    Some("this is a memo".to_string()),
                ),
                ParsedTo::new("u13axqdhqadqf3aua82uvnp8wle5vf8fgvnxhzr8nd2kpc23d3d06r25cgzsx4gz8gastt8lcqz4v2kyfdj0zvlkhv4vjudlxsrvprx48y".to_string(), "1 ZEC".to_string(), 1, true, true, None),
                ParsedTo::new("u13axqdhqadqf3aua82uvnp8wle5vf8fgvnxhzr8nd2kpc23d3d06r25cgzsx4gz8gastt8lcqz4v2kyfdj0zvlkhv4vjudlxsrvprx48y".to_string(), "1 ZEC".to_string(), 1, false, false, None),
            ],
        )),
        "2 ZEC".to_string(),
    );
    DisplayPczt::from(&parsed)
}
