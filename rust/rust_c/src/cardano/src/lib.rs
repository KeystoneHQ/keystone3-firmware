#![no_std]

extern crate alloc;

use alloc::string::{String, ToString};
use alloc::{format, slice};

use alloc::vec::Vec;
use app_cardano::errors::CardanoError;
use app_cardano::governance;
use app_cardano::structs::{CardanoCertKey, CardanoUtxo, ParseContext};
use app_cardano::transaction::calc_icarus_master_key;
use core::str::FromStr;
use cty::c_char;
use third_party::bitcoin::bip32::DerivationPath;
use third_party::ed25519_bip32_core::XPrv;
use third_party::hex;

use third_party::ur_registry::cardano::cardano_catalyst_signature::CardanoCatalystSignature;
use third_party::ur_registry::cardano::cardano_catalyst_voting_registration::CardanoCatalystVotingRegistrationRequest;
use third_party::ur_registry::cardano::cardano_delegation::CardanoDelegation;
use third_party::ur_registry::cardano::cardano_sign_data_request::CardanoSignDataRequest;
use third_party::ur_registry::cardano::cardano_sign_data_signature::CardanoSignDataSignature;
use third_party::ur_registry::cardano::cardano_sign_request::CardanoSignRequest;
use third_party::ur_registry::cardano::cardano_signature::CardanoSignature;
use third_party::ur_registry::crypto_key_path::CryptoKeyPath;

use crate::structs::{DisplayCardanoCatalyst, DisplayCardanoSignData, DisplayCardanoTx};
use common_rust_c::errors::{RustCError, R};
use common_rust_c::extract_ptr_with_type;
use common_rust_c::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use common_rust_c::types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{convert_c_char, recover_c_char};
use third_party::ur_registry::registry_types::{
    CARDANO_CATALYST_VOTING_REGISTRATION_SIGNATURE, CARDANO_SIGNATURE, CARDANO_SIGN_DATA_SIGNATURE,
};

pub mod address;
pub mod structs;

#[no_mangle]
pub extern "C" fn cardano_check_catalyst(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
) -> PtrT<TransactionCheckResult> {
    let cardano_catalyst_request =
        extract_ptr_with_type!(ptr, CardanoCatalystVotingRegistrationRequest);
    let mfp = unsafe { slice::from_raw_parts(master_fingerprint, 4) };
    let ur_mfp = cardano_catalyst_request
        .get_derivation_path()
        .get_source_fingerprint()
        .ok_or(RustCError::InvalidMasterFingerprint);

    if let Ok(mfp) = mfp.try_into() as Result<[u8; 4], _> {
        if hex::encode(mfp) != hex::encode(ur_mfp.unwrap()) {
            return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
        }
    }

    TransactionCheckResult::new().c_ptr()
}

#[no_mangle]
pub extern "C" fn cardano_check_sign_data(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
) -> PtrT<TransactionCheckResult> {
    let cardano_sign_data_reqeust = extract_ptr_with_type!(ptr, CardanoSignDataRequest);
    let mfp = unsafe { slice::from_raw_parts(master_fingerprint, 4) };
    let ur_mfp = cardano_sign_data_reqeust
        .get_derivation_path()
        .get_source_fingerprint()
        .ok_or(RustCError::InvalidMasterFingerprint);

    if let Ok(mfp) = mfp.try_into() as Result<[u8; 4], _> {
        if hex::encode(mfp) != hex::encode(ur_mfp.unwrap()) {
            return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
        }
    }

    TransactionCheckResult::new().c_ptr()
}

#[no_mangle]
pub extern "C" fn cardano_check_tx(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
) -> PtrT<TransactionCheckResult> {
    let cardano_sign_reqeust = extract_ptr_with_type!(ptr, CardanoSignRequest);
    let tx_hex = cardano_sign_reqeust.get_sign_data();
    let parse_context =
        prepare_parse_context(&cardano_sign_reqeust, master_fingerprint, cardano_xpub);
    match parse_context {
        Ok(parse_context) => match app_cardano::transaction::check_tx(tx_hex, parse_context) {
            Ok(_) => TransactionCheckResult::new().c_ptr(),
            Err(e) => TransactionCheckResult::from(e).c_ptr(),
        },
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn cardano_get_path(ptr: PtrUR) -> Ptr<SimpleResponse<c_char>> {
    let cardano_sign_reqeust = extract_ptr_with_type!(ptr, CardanoSignRequest);
    match cardano_sign_reqeust.get_cert_keys().get(0) {
        Some(_data) => match _data.get_key_path().get_path() {
            Some(_path) => {
                if let Some(path) = parse_cardano_root_path(_path) {
                    return SimpleResponse::success(convert_c_char(path)).simple_c_ptr();
                }
            }
            None => {}
        },
        None => {}
    };
    match cardano_sign_reqeust.get_utxos().get(0) {
        Some(_data) => match _data.get_key_path().get_path() {
            Some(_path) => {
                if let Some(path) = parse_cardano_root_path(_path) {
                    return SimpleResponse::success(convert_c_char(path)).simple_c_ptr();
                }
                SimpleResponse::from(CardanoError::InvalidTransaction(format!("invalid utxo")))
                    .simple_c_ptr()
            }
            None => SimpleResponse::from(CardanoError::InvalidTransaction(format!("invalid utxo")))
                .simple_c_ptr(),
        },
        None => SimpleResponse::from(CardanoError::InvalidTransaction(format!("invalid utxo")))
            .simple_c_ptr(),
    }
}

fn parse_cardano_root_path(path: String) -> Option<String> {
    let root_path = "1852'/1815'/";
    match path.strip_prefix(root_path) {
        Some(path) => {
            if let Some(index) = path.find("/") {
                let sub_path = &path[..index];
                Some(format!("{}{}", root_path, sub_path))
            } else {
                None
            }
        }
        None => None,
    }
}

#[no_mangle]
pub extern "C" fn cardano_parse_sign_data(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayCardanoSignData>> {
    let cardano_sign_data_reqeust = extract_ptr_with_type!(ptr, CardanoSignDataRequest);
    let sign_data = cardano_sign_data_reqeust.get_sign_data();
    let derviation_path = cardano_sign_data_reqeust
        .get_derivation_path()
        .get_path()
        .unwrap();
    let parsed_data = app_cardano::transaction::parse_sign_data(sign_data, derviation_path);
    match parsed_data {
        Ok(v) => TransactionParseResult::success(DisplayCardanoSignData::from(v).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn cardano_parse_catalyst(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayCardanoCatalyst>> {
    let cardano_catalyst_request =
        extract_ptr_with_type!(ptr, CardanoCatalystVotingRegistrationRequest);
    let res = DisplayCardanoCatalyst::from(cardano_catalyst_request.clone()).c_ptr();

    TransactionParseResult::success(res).c_ptr()
}

#[no_mangle]
pub extern "C" fn cardano_parse_tx(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
) -> PtrT<TransactionParseResult<DisplayCardanoTx>> {
    let cardano_sign_reqeust = extract_ptr_with_type!(ptr, CardanoSignRequest);
    let tx_hex = cardano_sign_reqeust.get_sign_data();
    let parse_context =
        prepare_parse_context(&cardano_sign_reqeust, master_fingerprint, cardano_xpub);
    match parse_context {
        Ok(parse_context) => match app_cardano::transaction::parse_tx(tx_hex, parse_context) {
            Ok(v) => TransactionParseResult::success(DisplayCardanoTx::from(v).c_ptr()).c_ptr(),
            Err(e) => TransactionParseResult::from(e).c_ptr(),
        },
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn cardano_sign_catalyst(
    ptr: PtrUR,
    entropy: PtrBytes,
    entropy_len: u32,
    passphrase: PtrString,
) -> PtrT<UREncodeResult> {
    let entropy = unsafe { alloc::slice::from_raw_parts(entropy, entropy_len as usize) };
    let passphrase = recover_c_char(passphrase);
    let icarus_master_key = calc_icarus_master_key(entropy, passphrase.as_bytes());
    cardano_sign_catalyst_by_icarus(ptr, icarus_master_key)
}

fn cardano_sign_catalyst_by_icarus(ptr: PtrUR, icarus_master_key: XPrv) -> PtrT<UREncodeResult> {
    let cardano_catalyst_request =
        extract_ptr_with_type!(ptr, CardanoCatalystVotingRegistrationRequest);
    let result = governance::sign(
        &cardano_catalyst_request
            .get_derivation_path()
            .get_path()
            .unwrap(),
        cardano_catalyst_request.get_delegations(),
        &cardano_catalyst_request.get_stake_pub(),
        &cardano_catalyst_request.get_payment_address(),
        cardano_catalyst_request.get_nonce(),
        cardano_catalyst_request.get_voting_purpose(),
        icarus_master_key,
    )
    .map(|v| {
        CardanoCatalystSignature::new(cardano_catalyst_request.get_request_id(), v.get_signature())
            .try_into()
    })
    .map_or_else(
        |e| UREncodeResult::from(e).c_ptr(),
        |v| {
            v.map_or_else(
                |e| UREncodeResult::from(e).c_ptr(),
                |data| {
                    UREncodeResult::encode(
                        data,
                        CARDANO_CATALYST_VOTING_REGISTRATION_SIGNATURE.get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                    )
                    .c_ptr()
                },
            )
        },
    );

    return result;
}

#[no_mangle]
pub extern "C" fn cardano_sign_sign_data(
    ptr: PtrUR,
    entropy: PtrBytes,
    entropy_len: u32,
    passphrase: PtrString,
) -> PtrT<UREncodeResult> {
    let entropy = unsafe { alloc::slice::from_raw_parts(entropy, entropy_len as usize) };
    let passphrase = recover_c_char(passphrase);
    let icarus_master_key = calc_icarus_master_key(entropy, passphrase.as_bytes());
    cardano_sign_sign_data_by_icarus(ptr, icarus_master_key)
}

fn cardano_sign_sign_data_by_icarus(ptr: PtrUR, icarus_master_key: XPrv) -> PtrT<UREncodeResult> {
    let cardano_sign_data_reqeust = extract_ptr_with_type!(ptr, CardanoSignDataRequest);
    let sign_data = cardano_sign_data_reqeust.get_sign_data();

    let result = app_cardano::transaction::sign_data(
        &cardano_sign_data_reqeust
            .get_derivation_path()
            .get_path()
            .unwrap(),
        hex::encode(sign_data).as_str(),
        icarus_master_key,
    )
    .map(|v| {
        CardanoSignDataSignature::new(
            cardano_sign_data_reqeust.get_request_id(),
            v.get_signature(),
            v.get_pub_key(),
        )
        .try_into()
    })
    .map_or_else(
        |e| UREncodeResult::from(e).c_ptr(),
        |v| {
            v.map_or_else(
                |e| UREncodeResult::from(e).c_ptr(),
                |data| {
                    UREncodeResult::encode(
                        data,
                        CARDANO_SIGN_DATA_SIGNATURE.get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                    )
                    .c_ptr()
                },
            )
        },
    );

    return result;
}

#[no_mangle]
pub extern "C" fn cardano_sign_tx(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
    entropy: PtrBytes,
    entropy_len: u32,
    passphrase: PtrString,
) -> PtrT<UREncodeResult> {
    let entropy = unsafe { alloc::slice::from_raw_parts(entropy, entropy_len as usize) };
    let passphrase = recover_c_char(passphrase);
    let icarus_master_key = calc_icarus_master_key(entropy, passphrase.as_bytes());
    cardano_sign_tx_by_icarus(ptr, master_fingerprint, cardano_xpub, icarus_master_key)
}

fn cardano_sign_tx_by_icarus(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
    icarus_master_key: XPrv,
) -> PtrT<UREncodeResult> {
    let cardano_sign_reqeust = extract_ptr_with_type!(ptr, CardanoSignRequest);
    let tx_hex = cardano_sign_reqeust.get_sign_data();
    let parse_context =
        prepare_parse_context(&cardano_sign_reqeust, master_fingerprint, cardano_xpub);

    match parse_context {
        Ok(parse_context) => {
            let sign_result =
                app_cardano::transaction::sign_tx(tx_hex, parse_context, icarus_master_key).map(
                    |v| CardanoSignature::new(cardano_sign_reqeust.get_request_id(), v).try_into(),
                );
            match sign_result {
                Ok(d) => match d {
                    Ok(data) => UREncodeResult::encode(
                        data,
                        CARDANO_SIGNATURE.get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                    )
                    .c_ptr(),
                    Err(e) => UREncodeResult::from(e).c_ptr(),
                },
                Err(e) => UREncodeResult::from(e).c_ptr(),
            }
        }
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

fn prepare_parse_context(
    cardano_sign_request: &CardanoSignRequest,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
) -> R<ParseContext> {
    let xpub = recover_c_char(cardano_xpub);
    let mfp = unsafe { core::slice::from_raw_parts(master_fingerprint, 4) };
    Ok(ParseContext::new(
        cardano_sign_request
            .get_utxos()
            .iter()
            .map(|v| {
                Ok(CardanoUtxo::new(
                    v.get_key_path()
                        .get_source_fingerprint()
                        .ok_or(RustCError::InvalidMasterFingerprint)?
                        .to_vec(),
                    v.get_address(),
                    convert_key_path(v.get_key_path())?,
                    v.get_amount()
                        .parse::<u64>()
                        .map_err(|e| RustCError::InvalidData(e.to_string()))?,
                    v.get_transaction_hash(),
                    v.get_index(),
                ))
            })
            .collect::<R<Vec<CardanoUtxo>>>()?,
        cardano_sign_request
            .get_cert_keys()
            .iter()
            .map(|v| {
                Ok(CardanoCertKey::new(
                    v.get_key_path()
                        .get_source_fingerprint()
                        .ok_or(RustCError::InvalidMasterFingerprint)?
                        .to_vec(),
                    v.get_key_hash(),
                    convert_key_path(v.get_key_path())?,
                ))
            })
            .collect::<R<Vec<CardanoCertKey>>>()?,
        xpub,
        mfp.to_vec(),
    ))
}

fn convert_key_path(key_path: CryptoKeyPath) -> R<DerivationPath> {
    match key_path.get_path() {
        Some(string) => {
            let path = format!("m/{}", string);
            DerivationPath::from_str(path.as_str()).map_err(|_e| RustCError::InvalidHDPath)
        }
        None => Err(RustCError::InvalidHDPath),
    }
}
