pub mod address;
pub mod wallet;

use crate::addresses::xyzpub::{convert_version, Version, VERSION_XPUB};
use crate::BitcoinError;
use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;
use core::fmt;
use core::str::FromStr;
use third_party::bitcoin::bip32;
use third_party::bitcoin::bip32::Xpub;
use third_party::bitcoin::hex::DisplayHex;
use third_party::serde_json;
use third_party::serde_json::Value;
use third_party::ur_registry::bytes::Bytes;
use third_party::ur_registry::crypto_account::CryptoAccount;
use third_party::ur_registry::crypto_coin_info::{
    CoinType, CryptoCoinInfo, Network as NetworkInner,
};
use third_party::ur_registry::crypto_hd_key::CryptoHDKey;
use third_party::ur_registry::crypto_key_path::{CryptoKeyPath, PathComponent};
use third_party::ur_registry::crypto_output::CryptoOutput;
use third_party::ur_registry::error::{URError, URResult};
use third_party::ur_registry::script_expression::ScriptExpression;

#[derive(Debug, PartialEq, Clone)]
pub enum Network {
    MainNet,
    TestNet,
}

impl From<&NetworkInner> for Network {
    fn from(value: &NetworkInner) -> Self {
        match *value {
            NetworkInner::MainNet => Network::MainNet,
            NetworkInner::TestNet => Network::TestNet,
        }
    }
}

impl TryFrom<&crate::network::Network> for Network {
    type Error = BitcoinError;

    fn try_from(value: &crate::network::Network) -> Result<Self, BitcoinError> {
        match *value {
            crate::network::Network::Bitcoin => Ok(Network::MainNet),
            crate::network::Network::BitcoinTestnet => Ok(Network::TestNet),
            _ => Err(BitcoinError::MultiSigNetworkError(
                "not support network".to_string(),
            )),
        }
    }
}

impl fmt::Display for Network {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Network::MainNet => write!(f, "mainnet"),
            Network::TestNet => write!(f, "testnet"),
        }
    }
}

pub enum MultiSigType {
    P2sh,
    P2wshP2sh,
    P2wsh,
    P2shTest,
    P2wshP2shTest,
    P2wshTest,
}

pub enum MultiSigFormat {
    P2sh,
    P2wshP2sh,
    P2wsh,
}

impl MultiSigFormat {
    pub fn from(format_str: &str) -> Result<Self, BitcoinError> {
        match format_str {
            "P2SH" => Ok(MultiSigFormat::P2sh),
            "P2WSH-P2SH" => Ok(MultiSigFormat::P2wshP2sh),
            "P2WSH" => Ok(MultiSigFormat::P2wsh),
            _ => Err(BitcoinError::MultiSigWalletFormatError(format!(
                "not support this format {}",
                format_str
            ))),
        }
    }

    pub fn get_multi_sig_format_string(&self) -> String {
        match self {
            MultiSigFormat::P2sh => "P2SH".to_string(),
            MultiSigFormat::P2wshP2sh => "P2WSH-P2SH".to_string(),
            MultiSigFormat::P2wsh => "P2WSH".to_string(),
        }
    }
}

//For MULTI_P2SH, the mainnet and testnet share the same path.
const MULTI_P2SH_PATH: &str = "m/45'";

const MULTI_P2WSH_P2SH_PATH: &str = "m/48'/0'/0'/1'";
const MULTI_P2WSH_P2SH_PATH_TEST: &str = "m/48'/1'/0'/1'";

const MULTI_P2WSH_PATH: &str = "m/48'/0'/0'/2'";
const MULTI_P2WSH_PATH_TEST: &str = "m/48'/1'/0'/2'";

#[derive(Debug)]
pub struct MultiSigXPubInfo {
    pub path: String,
    pub xfp: String,
    pub xpub: String,
}

pub fn extract_xpub_info_from_bytes(
    bytes: Bytes,
    format: &MultiSigFormat,
) -> Result<MultiSigXPubInfo, BitcoinError> {
    let data = String::from_utf8(bytes.get_bytes())
        .map_err(|e| BitcoinError::MultiSigWalletImportXpubError(e.to_string()))?;

    extract_xpub_info_from_str(&data, format)
}

pub fn extract_xpub_info_from_str(
    data: &str,
    format: &MultiSigFormat,
) -> Result<MultiSigXPubInfo, BitcoinError> {
    let json_data: Value = serde_json::from_str(data)
        .map_err(|e| BitcoinError::MultiSigWalletImportXpubError(e.to_string()))?;

    let (xpub_field_name, deriv_field_name) = match format {
        MultiSigFormat::P2sh => ("p2sh", "p2sh_deriv"),
        MultiSigFormat::P2wshP2sh => ("p2sh_p2wsh", "p2sh_p2wsh_deriv"),
        MultiSigFormat::P2wsh => ("p2wsh", "p2wsh_deriv"),
    };

    let path = json_data
        .get(deriv_field_name)
        .ok_or_else(|| {
            BitcoinError::MultiSigWalletImportXpubError(format!(
                "have no {} field in json",
                deriv_field_name
            ))
        })?
        .as_str()
        .ok_or_else(|| {
            BitcoinError::MultiSigWalletImportXpubError(format!(
                "{} field is not a string",
                deriv_field_name
            ))
        })?
        .to_string();

    let xpub = json_data
        .get(xpub_field_name)
        .ok_or_else(|| {
            BitcoinError::MultiSigWalletImportXpubError(format!(
                "have no {} field in json",
                xpub_field_name
            ))
        })?
        .as_str()
        .ok_or_else(|| {
            BitcoinError::MultiSigWalletImportXpubError(format!(
                "{} field is not a string",
                xpub_field_name
            ))
        })?
        .to_string();

    let xfp = json_data
        .get("xfp")
        .ok_or_else(|| {
            BitcoinError::MultiSigWalletImportXpubError("have no xfp field in json".to_string())
        })?
        .as_str()
        .ok_or_else(|| {
            BitcoinError::MultiSigWalletImportXpubError("xfp field is not a string".to_string())
        })?
        .to_string();

    Ok(MultiSigXPubInfo { path, xfp, xpub })
}

pub fn extract_xpub_info_from_crypto_account(
    crypto_account: &CryptoAccount,
    multi_sig_type: MultiSigType,
) -> Result<MultiSigXPubInfo, BitcoinError> {
    let script_expression = match multi_sig_type {
        MultiSigType::P2sh | MultiSigType::P2shTest => vec![ScriptExpression::ScriptHash],
        MultiSigType::P2wshP2sh | MultiSigType::P2wshP2shTest => vec![
            ScriptExpression::ScriptHash,
            ScriptExpression::WitnessScriptHash,
        ],
        MultiSigType::P2wsh | MultiSigType::P2wshTest => vec![ScriptExpression::WitnessScriptHash],
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
            MULTI_P2SH_PATH => outputs.push(generate_multi_sig_p2sh_output(
                master_fingerprint,
                extended_public_keys[index],
                &network,
            )?),
            MULTI_P2WSH_P2SH_PATH | MULTI_P2WSH_P2SH_PATH_TEST => {
                outputs.push(generate_multi_sig_p2wsh_p2sh_output(
                    master_fingerprint,
                    extended_public_keys[index],
                    &network,
                )?)
            }
            MULTI_P2WSH_PATH | MULTI_P2WSH_PATH_TEST => {
                outputs.push(generate_multi_sig_p2wsh_output(
                    master_fingerprint,
                    extended_public_keys[index],
                    &network,
                )?)
            }
            _ => {
                return Err(URError::UrEncodeError(format!(
                    "not supported path:{}",
                    path.to_string()
                )));
            }
        }
    }
    Ok(CryptoAccount::new(master_fingerprint.clone(), outputs))
}

fn crypto_output_to_multi_sig_xpub_info(
    crypto_output: &CryptoOutput,
) -> Result<MultiSigXPubInfo, BitcoinError> {
    let hdkey = crypto_output.get_hd_key().ok_or_else(|| {
        BitcoinError::MultiSigWalletImportXpubError("get hdkey error".to_string())
    })?;
    let origin = hdkey.get_origin().ok_or_else(|| {
        BitcoinError::MultiSigWalletImportXpubError("get origin error".to_string())
    })?;
    let path = format!(
        "m/{}",
        origin
            .get_path()
            .ok_or_else(|| BitcoinError::MultiSigWalletImportXpubError(
                "get path error".to_string()
            ))?
    );

    //do not limit path type
    // if !is_valid_multi_path(&path) {
    //     return Err(BitcoinError::MultiSigWalletImportXpubError(
    //         "not a valid multi path".to_string(),
    //     ));
    // }

    let depth = origin
        .get_depth()
        .unwrap_or(origin.get_components().len() as u32) as u8;
    let source_fingerprint = origin.get_source_fingerprint().ok_or_else(|| {
        BitcoinError::MultiSigWalletImportXpubError("get source fingerprint error".to_string())
    })?;

    let use_info = hdkey.get_use_info().ok_or_else(|| {
        BitcoinError::MultiSigWalletImportXpubError("get use info error".to_string())
    })?;
    let parent_fingerprint = hdkey.get_parent_fingerprint().ok_or_else(|| {
        BitcoinError::MultiSigWalletImportXpubError("get parent fingerprint error".to_string())
    })?;
    let key = hdkey.get_key();
    let chain_code = hdkey.get_chain_code().ok_or_else(|| {
        BitcoinError::MultiSigWalletImportXpubError("get chain code error".to_string())
    })?;
    let child_number = origin
        .get_components()
        .last()
        .ok_or_else(|| {
            BitcoinError::MultiSigWalletImportXpubError("get child number error".to_string())
        })?
        .get_canonical_index()
        .ok_or_else(|| {
            BitcoinError::MultiSigWalletImportXpubError("get child number error".to_string())
        })?;

    let mut ret = [0; 78];
    ret[0..4].copy_from_slice(&VERSION_XPUB);
    ret[4] = depth;
    ret[5..9].copy_from_slice(&parent_fingerprint);
    ret[9..13].copy_from_slice(&u32::from(child_number).to_be_bytes());
    ret[13..45].copy_from_slice(&chain_code);
    ret[45..78].copy_from_slice(&key);

    let xpub = Xpub::decode(&ret)
        .map_err(|e| BitcoinError::MultiSigWalletImportXpubError(e.to_string()))?
        .to_string();
    let xpub = convert_xpub(&path, &xpub, &Network::from(&use_info.get_network()))?;

    Ok(MultiSigXPubInfo {
        path,
        xfp: source_fingerprint.to_upper_hex_string(),
        xpub,
    })
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
            if *network == Network::MainNet {
                MultiSigType::P2sh
            } else {
                MultiSigType::P2shTest
            },
        )?),
        None,
    ))
}

fn generate_multi_sig_p2wsh_p2sh_output(
    master_fingerprint: &[u8; 4],
    extended_public_key: &str,
    network: &Network,
) -> URResult<CryptoOutput> {
    let script_expressions = vec![
        ScriptExpression::ScriptHash,
        ScriptExpression::WitnessScriptHash,
    ];
    Ok(CryptoOutput::new(
        script_expressions,
        None,
        Some(generate_multi_sig_crypto_hd_key(
            master_fingerprint,
            extended_public_key,
            if *network == Network::MainNet {
                MultiSigType::P2wshP2sh
            } else {
                MultiSigType::P2wshP2shTest
            },
        )?),
        None,
    ))
}

fn generate_multi_sig_p2wsh_output(
    master_fingerprint: &[u8; 4],
    extended_public_key: &str,
    network: &Network,
) -> URResult<CryptoOutput> {
    let script_expressions = vec![ScriptExpression::WitnessScriptHash];
    Ok(CryptoOutput::new(
        script_expressions,
        None,
        Some(generate_multi_sig_crypto_hd_key(
            master_fingerprint,
            extended_public_key,
            if *network == Network::MainNet {
                MultiSigType::P2wsh
            } else {
                MultiSigType::P2wshTest
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
    let bip32_extended_pub_key = bip32::Xpub::from_str(extended_public_key)
        .map_err(|e| URError::UrEncodeError(e.to_string()))?;
    let parent_fingerprint = bip32_extended_pub_key.parent_fingerprint;

    let network_inner;
    let path_components = match multi_sig_type {
        MultiSigType::P2sh => {
            network_inner = NetworkInner::MainNet;
            vec![get_path_component(Some(45), true)?]
        }
        MultiSigType::P2wshP2sh => {
            network_inner = NetworkInner::MainNet;
            vec![
                get_path_component(Some(48), true)?,
                get_path_component(Some(0), true)?,
                get_path_component(Some(0), true)?,
                get_path_component(Some(1), true)?,
            ]
        }
        MultiSigType::P2wsh => {
            network_inner = NetworkInner::MainNet;
            vec![
                get_path_component(Some(48), true)?,
                get_path_component(Some(0), true)?,
                get_path_component(Some(0), true)?,
                get_path_component(Some(2), true)?,
            ]
        }
        MultiSigType::P2shTest => {
            network_inner = NetworkInner::TestNet;
            vec![get_path_component(Some(45), true)?]
        }
        MultiSigType::P2wshP2shTest => {
            network_inner = NetworkInner::TestNet;
            vec![
                get_path_component(Some(48), true)?,
                get_path_component(Some(1), true)?,
                get_path_component(Some(0), true)?,
                get_path_component(Some(1), true)?,
            ]
        }
        MultiSigType::P2wshTest => {
            network_inner = NetworkInner::TestNet;
            vec![
                get_path_component(Some(48), true)?,
                get_path_component(Some(1), true)?,
                get_path_component(Some(0), true)?,
                get_path_component(Some(2), true)?,
            ]
        }
    };

    let coin_info = CryptoCoinInfo::new(Some(CoinType::Bitcoin), Some(network_inner));

    let origin = CryptoKeyPath::new(
        path_components,
        Some(master_fingerprint.clone()),
        Some(bip32_extended_pub_key.depth as u32),
    );

    let hd_key = CryptoHDKey::new_extended_key(
        Some(false),
        Vec::from(bip32_extended_pub_key.public_key.serialize()),
        Some(bip32_extended_pub_key.chain_code[..].to_vec()),
        Some(coin_info),
        Some(origin),
        None,
        Some(parent_fingerprint.to_bytes()),
        None,
        None,
    );

    Ok(hd_key)
}

fn get_path_component(index: Option<u32>, hardened: bool) -> URResult<PathComponent> {
    PathComponent::new(index, hardened).map_err(|e| URError::CborEncodeError(e))
}

fn is_valid_multi_path(path: &str) -> bool {
    const VALID_PATHS: [&str; 5] = [
        MULTI_P2SH_PATH,
        MULTI_P2WSH_P2SH_PATH,
        MULTI_P2WSH_P2SH_PATH_TEST,
        MULTI_P2WSH_PATH,
        MULTI_P2WSH_PATH_TEST,
    ];
    VALID_PATHS
        .iter()
        .any(|valid_path| path.starts_with(valid_path))
}

fn convert_xpub(path: &str, xpub: &str, network: &Network) -> Result<String, BitcoinError> {
    match (path, network) {
        (_, _) if path.starts_with(MULTI_P2SH_PATH) => {
            if *network == Network::MainNet {
                convert_version(xpub, &Version::Xpub)
            } else {
                convert_version(xpub, &Version::Tpub)
            }
        }
        (_, _) if path.starts_with(MULTI_P2WSH_P2SH_PATH) => {
            convert_version(xpub, &Version::YpubMultisig)
        }
        (_, _) if path.starts_with(MULTI_P2WSH_P2SH_PATH_TEST) => {
            convert_version(xpub, &Version::UpubMultisig)
        }
        (_, _) if path.starts_with(MULTI_P2WSH_PATH) => {
            convert_version(xpub, &Version::ZpubMultisig)
        }
        (_, _) if path.starts_with(MULTI_P2WSH_PATH_TEST) => {
            convert_version(xpub, &Version::VpubMultisig)
        }
        _ => {
            if *network == Network::MainNet {
                convert_version(xpub, &Version::Xpub)
            } else {
                convert_version(xpub, &Version::Tpub)
            }
        }
        // _ => Err(BitcoinError::MultiSigWalletImportXpubError(
        //     "have no xpub version matching path".to_string(),
        // )),
    }
}

#[cfg(test)]
mod tests {
    extern crate std;

    use alloc::vec::Vec;

    use crate::multi_sig::{
        export_xpub_by_crypto_account, extract_xpub_info_from_bytes,
        extract_xpub_info_from_crypto_account, extract_xpub_info_from_str, MultiSigFormat,
        MultiSigType, Network,
    };
    use third_party::hex;
    use third_party::hex::FromHex;
    use third_party::ur_registry::bytes::Bytes;
    use third_party::ur_registry::crypto_account::CryptoAccount;

    #[test]
    fn test_generate_multi_sig_crypto_account() {
        let mfp = "73C5DA0A";
        let mfp = Vec::from_hex(mfp).unwrap();
        let mfp: [u8; 4] = mfp.try_into().unwrap();

        let p2sh_path = "m/45'";
        let p2sh_path_xpub = "xpub68jrRzQopSUSiczuqjRwvVn3CFtSEZY6a3jbT66LM3tvt1rXtYT7Udi8dt3m1qj3q8pKZjt7tqrSt7bRN4LD2vSVq1167PSA5AyM31FUHwU";

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
                &[p2sh_path_xpub],
                &[p2sh_path],
                Network::MainNet,
            )
            .unwrap();
            let cbor: Vec<u8> = account.try_into().unwrap();
            assert_eq!("a2011a73c5da0a0281d90190d9012fa602f403582103c8f3bcfbaf73b709e89807afacf25519ba04314b1c9ce3651d5e11af6b40d2710458207b6f617e57a7d09a7080b35e15c9866fb711d375402cacbb96f131d1f71a77c705d90131a20100020006d90130a30182182df5021a73c5da0a0301081a73c5da0a",
                       hex::encode(cbor).to_lowercase()
            );
        }

        // p2wsh_p2sh  mainnet
        {
            let account = export_xpub_by_crypto_account(
                &mfp,
                &[p2wsh_p2sh_xpub],
                &[p2wsh_p2sh_path],
                Network::MainNet,
            )
            .unwrap();
            let cbor: Vec<u8> = account.try_into().unwrap();
            assert_eq!("a2011a73c5da0a0281d90190d90191d9012fa602f4035821036da4c96b78e8ad5cc3556bb464e1ad05132cccb0098eda9337d3f882241796aa045820ae2afa5bbcc9c85c2c9aefb0a89f27614878b1dfc7ac7c38d8614c64ffbfce7005d90131a20100020006d90130a301881830f500f500f501f5021a73c5da0a0304081a1cf29716",
                       hex::encode(cbor).to_lowercase()
            );
        }

        // p2wsh  mainnet
        {
            let account =
                export_xpub_by_crypto_account(&mfp, &[p2wsh_xpub], &[p2wsh_path], Network::MainNet)
                    .unwrap();
            let cbor: Vec<u8> = account.try_into().unwrap();
            assert_eq!("a2011a73c5da0a0281d90191d9012fa602f4035821021a3bf5fbf737d0f36993fd46dc4913093beb532d654fe0dfd98bd27585dc9f29045820bba0c7ca160a870efeb940ab90d0f4284fea1b5e0d2117677e823fc37e2d576305d90131a20100020006d90130a301881830f500f500f502f5021a73c5da0a0304081a1cf29716",
                       hex::encode(cbor).to_lowercase()
            );
        }

        // p2sh  testnet
        {
            let account = export_xpub_by_crypto_account(
                &mfp,
                &[p2sh_path_xpub],
                &[p2sh_path],
                Network::TestNet,
            )
            .unwrap();
            let cbor: Vec<u8> = account.try_into().unwrap();
            assert_eq!("a2011a73c5da0a0281d90190d9012fa602f403582103c8f3bcfbaf73b709e89807afacf25519ba04314b1c9ce3651d5e11af6b40d2710458207b6f617e57a7d09a7080b35e15c9866fb711d375402cacbb96f131d1f71a77c705d90131a20100020106d90130a30182182df5021a73c5da0a0301081a73c5da0a",
                       hex::encode(cbor).to_lowercase()
            );
        }

        // p2wsh_p2sh  testnet
        {
            let account = export_xpub_by_crypto_account(
                &mfp,
                &[p2wsh_p2sh_xpub_test],
                &[p2wsh_p2sh_path_test],
                Network::TestNet,
            )
            .unwrap();
            let cbor: Vec<u8> = account.try_into().unwrap();
            assert_eq!("a2011a73c5da0a0281d90190d90191d9012fa602f403582102e62a2a9973ee6b3a7af47c229a5bde70bca59bd04bbb297f5693d7aa256b976d045820a73adbe2878487634dcbfc3f7ebde8b1fc994f1ec06860cf01c3fe2ea791ddb605d90131a20100020106d90130a301881830f501f500f501f5021a73c5da0a0304081abac14839",
                       hex::encode(cbor).to_lowercase()
            );
        }

        // p2wsh  testnet
        {
            let account = export_xpub_by_crypto_account(
                &mfp,
                &[p2wsh_xpub_test],
                &[p2wsh_path_test],
                Network::TestNet,
            )
            .unwrap();
            let cbor: Vec<u8> = account.try_into().unwrap();
            assert_eq!("a2011a73c5da0a0281d90191d9012fa602f403582103568ea1f36051916ed1b690c39e12a8e70603b280bd30ce5f281f2918a55363aa0458201d4fbebdd967e1af714c0997d38fcf670b5ce9d301c0440bbcc3b6a20eb7721c05d90131a20100020106d90130a301881830f501f500f502f5021a73c5da0a0304081abac14839",
                       hex::encode(cbor).to_lowercase()
            );
        }
    }

    #[test]
    fn test_crypto_account_to_multi_sig_xpub_info() {
        // p2sh  mainnet
        {
            let bytes = Vec::from_hex(
                "a2011a73c5da0a0281d90190d9012fa602f403582103c8f3bcfbaf73b709e89807afacf25519ba04314b1c9ce3651d5e11af6b40d2710458207b6f617e57a7d09a7080b35e15c9866fb711d375402cacbb96f131d1f71a77c705d90131a20100020006d90130a30182182df5021a73c5da0a0301081a73c5da0a",
            )
                .unwrap();
            let account = CryptoAccount::try_from(bytes).unwrap();

            let result =
                extract_xpub_info_from_crypto_account(&account, MultiSigType::P2sh).unwrap();

            assert_eq!("m/45'", result.path);
            assert_eq!("73C5DA0A", result.xfp);
            assert_eq!("xpub68jrRzQopSUSiczuqjRwvVn3CFtSEZY6a3jbT66LM3tvt1rXtYT7Udi8dt3m1qj3q8pKZjt7tqrSt7bRN4LD2vSVq1167PSA5AyM31FUHwU", result.xpub);

            println!("reuslt is {:?}", result)
        }

        // p2wsh_p2sh  mainnet
        {
            let bytes = Vec::from_hex(
                "a2011a73c5da0a0281d90190d90191d9012fa602f4035821036da4c96b78e8ad5cc3556bb464e1ad05132cccb0098eda9337d3f882241796aa045820ae2afa5bbcc9c85c2c9aefb0a89f27614878b1dfc7ac7c38d8614c64ffbfce7005d90131a20100020006d90130a301881830f500f500f501f5021a73c5da0a0304081a1cf29716",
            )
                .unwrap();
            let account = CryptoAccount::try_from(bytes).unwrap();

            let result =
                extract_xpub_info_from_crypto_account(&account, MultiSigType::P2wshP2sh).unwrap();

            assert_eq!("m/48'/0'/0'/1'", result.path);
            assert_eq!("73C5DA0A", result.xfp);
            assert_eq!("Ypub6jUbbRukkGPp4DgJDD4HL2NKkSZ1UPk111mg59XtJRQZHvJ6XqvJzrntik9U4jCFQkgrBqevdKLPMdYZXU9KAGhKpMhW5XujwqiQ7Csmm4Z", result.xpub);

            println!("reuslt is {:?}", result)
        }

        // p2wsh  mainnet
        {
            let bytes = Vec::from_hex(
                "a2011a73c5da0a0281d90191d9012fa602f4035821021a3bf5fbf737d0f36993fd46dc4913093beb532d654fe0dfd98bd27585dc9f29045820bba0c7ca160a870efeb940ab90d0f4284fea1b5e0d2117677e823fc37e2d576305d90131a20100020006d90130a301881830f500f500f502f5021a73c5da0a0304081a1cf29716",
            )
                .unwrap();
            let account = CryptoAccount::try_from(bytes).unwrap();
            let result =
                extract_xpub_info_from_crypto_account(&account, MultiSigType::P2wsh).unwrap();

            assert_eq!("m/48'/0'/0'/2'", result.path);
            assert_eq!("73C5DA0A", result.xfp);
            assert_eq!("Zpub74Jru6aftwwHxCUCWEvP6DgrfFsdA4U6ZRtQ5i8qJpMcC39yZGv3egBhQfV3MS9pZtH5z8iV5qWkJsK6ESs6mSzt4qvGhzJxPeeVS2e1zUG", result.xpub);

            println!("reuslt is {:?}", result)
        }

        // p2sh  testnet
        {
            let bytes = Vec::from_hex(
                "a2011a73c5da0a0281d90190d9012fa602f403582103c8f3bcfbaf73b709e89807afacf25519ba04314b1c9ce3651d5e11af6b40d2710458207b6f617e57a7d09a7080b35e15c9866fb711d375402cacbb96f131d1f71a77c705d90131a20100020106d90130a30182182df5021a73c5da0a0301081a73c5da0a",
            )
                .unwrap();
            let account = CryptoAccount::try_from(bytes).unwrap();

            let result =
                extract_xpub_info_from_crypto_account(&account, MultiSigType::P2sh).unwrap();

            assert_eq!("m/45'", result.path);
            assert_eq!("73C5DA0A", result.xfp);
            assert_eq!("tpubD97UxEEVXiRtzRBmHvR38R7QXNz6Dx3A7gKtoe9UgxepdJXExmJCd5Nxsv8YYLgHd3MEBKPzRwgVaJ62kvBSvMtntbkPnv6Pf8Zkny5rC89", result.xpub);

            println!("reuslt is {:?}", result)
        }

        // p2wsh_p2sh  testnet
        {
            let bytes = Vec::from_hex(
                "a2011a73c5da0a0281d90190d90191d9012fa602f403582102e62a2a9973ee6b3a7af47c229a5bde70bca59bd04bbb297f5693d7aa256b976d045820a73adbe2878487634dcbfc3f7ebde8b1fc994f1ec06860cf01c3fe2ea791ddb605d90131a20100020106d90130a301881830f501f500f501f5021a73c5da0a0304081abac14839",
            )
                .unwrap();
            let account = CryptoAccount::try_from(bytes).unwrap();
            let result =
                extract_xpub_info_from_crypto_account(&account, MultiSigType::P2wshP2shTest)
                    .unwrap();

            assert_eq!("m/48'/1'/0'/1'", result.path);
            assert_eq!("73C5DA0A", result.xfp);
            assert_eq!("Upub5TJpKgtw4cBcaAom7tyqG1yU3gSsjTVPkwWuR97vgrChHsT4S6M9d3BJ3jRmUgCUJZ58GUZhkWt6eGUVM7sdizaeuZqvC61TGRSP43VHvGm", result.xpub);

            println!("reuslt is {:?}", result)
        }

        // p2wsh  testnet
        {
            let bytes = Vec::from_hex(
                "a2011a73c5da0a0281d90191d9012fa602f403582103568ea1f36051916ed1b690c39e12a8e70603b280bd30ce5f281f2918a55363aa0458201d4fbebdd967e1af714c0997d38fcf670b5ce9d301c0440bbcc3b6a20eb7721c05d90131a20100020106d90130a301881830f501f500f502f5021a73c5da0a0304081abac14839",
            )
                .unwrap();
            let account = CryptoAccount::try_from(bytes).unwrap();

            let result =
                extract_xpub_info_from_crypto_account(&account, MultiSigType::P2wshTest).unwrap();

            assert_eq!("m/48'/1'/0'/2'", result.path);
            assert_eq!("73C5DA0A", result.xfp);
            assert_eq!("Vpub5n95dMZrDHj6SeBgJ1oz4Fae2N2eJNuWK3VTKDb2dzGpMFLUHLmtyDfen7AaQxwQ5mZnMyXdVrkEaoMLVTH8FmVBRVWPGFYWhmtDUGehGmq", result.xpub);

            println!("reuslt is {:?}", result)
        }
    }

    #[test]
    fn test_extract_xpub_info_from_str() {
        let json_str = r#"{
  "p2sh_deriv": "m/45'",
  "p2sh": "xpub69cicR2MFe9QbMVTMHN882fGtXBKQV4g9gqWNZN7aEM9RASi3WmUzgPF9md8fLfUNuF4znQ8937VQrjG2bG8VgU7rjhUR8qCfBL9hJDQogL",
  "p2sh_desc": "sh(sortedmulti(M,[eb16731f/45']xpub69cicR2MFe9QbMVTMHN882fGtXBKQV4g9gqWNZN7aEM9RASi3WmUzgPF9md8fLfUNuF4znQ8937VQrjG2bG8VgU7rjhUR8qCfBL9hJDQogL/0/*,...))",
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

        assert_eq!("m/45'", result.path);
        assert_eq!("EB16731F", result.xfp);
        assert_eq!("xpub69cicR2MFe9QbMVTMHN882fGtXBKQV4g9gqWNZN7aEM9RASi3WmUzgPF9md8fLfUNuF4znQ8937VQrjG2bG8VgU7rjhUR8qCfBL9hJDQogL", result.xpub);

        let result = extract_xpub_info_from_str(json_str, &MultiSigFormat::P2wshP2sh).unwrap();
        println!("result is {:?}", result);

        assert_eq!("m/48'/0'/0'/1'", result.path);
        assert_eq!("EB16731F", result.xfp);
        assert_eq!("Ypub6jwcqK3XjbvvGXVFK6ghVrrqctFYJjFPh4ZiQ6ZFfMKxZSe5FWzZX3ib3THzRy2UKsesZwXmXVGsez8pyMuDmdbhnN55RcZbTLj7rFqViqo", result.xpub);

        let result = extract_xpub_info_from_str(json_str, &MultiSigFormat::P2wsh).unwrap();
        println!("result is {:?}", result);

        assert_eq!("m/48'/0'/0'/2'", result.path);
        assert_eq!("EB16731F", result.xfp);
        assert_eq!("Zpub74mt8yiStHUQASjHhGjAeBL6oU1PG8NCUQRRxucP9Dvwz7p65oHChnf8tQrhZfkCPesk7vays9n8cyLDDRM7Wqxmz2mihQHSikXfEwyp6kt", result.xpub);
    }

    #[test]
    fn test_extract_xpub_info_from_bytes() {
        let cbor = "59042c7b0a202022703273685f6465726976223a20226d2f343527222c0a20202270327368223a202278707562363963696352324d46653951624d56544d484e38383266477458424b51563467396771574e5a4e3761454d395241536933576d557a675046396d6438664c66554e7546347a6e51383933375651726a473262473856675537726a68555238714366424c39684a44516f674c222c0a202022703273685f64657363223a2022736828736f727465646d756c7469284d2c5b65623136373331662f3435275d78707562363963696352324d46653951624d56544d484e38383266477458424b51563467396771574e5a4e3761454d395241536933576d557a675046396d6438664c66554e7546347a6e51383933375651726a473262473856675537726a68555238714366424c39684a44516f674c2f302f2a2c2e2e2e2929222c0a202022703273685f70327773685f6465726976223a20226d2f3438272f30272f30272f3127222c0a202022703273685f7032777368223a202259707562366a7763714b33586a627676475856464b36676856727271637446594a6a465068345a6951365a46664d4b785a53653546577a5a583369623354487a527932554b7365735a77586d58564773657a3870794d75446d6462686e4e353552635a62544c6a377246715669716f222c0a202022703273685f70327773685f64657363223a202273682877736828736f727465646d756c7469284d2c5b65623136373331662f3438272f30272f30272f31275d787075623645444751516542317871347a66386b59355336546852586a383471396b614a54675070685350707561376674414677455453696f73435844594e7673643965676d553577737736426358706558755678534c4670756f53355a4746534a483748745062736a477a4a72482f302f2a2c2e2e2e292929222c0a20202270327773685f6465726976223a20226d2f3438272f30272f30272f3227222c0a2020227032777368223a20225a70756237346d74387969537448555141536a4868476a4165424c366f55315047384e435551525278756350394476777a377036356f4843686e6638745172685a666b435065736b3776617973396e3863794c4444524d375771786d7a326d6968514853696b586645777970366b74222c0a20202270327773685f64657363223a202277736828736f727465646d756c7469284d2c5b65623136373331662f3438272f30272f30272f32275d78707562364544475151654231787135334842673574677750766f486a6a6745415868634b756a4b55725a3531534c6e466a636970355a6f4e59557733487a34315244544c7561396b50506b346367586a45564b556f4e386d745575517447553842425548613856737264366b376f2f302f2a2c2e2e2e2929222c0a2020226163636f756e74223a202230222c0a202022786670223a20224542313637333146220a7d0a";
        let bytes = Bytes::try_from(Vec::from_hex(cbor).unwrap()).unwrap();

        let result = extract_xpub_info_from_bytes(bytes, &MultiSigFormat::P2wsh).unwrap();

        assert_eq!("m/48'/0'/0'/2'", result.path);
        assert_eq!("EB16731F", result.xfp);
        assert_eq!("Zpub74mt8yiStHUQASjHhGjAeBL6oU1PG8NCUQRRxucP9Dvwz7p65oHChnf8tQrhZfkCPesk7vays9n8cyLDDRM7Wqxmz2mihQHSikXfEwyp6kt", result.xpub);
    }
}
