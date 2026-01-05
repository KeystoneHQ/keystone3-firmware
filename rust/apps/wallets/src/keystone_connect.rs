use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use core::str::FromStr;

use {
    bitcoin::bip32::{ChildNumber, DerivationPath},
    bitcoin::secp256k1::Secp256k1,
    hex,
    ur_registry::{
        crypto_hd_key::CryptoHDKey,
        crypto_key_path::{CryptoKeyPath, PathComponent},
        error::{URError, URResult},
        extend::crypto_multi_accounts::CryptoMultiAccounts,
    },
};

use crate::{common::get_path_component, ExtendedPublicKey};

fn get_device_id(serial_number: &str) -> String {
    use cryptoxide::hashing::sha256;
    hex::encode(&sha256(&sha256(serial_number.as_bytes()))[0..20])
}

const BTC_LEGACY_PREFIX: &str = "44'/0'/0'";
const BTC_SEGWIT_PREFIX: &str = "49'/0'/0'";
const BTC_NATIVE_SEGWIT_PREFIX: &str = "84'/0'/0'";
const BTC_TAPROOT_PREFIX: &str = "86'/0'/0'";
const ETH_STANDARD_PREFIX: &str = "44'/60'/0'";
const ETH_LEDGER_LIVE_PREFIX: &str = "44'/60'"; //overlap with ETH_STANDARD at 0
const TRX_PREFIX: &str = "44'/195'/0'";
const DOGE_LEGACY_PREFIX: &str = "44'/3'/0'";
const XRP_PREFIX: &str = "44'/144'/0'";
const LTC_PREFIX: &str = "49'/2'/0'";
const LTC_NATIVE_SEGWIT_PREFIX: &str = "84'/2'/0'";

pub fn generate_crypto_multi_accounts(
    master_fingerprint: [u8; 4],
    serial_number: &str,
    extended_public_keys: Vec<ExtendedPublicKey>,
    device_type: &str,
    device_version: &str,
) -> URResult<CryptoMultiAccounts> {
    let device_id = get_device_id(serial_number);
    let mut keys = vec![];
    let k1_keys = [
        BTC_LEGACY_PREFIX.to_string(),
        BTC_SEGWIT_PREFIX.to_string(),
        BTC_NATIVE_SEGWIT_PREFIX.to_string(),
        BTC_TAPROOT_PREFIX.to_string(),
        TRX_PREFIX.to_string(),
        DOGE_LEGACY_PREFIX.to_string(),
        XRP_PREFIX.to_string(),
        LTC_PREFIX.to_string(),
        LTC_NATIVE_SEGWIT_PREFIX.to_string(),
    ];
    for ele in extended_public_keys {
        match ele.get_path() {
            _path if k1_keys.contains(&_path.to_string().to_lowercase()) => {
                keys.push(generate_k1_normal_key(
                    master_fingerprint,
                    ele.clone(),
                    None,
                    false,
                )?);
            }
            _path if _path.to_string().to_lowercase().eq(ETH_STANDARD_PREFIX) => {
                keys.push(generate_k1_normal_key(
                    master_fingerprint,
                    ele.clone(),
                    Some("account.standard".to_string()),
                    true,
                )?);
                keys.push(generate_eth_ledger_legacy_pubkey(
                    master_fingerprint,
                    ele.clone(),
                    Some("account.ledger_legacy".to_string()),
                )?);
                keys.push(generate_eth_ledger_live_key(
                    master_fingerprint,
                    ele,
                    Some("account.ledger_live".to_string()),
                )?);
            }
            _path
                if _path
                    .to_string()
                    .to_lowercase()
                    .starts_with(ETH_LEDGER_LIVE_PREFIX) =>
            {
                keys.push(generate_eth_ledger_live_key(
                    master_fingerprint,
                    ele,
                    Some("account.ledger_live".to_string()),
                )?);
            }
            _ => {
                return Err(URError::UrEncodeError(format!(
                    "Unknown key path: {}",
                    ele.path
                )))
            }
        }
    }

    Ok(CryptoMultiAccounts::new(
        master_fingerprint,
        keys,
        Some(device_type.to_string()),
        Some(device_id),
        Some(device_version.to_string()),
    ))
}

fn generate_eth_ledger_legacy_pubkey(
    mfp: [u8; 4],
    key: ExtendedPublicKey,
    note: Option<String>,
) -> URResult<CryptoHDKey> {
    let xpub = bitcoin::bip32::Xpub::decode(&key.get_key())
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let path = key.get_path();
    let sub_path =
        DerivationPath::from_str("m/0").map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let _target_key = xpub
        .derive_pub(&Secp256k1::new(), &sub_path)
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let target_path = path.child(ChildNumber::Normal { index: 0 });
    let key_path = CryptoKeyPath::new(
        target_path
            .into_iter()
            .map(|v| match v {
                ChildNumber::Normal { index } => get_path_component(Some(*index), false),
                ChildNumber::Hardened { index } => get_path_component(Some(*index), true),
            })
            .collect::<URResult<Vec<PathComponent>>>()?,
        Some(mfp),
        Some(xpub.depth as u32),
    );

    Ok(CryptoHDKey::new_extended_key(
        Some(false),
        _target_key.public_key.serialize().to_vec(),
        None,
        None,
        Some(key_path),
        None,
        Some(_target_key.parent_fingerprint.to_bytes()),
        Some("Keystone".to_string()),
        note,
    ))
}

fn generate_k1_normal_key(
    mfp: [u8; 4],
    key: ExtendedPublicKey,
    note: Option<String>,
    is_standard: bool,
) -> URResult<CryptoHDKey> {
    let xpub = bitcoin::bip32::Xpub::decode(&key.get_key())
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let path = key.get_path();
    let key_path = CryptoKeyPath::new(
        path.into_iter()
            .map(|v| match v {
                ChildNumber::Normal { index } => get_path_component(Some(*index), false),
                ChildNumber::Hardened { index } => get_path_component(Some(*index), true),
            })
            .collect::<URResult<Vec<PathComponent>>>()?,
        Some(mfp),
        Some(xpub.depth as u32),
    );

    let children = CryptoKeyPath::new(
        match is_standard {
            true => {
                vec![
                    get_path_component(Some(0), false)?,
                    get_path_component(None, false)?,
                ]
            }
            false => vec![get_path_component(None, false)?],
        },
        None,
        Some(0),
    );
    Ok(CryptoHDKey::new_extended_key(
        Some(false),
        xpub.public_key.serialize().to_vec(),
        Some(xpub.chain_code.to_bytes().to_vec()),
        None,
        Some(key_path),
        Some(children),
        Some(xpub.parent_fingerprint.to_bytes()),
        Some("Keystone".to_string()),
        note,
    ))
}

fn generate_eth_ledger_live_key(
    mfp: [u8; 4],
    key: ExtendedPublicKey,
    note: Option<String>,
) -> URResult<CryptoHDKey> {
    let xpub = bitcoin::bip32::Xpub::decode(&key.get_key())
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let path = key.get_path();
    let sub_path =
        DerivationPath::from_str("m/0/0").map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let _target_key = xpub
        .derive_pub(&Secp256k1::new(), &sub_path)
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let target_path = path
        .child(ChildNumber::Normal { index: 0 })
        .child(ChildNumber::Normal { index: 0 });
    let key_path = CryptoKeyPath::new(
        target_path
            .into_iter()
            .map(|v| match v {
                ChildNumber::Normal { index } => get_path_component(Some(*index), false),
                ChildNumber::Hardened { index } => get_path_component(Some(*index), true),
            })
            .collect::<URResult<Vec<PathComponent>>>()?,
        Some(mfp),
        Some(xpub.depth as u32),
    );
    Ok(CryptoHDKey::new_extended_key(
        Some(false),
        _target_key.public_key.serialize().to_vec(),
        None,
        None,
        Some(key_path),
        None,
        Some(_target_key.parent_fingerprint.to_bytes()),
        Some("Keystone".to_string()),
        note,
    ))
}

#[cfg(test)]
mod tests {
    extern crate std;
    use crate::DEVICE_TYPE;

    use super::*;
    use alloc::vec::Vec;

    use hex;
    use hex::FromHex;
    #[test]
    fn test_generate_crypto_multi_accounts() {
        let mfp = hex::encode([82, 116, 71, 3]);
        let mfp = Vec::from_hex(mfp).unwrap();
        let mfp: [u8; 4] = mfp.try_into().unwrap();

        let x_pub_1_path = "m/44'/60'/0'";
        let x_pub_1 = vec![
            4, 136, 178, 30, 3, 225, 141, 236, 200, 128, 0, 0, 0, 150, 103, 43, 221, 62, 231, 33,
            47, 231, 148, 112, 64, 231, 3, 205, 219, 170, 181, 180, 152, 129, 87, 135, 166, 1, 71,
            108, 117, 231, 231, 137, 157, 3, 76, 114, 154, 166, 56, 179, 38, 22, 64, 168, 240, 106,
            94, 171, 191, 228, 192, 77, 46, 10, 172, 67, 75, 52, 65, 71, 161, 165, 250, 53, 85,
            163,
        ];
        let x_pub_2_path = "m/44'/60'/1'";
        let x_pub_2 = vec![
            4, 136, 178, 30, 3, 225, 141, 236, 200, 128, 0, 0, 1, 148, 143, 211, 16, 153, 241, 166,
            9, 231, 144, 251, 55, 121, 251, 223, 181, 222, 157, 63, 38, 163, 162, 65, 50, 248, 30,
            93, 185, 103, 109, 120, 129, 2, 55, 60, 190, 7, 95, 36, 155, 222, 189, 16, 51, 171,
            118, 251, 92, 168, 24, 193, 204, 69, 67, 178, 12, 79, 7, 216, 134, 139, 30, 103, 186,
            251,
        ];

        let x_pub_3_path = "m/44'/60'/2'";
        let x_pub_3 = vec![
            4, 136, 178, 30, 3, 225, 141, 236, 200, 128, 0, 0, 2, 16, 107, 233, 212, 0, 17, 88,
            106, 139, 83, 176, 242, 100, 190, 71, 214, 17, 166, 120, 29, 31, 68, 180, 142, 32, 235,
            36, 81, 84, 233, 42, 191, 3, 151, 84, 229, 42, 100, 158, 78, 217, 45, 36, 148, 174, 59,
            182, 164, 145, 107, 245, 4, 218, 72, 95, 171, 102, 158, 205, 1, 177, 203, 247, 38, 220,
        ];
        let x_pub_4_path = "m/44'/60'/3'";
        let x_pub_4 = vec![
            4, 136, 178, 30, 3, 225, 141, 236, 200, 128, 0, 0, 3, 74, 223, 222, 76, 163, 132, 239,
            26, 157, 40, 237, 242, 53, 73, 61, 122, 193, 54, 44, 251, 71, 234, 29, 105, 138, 186,
            91, 239, 193, 10, 107, 68, 2, 159, 183, 206, 159, 91, 8, 120, 253, 25, 25, 122, 144,
            226, 118, 212, 104, 225, 155, 24, 210, 77, 167, 138, 132, 194, 194, 67, 128, 113, 141,
            1, 27,
        ];

        let x_pub_5_path = "m/44'/60'/4'";
        let x_pub_5 = vec![
            4, 136, 178, 30, 3, 225, 141, 236, 200, 128, 0, 0, 4, 138, 172, 17, 199, 232, 239, 212,
            251, 67, 191, 6, 103, 42, 86, 181, 161, 55, 146, 19, 187, 179, 189, 184, 247, 243, 149,
            25, 7, 176, 132, 47, 122, 2, 16, 111, 49, 181, 222, 200, 141, 94, 51, 86, 244, 64, 221,
            221, 193, 97, 67, 228, 185, 183, 66, 125, 195, 244, 6, 90, 12, 233, 210, 136, 60, 187,
        ];

        let x_pub_6_path = "m/44'/60'/5'";
        let x_pub_6 = vec![
            4, 136, 178, 30, 3, 225, 141, 236, 200, 128, 0, 0, 5, 62, 230, 228, 88, 237, 74, 149,
            156, 199, 102, 0, 127, 143, 56, 85, 221, 226, 212, 35, 82, 103, 109, 9, 24, 81, 48,
            216, 87, 112, 0, 49, 225, 3, 58, 34, 37, 52, 169, 50, 194, 74, 19, 194, 180, 6, 156,
            241, 54, 136, 69, 65, 171, 141, 95, 179, 189, 34, 150, 82, 2, 154, 186, 11, 116, 184,
        ];

        let x_pub_7_path = "m/44'/60'/6'";
        let x_pub_7 = vec![
            4, 136, 178, 30, 3, 225, 141, 236, 200, 128, 0, 0, 6, 158, 224, 95, 103, 228, 232, 148,
            203, 99, 195, 37, 140, 15, 199, 125, 197, 42, 209, 179, 116, 4, 24, 134, 222, 120, 76,
            123, 113, 130, 69, 68, 2, 2, 205, 139, 195, 67, 65, 220, 173, 157, 52, 93, 196, 12,
            242, 246, 89, 191, 217, 35, 106, 9, 37, 7, 143, 141, 162, 160, 73, 97, 69, 0, 140, 36,
        ];

        let x_pub_8_path = "m/44'/60'/7'";
        let x_pub_8 = vec![
            4, 136, 178, 30, 3, 225, 141, 236, 200, 128, 0, 0, 7, 108, 132, 52, 99, 212, 233, 149,
            22, 235, 206, 217, 140, 53, 85, 233, 66, 195, 191, 103, 228, 78, 24, 2, 63, 32, 170,
            61, 153, 173, 194, 5, 194, 2, 181, 234, 55, 152, 34, 162, 157, 219, 30, 82, 241, 134,
            12, 173, 159, 198, 171, 40, 44, 112, 78, 128, 106, 118, 51, 187, 150, 164, 210, 192,
            170, 96,
        ];

        let x_pub_9_path = "m/44'/60'/8'";
        let x_pub_9 = vec![
            4, 136, 178, 30, 3, 225, 141, 236, 200, 128, 0, 0, 8, 148, 81, 116, 27, 224, 203, 95,
            155, 195, 120, 112, 171, 17, 25, 0, 244, 176, 200, 94, 136, 65, 84, 191, 180, 100, 210,
            90, 91, 155, 119, 155, 111, 2, 147, 97, 104, 39, 151, 40, 160, 135, 226, 145, 37, 55,
            28, 147, 157, 160, 71, 20, 179, 46, 89, 242, 178, 211, 152, 240, 193, 81, 46, 185, 7,
            133,
        ];

        let x_pub_10_path = "m/44'/60'/9'";
        let x_pub_10 = vec![
            4, 136, 178, 30, 3, 225, 141, 236, 200, 128, 0, 0, 9, 207, 200, 119, 68, 9, 204, 133,
            63, 160, 113, 213, 93, 158, 70, 181, 249, 98, 3, 206, 34, 73, 6, 41, 10, 108, 33, 118,
            198, 159, 13, 156, 227, 3, 197, 46, 28, 0, 66, 10, 72, 140, 130, 55, 161, 162, 81, 181,
            48, 125, 109, 186, 143, 69, 241, 243, 214, 178, 67, 70, 221, 8, 37, 216, 238, 137,
        ];

        let x_pub_11_path = "m/44'/0'/0'";
        let x_pub_11 = vec![
            4, 136, 178, 30, 3, 116, 65, 243, 92, 128, 0, 0, 0, 81, 116, 56, 174, 194, 183, 142,
            129, 194, 117, 164, 27, 230, 61, 246, 8, 51, 88, 7, 12, 190, 253, 181, 157, 230, 155,
            210, 249, 156, 0, 62, 138, 3, 235, 213, 82, 2, 123, 115, 173, 177, 222, 26, 164, 148,
            202, 124, 237, 254, 120, 20, 52, 214, 241, 2, 165, 83, 85, 177, 24, 160, 197, 218, 120,
            188,
        ];

        let x_pub_12_path = "m/49'/0'/0'";
        let x_pub_12 = vec![
            4, 136, 178, 30, 3, 89, 252, 178, 101, 128, 0, 0, 0, 172, 49, 222, 228, 221, 63, 70,
            50, 249, 132, 224, 164, 30, 135, 40, 237, 195, 236, 103, 246, 20, 200, 240, 49, 129,
            73, 12, 137, 69, 209, 157, 116, 2, 243, 185, 124, 243, 243, 56, 126, 44, 77, 140, 113,
            65, 162, 21, 41, 169, 14, 5, 133, 233, 240, 50, 112, 103, 152, 209, 143, 152, 138, 86,
            227, 241,
        ];

        let _x_pub_13_path = "m/84'/0'/0'";
        let _x_pub_13 = vec![
            4, 136, 178, 30, 3, 129, 255, 52, 49, 128, 0, 0, 0, 57, 156, 154, 156, 107, 152, 113,
            18, 53, 162, 72, 79, 142, 68, 203, 181, 241, 230, 86, 48, 106, 225, 90, 11, 178, 154,
            13, 126, 210, 39, 240, 168, 2, 56, 80, 109, 189, 148, 232, 33, 102, 203, 104, 83, 111,
            250, 13, 14, 20, 95, 203, 135, 185, 117, 217, 219, 208, 71, 95, 219, 102, 79, 61, 174,
            166,
        ];

        let _x_pub_14_path = "m/86'/0'/0'";
        let _x_pub_14 = vec![
            4, 136, 178, 30, 3, 160, 104, 43, 1, 128, 0, 0, 0, 175, 58, 35, 239, 123, 26, 84, 211,
            219, 219, 108, 62, 80, 35, 130, 229, 93, 229, 255, 87, 95, 19, 206, 172, 245, 43, 224,
            27, 227, 124, 11, 68, 2, 108, 57, 94, 23, 99, 245, 166, 240, 122, 222, 53, 87, 66, 156,
            75, 218, 180, 93, 84, 135, 89, 158, 210, 131, 231, 133, 52, 172, 24, 22, 64, 143,
        ];

        let account = generate_crypto_multi_accounts(
            mfp,
            "31206",
            vec![
                ExtendedPublicKey::new(DerivationPath::from_str(x_pub_1_path).unwrap(), x_pub_1),
                ExtendedPublicKey::new(DerivationPath::from_str(x_pub_2_path).unwrap(), x_pub_2),
                ExtendedPublicKey::new(DerivationPath::from_str(x_pub_3_path).unwrap(), x_pub_3),
                ExtendedPublicKey::new(DerivationPath::from_str(x_pub_4_path).unwrap(), x_pub_4),
                ExtendedPublicKey::new(DerivationPath::from_str(x_pub_5_path).unwrap(), x_pub_5),
                ExtendedPublicKey::new(DerivationPath::from_str(x_pub_6_path).unwrap(), x_pub_6),
                ExtendedPublicKey::new(DerivationPath::from_str(x_pub_7_path).unwrap(), x_pub_7),
                ExtendedPublicKey::new(DerivationPath::from_str(x_pub_8_path).unwrap(), x_pub_8),
                ExtendedPublicKey::new(DerivationPath::from_str(x_pub_9_path).unwrap(), x_pub_9),
                ExtendedPublicKey::new(DerivationPath::from_str(x_pub_10_path).unwrap(), x_pub_10),
                ExtendedPublicKey::new(DerivationPath::from_str(x_pub_11_path).unwrap(), x_pub_11),
                ExtendedPublicKey::new(DerivationPath::from_str(x_pub_12_path).unwrap(), x_pub_12),
            ],
            DEVICE_TYPE,
            "1.1.0",
        )
        .unwrap();
        let cbor: Vec<u8> = account.try_into().unwrap();

        assert_eq!(hex::encode(cbor).to_lowercase(),
               "a5011a52744703028ed9012fa802f4035821034c729aa638b3261640a8f06a5eabbfe4c04d2e0aac434b344147a1a5fa3555a304582096672bdd3ee7212fe7947040e703cddbaab5b498815787a601476c75e7e7899d06d90130a30186182cf5183cf500f5021a52744703030307d90130a2018400f480f40300081ae18decc809684b657973746f6e650a706163636f756e742e7374616e64617264d9012fa602f4035821036b287717eb691cb108d23293fbef416906fa564942ebfeb659a506ed39d9a07f06d90130a30188182cf5183cf500f500f4021a527447030303081a87be59b409684b657973746f6e650a756163636f756e742e6c65646765725f6c6567616379d9012fa602f4035821039f4e693730f116e7ab01dac46b94ad4fcabc3ca7d91a6b121cc26782a8f2b8b206d90130a3018a182cf5183cf500f500f400f4021a527447030303081aaea3081709684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f403582102f6cb8d18cef477defff35eae0d4fbfdb7e929fcf124056642bb52005121f2b8206d90130a3018a182cf5183cf501f500f400f4021a527447030303081afbdad0f009684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f4035821025bac93928fb3a1b42abd1ef04a9df7152c3e3ac9784b0adeb8c74f02d6e081a706d90130a3018a182cf5183cf502f500f400f4021a527447030303081a127d09a409684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f4035821031c4731b1d0f2813f25732a07bc8634a1d43678fb5f3e339e31868e7a3e161b3106d90130a3018a182cf5183cf503f500f400f4021a527447030303081aaa7ae17b09684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f403582103ee66c1903304ae22499bb7236ed9afe9de557ae4aa8756660bd13468e016ebfe06d90130a3018a182cf5183cf504f500f400f4021a527447030303081a53cb808a09684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f403582103b36c22174290c6ce6edb10df4386f7a4dee680313a7daecab7ceb48edb41d0d406d90130a3018a182cf5183cf505f500f400f4021a527447030303081a37d6210c09684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f40358210301e2533680aceb0eea3cd0c23ca1fcec7ef0c085c4ba89986b56d919c6e775c106d90130a3018a182cf5183cf506f500f400f4021a527447030303081a14ff4a1509684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f403582102a54598a178b1153c19a3678851c4e35162c018a1b5f354ad9cbf169fc365e6e106d90130a3018a182cf5183cf507f500f400f4021a527447030303081af4cff0d209684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f403582103934334794763b31983061359e7c5de9aa400083fcdffbb7c3c8791bda35700fe06d90130a3018a182cf5183cf508f500f400f4021a527447030303081a4267812709684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f4035821026f95e551487e0c75ebfdbdcf6c68d4cf3596729f6b0700e4b789f161dd41557906d90130a3018a182cf5183cf509f500f400f4021a527447030303081aa109680309684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa702f403582103ebd552027b73adb1de1aa494ca7cedfe781434d6f102a55355b118a0c5da78bc045820517438aec2b78e81c275a41be63df6083358070cbefdb59de69bd2f99c003e8a06d90130a30186182cf500f500f5021a52744703030307d90130a2018280f40300081a7441f35c09684b657973746f6e65d9012fa702f403582102f3b97cf3f3387e2c4d8c7141a21529a90e0585e9f032706798d18f988a56e3f1045820ac31dee4dd3f4632f984e0a41e8728edc3ec67f614c8f03181490c8945d19d7406d90130a301861831f500f500f5021a52744703030307d90130a2018280f40300081a59fcb26509684b657973746f6e65036e4b657973746f6e6520332050726f047828626635353461326136316463336362303366656362303031333335323563353561356138316433310565312e312e30")
    }

    #[test]
    fn test_generate_with_ton_keys() {
        let mfp = [18, 80, 182, 188];
        let serial_number = "DVT230712345";
        let device_type = "Keystone 3 Pro";
        let device_version = "1.7.0";

        let keys = vec![
            ExtendedPublicKey::new(DerivationPath::from_str("m/44'/60'/0'").unwrap(), hex::decode("0488b21e0363c5ea6780000000bf103cfabcab325a7463ab5c432a4042109279d6c3531c8c5ea712684cf31f3703c4ad25226a98764f7b753b6c763be112c940182e0b6698944c9def253cec4676").unwrap()),
            ExtendedPublicKey::new(DerivationPath::from_str("m/44'/60'/1'").unwrap(), hex::decode("0488b21e0363c5ea678000000198a39a788ccbb42873c3301841cf2f1b9cec097825b7723c3613411b5edfe36e02d89a27cf3e1e256e37f1c8eb5c80825c064b499443eae32469b6758686c6faaf").unwrap()),
            ExtendedPublicKey::new(DerivationPath::from_str("m/44'/60'/2'").unwrap(), hex::decode("0488b21e0363c5ea67800000029ccc3e21cc93df5a845c786733e19bac47e4b862642e53772b314a01161b3b8b022ec57c196ad963bc52fa7ce43050ec3d9316a537d97507744cf44831aa7b08fd").unwrap()),
            ExtendedPublicKey::new(DerivationPath::from_str("m/44'/60'/3'").unwrap(), hex::decode("0488b21e0363c5ea67800000039993b525a98e702cf95de593f8d2fe86a4cadb5a51aa0c1391887e20353ef42e03f2bd77ec2cf50e0746591beac19a949166b91619a628bd3893ff6f09605d2dea").unwrap()),
            ExtendedPublicKey::new(DerivationPath::from_str("m/44'/60'/4'").unwrap(), hex::decode("0488b21e0363c5ea6780000004824a734567f27f87812bcc0beda329ae998e1c5fcf559b32418d995d30c17d6a02a7ce81a54633ef9e853214d1cf915e2889730fa691ac8da9e58dbbc70dca52c9").unwrap()),
            ExtendedPublicKey::new(DerivationPath::from_str("m/44'/60'/5'").unwrap(), hex::decode("0488b21e0363c5ea6780000005799bffbf6bf0b74ffb899096528c523e9c12caa1f96183779ded0292224fbed602c074419d25973cf7fce728691f8c59face7a92d87288d7319ac0c1a35d9c11ef").unwrap()),
            ExtendedPublicKey::new(DerivationPath::from_str("m/44'/60'/6'").unwrap(), hex::decode("0488b21e0363c5ea67800000060bb4adaeee1876fd5e828d6ea83cc19833067e7a10eea23360f2fcc76b7009a602f18e29894166aa95184993c88174dd1f62b5c16dfbbc5bdc9a62e476c2af5216").unwrap()),
            ExtendedPublicKey::new(DerivationPath::from_str("m/44'/60'/7'").unwrap(), hex::decode("0488b21e0363c5ea6780000007a1781270805f0361c3bc96a11b333c7e996d3a4e0081e649884a2eeb9ab4af1603706361f5ced38b85aae5defcd63b62667eca3f7a7cca1cc1b41ef454675c820a").unwrap()),
            ExtendedPublicKey::new(DerivationPath::from_str("m/44'/60'/8'").unwrap(), hex::decode("0488b21e0363c5ea6780000008ebdb2a570770fbe2feb042b43fc80472209e67051eec4884f41f4ea0bf5488fe026f84908e600a4c4830228fe98e05ca8a648d819d82b957377a232742709fd6ce").unwrap()),
            ExtendedPublicKey::new(DerivationPath::from_str("m/44'/60'/9'").unwrap(), hex::decode("0488b21e0363c5ea67800000092f8a054f02f61f09ea86a8e66fcadb95a52a27c7cfe17cecdf4a97c5df80a3c103fbd0987d1f47906476ac432d2a0c91563cf2d30bd31f4593d87cc0543afa55dd").unwrap()),
            ExtendedPublicKey::new(DerivationPath::from_str("m/44'/0'/0'").unwrap(), hex::decode("0488b21e03b5d6f4508000000051872c7bb8270a7655ae15d738bb21379962ff11fb048b55630395afcfef44280246696468fa5c46647bdd186f568983e2a26773dc5b4c174790fee7ef9b9a7c99").unwrap()),
            ExtendedPublicKey::new(DerivationPath::from_str("m/49'/0'/0'").unwrap(), hex::decode("0488b21e03197077c880000000bad8f929afd65c4b3ff78db1a386062374f2d2e6552e9fdc59aa7dd9b5db6e2c03ab6a1908b437179b914dc3cf4b829d7387adf60fdee483e9c748d5ee79ba4be4").unwrap()),
            ExtendedPublicKey::new(DerivationPath::from_str("m/84'/0'/0'").unwrap(), hex::decode("0488b21e039f6b7e4b80000000de47dd17679605b84abf439bdf5477c6cb94ff5a541d3cb1734afebfa3d3fdb90262ba42544acc4223c74511b37bae54de22565f25f8034f5af8aa10a04c3621b9").unwrap()),
            ExtendedPublicKey::new(DerivationPath::from_str("m/86'/0'/0'").unwrap(), hex::decode("0488b21e030484004d80000000a75805b89c5079ba74e9002c3e587f7c97ea8018ab1d5245638a45de3c845e0b03e0db6ca7c71e72a40091141b9dc4596ab3a9192ac41eebe1f2b048c654423ecb").unwrap()),
        ];

        let account =
            generate_crypto_multi_accounts(mfp, serial_number, keys, device_type, device_version)
                .unwrap();
        let cbor: Vec<u8> = account.try_into().unwrap();
        assert_eq!(hex::encode(cbor), "a5011a1250b6bc0290d9012fa802f403582103c4ad25226a98764f7b753b6c763be112c940182e0b6698944c9def253cec4676045820bf103cfabcab325a7463ab5c432a4042109279d6c3531c8c5ea712684cf31f3706d90130a30186182cf5183cf500f5021a1250b6bc030307d90130a2018400f480f40300081a63c5ea6709684b657973746f6e650a706163636f756e742e7374616e64617264d9012fa602f403582103346d8362d8b3c348cc821abe96e79bbc005af54567d0742c9052215322eaa08a06d90130a30188182cf5183cf500f500f4021a1250b6bc0303081a6e7c9e1809684b657973746f6e650a756163636f756e742e6c65646765725f6c6567616379d9012fa602f403582103de2f443b5070e839d6a209ed1afaadfd43daf7bc610a1fefef201a3aca5350ee06d90130a3018a182cf5183cf500f500f400f4021a1250b6bc0303081a769cd41a09684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f403582103a96bae34b46e82eee9c5beae52e446693b5c2e365714b223adea7caae47e1b7f06d90130a3018a182cf5183cf501f500f400f4021a1250b6bc0303081a998dec6209684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f4035821029299ea371a1c6cdcc75b91d26322a2dbdfedb58285e33760a2af82cfee1a7a6e06d90130a3018a182cf5183cf502f500f400f4021a1250b6bc0303081ac49452d709684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f4035821030f1808f972aed2b8e01d92b09b9c0d55b994592fb5599557e0d4828705204a4106d90130a3018a182cf5183cf503f500f400f4021a1250b6bc0303081a0823c44c09684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f4035821021c885e79f19ba2658755952801366f00f5304db8597eb72a78a6886d8175037d06d90130a3018a182cf5183cf504f500f400f4021a1250b6bc0303081aef4e5bae09684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f4035821037653d3a64a3d1fce16d5693e044e9aea644082371f4d31f6eb83096b5b84d97b06d90130a3018a182cf5183cf505f500f400f4021a1250b6bc0303081a364c571b09684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f403582103ded46d3bc515112c1c99b5f5b5f6c4abc76cc40d98ba4c1808c97038fe976ff606d90130a3018a182cf5183cf506f500f400f4021a1250b6bc0303081a456759fc09684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f403582102447d94e1937819a79a0ef56cd7b7a29de7c80a45b34480ef0b593083e418557906d90130a3018a182cf5183cf507f500f400f4021a1250b6bc0303081a09f362d709684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f403582103d94fb59028252ab9353ee376e868cfd59e9d0bc24ef35fe1b06310cd0e4e908b06d90130a3018a182cf5183cf508f500f400f4021a1250b6bc0303081a4e09767009684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f4035821034d53b1c2630e12b853b96d70fb671363d5d4ec9406f722b4deea1f2a9eb6e76406d90130a3018a182cf5183cf509f500f400f4021a1250b6bc0303081ac418564209684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa702f40358210246696468fa5c46647bdd186f568983e2a26773dc5b4c174790fee7ef9b9a7c9904582051872c7bb8270a7655ae15d738bb21379962ff11fb048b55630395afcfef442806d90130a30186182cf500f500f5021a1250b6bc030307d90130a2018280f40300081ab5d6f45009684b657973746f6e65d9012fa702f403582103ab6a1908b437179b914dc3cf4b829d7387adf60fdee483e9c748d5ee79ba4be4045820bad8f929afd65c4b3ff78db1a386062374f2d2e6552e9fdc59aa7dd9b5db6e2c06d90130a301861831f500f500f5021a1250b6bc030307d90130a2018280f40300081a197077c809684b657973746f6e65d9012fa702f40358210262ba42544acc4223c74511b37bae54de22565f25f8034f5af8aa10a04c3621b9045820de47dd17679605b84abf439bdf5477c6cb94ff5a541d3cb1734afebfa3d3fdb906d90130a301861854f500f500f5021a1250b6bc030307d90130a2018280f40300081a9f6b7e4b09684b657973746f6e65d9012fa702f403582103e0db6ca7c71e72a40091141b9dc4596ab3a9192ac41eebe1f2b048c654423ecb045820a75805b89c5079ba74e9002c3e587f7c97ea8018ab1d5245638a45de3c845e0b06d90130a301861856f500f500f5021a1250b6bc030307d90130a2018280f40300081a0484004d09684b657973746f6e65036e4b657973746f6e6520332050726f047828626339633466373135376165363937643433346163383536323332306564353335653737373638300565312e372e30");
    }
}
