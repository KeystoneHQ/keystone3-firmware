use crate::errors::Result;
use crate::get_address;
use alloc::string::String;

#[derive(Clone, Debug)]
pub struct SolanaMessage {
    pub raw_message: String,
    pub utf8_message: String,
    pub from: String,
}

impl SolanaMessage {
    pub fn from(raw_message: String, utf8_message: String, from: &String) -> Result<Self> {
        Ok(Self {
            raw_message,
            utf8_message,
            from: get_address(from)?,
        })
    }
}
