use alloc::{
    string::{String, ToString},
    vec::Vec,
};

use cryptoxide::hashing::sha256;
use hex;
use itertools::Itertools;
use ur_registry::bytes::Bytes;

use zcash_vendor::{bip32, secp256k1, zcash_script::descriptor};

use crate::{
    multi_sig::{MultiSigFormat, MultiSigXPubInfo, Network, MULTI_P2SH_PATH, MULTI_P2SH_PATH_TEST},
    ZcashError,
};

pub(crate) fn fixed_xfp(fingerprint: [u8; 4]) -> descriptor::KeyOrigin {
    descriptor::KeyOrigin::from_parts(fingerprint, vec![])
}

pub(crate) fn convert_version(
    key: descriptor::Key,
    version: bip32::Prefix,
) -> Result<descriptor::Key, ZcashError> {
    match key {
        descriptor::Key::Public { .. } => Err(ZcashError::MultiSigWalletAddressCalError(
            "key is not xpub".to_string(),
        )),

        descriptor::Key::Xpub { key, child, .. } => Ok(descriptor::Key::Xpub {
            prefix: version,
            key,
            child,
        }),
    }
}

#[derive(Debug, Clone)]
pub struct MultiSigXPubItem {
    pub path: bip32::DerivationPath,
    pub key_expr: descriptor::KeyExpression,
}

#[derive(Debug)]
pub struct MultiSigWalletConfig {
    pub creator: String,
    pub name: String,
    pub threshold: u8,
    pub total: u8,
    pub derivations: Vec<String>,
    pub format: String,
    pub xpub_items: Vec<descriptor::KeyExpression>,
    pub verify_code: String,
    pub verify_without_mfp: String,
    pub config_text: String,
    pub network: Network,
}

impl Default for MultiSigWalletConfig {
    fn default() -> Self {
        MultiSigWalletConfig {
            creator: String::new(),
            name: String::new(),
            threshold: 0,
            total: 0,
            derivations: vec![],
            format: String::new(),
            xpub_items: vec![],
            verify_code: String::new(),
            verify_without_mfp: String::new(),
            config_text: String::new(),
            network: Network::MainNetwork,
        }
    }
}

impl MultiSigWalletConfig {
    pub fn get_network(&self) -> &Network {
        &self.network
    }

    pub fn get_network_u32(&self) -> u32 {
        match &self.network {
            Network::MainNetwork => 0,
            Network::TestNetwork => 1,
        }
    }

    pub fn get_wallet_path(&self) -> Result<&str, ZcashError> {
        let path = match (self.format.as_str(), self.get_network()) {
            ("P2SH", Network::MainNetwork) => MULTI_P2SH_PATH,
            ("P2SH", Network::TestNetwork) => MULTI_P2SH_PATH_TEST,
            _ => {
                return Err(ZcashError::MultiSigWalletParseError(format!(
                    "not support format {}",
                    self.format.as_str()
                )));
            }
        };
        Ok(path)
    }

    pub fn get_derivation_by_index(&self, index: usize) -> Option<String> {
        let path = if self.derivations.len() == 1 {
            self.derivations.first()
        } else {
            self.derivations.get(index)
        };

        path.map(|p| p.strip_prefix("m/").unwrap_or(p).to_string())
    }
}

#[derive(Debug)]
pub struct BsmsWallet {
    pub bsms_version: String,
    pub xfp: String,
    pub derivation_path: String,
    pub extended_pubkey: String,
}

impl Default for BsmsWallet {
    fn default() -> Self {
        BsmsWallet {
            bsms_version: String::from("BSMS 1.0"),
            xfp: String::new(),
            derivation_path: String::new(),
            extended_pubkey: String::new(),
        }
    }
}

fn _parse_plain_xpub_config(content: &str) -> Result<BsmsWallet, ZcashError> {
    let mut bsms_wallet = BsmsWallet::default();

    for line in content.lines() {
        if line.trim().starts_with("BSMS") {
            if line.contains("BSMS 1.0") {
                bsms_wallet.bsms_version = String::from("BSMS 1.0");
            }
        } else if line.starts_with("[") {
            if let Some(end_bracket_pos) = line.find(']') {
                if end_bracket_pos >= 9 {
                    let xfp = &line[1..=8].trim();
                    let derivation_path = &line[9..=end_bracket_pos - 1].trim();
                    let extended_pubkey = &line[end_bracket_pos + 1..].trim();
                    bsms_wallet.xfp = xfp.to_string();
                    bsms_wallet.derivation_path = format!("m{derivation_path}");
                    bsms_wallet.extended_pubkey = extended_pubkey.to_string();
                    return Ok(bsms_wallet);
                }
            }
        }
    }
    Err(ZcashError::MultiSigWalletImportXpubError(String::from(
        "not a valid xpub config",
    )))
}

pub fn parse_bsms_wallet_config(bytes: Bytes) -> Result<BsmsWallet, ZcashError> {
    let content = String::from_utf8(bytes.get_bytes())
        .map_err(|e| ZcashError::MultiSigWalletImportXpubError(e.to_string()))?;
    let wallet = _parse_plain_xpub_config(&content)?;
    Ok(wallet)
}

pub fn create_wallet(
    creator: &str,
    name: &str,
    threshold: u8,
    total: u8,
    format: &str,
    xpub_infos: &Vec<MultiSigXPubInfo>,
    network: Network,
    xfp: &descriptor::KeyOrigin,
) -> Result<MultiSigWalletConfig, ZcashError> {
    is_valid_multi_sig_policy(total, threshold).map_err(ZcashError::MultiSigWalletCreateError)?;

    let mut wallet = MultiSigWalletConfig::default();
    wallet.network = network;

    let is_same_path = xpub_infos.iter().all(|x| x.path == xpub_infos[0].path);

    if is_same_path {
        wallet.derivations.push(xpub_infos[0].path.to_string());
    } else {
        wallet
            .derivations
            .extend(xpub_infos.iter().map(|x| x.path.to_string()));
    }

    wallet
        .xpub_items
        .extend(xpub_infos.iter().map(|x| x.key_expr.clone()));

    wallet.creator = creator.to_string();
    wallet.total = total;
    wallet.threshold = threshold;
    wallet.format = format.to_string();
    wallet.name = name.to_string();

    verify_wallet_config(&wallet, xfp)?;
    wallet.config_text = generate_config_data(&wallet, xfp)?;
    calculate_wallet_verify_code(&mut wallet, xfp)?;
    Ok(wallet)
}

fn _parse_plain_wallet_config(content: &str) -> Result<MultiSigWalletConfig, ZcashError> {
    let mut wallet = MultiSigWalletConfig::default();

    let mut is_first = true;

    for line in content.lines() {
        if line.trim().starts_with("#") {
            if line.contains("Coldcard") {
                wallet.creator = String::from("Coldcard");
            } else if line.contains("Keystone") {
                wallet.creator = String::from("Keystone");
            }
        } else {
            let splits = line.split(":").collect::<Vec<_>>();
            if splits.len() != 2 {
                continue;
            }

            let label = splits[0].trim();
            let value = splits[1].trim();

            match label {
                "Name" => wallet.name = String::from(value),
                "Policy" => parse_and_set_policy(&mut wallet, value)?,
                "Format" => wallet.format = String::from(value),
                "Derivation" => wallet.derivations.push(value.replace("h", "\'")),
                _ => format!("[{label}]{value}")
                    .parse()
                    .map_err(|e: descriptor::ParseError| {
                        ZcashError::MultiSigWalletParseError(e.to_string())
                    })
                    .and_then(|pk| process_xpub_and_xfp(&mut wallet, pk))?,
            }
        }
    }

    for xpub_item in wallet.xpub_items.iter() {
        let this_network = detect_network(&xpub_item.clone().into_parts().1);
        if this_network == Network::TestNetwork {
            return Err(ZcashError::MultiSigWalletParseError(
                "we don't support testnet for multisig yet".to_string(),
            ));
        }
        if is_first {
            wallet.network = this_network;
            is_first = false;
        } else if wallet.network != this_network {
            return Err(ZcashError::MultiSigWalletParseError(
                "xpub networks inconsistent".to_string(),
            ));
        }
    }

    wallet.config_text = content.to_string();
    Ok(wallet)
}

pub fn parse_wallet_config(
    content: &str,
    xfp: &descriptor::KeyOrigin,
) -> Result<MultiSigWalletConfig, ZcashError> {
    let mut wallet = _parse_plain_wallet_config(content)?;
    verify_wallet_config(&wallet, xfp)?;
    calculate_wallet_verify_code(&mut wallet, xfp)?;
    Ok(wallet)
}

pub fn export_wallet_by_ur(
    config: &MultiSigWalletConfig,
    xfp: &descriptor::KeyOrigin,
) -> Result<Bytes, ZcashError> {
    let config_data = generate_config_data(config, xfp)?;
    Ok(Bytes::new(config_data.into_bytes()))
}

pub fn generate_config_data(
    config: &MultiSigWalletConfig,
    xfp: &descriptor::KeyOrigin,
) -> Result<String, ZcashError> {
    let mut config_data = String::new();
    let network = config.network;

    let policy = format!("{} of {}", config.threshold, config.total);
    config_data.push_str(&format!(
        "# {} Multisig setup file (created on {})\n",
        config.creator,
        hex::encode(xfp.fingerprint())
    ));
    config_data.push_str("#\n");
    config_data.push_str(&format!("Name: {}\n", config.name));
    config_data.push_str(&format!("Policy: {policy}\n"));

    if config.derivations.len() == 1 {
        config_data.push_str(&format!("Derivation: {}\n", config.derivations[0]));
        config_data.push_str(&format!("Format: {}\n", config.format));
        config_data.push('\n');
        let xpub_items = config
            .xpub_items
            .iter()
            .map(|item| {
                let (xfp, key) = item.clone().into_parts();
                match xfp {
                    None => unreachable!("should have an XFP for all keys here"),
                    Some(xfp) => Ok(format!("{}: {key}", hex::encode(xfp.fingerprint()))),
                }
            })
            .collect::<Result<Vec<String>, ZcashError>>()?;

        config_data.push_str(&xpub_items.join("\n"));
    } else {
        config_data.push_str(&format!("Format: {}\n", config.format));

        for (index, item) in config.xpub_items.iter().enumerate() {
            config_data.push_str(&format!("\nDerivation: {}\n", config.derivations[index]));
            let (xfp, key) = item.clone().into_parts();
            match xfp {
                None => unreachable!("should have an XFP for all keys here"),
                Some(xfp) => {
                    config_data.push_str(&format!("{}: {key}", hex::encode(xfp.fingerprint())))
                }
            }
        }
    }

    Ok(config_data)
}

pub fn import_wallet_by_ur(
    bytes: &Bytes,
    xfp: &descriptor::KeyOrigin,
) -> Result<MultiSigWalletConfig, ZcashError> {
    let data = String::from_utf8(bytes.get_bytes())
        .map_err(|e| ZcashError::MultiSigWalletImportXpubError(e.to_string()))?;

    parse_wallet_config(&data, xfp)
}

pub fn is_valid_xpub_config(bytes: &Bytes) -> bool {
    if let Ok(d) = String::from_utf8(bytes.get_bytes())
        .map_err(|e| ZcashError::MultiSigWalletImportXpubError(e.to_string()))
    {
        if _parse_plain_xpub_config(&d).is_ok() {
            return true;
        }
    }
    false
}

pub fn is_valid_wallet_config(bytes: &Bytes) -> bool {
    if let Ok(d) = String::from_utf8(bytes.get_bytes())
        .map_err(|e| ZcashError::MultiSigWalletImportXpubError(e.to_string()))
    {
        if _parse_plain_wallet_config(&d).is_ok() {
            return true;
        }
    }
    false
}

fn parse_and_set_policy(wallet: &mut MultiSigWalletConfig, value: &str) -> Result<(), ZcashError> {
    let policy = value.split(" of ").collect::<Vec<_>>();
    if policy.len() == 2 {
        let threshold: u8 = policy[0].parse().map_err(|_e| {
            ZcashError::MultiSigWalletParseError("parse threshold error".to_string())
        })?;
        let total: u8 = policy[1]
            .parse()
            .map_err(|_e| ZcashError::MultiSigWalletParseError("parse total error".to_string()))?;

        is_valid_multi_sig_policy(total, threshold)
            .map_err(|e| ZcashError::MultiSigWalletParseError(e.to_string()))?;
        wallet.threshold = threshold.clone();
        wallet.total = total.clone();
    }

    Ok(())
}

fn process_xpub_and_xfp(
    wallet: &mut MultiSigWalletConfig,
    key_expr: descriptor::KeyExpression,
) -> Result<(), ZcashError> {
    let (xfp, key) = key_expr.clone().into_parts();
    match xfp {
        None => {
            return Err(ZcashError::MultiSigWalletParseError(
                "this is not a valid xfp".to_string(),
            ))
        }
        Some(_) => match key {
            descriptor::Key::Public { .. } => {
                return Err(ZcashError::MultiSigWalletParseError(
                    "this is not a valid xpub".to_string(),
                ))
            }
            descriptor::Key::Xpub { .. } => {
                for xpub_item in wallet.xpub_items.iter() {
                    let result1 =
                        convert_version(xpub_item.clone().into_parts().1, bip32::Prefix::XPUB)?;
                    let result2 =
                        convert_version(key_expr.clone().into_parts().1, bip32::Prefix::XPUB)?;
                    // TODO: Why no Eq?
                    if result1.to_string() == result2.to_string() {
                        return Err(ZcashError::MultiSigWalletParseError(
                            "found duplicated xpub".to_string(),
                        ));
                    }
                }
            }
        },
    }
    wallet.xpub_items.push(key_expr);
    Ok(())
}

fn is_valid_multi_sig_policy(total: u8, threshold: u8) -> Result<(), descriptor::Error> {
    if total < 2 {
        Err(descriptor::Error::NoPubKeys)
    } else if 15 < total {
        Err(descriptor::Error::TooManyPubKeys(total.into()))
    } else if threshold < 1 || total < threshold {
        Err(descriptor::Error::InvalidThreshold(threshold, total))
    } else {
        Ok(())
    }
}

fn detect_network(xpub: &descriptor::Key) -> Network {
    match xpub {
        descriptor::Key::Public { .. } => unreachable!("all keys are Xpub"),
        descriptor::Key::Xpub { prefix, .. } => match *prefix {
            bip32::Prefix::TPUB => Network::TestNetwork,
            _ => Network::MainNetwork,
        },
    }
}

fn verify_wallet_config(
    wallet: &MultiSigWalletConfig,
    xfp: &descriptor::KeyOrigin,
) -> Result<(), ZcashError> {
    if wallet.xpub_items.is_empty() {
        return Err(ZcashError::MultiSigWalletParseError(
            "have no xpub in config file".to_string(),
        ));
    }

    if usize::from(wallet.total) != wallet.xpub_items.len() {
        return Err(ZcashError::MultiSigWalletParseError(format!(
            "multisig wallet requires {} cosigners, but found {}",
            wallet.total,
            wallet.xpub_items.len()
        )));
    }

    if !wallet.xpub_items.iter().any(|item| {
        &item
            .clone()
            .into_parts()
            .0
            .map(|origin| *origin.fingerprint())
            == &Some(*xfp.fingerprint())
    }) {
        return Err(ZcashError::MultiSigWalletNotMyWallet);
    }

    if wallet.derivations.len() != 1 && wallet.derivations.len() != wallet.xpub_items.len() {
        return Err(ZcashError::MultiSigWalletParseError(
            "num of derivations is not right".to_string(),
        ));
    }
    Ok(())
}

fn calculate_wallet_verify_code(
    wallet: &mut MultiSigWalletConfig,
    xfp: &descriptor::KeyOrigin,
) -> Result<(), ZcashError> {
    wallet.verify_without_mfp = calculate_multi_sig_verify_code(
        &wallet.xpub_items,
        wallet.threshold as u8,
        wallet.total as u8,
        MultiSigFormat::from(&wallet.format)?,
        wallet.get_network(),
        None,
    )?;
    wallet.verify_code = calculate_multi_sig_verify_code(
        &wallet.xpub_items,
        wallet.threshold as u8,
        wallet.total as u8,
        MultiSigFormat::from(&wallet.format)?,
        wallet.get_network(),
        Some(xfp.clone()),
    )?;
    Ok(())
}

pub fn calculate_multi_sig_verify_code(
    xpubs: &[descriptor::KeyExpression],
    threshold: u8,
    total: u8,
    format: MultiSigFormat,
    network: &Network,
    xfp: Option<descriptor::KeyOrigin>,
) -> Result<String, ZcashError> {
    let join_xpubs = xpubs
        .iter()
        .map(|x| x.clone().into_parts().1.to_string())
        .sorted()
        .join(" ");

    let path = match (format, network) {
        (MultiSigFormat::P2sh, Network::MainNetwork) => MULTI_P2SH_PATH,
        (MultiSigFormat::P2sh, Network::TestNetwork) => MULTI_P2SH_PATH_TEST,
    };

    let data = match xfp {
        Some(xfp) => format!("{xfp}{join_xpubs}{threshold}of{total}{path}"),
        None => format!("{join_xpubs}{threshold}of{total}{path}"),
    };

    Ok(hex::encode(sha256(data.as_bytes()))[0..8].to_string())
}

pub fn strict_verify_wallet_config(
    seed: &[u8],
    wallet: &MultiSigWalletConfig,
    xfp: &descriptor::KeyOrigin,
) -> Result<(), ZcashError> {
    verify_wallet_config(wallet, xfp)?;
    let same_derivation = wallet.derivations.len() == 1;
    for (index, xpub_item) in wallet.xpub_items.iter().enumerate() {
        let (item_xfp, item_key) = xpub_item.clone().into_parts();
        if item_xfp == Some(xfp.clone()) {
            let true_index = match same_derivation {
                true => 0,
                false => index,
            };
            let true_derivation =
                wallet
                    .derivations
                    .get(true_index)
                    .ok_or(ZcashError::MultiSigWalletParseError(
                        "Invalid derivations".to_string(),
                    ))?;
            let true_xpub = bip32::ExtendedPrivateKey::<secp256k1::SecretKey>::derive_from_path(
                seed,
                &true_derivation.parse().map_err(|_| {
                    ZcashError::MultiSigWalletParseError("invalid true derivation".to_string())
                })?,
            )
            .map(|prv| prv.public_key())
            .map_err(|e| {
                ZcashError::MultiSigWalletParseError(format!(
                    "Unable to generate xpub, {}",
                    e.to_string()
                ))
            })?;
            match item_key {
                descriptor::Key::Public { .. } => {
                    return Err(ZcashError::MultiSigWalletParseError(format!(
                        "extended public key not match, xfp: {}",
                        xfp
                    )));
                }
                descriptor::Key::Xpub { key, .. } => {
                    if !(true_xpub == key) {
                        return Err(ZcashError::MultiSigWalletParseError(format!(
                            "extended public key not match, xfp: {xfp}",
                        )));
                    }
                }
            }
        }
    }
    Ok(())
}

#[cfg(test)]
mod tests {

    use alloc::string::ToString;

    use crate::{
        multi_sig::{
            wallet::{
                create_wallet, fixed_xfp, generate_config_data,
                parse_bsms_wallet_config, parse_wallet_config, strict_verify_wallet_config,
            },
            zip_48_path, MultiSigXPubInfo, Network,
        },
        ZcashError,
    };
    use hex;
    use ur_registry::bytes::Bytes;

    #[test]
    fn test_parse_wallet_config() {
        // 2-3 single path
        {
            let config = r#"/*
            # Coldcard Multisig setup file (created on 5271C071)
            #
            Name: CC-2-of-3
            Policy: 2 of 3
            Derivation: m/48'/133'/0'/133000'
            Format: P2SH

            748CC6AA: xpub6F6iZVTmc3KMgAUkV9JRNaouxYYwChRswPN1ut7nTfecn6VPRYLXFgXar1gvPUX27QH1zaVECqVEUoA2qMULZu5TjyKrjcWcLTQ6LkhrZAj
            C2202A77: xpub6EiTGcKqBQy2uTat1QQPhYQWt8LGmZStNqKDoikedkB72sUqgF9fXLUYEyPthqLSb6VP4akUAsy19MV5LL8SvqdzvcABYUpKw45jA1KZMhm
            5271C071: xpub6EWksRHwPbDmXWkjQeA6wbCmXZeDPXieMob9hhbtJjmrmk647bWkh7om5rk2eoeDKcKG6NmD8nT7UZAFxXQMjTnhENTwTEovQw3MDQ8jJ16
            */"#;

            let config = parse_wallet_config(config, &fixed_xfp([0x74, 0x8c, 0xc6, 0xaa])).unwrap();

            assert_eq!("Coldcard", config.creator);
            assert_eq!("CC-2-of-3", config.name);
            assert_eq!(2, config.threshold);
            assert_eq!(3, config.total);
            assert_eq!("P2SH", config.format);
            assert_eq!(1, config.derivations.len());
            assert_eq!("m/48'/133'/0'/133000'", config.derivations[0]);
            assert_eq!(3, config.xpub_items.len());
            let (xfp, xpub) = config.xpub_items[0].clone().into_parts();
            assert_eq!("xpub6F6iZVTmc3KMgAUkV9JRNaouxYYwChRswPN1ut7nTfecn6VPRYLXFgXar1gvPUX27QH1zaVECqVEUoA2qMULZu5TjyKrjcWcLTQ6LkhrZAj", xpub.to_string());
            assert_eq!("748cc6aa", hex::encode(xfp.unwrap().fingerprint()));
            assert_eq!(Network::MainNetwork, config.network);
            let (xfp, xpub) = config.xpub_items[1].clone().into_parts();
            assert_eq!("xpub6EiTGcKqBQy2uTat1QQPhYQWt8LGmZStNqKDoikedkB72sUqgF9fXLUYEyPthqLSb6VP4akUAsy19MV5LL8SvqdzvcABYUpKw45jA1KZMhm", xpub.to_string());
            assert_eq!("c2202a77", hex::encode(xfp.unwrap().fingerprint()));
            let (xfp, xpub) = config.xpub_items[2].clone().into_parts();
            assert_eq!("xpub6EWksRHwPbDmXWkjQeA6wbCmXZeDPXieMob9hhbtJjmrmk647bWkh7om5rk2eoeDKcKG6NmD8nT7UZAFxXQMjTnhENTwTEovQw3MDQ8jJ16", xpub.to_string());
            assert_eq!("5271c071", hex::encode(xfp.unwrap().fingerprint()));
            assert_eq!("f439fccd", config.verify_code);
        }

        // 2-3 multi path
        {
            let config = r#"/*
            # Coldcard Multisig setup file (exported from unchained-wallets)
            # https://github.com/unchained-capital/unchained-wallets
            # v1.0.0
            #
            Name: My Multisig Wallet
            Policy: 2 of 3
            Format: P2SH

            Derivation: m/48'/133'/0'/133000'/12/32/5
            748cc6aa: xpub6KMfgiWkVW33LfMbZoGjk6M3CvdZtrzkn38RP2SjbGGU9E85JTXDaX6Jn6bXVqnmq2EnRzWTZxeF3AZ1ZLcssM4DT9GY5RSuJBt1GRx3xm2
            Derivation: m/48'/133'/0'/133000'/5/6/7
            5271c071: xpub6LfFMiP3hcgrKeTrho9MgKj2zdKGPsd6ufJzrsQNaHSFZ7uj8e1vnSwibBVQ33VfXYJM5zn9G7E9VrMkFPVcdRtH3Brg9ndHLJs8v2QtwHa
            Derivation: m/48'/133'/0'/133000'/110/8/9
            c2202a77: xpub6LZnaHgbbxyZpChT4w9V5NC91qaZC9rrPoebgH3qGjZmcDKvPjLivfZSKLu5R1PjEpboNsznNwtqBifixCuKTfPxDZVNVN9mnjfTBpafqQf
            */"#;

            let config = parse_wallet_config(config, &fixed_xfp([0x74, 0x8c, 0xc6, 0xaa])).unwrap();
            assert_eq!("Coldcard", config.creator);
            assert_eq!("My Multisig Wallet", config.name);
            assert_eq!(2, config.threshold);
            assert_eq!(3, config.total);
            assert_eq!("P2SH", config.format);
            assert_eq!(3, config.derivations.len());
            assert_eq!("m/48'/133'/0'/133000'/12/32/5", config.derivations[0]);
            assert_eq!("m/48'/133'/0'/133000'/5/6/7", config.derivations[1]);
            assert_eq!("m/48'/133'/0'/133000'/110/8/9", config.derivations[2]);
            assert_eq!(3, config.xpub_items.len());
            let (xfp, xpub) = config.xpub_items[0].clone().into_parts();
            assert_eq!("xpub6KMfgiWkVW33LfMbZoGjk6M3CvdZtrzkn38RP2SjbGGU9E85JTXDaX6Jn6bXVqnmq2EnRzWTZxeF3AZ1ZLcssM4DT9GY5RSuJBt1GRx3xm2", xpub.to_string());
            assert_eq!("748cc6aa", hex::encode(xfp.unwrap().fingerprint()));
            assert_eq!(Network::MainNetwork, config.network);
            let (xfp, xpub) = config.xpub_items[1].clone().into_parts();
            assert_eq!("xpub6LfFMiP3hcgrKeTrho9MgKj2zdKGPsd6ufJzrsQNaHSFZ7uj8e1vnSwibBVQ33VfXYJM5zn9G7E9VrMkFPVcdRtH3Brg9ndHLJs8v2QtwHa", xpub.to_string());
            assert_eq!("5271c071", hex::encode(xfp.unwrap().fingerprint()));
            let (xfp, xpub) = config.xpub_items[2].clone().into_parts();
            assert_eq!("xpub6LZnaHgbbxyZpChT4w9V5NC91qaZC9rrPoebgH3qGjZmcDKvPjLivfZSKLu5R1PjEpboNsznNwtqBifixCuKTfPxDZVNVN9mnjfTBpafqQf", xpub.to_string());
            assert_eq!("c2202a77", hex::encode(xfp.unwrap().fingerprint()));
            assert_eq!("f3407658", config.verify_code);
        }
    }

    #[test]
    fn test_parse_multisig_wallet_config_with_err() {
        {
            // inconsistent network
            let config = r#"/*
            # Coldcard Multisig setup file (exported from unchained-wallets)
            # https://github.com/unchained-capital/unchained-wallets
            # v1.0.0
            #
            Name: My Multisig Wallet
            Policy: 2 of 3
            Format: P2SH

            Derivation: m/48'/0'/0'/1'/12/32/5
            748cc6aa: xpub6KMfgiWkVW33LfMbZoGjk6M3CvdZtrzkn38RP2SjbGGU9E85JTXDaX6Jn6bXVqnmq2EnRzWTZxeF3AZ1ZLcssM4DT9GY5RSuJBt1GRx3xm2
            Derivation: m/48'/0'/0'/1'/5/6/7
            5271c071: xpub6LfFMiP3hcgrKeTrho9MgKj2zdKGPsd6ufJzrsQNaHSFZ7uj8e1vnSwibBVQ33VfXYJM5zn9G7E9VrMkFPVcdRtH3Brg9ndHLJs8v2QtwHa
            Derivation: m/48'/0'/0'/1'/110/8/9
            c2202a77: tpubDLwR6XWHKEw25ztJX88aHHXWLxgDBYMuwSEu2q6yceKfMVzdTxBp57EGZNyrwWLy2j8hzTWev3issuALM4kZM6rFHAEgAtp1NhFrwr9MzF5
            */"#;

            let config = parse_wallet_config(config, &fixed_xfp([0x74, 0x8c, 0xc6, 0xaa]));
            // NB: This _should_ fail with “xpub networks inconsistent”, but since testnet isn’t
            //     supported yet, we get this error before we even check for consistency. It should
            //     change once testnet is supported.
            assert_eq!(
                Err(ZcashError::MultiSigWalletParseError(
                    "we don't support testnet for multisig yet".to_string()
                )),
                config.map(|_| ())
            );
        }

        {
            //total is 4 but only provide 3 keys
            let config = r#"/*
            # Coldcard Multisig setup file (exported from unchained-wallets)
            # https://github.com/unchained-capital/unchained-wallets
            # v1.0.0
            #
            Name: My Multisig Wallet
            Policy: 2 of 4
            Format: P2SH

            Derivation: m/48'/0'/0'/1'/12/32/5
            748cc6aa: xpub6KMfgiWkVW33LfMbZoGjk6M3CvdZtrzkn38RP2SjbGGU9E85JTXDaX6Jn6bXVqnmq2EnRzWTZxeF3AZ1ZLcssM4DT9GY5RSuJBt1GRx3xm2
            Derivation: m/48'/0'/0'/1'/5/6/7
            5271c071: xpub6LfFMiP3hcgrKeTrho9MgKj2zdKGPsd6ufJzrsQNaHSFZ7uj8e1vnSwibBVQ33VfXYJM5zn9G7E9VrMkFPVcdRtH3Brg9ndHLJs8v2QtwHa
            Derivation: m/48'/0'/0'/1'/110/8/9
            c2202a77: xpub6LZnaHgbbxyZpChT4w9V5NC91qaZC9rrPoebgH3qGjZmcDKvPjLivfZSKLu5R1PjEpboNsznNwtqBifixCuKTfPxDZVNVN9mnjfTBpafqQf
            */"#;

            let config = parse_wallet_config(config, &fixed_xfp([0x74, 0x8c, 0xc6, 0xaa]));
            assert_eq!(
                Err(ZcashError::MultiSigWalletParseError(
                    "multisig wallet requires 4 cosigners, but found 3".to_string()
                )),
                config.map(|_| ())
            );
        }

        {
            let config = r#"/*
                # Coldcard Multisig setup file (exported from unchained-wallets)
                # https://github.com/unchained-capital/unchained-wallets
                # v1.0.0
                #
                Name: My Multisig Wallet
                Policy: 2 of 3
                Format: P2SH

                52744703: Zpub75TRDhcAARKcWjwnLvoUP5AkszA13W5EnHTxL3vfnKtEXZYQsHKjPFxWLGT98ztfRY1ttG5RKhNChZuoTkFFfXEqyKSKDi99LtJcSKzUfDx
                50659928: xpub6E9nHcM3Q2cBHFhYHfz7cghFutBpKWsAGXukyyd13TkHczjDaazdCZwbKXJCWkCeH4KcNtuCAmVycVNugJh1UcHMJeVi25csjzLxChEpCW4
                50659928: xpub6E9nHcM3Q2cBHFhYHfz7cghFutBpKWsAGXukyyd13TkHczjDaazdCZwbKXJCWkCeH4KcNtuCAmVycVNugJh1UcHMJeVi25csjzLxChEpCW4
                */"#;

            let config = parse_wallet_config(config, &fixed_xfp([0x74, 0x8c, 0xc6, 0xaa]));
            assert_eq!(
                Err(ZcashError::MultiSigWalletParseError(
                    "found duplicated xpub".to_string()
                )),
                config.map(|_| ())
            );
        }
    }

    #[test]
    fn test_strict_verify_wallet_config() {
        let config = r#"/*
            # Coldcard Multisig setup file (exported from unchained-wallets)
            # https://github.com/unchained-capital/unchained-wallets
            # v1.0.0
            #
            Name: My Multisig Wallet
            Policy: 2 of 3
            Format: P2SH

            Derivation: m/48'/133'/0'/133000'
            73c5da0a: xpub6EghKBv28q43t91tEvAr7xGvGD4m8n1GT99miHXL73dyPLFpPNmWDUNuFxxN13VCmDBisLXQ6bBMAw7Vh2XhCRAB6MvcMdT97j2XQvu2Ynr
            Derivation: m/48'/133'/0'/133000'/5/6/7
            5271c071: xpub6LfFMiP3hcgrKeTrho9MgKj2zdKGPsd6ufJzrsQNaHSFZ7uj8e1vnSwibBVQ33VfXYJM5zn9G7E9VrMkFPVcdRtH3Brg9ndHLJs8v2QtwHa
            Derivation: m/48'/133'/0'/133000'/110/8/9
            c2202a77: xpub6LZnaHgbbxyZpChT4w9V5NC91qaZC9rrPoebgH3qGjZmcDKvPjLivfZSKLu5R1PjEpboNsznNwtqBifixCuKTfPxDZVNVN9mnjfTBpafqQf
            */"#;

        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let config = parse_wallet_config(config, &fixed_xfp([0x73, 0xc5, 0xda, 0x0a])).unwrap();
        let result =
            strict_verify_wallet_config(&seed, &config, &fixed_xfp([0x73, 0xc5, 0xda, 0x0a]));
        assert_eq!(Ok(()), result);
    }

    #[test]
    fn test_strict_verify_wallet_config_wrong_xpub() {
        let config = r#"/*
            # Coldcard Multisig setup file (exported from unchained-wallets)
            # https://github.com/unchained-capital/unchained-wallets
            # v1.0.0
            #
            Name: My Multisig Wallet
            Policy: 2 of 3
            Format: P2SH

            Derivation: m/48'/133'/0'/133000'/5/6/7
            5271c071: xpub6LfFMiP3hcgrKeTrho9MgKj2zdKGPsd6ufJzrsQNaHSFZ7uj8e1vnSwibBVQ33VfXYJM5zn9G7E9VrMkFPVcdRtH3Brg9ndHLJs8v2QtwHa
            Derivation: m/48'/133'/0'/133000'/110/8/9
            c2202a77: xpub6LZnaHgbbxyZpChT4w9V5NC91qaZC9rrPoebgH3qGjZmcDKvPjLivfZSKLu5R1PjEpboNsznNwtqBifixCuKTfPxDZVNVN9mnjfTBpafqQf
            Derivation: m/48'/133'/0'/133000'
            73c5da0a: xpub661MyMwAqRbcFkPHucMnrGNzDwb6teAX1RbKQmqtEF8kK3Z7LZ59qafCjB9eCRLiTVG3uxBxgKvRgbubRhqSKXnGGb1aoaqLrpMBDrVxga8
            */"#;

        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let config = parse_wallet_config(config, &fixed_xfp([0x73, 0xc5, 0xda, 0x0a])).unwrap();
        let config =
            strict_verify_wallet_config(&seed, &config, &fixed_xfp([0x73, 0xc5, 0xda, 0x0a]));
        assert_eq!(
            Err(ZcashError::MultiSigWalletParseError(
                "extended public key not match, xfp: [73c5da0a]".to_string()
            )),
            config.map(|_| ()),
        );
    }
    // comment because we disable testnet currently;
    //     #[test]
    //     fn test_generate_wallet_config() {
    //         let config_str = "# Keystone Multisig setup file (created on 73C5DA0A)
    // #
    // Name: testnet1
    // Policy: 2 of 2
    // Derivation: m/45'
    // Format: P2SH

    // C45358FA: tpubD9hphZzCi9u5Wcbtq3jQYTzbPv6igoaRWDuhxLUDv5VTffE3gEVovYaqwfVMCa6q8VMdwAcPpFgAdajgmLML6XgYrKBquyYEDQg1HnKm3wQ
    // 73C5DA0A: tpubD97UxEEVXiRtzRBmHvR38R7QXNz6Dx3A7gKtoe9UgxepdJXExmJCd5Nxsv8YYLgHd3MEBKPzRwgVaJ62kvBSvMtntbkPnv6Pf8Zkny5rC89";
    //         let config = parse_wallet_config(config_str, "73C5DA0A").unwrap();
    //         let data = generate_config_data(&config, "73C5DA0A").unwrap();
    //         assert_eq!(data, config_str);
    //     }
    //     #[test]
    //     fn test_generate_wallet_config_ur() {
    //         let config_str = "# Keystone Multisig setup file (created on 73C5DA0A)
    // #
    // Name: testnet1
    // Policy: 2 of 2
    // Derivation: m/45'
    // Format: P2SH

    // C45358FA: tpubD9hphZzCi9u5Wcbtq3jQYTzbPv6igoaRWDuhxLUDv5VTffE3gEVovYaqwfVMCa6q8VMdwAcPpFgAdajgmLML6XgYrKBquyYEDQg1HnKm3wQ
    // 73C5DA0A: tpubD97UxEEVXiRtzRBmHvR38R7QXNz6Dx3A7gKtoe9UgxepdJXExmJCd5Nxsv8YYLgHd3MEBKPzRwgVaJ62kvBSvMtntbkPnv6Pf8Zkny5rC89";
    //         let config = parse_wallet_config(config_str, "73C5DA0A").unwrap();
    //         let data = export_wallet_by_ur(&config, "73C5DA0A").unwrap();
    //         assert_eq!(
    //             "59016823204B657973746F6E65204D756C74697369672073657475702066696C65202863726561746564206F6E203733433544413041290A230A4E616D653A20746573746E6574310A506F6C6963793A2032206F6620320A44657269766174696F6E3A206D2F3435270A466F726D61743A20503253480A0A43343533353846413A207470756244396870685A7A43693975355763627471336A5159547A6250763669676F615257447568784C554476355654666645336745566F765961717766564D4361367138564D64774163507046674164616A676D4C4D4C36586759724B42717579594544516731486E4B6D3377510A37334335444130413A20747075624439375578454556586952747A52426D4876523338523751584E7A364478334137674B746F65395567786570644A5845786D4A4364354E7873763859594C674864334D45424B507A52776756614A36326B764253764D746E74626B506E76365066385A6B6E793572433839",
    //             hex::encode::<Vec<u8>>(data.clone().try_into().unwrap()).to_uppercase()
    //         );

    //         let bytes = Bytes::try_from(TryInto::<Vec<u8>>::try_into(data.clone()).unwrap()).unwrap();
    //         assert_eq!(data.get_bytes(), bytes.get_bytes());
    //     }

    #[test]
    fn test_create_wallet_config() {
        // single derivation
        {
            let config = "# test Multisig setup file (created on 73c5da0a)
#
Name: test1
Policy: 2 of 2
Derivation: m/48'/133'/0'/133000'
Format: P2SH

c45358fa: tpubD9hphZzCi9u5Wcbtq3jQYTzbPv6igoaRWDuhxLUDv5VTffE3gEVovYaqwfVMCa6q8VMdwAcPpFgAdajgmLML6XgYrKBquyYEDQg1HnKm3wQ
73c5da0a: tpubD97UxEEVXiRtzRBmHvR38R7QXNz6Dx3A7gKtoe9UgxepdJXExmJCd5Nxsv8YYLgHd3MEBKPzRwgVaJ62kvBSvMtntbkPnv6Pf8Zkny5rC89";
            let xpub_info = vec![
                MultiSigXPubInfo {
                    path: zip_48_path(&Network::MainNetwork, 0),
                    key_expr: format!("[{}]{}", "c45358fa", "tpubD9hphZzCi9u5Wcbtq3jQYTzbPv6igoaRWDuhxLUDv5VTffE3gEVovYaqwfVMCa6q8VMdwAcPpFgAdajgmLML6XgYrKBquyYEDQg1HnKm3wQ").parse().unwrap(),
                },
                MultiSigXPubInfo {
                    path: zip_48_path(&Network::MainNetwork, 0),
                    key_expr: format!("[{}]{}", "73c5da0a", "tpubD97UxEEVXiRtzRBmHvR38R7QXNz6Dx3A7gKtoe9UgxepdJXExmJCd5Nxsv8YYLgHd3MEBKPzRwgVaJ62kvBSvMtntbkPnv6Pf8Zkny5rC89").parse().unwrap(),
                },
            ];

            let wallet = create_wallet(
                "test",
                "test1",
                2,
                2,
                "P2SH",
                &xpub_info,
                Network::TestNetwork,
                &fixed_xfp([0x73, 0xc5, 0xda, 0x0a]),
            )
            .unwrap();

            let data = generate_config_data(&wallet, &fixed_xfp([0x73, 0xc5, 0xda, 0x0a])).unwrap();
            assert_eq!(config, data);
        }

        // multi derivation
        {
            let config = "# test Multisig setup file (created on 748cc6aa)
#
Name: test1
Policy: 2 of 3
Format: P2SH

Derivation: m/48'/133'/0'/133000'/12/32/5
748cc6aa: xpub6KMfgiWkVW33LfMbZoGjk6M3CvdZtrzkn38RP2SjbGGU9E85JTXDaX6Jn6bXVqnmq2EnRzWTZxeF3AZ1ZLcssM4DT9GY5RSuJBt1GRx3xm2
Derivation: m/48'/133'/0'/133000'/5/6/7
5271c071: xpub6LfFMiP3hcgrKeTrho9MgKj2zdKGPsd6ufJzrsQNaHSFZ7uj8e1vnSwibBVQ33VfXYJM5zn9G7E9VrMkFPVcdRtH3Brg9ndHLJs8v2QtwHa
Derivation: m/48'/133'/0'/133000'/110/8/9
c2202a77: xpub6LZnaHgbbxyZpChT4w9V5NC91qaZC9rrPoebgH3qGjZmcDKvPjLivfZSKLu5R1PjEpboNsznNwtqBifixCuKTfPxDZVNVN9mnjfTBpafqQf";

            let xpub_info = vec![
                MultiSigXPubInfo {
                    path: format!("{}/12/32/5", zip_48_path(&Network::MainNetwork, 0)),
                    key_expr: format!("[{}]{}", "748cc6aa", "xpub6KMfgiWkVW33LfMbZoGjk6M3CvdZtrzkn38RP2SjbGGU9E85JTXDaX6Jn6bXVqnmq2EnRzWTZxeF3AZ1ZLcssM4DT9GY5RSuJBt1GRx3xm2").parse().unwrap(),
                },
                MultiSigXPubInfo {
                    path: format!("{}/5/6/7", zip_48_path(&Network::MainNetwork, 0)),
                    key_expr: format!("[{}]{}", "5271c071", "xpub6LfFMiP3hcgrKeTrho9MgKj2zdKGPsd6ufJzrsQNaHSFZ7uj8e1vnSwibBVQ33VfXYJM5zn9G7E9VrMkFPVcdRtH3Brg9ndHLJs8v2QtwHa").parse().unwrap(),
                },
                MultiSigXPubInfo {
                    path: format!("{}/110/8/9", zip_48_path(&Network::MainNetwork, 0)),
                    key_expr: format!("[{}]{}", "c2202a77", "xpub6LZnaHgbbxyZpChT4w9V5NC91qaZC9rrPoebgH3qGjZmcDKvPjLivfZSKLu5R1PjEpboNsznNwtqBifixCuKTfPxDZVNVN9mnjfTBpafqQf").parse().unwrap(),
                },
            ];

            let wallet = create_wallet(
                "test",
                "test1",
                2,
                3,
                "P2SH",
                &xpub_info,
                Network::MainNetwork,
                &fixed_xfp([0x74, 0x8c, 0xc6, 0xaa]),
            )
            .unwrap();

            let data = generate_config_data(&wallet, &fixed_xfp([0x74, 0x8c, 0xc6, 0xaa])).unwrap();

            assert_eq!(config, data);
        }
    }

    #[test]
    fn test_parse_bsms_wallet_config() {
        let config_str = Bytes::new("BSMS 1.0
00
[73c5da0a/48'/133'/0'/133000']xpub6DkFAXWQ2dHxq2vatrt9qyA3bXYU4ToWQwCHbf5XB2mSTexcHZCeKS1VZYcPoBd5X8yVcbXFHJR9R8UCVpt82VX1VhR28mCyxUFL4r6KFrf
BIP39 4".as_bytes().to_vec());
        let wallet = parse_bsms_wallet_config(config_str).unwrap();

        assert_eq!(wallet.bsms_version, "BSMS 1.0");
        assert_eq!(wallet.xfp, "73c5da0a");
        assert_eq!(
            wallet.derivation_path,
            zip_48_path(&Network::MainNetwork, 0)
        );
        assert_eq!(wallet.extended_pubkey, "xpub6DkFAXWQ2dHxq2vatrt9qyA3bXYU4ToWQwCHbf5XB2mSTexcHZCeKS1VZYcPoBd5X8yVcbXFHJR9R8UCVpt82VX1VhR28mCyxUFL4r6KFrf");
    }
}
