use alloc::format;
use alloc::string::String;
use core::ops::Div;

use crate::errors::{Result, SolanaError};

pub mod squads_v4;
pub mod stake;
pub mod system;
pub mod token;
pub mod token_lending;
pub mod token_swap_v3;
pub mod vote;

pub mod jupiter_v6;

pub const DIVIDER: f64 = 1_000_000_000_f64;

pub fn format_amount(value: String) -> Result<String> {
    if let Ok(value) = value.as_str().parse::<f64>() {
        return Ok(format!("{} {}", value.div(DIVIDER), "SOL"));
    }
    Err(SolanaError::ParseTxError(format!(
        "invalid value {:?}",
        value
    )))
}
