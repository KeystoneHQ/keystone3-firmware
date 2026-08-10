/// Deprecated raw-protobuf UTXO transaction implementation.
///
/// Retained only for BCH, DASH and LTC compatibility. Bitcoin transactions
/// must use PSBT; other legacy UTXO variants are rejected at the product entry.
#[deprecated(note = "raw-protobuf Bitcoin transactions are deprecated; use PSBT for Bitcoin")]
pub mod legacy;
pub mod parsed_tx;
pub mod psbt;
mod script_type;
pub mod tx_checker;
