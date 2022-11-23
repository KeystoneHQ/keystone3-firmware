use alloc::string::String;
use alloc::vec;
use alloc::vec::Vec;
use core::str::FromStr;
use third_party::bitcoin::bip32;
use third_party::ur_registry::crypto_account::CryptoAccount;
use third_party::ur_registry::crypto_coin_info::{CoinType, CryptoCoinInfo, Network};
use third_party::ur_registry::crypto_hd_key::CryptoHDKey;
use third_party::ur_registry::crypto_key_path::{CryptoKeyPath, PathComponent};
use third_party::ur_registry::crypto_output::CryptoOutput;
use third_party::ur_registry::error::{URError, URResult};

const ETH_PURPOSE: u32 = 44;
const ETH_COIN_TYPE: u32 = 60;
const KEY_NAME: &str = "Keystone";

pub enum ETHAccountTypeApp {
    Bip44Standard,
    LedgerLive,
    LedgerLegacy,
}

impl ETHAccountTypeApp {
    pub fn to_i32(&self) -> i32 {
        match self {
            ETHAccountTypeApp::Bip44Standard => 0,
            ETHAccountTypeApp::LedgerLive => 1,
            ETHAccountTypeApp::LedgerLegacy => 2,
        }
    }
}

pub fn generate_standard_legacy_hd_key(
    master_fingerprint: &[u8; 4],
    extended_public_key: &str,
    account_type: ETHAccountTypeApp,
) -> URResult<CryptoHDKey> {
    let bip32_extended_pub_key = bip32::ExtendedPubKey::from_str(extended_public_key).unwrap();
    let parent_fingerprint = bip32_extended_pub_key.parent_fingerprint;

    let coin_info = CryptoCoinInfo::new(Some(CoinType::Ethereum), Some(Network::MainNet));

    let origin = CryptoKeyPath::new(
        vec![
            get_path_component(Some(ETH_PURPOSE), true)?,
            get_path_component(Some(ETH_COIN_TYPE), true)?,
            get_path_component(Some(0), true)?,
        ],
        Some(master_fingerprint.clone()),
        Some(bip32_extended_pub_key.depth as u32),
    );

    let children = CryptoKeyPath::new(
        match account_type {
            ETHAccountTypeApp::Bip44Standard => {
                vec![
                    get_path_component(Some(0), false)?,
                    get_path_component(None, false)?,
                ]
            }
            _ => vec![get_path_component(None, false)?],
        },
        None,
        Some(0),
    );
    let hd_key: CryptoHDKey = CryptoHDKey::new_extended_key(
        Some(false),
        Vec::from(bip32_extended_pub_key.public_key.serialize()),
        Some(bip32_extended_pub_key.chain_code[..].to_vec()),
        Some(coin_info),
        Some(origin),
        Some(children),
        Some(parent_fingerprint.to_bytes()),
        Some(String::from(KEY_NAME)),
        match account_type {
            ETHAccountTypeApp::Bip44Standard => Some(String::from("account.standard")),
            ETHAccountTypeApp::LedgerLegacy => Some(String::from("account.ledger_legacy")),
            _ => None,
        },
    );

    Ok(hd_key)
}

pub fn generate_ledger_live_account(
    master_fingerprint: &[u8; 4],
    extended_public_keys: &Vec<String>,
) -> URResult<CryptoAccount> {
    let mut outputs: Vec<CryptoOutput> = Vec::new();
    for (i, x) in extended_public_keys.iter().enumerate() {
        let output = generate_ledger_live_crypto_output(master_fingerprint, x.as_str(), i as u32)?;
        outputs.push(output);
    }
    Ok(CryptoAccount::new(master_fingerprint.clone(), outputs))
}

fn generate_ledger_live_crypto_output(
    master_fingerprint: &[u8; 4],
    extended_public_key: &str,
    account_index: u32,
) -> URResult<CryptoOutput> {
    let script_expressions = vec![];
    Ok(CryptoOutput::new(
        script_expressions,
        None,
        Some(generate_ledger_live_hd_key(
            master_fingerprint,
            extended_public_key,
            account_index,
        )?),
        None,
    ))
}

pub fn generate_ledger_live_hd_key(
    master_fingerprint: &[u8; 4],
    extended_public_key: &str,
    account_index: u32,
) -> URResult<CryptoHDKey> {
    let bip32_extended_pub_key: bip32::ExtendedPubKey =
        bip32::ExtendedPubKey::from_str(extended_public_key).unwrap();

    let origin = CryptoKeyPath::new(
        vec![
            get_path_component(Some(ETH_PURPOSE), true)?,
            get_path_component(Some(ETH_COIN_TYPE), true)?,
            get_path_component(Some(account_index), true)?,
            get_path_component(Some(0), false)?,
            get_path_component(Some(0), false)?,
        ],
        Some(master_fingerprint.clone()),
        Some(bip32_extended_pub_key.depth as u32),
    );

    let hd_key = CryptoHDKey::new_extended_key(
        Some(false),
        Vec::from(bip32_extended_pub_key.public_key.serialize()),
        None,
        None,
        Some(origin),
        None,
        None,
        Some(String::from(KEY_NAME)),
        Some(String::from("account.ledger_live")),
    );

    Ok(hd_key)
}

fn get_path_component(index: Option<u32>, hardened: bool) -> URResult<PathComponent> {
    PathComponent::new(index, hardened).map_err(|e| URError::CborEncodeError(e))
}

#[cfg(test)]
mod tests {
    extern crate std;

    use crate::metamask::generate_ledger_live_account;
    use alloc::string::ToString;
    use alloc::vec;
    use alloc::vec::Vec;
    use third_party::hex;
    use third_party::hex::FromHex;

    #[test]
    fn test_ledger_live() {
        let mfp = "757E6FC9";
        let mfp = Vec::from_hex(mfp).unwrap();
        let mfp: [u8; 4] = mfp.try_into().unwrap();

        let x_pub_1 = "xpub6C8zKiZZ8V75XynjThhvdjy7hbnJHAFkhW7jL9EvBCsRFSRov4sXUJATU6CqUF9BxAbryiU3eghdHDLbwgF8ASE4AwHTzkLHaHsbwiCnkHc".to_string();
        let x_pub_2 = "xpub6C8zKiZZ8V75aXTgYAswqCgYUHeBYg1663a4Ri6zdJ4GW58r67Kmj4Fr8pbBK9usq45o8iQ8dBM75o67ct1M38yeb6RhFTWBxiYeq8Kg84Z".to_string();
        let x_pub_3 = "xpub6C8zKiZZ8V75e1B4yeB8hpw9itcx9aKWvxfWH5jmX9xf1YeqaUM1zotxXg2pX8Jin4NUS7L1KKGTUuuCr1kMf2ox6c5ebQ14XTZa9seWuCW".to_string();
        let x_pub_4 = "xpub6C8zKiZZ8V75gaav5YzPCb19oSARjXNWHrBRtoQA7Fx7uMSXSnkgCrwviTPqHswrP5JSxzMshS2jetwo7V9sR3mbjMqr3bukHy3She7adEX".to_string();
        let x_pub_5 = "xpub6C8zKiZZ8V75hek8cS7byAQtfQBWov8osxA4bREa8JLvXBkmynFd8fttUVX723ZCHvVaYnJMgQsE35E7JBQr5eqYSwYwGBYa2cjFXYi9Z6u".to_string();
        let x_pub_6 = "xpub6C8zKiZZ8V75nabUFPJEspYfLqsJRqbmQ5LBvUy8nDRzj9SejBCGCKbAT127LQjKQVoFasayzam5ozijG3f2kCf7uZQrPBQG4zAfSFJ6c2Z".to_string();
        let x_pub_7 = "xpub6C8zKiZZ8V75phnqeXtW6Qm8duCmHXUE18PkYHHJqmCPmcf5iEdqLPfSVNsPKm7HsUGPorG5KSxmWc3nAkfbhmWN7PuqFzpYbmHocAiqMH1".to_string();
        let x_pub_8 = "xpub6C8zKiZZ8V75qcUxEpYCXebRF23VMcJqcWdCSvZjcfMWYU4dcQnKZ8LiXPXBwYioYc62wC6F1B6UhnWYtX1Ss9ZfT3dC6e63Bzfq4AHULQh".to_string();
        let x_pub_9 = "xpub6C8zKiZZ8V75tgxbNbnWfAbC63k1s6tQvfpYLNLaEzq3aUhhZu798JUyMJVL3fNK2CcF2vKauyBdF73TxjLyfU9Cfb4SLKxLDr3SUVVSAQL".to_string();
        let x_pub_10 = "xpub6C8zKiZZ8V75xAiMExpbA2YBTvgC3o5sLkHjv6jm1zdWrAQ2pCq9CUnGQgBiVd2xrUh2yxBSUccXguJgdJ3SdnJvhAD5KGhWa6fjKsyt8Wy".to_string();

        let keys = vec![
            x_pub_1, x_pub_2, x_pub_3, x_pub_4, x_pub_5, x_pub_6, x_pub_7, x_pub_8, x_pub_9,
            x_pub_10,
        ];

        let result = generate_ledger_live_account(&mfp, &keys).unwrap();
        let cbor: Vec<u8> = result.try_into().unwrap();

        assert_eq!(hex::encode(cbor).to_lowercase(),
                   "a2011a757e6fc9028ad9012fa502f40358210326bd25d39a5eeb0217c4508bc5c50d28087121372daa5d2e48b30a8f3345998606d90130a3018a182cf5183cf500f500f400f4021a757e6fc9030309684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa502f4035821030b235be63214e20961295fb82ff3fe365c21e3c5c203bc7a995b5f31fa10a86506d90130a3018a182cf5183cf501f500f400f4021a757e6fc9030309684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa502f4035821020a48e423f5991eba82dc251aa4b7191bb4bf72838ff61e761fe1e08f374968e706d90130a3018a182cf5183cf502f500f400f4021a757e6fc9030309684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa502f403582103c2d7cb8f289f4211064258c63c86a57d22eb4e2565fa1c9b203797251a1fad9d06d90130a3018a182cf5183cf503f500f400f4021a757e6fc9030309684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa502f403582103435a76cc11055f740187fbfe68a7a1f0180fe43f6835bd52797bb50c6e567a2506d90130a3018a182cf5183cf504f500f400f4021a757e6fc9030309684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa502f403582102f41789edb56ce2786392d4fb040565ac767be3e43355610a91853e65212a02a106d90130a3018a182cf5183cf505f500f400f4021a757e6fc9030309684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa502f4035821038f4e60d5035d28a76d1b9a64dd7867b11fb652a56605a0e51923b4f49e23522a06d90130a3018a182cf5183cf506f500f400f4021a757e6fc9030309684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa502f403582102765303723f047bac13ff1a9617c0891fd23ebcf7c7053d4c2d1dc49f24c9e0d906d90130a3018a182cf5183cf507f500f400f4021a757e6fc9030309684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa502f403582103ac7982290292097e2d072b5520911a4c94af713ae49cdd8ab6fcd1895c194ef306d90130a3018a182cf5183cf508f500f400f4021a757e6fc9030309684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa502f4035821039fcc0a270027ee3d8ad2b41f172d03aa8f5353a9876301e0799f845bda6f1f3506d90130a3018a182cf5183cf509f500f400f4021a757e6fc9030309684b657973746f6e650a736163636f756e742e6c65646765725f6c697665");
    }
}
