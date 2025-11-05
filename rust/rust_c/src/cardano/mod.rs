use alloc::{format, slice};
use alloc::{
    string::{String, ToString},
    vec,
};

use alloc::vec::Vec;
use app_cardano::address::derive_xpub_from_xpub;
use app_cardano::errors::CardanoError;
use app_cardano::governance;
use app_cardano::structs::{CardanoCertKey, CardanoUtxo, ParseContext};
use app_cardano::transaction::calc_icarus_master_key;
use bitcoin::bip32::DerivationPath;
use core::str::FromStr;
use cryptoxide::hashing::blake2b_224;
use cty::c_char;
use ed25519_bip32_core::XPrv;

use structs::DisplayCardanoSignTxHash;

use crate::common::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use crate::common::types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::common::{
    errors::{RustCError, R},
    ur::FRAGMENT_UNLIMITED_LENGTH,
};
use crate::{extract_array, extract_array_mut, extract_ptr_with_type};
use structs::{DisplayCardanoCatalyst, DisplayCardanoSignData, DisplayCardanoTx};
use ur_registry::cardano::cardano_sign_data_signature::CardanoSignDataSignature;
use ur_registry::cardano::cardano_sign_request::CardanoSignRequest;
use ur_registry::cardano::cardano_sign_tx_hash_request::CardanoSignTxHashRequest;
use ur_registry::cardano::cardano_signature::CardanoSignature;
use ur_registry::cardano::{
    cardano_catalyst_signature::CardanoCatalystSignature,
    cardano_sign_cip8_data_request::CardanoSignCip8DataRequest,
};
use ur_registry::cardano::{
    cardano_catalyst_voting_registration::CardanoCatalystVotingRegistrationRequest,
    cardano_sign_cip8_data_signature::CardanoSignCip8DataSignature,
};
use ur_registry::crypto_key_path::CryptoKeyPath;
use ur_registry::registry_types::{
    CARDANO_CATALYST_VOTING_REGISTRATION_SIGNATURE, CARDANO_SIGNATURE, CARDANO_SIGN_DATA_SIGNATURE,
};
use ur_registry::{
    cardano::cardano_sign_data_request::CardanoSignDataRequest,
    registry_types::CARDANO_SIGN_CIP8_DATA_SIGNATURE,
};
use zeroize::Zeroize;

pub mod cip8_cbor_data_ledger;

pub mod address;
pub mod structs;
use cip8_cbor_data_ledger::CardanoCip8SigStructureLedgerType;
#[no_mangle]
pub unsafe extern "C" fn cardano_catalyst_xpub(ptr: PtrUR) -> Ptr<SimpleResponse<c_char>> {
    let cardano_catalyst_request =
        extract_ptr_with_type!(ptr, CardanoCatalystVotingRegistrationRequest);
    let xpub = cardano_catalyst_request.get_stake_pub();
    SimpleResponse::success(convert_c_char(hex::encode(xpub))).simple_c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn cardano_check_catalyst(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
) -> PtrT<TransactionCheckResult> {
    let cardano_catalyst_request =
        extract_ptr_with_type!(ptr, CardanoCatalystVotingRegistrationRequest);
    let mfp = extract_array!(master_fingerprint, u8, 4);
    let ur_mfp = match cardano_catalyst_request
        .get_derivation_path()
        .get_source_fingerprint()
    {
        Some(fp) => fp,
        None => return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    };

    if let Ok(mfp) = mfp.try_into() as Result<[u8; 4], _> {
        if mfp != ur_mfp {
            return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
        }
    }

    TransactionCheckResult::new().c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn cardano_check_catalyst_path_type(
    ptr: PtrUR,
    cardano_xpub: PtrString,
) -> PtrT<TransactionCheckResult> {
    let cardano_catalyst_request =
        extract_ptr_with_type!(ptr, CardanoCatalystVotingRegistrationRequest);
    let cardano_xpub = recover_c_char(cardano_xpub);
    let xpub = hex::encode(cardano_catalyst_request.get_stake_pub());
    let derivation_path =
        get_cardano_derivation_path(cardano_catalyst_request.get_derivation_path());
    match derivation_path {
        Ok(derivation_path) => match derive_xpub_from_xpub(cardano_xpub, derivation_path) {
            Ok(_xpub) => {
                if _xpub == xpub {
                    TransactionCheckResult::new().c_ptr()
                } else {
                    TransactionCheckResult::from(RustCError::InvalidData(
                        "invalid xpub".to_string(),
                    ))
                    .c_ptr()
                }
            }
            Err(e) => TransactionCheckResult::from(e).c_ptr(),
        },
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_get_catalyst_root_index(
    ptr: PtrUR,
) -> Ptr<SimpleResponse<c_char>> {
    let cardano_catalyst_request =
        extract_ptr_with_type!(ptr, CardanoCatalystVotingRegistrationRequest);
    let derviation_path: CryptoKeyPath = cardano_catalyst_request.get_derivation_path();
    match derviation_path.get_components().get(2) {
        Some(_data) => match _data.get_index() {
            Some(index) => {
                SimpleResponse::success(convert_c_char(index.to_string())).simple_c_ptr()
            }
            None => SimpleResponse::from(CardanoError::InvalidTransaction(
                "invalid path index".to_string(),
            ))
            .simple_c_ptr(),
        },
        None => SimpleResponse::from(CardanoError::InvalidTransaction("invalid path".to_string()))
            .simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_get_sign_data_root_index(
    ptr: PtrUR,
) -> Ptr<SimpleResponse<c_char>> {
    let cardano_sign_data_reqeust = extract_ptr_with_type!(ptr, CardanoSignDataRequest);
    let derviation_path: CryptoKeyPath = cardano_sign_data_reqeust.get_derivation_path();
    match derviation_path.get_components().get(2) {
        Some(_data) => match _data.get_index() {
            Some(index) => {
                SimpleResponse::success(convert_c_char(index.to_string())).simple_c_ptr()
            }
            None => SimpleResponse::from(CardanoError::InvalidTransaction(
                "invalid path index".to_string(),
            ))
            .simple_c_ptr(),
        },
        None => SimpleResponse::from(CardanoError::InvalidTransaction("invalid path".to_string()))
            .simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_get_sign_cip8_data_root_index(
    ptr: PtrUR,
) -> Ptr<SimpleResponse<c_char>> {
    let cardano_sign_data_reqeust = extract_ptr_with_type!(ptr, CardanoSignCip8DataRequest);
    let derviation_path: CryptoKeyPath = cardano_sign_data_reqeust.get_derivation_path();
    match derviation_path.get_components().get(2) {
        Some(_data) => match _data.get_index() {
            Some(index) => {
                SimpleResponse::success(convert_c_char(index.to_string())).simple_c_ptr()
            }
            None => SimpleResponse::from(CardanoError::InvalidTransaction(
                "invalid path index".to_string(),
            ))
            .simple_c_ptr(),
        },
        None => SimpleResponse::from(CardanoError::InvalidTransaction("invalid path".to_string()))
            .simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_check_sign_data_path_type(
    ptr: PtrUR,
    cardano_xpub: PtrString,
) -> PtrT<TransactionCheckResult> {
    let cardano_sign_data_reqeust = extract_ptr_with_type!(ptr, CardanoSignDataRequest);
    let cardano_xpub = recover_c_char(cardano_xpub);
    let xpub = cardano_sign_data_reqeust.get_xpub();
    let xpub = hex::encode(xpub);
    let derivation_path =
        get_cardano_derivation_path(cardano_sign_data_reqeust.get_derivation_path());
    match derivation_path {
        Ok(derivation_path) => match derive_xpub_from_xpub(cardano_xpub, derivation_path) {
            Ok(_xpub) => {
                if _xpub == xpub {
                    TransactionCheckResult::new().c_ptr()
                } else {
                    TransactionCheckResult::from(RustCError::InvalidData(
                        "invalid xpub".to_string(),
                    ))
                    .c_ptr()
                }
            }
            Err(e) => TransactionCheckResult::from(e).c_ptr(),
        },
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_check_sign_data_is_sign_opcert(
    ptr: PtrUR,
) -> PtrT<TransactionCheckResult> {
    let cardano_sign_data_reqeust = extract_ptr_with_type!(ptr, CardanoSignDataRequest);
    let derviation_path: CryptoKeyPath = cardano_sign_data_reqeust.get_derivation_path();
    // if path first element is 1853 and path length is 4, that means it is a valid cardano cip1853 path
    if derviation_path.get_components().len() == 4
        && derviation_path.get_components()[0].get_index() == Some(1853)
    {
        TransactionCheckResult::new().c_ptr()
    } else {
        TransactionCheckResult::from(RustCError::InvalidData("not opcert".to_string())).c_ptr()
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_check_sign_data(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
) -> PtrT<TransactionCheckResult> {
    let cardano_sign_data_reqeust = extract_ptr_with_type!(ptr, CardanoSignDataRequest);
    let mfp = extract_array!(master_fingerprint, u8, 4);
    let ur_mfp = match cardano_sign_data_reqeust
        .get_derivation_path()
        .get_source_fingerprint()
    {
        Some(fp) => fp,
        None => return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    };

    if let Ok(mfp) = mfp.try_into() as Result<[u8; 4], _> {
        if mfp != ur_mfp {
            return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
        }
    }

    TransactionCheckResult::new().c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn cardano_check_sign_cip8_data(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
) -> PtrT<TransactionCheckResult> {
    let cardano_sign_cip8_data_reqeust = extract_ptr_with_type!(ptr, CardanoSignCip8DataRequest);
    let mfp = extract_array!(master_fingerprint, u8, 4);
    let ur_mfp = match cardano_sign_cip8_data_reqeust
        .get_derivation_path()
        .get_source_fingerprint()
    {
        Some(fp) => fp,
        None => return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    };

    if let Ok(mfp) = mfp.try_into() as Result<[u8; 4], _> {
        if mfp != ur_mfp {
            return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
        }
    }

    TransactionCheckResult::new().c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn cardano_check_sign_cip8_data_path_type(
    ptr: PtrUR,
    cardano_xpub: PtrString,
) -> PtrT<TransactionCheckResult> {
    let cardano_sign_cip8_data_reqeust = extract_ptr_with_type!(ptr, CardanoSignCip8DataRequest);
    let cardano_xpub = recover_c_char(cardano_xpub);
    let xpub = cardano_sign_cip8_data_reqeust.get_xpub();
    let xpub = hex::encode(xpub);
    let derivation_path =
        get_cardano_derivation_path(cardano_sign_cip8_data_reqeust.get_derivation_path());
    match derivation_path {
        Ok(derivation_path) => match derive_xpub_from_xpub(cardano_xpub, derivation_path) {
            Ok(_xpub) => {
                if _xpub == xpub {
                    TransactionCheckResult::new().c_ptr()
                } else {
                    TransactionCheckResult::from(RustCError::InvalidData(
                        "invalid xpub".to_string(),
                    ))
                    .c_ptr()
                }
            }
            Err(e) => TransactionCheckResult::from(e).c_ptr(),
        },
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_check_tx(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
) -> PtrT<TransactionCheckResult> {
    let cardano_sign_reqeust = extract_ptr_with_type!(ptr, CardanoSignRequest);
    let tx_hex = cardano_sign_reqeust.get_sign_data();
    let parse_context =
        prepare_parse_context(cardano_sign_reqeust, master_fingerprint, cardano_xpub);
    match parse_context {
        Ok(parse_context) => match app_cardano::transaction::check_tx(tx_hex, parse_context) {
            Ok(_) => TransactionCheckResult::new().c_ptr(),
            Err(e) => TransactionCheckResult::from(e).c_ptr(),
        },
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}
#[no_mangle]
pub unsafe extern "C" fn cardano_check_tx_hash(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
) -> PtrT<TransactionCheckResult> {
    let cardano_sign_tx_hash_reqeust = extract_ptr_with_type!(ptr, CardanoSignTxHashRequest);
    let expected_mfp = extract_array!(master_fingerprint, u8, 4);
    // check mfp
    let paths = cardano_sign_tx_hash_reqeust.get_paths();
    for path in paths {
        let mfp = path.get_source_fingerprint();
        if let Some(mfp) = mfp {
            if hex::encode(mfp) != hex::encode(expected_mfp) {
                return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
            }
        }
    }
    TransactionCheckResult::new().c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn cardano_parse_sign_tx_hash(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayCardanoSignTxHash>> {
    let sign_hash_request = extract_ptr_with_type!(ptr, CardanoSignTxHashRequest);
    let message = sign_hash_request.get_tx_hash();
    let crypto_key_paths = sign_hash_request.get_paths();
    let paths = crypto_key_paths
        .iter()
        .map(|v| v.get_path())
        .collect::<Option<Vec<String>>>()
        .unwrap_or_default();
    let address_list = sign_hash_request.get_address_list();
    let network = "Cardano".to_string();
    let result = DisplayCardanoSignTxHash::new(network, paths, message, address_list);
    TransactionParseResult::success(result.c_ptr()).c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn cardano_get_path(ptr: PtrUR) -> Ptr<SimpleResponse<c_char>> {
    let cardano_sign_reqeust = extract_ptr_with_type!(ptr, CardanoSignRequest);
    if let Some(_data) = cardano_sign_reqeust.get_cert_keys().first() {
        if let Some(_path) = _data.get_key_path().get_path() {
            if let Some(path) = parse_cardano_root_path(_path) {
                return SimpleResponse::success(convert_c_char(path)).simple_c_ptr();
            }
        }
    };
    match cardano_sign_reqeust.get_utxos().first() {
        Some(_data) => match _data.get_key_path().get_path() {
            Some(_path) => {
                if let Some(path) = parse_cardano_root_path(_path) {
                    return SimpleResponse::success(convert_c_char(path)).simple_c_ptr();
                }
                SimpleResponse::from(CardanoError::InvalidTransaction("invalid utxo".to_string()))
                    .simple_c_ptr()
            }
            None => {
                SimpleResponse::from(CardanoError::InvalidTransaction("invalid utxo".to_string()))
                    .simple_c_ptr()
            }
        },
        None => SimpleResponse::from(CardanoError::InvalidTransaction("invalid utxo".to_string()))
            .simple_c_ptr(),
    }
}

fn generate_master_key(
    entropy: &[u8],
    passphrase: &str,
    is_slip39: bool,
) -> Result<XPrv, CardanoError> {
    if is_slip39 {
        app_cardano::slip23::from_seed_slip23(entropy)
    } else {
        match calc_icarus_master_key(entropy, passphrase.as_bytes()) {
            Ok(v) => Ok(v),
            Err(e) => Err(e),
        }
    }
}

fn parse_cardano_root_path(path: String) -> Option<String> {
    let root_path = "1852'/1815'/";
    match path.strip_prefix(root_path) {
        Some(path) => {
            if let Some(index) = path.find('/') {
                let sub_path = &path[..index];
                Some(format!("{root_path}{sub_path}"))
            } else {
                None
            }
        }
        None => None,
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_parse_sign_data(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayCardanoSignData>> {
    let cardano_sign_data_reqeust = extract_ptr_with_type!(ptr, CardanoSignDataRequest);
    let sign_data = cardano_sign_data_reqeust.get_sign_data();
    let derviation_path = match cardano_sign_data_reqeust.get_derivation_path().get_path() {
        Some(path) => path,
        None => {
            return TransactionParseResult::from(CardanoError::InvalidTransaction(
                "Invalid derivation path".to_string(),
            ))
            .c_ptr()
        }
    };
    let xpub = cardano_sign_data_reqeust.get_xpub();
    let parsed_data =
        app_cardano::transaction::parse_sign_data(sign_data, derviation_path, hex::encode(xpub));
    match parsed_data {
        Ok(v) => TransactionParseResult::success(DisplayCardanoSignData::from(v).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_parse_sign_cip8_data(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayCardanoSignData>> {
    let cardano_sign_cip8_data_reqeust = extract_ptr_with_type!(ptr, CardanoSignCip8DataRequest);
    let sign_data = cardano_sign_cip8_data_reqeust.get_sign_data();
    let derviation_path = match cardano_sign_cip8_data_reqeust
        .get_derivation_path()
        .get_path()
    {
        Some(path) => path,
        None => {
            return TransactionParseResult::from(CardanoError::InvalidTransaction(
                "Invalid derivation path".to_string(),
            ))
            .c_ptr()
        }
    };
    let xpub = cardano_sign_cip8_data_reqeust.get_xpub();
    let parsed_data = app_cardano::transaction::parse_sign_cip8_data(
        sign_data,
        derviation_path,
        hex::encode(xpub),
        cardano_sign_cip8_data_reqeust.get_hash_payload(),
    );
    match parsed_data {
        Ok(v) => TransactionParseResult::success(DisplayCardanoSignData::from(v).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_parse_catalyst(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayCardanoCatalyst>> {
    let cardano_catalyst_request =
        extract_ptr_with_type!(ptr, CardanoCatalystVotingRegistrationRequest);
    let res = DisplayCardanoCatalyst::from(cardano_catalyst_request.clone()).c_ptr();

    TransactionParseResult::success(res).c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn cardano_parse_tx(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
) -> PtrT<TransactionParseResult<DisplayCardanoTx>> {
    let cardano_sign_reqeust = extract_ptr_with_type!(ptr, CardanoSignRequest);
    let tx_hex = cardano_sign_reqeust.get_sign_data();
    let parse_context =
        prepare_parse_context(cardano_sign_reqeust, master_fingerprint, cardano_xpub);
    match parse_context {
        Ok(parse_context) => match app_cardano::transaction::parse_tx(tx_hex, parse_context) {
            Ok(v) => TransactionParseResult::success(DisplayCardanoTx::from(v).c_ptr()).c_ptr(),
            Err(e) => TransactionParseResult::from(e).c_ptr(),
        },
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_sign_catalyst_with_ledger_bitbox02(
    ptr: PtrUR,
    mnemonic: PtrString,
    passphrase: PtrString,
) -> PtrT<UREncodeResult> {
    let mnemonic = recover_c_char(mnemonic);
    let passphrase = recover_c_char(passphrase);
    let master_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_ledger_bitbox02_master_key_by_mnemonic(
            passphrase.as_bytes(),
            mnemonic,
        );

    match master_key {
        Ok(master_key) => cardano_sign_catalyst_by_icarus(ptr, master_key),
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_sign_catalyst(
    ptr: PtrUR,
    entropy: PtrBytes,
    entropy_len: u32,
    passphrase: PtrString,
    is_slip39: bool,
) -> PtrT<UREncodeResult> {
    let mut entropy: &mut [u8] = extract_array_mut!(entropy, u8, entropy_len as usize);
    let passphrase = recover_c_char(passphrase);
    let master_key = match generate_master_key(entropy, &passphrase, is_slip39) {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    entropy.zeroize();
    cardano_sign_catalyst_by_icarus(ptr, master_key)
}

unsafe fn cardano_sign_catalyst_by_icarus(
    ptr: PtrUR,
    icarus_master_key: XPrv,
) -> PtrT<UREncodeResult> {
    let cardano_catalyst_request =
        extract_ptr_with_type!(ptr, CardanoCatalystVotingRegistrationRequest);

    let path = match cardano_catalyst_request.get_derivation_path().get_path() {
        Some(p) => p,
        None => {
            return UREncodeResult::from(CardanoError::InvalidTransaction(
                "Invalid derivation path".to_string(),
            ))
            .c_ptr()
        }
    };

    governance::sign(
        &path,
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
                        FRAGMENT_MAX_LENGTH_DEFAULT,
                    )
                    .c_ptr()
                },
            )
        },
    )
}

#[no_mangle]
pub unsafe extern "C" fn cardano_sign_sign_data_with_ledger_bitbox02(
    ptr: PtrUR,
    mnemonic: PtrString,
    passphrase: PtrString,
) -> PtrT<UREncodeResult> {
    let mnemonic = recover_c_char(mnemonic);
    let passphrase = recover_c_char(passphrase);
    let master_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_ledger_bitbox02_master_key_by_mnemonic(
            passphrase.as_bytes(),
            mnemonic,
        );

    match master_key {
        Ok(master_key) => cardano_sign_sign_data_by_icarus(ptr, master_key),
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_sign_sign_cip8_data_with_ledger_bitbox02(
    ptr: PtrUR,
    mnemonic: PtrString,
    passphrase: PtrString,
) -> PtrT<UREncodeResult> {
    let mnemonic = recover_c_char(mnemonic);
    let passphrase = recover_c_char(passphrase);
    let master_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_ledger_bitbox02_master_key_by_mnemonic(
            passphrase.as_bytes(),
            mnemonic,
        );

    match master_key {
        Ok(master_key) => cardano_sign_sign_cip8_data_by_icarus(ptr, master_key),
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_sign_sign_data(
    ptr: PtrUR,
    entropy: PtrBytes,
    entropy_len: u32,
    passphrase: PtrString,
    is_slip39: bool,
) -> PtrT<UREncodeResult> {
    let mut entropy = extract_array_mut!(entropy, u8, entropy_len as usize);
    let passphrase = recover_c_char(passphrase);
    let master_key = match generate_master_key(entropy, &passphrase, is_slip39) {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    entropy.zeroize();

    cardano_sign_sign_data_by_icarus(ptr, master_key)
}

unsafe fn cardano_sign_sign_data_by_icarus(
    ptr: PtrUR,
    icarus_master_key: XPrv,
) -> PtrT<UREncodeResult> {
    let cardano_sign_data_reqeust = extract_ptr_with_type!(ptr, CardanoSignDataRequest);
    let sign_data = cardano_sign_data_reqeust.get_sign_data();

    let path = match cardano_sign_data_reqeust.get_derivation_path().get_path() {
        Some(p) => p,
        None => {
            return UREncodeResult::from(CardanoError::InvalidTransaction(
                "Invalid derivation path".to_string(),
            ))
            .c_ptr()
        }
    };

    let result = app_cardano::transaction::sign_data(
        &path,
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
                        FRAGMENT_MAX_LENGTH_DEFAULT,
                    )
                    .c_ptr()
                },
            )
        },
    );

    result
}

#[no_mangle]
pub unsafe extern "C" fn cardano_sign_sign_cip8_data(
    ptr: PtrUR,
    entropy: PtrBytes,
    entropy_len: u32,
    passphrase: PtrString,
    is_slip39: bool,
) -> PtrT<UREncodeResult> {
    let mut entropy = extract_array_mut!(entropy, u8, entropy_len as usize);
    let passphrase = recover_c_char(passphrase);
    let master_key = match generate_master_key(entropy, &passphrase, is_slip39) {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };

    cardano_sign_sign_cip8_data_by_icarus(ptr, master_key)
}

#[no_mangle]
pub unsafe extern "C" fn cardano_sign_tx_with_ledger_bitbox02(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
    mnemonic: PtrString,
    passphrase: PtrString,
    enable_blind_sign: bool,
) -> PtrT<UREncodeResult> {
    let mnemonic = recover_c_char(mnemonic);
    let passphrase = recover_c_char(passphrase);
    let master_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_ledger_bitbox02_master_key_by_mnemonic(
            passphrase.as_bytes(),
            mnemonic,
        );
    match master_key {
        Ok(master_key) => {
            if enable_blind_sign {
                cardano_sign_tx_hash_by_icarus(ptr, master_key)
            } else {
                cardano_sign_tx_by_icarus(ptr, master_fingerprint, cardano_xpub, master_key)
            }
        }
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_sign_tx_with_ledger_bitbox02_unlimited(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
    mnemonic: PtrString,
    passphrase: PtrString,
) -> PtrT<UREncodeResult> {
    let mnemonic = recover_c_char(mnemonic);
    let passphrase = recover_c_char(passphrase);
    let master_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_ledger_bitbox02_master_key_by_mnemonic(
            passphrase.as_bytes(),
            mnemonic,
        );

    match master_key {
        Ok(master_key) => {
            cardano_sign_tx_by_icarus_unlimited(ptr, master_fingerprint, cardano_xpub, master_key)
        }
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_sign_tx(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
    entropy: PtrBytes,
    entropy_len: u32,
    passphrase: PtrString,
    enable_blind_sign: bool,
    is_slip39: bool,
) -> PtrT<UREncodeResult> {
    let mut entropy = extract_array_mut!(entropy, u8, entropy_len as usize);
    let passphrase = recover_c_char(passphrase);
    let master_key = match generate_master_key(entropy, &passphrase, is_slip39) {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    entropy.zeroize();
    if enable_blind_sign {
        cardano_sign_tx_hash_by_icarus(ptr, master_key)
    } else {
        cardano_sign_tx_by_icarus(ptr, master_fingerprint, cardano_xpub, master_key)
    }
}

unsafe fn cardano_sign_tx_hash_by_icarus(
    ptr: PtrUR,
    icarus_master_key: XPrv,
) -> PtrT<UREncodeResult> {
    let cardano_sign_tx_hash_request = extract_ptr_with_type!(ptr, CardanoSignTxHashRequest);
    let tx_hash = cardano_sign_tx_hash_request.get_tx_hash();
    let paths = cardano_sign_tx_hash_request.get_paths();
    let sign_result = app_cardano::transaction::sign_tx_hash(&tx_hash, &paths, icarus_master_key);
    match sign_result {
        Ok(v) => {
            UREncodeResult::encode(v, CARDANO_SIGNATURE.get_type(), FRAGMENT_MAX_LENGTH_DEFAULT)
                .c_ptr()
        }
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn cardano_sign_tx_unlimited(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
    entropy: PtrBytes,
    entropy_len: u32,
    passphrase: PtrString,
    is_slip39: bool,
) -> PtrT<UREncodeResult> {
    let mut entropy = extract_array_mut!(entropy, u8, entropy_len as usize);
    let passphrase = recover_c_char(passphrase);
    let master_key = match generate_master_key(entropy, &passphrase, is_slip39) {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    entropy.zeroize();
    cardano_sign_tx_by_icarus_unlimited(ptr, master_fingerprint, cardano_xpub, master_key)
}

#[no_mangle]
pub unsafe extern "C" fn cardano_get_pubkey_by_slip23(
    entropy: PtrBytes,
    entropy_len: u32,
    path: PtrString,
) -> *mut SimpleResponse<c_char> {
    if entropy_len != 16 && entropy_len != 32 {
        return SimpleResponse::from(RustCError::InvalidData(
            "Invalid entropy length".to_string(),
        ))
        .simple_c_ptr();
    }
    let mut entropy = extract_array_mut!(entropy, u8, entropy_len as usize);
    let path = recover_c_char(path).to_lowercase();
    let xpub = app_cardano::slip23::from_seed_slip23_path(entropy, path.as_str());
    entropy.zeroize();
    match xpub {
        Ok(xpub) => {
            SimpleResponse::success(convert_c_char(xpub.public().to_string())).simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

unsafe fn cardano_sign_tx_by_icarus(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
    icarus_master_key: XPrv,
) -> PtrT<UREncodeResult> {
    cardano_sign_tx_by_icarus_dynamic(
        ptr,
        master_fingerprint,
        cardano_xpub,
        icarus_master_key,
        FRAGMENT_MAX_LENGTH_DEFAULT,
    )
}

unsafe fn cardano_sign_tx_by_icarus_unlimited(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
    icarus_master_key: XPrv,
) -> PtrT<UREncodeResult> {
    cardano_sign_tx_by_icarus_dynamic(
        ptr,
        master_fingerprint,
        cardano_xpub,
        icarus_master_key,
        FRAGMENT_UNLIMITED_LENGTH,
    )
}

unsafe fn cardano_sign_tx_by_icarus_dynamic(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
    icarus_master_key: XPrv,
    fragment_length: usize,
) -> PtrT<UREncodeResult> {
    let cardano_sign_reqeust = extract_ptr_with_type!(ptr, CardanoSignRequest);
    let tx_hex = cardano_sign_reqeust.get_sign_data();
    let parse_context =
        prepare_parse_context(cardano_sign_reqeust, master_fingerprint, cardano_xpub);
    match parse_context {
        Ok(parse_context) => {
            let sign_result =
                app_cardano::transaction::sign_tx(tx_hex, parse_context, icarus_master_key).map(
                    |v| CardanoSignature::new(cardano_sign_reqeust.get_request_id(), v).try_into(),
                );
            match sign_result {
                Ok(d) => match d {
                    Ok(data) => {
                        UREncodeResult::encode(data, CARDANO_SIGNATURE.get_type(), fragment_length)
                            .c_ptr()
                    }
                    Err(e) => UREncodeResult::from(e).c_ptr(),
                },
                Err(e) => UREncodeResult::from(e).c_ptr(),
            }
        }
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

unsafe fn cardano_sign_sign_cip8_data_by_icarus(
    ptr: PtrUR,
    icarus_master_key: XPrv,
) -> PtrT<UREncodeResult> {
    let cardano_sign_data_reqeust = extract_ptr_with_type!(ptr, CardanoSignCip8DataRequest);
    let mut sign_data = cardano_sign_data_reqeust.get_sign_data();
    // Signature1
    // A20127676164647265737358390014C16D7F43243BD81478E68B9DB53A8528FD4FB1078D58D54A7F11241D227AEFA4B773149170885AADBA30AAB3127CC611DDBC4999DEF61C40581C42D1854B7D69E3B57C64FCC7B4F64171B47DFF43FBA6AC0499FF437F
    // []
    // 42D1854B7D69E3B57C64FCC7B4F64171B47DFF43FBA6AC0499FF437F
    // construct cardano cip8 data using ledger style
    if cardano_sign_data_reqeust.get_hash_payload() {
        sign_data = blake2b_224(&sign_data).to_vec();
    }
    let address_field = {
        let address_type = cardano_sign_data_reqeust.get_address_type();
        if address_type.as_str() == "ADDRESS" {
            let bech = match cardano_sign_data_reqeust.get_address_bench32() {
                Some(s) => s,
                None => {
                    return UREncodeResult::from(RustCError::InvalidData(
                        "missing address".to_string(),
                    ))
                    .c_ptr()
                }
            };
            match bitcoin::bech32::decode(bech.as_str()) {
                Ok((_hrp, data)) => data,
                Err(_) => {
                    return UREncodeResult::from(RustCError::InvalidData(
                        "invalid bech32".to_string(),
                    ))
                    .c_ptr()
                }
            }
        } else {
            let public_key = cardano_sign_data_reqeust.get_xpub();
            blake2b_224(&public_key).to_vec()
        }
    };

    let cip8_data = CardanoCip8SigStructureLedgerType {
        address_field: address_field.clone(),
        payload: sign_data,
    };
    let cip8_cbor_data_ledger_type = match minicbor::to_vec(&cip8_data) {
        Ok(v) => hex::encode(v).to_uppercase(),
        Err(e) => return UREncodeResult::from(RustCError::InvalidData(e.to_string())).c_ptr(),
    };
    let path = match cardano_sign_data_reqeust.get_derivation_path().get_path() {
        Some(p) => p,
        None => {
            return UREncodeResult::from(CardanoError::InvalidTransaction(
                "Invalid derivation path".to_string(),
            ))
            .c_ptr()
        }
    };
    let result = app_cardano::transaction::sign_data(
        &path,
        cip8_cbor_data_ledger_type.as_str(),
        icarus_master_key,
    )
    .map(|v| {
        CardanoSignCip8DataSignature::new(
            cardano_sign_data_reqeust.get_request_id(),
            v.get_signature(),
            v.get_pub_key(),
            address_field.clone(),
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
                        CARDANO_SIGN_CIP8_DATA_SIGNATURE.get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT,
                    )
                    .c_ptr()
                },
            )
        },
    );

    result
}

unsafe fn prepare_parse_context(
    cardano_sign_request: &CardanoSignRequest,
    master_fingerprint: PtrBytes,
    cardano_xpub: PtrString,
) -> R<ParseContext> {
    let xpub = if cardano_xpub.is_null() {
        None
    } else {
        Some(recover_c_char(cardano_xpub))
    };
    let mfp = extract_array!(master_fingerprint, u8, 4);
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
            let path = format!("m/{string}");
            DerivationPath::from_str(path.as_str()).map_err(|_e| RustCError::InvalidHDPath)
        }
        None => Err(RustCError::InvalidHDPath),
    }
}

fn get_cardano_derivation_path(path: CryptoKeyPath) -> R<CryptoKeyPath> {
    let components = path.get_components();
    let mut new_components = Vec::new();
    for item in components.iter().skip(3) {
        new_components.push(*item);
    }
    Ok(CryptoKeyPath::new(
        new_components,
        path.get_source_fingerprint(),
        path.get_depth(),
    ))
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::vec;
    use app_cardano::address::AddressType;
    use bitcoin::bech32::decode;
    use keystore::algorithms::ed25519::bip32_ed25519::derive_extended_privkey_by_xprv;
    use ur_registry::crypto_key_path::PathComponent;

    #[test]
    fn test_get_cardano_derivation_path() {
        let path = CryptoKeyPath::new(
            vec![
                PathComponent::new(Some(1852), false).unwrap(),
                PathComponent::new(Some(1815), false).unwrap(),
                PathComponent::new(Some(0), false).unwrap(),
                PathComponent::new(Some(2), false).unwrap(),
                PathComponent::new(Some(0), false).unwrap(),
            ],
            Some([0, 0, 0, 0]),
            None,
        );
        let result = get_cardano_derivation_path(path);
        assert!(result.is_ok());

        assert_eq!(result.unwrap().get_path().unwrap(), "2/0");
    }

    #[test]
    fn test_sign_data() {
        let payload = hex::encode("hello world");
        let expected_message_hash = "42d1854b7d69e3b57c64fcc7b4f64171b47dff43fba6ac0499ff437f";
        let message_hash = hex::encode(cryptoxide::hashing::blake2b_224(
            hex::decode(&payload).unwrap().as_slice(),
        ));
        assert_eq!(expected_message_hash, message_hash);
        let master_key_expected = "402b03cd9c8bed9ba9f9bd6cd9c315ce9fcc59c7c25d37c85a36096617e69d418e35cb4a3b737afd007f0688618f21a8831643c0e6c77fc33c06026d2a0fc93832596435e70647d7d98ef102a32ea40319ca8fb6c851d7346d3bd8f9d1492658";
        let mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
        let passphrase = "";
        let master_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_ledger_bitbox02_master_key_by_mnemonic(
            passphrase.as_bytes(),
            mnemonic.to_string(),
        ).unwrap();
        assert_eq!(master_key_expected, hex::encode(master_key.as_ref()));
        let bip32_signing_key =
            keystore::algorithms::ed25519::bip32_ed25519::derive_extended_privkey_by_icarus_master_key(
                &master_key.as_ref(),
                &"m/1852'/1815'/0'".to_string(),
            )
            .unwrap();
        // assert_eq!(
        //     hex::encode(bip32_signing_key.public().public_key().to_vec()),
        //     "cd2b047d1a803eee059769cffb3dfd0a4b9327e55bc78aa962d9bd4f720db0b2"
        // );
        // assert_eq!(
        //     hex::encode(bip32_signing_key.chain_code()),
        //     "914ba07fb381f23c5c09bce26587bdf359aab7ea8f4192adbf93a38fd893ccea"
        // );
        // 5840cd2b047d1a803eee059769cffb3dfd0a4b9327e55bc78aa962d9bd4f720db0b2914ba07fb381f23c5c09bce26587bdf359aab7ea8f4192adbf93a38fd893ccea
        //    cd2b047d1a803eee059769cffb3dfd0a4b9327e55bc78aa962d9bd4f720db0b2914ba07fb381f23c5c09bce26587bdf359aab7ea8f4192adbf93a38fd893ccea
        let xpub = bip32_signing_key.public();
        let real_testnet_xpub = "0d94fa4489745249e9cd999c907f2692e0e5c7ac868a960312ed5d480c59f2dc231adc1ee85703f714abe70c6d95f027e76ee947f361cbb72a155ac8cad6d23f".to_string();
        assert_eq!(real_testnet_xpub, hex::encode(xpub.as_ref()));
        let address =
            app_cardano::address::derive_address(real_testnet_xpub, 0, 0, 0, AddressType::Base, 0)
                .unwrap();
        assert_eq!("addr_test1qq2vzmtlgvjrhkq50rngh8d482zj3l20kyrc6kx4ffl3zfqayfawlf9hwv2fzuygt2km5v92kvf8e3s3mk7ynxw77cwq2glhm4", address);

        let private_key =
            derive_extended_privkey_by_xprv(&bip32_signing_key, &"0/0".to_string()).unwrap();
        assert_eq!("00e931ab7c17c922c5e905d46991cb590818f49eb39922a15018fc2124e69d41b3c7dfec8d2c21667419404ce6fe0fe6bc9e8e2b361487a59313bb9a0b2322cd", hex::encode(private_key.extended_secret_key_bytes()));

        let address_field = decode(address.as_str()).unwrap().1;
        assert_eq!(
            "0014c16d7f43243bd81478e68b9db53a8528fd4fb1078d58d54a7f11241d227aefa4b773149170885aadba30aab3127cc611ddbc4999def61c",
            hex::encode(address_field.as_slice())
        );

        // let result = app_cardano::transaction::sign_data(
        //     &"m/1852'/1815'/0'/0/0".to_string(),
        //     &message_hash,
        //     master_key,
        // )
        // .unwrap();
        // let signature = hex::encode(result.get_signature());
        // assert_eq!(signature,"56ebf5bbea63aafbf1440cd63c5fbcbe3de799de401d48165a366e10f36c17b490c261ea8a00cf464cf7140732369cc4e333eb6714cabe625abddac1cd9dd20b");
    }

    #[test]
    fn test_sign_op_cert() {
        // op_cert_hash = Buffer.concat([dkesPublicKeyHex,issueCounter,kesPeriod)
        // op_cert_hash length must be 48
        let message_hash = "f70601c4de155e67797e057c07fb768b5590b2241b05ec30235a85b71e2ae858000000000000000100000000000000fb";
        let master_key_expected = "402b03cd9c8bed9ba9f9bd6cd9c315ce9fcc59c7c25d37c85a36096617e69d418e35cb4a3b737afd007f0688618f21a8831643c0e6c77fc33c06026d2a0fc93832596435e70647d7d98ef102a32ea40319ca8fb6c851d7346d3bd8f9d1492658";
        let mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
        let passphrase = "";
        let master_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_ledger_bitbox02_master_key_by_mnemonic(
            passphrase.as_bytes(),
            mnemonic.to_string(),
        ).unwrap();
        assert_eq!(master_key_expected, hex::encode(master_key.as_ref()));

        let bip32_signing_key =
        keystore::algorithms::ed25519::bip32_ed25519::derive_extended_privkey_by_icarus_master_key(
            &master_key.as_ref(),
            &"m/1853'/1815'/0'".to_string(),
        )
        .unwrap();
        let xpub = bip32_signing_key.public();
        assert_eq!("", hex::encode(xpub.as_ref()));

        let result = app_cardano::transaction::sign_data(
            &"m/1853'/1815'/0'/0'".to_string(),
            &message_hash,
            master_key,
        )
        .unwrap();
        let signature = hex::encode(result.get_signature());
        assert_eq!(signature,"b44fcc4505aee4c93a716014ec709d17b28e0c95637384b78d2f8a4cebb92d1e01b54ce952e11771bbeaceda0eaf7a660e5c416f357bdec94e4ce2977997d204")
    }
}
