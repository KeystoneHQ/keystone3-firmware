use thiserror::Error;

use crate::solana_lib::solana_program::errors::ProgramError;

#[derive(Clone, Debug, Eq, Error, PartialEq)]
pub enum JupiterError {
    #[error("Unkown jupiter instruction data")]
    UnknownJupiterInstruction,
}

impl From<JupiterError> for ProgramError {
    fn from(e: JupiterError) -> Self {
        ProgramError::Custom(e as u32)
    }
}
