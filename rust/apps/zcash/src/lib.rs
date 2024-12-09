#![no_std]
#![feature(error_in_core)]
extern crate alloc;

pub mod errors;
pub mod pczt;

use errors::{Result, ZcashError};

use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use pczt::structs::ParsedPczt;
use zcash_vendor::{
    pczt::Pczt,
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
    let ufvk = UnifiedFullViewingKey::decode(&MainNetwork, ufvk_text)
        .map_err(|e| ZcashError::InvalidDataError(e.to_string()))?;
    let pczt =
        Pczt::parse(pczt).map_err(|_e| ZcashError::InvalidPczt(format!("invalid pczt data")))?;
    pczt::check::check_pczt(seed_fingerprint, &ufvk, &pczt)?;
    Ok(())
}

pub fn parse_pczt(pczt: &[u8], ufvk_text: &str, seed_fingerprint: &[u8; 32]) -> Result<ParsedPczt> {
    let ufvk = UnifiedFullViewingKey::decode(&MainNetwork, ufvk_text)
        .map_err(|e| ZcashError::InvalidDataError(e.to_string()))?;
    let pczt =
        Pczt::parse(pczt).map_err(|_e| ZcashError::InvalidPczt(format!("invalid pczt data")))?;
    pczt::parse::parse_pczt(seed_fingerprint, &ufvk, &pczt)
}

pub fn sign_pczt(pczt: &[u8], seed: &[u8]) -> Result<Vec<u8>> {
    let pczt =
        Pczt::parse(pczt).map_err(|_e| ZcashError::InvalidPczt(format!("invalid pczt data")))?;
    pczt::sign::sign_pczt(&pczt, seed)
}

// pub fn sign_transaction(tx: &[u8], seed: &[u8]) -> Result<Vec<u8>> {
//     let mut transaction = tx.clone();
//     let pczt = PartiallyCreatedTransaction::decode(transaction)?;

// }
