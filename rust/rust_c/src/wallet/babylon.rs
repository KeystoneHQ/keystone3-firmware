//! FFI layer for Babylon `deriveContextHash` (Phases 2 + 4).
//!
//! Crypto core lives in `app_babylon`. Transport is the `qr-hardware-call` UR with
//! `CallParams::DeriveContextHash`. This module decodes the request, derives the
//! connected address for display, and produces the 32-byte result as a `Bytes` UR.

use alloc::string::{String, ToString};

use app_babylon::errors::BabylonError;
use app_babylon::{
    derive_context_hash, validate_app_name, validate_context_hex, BabylonNetwork,
    DeriveContextHashInput, IKM_DERIVATION_PATH,
};
use bitcoin::key::UntweakedPublicKey;
use bitcoin::secp256k1::Secp256k1;
use bitcoin::{Address, CompressedPublicKey, Network as BitcoinNetwork, PublicKey};
use keystore::algorithms::secp256k1::{get_private_key_by_seed, get_public_key_by_seed};
use ur_registry::bytes::Bytes;
use ur_registry::extend::qr_hardware_call::{CallParams, QRHardwareCall};
use ur_registry::traits::RegistryItem;
use zeroize::Zeroize;

use crate::common::errors::RustCError;
use crate::common::free::Free;
use crate::common::structs::Response;
use crate::common::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::{extract_ptr_with_type, free_str_ptr, impl_c_ptr, make_free_method};

impl From<BabylonError> for RustCError {
    fn from(value: BabylonError) -> Self {
        RustCError::InvalidData(value.to_string())
    }
}

/// Fields decoded from a `derive-context-hash` qr-hardware-call.
struct DecodedRequest {
    app_name: String,
    network: String,
    key_path: String,
    context: String,
    origin: Option<String>,
}

/// # Safety
/// `ur` must point to a valid `QRHardwareCall`.
unsafe fn decode_request(ur: PtrUR) -> Result<DecodedRequest, RustCError> {
    let call = extract_ptr_with_type!(ur, QRHardwareCall);
    match call.get_params() {
        CallParams::DeriveContextHash(d) => {
            let key_path = d
                .get_key_path()
                .get_path()
                .ok_or(RustCError::InvalidData("missing key path".to_string()))?;
            Ok(DecodedRequest {
                app_name: d.get_app_name(),
                network: d.get_network(),
                key_path,
                context: d.get_context(),
                origin: call.get_origin(),
            })
        }
        _ => Err(RustCError::InvalidData(
            "not a derive-context-hash call".to_string(),
        )),
    }
}

/// Display data shown on the approval screen (no secret material).
#[repr(C)]
pub struct DeriveContextHashCallData {
    pub app_name: PtrString,
    pub network: PtrString,
    pub key_path: PtrString,
    pub context: PtrString,
    pub origin: PtrString,
}

impl_c_ptr!(DeriveContextHashCallData);

impl Free for DeriveContextHashCallData {
    unsafe fn free(&self) {
        free_str_ptr!(self.app_name);
        free_str_ptr!(self.network);
        free_str_ptr!(self.key_path);
        free_str_ptr!(self.context);
        free_str_ptr!(self.origin);
    }
}

make_free_method!(Response<DeriveContextHashCallData>);

/// Decode + validate the request and produce display data for the approval screen.
///
/// Rejects disallowed appNames and unsupported networks *before* the screen is
/// shown. The UI derives the display address from the cached account xpub after
/// this validation, using the same validated network and key path.
///
/// # Safety
/// `ur` must point to a valid `QRHardwareCall`.
#[no_mangle]
pub unsafe extern "C" fn parse_derive_context_hash(
    ur: PtrUR,
) -> PtrT<Response<DeriveContextHashCallData>> {
    let req = match decode_request(ur) {
        Ok(v) => v,
        Err(e) => return Response::from(e).c_ptr(),
    };

    if let Err(e) = validate_app_name(&req.app_name) {
        return Response::from(RustCError::from(e)).c_ptr();
    }
    if let Err(e) = BabylonNetwork::from_name(&req.network) {
        return Response::from(RustCError::from(e)).c_ptr();
    }
    if let Err(e) = validate_context_hex(&req.context) {
        return Response::from(RustCError::from(e)).c_ptr();
    }
    if let Err(e) = validate_cached_xpub_path(&req.network, &req.key_path) {
        return Response::from(e).c_ptr();
    }

    let data = DeriveContextHashCallData {
        app_name: convert_c_char(req.app_name),
        network: convert_c_char(req.network),
        key_path: convert_c_char(req.key_path),
        context: convert_c_char(req.context),
        origin: match req.origin {
            Some(o) => convert_c_char(o),
            None => core::ptr::null_mut(),
        },
    };
    Response::success(data).c_ptr()
}

/// Run the full derivation and return the 32-byte result as a hex `Bytes` UR.
///
/// # Safety
/// `ur` must point to a valid `QRHardwareCall`; `seed` must be valid for `seed_len` bytes;
/// `displayed_address` must point to the address shown to the user on the approval screen.
#[no_mangle]
pub unsafe extern "C" fn generate_derive_context_hash_ur(
    ur: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    displayed_address: PtrString,
) -> PtrT<UREncodeResult> {
    let req = match decode_request(ur) {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    if let Err(e) = validate_cached_xpub_path(&req.network, &req.key_path) {
        return UREncodeResult::from(e).c_ptr();
    }
    let mut seed = core::slice::from_raw_parts(seed, seed_len as usize).to_vec();
    if displayed_address.is_null() {
        seed.zeroize();
        return UREncodeResult::from(RustCError::InvalidData(
            "missing displayed address".to_string(),
        ))
        .c_ptr();
    }
    let displayed_address = recover_c_char(displayed_address);

    let result =
        match verify_displayed_address(&seed, &req.network, &req.key_path, &displayed_address) {
            Ok(()) => compute_context_hash_hex(
                &seed,
                &req.app_name,
                &req.network,
                &req.key_path,
                &req.context,
            )
            .map_err(RustCError::from),
            Err(e) => Err(e),
        };
    seed.zeroize();

    let hash_hex = match result {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };

    let bytes = Bytes::new(hash_hex.into_bytes());
    let cbor: alloc::vec::Vec<u8> = match bytes.try_into() {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    UREncodeResult::encode(
        cbor,
        Bytes::get_registry_type().get_type(),
        FRAGMENT_MAX_LENGTH_DEFAULT,
    )
    .c_ptr()
}

/// Core (testable) derivation: seed + request fields -> lowercase hex hash string.
fn compute_context_hash_hex(
    seed: &[u8],
    app_name: &str,
    network: &str,
    key_path: &str,
    context_hex: &str,
) -> Result<String, BabylonError> {
    validate_app_name(app_name)?;
    let network = BabylonNetwork::from_name(network)?;
    let context = validate_context_hex(context_hex)?;

    let ikm = get_private_key_by_seed(seed, &IKM_DERIVATION_PATH.to_string())
        .map_err(|e| BabylonError::DeriveError(e.to_string()))?
        .secret_bytes();
    let connected_pubkey = get_public_key_by_seed(seed, &normalized_derivation_key_path(key_path))
        .map_err(|e| BabylonError::DeriveError(e.to_string()))?
        .serialize();

    let out = derive_context_hash(&DeriveContextHashInput {
        ikm,
        app_name,
        network,
        connected_pubkey,
        context: &context,
    })?;
    Ok(hex::encode(out))
}

fn verify_displayed_address(
    seed: &[u8],
    network: &str,
    key_path: &str,
    displayed_address: &str,
) -> Result<(), RustCError> {
    let expected = derive_connected_address_from_seed(seed, network, key_path)?;
    if expected == displayed_address {
        return Ok(());
    }
    Err(RustCError::InvalidData(
        "displayed address does not match seed-derived key path".to_string(),
    ))
}

fn derive_connected_address_from_seed(
    seed: &[u8],
    network: &str,
    key_path: &str,
) -> Result<String, RustCError> {
    let network = BabylonNetwork::from_name(network).map_err(RustCError::from)?;
    let pubkey = get_public_key_by_seed(seed, &normalized_derivation_key_path(key_path))
        .map_err(|e| RustCError::InvalidData(e.to_string()))?;

    let btc_network = match network {
        BabylonNetwork::Mainnet => BitcoinNetwork::Bitcoin,
        BabylonNetwork::Testnet | BabylonNetwork::Signet | BabylonNetwork::Regtest => {
            BitcoinNetwork::Testnet
        }
    };

    let address = match path_purpose(key_path)? {
        44 => Address::p2pkh(PublicKey::new(pubkey).pubkey_hash(), btc_network),
        49 => Address::p2shwpkh(&CompressedPublicKey(pubkey), btc_network),
        84 => Address::p2wpkh(&CompressedPublicKey(pubkey), btc_network),
        86 => {
            let secp = Secp256k1::verification_only();
            Address::p2tr(&secp, UntweakedPublicKey::from(pubkey), None, btc_network)
        }
        _ => return Err(RustCError::InvalidHDPath),
    };

    Ok(address.to_string())
}

/// Extract the BIP-44 purpose (first path component) from a derivation path.
fn path_purpose(key_path: &str) -> Result<u32, RustCError> {
    let trimmed = key_path
        .trim_start_matches("m/")
        .trim_start_matches("M/")
        .trim_start_matches('m')
        .trim_start_matches('M')
        .trim_start_matches('/');
    let first = trimmed.split('/').next().unwrap_or("");
    let digits = first.trim_end_matches(|c| c == '\'' || c == 'h' || c == 'H');
    digits.parse::<u32>().map_err(|_| RustCError::InvalidHDPath)
}

fn normalized_derivation_key_path(key_path: &str) -> String {
    if key_path.starts_with("M/") {
        let mut path = key_path.to_string();
        path.replace_range(0..1, "m");
        path
    } else {
        key_path.to_string()
    }
}

fn normalized_key_path(key_path: &str) -> String {
    if key_path.starts_with("m/") || key_path.starts_with("M/") {
        let mut path = key_path.to_string();
        path.replace_range(0..1, "M");
        path
    } else {
        alloc::format!("M/{key_path}")
    }
}

fn validate_cached_xpub_path(network: &str, key_path: &str) -> Result<(), RustCError> {
    // The current multi-coins UI displays the connected address from cached BTC
    // account xpubs. Only accept paths that are under those cached account xpubs.
    const MAX_DISPLAY_HD_PATH_LEN: usize = 64;
    let network = BabylonNetwork::from_name(network).map_err(RustCError::from)?;
    if network == BabylonNetwork::Regtest {
        return Err(RustCError::InvalidData(
            "regtest address display is not supported from cached xpub".to_string(),
        ));
    }

    const CACHED_XPUB_ACCOUNT_PATHS: [&str; 4] =
        ["M/44'/0'/0'", "M/49'/0'/0'", "M/84'/0'/0'", "M/86'/0'/0'"];
    let path = normalized_key_path(key_path);
    if path.len() >= MAX_DISPLAY_HD_PATH_LEN {
        return Err(RustCError::InvalidData(
            "key path is too long for address display".to_string(),
        ));
    }
    if CACHED_XPUB_ACCOUNT_PATHS.iter().any(|account_path| {
        path.strip_prefix(account_path)
            .map(is_valid_cached_xpub_address_suffix)
            .unwrap_or(false)
    }) {
        return Ok(());
    }

    Err(RustCError::InvalidData(
        "key path is not available from cached xpub".to_string(),
    ))
}

fn is_valid_cached_xpub_address_suffix(rest: &str) -> bool {
    let Some(suffix) = rest.strip_prefix('/') else {
        return false;
    };
    let mut parts = suffix.split('/');
    let Some(change) = parts.next() else {
        return false;
    };
    let Some(index) = parts.next() else {
        return false;
    };
    parts.next().is_none()
        && is_non_hardened_child_index(change)
        && is_non_hardened_child_index(index)
}

fn is_non_hardened_child_index(component: &str) -> bool {
    if component.is_empty() || !component.bytes().all(|b| b.is_ascii_digit()) {
        return false;
    }
    component
        .parse::<u32>()
        .map(|index| index < 0x8000_0000)
        .unwrap_or(false)
}

#[cfg(test)]
mod tests {
    use super::*;
    extern crate std;

    // BIP-39 seed for "abandon abandon ... about" (empty passphrase).
    const TEST_SEED_HEX: &str = "5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4";

    #[test]
    fn test_compute_context_hash_hex_official_vector() {
        let seed = hex::decode(TEST_SEED_HEX).unwrap();
        // "test-app" is NOT on the allow-list, so go below validation to prove the
        // crypto path; allow-list enforcement is covered separately below.
        let ikm = get_private_key_by_seed(&seed, &IKM_DERIVATION_PATH.to_string())
            .unwrap()
            .secret_bytes();
        let connected_pubkey = get_public_key_by_seed(&seed, &"m/44'/0'/0'/0/0".to_string())
            .unwrap()
            .serialize();
        let out = derive_context_hash(&DeriveContextHashInput {
            ikm,
            app_name: "test-app",
            network: BabylonNetwork::Mainnet,
            connected_pubkey,
            context: &hex::decode("deadbeef").unwrap(),
        })
        .unwrap();
        assert_eq!(
            hex::encode(out),
            "f82ced3be0e29591a7863ece03d65f79fb494fe0de7203549855f462455df008"
        );
    }

    #[test]
    fn test_compute_rejects_disallowed_app() {
        let seed = hex::decode(TEST_SEED_HEX).unwrap();
        let err = compute_context_hash_hex(
            &seed,
            "test-app",
            "bitcoin-mainnet",
            "m/44'/0'/0'/0/0",
            "deadbeef",
        )
        .unwrap_err();
        assert!(matches!(err, BabylonError::AppNotAllowed(_)));
    }

    #[test]
    fn test_compute_allowed_app_succeeds() {
        let seed = hex::decode(TEST_SEED_HEX).unwrap();
        let hex_out = compute_context_hash_hex(
            &seed,
            "babylon-btc-vault",
            "bitcoin-mainnet",
            "m/86'/0'/0'/0/0",
            "deadbeef",
        )
        .unwrap();
        assert_eq!(hex_out.len(), 64);
    }

    #[test]
    fn test_compute_rejects_unsupported_network() {
        let seed = hex::decode(TEST_SEED_HEX).unwrap();
        let err = compute_context_hash_hex(
            &seed,
            "babylon-btc-vault",
            "ethereum",
            "m/86'/0'/0'/0/0",
            "deadbeef",
        )
        .unwrap_err();
        assert!(matches!(err, BabylonError::UnsupportedNetwork(_)));
    }

    #[test]
    fn test_path_purpose() {
        assert_eq!(path_purpose("m/44'/0'/0'/0/0").unwrap(), 44);
        assert_eq!(path_purpose("M/49'/0'/0'/0/0").unwrap(), 49);
        assert_eq!(path_purpose("86'/0'/0'/0/0").unwrap(), 86);
        assert_eq!(path_purpose("m/84h/0h/0h/0/0").unwrap(), 84);
        assert!(path_purpose("m/x/0").is_err());
    }

    #[test]
    fn test_validate_cached_xpub_path_accepts_cached_mainnet_paths() {
        for path in [
            "44'/0'/0'/0/0",
            "m/49'/0'/0'/0/0",
            "M/84'/0'/0'/0/0",
            "86'/0'/0'/0/0",
        ] {
            validate_cached_xpub_path("bitcoin-mainnet", path).unwrap();
        }
    }

    #[test]
    fn test_validate_cached_xpub_path_accepts_cached_paths_for_supported_networks() {
        for network in ["bitcoin-testnet", "bitcoin-signet"] {
            validate_cached_xpub_path(network, "m/84'/0'/0'/0/0").unwrap();
        }
    }

    #[test]
    fn test_validate_cached_xpub_path_rejects_non_cached_paths() {
        assert!(validate_cached_xpub_path("bitcoin-mainnet", "m/44'/1'/0'/0/0").is_err());
        assert!(validate_cached_xpub_path("bitcoin-mainnet", "m/48'/0'/0'/0/0").is_err());
        assert!(validate_cached_xpub_path("bitcoin-mainnet", "m/44'/0'/1'/0/0").is_err());
        assert!(validate_cached_xpub_path("bitcoin-mainnet", "m/44'/0'/0'").is_err());
        assert!(validate_cached_xpub_path("bitcoin-mainnet", "m/84'/0'/0'/").is_err());
        assert!(validate_cached_xpub_path("bitcoin-mainnet", "m/84'/0'/0'/0'").is_err());
        assert!(validate_cached_xpub_path("bitcoin-mainnet", "m/84'/0'/0'/0'/0").is_err());
        assert!(validate_cached_xpub_path("bitcoin-mainnet", "m/84'/0'/0'/0/0'").is_err());
        assert!(validate_cached_xpub_path("bitcoin-mainnet", "m/84'/0'/0'/0/0/1").is_err());
        assert!(validate_cached_xpub_path("bitcoin-mainnet", "m/84'/0'/0'/0/2147483648").is_err());
        assert!(validate_cached_xpub_path(
            "bitcoin-mainnet",
            "m/44'/0'/0'/0/000000000000000000000000000000000000000000000000000"
        )
        .is_err());
        assert!(validate_cached_xpub_path("bitcoin-regtest", "m/84'/0'/0'/0/0").is_err());
    }

    #[test]
    fn test_verify_displayed_address_matches_seed_path() {
        let seed = hex::decode(TEST_SEED_HEX).unwrap();
        let key_path = "m/84'/0'/0'/0/0";
        let address =
            derive_connected_address_from_seed(&seed, "bitcoin-testnet", key_path).unwrap();
        verify_displayed_address(&seed, "bitcoin-testnet", key_path, &address).unwrap();
    }

    #[test]
    fn test_verify_displayed_address_rejects_mismatch() {
        let seed = hex::decode(TEST_SEED_HEX).unwrap();
        let err =
            verify_displayed_address(&seed, "bitcoin-mainnet", "m/84'/0'/0'/0/0", "bc1qmismatch")
                .unwrap_err();
        assert!(matches!(err, RustCError::InvalidData(_)));
    }

    // `qr-hardware-call` CBOR produced by the JS lib (@keystonehq/bc-ur-registry) and
    // pinned identically in the ur-registry Rust tests. Exercises the real decode path.
    const JS_CBOR_HEX: &str = "a4010102d90517a40171626162796c6f6e2d6274632d7661756c74026f626974636f696e2d6d61696e6e657403d90130a1018a182cf500f500f500f400f4046864656164626565660367626162796c6f6e0401";

    #[test]
    fn test_decode_request_from_js_cbor() {
        let bytes = hex::decode(JS_CBOR_HEX).unwrap();
        let mut call = QRHardwareCall::try_from(bytes).unwrap();
        let req = unsafe { decode_request(&mut call as *mut QRHardwareCall as PtrUR) }.unwrap();

        assert_eq!(req.app_name, "babylon-btc-vault");
        assert_eq!(req.network, "bitcoin-mainnet");
        assert_eq!(req.key_path, "44'/0'/0'/0/0");
        assert_eq!(req.context, "deadbeef");
        assert_eq!(req.origin, Some("babylon".to_string()));
    }

    #[test]
    fn test_compute_from_js_cbor_end_to_end() {
        let bytes = hex::decode(JS_CBOR_HEX).unwrap();
        let mut call = QRHardwareCall::try_from(bytes).unwrap();
        let req = unsafe { decode_request(&mut call as *mut QRHardwareCall as PtrUR) }.unwrap();

        let seed = hex::decode(TEST_SEED_HEX).unwrap();
        let hash = compute_context_hash_hex(
            &seed,
            &req.app_name,
            &req.network,
            &req.key_path,
            &req.context,
        )
        .unwrap();
        // Deterministic result for (babylon-btc-vault, bitcoin-mainnet, 44'/0'/0'/0/0,
        // deadbeef) over the standard abandon..about seed.
        assert_eq!(
            hash,
            "564906234635460f327d8c7d87a5a49054b672b725d5b8f0bc4a239956f32ba2"
        );
    }
}
