use alloc::{
    string::{String, ToString},
    vec::Vec,
};

use third_party::ur_registry::bytes::Bytes;
use third_party::{
    bitcoin::bip32::ChildNumber,
    ur_registry::{
        crypto_hd_key::CryptoHDKey,
        crypto_key_path::{CryptoKeyPath, PathComponent},
        error::{URError, URResult},
        extend::crypto_multi_accounts::CryptoMultiAccounts,
    },
};

use crate::companion_app::{AccountConfig, CoinConfig};
use crate::{common::get_path_component, ExtendedPublicKey};

fn get_device_id(serial_number: &str) -> String {
    use third_party::cryptoxide::hashing::sha256;
    third_party::hex::encode(&sha256(&sha256(serial_number.as_bytes()))[0..20])
}

const BTC_LEGACY_PREFIX: &str = "m/44'/0'/0'";
const BTC_SEGWIT_PREFIX: &str = "m/49'/0'/0'";
const BTC_NATIVE_SEGWIT_PREFIX: &str = "m/84'/0'/0'";
const ETH_STANDARD_PREFIX: &str = "m/44'/60'/0'";
const BCH_PREFIX: &str = "m/44'/145'/0'";
const DASH_PREFIX: &str = "m/44'/5'/0'";
const LTC_PREFIX: &str = "m/49'/2'/0'";
const TRX_PREFIX: &str = "m/44'/195'/0'";
const XRP_PREFIX: &str = "m/44'/144'/0'";

const COLD_WALLET_VERSION: i32 = 31206;
fn path_to_coin_code(path: &str) -> String {
    let path = path
        .split('/')
        .take(4)
        .collect::<Vec<&str>>()
        .join("/")
        .to_lowercase();
    rust_tools::debug!(format!("path: {}", path));
    match path.as_str() {
        BTC_LEGACY_PREFIX => "BTC_LEGACY".to_string(),
        BTC_SEGWIT_PREFIX => "BTC".to_string(),
        BTC_NATIVE_SEGWIT_PREFIX => "BTC_NATIVE_SEGWIT".to_string(),
        ETH_STANDARD_PREFIX => "ETH".to_string(),
        BCH_PREFIX => "BCH".to_string(),
        DASH_PREFIX => "DASH".to_string(),
        LTC_PREFIX => "LTC".to_string(),
        TRX_PREFIX => "TRON".to_string(),
        XRP_PREFIX => "XRP".to_string(),
        _ => "Unknown".to_string(),
    }
}

pub fn generate_crypto_multi_accounts(
    master_fingerprint: [u8; 4],
    serial_number: &str,
    extended_public_keys: Vec<ExtendedPublicKey>,
    device_type: &str,
    device_version: &str,
) -> URResult<Bytes> {
    rust_tools::debug!(format!("master_fingerprint: {:?}", &master_fingerprint));
    let device_id = get_device_id(serial_number);
    let mut keys = vec![];
    let k1_keys = vec![
        BTC_LEGACY_PREFIX.to_string(),
        BTC_SEGWIT_PREFIX.to_string(),
        BTC_NATIVE_SEGWIT_PREFIX.to_string(),
        ETH_STANDARD_PREFIX.to_string(),
        BCH_PREFIX.to_string(),
        DASH_PREFIX.to_string(),
        LTC_PREFIX.to_string(),
        TRX_PREFIX.to_string(),
        XRP_PREFIX.to_string(),
    ];
    for ele in extended_public_keys {
        match ele.get_path() {
            _path if k1_keys.contains(&_path.to_string().to_lowercase()) => {
                keys.push(generate_k1_normal_key(
                    master_fingerprint,
                    ele.clone(),
                    None,
                )?);
            }
            _path if _path.to_string().to_lowercase().eq(ETH_STANDARD_PREFIX) => {
                keys.push(generate_k1_normal_key(
                    master_fingerprint,
                    ele.clone(),
                    Some("account.standard".to_string()),
                )?);
            }
            _ => {
                return Err(URError::UrEncodeError(format!(
                    "Unknown key path: {}",
                    ele.path.to_string()
                )))
            }
        }
    }
    let data = CryptoMultiAccounts::new(
        master_fingerprint,
        keys,
        Some(device_type.to_string()),
        Some(device_id),
        Some(device_version.to_string()),
    );

    // convert crypto_multi_accounts to keystone sync ur
    let mut coin_configs: Vec<CoinConfig> = vec![];
    for key in data.get_keys() {
        let mut accounts: Vec<AccountConfig> = vec![];
        let hd_path = "M/".to_string() + &*key.get_origin().unwrap().get_path().unwrap();

        let x_pub = key.get_bip32_key();

        let coin_code = path_to_coin_code(&hd_path);
        rust_tools::debug!(format!(
            "coin_code: {}, hd_path: {}, x_pub: {}",
            coin_code, hd_path, x_pub
        ));
        if coin_code == "Unknown" {
            continue;
        }

        let account = AccountConfig {
            hd_path,
            x_pub,
            address_length: 1,
            is_multi_sign: false,
        };
        accounts.push(account);
        let coin_config = CoinConfig {
            coin_code,
            is_active: true,
            accounts,
        };
        coin_configs.push(coin_config);
    }
    // device version: 1.5.6 == 100000156
    let cold_wallet_device_version_str = device_version.split('.').collect::<Vec<&str>>().join("");
    let cold_wallet_device_version =
        100_000 + cold_wallet_device_version_str.parse::<i32>().unwrap();
    let keystone_sync_ur = crate::companion_app::generate_companion_app_sync_ur(
        &master_fingerprint,
        cold_wallet_device_version,
        coin_configs,
    )?;

    // this below is the expected output , and will send to the keystone solfware
    // let mfp = "1250B6BC";
    // let mfp = Vec::from_hex(mfp).unwrap();
    // let mfp: [u8; 4] = mfp.try_into().unwrap();
    // Base { version: 1, description: "keystone qrcode",
    // data: Some(Payload { r#type: Sync, xfp: "1250B6BC",
    // content: Some(Sync(Sync {
    // coins: [
    // Coin { coin_code: "BTC", active: true, accounts: [Account { hd_path: "M/49'/0'/0'", x_pub: "xpub6BqcZUPst99NuHmP1TNcCXftBBUjf87UAgZYfGYLKnafbhSo9rjoQoVhXDYwcf4Bt7wv2CszAiBXfrhgx5yoJNo8qe1FVQPNyTEtsaWJp52", address_length: 1, is_multi_sign: false }] },
    // Coin { coin_code: "BTC_LEGACY", active: false, accounts: [Account { hd_path: "M/44'/0'/0'", x_pub: "xpub6CzHiXpZKvPYnCwnd6qq511yn8B63yB8rCtxsY4azeuCxPYJn6B7KNEa6sDCkNAj54H5AW3zYY8QLSzG6qe7SZBvKW3ighdPGTYQgDXxfHQ", address_length: 1, is_multi_sign: false }] },
    // Coin { coin_code: "BTC_NATIVE_SEGWIT", active: false, accounts: [Account { hd_path: "M/84'/0'/0'", x_pub: "xpub6CpjN9cV2eSSHvzA5113pRqD5qaWRhUXS7ABs5AqGiau3BbAnW2fx1JwEEwn9ugVAgx6vbpXAKEjQbKjYHHPCjaxHEwyfLcUvwxjbaBEPRe", address_length: 1, is_multi_sign: false }] },
    // Coin { coin_code: "ETH", active: true, accounts: [Account { hd_path: "M/44'/60'/0'", x_pub: "xpub6CPJYBVf9pKYuMRpT7PhAPoCnCwh8KyYXndnFGpQPUM7uzB4n4gRcdbest2t3yFkyquPYqRrVY69dZ68kJWjGspZewyNW8rXG6YYQppUUi9", address_length: 1, is_multi_sign: false }] },
    // Coin { coin_code: "BCH", active: true, accounts: [Account { hd_path: "M/44'/145'/0'", x_pub: "xpub6Ce7XFJkp8aearw9nhdoiuaeuocR9ZuEzSMnR5WKtU99fbXmMgnobXMn35kQJGaF32JZDNS9UtEJ3sonSZ6JVDSuKFU2vRD1qbaE2zyG4V4", address_length: 1, is_multi_sign: false }] },
    // Coin { coin_code: "DASH", active: true, accounts: [Account { hd_path: "M/44'/5'/0'", x_pub: "xpub6CdBLESkJwrjUMoMV6Z3doPTLZSJ9knVp22mGmkJjH74hJ4riU8z9JCJf9APwSgWFpBkC17Are9Z8Sk6qi2XaXNnHUi9A5BbvJLaBh5WwkF", address_length: 1, is_multi_sign: false }] },
    // Coin { coin_code: "LTC", active: true, accounts: [Account { hd_path: "M/49'/2'/0'", x_pub: "xpub6CASR9TUvDwDP4u3RiH6x6bNMgprtwz9KaXzGT6w9HnoAeYDgG2Re88mZePxncSmNSrcLXeCGW3U8cGmiTpPih7PoYDpmonFygdyxDFqZZZ", address_length: 1, is_multi_sign: false }] },
    // Coin { coin_code: "TRON", active: true, accounts: [Account { hd_path: "M/44'/195'/0'", x_pub: "xpub6C5UAFMYy6o6CucPhqniZxZEtBKYS9xY433C9DunttXBwM9sNmnsuCdZFADLgUXsSjjThUYQLKWwHnPraacNJAHRvCe7Q3wrWE1wmw6dvyp", address_length: 1, is_multi_sign: false }] },
    // Coin { coin_code: "XRP", active: true, accounts: [Account { hd_path: "M/44'/144'/0'", x_pub: "xpub6BmXbwNG7wfcEs5VUsBKK5CwJk3Vf8QFKF3wTeutW1zWwStQ7qGZJr9W6KxdBF5pyzju17Hrsus4kjdjvYfu3PB46BsUQY61WQ2d9rP5v7i", address_length: 1, is_multi_sign: false }] },
    // Coin { coin_code: "DOT", active: true, accounts: [Account { hd_path: "//polkadot", x_pub: "xpub68wKPdEKjbZ4o1mSG7eN6b7b6vHXpShMTxJE4emZ98Fbs93cPxrkKxRmLkq7pi937qPyjxvXk4Zn2nUCvuQg6zbRvqQpVzXQyaM8miwcs7E", address_length: 1, is_multi_sign: false }] }] })) }),
    // device_type: "keystone Pro", content: Some(ColdVersion(31206)) }
    Ok(keystone_sync_ur)
}

fn generate_k1_normal_key(
    mfp: [u8; 4],
    key: ExtendedPublicKey,
    note: Option<String>,
) -> URResult<CryptoHDKey> {
    let xpub = third_party::bitcoin::bip32::Xpub::decode(&key.get_key())
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let path = key.get_path();
    let key_path = CryptoKeyPath::new(
        path.into_iter()
            .map(|v| match v {
                ChildNumber::Normal { index } => get_path_component(Some(index.clone()), false),
                ChildNumber::Hardened { index } => get_path_component(Some(index.clone()), true),
            })
            .collect::<URResult<Vec<PathComponent>>>()?,
        Some(mfp),
        Some(xpub.depth as u32),
    );
    Ok(CryptoHDKey::new_extended_key(
        Some(false),
        xpub.public_key.serialize().to_vec(),
        Some(xpub.chain_code.to_bytes().to_vec()),
        None,
        Some(key_path),
        None,
        Some(xpub.parent_fingerprint.to_bytes()),
        Some("Keystone".to_string()),
        note,
    ))
}
