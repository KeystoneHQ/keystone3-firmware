use crate::solana_lib::solana_program::errors::ProgramError;
use thiserror::Error;

/// Errors that may be returned by the TokenSwap program.
#[derive(Clone, Debug, Eq, Error, PartialEq)]
pub enum SwapError {
    /// The provided curve parameters are invalid
    #[error("The provided curve parameters are invalid")]
    InvalidCurve,
    /// Invalid instruction number passed in.
    #[error("Invalid instruction")]
    InvalidInstruction,
    /// The input token account is empty.
    #[error("Input token account empty")]
    EmptySupply,
}

impl From<SwapError> for ProgramError {
    fn from(e: SwapError) -> Self {
        ProgramError::Custom(e as u32)
    }
}
