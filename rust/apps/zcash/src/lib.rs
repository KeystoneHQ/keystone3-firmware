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
    pczt::Pczt, zcash_keys::keys::UnifiedFullViewingKey, zcash_protocol::consensus::MainNetwork,
};

pub fn get_address(ufvk_text: &str) -> Result<String> {
    let ufvk = UnifiedFullViewingKey::decode(&MainNetwork, ufvk_text)
        .map_err(|e| ZcashError::GenerateAddressError(e.to_string()))?;
    let (address, _) = ufvk
        .default_address(None)
        .map_err(|e| ZcashError::GenerateAddressError(e.to_string()))?;
    Ok(address.encode(&MainNetwork))
}

pub fn check_pczt(pczt: &[u8], ufvk_text: &str, seed_fingerprint: &[u8; 32]) -> Result<()> {
    // let ufvk = UnifiedFullViewingKey::decode(&MainNetwork, ufvk_text)
    //     .map_err(|e| ZcashError::InvalidDataError(e.to_string()))?;
    // let pczt =
    //     Pczt::parse(pczt).map_err(|_e| ZcashError::InvalidPczt(format!("invalid pczt data")))?;
    // pczt::check::check_pczt(seed_fingerprint, &ufvk, &pczt)?;
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

#[cfg(test)]
mod tests {
    use super::*;
    extern crate std;
    use std::println;
    #[test]
    fn test_get_address() {
        let address = get_address("uview1s2e0495jzhdarezq4h4xsunfk4jrq7gzg22tjjmkzpd28wgse4ejm6k7yfg8weanaghmwsvc69clwxz9f9z2hwaz4gegmna0plqrf05zkeue0nevnxzm557rwdkjzl4pl4hp4q9ywyszyjca8jl54730aymaprt8t0kxj8ays4fs682kf7prj9p24dnlcgqtnd2vnskkm7u8cwz8n0ce7yrwx967cyp6dhkc2wqprt84q0jmwzwnufyxe3j0758a9zgk9ssrrnywzkwfhu6ap6cgx3jkxs3un53n75s3");
        println!("{:?}", address);
    }
}
