use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;
use third_party::hex;
use third_party::ur_registry::bytes::Bytes;
use third_party::ur_registry::error::URResult;
use third_party::ur_registry::pb;
use third_party::ur_registry::pb::protoc;
use third_party::ur_registry::pb::protoc::base::Content;
use third_party::ur_registry::pb::protoc::{payload, Account, Base, Coin, Payload};

use crate::DEVICE_TYPE;

pub const DESCRIPTION: &str = "keystone qrcode";

pub struct AccountConfig {
    pub hd_path: String,
    pub x_pub: String,
    pub address_length: i32,
    pub is_multi_sign: bool,
}

pub struct CoinConfig {
    pub is_active: bool,
    pub coin_code: String,
    pub accounts: Vec<AccountConfig>,
}

pub fn generate_companion_app_sync_ur(
    master_fingerprint: &[u8; 4],
    cold_version: i32,
    coin_configs: Vec<CoinConfig>,
) -> URResult<Bytes> {
    let mut coins: Vec<Coin> = vec![];
    for coin_config in coin_configs {
        coins.push(generate_coin_by_config(coin_config));
    }
    let payload = Payload {
        r#type: payload::Type::Sync.into(),
        xfp: hex::encode(master_fingerprint).to_uppercase(),
        content: Some(payload::Content::Sync(protoc::Sync { coins })),
    };
    let base = Base {
        data: Some(payload),
        version: 1,
        content: Some(Content::ColdVersion(cold_version)),
        description: DESCRIPTION.to_string(),
        device_type: DEVICE_TYPE.to_string(),
    };
    let encode_result = pb::protobuf_parser::serialize_protobuf(base);
    let zip_data = pb::protobuf_parser::zip(&encode_result)?;
    Ok(Bytes::new(zip_data))
}

fn generate_coin_by_config(coin_config: CoinConfig) -> Coin {
    let mut accounts: Vec<Account> = vec![];
    for account in coin_config.accounts {
        accounts.push(Account {
            hd_path: account.hd_path,
            x_pub: account.x_pub,
            address_length: account.address_length,
            is_multi_sign: account.is_multi_sign,
        })
    }
    Coin {
        coin_code: coin_config.coin_code,
        active: coin_config.is_active,
        accounts,
    }
}

#[cfg(test)]
mod tests {
    extern crate std;

    use super::AccountConfig;
    use super::CoinConfig;
    use alloc::string::ToString;
    use alloc::vec::Vec;
    use third_party::hex;
    use third_party::hex::FromHex;

    use super::*;

    const BTC_COIN_CODE: &'static str = "BTC";
    const BTC_PATH: &'static str = "M/49'/0'/0'";

    const BTC_LEGACY_COIN_CODE: &'static str = "BTC_LEGACY";
    const BTC_LEGACY_PATH: &'static str = "M/44'/0'/0'";

    const BTC_NATIVE_SEGWIT_COIN_CODE: &'static str = "BTC_NATIVE_SEGWIT";
    const BTC_NATIVE_SEGWIT_PATH: &'static str = "M/84'/0'/0'";

    #[test]
    fn test_generate_crypto_account() {
        let mfp = "73C5DA0A";
        let mfp = Vec::from_hex(mfp).unwrap();
        let mfp: [u8; 4] = mfp.try_into().unwrap();

        let btc = CoinConfig {
            coin_code: BTC_COIN_CODE.to_string(),
            is_active: true,
            accounts: vec![AccountConfig {
                hd_path: BTC_PATH.to_string(),
                x_pub: "xpub6C6nQwHaWbSrzs5tZ1q7m5R9cPK9eYpNMFesiXsYrgc1P8bvLLAet9JfHjYXKjToD8cBRswJXXbbFpXgwsswVPAZzKMa1jUp2kVkGVUaJa7".to_string(),
                address_length: 20,
                is_multi_sign: false,
            }],
        };
        let btc_legacy = CoinConfig {
            coin_code: BTC_LEGACY_COIN_CODE.to_string(),
            is_active: true,
            accounts: vec![AccountConfig {
                hd_path: BTC_LEGACY_PATH.to_string(),
                x_pub: "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYJVVhhawA7d4R5WSWGFNbi8Aw6ZRc1brxMyWMzG3DSSSSoekkudhUd9yLb6qx39T9nMdj".to_string(),
                address_length: 20,
                is_multi_sign: false,
            }],
        };

        let btc_native_segwit = CoinConfig {
            coin_code: BTC_NATIVE_SEGWIT_COIN_CODE.to_string(),
            is_active: true,
            accounts: vec![AccountConfig {
                hd_path: BTC_NATIVE_SEGWIT_PATH.to_string(),
                x_pub: "xpub6CatWdiZiodmUeTDp8LT5or8nmbKNcuyvz7WyksVFkKB4RHwCD3XyuvPEbvqAQY3rAPshWcMLoP2fMFMKHPJ4ZeZXYVUhLv1VMrjPC7PW6V".to_string(),
                address_length: 20,
                is_multi_sign: false,
            }],
        };
        let result =
            generate_companion_app_sync_ur(&mfp, 31000, vec![btc, btc_legacy, btc_native_segwit])
                .unwrap();

        assert_eq!("1f8b08000000000000035dd1376edb60000560105a0c0648313c189abc259b2d516c238b48990d14cbcfb2186c320bc4f68ba4c82973b66c3e4c6e90dbe404a180c406fcf0a6377dc0bb42ae3f15c9084f5599dc356d54c5c9f2f7625eaf488cc379e68159fe5aa03f1074c15adc6764f91d413fa8f71bfaebfdc3a5d7d5b9ee428223cafdb00b9cd06c27889ffc55431e71838e74994ebc5a538504662ef4dae768a55361af284c72a2a5c32ef75c39b72a9e8a58030e92eb86a150bbcf038403d0197f92d56095dbf5ba008508ec400ac8db1bf42782a233e649d98a0ce7bd993617d0dc7f26b68207aecc0ed3391286d69cf659d3ad791658302d3837887a0d3aa2270190a6c1c090f1c6c01dd311052dcc2866207c235a85ed591d1d751231de9c532545d1c5a91dd3a31212cd19a32dba54e37c36bd20e8978b4963ac47b07d32b7a2f368bdd2a877342e383971e667557cb4138baf29c5c2ab962a8fa1ac45ddd84fa43316100885cc6e8cddc0f1983b76bdbe0dfb86d97b58cbe830752255a9f4f541155479a74b1b3ff15d0fd8a9d2af80dae63a47ea0e016e6fbebdfc41d61fe5ff1763777a5bfd053ac4f633f5010000",
        hex::encode(result.get_bytes()));
    }
}
