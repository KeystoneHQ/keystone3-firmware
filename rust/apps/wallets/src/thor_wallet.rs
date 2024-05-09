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
    extended_public_keys_path: &[&str],
) -> URResult<CryptoAccount> {
    let mut outputs = vec![];

    for (index, _) in extended_public_keys_path.iter().enumerate() {
        outputs.push(generate_output(
            master_fingerprint,
            extended_public_keys[index],
            extended_public_keys_path[index],
        )?);
    }

    Ok(CryptoAccount::new(master_fingerprint.clone(), outputs))
}

fn get_path_level_number(path: &str, index: usize) -> Option<u32> {
    let segments = path.split('/').collect::<Vec<_>>();

    if index >= segments.len() {
        return None;
    }

    let num_str = segments[index].trim_matches('\'');
    match num_str.parse::<u32>() {
        Ok(num) => Some(num),
        Err(_) => None,
    }
}

fn generate_output(
    master_fingerprint: &[u8; 4],
    extended_public_key: &str,
    extended_public_keys_path: &str,
) -> URResult<CryptoOutput> {
    let purpose = get_path_level_number(extended_public_keys_path, 1)
        .ok_or(URError::UrEncodeError("get purpose err".to_string()))?;
    let coin_type = get_path_level_number(extended_public_keys_path, 2)
        .ok_or(URError::UrEncodeError("get coin_type err".to_string()))?;
    let script_expressions = match purpose {
        PURPOSE_TAPROOT => vec![ScriptExpression::Taproot],
        PURPOSE_LEGACY => vec![ScriptExpression::PublicKeyHash],
        PURPOSE_NESTED_SEGWIT => vec![
            ScriptExpression::ScriptHash,
            ScriptExpression::WitnessPublicKeyHash,
        ],
        PURPOSE_NATIVE_SEGWIT => vec![ScriptExpression::WitnessPublicKeyHash],
        _ => {
            return Err(URError::UrEncodeError(format!(
                "not supported purpose:{}",
                purpose
            )))
        }
    };

    Ok(CryptoOutput::new(
        script_expressions,
        None,
        Some(generate_crypto_hd_key(
            master_fingerprint,
            extended_public_key,
            purpose,
            coin_type,
        )?),
        None,
    ))
}

fn generate_crypto_hd_key(
    master_fingerprint: &[u8; 4],
    extended_public_key: &str,
    purpose: u32,
    coin_type: u32,
) -> URResult<CryptoHDKey> {
    let bip32_extended_pub_key = bip32::Xpub::from_str(extended_public_key)
        .map_err(|e| URError::UrEncodeError(e.to_string()))?;
    let parent_fingerprint = bip32_extended_pub_key.parent_fingerprint;

    let coin_info = CryptoCoinInfo::new(Some(CoinType::Bitcoin), Some(Network::MainNet));

    let origin = CryptoKeyPath::new(
        vec![
            get_path_component(Some(purpose), true)?,
            get_path_component(Some(coin_type), true)?,
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

    use super::*;
    use alloc::vec::Vec;

    use third_party::hex;
    use third_party::hex::FromHex;

    #[test]
    fn test_generate_crypto_account() {
        let mfp = "73C5DA0A";
        let mfp = Vec::from_hex(mfp).unwrap();
        let mfp: [u8; 4] = mfp.try_into().unwrap();

        let x_pub_1 = "xpub6CcBrNAXBhrdb29q4BFApXgKgCdnHevzGnwFKnDSYfWWMcqkbH17ay6vaUJDZxFdZx5y5AdcoEzLfURSdwtQEEZ93Y5VXUSJ9S8hm5SY7Si";
        let x_pub_2 = "xpub6CK8ZyoANWjWk24vmGhZ3V5x28QinZ3C66P3es5oDgtrvZLDK8txJHXu88zKsGc3WA7HFUDPHYcoWir4j2cMNMKBBhfHCB37StVhxozA5Lp";
        let x_pub_3 = "xpub6CWL8m4zcbAPXjWkfFWkyjkorenkhBV8P6VFFCmoMn9WZZZhC3ehf7jovLr5HYXGnHZXZbEBFCWo6KqZiqzaV1gMMc5fdprGiWiaA6vynpA";
        let x_pub_4 = "xpub6CexGUAW8CXpTAZ19JxEGRxt2g4W7YNc3XSopBxw27jjBWDF67KShM7JqUibfQpHTsjzBdEwAw9X7QsBTVxjRpgK3bUbhS4e3y6kVhUfkek";
        let x_pub_5 = "xpub6DJvcCRxBqSydGgQ1TZ7xKRaQ1TriTLCpH7bd912nvJfET2HwfQo292zEcGU6pSSH7Q7PpijZf1kAgk8Rt8pkgVd5UJNFSADZN7odCbX314";

        let x_pub_1_path = "m/84'/0'/0'";
        let x_pub_2_path = "m/49'/0'/0'";
        let x_pub_3_path = "m/44'/0'/0'";
        let x_pub_4_path = "m/44'/931'/0'";
        let x_pub_5_path = "m/44'/60'/0'";

        let account = generate_crypto_account(
            &mfp,
            &[x_pub_1, x_pub_2, x_pub_3, x_pub_4, x_pub_5],
            &[
                x_pub_1_path,
                x_pub_2_path,
                x_pub_3_path,
                x_pub_4_path,
                x_pub_5_path,
            ],
        )
        .unwrap();
        let cbor: Vec<u8> = account.try_into().unwrap();
        assert_eq!("a2011a73c5da0a0285d90194d9012fa702f40358210238506dbd94e82166cb68536ffa0d0e145fcb87b975d9dbd0475fdb664f3daea6045820399c9a9c6b98711235a2484f8e44cbb5f1e656306ae15a0bb29a0d7ed227f0a805d90131a20100020006d90130a301861854f500f500f5021a73c5da0a030307d90130a2018400f480f40300081a81ff3431d90190d90194d9012fa702f403582102f3b97cf3f3387e2c4d8c7141a21529a90e0585e9f032706798d18f988a56e3f1045820ac31dee4dd3f4632f984e0a41e8728edc3ec67f614c8f03181490c8945d19d7405d90131a20100020006d90130a301861831f500f500f5021a73c5da0a030307d90130a2018400f480f40300081a59fcb265d90193d9012fa702f403582103ebd552027b73adb1de1aa494ca7cedfe781434d6f102a55355b118a0c5da78bc045820517438aec2b78e81c275a41be63df6083358070cbefdb59de69bd2f99c003e8a05d90131a20100020006d90130a30186182cf500f500f5021a73c5da0a030307d90130a2018400f480f40300081a7441f35cd90193d9012fa702f40358210272026869a49a77a4f219b42ab832b6b12742bdf2dc7e12baa81e1377bf4f0d92045820b16c330d8d6ae98a126a8f18aba631cab293174ccb790140fc2bfd46d619f24905d90131a20100020006d90130a30186182cf51903a3f500f5021a73c5da0a030307d90130a2018400f480f40300081a887c0fd0d90193d9012fa702f4035821034c729aa638b3261640a8f06a5eabbfe4c04d2e0aac434b344147a1a5fa3555a304582096672bdd3ee7212fe7947040e703cddbaab5b498815787a601476c75e7e7899d05d90131a20100020006d90130a30186182cf5183cf500f5021a73c5da0a030307d90130a2018400f480f40300081ae18decc8",
                   hex::encode(cbor).to_lowercase()
        );
    }
}
