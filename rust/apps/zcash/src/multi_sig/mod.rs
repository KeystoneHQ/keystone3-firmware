pub mod address;
pub mod wallet;

use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use core::str::FromStr;

use serde_json::{self, Value};
use ur_registry::{
    bytes::Bytes,
    crypto_account::CryptoAccount,
    crypto_coin_info::{CoinType, CryptoCoinInfo, Network as NetworkInner},
    crypto_hd_key::CryptoHDKey,
    crypto_key_path::{CryptoKeyPath, PathComponent},
    crypto_output::CryptoOutput,
    error::{URError, URResult},
    script_expression::ScriptExpression,
};
use zcash_vendor::{
    bip32, secp256k1, zcash_protocol::consensus::Network, zcash_script::descriptor,
};

use crate::ZcashError;

fn network_from_inner(value: NetworkInner) -> Network {
    match value {
        NetworkInner::MainNet => Network::MainNetwork,
        NetworkInner::TestNet => Network::TestNetwork,
    }
}

pub enum MultiSigType {
    P2sh,
    P2shTest,
}

pub enum MultiSigFormat {
    P2sh,
}

impl MultiSigFormat {
    pub fn from(format_str: &str) -> Result<Self, ZcashError> {
        match format_str {
            "P2SH" => Ok(MultiSigFormat::P2sh),
            _ => Err(ZcashError::MultiSigUnsupportedWalletFormat(
                format_str.to_string(),
            )),
        }
    }

    pub fn get_multi_sig_format_string(&self) -> String {
        match self {
            MultiSigFormat::P2sh => "P2SH".to_string(),
        }
    }
}

const MULTI_P2SH_PATH: &str = "m/48'/133'/0'/133000'";
const MULTI_P2SH_PATH_TEST: &str = "m/48'/1'/0'/133000'";

pub fn bip_48_path(coin_type: usize, account: usize, script_type: usize) -> String {
    let purpose = 48;

    format!("m/{purpose}'/{coin_type}'/{account}'/{script_type}'")
}

pub fn zip_48_path(network: &Network, account: usize) -> String {
    let coin_type = match network {
        Network::MainNetwork => 133,
        Network::TestNetwork => 1,
    };
    let script_type = 133000;

    bip_48_path(coin_type, account, script_type)
}

#[derive(Debug)]
pub struct MultiSigXPubInfo {
    pub path: String,
    pub key_expr: descriptor::KeyExpression,
}

pub fn extract_xpub_info_from_ur_bytes(
    bytes: Bytes,
    format: &MultiSigFormat,
) -> Result<MultiSigXPubInfo, ZcashError> {
    let data = String::from_utf8(bytes.get_bytes())
        .map_err(|e| ZcashError::MultiSigWalletImportXpubError(e.to_string()))?;

    extract_xpub_info_from_str(&data, format)
}

pub fn extract_xpub_info_from_bytes(
    bytes: Bytes,
    format: &MultiSigFormat,
) -> Result<MultiSigXPubInfo, ZcashError> {
    let data = String::from_utf8(bytes.get_bytes())
        .map_err(|e| ZcashError::MultiSigWalletImportXpubError(e.to_string()))?;

    extract_xpub_info_from_str(&data, format)
}

pub fn extract_xpub_info_from_str(
    data: &str,
    format: &MultiSigFormat,
) -> Result<MultiSigXPubInfo, ZcashError> {
    let json_data: Value = serde_json::from_str(data)
        .map_err(|e| ZcashError::MultiSigWalletImportXpubError(e.to_string()))?;

    let (xpub_field_name, deriv_field_name) = match format {
        MultiSigFormat::P2sh => ("p2sh", "p2sh_deriv"),
    };

    let path = json_data
        .get(deriv_field_name)
        .ok_or_else(|| {
            ZcashError::MultiSigWalletImportXpubError(format!(
                "have no {deriv_field_name} field in json"
            ))
        })?
        .as_str()
        .ok_or_else(|| {
            ZcashError::MultiSigWalletImportXpubError(format!(
                "{deriv_field_name} field is not a string"
            ))
        })?
        .to_string();

    let xpub = json_data
        .get(xpub_field_name)
        .ok_or_else(|| {
            ZcashError::MultiSigWalletImportXpubError(format!(
                "have no {xpub_field_name} field in json"
            ))
        })?
        .as_str()
        .ok_or_else(|| {
            ZcashError::MultiSigWalletImportXpubError(format!(
                "{xpub_field_name} field is not a string"
            ))
        })?
        .parse()
        .map_err(|e: bip32::Error| ZcashError::MultiSigWalletImportXpubError(e.to_string()))?;

    let xfp = json_data
        .get("xfp")
        .ok_or_else(|| {
            ZcashError::MultiSigWalletImportXpubError("have no xfp field in json".to_string())
        })?
        .as_str()
        .ok_or_else(|| {
            ZcashError::MultiSigWalletImportXpubError("xfp field is not a string".to_string())
        })?;

    let xfp = hex::decode(xfp)
        .map_err(|e| e.to_string())
        .and_then(|v| {
            v.try_into()
                .map_err(|_| "xfp field is the wrong length".to_string())
        })
        .map_err(ZcashError::MultiSigWalletImportXpubError)?;

    let key_expr = descriptor::KeyExpression::from_xpub(
        Some(descriptor::KeyOrigin::from_parts(xfp, vec![])),
        bip32::Prefix::XPUB,
        xpub,
        vec![],
    )
    .ok_or_else(|| {
        ZcashError::MultiSigWalletImportXpubError(
            "couldn’t recover expression from xpub".to_string(),
        )
    })?;

    Ok(MultiSigXPubInfo { path, key_expr })
}

pub fn extract_xpub_info_from_crypto_account(
    crypto_account: &CryptoAccount,
    multi_sig_type: MultiSigType,
) -> Result<MultiSigXPubInfo, ZcashError> {
    let script_expression = match multi_sig_type {
        MultiSigType::P2sh | MultiSigType::P2shTest => vec![ScriptExpression::ScriptHash],
    };

    let output_descriptors = crypto_account.get_output_descriptors();

    let target_output = output_descriptors
        .iter()
        .find(|output| output.get_script_expressions() == script_expression)
        .unwrap_or(&output_descriptors[0]);

    crypto_output_to_multi_sig_xpub_info(target_output)
}

pub fn export_xpub_by_crypto_account(
    master_fingerprint: &[u8; 4],
    extended_public_keys: &[&str],
    extended_public_keys_path: &[&str],
    network: Network,
) -> URResult<CryptoAccount> {
    let mut outputs = vec![];

    if extended_public_keys.len() != extended_public_keys_path.len() {
        return Err(URError::UrEncodeError(format!(
            "input data length mismatch, length of keys is {}, length of paths is {}",
            extended_public_keys.len(),
            extended_public_keys_path.len()
        )));
    }

    for (index, path) in extended_public_keys_path.iter().enumerate() {
        match path.to_lowercase().as_str() {
            MULTI_P2SH_PATH | MULTI_P2SH_PATH_TEST => outputs.push(generate_multi_sig_p2sh_output(
                master_fingerprint,
                extended_public_keys[index],
                &network,
            )?),
            _ => {
                return Err(URError::UrEncodeError(format!("not supported path:{path}")));
            }
        }
    }
    Ok(CryptoAccount::new(*master_fingerprint, outputs))
}

fn crypto_output_to_multi_sig_xpub_info(
    crypto_output: &CryptoOutput,
) -> Result<MultiSigXPubInfo, ZcashError> {
    let hdkey = crypto_output
        .get_hd_key()
        .ok_or_else(|| ZcashError::MultiSigWalletImportXpubError("get hdkey error".to_string()))?;
    let origin = hdkey
        .get_origin()
        .ok_or_else(|| ZcashError::MultiSigWalletImportXpubError("get origin error".to_string()))?;
    let path = format!(
        "m/{}",
        origin
            .get_path()
            .ok_or_else(|| ZcashError::MultiSigWalletImportXpubError(
                "get path error".to_string()
            ))?
    );

    if !is_valid_multi_path(&path) {
        return Err(ZcashError::MultiSigWalletImportXpubError(
            "not a valid multi path".to_string(),
        ));
    }

    let depth = origin
        .get_depth()
        .unwrap_or(origin.get_components().len() as u32) as u8;
    let source_fingerprint = origin.get_source_fingerprint().ok_or_else(|| {
        ZcashError::MultiSigWalletImportXpubError("get source fingerprint error".to_string())
    })?;

    let net_work = match hdkey.get_use_info() {
        Some(use_info) => use_info.get_network(),
        None => NetworkInner::MainNet,
    };

    let parent_fingerprint = hdkey.get_parent_fingerprint().ok_or_else(|| {
        ZcashError::MultiSigWalletImportXpubError("get parent fingerprint error".to_string())
    })?;
    let key = hdkey
        .get_key()
        .try_into()
        .map_err(|_| ZcashError::MultiSigWalletImportXpubError("wrong key length".to_string()))?;
    let chain_code = hdkey
        .get_chain_code()
        .ok_or_else(|| {
            ZcashError::MultiSigWalletImportXpubError("get chain code error".to_string())
        })
        .and_then(|v| {
            v.try_into().map_err(|_| {
                ZcashError::MultiSigWalletImportXpubError("wrong chain code length".to_string())
            })
        })?;
    let child_number = origin
        .get_components()
        .last()
        .ok_or_else(|| {
            ZcashError::MultiSigWalletImportXpubError("get child number error".to_string())
        })?
        .get_canonical_index()
        .ok_or_else(|| {
            ZcashError::MultiSigWalletImportXpubError("get child number error".to_string())
        })
        .map(bip32::ChildNumber)?;

    let xpub = bip32::ExtendedPublicKey::new(
        bip32::PublicKey::from_bytes(key).map_err(|e: bip32::Error| {
            ZcashError::MultiSigWalletImportXpubError(format!("bip32 error: {e}"))
        })?,
        bip32::ExtendedKeyAttrs {
            depth,
            parent_fingerprint,
            child_number,
            chain_code,
        },
    );

    let key_expr = descriptor::KeyExpression::from_xpub(
        Some(descriptor::KeyOrigin::from_parts(
            source_fingerprint,
            vec![],
        )),
        convert_xpub(&network_from_inner(net_work)),
        xpub,
        vec![],
    )
    .ok_or(ZcashError::MultiSigWalletImportXpubError(
        "couldn’t recover expression from xpub".to_string(),
    ))?;

    Ok(MultiSigXPubInfo { path, key_expr })
}

fn generate_multi_sig_p2sh_output(
    master_fingerprint: &[u8; 4],
    extended_public_key: &str,
    network: &Network,
) -> URResult<CryptoOutput> {
    let script_expressions = vec![ScriptExpression::ScriptHash];
    Ok(CryptoOutput::new(
        script_expressions,
        None,
        Some(generate_multi_sig_crypto_hd_key(
            master_fingerprint,
            extended_public_key,
            if *network == Network::MainNetwork {
                MultiSigType::P2sh
            } else {
                MultiSigType::P2shTest
            },
        )?),
        None,
    ))
}

fn generate_multi_sig_crypto_hd_key(
    master_fingerprint: &[u8; 4],
    extended_public_key: &str,
    multi_sig_type: MultiSigType,
) -> URResult<CryptoHDKey> {
    let bip32_extended_pub_key =
        bip32::ExtendedPublicKey::<secp256k1::PublicKey>::from_str(extended_public_key)
            .map_err(|e| URError::UrEncodeError(e.to_string()))?;
    let parent_fingerprint = bip32_extended_pub_key.attrs().parent_fingerprint;

    let network_inner;
    let path_components = match multi_sig_type {
        MultiSigType::P2sh => {
            network_inner = NetworkInner::MainNet;
            vec![
                get_path_component(Some(48), true)?,
                get_path_component(Some(133), true)?,
                get_path_component(Some(0), true)?,
                get_path_component(Some(133000), true)?,
            ]
        }
        MultiSigType::P2shTest => {
            network_inner = NetworkInner::TestNet;
            vec![
                get_path_component(Some(48), true)?,
                get_path_component(Some(1), true)?,
                get_path_component(Some(0), true)?,
                get_path_component(Some(133000), true)?,
            ]
        }
    };

    let coin_info = CryptoCoinInfo::new(Some(CoinType::Bitcoin), Some(network_inner));

    let origin = CryptoKeyPath::new(
        path_components,
        Some(*master_fingerprint),
        Some(bip32_extended_pub_key.attrs().depth as u32),
    );

    let hd_key = CryptoHDKey::new_extended_key(
        Some(false),
        Vec::from(bip32_extended_pub_key.public_key().serialize()),
        Some(bip32_extended_pub_key.attrs().chain_code[..].to_vec()),
        Some(coin_info),
        Some(origin),
        None,
        Some(parent_fingerprint),
        None,
        None,
    );

    Ok(hd_key)
}

fn get_path_component(index: Option<u32>, hardened: bool) -> URResult<PathComponent> {
    PathComponent::new(index, hardened).map_err(URError::CborEncodeError)
}

#[allow(unused)]
fn is_valid_multi_path(path: &str) -> bool {
    const VALID_PATHS: [&str; 2] = [MULTI_P2SH_PATH, MULTI_P2SH_PATH_TEST];
    VALID_PATHS
        .iter()
        .any(|valid_path| path.starts_with(valid_path))
}

/// __NB__: The Bitcoin version of this dispatches on the path _and_ the network, but they should
///         agree, and the network is a better-defined type.
fn convert_xpub(network: &Network) -> bip32::Prefix {
    match network {
        Network::MainNetwork => bip32::Prefix::XPUB,
        Network::TestNetwork => bip32::Prefix::TPUB,
    }
}

#[cfg(test)]
mod tests {
    extern crate std;

    use alloc::{string::ToString, vec::Vec};
    use std::println;

    use hex::{self, FromHex};
    use ur_registry::{bytes::Bytes, crypto_account::CryptoAccount, error::URError};

    use crate::{
        multi_sig::{
            export_xpub_by_crypto_account, extract_xpub_info_from_bytes,
            extract_xpub_info_from_crypto_account, extract_xpub_info_from_str,
            MultiSigFormat, MultiSigType, Network,
        },
        ZcashError,
    };

    use super::{MULTI_P2SH_PATH, MULTI_P2SH_PATH_TEST};

    // define all values in one place, to make it easier to check round-tripping.
    const master_fingerprint: &str = "73c5da0a";

    const p2sh_path: &str = MULTI_P2SH_PATH;
    const p2sh_xpub: &str = "xpub6EghKBv28q43t91tEvAr7xGvGD4m8n1GT99miHXL73dyPLFpPNmWDUNuFxxN13VCmDBisLXQ6bBMAw7Vh2XhCRAB6MvcMdT97j2XQvu2Ynr";
    const p2sh_crypto_account: &str = "a2011a73c5da0a0281d90190d9012fa602f40358210230eac4810a3e52b004273fbc72343fb311a9e116165e7aa01081f96556853cd904582071e739fc7de677841d2187265f5ff55b8fec61383c5c0df13c39874ed19aa79a05d90131a20100020006d90130a301881830f51885f500f51a00020788f5021a73c5da0a0304081a9cac6539";

    const p2sh_path_test: &str = MULTI_P2SH_PATH_TEST;
    const p2sh_xpub_test: &str = "tpubDFH9dgzvezwuSqjsRbuZAzpxjWCbDWP7X1FoQgX3So1uaDDeM6RhLYDDyQJRdWvN63BVRgRPGDWAzdPDH5MomzjdP7TEwStJZE2hcu3fobP";
    const p2sh_crypto_account_test: &str = "a2011a73c5da0a0281d90190d9012fa602f403582103a9304c5e9e6827dd8cb30dee711295d6f7751f3c1132e475740c25f430aef23704582052a75797184c80c4e39048706ec396cb3107bf5b5ffb076ed23e31e53e49515c05d90131a20100020106d90130a301881830f501f500f51a00020788f5021a73c5da0a0304081abac14839";

    #[test]
    fn test_generate_multi_sig_crypto_account() {
        let mfp = Vec::from_hex(master_fingerprint).unwrap();
        let mfp: [u8; 4] = mfp.try_into().unwrap();

        let p2wsh_p2sh_path = "m/48'/0'/0'/1'";
        let p2wsh_p2sh_xpub = "xpub6DkFAXWQ2dHxnMKoSBogHrw1rgNJKR4umdbnNVNTYeCGcduxWnNUHgGptqEQWPKRmeW4Zn4FHSbLMBKEWYaMDYu47Ytg6DdFnPNt8hwn5mE";

        let p2wsh_p2sh_path_test = "m/48'/1'/0'/1'";
        let p2wsh_p2sh_xpub_test = "xpub6EuX7TBEwhFghVDjgJsj4CvAqnqxLxnJC1Rtr4Y3T6VvpzKqRfTZQ7HnJeM3uxwLJ1MZeKMGFHZFAxhRCyxiyDWng7pnYQzvBsMSdmog9kW";

        let p2wsh_path = "m/48'/0'/0'/2'";
        let p2wsh_xpub = "xpub6DkFAXWQ2dHxq2vatrt9qyA3bXYU4ToWQwCHbf5XB2mSTexcHZCeKS1VZYcPoBd5X8yVcbXFHJR9R8UCVpt82VX1VhR28mCyxUFL4r6KFrf";

        let p2wsh_path_test = "m/48'/1'/0'/2'";
        let p2wsh_xpub_test = "xpub6EuX7TBEwhFgifQY24vFeMRqeWHGyGCupztDxk7G2ECAqGQ22Fik8E811p8GrM2LfajQzLidXy4qECxhdcxChkjiKhnq2fiVMVjdfSoZQwg";

        // p2sh  mainnet
        {
            let account = export_xpub_by_crypto_account(
                &mfp,
                &[p2sh_xpub],
                &[&p2sh_path],
                Network::MainNetwork,
            )
            .unwrap();
            let cbor: Vec<u8> = account.try_into().unwrap();
            assert_eq!(p2sh_crypto_account, hex::encode(cbor).to_lowercase());
        }

        // p2wsh_p2sh  mainnet
        {
            let account = export_xpub_by_crypto_account(
                &mfp,
                &[p2wsh_p2sh_xpub],
                &[p2wsh_p2sh_path],
                Network::MainNetwork,
            );
            assert_eq!(
                Err(URError::UrEncodeError(
                    "not supported path:m/48'/0'/0'/1'".to_string()
                )),
                account.map(|_| ())
            );
        }

        // p2wsh  mainnet
        {
            let account = export_xpub_by_crypto_account(
                &mfp,
                &[p2wsh_xpub],
                &[p2wsh_path],
                Network::MainNetwork,
            );
            assert_eq!(
                Err(URError::UrEncodeError(
                    "not supported path:m/48'/0'/0'/2'".to_string()
                )),
                account.map(|_| ())
            );
        }

        // p2sh  testnet
        {
            let account = export_xpub_by_crypto_account(
                &mfp,
                &[p2sh_xpub_test],
                &[&p2sh_path_test],
                Network::TestNetwork,
            )
            .unwrap();
            let cbor: Vec<u8> = account.try_into().unwrap();
            assert_eq!(p2sh_crypto_account_test, hex::encode(cbor).to_lowercase());
        }

        // p2wsh_p2sh  testnet
        {
            let account = export_xpub_by_crypto_account(
                &mfp,
                &[p2wsh_p2sh_xpub_test],
                &[p2wsh_p2sh_path_test],
                Network::TestNetwork,
            );
            assert_eq!(
                Err(URError::UrEncodeError(
                    "not supported path:m/48'/1'/0'/1'".to_string()
                )),
                account.map(|_| ())
            );
        }

        // p2wsh  testnet
        {
            let account = export_xpub_by_crypto_account(
                &mfp,
                &[p2wsh_xpub_test],
                &[p2wsh_path_test],
                Network::TestNetwork,
            );
            assert_eq!(
                Err(URError::UrEncodeError(
                    "not supported path:m/48'/1'/0'/2'".to_string()
                )),
                account.map(|_| ())
            );
        }
    }

    #[test]
    fn test_crypto_account_to_multi_sig_xpub_info() {
        // p2sh  mainnet
        {
            let bytes = Vec::from_hex(p2sh_crypto_account).unwrap();
            let account = CryptoAccount::try_from(bytes).unwrap();

            let result =
                extract_xpub_info_from_crypto_account(&account, MultiSigType::P2sh).unwrap();

            assert_eq!(p2sh_path, result.path);
            let (xfp, xpub) = result.key_expr.clone().into_parts();
            assert_eq!(master_fingerprint, hex::encode(xfp.unwrap().fingerprint()));
            assert_eq!(p2sh_xpub, xpub.to_string());

            println!("result is {:?}", result)
        }

        // p2wsh_p2sh  mainnet
        {
            let bytes = Vec::from_hex(
                "a2011a73c5da0a0281d90190d90191d9012fa602f4035821036da4c96b78e8ad5cc3556bb464e1ad05132cccb0098eda9337d3f882241796aa045820ae2afa5bbcc9c85c2c9aefb0a89f27614878b1dfc7ac7c38d8614c64ffbfce7005d90131a20100020006d90130a301881830f500f500f501f5021a73c5da0a0304081a1cf29716",
            )
                .unwrap();
            let account = CryptoAccount::try_from(bytes).unwrap();

            let result = extract_xpub_info_from_crypto_account(&account, MultiSigType::P2sh);

            assert_eq!(
                Err(ZcashError::MultiSigWalletImportXpubError(
                    "not a valid multi path".to_string()
                )),
                result.map(|_| ())
            );
        }

        // p2wsh  mainnet
        {
            let bytes = Vec::from_hex(
                "a2011a73c5da0a0281d90191d9012fa602f4035821021a3bf5fbf737d0f36993fd46dc4913093beb532d654fe0dfd98bd27585dc9f29045820bba0c7ca160a870efeb940ab90d0f4284fea1b5e0d2117677e823fc37e2d576305d90131a20100020006d90130a301881830f500f500f502f5021a73c5da0a0304081a1cf29716",
            )
                .unwrap();
            let account = CryptoAccount::try_from(bytes).unwrap();
            let result = extract_xpub_info_from_crypto_account(&account, MultiSigType::P2sh);

            assert_eq!(
                Err(ZcashError::MultiSigWalletImportXpubError(
                    "not a valid multi path".to_string()
                )),
                result.map(|_| ())
            );
        }

        // p2sh  testnet
        {
            let bytes = Vec::from_hex(p2sh_crypto_account_test).unwrap();
            let account = CryptoAccount::try_from(bytes).unwrap();

            let result =
                extract_xpub_info_from_crypto_account(&account, MultiSigType::P2sh).unwrap();

            assert_eq!(p2sh_path_test, result.path);
            let (xfp, xpub) = result.key_expr.clone().into_parts();
            assert_eq!(master_fingerprint, hex::encode(xfp.unwrap().fingerprint()));
            assert_eq!(p2sh_xpub_test, xpub.to_string());

            println!("reuslt is {:?}", result)
        }

        // p2wsh_p2sh  testnet
        {
            let bytes = Vec::from_hex(
                "a2011a73c5da0a0281d90190d90191d9012fa602f403582102e62a2a9973ee6b3a7af47c229a5bde70bca59bd04bbb297f5693d7aa256b976d045820a73adbe2878487634dcbfc3f7ebde8b1fc994f1ec06860cf01c3fe2ea791ddb605d90131a20100020106d90130a301881830f501f500f501f5021a73c5da0a0304081abac14839",
            )
                .unwrap();
            let account = CryptoAccount::try_from(bytes).unwrap();
            let result = extract_xpub_info_from_crypto_account(&account, MultiSigType::P2shTest);

            assert_eq!(
                Err(ZcashError::MultiSigWalletImportXpubError(
                    "not a valid multi path".to_string()
                )),
                result.map(|_| ())
            );
        }

        // p2wsh  testnet
        {
            let bytes = Vec::from_hex(
                "a2011a73c5da0a0281d90191d9012fa602f403582103568ea1f36051916ed1b690c39e12a8e70603b280bd30ce5f281f2918a55363aa0458201d4fbebdd967e1af714c0997d38fcf670b5ce9d301c0440bbcc3b6a20eb7721c05d90131a20100020106d90130a301881830f501f500f502f5021a73c5da0a0304081abac14839",
            )
                .unwrap();
            let account = CryptoAccount::try_from(bytes).unwrap();

            let result = extract_xpub_info_from_crypto_account(&account, MultiSigType::P2shTest);

            assert_eq!(
                Err(ZcashError::MultiSigWalletImportXpubError(
                    "not a valid multi path".to_string()
                )),
                result.map(|_| ())
            );
        }
    }

    const extract_xpub_master_fingerprint: &str = "eb16731f";
    const extract_xpub_xpub: &str = "xpub69cicR2MFe9QbMVTMHN882fGtXBKQV4g9gqWNZN7aEM9RASi3WmUzgPF9md8fLfUNuF4znQ8937VQrjG2bG8VgU7rjhUR8qCfBL9hJDQogL";

    #[test]
    fn test_extract_xpub_info_from_str() {
        let json_str = r#"{
  "p2sh_deriv": "m/48'/133'/0'/133000'",
  "p2sh": "xpub69cicR2MFe9QbMVTMHN882fGtXBKQV4g9gqWNZN7aEM9RASi3WmUzgPF9md8fLfUNuF4znQ8937VQrjG2bG8VgU7rjhUR8qCfBL9hJDQogL",
  "p2sh_desc": "sh(sortedmulti(M,[eb16731f/48'/133'/0'/133000']xpub69cicR2MFe9QbMVTMHN882fGtXBKQV4g9gqWNZN7aEM9RASi3WmUzgPF9md8fLfUNuF4znQ8937VQrjG2bG8VgU7rjhUR8qCfBL9hJDQogL/0/*,...))",
  "p2sh_p2wsh_deriv": "m/48'/0'/0'/1'",
  "p2sh_p2wsh": "Ypub6jwcqK3XjbvvGXVFK6ghVrrqctFYJjFPh4ZiQ6ZFfMKxZSe5FWzZX3ib3THzRy2UKsesZwXmXVGsez8pyMuDmdbhnN55RcZbTLj7rFqViqo",
  "p2sh_p2wsh_desc": "sh(wsh(sortedmulti(M,[eb16731f/48'/0'/0'/1']xpub6EDGQQeB1xq4zf8kY5S6ThRXj84q9kaJTgPphSPpua7ftAFwETSiosCXDYNvsd9egmU5wsw6BcXpeXuVxSLFpuoS5ZGFSJH7HtPbsjGzJrH/0/*,...)))",
  "p2wsh_deriv": "m/48'/0'/0'/2'",
  "p2wsh": "Zpub74mt8yiStHUQASjHhGjAeBL6oU1PG8NCUQRRxucP9Dvwz7p65oHChnf8tQrhZfkCPesk7vays9n8cyLDDRM7Wqxmz2mihQHSikXfEwyp6kt",
  "p2wsh_desc": "wsh(sortedmulti(M,[eb16731f/48'/0'/0'/2']xpub6EDGQQeB1xq53HBg5tgwPvoHjjgEAXhcKujKUrZ51SLnFjcip5ZoNYUw3Hz41RDTLua9kPPk4cgXjEVKUoN8mtUuQtGU8BBUHa8Vsrd6k7o/0/*,...))",
  "account": "0",
  "xfp": "EB16731F"
}
"#;

        let result = extract_xpub_info_from_str(json_str, &MultiSigFormat::P2sh).unwrap();
        println!("result is {:?}", result);

        assert_eq!(MULTI_P2SH_PATH, result.path);
        let (xfp, xpub) = result.key_expr.clone().into_parts();
        assert_eq!(
            extract_xpub_master_fingerprint,
            hex::encode(xfp.unwrap().fingerprint())
        );
        assert_eq!(extract_xpub_xpub, xpub.to_string());
    }

    #[test]
    fn test_extract_xpub_info_from_bytes() {
        let cbor = "59044c7b0a202022703273685f6465726976223a20226d2f3438272f313333272f30272f31333330303027222c0a20202270327368223a202278707562363963696352324d46653951624d56544d484e38383266477458424b51563467396771574e5a4e3761454d395241536933576d557a675046396d6438664c66554e7546347a6e51383933375651726a473262473856675537726a68555238714366424c39684a44516f674c222c0a202022703273685f64657363223a2022736828736f727465646d756c7469284d2c5b65623136373331662f3438272f313333272f30272f313333303030275d78707562363963696352324d46653951624d56544d484e38383266477458424b51563467396771574e5a4e3761454d395241536933576d557a675046396d6438664c66554e7546347a6e51383933375651726a473262473856675537726a68555238714366424c39684a44516f674c2f302f2a2c2e2e2e2929222c0a202022703273685f70327773685f6465726976223a20226d2f3438272f30272f30272f3127222c0a202022703273685f7032777368223a202259707562366a7763714b33586a627676475856464b36676856727271637446594a6a465068345a6951365a46664d4b785a53653546577a5a583369623354487a527932554b7365735a77586d58564773657a3870794d75446d6462686e4e353552635a62544c6a377246715669716f222c0a202022703273685f70327773685f64657363223a202273682877736828736f727465646d756c7469284d2c5b65623136373331662f3438272f30272f30272f31275d787075623645444751516542317871347a66386b59355336546852586a383471396b614a54675070685350707561376674414677455453696f73435844594e7673643965676d553577737736426358706558755678534c4670756f53355a4746534a483748745062736a477a4a72482f302f2a2c2e2e2e292929222c0a20202270327773685f6465726976223a20226d2f3438272f30272f30272f3227222c0a2020227032777368223a20225a70756237346d74387969537448555141536a4868476a4165424c366f55315047384e435551525278756350394476777a377036356f4843686e6638745172685a666b435065736b3776617973396e3863794c4444524d375771786d7a326d6968514853696b586645777970366b74222c0a20202270327773685f64657363223a202277736828736f727465646d756c7469284d2c5b65623136373331662f3438272f30272f30272f32275d78707562364544475151654231787135334842673574677750766f486a6a6745415868634b756a4b55725a3531534c6e466a636970355a6f4e59557733487a34315244544c7561396b50506b346367586a45564b556f4e386d745575517447553842425548613856737264366b376f2f302f2a2c2e2e2e2929222c0a2020226163636f756e74223a202230222c0a202022786670223a20224542313637333146220a7d0a";
        let bytes = Bytes::try_from(Vec::from_hex(cbor).unwrap()).unwrap();

        let result = extract_xpub_info_from_bytes(bytes, &MultiSigFormat::P2sh).unwrap();

        assert_eq!(MULTI_P2SH_PATH, result.path);
        let (xfp, xpub) = result.key_expr.clone().into_parts();
        assert_eq!(
            extract_xpub_master_fingerprint,
            hex::encode(xfp.unwrap().fingerprint())
        );
        assert_eq!(extract_xpub_xpub, xpub.to_string());
    }
}
