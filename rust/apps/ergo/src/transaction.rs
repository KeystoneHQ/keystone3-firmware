use crate::errors::{ErgoError, Result};
use crate::structs::{ParseContext, ParsedErgoTx};
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::str::FromStr;
use core2::io::Read;
use ergo_lib::chain::transaction::reduced::ReducedTransaction;
use ergo_lib::chain::transaction::Transaction;
use ergo_lib::ergotree_ir::serialization::SigmaSerializable;
use ergo_lib::wallet::derivation_path::DerivationPath;
use ergo_lib::wallet::ext_secret_key::ExtSecretKey;
use ergo_lib::wallet::mnemonic::MnemonicSeed;
use ergo_lib::wallet::Wallet;

pub fn parse_tx(tx: Vec<u8>, context: ParseContext) -> Result<ParsedErgoTx> {
    let reduced_ergo_tx = ReducedTransaction::sigma_parse_bytes(tx.clone().as_mut_slice())
        .map_err(|e| ErgoError::TransactionParseError(e.to_string()))?;
    ParsedErgoTx::from_ergo_tx(reduced_ergo_tx.unsigned_tx, context)
}

pub fn sign_tx(
    tx: Vec<u8>,
    seed: &[u8],
    entropy: &[u8],
    derivation_paths: &Vec<String>,
) -> Result<Vec<u8>> {
    let reduced_ergo_tx = ReducedTransaction::sigma_parse_bytes(tx.clone().as_mut_slice())
        .map_err(|e| ErgoError::TransactionParseError(e.to_string()))?;

    let _seed: [u8; 64] = seed
        .try_into()
        .map_err(|_| ErgoError::SigningFailed("invalid seed".to_string()))?;
    let master_key = ExtSecretKey::derive_master(MnemonicSeed::from(_seed))
        .map_err(|e| ErgoError::SigningFailed(e.to_string()))?;

    let secrets = derivation_paths
        .iter()
        .map(|path| DerivationPath::from_str(path.as_str()))
        .flatten()
        .map(|derivation_path| master_key.derive(derivation_path))
        .flatten()
        .map(|ext_secret_key| ext_secret_key.secret_key())
        .collect();

    let wallet = Wallet::from_secrets(secrets);

    let signed_tx: Transaction = wallet
        .sign_reduced_transaction_deterministic(reduced_ergo_tx, entropy)
        .map_err(|e| ErgoError::SigningFailed(e.to_string()))?;

    signed_tx
        .sigma_serialize_bytes()
        .map_err(|e| ErgoError::SigningFailed(e.to_string()))
}

pub fn check_tx(tx: Vec<u8>, context: ParseContext) -> Result<()> {
    let reduced_ergo_tx = ReducedTransaction::sigma_parse_bytes(tx.clone().as_mut_slice())
        .map_err(|e| ErgoError::TransactionParseError(e.to_string()))?;

    ParsedErgoTx::verify(reduced_ergo_tx.unsigned_tx, context)
}
