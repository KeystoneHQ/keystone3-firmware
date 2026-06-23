#![no_std]
extern crate alloc;

pub mod errors;
pub mod pczt;
pub mod version;

use errors::{Result, ZcashError};

use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use pczt::structs::ParsedPczt;
use zcash_vendor::{
    zcash_keys::keys::{UnifiedAddressRequest, UnifiedFullViewingKey},
    zcash_protocol::consensus::{self},
    zip32,
};

#[cfg(any(test, feature = "multi_coins"))]
use zcash_vendor::pczt::Pczt;

/// Generates a Zcash address from a Unified Full Viewing Key (UFVK).
///
/// # Parameters
/// * `params` - The consensus parameters for the Zcash network (mainnet or testnet)
/// * `ufvk_text` - The string representation of the Unified Full Viewing Key
///
/// # Returns
/// * `Result<String>` - The encoded Zcash address if successful, or an error if the UFVK is invalid
///                      or if there was an issue generating the address
///
/// # Errors
/// * `ZcashError::GenerateAddressError` - If the UFVK cannot be decoded or if the address cannot be generated
pub fn get_address<P: consensus::Parameters>(params: &P, ufvk_text: &str) -> Result<String> {
    let ufvk = UnifiedFullViewingKey::decode(params, ufvk_text)
        .map_err(|e| ZcashError::GenerateAddressError(e.to_string()))?;
    let (address, _) = ufvk
        .default_address(UnifiedAddressRequest::AllAvailableKeys)
        .map_err(|e| ZcashError::GenerateAddressError(e.to_string()))?;
    Ok(address.encode(params))
}

/// Validates a Partially Created Zcash Transaction (PCZT) against a Unified Full Viewing Key.
///
/// # Parameters
/// * `params` - The consensus parameters for the Zcash network (mainnet or testnet)
/// * `pczt` - The binary representation of the PCZT to validate
/// * `ufvk_text` - The string representation of the Unified Full Viewing Key
/// * `seed_fingerprint` - A 32-byte fingerprint of the seed used to derive keys
/// * `account_index` - The account index for the keys to check against
///
/// # Returns
/// * `Result<()>` - Ok if the PCZT is valid for the given UFVK, or an error otherwise
///
/// # Errors
/// * `ZcashError::InvalidDataError` - If the UFVK cannot be decoded or the account index is invalid
/// * `ZcashError::InvalidPczt` - If the PCZT data is malformed or cannot be parsed
/// * Other errors from the underlying validation process
#[cfg(feature = "cypherpunk")]
pub fn check_pczt_cypherpunk<P: consensus::Parameters>(
    params: &P,
    pczt: &[u8],
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<()> {
    let pczt = pczt::parse_pczt(pczt)?;
    let account_index = zip32::AccountId::try_from(account_index)
        .map_err(|_e| ZcashError::InvalidDataError("invalid account index".to_string()))?;
    let ufvk = UnifiedFullViewingKey::decode(params, ufvk_text)
        .map_err(|e| ZcashError::InvalidDataError(e.to_string()))?;
    let xpub = ufvk.transparent().ok_or(ZcashError::InvalidDataError(
        "transparent xpub is not present".to_string(),
    ))?;
    pczt::check::check_pczt_orchard(params, seed_fingerprint, account_index, &ufvk, &pczt)?;
    pczt::check::check_pczt_transparent(
        params,
        seed_fingerprint,
        account_index,
        xpub,
        &pczt,
        false,
    )?;
    Ok(())
}

#[cfg(feature = "multi_coins")]
pub fn check_pczt_multi_coins<P: consensus::Parameters>(
    params: &P,
    pczt: &[u8],
    xpub: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<()> {
    let pczt = pczt::parse_pczt(pczt)?;
    let account_pubkey = transparent_account_pubkey_from_xpub(xpub)?;
    let account_index = zip32::AccountId::try_from(account_index)
        .map_err(|_e| ZcashError::InvalidDataError("invalid account index".to_string()))?;

    pczt::check::check_pczt_transparent(
        params,
        seed_fingerprint,
        account_index,
        &account_pubkey,
        &pczt,
        true,
    )?;
    Ok(())
}

#[cfg(feature = "multi_coins")]
fn transparent_account_pubkey_from_xpub(
    xpub: &str,
) -> Result<zcash_vendor::transparent::keys::AccountPubKey> {
    use core::str::FromStr;
    use zcash_vendor::{bip32, transparent};

    let xpub: bip32::ExtendedPublicKey<bitcoin::secp256k1::PublicKey> =
        bip32::ExtendedPublicKey::from_str(xpub)
            .map_err(|e| ZcashError::InvalidDataError(e.to_string()))?;

    let key = {
        let chain_code = xpub.attrs().chain_code;
        let pubkey = xpub.public_key().serialize();
        let mut bytes = [0u8; 65];
        bytes[..32].copy_from_slice(&chain_code);
        bytes[32..].copy_from_slice(&pubkey);
        bytes
    };

    transparent::keys::AccountPubKey::deserialize(&key)
        .map_err(|e| ZcashError::InvalidDataError(e.to_string()))
}

/// Parses a Partially Created Zcash Transaction (PCZT) and extracts its details.
///
/// This function takes a binary PCZT and a Unified Full Viewing Key (UFVK), parses the transaction,
/// and returns a structured representation of the transaction's contents.
///
/// # Parameters
/// * `params` - The consensus parameters for the Zcash network (mainnet or testnet)
/// * `pczt` - The binary representation of the PCZT to parse
/// * `ufvk_text` - The string representation of the Unified Full Viewing Key
/// * `seed_fingerprint` - A 32-byte fingerprint of the seed used to derive keys
/// # Returns
/// * `Result<ParsedPczt>` - A structured representation of the PCZT if successful
///
/// # Errors
/// * `ZcashError::InvalidDataError` - If the UFVK cannot be decoded
/// * `ZcashError::InvalidPczt` - If the PCZT data is malformed or cannot be parsed
/// * Other errors from the underlying parsing process
#[cfg(feature = "cypherpunk")]
pub fn parse_pczt_cypherpunk<P: consensus::Parameters>(
    params: &P,
    pczt: &[u8],
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
) -> Result<ParsedPczt> {
    let ufvk = UnifiedFullViewingKey::decode(params, ufvk_text)
        .map_err(|e| ZcashError::InvalidDataError(e.to_string()))?;
    let pczt = pczt::parse_pczt(pczt)?;
    pczt::parse::parse_pczt_cypherpunk(params, seed_fingerprint, &ufvk, &pczt)
}

#[cfg(test)]
mod additional_tests {
    use super::*;
    use zcash_vendor::zcash_protocol::consensus::MAIN_NETWORK;

    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_get_address() {
        let ufvk_text = "uview10zf3gnxd08cne6g7ryh6lln79duzsayg0qxktvyc3l6uutfk0agmyclm5g82h5z0lqv4c2gzp0eu0qc0nxzurxhj4ympwn3gj5c3dc9g7ca4eh3q09fw9kka7qplzq0wnauekf45w9vs4g22khtq57sc8k6j6s70kz0rtqlyat6zsjkcqfrlm9quje8vzszs8y9mjvduf7j2vx329hk2v956g6svnhqswxfp3n760mw233w7ffgsja2szdhy5954hsfldalf28wvav0tctxwkmkgrk43tq2p7sqchzc6";
        let addr = get_address(&MAIN_NETWORK, ufvk_text).expect("should generate address");
        // We can print this address to see what it is, and then pin it in the test.
        // For now, let's just assert it is valid and not empty.
        assert!(!addr.is_empty());
        assert!(addr.starts_with("u1")); // Mainnet unified address starts with u1
    }

    #[test]
    fn test_get_address_invalid_ufvk() {
        let ufvk_text = "invalid_ufvk";
        let result = get_address(&MAIN_NETWORK, ufvk_text);
        assert!(result.is_err());
    }
}

#[cfg(feature = "multi_coins")]
pub fn parse_pczt_multi_coins<P: consensus::Parameters>(
    params: &P,
    pczt: &[u8],
    seed_fingerprint: &[u8; 32],
) -> Result<ParsedPczt> {
    let pczt = pczt::parse_pczt(pczt)?;

    pczt::parse::parse_pczt_multi_coins(params, seed_fingerprint, &pczt)
}

/// Signs a Partially Created Zcash Transaction (PCZT) using a seed.
///
/// This function takes a binary PCZT and a seed, parses the transaction,
/// and returns a signed PCZT.
///
/// # Parameters
/// * `pczt` - The binary representation of the PCZT to sign
/// * `seed` - The seed to sign the PCZT with
///
/// # Returns
/// * `Result<Vec<u8>>` - The signed PCZT if successful, or an error otherwise
///
/// # Errors
/// * `ZcashError::InvalidPczt` - If the PCZT data is malformed or cannot be parsed
/// * Other errors from the underlying signing process
pub fn sign_pczt(pczt: &[u8], seed: &[u8]) -> Result<Vec<u8>> {
    let pczt = pczt::parse_pczt(pczt)?;
    pczt::sign::sign_pczt(pczt, seed)
}
