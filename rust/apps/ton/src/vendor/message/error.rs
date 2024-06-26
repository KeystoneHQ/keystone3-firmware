use alloc::string::String;
use third_party::thiserror::Error;
use third_party::thiserror;

use crate::vendor::cell::TonCellError;

#[derive(Error, Debug)]
pub enum TonMessageError {
    #[error("ForwardTonAmountIsNegative error: Forward_ton_amount must be positive when specifying forward_payload")]
    ForwardTonAmountIsNegative,

    #[error("NaCl cryptographic error ({0})")]
    NaclCryptographicError(String),

    #[error("TonCellError ({0})")]
    TonCellError(#[from] TonCellError),
}
