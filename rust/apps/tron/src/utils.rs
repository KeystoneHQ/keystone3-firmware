use crate::errors::Result;
use alloc::string::String;
use alloc::vec::Vec;
use bitcoin::base58;

pub fn base58check_to_u8_slice(input: String) -> Result<Vec<u8>> {
    let result = base58::decode_check(input.as_str())?;
    Ok(result)
}
