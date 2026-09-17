use alloc::format;
use alloc::string::ToString;
use alloc::vec::Vec;
use alloc::{boxed::Box, string::String};
use app_cardano::structs::{
    CardanoCertificate, CardanoFrom, CardanoTo, CardanoWithdrawal, ParsedCardanoSignCip8Data,
    ParsedCardanoSignData, ParsedCardanoTx, VotingProcedure, VotingProposal,
};
use core::ptr::null_mut;
use hex;
use itertools::Itertools;

use crate::common::ffi::VecFFI;
use crate::common::free::{free_ptr_string, Free};
use crate::common::structs::TransactionParseResult;
use crate::common::types::{Ptr, PtrString, PtrT};
use crate::common::utils::convert_c_char;
use crate::{free_str_ptr, free_vec, impl_c_ptr, impl_c_ptrs, make_free_method};
use app_sui::Intent;
use sui_transaction_types_core::{
    Argument, CallArg, Command, ProgrammableMoveCall, TransactionData, TransactionKind,
};

const IOTA_SYSTEM_PACKAGE_HEX: &str =
    "0000000000000000000000000000000000000000000000000000000000000003";

fn format_iota_amount(value: String) -> Option<String> {
    let value = value.parse::<u64>().ok()?;
    let whole = value / 1_000_000_000;
    let fraction = value % 1_000_000_000;
    if fraction == 0 {
        Some(format!("{whole} IOTA"))
    } else {
        let fraction = format!("{fraction:09}");
        Some(format!("{whole}.{} IOTA", fraction.trim_end_matches('0')))
    }
}

#[repr(C)]
pub struct DisplayIotaSignData {
    pub payload: PtrString,
    pub derivation_path: PtrString,
    pub message_hash: PtrString,
    pub xpub: PtrString,
}

#[repr(C)]
pub struct DisplayIotaIntentData {
    amount: PtrString,
    max_gas_fee: PtrString,
    gas_sponsored: bool,
    sender: PtrString,
    recipient: PtrString,
    details: PtrString,
    transaction_type: PtrString,
    method: PtrString,
    message: PtrString,
}

impl_c_ptr!(DisplayIotaIntentData);

impl DisplayIotaIntentData {
    pub fn personal_message(message: &[u8], address: String) -> Self {
        Self {
            amount: null_mut(),
            max_gas_fee: null_mut(),
            gas_sponsored: false,
            sender: convert_c_char(address),
            recipient: null_mut(),
            details: null_mut(),
            transaction_type: convert_c_char("Message".to_string()),
            method: null_mut(),
            message: null_mut(),
        }
        .with_personal_message_bytes(message)
    }

    pub fn with_address(mut self, address: String) -> Self {
        self.sender = convert_c_char(address);
        self
    }

    pub fn with_personal_message_bytes(mut self, message: &[u8]) -> Self {
        let display_message = core::str::from_utf8(message).ok().filter(|_| {
            message
                .iter()
                .all(|byte| byte.is_ascii_graphic() || *byte == b' ')
        });

        unsafe {
            free_ptr_string(self.message);
            free_ptr_string(self.details);
        }
        self.details = null_mut();
        if let Some(message) = display_message {
            self.message = convert_c_char(message.to_string());
        } else {
            self.message = null_mut();
            self.details = convert_c_char(message.chunks(8).map(hex::encode).join(" "));
        }
        self
    }
}

impl From<Intent> for DisplayIotaIntentData {
    fn from(value: Intent) -> Self {
        match value {
            Intent::TransactionData(transaction_data) => {
                let TransactionData::V1(data) = transaction_data.value;
                let details = serde_json::to_string(&data)
                    .unwrap_or_else(|_| "Failed to serialize transaction data".to_string());

                let TransactionKind::ProgrammableTransaction(kind_data) = data.kind else {
                    unreachable!("Only ProgrammableTransaction is supported")
                };

                let (method, amount, recipient) =
                    extract_transaction_params(&kind_data.inputs, &kind_data.commands);
                let max_gas_fee = format_iota_amount(data.gas_data.budget.to_string())
                    .map(convert_c_char)
                    .unwrap_or(null_mut());
                let gas_sponsored = data.gas_data.owner != data.sender;

                Self {
                    amount,
                    max_gas_fee,
                    gas_sponsored,
                    recipient,
                    sender: convert_c_char(format!("0x{}", hex::encode(data.sender.as_slice()))),
                    details: convert_c_char(details),
                    transaction_type: convert_c_char("Programmable Transaction".to_string()),
                    method,
                    message: null_mut(),
                }
            }
            Intent::PersonalMessage(personal_message) => Self {
                amount: null_mut(),
                max_gas_fee: null_mut(),
                gas_sponsored: false,
                recipient: null_mut(),
                sender: null_mut(),
                details: null_mut(),
                transaction_type: convert_c_char("Message".to_string()),
                method: null_mut(),
                message: convert_c_char(personal_message.value.message),
            },
            _ => todo!("Other Intent types not implemented"),
        }
    }
}

fn extract_transaction_params(
    inputs: &Vec<CallArg>,
    commands: &Vec<Command>,
) -> (PtrString, PtrString, PtrString) {
    let pure_at = |arg: &Argument, expect_len: usize| -> Option<&Vec<u8>> {
        let Argument::Input(idx) = arg else {
            return None;
        };
        match inputs.get(*idx as usize) {
            Some(CallArg::Pure(bytes)) if bytes.len() == expect_len => Some(bytes),
            _ => None,
        }
    };
    let is_split_result = |arg: &Argument| {
        matches!(arg, Argument::Result(0)) || matches!(arg, Argument::NestedResult(0, 0))
    };
    let is_system_call = |call: &ProgrammableMoveCall, function: &str| {
        hex::encode(call.package.as_slice()) == IOTA_SYSTEM_PACKAGE_HEX
            && call.module.to_string() == "iota_system"
            && call.function.to_string() == function
    };

    if let [Command::SplitCoins(Argument::GasCoin, split_amounts), Command::TransferObjects(objects, target)] =
        commands.as_slice()
    {
        if let ([amount_arg], [sole_output]) = (split_amounts.as_slice(), objects.as_slice()) {
            if is_split_result(sole_output) {
                if let (Some(amount_bytes), Some(recipient_bytes)) =
                    (pure_at(amount_arg, 8), pure_at(target, 32))
                {
                    let value =
                        u64::from_le_bytes(amount_bytes.as_slice().try_into().unwrap_or([0; 8]));
                    let amount = format_iota_amount(value.to_string())
                        .map(convert_c_char)
                        .unwrap_or(null_mut());
                    let recipient = convert_c_char(format!("0x{}", hex::encode(recipient_bytes)));
                    return (convert_c_char("Transfer".to_string()), amount, recipient);
                }
            }
        }
    }

    if let [Command::TransferObjects(objects, target)] = commands.as_slice() {
        if matches!(objects.as_slice(), [Argument::GasCoin]) {
            if let Some(recipient_bytes) = pure_at(target, 32) {
                let recipient = convert_c_char(format!("0x{}", hex::encode(recipient_bytes)));
                return (
                    convert_c_char("Transfer".to_string()),
                    convert_c_char("max".to_string()),
                    recipient,
                );
            }
        }
    }

    if let [Command::SplitCoins(Argument::GasCoin, split_amounts), Command::MoveCall(call)] =
        commands.as_slice()
    {
        if is_system_call(call, "request_add_stake") {
            if let ([amount_arg], [_state, coin_arg, validator_arg]) =
                (split_amounts.as_slice(), call.arguments.as_slice())
            {
                if is_split_result(coin_arg) {
                    if let (Some(amount_bytes), Some(validator_bytes)) =
                        (pure_at(amount_arg, 8), pure_at(validator_arg, 32))
                    {
                        let value = u64::from_le_bytes(
                            amount_bytes.as_slice().try_into().unwrap_or([0; 8]),
                        );
                        let amount = format_iota_amount(value.to_string())
                            .map(convert_c_char)
                            .unwrap_or(null_mut());
                        let validator =
                            convert_c_char(format!("0x{}", hex::encode(validator_bytes)));
                        return (convert_c_char("Stake".to_string()), amount, validator);
                    }
                }
            }
        }
    }

    if let [Command::MakeMoveVec(_, _), Command::MoveCall(call)] = commands.as_slice() {
        if is_system_call(call, "request_add_stake_mul_coin") {
            if let [_state, coins_arg, amount_arg, validator_arg] = call.arguments.as_slice() {
                if is_split_result(coins_arg) {
                    if let Some(validator_bytes) = pure_at(validator_arg, 32) {
                        let amount: Option<PtrString> = if let Some(bytes) = pure_at(amount_arg, 9)
                        {
                            match bytes.first() {
                                Some(1) => bytes
                                    .get(1..9)
                                    .and_then(|b| b.try_into().ok())
                                    .map(u64::from_le_bytes)
                                    .and_then(|value| format_iota_amount(value.to_string()))
                                    .map(convert_c_char),
                                _ => None,
                            }
                        } else {
                            None
                        };
                        if let Some(amount) = amount {
                            let validator =
                                convert_c_char(format!("0x{}", hex::encode(validator_bytes)));
                            return (convert_c_char("Stake".to_string()), amount, validator);
                        }
                    }
                }
            }
        }
    }

    (null_mut(), null_mut(), null_mut())
}

#[repr(C)]
pub struct DisplayIotaSignMessageHash {
    pub network: PtrString,
    pub path: PtrString,
    pub from_address: PtrString,
    pub message: PtrString,
}

impl DisplayIotaSignMessageHash {
    pub fn new(network: String, path: String, message: String, from_address: String) -> Self {
        Self {
            network: convert_c_char(network),
            path: convert_c_char(path),
            message: convert_c_char(message),
            from_address: convert_c_char(from_address),
        }
    }
}

impl_c_ptr!(DisplayIotaSignMessageHash);

impl Free for DisplayIotaSignMessageHash {
    unsafe fn free(&self) {
        free_str_ptr!(self.network);
        free_str_ptr!(self.path);
        free_str_ptr!(self.message);
        free_str_ptr!(self.from_address);
    }
}

make_free_method!(TransactionParseResult<DisplayIotaSignMessageHash>);

impl Free for DisplayIotaIntentData {
    unsafe fn free(&self) {
        free_str_ptr!(self.max_gas_fee);
        free_str_ptr!(self.sender);
        free_str_ptr!(self.recipient);
        free_str_ptr!(self.details);
        free_str_ptr!(self.transaction_type);
        free_str_ptr!(self.method);
        free_str_ptr!(self.amount);
        free_str_ptr!(self.message);
    }
}

make_free_method!(TransactionParseResult<DisplayIotaIntentData>);

#[cfg(test)]
mod tests {
    use super::*;
    use crate::common::utils::recover_c_char;
    use alloc::vec;
    use app_sui::types::{
        intent::{Intent as SuiIntent, IntentMessage, IntentScope},
        msg::PersonalMessageUtf8,
    };
    use sui_transaction_types_core::{
        ObjectArg, ObjectDigest, SequenceNumber, SharedObjectMutability, SuiAddress,
    };

    struct RequestFixture {
        request: ur_registry::iota::iota_sign_request::IotaSignRequest,
        keys: Vec<crate::common::structs::ExtendedPublicKey>,
        address: String,
    }

    impl RequestFixture {
        fn new(bytes: Vec<u8>) -> Self {
            use ur_registry::crypto_key_path::CryptoKeyPath;
            let path = "m/44'/4218'/0'/0'/0'".to_string();
            let pubkey = hex::encode(crate::sui::get_public_key(&[1; 32], &path).unwrap());
            let address = app_iota::address::get_address_from_pubkey(pubkey.clone()).unwrap();
            Self {
                request: ur_registry::iota::iota_sign_request::IotaSignRequest::new(
                    None,
                    bytes,
                    vec![CryptoKeyPath::from_path(path.clone(), Some([1, 2, 3, 4])).unwrap()],
                    Some(vec![hex::decode(&address[2..]).unwrap()]),
                    None,
                ),
                keys: vec![crate::common::structs::ExtendedPublicKey {
                    path: convert_c_char(path),
                    xpub: convert_c_char(pubkey),
                }],
                address,
            }
        }

        unsafe fn parse(&self) -> Result<DisplayIotaIntentData, app_sui::errors::SuiError> {
            super::super::parse_iota_request(&self.request, &self.keys)
        }
    }

    impl Drop for RequestFixture {
        fn drop(&mut self) {
            for key in &self.keys {
                unsafe { key.free() };
            }
        }
    }

    fn bcs_message(bytes: &[u8]) -> Vec<u8> {
        bcs::to_bytes(&IntentMessage {
            intent: SuiIntent::sui_app(IntentScope::PersonalMessage),
            value: app_sui::types::intent::PersonalMessage {
                message: bytes.to_vec(),
            },
        })
        .unwrap()
    }

    #[test]
    fn request_parser_rejects_raw_message_fallback() {
        let fixture = RequestFixture::new(hex::decode("030000616263").unwrap());
        unsafe {
            let result = fixture.parse();
            if let Ok(display) = &result {
                display.free();
            }
            assert!(
                result.is_err(),
                "raw input must not be reinterpreted after BCS failure"
            );
        }
    }

    #[test]
    fn parse_entry_passes_device_keys_and_returns_display_or_error() {
        #[repr(C)]
        struct ParseResponse {
            data: *mut DisplayIotaIntentData,
            error_code: u32,
            error_message: PtrString,
        }
        let mut fixture = RequestFixture::new(bcs_message(b"abc\0def"));
        unsafe {
            for with_keys in [true, false] {
                let (keys, len) = if with_keys {
                    (fixture.keys.as_mut_ptr(), fixture.keys.len() as u32)
                } else {
                    (null_mut(), 0)
                };
                let ptr =
                    super::super::iota_parse_intent(&mut fixture.request as *mut _ as _, keys, len);
                let response = &*(ptr as *const ParseResponse);
                if with_keys {
                    assert_eq!(response.error_code, 0);
                    let display = &*response.data;
                    assert!(display.message.is_null());
                    assert_eq!(recover_opt(display.details).unwrap(), "61626300646566");
                    assert_eq!(recover_opt(display.sender).unwrap(), fixture.address);
                } else {
                    assert_ne!(response.error_code, 0);
                    assert!(response.data.is_null());
                }
                Box::from_raw(ptr).free();
            }
        }
    }

    #[test]
    fn request_parser_rejects_spoofed_address() {
        let mut fixture = RequestFixture::new(bcs_message(b"abc"));
        fixture.request.set_addresses(Some(vec![vec![0x42; 32]]));
        unsafe {
            let result = fixture.parse();
            if let Ok(display) = &result {
                display.free();
            }
            assert!(
                result.is_err(),
                "request address must match the signing key"
            );
        }
    }

    #[test]
    fn request_parser_preserves_standard_bcs_text() {
        for message in [
            b"abc".to_vec(),
            Vec::new(),
            vec![b'A'; 128],
            b"YWJj".to_vec(),
        ] {
            let fixture = RequestFixture::new(bcs_message(&message));
            unsafe {
                let display = fixture.parse().unwrap();
                assert_eq!(recover_opt(display.message).unwrap().as_bytes(), message);
                assert!(display.details.is_null());
                assert_eq!(recover_opt(display.sender).unwrap(), fixture.address);
                display.free();
            }
        }
        assert_eq!(bcs_message(b"abc"), hex::decode("03000003616263").unwrap());
    }

    #[test]
    fn request_parser_preserves_non_renderable_bytes_as_hex() {
        for message in [
            b"abc\0def".to_vec(),
            b"A\x01B".to_vec(),
            vec![0xff, 0xfe],
            hex::decode("e4bda0e5a5bd").unwrap(),
            hex::decode("f09f9880").unwrap(),
        ] {
            let fixture = RequestFixture::new(bcs_message(&message));
            unsafe {
                let display = fixture.parse().unwrap();
                assert!(display.message.is_null());
                assert_eq!(
                    recover_opt(display.details).unwrap().replace(' ', ""),
                    hex::encode(&message)
                );
                display.free();
            }
        }
    }

    #[test]
    fn request_parser_rejects_malformed_bcs_and_intents() {
        for bytes in [
            "",
            "03",
            "0300",
            "030000",
            "0301000161",
            "0300010161",
            "030000036162",
            "030000016162",
            "030000810061",
            "03000080",
            "030000610062",
        ] {
            let fixture = RequestFixture::new(hex::decode(bytes).unwrap());
            unsafe {
                assert!(fixture.parse().is_err(), "accepted {bytes}");
            }
        }
    }

    #[test]
    fn request_parser_uses_device_address_when_optional_address_is_absent() {
        let mut fixture = RequestFixture::new(bcs_message(b"abc"));
        fixture.request.set_addresses(None);
        unsafe {
            let display = fixture.parse().unwrap();
            assert_eq!(recover_opt(display.sender).unwrap(), fixture.address);
            display.free();
        }
    }

    #[test]
    fn request_parser_rejects_malformed_addresses_and_missing_keys() {
        let mut fixture = RequestFixture::new(bcs_message(b"abc"));
        for addresses in [vec![], vec![vec![]], vec![vec![0; 31]], vec![vec![0; 33]]] {
            fixture.request.set_addresses(Some(addresses));
            unsafe {
                assert!(fixture.parse().is_err());
            }
        }
        fixture.request.set_addresses(None);
        unsafe {
            assert!(super::super::parse_iota_request(&fixture.request, &[]).is_err());
            free_ptr_string(fixture.keys[0].xpub);
            fixture.keys[0].xpub = null_mut();
            assert!(fixture.parse().is_err());
        }
    }

    #[test]
    fn request_parser_selects_exact_device_path_not_first_key() {
        use ur_registry::crypto_key_path::CryptoKeyPath;
        let mut fixture = RequestFixture::new(bcs_message(b"abc"));
        let path = "M/44'/4218'/1'/0'/0'".to_string();
        let pubkey = hex::encode(crate::sui::get_public_key(&[1; 32], &path).unwrap());
        let address = app_iota::address::get_address_from_pubkey(pubkey.clone()).unwrap();
        fixture
            .keys
            .push(crate::common::structs::ExtendedPublicKey {
                path: convert_c_char(path.clone()),
                xpub: convert_c_char(pubkey),
            });
        fixture
            .request
            .set_derivation_paths(vec![
                CryptoKeyPath::from_path(path, Some([1, 2, 3, 4])).unwrap()
            ]);
        unsafe {
            assert!(fixture.parse().is_err());
            fixture
                .request
                .set_addresses(Some(vec![hex::decode(&address[2..]).unwrap()]));
            let display = fixture.parse().unwrap();
            assert_eq!(recover_opt(display.sender).unwrap(), address);
            display.free();
        }
    }

    #[test]
    fn request_parser_rejects_missing_or_unmatched_paths() {
        use ur_registry::crypto_key_path::{CryptoKeyPath, PathComponent};
        let mut fixture = RequestFixture::new(bcs_message(b"abc"));
        fixture.request.set_addresses(None);
        for path in [
            "m/44'/4218'/10'/0'/0'",
            "m/44'/4218'/0'/0'/1'",
            "m/44'/4218'/0'/0/0'",
            "m/44'/784'/0'/0'/0'",
        ] {
            fixture
                .request
                .set_derivation_paths(vec![CryptoKeyPath::from_path(
                    path.to_string(),
                    Some([1, 2, 3, 4]),
                )
                .unwrap()]);
            unsafe {
                assert!(fixture.parse().is_err(), "accepted {path}");
            }
        }
        fixture
            .request
            .set_derivation_paths(vec![CryptoKeyPath::new(
                vec![PathComponent::new(None, true).unwrap()],
                None,
                None,
            )]);
        unsafe {
            assert!(fixture.parse().is_err());
        }
        fixture
            .request
            .set_derivation_paths(vec![CryptoKeyPath::default()]);
        unsafe {
            assert!(fixture.parse().is_err());
        }
        fixture.request.set_derivation_paths(vec![]);
        unsafe {
            assert!(fixture.parse().is_err());
        }
    }

    #[test]
    fn signing_entry_rejects_invalid_messages_and_clears_seed() {
        for bytes in ["030000616263", "030000036162", "0301000161"] {
            let mut fixture = RequestFixture::new(hex::decode(bytes).unwrap());
            let mut seed = [1u8; 32];
            unsafe {
                let result = Box::from_raw(super::super::iota_sign_intent(
                    &mut fixture.request as *mut _ as _,
                    seed.as_mut_ptr(),
                    seed.len() as u32,
                ));
                assert!(result.data.is_null(), "signed invalid request {bytes}");
                assert_eq!(seed, [0; 32]);
                result.free();
            }
        }
    }

    #[test]
    fn signing_entry_binds_address_and_preserves_valid_signatures() {
        let mut fixture = RequestFixture::new(bcs_message(b"abc\0def"));
        let path = "m/44'/4218'/0'/0'/0'".to_string();
        let expected_signature =
            app_sui::sign_intent(&[1; 32], &path, &fixture.request.get_intent_message()).unwrap();
        let pubkey = crate::sui::get_public_key(&[1; 32], &path).unwrap();
        let signature = ur_registry::iota::iota_signature::IotaSignature::new(
            None,
            expected_signature.to_vec(),
            Some(pubkey),
        );
        let bytes: Vec<u8> = signature.try_into().unwrap();
        let expected = crate::common::ur::UREncodeResult::encode(
            bytes,
            "iota-signature".to_string(),
            crate::common::ur::FRAGMENT_MAX_LENGTH_DEFAULT,
        );
        unsafe {
            let mut seed = [1u8; 32];
            let result = Box::from_raw(super::super::iota_sign_intent(
                &mut fixture.request as *mut _ as _,
                seed.as_mut_ptr(),
                seed.len() as u32,
            ));
            assert_eq!(recover_opt(result.data), recover_opt(expected.data));
            assert!(!result.data.is_null());
            assert_eq!(seed, [0; 32]);
            result.free();
            expected.free();

            fixture.request.set_addresses(Some(vec![vec![0x42; 32]]));
            let mut seed = [1u8; 32];
            let result = Box::from_raw(super::super::iota_sign_intent(
                &mut fixture.request as *mut _ as _,
                seed.as_mut_ptr(),
                seed.len() as u32,
            ));
            assert!(result.data.is_null());
            assert_eq!(seed, [0; 32]);
            result.free();
        }
    }

    #[no_mangle]
    extern "C" fn GenerateTRNGRandomness(randomness: *mut u8, len: u8) -> i32 {
        unsafe {
            core::slice::from_raw_parts_mut(randomness, len as usize).fill(0x42);
        }
        0
    }

    fn system_state_object() -> CallArg {
        let mut id = [0u8; 32];
        id[31] = 5;
        CallArg::Object(ObjectArg::SharedObject {
            id: SuiAddress::from_bytes(&id).unwrap(),
            initial_shared_version: SequenceNumber::new(1),
            mutability: SharedObjectMutability::Mutable,
        })
    }

    fn owned_coin_object(id_byte: u8) -> CallArg {
        let mut id = [0u8; 32];
        id[31] = id_byte;
        CallArg::Object(ObjectArg::ImmOrOwnedObject((
            SuiAddress::from_bytes(&id).unwrap(),
            SequenceNumber::new(1),
            ObjectDigest::MIN,
        )))
    }

    #[test]
    fn formats_iota_amount_for_display() {
        assert_eq!(format_iota_amount("0".to_string()).unwrap(), "0 IOTA");
        assert_eq!(
            format_iota_amount("1".to_string()).unwrap(),
            "0.000000001 IOTA"
        );
        assert_eq!(
            format_iota_amount("1500000000".to_string()).unwrap(),
            "1.5 IOTA"
        );
        assert_eq!(format_iota_amount("invalid".to_string()), None);
    }

    fn iota_system_package() -> SuiAddress {
        SuiAddress::from_bytes(&hex::decode(IOTA_SYSTEM_PACKAGE_HEX).unwrap()).unwrap()
    }

    fn move_call(
        module: &str,
        function: &str,
        package: SuiAddress,
        arguments: Vec<Argument>,
    ) -> Command {
        Command::MoveCall(Box::new(ProgrammableMoveCall {
            package,
            module: module.to_string(),
            function: function.to_string(),
            type_arguments: Vec::new(),
            arguments,
        }))
    }

    unsafe fn recover_opt(ptr: PtrString) -> Option<String> {
        if ptr.is_null() {
            None
        } else {
            Some(recover_c_char(ptr))
        }
    }

    #[test]
    fn formats_non_text_message_as_grouped_hex() {
        let message = Intent::PersonalMessage(IntentMessage {
            intent: SuiIntent::sui_app(IntentScope::PersonalMessage),
            value: PersonalMessageUtf8 {
                message: "[255,254,0,65,66,67,68,69,70]".to_string(),
            },
        });

        let display = DisplayIotaIntentData::from(message)
            .with_personal_message_bytes(&[0xff, 0xfe, 0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46]);
        unsafe {
            assert!(display.message.is_null());
            assert_eq!(
                recover_opt(display.details),
                Some("fffe004142434445 46".to_string())
            );
            display.free();
        }
    }

    #[test]
    fn preserves_base64_looking_text_message() {
        let message = Intent::PersonalMessage(IntentMessage {
            intent: SuiIntent::sui_app(IntentScope::PersonalMessage),
            value: PersonalMessageUtf8 {
                message: "YWJj".to_string(),
            },
        });

        let display = DisplayIotaIntentData::from(message).with_personal_message_bytes(b"YWJj");
        unsafe {
            assert_eq!(recover_opt(display.message), Some("YWJj".to_string()));
            assert!(display.details.is_null());
            display.free();
        }
    }

    #[test]
    fn formats_emoji_message_as_raw_hex() {
        let text = "I love Nightly 🦊";
        let message = Intent::PersonalMessage(IntentMessage {
            intent: SuiIntent::sui_app(IntentScope::PersonalMessage),
            value: PersonalMessageUtf8 {
                message: text.to_string(),
            },
        });

        let display =
            DisplayIotaIntentData::from(message).with_personal_message_bytes(text.as_bytes());
        unsafe {
            assert!(display.message.is_null());
            assert_eq!(
                recover_opt(display.details),
                Some("49206c6f7665204e 696768746c7920f0 9fa68a".to_string())
            );
            display.free();
        }
    }

    #[test]
    fn formats_control_character_message_as_raw_hex() {
        let message = Intent::PersonalMessage(IntentMessage {
            intent: SuiIntent::sui_app(IntentScope::PersonalMessage),
            value: PersonalMessageUtf8 {
                message: "[65,1,66]".to_string(),
            },
        });

        let display = DisplayIotaIntentData::from(message).with_personal_message_bytes(b"A\x01B");
        unsafe {
            assert!(display.message.is_null());
            assert_eq!(recover_opt(display.details), Some("410142".to_string()));
            display.free();
        }
    }

    #[test]
    fn formats_cjk_message_as_raw_hex() {
        let text = "你好";
        let message = Intent::PersonalMessage(IntentMessage {
            intent: SuiIntent::sui_app(IntentScope::PersonalMessage),
            value: PersonalMessageUtf8 {
                message: text.to_string(),
            },
        });

        let display =
            DisplayIotaIntentData::from(message).with_personal_message_bytes(text.as_bytes());
        unsafe {
            assert!(display.message.is_null());
            assert_eq!(
                recover_opt(display.details),
                Some("e4bda0e5a5bd".to_string())
            );
            display.free();
        }
    }

    #[test]
    fn recognises_fixed_transfer() {
        let inputs = vec![
            CallArg::Pure(1_000_000_000u64.to_le_bytes().to_vec()),
            CallArg::Pure([0xaa; 32].to_vec()),
        ];
        let commands = vec![
            Command::SplitCoins(Argument::GasCoin, vec![Argument::Input(0)]),
            Command::TransferObjects(vec![Argument::Result(0)], Argument::Input(1)),
        ];
        let (method, amount, recipient) = extract_transaction_params(&inputs, &commands);
        unsafe {
            assert_eq!(recover_opt(method).unwrap(), "Transfer");
            assert_eq!(recover_opt(amount).unwrap(), "1 IOTA");
            assert_eq!(
                recover_opt(recipient).unwrap(),
                format!("0x{}", hex::encode([0xaa; 32]))
            );
        }
    }

    #[test]
    fn recognises_max_transfer() {
        let inputs = vec![CallArg::Pure([0xbb; 32].to_vec())];
        let commands = vec![Command::TransferObjects(
            vec![Argument::GasCoin],
            Argument::Input(0),
        )];
        let (method, amount, recipient) = extract_transaction_params(&inputs, &commands);
        unsafe {
            assert_eq!(recover_opt(method).unwrap(), "Transfer");
            assert_eq!(recover_opt(amount).unwrap(), "max");
            assert_eq!(
                recover_opt(recipient).unwrap(),
                format!("0x{}", hex::encode([0xbb; 32]))
            );
        }
    }

    #[test]
    fn recognises_single_coin_stake() {
        let package = iota_system_package();
        let inputs = vec![
            CallArg::Pure(Vec::new()),
            CallArg::Pure(2_000_000_000u64.to_le_bytes().to_vec()),
            CallArg::Pure([0xcc; 32].to_vec()),
        ];
        let commands = vec![
            Command::SplitCoins(Argument::GasCoin, vec![Argument::Input(1)]),
            move_call(
                "iota_system",
                "request_add_stake",
                package,
                vec![Argument::Input(0), Argument::Result(0), Argument::Input(2)],
            ),
        ];
        let (method, amount, validator) = extract_transaction_params(&inputs, &commands);
        unsafe {
            assert_eq!(recover_opt(method).unwrap(), "Stake");
            assert_eq!(recover_opt(amount).unwrap(), "2 IOTA");
            assert_eq!(
                recover_opt(validator).unwrap(),
                format!("0x{}", hex::encode([0xcc; 32]))
            );
        }
    }

    #[test]
    fn recognises_multi_coin_stake_with_explicit_amount() {
        let package = iota_system_package();
        let mut amount_bytes = vec![1u8];
        amount_bytes.extend_from_slice(&3_000_000_000u64.to_le_bytes());
        let inputs = vec![
            system_state_object(),
            owned_coin_object(0x01),
            CallArg::Pure(amount_bytes),
            CallArg::Pure([0xdd; 32].to_vec()),
        ];
        let commands = vec![
            Command::MakeMoveVec(None, vec![Argument::Input(1)]),
            move_call(
                "iota_system",
                "request_add_stake_mul_coin",
                package,
                vec![
                    Argument::Input(0),
                    Argument::Result(0),
                    Argument::Input(2),
                    Argument::Input(3),
                ],
            ),
        ];
        let (method, amount, validator) = extract_transaction_params(&inputs, &commands);
        unsafe {
            assert_eq!(recover_opt(method).unwrap(), "Stake");
            assert_eq!(recover_opt(amount).unwrap(), "3 IOTA");
            assert_eq!(
                recover_opt(validator).unwrap(),
                format!("0x{}", hex::encode([0xdd; 32]))
            );
        }
    }

    #[test]
    fn rejects_multi_coin_stake_with_none_amount() {
        let package = iota_system_package();
        let inputs = vec![
            system_state_object(),
            owned_coin_object(0x02),
            CallArg::Pure(vec![0u8]),
            CallArg::Pure([0xee; 32].to_vec()),
        ];
        let commands = vec![
            Command::MakeMoveVec(None, vec![Argument::Input(1)]),
            move_call(
                "iota_system",
                "request_add_stake_mul_coin",
                package,
                vec![
                    Argument::Input(0),
                    Argument::Result(0),
                    Argument::Input(2),
                    Argument::Input(3),
                ],
            ),
        ];
        let (method, amount, validator) = extract_transaction_params(&inputs, &commands);
        assert!(method.is_null());
        assert!(amount.is_null());
        assert!(validator.is_null());
    }

    #[test]
    fn rejects_multi_coin_stake_with_malformed_amount_option() {
        let package = iota_system_package();
        let inputs = vec![
            system_state_object(),
            owned_coin_object(0x03),
            CallArg::Pure(vec![7u8]),
            CallArg::Pure([0xff; 32].to_vec()),
        ];
        let commands = vec![
            Command::MakeMoveVec(None, vec![Argument::Input(1)]),
            move_call(
                "iota_system",
                "request_add_stake_mul_coin",
                package,
                vec![
                    Argument::Input(0),
                    Argument::Result(0),
                    Argument::Input(2),
                    Argument::Input(3),
                ],
            ),
        ];
        let (method, amount, validator) = extract_transaction_params(&inputs, &commands);
        assert!(method.is_null());
        assert!(amount.is_null());
        assert!(validator.is_null());
    }

    #[test]
    fn rejects_stake_call_from_impostor_package() {
        let fake_package = SuiAddress::from_bytes(&[0x42; 32]).unwrap();
        let inputs = vec![
            CallArg::Pure(Vec::new()),
            CallArg::Pure(1_000_000_000u64.to_le_bytes().to_vec()),
            CallArg::Pure([0x11; 32].to_vec()),
        ];
        let commands = vec![
            Command::SplitCoins(Argument::GasCoin, vec![Argument::Input(1)]),
            move_call(
                "iota_system",
                "request_add_stake",
                fake_package,
                vec![Argument::Input(0), Argument::Result(0), Argument::Input(2)],
            ),
        ];
        let (method, amount, recipient) = extract_transaction_params(&inputs, &commands);
        assert!(method.is_null());
        assert!(amount.is_null());
        assert!(recipient.is_null());
    }

    #[test]
    fn rejects_transfer_with_extra_trailing_command() {
        let inputs = vec![
            CallArg::Pure(1_000_000_000u64.to_le_bytes().to_vec()),
            CallArg::Pure([0xaa; 32].to_vec()),
        ];
        let commands = vec![
            Command::SplitCoins(Argument::GasCoin, vec![Argument::Input(0)]),
            Command::TransferObjects(vec![Argument::Result(0)], Argument::Input(1)),
            Command::TransferObjects(vec![Argument::GasCoin], Argument::Input(1)),
        ];
        let (method, amount, recipient) = extract_transaction_params(&inputs, &commands);
        assert!(method.is_null());
        assert!(amount.is_null());
        assert!(recipient.is_null());
    }

    #[test]
    fn rejects_transfer_target_that_is_not_a_32_byte_pure() {
        let inputs = vec![
            CallArg::Pure(1_000_000_000u64.to_le_bytes().to_vec()),
            CallArg::Pure(vec![0xaa; 20]),
        ];
        let commands = vec![
            Command::SplitCoins(Argument::GasCoin, vec![Argument::Input(0)]),
            Command::TransferObjects(vec![Argument::Result(0)], Argument::Input(1)),
        ];
        let (method, amount, recipient) = extract_transaction_params(&inputs, &commands);
        assert!(method.is_null());
        assert!(amount.is_null());
        assert!(recipient.is_null());
    }

    #[test]
    fn rejects_unrecognised_move_call() {
        let package = iota_system_package();
        let inputs: Vec<CallArg> = Vec::new();
        let commands = vec![move_call(
            "some_module",
            "some_function",
            package,
            Vec::new(),
        )];
        let (method, amount, recipient) = extract_transaction_params(&inputs, &commands);
        assert!(method.is_null());
        assert!(amount.is_null());
        assert!(recipient.is_null());
    }
}
