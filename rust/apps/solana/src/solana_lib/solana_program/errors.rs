use serde_derive::{Deserialize, Serialize};
use thiserror::Error;

/// Reasons the program may fail
#[derive(Error, Clone, Debug, Deserialize, Eq, PartialEq, Serialize)]
pub enum ProgramError {
    #[error("An attempt to operate on an account that hasn't been initialized")]
    UninitializedAccount,
    #[error("An account's data contents was invalid")]
    InvalidAccountData,
    /// Allows on-chain programs to implement program-specific error types and see them returned
    /// by the Solana runtime. A program-specific error may be any type that is represented as
    /// or serialized to a u32 integer.
    #[error("Custom program error: {0:#x}")]
    Custom(u32),
}

/// Reasons the runtime might have rejected an instruction.
///
/// Instructions errors are included in the bank hashes and therefore are
/// included as part of the transaction results when determining consensus.
/// Because of this, members of this enum must not be removed, but new ones can
/// be added.  Also, it is crucial that meta-information if any that comes along
/// with an error be consistent across software versions.  For example, it is
/// dangerous to include error strings from 3rd party crates because they could
/// change at any time and changes to them are difficult to detect.
#[derive(Error, Debug)]
pub enum InstructionError {
    /// An instruction's data contents were invalid
    #[error("invalid instruction data")]
    InvalidInstructionData,
}
