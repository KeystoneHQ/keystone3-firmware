// FFI interface for Kaspa PSKT (Partially Signed Kaspa Transaction)
use crate::common::errors::*;
use crate::common::ffi::VecFFI;
use crate::common::free::Free;
use crate::common::structs::*;
use crate::common::types::*;
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::extract_array;
use crate::extract_ptr_with_type;
use crate::kaspa::structs::{DisplayKaspaInput, DisplayKaspaOutput, DisplayKaspaTx};
use alloc::boxed::Box;
use alloc::vec::Vec;
use app_kaspa::errors::KaspaError;
use app_kaspa::pskt::{Pskt, PsktSigner};
use bitcoin::bip32::Fingerprint;
use core::ffi::c_void;
use ur_registry::traits::To;
use alloc::string::{String, ToString};
use ur_registry::kaspa::kaspa_pskt::KaspaPskt;

/// Parse PSKT from hex string and display transaction details
///
/// # Parameters
/// - pskt_hex: PSKT hex string (format: "PSKT{hex_json}")
/// - mfp: Master fingerprint (4 bytes)
/// - mfp_len: Length of mfp (should be 4)
///
/// # Returns
/// TransactionParseResult containing DisplayKaspaTx or error
///
// #[no_mangle]
// pub unsafe extern "C" fn kaspa_parse_pskt(
//     pskt_hex: PtrString,
//     mfp: PtrBytes,
//     mfp_len: u32,
// ) -> *mut TransactionParseResult<DisplayKaspaTx> {
//     if mfp.is_null() || mfp_len != 4 {
//         return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
//     }

//     let pskt_hex_str = recover_c_char(pskt_hex);

//     let mfp_bytes = core::slice::from_raw_parts(mfp, mfp_len as usize);

//     let mut mfp_array = [0u8; 4];
//     mfp_array.copy_from_slice(mfp_bytes);
//     let master_fingerprint = Fingerprint::from(mfp_array);

//     // Parse PSKT
//     let pskt = match Pskt::from_hex(&pskt_hex_str) {
//         Ok(p) => p,
//         Err(e) => return TransactionParseResult::from(e).c_ptr(),
//     };

//     // Convert to display format
//     match parse_pskt_to_display(&pskt, master_fingerprint) {
//         Ok(display_tx) => {
//             TransactionParseResult::success(Box::into_raw(Box::new(display_tx))).c_ptr()
//         }
//         Err(e) => TransactionParseResult::from(e).c_ptr(),
//     }
// }

#[no_mangle]
pub unsafe extern "C" fn kaspa_parse_pskt(
    ptr: PtrUR,
    mfp_ptr: PtrBytes,
    mfp_len: u32,
) -> *mut TransactionParseResult<DisplayKaspaTx> {
    if mfp_len != 4 {
        return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }

    let crypto_pskt = extract_ptr_with_type!(ptr, KaspaPskt);
    let pskt_data = crypto_pskt.get_pskt();

    let mfp_slice = extract_array!(mfp_ptr, u8, 4);
    let mut mfp_array = [0u8; 4];
    mfp_array.copy_from_slice(mfp_slice);
    let master_fingerprint = Fingerprint::from(mfp_array);

    let pskt_hex_str: String = match String::from_utf8(pskt_data) {
        Ok(s) => s,
        Err(_) => return TransactionParseResult::from(RustCError::InvalidData("UTF8 Error".into())).c_ptr(),
    };
    let pskt_res = Pskt::from_hex(&pskt_hex_str);

    match pskt_res {
        Ok(pskt) => match parse_pskt_to_display(&pskt, master_fingerprint) {
            Ok(display_tx) => TransactionParseResult::success(Box::into_raw(Box::new(display_tx))).c_ptr(),
            Err(e) => TransactionParseResult::from(e).c_ptr(),
        },
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

/// Sign PSKT with seed
///
/// # Parameters
/// - pskt_hex: PSKT hex string
/// - seed: BIP39 seed bytes
/// - seed_len: Length of seed
/// - mfp: Master fingerprint (4 bytes)
/// - mfp_len: Length of mfp (should be 4)
///
/// # Returns
/// UREncodeResult containing signed PSKT hex or error
#[no_mangle]
pub unsafe extern "C" fn kaspa_sign_pskt(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    mfp_ptr: PtrBytes,
    mfp_len: u32,
) -> *mut UREncodeResult {
    if seed.is_null() || mfp_ptr.is_null() || mfp_len != 4 {
        return UREncodeResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }

    let crypto_pskt = extract_ptr_with_type!(ptr, KaspaPskt);
    let pskt_data = crypto_pskt.get_pskt();
    
    let seed_slice = core::slice::from_raw_parts(seed, seed_len as usize);

    let mfp_slice = extract_array!(mfp_ptr, u8, 4);
    let mut mfp_array = [0u8; 4];
    mfp_array.copy_from_slice(mfp_slice);
    let master_fingerprint = Fingerprint::from(mfp_array);

    let pskt_hex_str = match String::from_utf8(pskt_data) {
        Ok(s) => s,
        Err(_) => return UREncodeResult::from(RustCError::InvalidData("UTF8 Error".into())).c_ptr(),
    };

    let mut pskt = match Pskt::from_hex(&pskt_hex_str) {
        Ok(p) => p,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };

    let mut signer = PsktSigner::new(&mut pskt);
    if let Err(e) = signer.sign(seed_slice, master_fingerprint) {
        return UREncodeResult::from(e).c_ptr();
    }

    match pskt.to_hex() {
        Ok(hex_str) => {
            let kaspa_ur = ur_registry::kaspa::kaspa_pskt::KaspaPskt::new(hex_str.into_bytes());
            match kaspa_ur.to_bytes() {
                Ok(pskt_bytes) => UREncodeResult::encode(
                    pskt_bytes,
                    "kaspa-pskt".to_string(),
                    FRAGMENT_MAX_LENGTH_DEFAULT,
                )
                .c_ptr(),
                Err(e) => UREncodeResult::from(KaspaError::InvalidPskt(e.to_string())).c_ptr(),
            }
        }
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

/// Check if PSKT can be signed with the given master fingerprint
///
/// # Parameters
/// - pskt_hex: PSKT hex string
/// - mfp: Master fingerprint (4 bytes)
/// - mfp_len: Length of mfp (should be 4)
///
/// # Returns
/// TransactionCheckResult indicating if PSKT is valid for signing
#[no_mangle]
pub unsafe extern "C" fn kaspa_check_pskt(
    ptr: PtrUR,
    mfp_ptr: PtrBytes,
    mfp_len: u32,
) -> *mut TransactionCheckResult {
    if mfp_ptr.is_null() || mfp_len != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }

    let crypto_pskt = extract_ptr_with_type!(ptr, KaspaPskt);
    let pskt_data = crypto_pskt.get_pskt();
    
    let mfp_slice = extract_array!(mfp_ptr, u8, 4);
    let mut mfp_array = [0u8; 4];
    mfp_array.copy_from_slice(mfp_slice);
    let master_fingerprint = Fingerprint::from(mfp_array);

    let pskt_hex_str = match String::from_utf8(pskt_data) {
        Ok(s) => s,
        Err(_) => return TransactionCheckResult::from(RustCError::InvalidData("UTF8 Error".into())).c_ptr(),
    };

    let pskt = match Pskt::from_hex(&pskt_hex_str) {
        Ok(p) => p,
        Err(e) => return TransactionCheckResult::from(e).c_ptr(),
    };

    // Check if we have keys for all inputs
    for input in &pskt.inputs {
        let has_our_key = if let Some(ref mfp_hex) = input.master_fingerprint {
            if let Ok(mfp_bytes) = hex::decode(mfp_hex) {
                if mfp_bytes.len() == 4 {
                    let mut mfp_array = [0u8; 4];
                    mfp_array.copy_from_slice(&mfp_bytes);
                    Fingerprint::from(mfp_array) == master_fingerprint
                } else {
                    false
                }
            } else {
                false
            }
        } else {
            false
        };

        if !has_our_key {
            return TransactionCheckResult::error(
                ErrorCodes::UnsupportedTransaction,
                "No matching keys found for input".to_string(),
            )
            .c_ptr();
        }
    }

    TransactionCheckResult::new().c_ptr()
}

// Note: kaspa_extract_pskt_tx removed
// Transaction extraction should be done by software wallet (Kaspium)
// using kaspa_consensus_core after receiving signed PSKT

// Helper function to convert PSKT to display format
fn parse_pskt_to_display(
    pskt: &Pskt,
    master_fingerprint: Fingerprint,
) -> Result<DisplayKaspaTx, KaspaError> {
    let mut inputs = Vec::new();
    let mut total_input = 0u64;

    // Parse inputs
    for input in &pskt.inputs {
        // Check if this input belongs to us
        let is_mine = if let Some(ref mfp_hex) = input.master_fingerprint {
            if let Ok(mfp_bytes) = hex::decode(mfp_hex) {
                if mfp_bytes.len() == 4 {
                    let mut mfp_array = [0u8; 4];
                    mfp_array.copy_from_slice(&mfp_bytes);
                    Fingerprint::from(mfp_array) == master_fingerprint
                } else {
                    false
                }
            } else {
                false
            }
        } else {
            false
        };

        let path = if is_mine {
            input.derivation_path.clone()
        } else {
            None
        };

        inputs.push(DisplayKaspaInput {
            //need to show address
            address: convert_c_char("".to_string()),
            amount: convert_c_char(format_sompi(input.amount)),
            value: input.amount,
            path: path.map(convert_c_char).unwrap_or(core::ptr::null_mut()),
            is_mine,
        });

        total_input += input.amount;
    }

    // Parse outputs
    let mut outputs = Vec::new();
    let mut total_output = 0u64;

    for output in &pskt.outputs {
        // Check if this output belongs to us (change address)
        let (is_mine, is_change) = if let Some(ref mfp_hex) = output.master_fingerprint {
            if let Ok(mfp_bytes) = hex::decode(mfp_hex) {
                if mfp_bytes.len() == 4 {
                    let mut mfp_array = [0u8; 4];
                    mfp_array.copy_from_slice(&mfp_bytes);
                    let is_ours = Fingerprint::from(mfp_array) == master_fingerprint;
                    // Check if it's a change address (BIP44 change chain = 1)
                    let is_change_chain = output
                        .derivation_path
                        .as_ref()
                        .map(|p| p.contains("/1/"))
                        .unwrap_or(false);
                    (is_ours, is_change_chain)
                } else {
                    (false, false)
                }
            } else {
                (false, false)
            }
        } else {
            (false, false)
        };

        let path = if is_mine {
            output.derivation_path.clone()
        } else {
            None
        };

        outputs.push(DisplayKaspaOutput {
            address: convert_c_char(output.address.clone().unwrap_or_default()),
            amount: convert_c_char(format_sompi(output.amount)),
            value: output.amount,
            path: path.map(convert_c_char).unwrap_or(core::ptr::null_mut()),
            is_mine,
            is_external: !is_change, // External = not change
        });

        total_output += output.amount;
    }

    let fee = total_input.saturating_sub(total_output);

    Ok(DisplayKaspaTx {
        network: convert_c_char("Kaspa Mainnet".to_string()),
        total_spend: convert_c_char(format_sompi(total_output)),
        fee,
        fee_per_sompi: 0.0,
        inputs: VecFFI::from(inputs),
        outputs: VecFFI::from(outputs),
        total_input,
        total_output,
    })
}

// Format Sompi (Kaspa's smallest unit) to string
// Uses integer arithmetic to avoid floating point operations in embedded firmware
fn format_sompi(sompi: u64) -> alloc::string::String {
    // 1 KAS = 100,000,000 Sompi
    const SOMPI_PER_KAS: u64 = 100_000_000;
    let kas = sompi / SOMPI_PER_KAS;
    let remainder = sompi % SOMPI_PER_KAS;
    alloc::format!("{}.{:08} KAS", kas, remainder)
}
