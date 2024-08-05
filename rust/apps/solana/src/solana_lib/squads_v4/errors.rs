use thiserror::Error;

use crate::solana_lib::solana_program::errors::ProgramError;

// Errors that may be returned by the Token program.
#[derive(Clone, Debug, Eq, Error, PartialEq)]
pub enum SquadsV4Error {
    #[error("Invalid instruction")]
    InvalidInstruction,
    #[error("Unkown squads instruction data")]
    UnknownSquadV4Instruction,
}

impl From<SquadsV4Error> for ProgramError {
    fn from(e: SquadsV4Error) -> Self {
        ProgramError::Custom(e as u32)
    }
}
