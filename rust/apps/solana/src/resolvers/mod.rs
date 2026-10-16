use crate::errors::Result;
use alloc::format;
use alloc::string::String;

pub mod squads_v4;
pub mod stake;
pub mod system;
pub mod token;
pub mod token_lending;
pub mod token_swap_v3;
pub mod vote;

pub mod jupiter_v6;

pub fn format_amount(value: String) -> Result<String> {
    Ok(format!(
        "{} SOL",
        crate::utils::format_token_amount(&value, 9)?
    ))
}
