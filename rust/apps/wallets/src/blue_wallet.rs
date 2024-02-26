use core::str::FromStr;

use alloc::string::ToString;
use alloc::vec;
use alloc::vec::Vec;
use third_party::bitcoin::bip32;
use third_party::ur_registry::crypto_account::CryptoAccount;
use third_party::ur_registry::crypto_coin_info::{CoinType, CryptoCoinInfo, Network};
use third_party::ur_registry::crypto_hd_key::CryptoHDKey;
use third_party::ur_registry::crypto_key_path::{CryptoKeyPath, PathComponent};
use third_party::ur_registry::crypto_output::CryptoOutput;
use third_party::ur_registry::error::{URError, URResult};
use third_party::ur_registry::script_expression::ScriptExpression;

const PURPOSE_LEGACY: u32 = 44;
const PURPOSE_NESTED_SEGWIT: u32 = 49;
const PURPOSE_NATIVE_SEGWIT: u32 = 84;
const PURPOSE_TAPROOT: u32 = 86;

pub fn generate_crypto_account(
    master_fingerprint: &[u8; 4],
    extended_public_keys: &[&str],
) -> URResult<CryptoAccount> {
    let mut outputs = vec![];
    outputs.extend([
        generate_native_segwit_output(master_fingerprint, extended_public_keys[0])?,
        generate_legacy_output(master_fingerprint, extended_public_keys[1])?,
        generate_nested_segwit_output(master_fingerprint, extended_public_keys[2])?,
    ]);
    // In the previous step of the call, the length has been checked and it won't be less than 3.
    let has_taproot_path = extended_public_keys.len() == 4;
    if has_taproot_path {
        outputs.extend([generate_taproot_output(
            master_fingerprint,
            extended_public_keys[3],
        )?]);
    }
    Ok(CryptoAccount::new(master_fingerprint.clone(), outputs))
}

fn generate_taproot_output(
    master_fingerprint: &[u8; 4],
    extended_public_key: &str,
) -> URResult<CryptoOutput> {
    let script_expressions = vec![ScriptExpression::Taproot];
    Ok(CryptoOutput::new(
        script_expressions,
        None,
        Some(generate_crypto_hd_key(
            master_fingerprint,
            extended_public_key,
            PURPOSE_TAPROOT,
        )?),
        None,
    ))
}

fn generate_legacy_output(
    master_fingerprint: &[u8; 4],
    extended_public_key: &str,
) -> URResult<CryptoOutput> {
    let script_expressions = vec![ScriptExpression::PublicKeyHash];
    Ok(CryptoOutput::new(
        script_expressions,
        None,
        Some(generate_crypto_hd_key(
            master_fingerprint,
            extended_public_key,
            PURPOSE_LEGACY,
        )?),
        None,
    ))
}

fn generate_nested_segwit_output(
    master_fingerprint: &[u8; 4],
    extended_public_key: &str,
) -> URResult<CryptoOutput> {
    let script_expressions = vec![
        ScriptExpression::ScriptHash,
        ScriptExpression::WitnessPublicKeyHash,
    ];
    Ok(CryptoOutput::new(
        script_expressions,
        None,
        Some(generate_crypto_hd_key(
            master_fingerprint,
            extended_public_key,
            PURPOSE_NESTED_SEGWIT,
        )?),
        None,
    ))
}

fn generate_native_segwit_output(
    master_fingerprint: &[u8; 4],
    extended_public_key: &str,
) -> URResult<CryptoOutput> {
    let script_expressions = vec![ScriptExpression::WitnessPublicKeyHash];
    Ok(CryptoOutput::new(
        script_expressions,
        None,
        Some(generate_crypto_hd_key(
            master_fingerprint,
            extended_public_key,
            PURPOSE_NATIVE_SEGWIT,
        )?),
        None,
    ))
}

fn generate_crypto_hd_key(
    master_fingerprint: &[u8; 4],
    extended_public_key: &str,
    purpose: u32,
) -> URResult<CryptoHDKey> {
    let bip32_extended_pub_key = bip32::Xpub::from_str(extended_public_key)
        .map_err(|e| URError::UrEncodeError(e.to_string()))?;
    let parent_fingerprint = bip32_extended_pub_key.parent_fingerprint;

    let coin_info = CryptoCoinInfo::new(Some(CoinType::Bitcoin), Some(Network::MainNet));

    let origin = CryptoKeyPath::new(
        vec![
            get_path_component(Some(purpose), true)?,
            get_path_component(Some(0), true)?,
            get_path_component(Some(0), true)?,
        ],
        Some(master_fingerprint.clone()),
        Some(bip32_extended_pub_key.depth as u32),
    );

    let children = CryptoKeyPath::new(
        vec![
            get_path_component(Some(0), false)?,
            get_path_component(None, false)?,
        ],
        None,
        Some(0),
    );
    let hd_key = CryptoHDKey::new_extended_key(
        Some(false),
        Vec::from(bip32_extended_pub_key.public_key.serialize()),
        Some(bip32_extended_pub_key.chain_code[..].to_vec()),
        Some(coin_info),
        Some(origin),
        Some(children),
        Some(parent_fingerprint.to_bytes()),
        None,
        None,
    );

    Ok(hd_key)
}

fn get_path_component(index: Option<u32>, hardened: bool) -> URResult<PathComponent> {
    PathComponent::new(index, hardened).map_err(|e| URError::CborEncodeError(e))
}

#[cfg(test)]
mod tests {
    extern crate std;

    use crate::blue_wallet::generate_crypto_account;
    use alloc::vec::Vec;

    use third_party::hex;
    use third_party::hex::FromHex;

    #[test]
    fn test_generate_crypto_account() {
        let mfp = "73C5DA0A";
        let mfp = Vec::from_hex(mfp).unwrap();
        let mfp: [u8; 4] = mfp.try_into().unwrap();

        let x_pub_1 = "xpub6CatWdiZiodmUeTDp8LT5or8nmbKNcuyvz7WyksVFkKB4RHwCD3XyuvPEbvqAQY3rAPshWcMLoP2fMFMKHPJ4ZeZXYVUhLv1VMrjPC7PW6V";
        let x_pub_2 = "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYJVVhhawA7d4R5WSWGFNbi8Aw6ZRc1brxMyWMzG3DSSSSoekkudhUd9yLb6qx39T9nMdj";
        let x_pub_3 = "xpub6C6nQwHaWbSrzs5tZ1q7m5R9cPK9eYpNMFesiXsYrgc1P8bvLLAet9JfHjYXKjToD8cBRswJXXbbFpXgwsswVPAZzKMa1jUp2kVkGVUaJa7";
        let x_pub_4 = "xpub6BgBgsespWvERF3LHQu6CnqdvfEvtMcQjYrcRzx53QJjSxarj2afYWcLteoGVky7D3UKDP9QyrLprQ3VCECoY49yfdDEHGCtMMj92pReUsQ";

        let account = generate_crypto_account(&mfp, &[x_pub_1, x_pub_2, x_pub_3, x_pub_4]).unwrap();
        let cbor: Vec<u8> = account.try_into().unwrap();

        assert_eq!("a2011a73c5da0a0284d90194d9012fa702f403582102707a62fdacc26ea9b63b1c197906f56ee0180d0bcf1966e1a2da34f5f3a09a9b0458204a53a0ab21b9dc95869c4e92a161194e03c0ef3ff5014ac692f433c4765490fc05d90131a20100020006d90130a301861854f500f500f5021a73c5da0a030307d90130a2018400f480f40300081a7ef32bdbd90193d9012fa702f403582103774c910fcf07fa96886ea794f0d5caed9afe30b44b83f7e213bb92930e7df4bd0458203da4bc190a2680111d31fadfdc905f2a7f6ce77c6f109919116f253d4344521905d90131a20100020006d90130a30186182cf500f500f5021a73c5da0a030307d90130a2018400f480f40300081a155bca59d90190d90194d9012fa702f403582102f1f347891b20f7568eae3ec9869fbfb67bcab6f358326f10ecc42356bd55939d0458206eaae365ae0e0a0aab84325cfe7cd76c3b909035f889e7d3f1b847a9a0797ecb05d90131a20100020006d90130a301861831f500f500f5021a73c5da0a030307d90130a2018400f480f40300081a3d05ff75d90199d9012fa702f403582103418278a2885c8bb98148158d1474634097a179c642f23cf1cc04da629ac6f0fb045820c61a8f27e98182314d2444da3e600eb5836ec8ad183c86c311f95df8082b18aa05d90131a20100020006d90130a301861856f500f500f5021a73c5da0a030307d90130a2018400f480f40300081a035270da",
                   hex::encode(cbor).to_lowercase()
        );
    }
}
