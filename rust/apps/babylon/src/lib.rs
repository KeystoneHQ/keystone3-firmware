//! Babylon `deriveContextHash` core logic.
//!
//! Implements the wallet-side derivation described in
//! <https://github.com/babylonlabs-io/babylon-toolkit/blob/main/docs/specs/derive-context-hash.md>
//!
//! ```text
//! ikm    = secp256k1 BIP-32 private key (32-byte scalar) at m/73681862'
//! salt   = "derive-context-hash"
//! info   = SHA-256(appName) || SHA-256(networkName) || connectedPubkey(33) || context
//! output = HKDF-SHA-256(ikm, salt, info, 32)
//! ```
//!
//! This crate is the pure cryptographic core (no FFI, no UR, no seed access). The
//! `ikm` and `connectedPubkey` are derived by the caller (the FFI layer) and passed
//! in as bytes. Input validation helpers live here so they can be unit-tested in
//! isolation; the appName allow-list is a device policy enforced by these helpers
//! and by the FFI request handler, NOT by [`derive_context_hash`] itself.

#![no_std]

#[macro_use]
extern crate alloc;

pub mod errors;

use alloc::string::ToString;
use alloc::vec::Vec;

use cryptoxide::digest::Digest;
use cryptoxide::hkdf::{hkdf_expand, hkdf_extract};
use cryptoxide::sha2::Sha256;
use zeroize::Zeroize;

use crate::errors::{BabylonError, Result};

/// Hardened BIP-32 purpose index used to derive the IKM.
///
/// Equals `trunc31_be(SHA-256("derive-context-hash"))` — see `test_purpose_index`.
pub const DERIVE_CONTEXT_HASH_PURPOSE: u32 = 73681862;

/// BIP-32 path (a single hardened child of the secp256k1 master) for the IKM.
pub const IKM_DERIVATION_PATH: &str = "m/73681862'";

/// HKDF salt providing domain separation (RFC 5869).
const SALT: &[u8] = b"derive-context-hash";

/// Output length in bytes.
pub const OUTPUT_LEN: usize = 32;

/// Maximum `appName` length in bytes.
pub const APP_NAME_MAX_LEN: usize = 64;

/// Maximum decoded `context` length in bytes.
pub const CONTEXT_MAX_LEN: usize = 1024;

/// Apps allowed to request a context hash on this device (device policy).
pub const ALLOWED_APP_NAMES: [&str; 2] = ["babylon-btc-vault", "ordinals-market"];

/// The wallet's current Bitcoin network (canonical name per the spec).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum BabylonNetwork {
    Mainnet,
    Testnet,
    Signet,
    Regtest,
}

impl BabylonNetwork {
    /// The canonical network name hashed into `info`.
    pub const fn canonical_name(&self) -> &'static str {
        match self {
            BabylonNetwork::Mainnet => "bitcoin-mainnet",
            BabylonNetwork::Testnet => "bitcoin-testnet",
            BabylonNetwork::Signet => "bitcoin-signet",
            BabylonNetwork::Regtest => "bitcoin-regtest",
        }
    }

    /// Parse a canonical network name; unknown names are rejected (no fallback).
    pub fn from_name(name: &str) -> Result<Self> {
        match name {
            "bitcoin-mainnet" => Ok(BabylonNetwork::Mainnet),
            "bitcoin-testnet" => Ok(BabylonNetwork::Testnet),
            "bitcoin-signet" => Ok(BabylonNetwork::Signet),
            "bitcoin-regtest" => Ok(BabylonNetwork::Regtest),
            other => Err(BabylonError::UnsupportedNetwork(other.to_string())),
        }
    }
}

/// Inputs to the pure derivation. All values are assumed already derived/decoded.
pub struct DeriveContextHashInput<'a> {
    /// Raw 32-byte private scalar at `m/73681862'`.
    pub ikm: [u8; 32],
    /// Human-readable app identifier (domain separation).
    pub app_name: &'a str,
    /// Wallet's current Bitcoin network.
    pub network: BabylonNetwork,
    /// 33-byte compressed SEC1 public key of the connected key.
    pub connected_pubkey: [u8; 33],
    /// Application-specific context (already hex-decoded).
    pub context: &'a [u8],
}

fn sha256(data: &[u8]) -> [u8; 32] {
    let mut hasher = Sha256::new();
    let mut out = [0u8; 32];
    hasher.input(data);
    hasher.result(&mut out);
    out
}

/// Compute the 32-byte context hash per the Babylon `deriveContextHash` spec.
///
/// This is the pure cryptographic core: it enforces structural `context` bounds but
/// does NOT enforce the appName allow-list (a device policy applied one layer up).
pub fn derive_context_hash(input: &DeriveContextHashInput) -> Result<[u8; OUTPUT_LEN]> {
    if input.context.is_empty() {
        return Err(BabylonError::InvalidContext(
            "context must not be empty".to_string(),
        ));
    }
    if input.context.len() > CONTEXT_MAX_LEN {
        return Err(BabylonError::InvalidContext(format!(
            "context exceeds {CONTEXT_MAX_LEN} bytes"
        )));
    }

    // info = SHA-256(appName) || SHA-256(networkName) || connectedPubkey || context
    let mut info = Vec::with_capacity(32 + 32 + 33 + input.context.len());
    info.extend_from_slice(&sha256(input.app_name.as_bytes()));
    info.extend_from_slice(&sha256(input.network.canonical_name().as_bytes()));
    info.extend_from_slice(&input.connected_pubkey);
    info.extend_from_slice(input.context);

    // HKDF-SHA-256 (RFC 5869): extract then expand.
    let mut prk = [0u8; 32];
    hkdf_extract(Sha256::new(), SALT, &input.ikm, &mut prk);

    let mut okm = [0u8; OUTPUT_LEN];
    hkdf_expand(Sha256::new(), &prk, &info, &mut okm);

    prk.zeroize();
    Ok(okm)
}

/// Validate `appName`: length `1..=64`, charset `[a-z0-9-]`, AND membership in the
/// device allow-list ([`ALLOWED_APP_NAMES`]).
pub fn validate_app_name(app_name: &str) -> Result<()> {
    let len = app_name.len();
    if len == 0 || len > APP_NAME_MAX_LEN {
        return Err(BabylonError::InvalidAppName(format!(
            "app name length must be 1..={APP_NAME_MAX_LEN}, got {len}"
        )));
    }
    if !app_name
        .bytes()
        .all(|b| matches!(b, b'a'..=b'z' | b'0'..=b'9' | b'-'))
    {
        return Err(BabylonError::InvalidAppName(
            "app name must match [a-z0-9-]".to_string(),
        ));
    }
    if !ALLOWED_APP_NAMES.contains(&app_name) {
        return Err(BabylonError::AppNotAllowed(app_name.to_string()));
    }
    Ok(())
}

/// Validate and decode the `context` hex string.
///
/// Rules: non-empty, even length, lowercase hex with no `0x` prefix, and a decoded
/// length of at most [`CONTEXT_MAX_LEN`] bytes.
pub fn validate_context_hex(context_hex: &str) -> Result<Vec<u8>> {
    if context_hex.is_empty() {
        return Err(BabylonError::InvalidContext(
            "context must not be empty".to_string(),
        ));
    }
    if context_hex.len() % 2 != 0 {
        return Err(BabylonError::InvalidContext(
            "context hex must be even length".to_string(),
        ));
    }
    if context_hex
        .bytes()
        .any(|b| !matches!(b, b'0'..=b'9' | b'a'..=b'f'))
    {
        return Err(BabylonError::InvalidContext(
            "context must be lowercase hex with no 0x prefix".to_string(),
        ));
    }
    let bytes = hex::decode(context_hex)
        .map_err(|e| BabylonError::InvalidContext(format!("invalid hex: {e}")))?;
    if bytes.len() > CONTEXT_MAX_LEN {
        return Err(BabylonError::InvalidContext(format!(
            "context exceeds {CONTEXT_MAX_LEN} bytes"
        )));
    }
    Ok(bytes)
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;
    use keystore::algorithms::secp256k1::{get_private_key_by_seed, get_public_key_by_seed};

    // BIP-39 seed for "abandon abandon abandon abandon abandon abandon abandon
    // abandon abandon abandon abandon about" with an empty passphrase.
    const TEST_SEED_HEX: &str = "5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4";

    #[test]
    fn test_official_vector() {
        let seed = hex::decode(TEST_SEED_HEX).unwrap();

        let ikm = get_private_key_by_seed(&seed, &IKM_DERIVATION_PATH.to_string())
            .unwrap()
            .secret_bytes();
        let connected_pubkey = get_public_key_by_seed(&seed, &"m/44'/0'/0'/0/0".to_string())
            .unwrap()
            .serialize();
        let context = hex::decode("deadbeef").unwrap();

        let input = DeriveContextHashInput {
            ikm,
            app_name: "test-app",
            network: BabylonNetwork::Mainnet,
            connected_pubkey,
            context: &context,
        };

        let out = derive_context_hash(&input).unwrap();
        assert_eq!(
            hex::encode(out),
            "f82ced3be0e29591a7863ece03d65f79fb494fe0de7203549855f462455df008"
        );
    }

    #[test]
    fn test_purpose_index() {
        // 73681862 == trunc31_be(SHA-256("derive-context-hash"))
        let h = sha256(b"derive-context-hash");
        let be = u32::from_be_bytes([h[0], h[1], h[2], h[3]]);
        let trunc31 = be & 0x7FFF_FFFF;
        assert_eq!(trunc31, DERIVE_CONTEXT_HASH_PURPOSE);
    }

    #[test]
    fn test_info_layout_and_determinism() {
        let input = DeriveContextHashInput {
            ikm: [7u8; 32],
            app_name: "babylon-btc-vault",
            network: BabylonNetwork::Signet,
            connected_pubkey: [0x02; 33],
            context: &[0xde, 0xad],
        };
        let a = derive_context_hash(&input).unwrap();
        let b = derive_context_hash(&input).unwrap();
        assert_eq!(a, b, "derivation must be deterministic");
        assert_eq!(a.len(), OUTPUT_LEN);
    }

    #[test]
    fn test_context_changes_output() {
        let base = DeriveContextHashInput {
            ikm: [7u8; 32],
            app_name: "babylon-btc-vault",
            network: BabylonNetwork::Mainnet,
            connected_pubkey: [0x02; 33],
            context: &[0x01],
        };
        let other = DeriveContextHashInput {
            context: &[0x02],
            ..DeriveContextHashInput {
                ikm: base.ikm,
                app_name: base.app_name,
                network: base.network,
                connected_pubkey: base.connected_pubkey,
                context: base.context,
            }
        };
        assert_ne!(
            derive_context_hash(&base).unwrap(),
            derive_context_hash(&other).unwrap()
        );
    }

    #[test]
    fn test_derive_rejects_empty_context() {
        let input = DeriveContextHashInput {
            ikm: [1u8; 32],
            app_name: "babylon-btc-vault",
            network: BabylonNetwork::Mainnet,
            connected_pubkey: [2u8; 33],
            context: &[],
        };
        assert!(matches!(
            derive_context_hash(&input),
            Err(BabylonError::InvalidContext(_))
        ));
    }

    #[test]
    fn test_derive_rejects_oversized_context() {
        let context = vec![0u8; CONTEXT_MAX_LEN + 1];
        let input = DeriveContextHashInput {
            ikm: [1u8; 32],
            app_name: "babylon-btc-vault",
            network: BabylonNetwork::Mainnet,
            connected_pubkey: [2u8; 33],
            context: &context,
        };
        assert!(matches!(
            derive_context_hash(&input),
            Err(BabylonError::InvalidContext(_))
        ));
    }

    #[test]
    fn test_validate_app_name() {
        assert!(validate_app_name("babylon-btc-vault").is_ok());
        assert!(validate_app_name("ordinals-market").is_ok());
        // Spec-valid charset but not on the allow-list (e.g. the official vector's app).
        assert!(matches!(
            validate_app_name("test-app"),
            Err(BabylonError::AppNotAllowed(_))
        ));
        // Uppercase -> charset failure.
        assert!(matches!(
            validate_app_name("Babylon"),
            Err(BabylonError::InvalidAppName(_))
        ));
        // Empty.
        assert!(matches!(
            validate_app_name(""),
            Err(BabylonError::InvalidAppName(_))
        ));
        // Too long.
        let long = "a".repeat(APP_NAME_MAX_LEN + 1);
        assert!(matches!(
            validate_app_name(&long),
            Err(BabylonError::InvalidAppName(_))
        ));
    }

    #[test]
    fn test_validate_context_hex() {
        assert_eq!(
            validate_context_hex("deadbeef").unwrap(),
            vec![0xde, 0xad, 0xbe, 0xef]
        );
        assert!(matches!(
            validate_context_hex(""),
            Err(BabylonError::InvalidContext(_))
        ));
        // Odd length.
        assert!(matches!(
            validate_context_hex("abc"),
            Err(BabylonError::InvalidContext(_))
        ));
        // Uppercase.
        assert!(matches!(
            validate_context_hex("DEADBEEF"),
            Err(BabylonError::InvalidContext(_))
        ));
        // 0x prefix (the 'x' is not lowercase hex).
        assert!(matches!(
            validate_context_hex("0xdeadbeef"),
            Err(BabylonError::InvalidContext(_))
        ));
        // Decodes to 1025 bytes (> 1024).
        let big = "ab".repeat(CONTEXT_MAX_LEN + 1);
        assert!(matches!(
            validate_context_hex(&big),
            Err(BabylonError::InvalidContext(_))
        ));
    }

    #[test]
    fn test_network_from_name() {
        assert_eq!(
            BabylonNetwork::from_name("bitcoin-mainnet").unwrap(),
            BabylonNetwork::Mainnet
        );
        assert_eq!(
            BabylonNetwork::from_name("bitcoin-testnet").unwrap(),
            BabylonNetwork::Testnet
        );
        assert_eq!(
            BabylonNetwork::from_name("bitcoin-signet").unwrap(),
            BabylonNetwork::Signet
        );
        assert_eq!(
            BabylonNetwork::from_name("bitcoin-regtest").unwrap(),
            BabylonNetwork::Regtest
        );
        assert!(matches!(
            BabylonNetwork::from_name("ethereum"),
            Err(BabylonError::UnsupportedNetwork(_))
        ));
        // Round-trip every variant.
        for n in [
            BabylonNetwork::Mainnet,
            BabylonNetwork::Testnet,
            BabylonNetwork::Signet,
            BabylonNetwork::Regtest,
        ] {
            assert_eq!(BabylonNetwork::from_name(n.canonical_name()).unwrap(), n);
        }
    }
}
