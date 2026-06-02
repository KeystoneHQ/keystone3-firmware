//! FFI layer for Babylon `deriveContextHash` (Phase 2).
//!
//! The cryptographic core lives in `app_babylon`. This module wires it to C:
//! parse a request into display data, derive the connected address, and produce
//! the 32-byte result as a `Bytes` UR.
//!
//! NOTE: the request type [`DeriveContextHashRequest`] is a *mock* stand-in for the
//! decoded `qr-hardware-call` params. Phase 4 replaces the mock constructor with a
//! real `ur_registry` decode (`CallParams::DeriveContextHash`); the rest of this
//! module is unaffected.

use alloc::string::{String, ToString};

use app_babylon::errors::BabylonError;
use app_babylon::{
    derive_context_hash, validate_app_name, validate_context_hex, BabylonNetwork,
    DeriveContextHashInput, IKM_DERIVATION_PATH,
};
use bitcoin::key::UntweakedPublicKey;
use bitcoin::secp256k1::Secp256k1;
use bitcoin::{Address, CompressedPublicKey, Network as BitcoinNetwork, PublicKey};
use cty::c_char;
use keystore::algorithms::secp256k1::{get_private_key_by_seed, get_public_key_by_seed};
use ur_registry::bytes::Bytes;
use ur_registry::traits::RegistryItem;
use zeroize::Zeroize;

use crate::common::errors::RustCError;
use crate::common::free::Free;
use crate::common::structs::{Response, SimpleResponse};
use crate::common::types::{PtrBytes, PtrString, PtrT};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::{free_str_ptr, impl_c_ptr, make_free_method};

impl From<BabylonError> for RustCError {
    fn from(value: BabylonError) -> Self {
        RustCError::InvalidData(value.to_string())
    }
}

/// Decoded derive-context-hash request (mock stand-in for the UR params).
pub struct DeriveContextHashRequest {
    app_name: String,
    network: String,
    key_path: String,
    context: String,
    origin: Option<String>,
}

impl_c_ptr!(DeriveContextHashRequest);

impl Free for DeriveContextHashRequest {
    unsafe fn free(&self) {}
}

make_free_method!(DeriveContextHashRequest);

/// Display data shown on the approval screen (no secret material).
#[repr(C)]
pub struct DeriveContextHashCallData {
    pub app_name: PtrString,
    pub network: PtrString,
    pub key_path: PtrString,
    pub address: PtrString,
    pub context: PtrString,
    pub origin: PtrString,
}

impl_c_ptr!(DeriveContextHashCallData);

impl Free for DeriveContextHashCallData {
    unsafe fn free(&self) {
        free_str_ptr!(self.app_name);
        free_str_ptr!(self.network);
        free_str_ptr!(self.key_path);
        free_str_ptr!(self.address);
        free_str_ptr!(self.context);
        free_str_ptr!(self.origin);
    }
}

make_free_method!(Response<DeriveContextHashCallData>);

/// Construct a mock decoded request from C strings.
///
/// This stands in for `QRHardwareCall` → `CallParams::DeriveContextHash` decoding
/// until the `ur_registry` extension lands (Phase 4). `origin` may be NULL.
#[no_mangle]
pub extern "C" fn derive_context_hash_new_mock_request(
    app_name: PtrString,
    network: PtrString,
    key_path: PtrString,
    context: PtrString,
    origin: PtrString,
) -> PtrT<DeriveContextHashRequest> {
    unsafe {
        DeriveContextHashRequest {
            app_name: recover_c_char(app_name),
            network: recover_c_char(network),
            key_path: recover_c_char(key_path),
            context: recover_c_char(context),
            origin: if origin.is_null() {
                None
            } else {
                Some(recover_c_char(origin))
            },
        }
        .c_ptr()
    }
}

/// Validate the request and produce display data for the approval screen.
///
/// Rejects disallowed appNames and unsupported networks *before* the screen is
/// shown. The connected `address` is left NULL here and filled in by
/// [`derive_context_hash_address`] once the device can derive the key.
#[no_mangle]
pub extern "C" fn parse_derive_context_hash(
    request: PtrT<DeriveContextHashRequest>,
) -> PtrT<Response<DeriveContextHashCallData>> {
    let request = unsafe { &*request };

    if let Err(e) = validate_app_name(&request.app_name) {
        return Response::from(RustCError::from(e)).c_ptr();
    }
    if let Err(e) = BabylonNetwork::from_name(&request.network) {
        return Response::from(RustCError::from(e)).c_ptr();
    }
    if let Err(e) = validate_context_hex(&request.context) {
        return Response::from(RustCError::from(e)).c_ptr();
    }

    let data = DeriveContextHashCallData {
        app_name: convert_c_char(request.app_name.clone()),
        network: convert_c_char(request.network.clone()),
        key_path: convert_c_char(request.key_path.clone()),
        address: core::ptr::null_mut(),
        context: convert_c_char(request.context.clone()),
        origin: match &request.origin {
            Some(o) => convert_c_char(o.clone()),
            None => core::ptr::null_mut(),
        },
    };
    Response::success(data).c_ptr()
}

/// Derive the Bitcoin address for the connected key (for the confirmation screen).
///
/// Uses the seed to derive the pubkey at `key_path`, then renders an address whose
/// script type is inferred from the path purpose (44/49/84/86).
#[no_mangle]
pub extern "C" fn derive_context_hash_address(
    request: PtrT<DeriveContextHashRequest>,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<SimpleResponse<c_char>> {
    let request = unsafe { &*request };
    let seed = unsafe { core::slice::from_raw_parts(seed, seed_len as usize) };

    match derive_connected_address(seed, &request.network, &request.key_path) {
        Ok(address) => {
            SimpleResponse::success(convert_c_char(address) as *mut c_char).simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

/// Run the full derivation and return the 32-byte result as a hex `Bytes` UR.
#[no_mangle]
pub extern "C" fn generate_derive_context_hash_ur(
    request: PtrT<DeriveContextHashRequest>,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let request = unsafe { &*request };
    let mut seed = unsafe { core::slice::from_raw_parts(seed, seed_len as usize).to_vec() };

    let result = compute_context_hash_hex(
        &seed,
        &request.app_name,
        &request.network,
        &request.key_path,
        &request.context,
    );
    seed.zeroize();

    let hash_hex = match result {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(RustCError::from(e)).c_ptr(),
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
    let connected_pubkey = get_public_key_by_seed(seed, &key_path.to_string())
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

/// Derive the displayable Bitcoin address for the connected key.
fn derive_connected_address(
    seed: &[u8],
    network: &str,
    key_path: &str,
) -> Result<String, RustCError> {
    let network = BabylonNetwork::from_name(network).map_err(RustCError::from)?;
    let pubkey = get_public_key_by_seed(seed, &key_path.to_string())
        .map_err(|e| RustCError::InvalidData(e.to_string()))?;

    let btc_network = match network {
        BabylonNetwork::Mainnet => BitcoinNetwork::Bitcoin,
        BabylonNetwork::Testnet => BitcoinNetwork::Testnet,
        BabylonNetwork::Signet => BitcoinNetwork::Signet,
        BabylonNetwork::Regtest => BitcoinNetwork::Regtest,
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
        .trim_start_matches('m')
        .trim_start_matches('/');
    let first = trimmed.split('/').next().unwrap_or("");
    let digits = first
        .trim_end_matches(|c| c == '\'' || c == 'h' || c == 'H');
    digits.parse::<u32>().map_err(|_| RustCError::InvalidHDPath)
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
        assert_eq!(path_purpose("86'/0'/0'/0/0").unwrap(), 86);
        assert_eq!(path_purpose("m/84h/0h/0h/0/0").unwrap(), 84);
        assert!(path_purpose("m/x/0").is_err());
    }

    #[test]
    fn test_derive_connected_address_taproot_mainnet() {
        let seed = hex::decode(TEST_SEED_HEX).unwrap();
        let addr = derive_connected_address(&seed, "bitcoin-mainnet", "m/86'/0'/0'/0/0").unwrap();
        assert!(addr.starts_with("bc1p"), "expected p2tr mainnet, got {addr}");
    }
}
