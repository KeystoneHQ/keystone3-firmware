#![no_std]
#![feature(error_in_core)]
extern crate alloc;

pub mod errors;
pub mod pczt;

use errors::{Result, ZcashError};

use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use pczt::structs::ParsedPczt;
use zcash_vendor::{
    zcash_keys::keys::{UnifiedAddressRequest, UnifiedFullViewingKey},
    zcash_protocol::consensus::MainNetwork,
};

pub fn get_address(ufvk_text: &str) -> Result<String> {
    let ufvk = UnifiedFullViewingKey::decode(&MainNetwork, ufvk_text)
        .map_err(|e| ZcashError::GenerateAddressError(e.to_string()))?;
    let (address, _) = ufvk
        .default_address(UnifiedAddressRequest::all().unwrap())
        .map_err(|e| ZcashError::GenerateAddressError(e.to_string()))?;
    Ok(address.encode(&MainNetwork))
}

pub fn check_pczt(pczt: &[u8], ufvk_text: &str, seed_fingerprint: &[u8; 32]) -> Result<()> {
    unimplemented!()
}

pub fn parse_pczt(pczt: &[u8], ufvk_text: &str, seed_fingerprint: &[u8; 32]) -> Result<ParsedPczt> {
    unimplemented!()
}

pub fn sign_pczt(pczt: &[u8], seed: &[u8]) -> Result<Vec<u8>> {
    unimplemented!()
}

// pub fn sign_transaction(tx: &[u8], seed: &[u8]) -> Result<Vec<u8>> {
//     let mut transaction = tx.clone();
//     let pczt = PartiallyCreatedTransaction::decode(transaction)?;

// }
