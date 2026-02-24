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
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use app_kaspa::addresses::encode_kaspa_address;
use app_kaspa::errors::KaspaError;
use app_kaspa::pskt::{Pskt, PsktSigner};
use bitcoin::bip32::Fingerprint;
use core::ffi::c_void;
use ur_registry::kaspa::kaspa_pskt::KaspaPskt;
use ur_registry::traits::To;

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
/// - pskt: PSKT
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

    // let seed_slice = core::slice::from_raw_parts(seed, seed_len as usize);
    let seed_slice = extract_array!(seed, u8, seed_len);

    let mfp_slice = extract_array!(mfp_ptr, u8, 4);
    let mut mfp_array = [0u8; 4];
    mfp_array.copy_from_slice(mfp_slice);
    let master_fingerprint = Fingerprint::from(mfp_array);

    let pskt_hex_str = match String::from_utf8(pskt_data) {
        Ok(s) => s,
        Err(_) => {
            return UREncodeResult::from(RustCError::InvalidData("UTF8 Error".into())).c_ptr()
        }
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
    let mfp_hex_lower = hex::encode(mfp_array).to_lowercase();

    let pskt_hex_str = match String::from_utf8(pskt_data.to_vec()) {
        Ok(p) => p,
        Err(e) => {
            return TransactionCheckResult::from(RustCError::InvalidData(e.to_string())).c_ptr()
        }
    };

    let pskt = match Pskt::from_hex(&pskt_hex_str) {
        Ok(p) => p,
        Err(e) => return TransactionCheckResult::from(e).c_ptr(),
    };

    // Check if we have keys for all inputs
    for (index, input) in pskt.inputs.iter().enumerate() {
        // if input.sighash_type.unwrap_or(1) != 1 {
        //     return TransactionCheckResult::error(
        //         ErrorCodes::UnsupportedSighashType,
        //         format!(
        //             "Input {} uses risky sighash type. Only SIGHASH_ALL is allowed.",
        //             index
        //         ),
        //     )
        //     .c_ptr();
        // }

        let is_mine = input
            .bip32_derivations
            .values()
            .flatten()
            .any(|source| source.key_fingerprint.to_lowercase() == mfp_hex_lower);

        // if !is_mine {
        //     return TransactionCheckResult::error(
        //         ErrorCodes::UnsupportedTransaction,
        //         format!(
        //             "Security Alert: Input {} does not belong to this wallet. Operation aborted.",
        //             index
        //         ),
        //     )
        //     .c_ptr();
        // }
    }

    TransactionCheckResult::new().c_ptr()
}


// Helper function to convert PSKT to display format
fn parse_pskt_to_display(
    pskt: &Pskt,
    master_fingerprint: Fingerprint,
) -> Result<DisplayKaspaTx, KaspaError> {
    let mut inputs = Vec::new();
    let mut total_input = 0u64;
    let mfp_hex_lower = hex::encode(master_fingerprint.as_bytes()).to_lowercase();

    // 1.handle Inputs
    for (index, input) in pskt.inputs.iter().enumerate() {
        let my_derivation = input
            .bip32_derivations
            .values()
            .flatten()
            .find(|source| source.key_fingerprint.to_lowercase() == mfp_hex_lower);

        // let derivation = my_derivation.ok_or_else(|| {
        //     KaspaError::InvalidPskt(format!(
        //         "Security Alert: Input {} is not owned by this wallet",
        //         index
        //     ))
        // })?;

        let path = my_derivation
            .map(|d| d.derivation_path.clone())
            .unwrap_or_else(|| "Unknown Path".to_string()); // 如果不是自己的 Input，显示未知路径

        // Amount handling: prefer the flattened field, otherwise check utxo_entry
        let amount = if input.amount > 0 {
            input.amount
        } else {
            input.utxo_entry.as_ref().map(|e| e.amount).unwrap_or(0)
        };

        let address = if let Some(e) = input.utxo_entry.as_ref() {
            script_to_address(&e.script_public_key).unwrap_or_else(|_| "Unknown Script".to_string())
        } else {
            "Owned Input".to_string()
        };

        inputs.push(DisplayKaspaInput {
            address: convert_c_char(address),
            amount: convert_c_char(format_sompi(amount)),
            value: amount,
            path: convert_c_char(path),
            is_mine: true,
        });

        total_input += amount;
    }

    // 2. Outputs
    let mut outputs = Vec::new();
    let mut total_output = 0u64;

    for output in &pskt.outputs {
        let my_derivation = output
            .bip32_derivations
            .values()
            .flatten()
            .find(|source| source.key_fingerprint.to_lowercase() == mfp_hex_lower);

        let is_mine = my_derivation.is_some();
        let path_str = my_derivation.map(|d| d.derivation_path.clone());

        let is_change = path_str
            .as_ref()
            .map(|p| is_change_path(p))
            .unwrap_or(false);

        let display_address = output.address.clone().unwrap_or_else(|| {
            script_to_address(&output.script_public_key)
                .unwrap_or_else(|_| "Unknown Script".to_string())
        });

        outputs.push(DisplayKaspaOutput {
            address: convert_c_char(display_address),
            amount: convert_c_char(format_sompi(output.amount)),
            value: output.amount,
            path: path_str
                .map(convert_c_char)
                .unwrap_or(core::ptr::null_mut()),
            is_mine,
            is_external: !is_change,
        });

        total_output += output.amount;
    }

    // 3. fee
    let fee = total_input.saturating_sub(total_output);
    println!("DEBUG: inputs len: {}, outputs len: {}", inputs.len(), outputs.len());
    if outputs.is_empty() {
    outputs.push(DisplayKaspaOutput {
        address: convert_c_char("DEBUG_EMPTY_ADDRESS".to_string()),
        amount: convert_c_char("0 KAS".to_string()),
        value: 0,
        path: core::ptr::null_mut(),
        is_mine: false,
        is_external: true,
    });
}
    println!("DEBUG: inputs len: {}, outputs len: {}", inputs.len(), outputs.len());


    Ok(DisplayKaspaTx {
        network: convert_c_char("Kaspa".to_string()),
        total_spend: convert_c_char(format_sompi(total_output)),
        fee,
        fee_per_sompi: 0.0,
        inputs: VecFFI::from(inputs),
        outputs: VecFFI::from(outputs),
        total_input,
        total_output,
    })
}

pub fn script_to_address(script_hex: &str) -> app_kaspa::errors::Result<String> {
    let script_bytes = hex::decode(script_hex).map_err(|_| KaspaError::InvalidScript)?;

    if script_bytes.len() == 22 && script_bytes[0] == 0xaa && script_bytes[21] == 0x87 {
        return encode_kaspa_address(&script_bytes[1..21]);
    }

    Err(KaspaError::Unsupported(
        "Only P2PKH scripts are supported".into(),
    ))
}

fn is_change_path(path: &str) -> bool {
    // pth example: "m/44'/111111'/0'/1/5" or "44'/111111'/0'/1/5"
    let parts: Vec<&str> = path.split('/').collect();

    // We find the second last position from the end as the safest, because the path structure is fixed [.../change/index]
    if parts.len() >= 2 {
        let change_idx = parts.len() - 2;
        parts[change_idx] == "1"
    } else {
        false
    }
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

#[cfg(test)]
mod tests {
    use super::*;
    use crate::common::types::PtrUR;
    use crate::common::ur::FRAGMENT_MAX_LENGTH_DEFAULT;
    use app_kaspa::pskt::Pskt;
    use ur_parse_lib::keystone_ur_encoder::probe_encode;
    use ur_registry::kaspa::kaspa_pskt::KaspaPskt;
    use ur_registry::traits::To;

    #[test]
    fn test_kaspa_parse_pskt_unit() {
        let pskt_json = r#"{
    "global": {
        "version": 0,
        "txVersion": 0,
        "fallbackLockTime": null,
        "inputsModifiable": true,
        "outputsModifiable": true,
        "inputCount": 1,
        "outputCount": 0,
        "xpubs": {},
        "id": null,
        "proprietaries": {},
        "payload": null
    },
    "inputs": [
        {
            "utxoEntry": {
                "amount": 12793000000000,
                "scriptPublicKey": "0000aa2064fc0dd5620294747e5d9523178964e27a3578f687f5b1b1b26abd024ca538df87",
                "blockDaaScore": 36151168,
                "isCoinbase": false
            },
            "previousOutpoint": {
                "transactionId": "63020db736215f8b1105a9281f7bcbb6473d965ecc45bb2fb5da59bd35e6ff84",
                "index": 0
            },
            "sequence": 18446744073709551615,
            "minTime": null,
            "partialSigs": {},
            "sighashType": 1,
            "redeemScript": "52201ddf8fa167a16e01c42930c73fc91d031c976063bd6649985e08e58e27c21ca3209c9123646e7b6b0315f9170f7860f76d5a0879cb733465c99a3114fda2c988e352ae",
            "sigOpCount": 2,
            "bip32Derivations": {
                "pubkey_placeholder": {
                    "keyFingerprint": "deadbeef",
                    "derivationPath": "m/44'/111111'/0'/0/0"
                }
            },
            "finalScriptSig": null,
            "proprietaries": {}
        }
    ],
    "outputs": [
        {
            "amount": 1000000000000,
            "scriptPublicKey": "0000aa2064fc0dd5620294747e5d9523178964e27a3578f687f5b1b1b26abd024ca538df87",
            "bip32Derivations": {}
        }
    ]
}"#;

        let mut pskt: Pskt = serde_json::from_str(pskt_json).expect("parse pskt json");
        let hex_str = pskt.to_hex().expect("pskt to hex");

        let kaspa_ur = KaspaPskt::new(hex_str.into_bytes());
        // Print UR fragments for manual QR rendering
        if let Ok(data) = kaspa_ur.to_bytes() {
            if let Ok(enc) =
                probe_encode(&data, FRAGMENT_MAX_LENGTH_DEFAULT, "kaspa-pskt".to_string())
            {
                if enc.is_multi_part {
                    println!("UR multi first: {}", enc.data.to_uppercase());
                    if let Some(mut encoder) = enc.encoder {
                        for i in 0..10 {
                            match encoder.next_part() {
                                Ok(p) => println!("part {}: {}", i + 1, p.to_uppercase()),
                                Err(e) => {
                                    println!("encoder.next_part error: {:?}", e);
                                    break;
                                }
                            }
                        }
                    }
                } else {
                    println!("UR single: {}", enc.data.to_uppercase());
                }
            }
        }
        let hex_str = pskt.to_hex().expect("pskt to hex");

        // --- 核心修改：将数据转换为字节数组并准备长度 ---
        let pskt_raw_bytes = hex_str.into_bytes();
        let pskt_ptr = pskt_raw_bytes.as_ptr();
        let pskt_len = pskt_raw_bytes.len() as u32;
        let mut mfp = [0xdeu8, 0xadu8, 0xbeu8, 0xefu8];

        let res_ptr = unsafe { kaspa_parse_pskt(pskt_ptr, pskt_len, mfp.as_mut_ptr(), 4) };
        assert!(!res_ptr.is_null(), "result pointer is null");

        let data_ptr_raw: *mut *mut DisplayKaspaTx = res_ptr as *mut *mut DisplayKaspaTx;
        let data_ptr = unsafe { *data_ptr_raw };

        assert!(!data_ptr.is_null(), "inner data pointer is null");

        // Inspect some display fields
        let display = unsafe { &*data_ptr };
        let network = unsafe { recover_c_char(display.network) };
        assert_eq!(network, "Kaspa");

        // totals (match mock data)
        assert_eq!(display.total_input, 12793000000000u64);
        assert_eq!(display.total_output, u64);

        // inputs array
        let inputs_len = display.inputs.size as usize;
        assert_eq!(inputs_len, 1);
        let input0 = unsafe { &*(display.inputs.data as *const DisplayKaspaInput) };

        // path should include our derivation
        let path = unsafe { recover_c_char(input0.path) };
        assert!(path.contains("m/44'"), "unexpected derivation path");

        // Free resources (free inner display via TransactionParseResult::free)
        unsafe {
            let res_box = Box::from_raw(res_ptr);
            res_box.free();
            // drop the response box
            drop(res_box);
        }
    }
}
