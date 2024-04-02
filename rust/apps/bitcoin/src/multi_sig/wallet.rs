use alloc::fmt::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::str::FromStr;
use third_party::bitcoin::bip32::Xpub;
use third_party::cryptoxide::hashing::sha256;
use third_party::hex;
use third_party::itertools::Itertools;
use third_party::ur_registry::bytes::Bytes;

use crate::addresses::xyzpub;
use crate::addresses::xyzpub::Version;
use crate::BitcoinError;

use crate::multi_sig::{
    convert_xpub, MultiSigXPubInfo, Network, MULTI_P2SH_PATH, MULTI_P2WSH_P2SH_PATH,
    MULTI_P2WSH_P2SH_PATH_TEST, MULTI_P2WSH_PATH, MULTI_P2WSH_PATH_TEST,
};

const CHANGE_XPUB_PREFIX_BY_PATH: bool = true;

#[derive(Debug, Clone)]
pub struct MultiSigXPubItem {
    pub network: Network,
    pub xfp: String,
    pub xpub: String,
}

#[derive(Debug)]
pub struct MultiSigWalletConfig {
    pub creator: String,
    pub name: String,
    pub threshold: u32,
    pub total: u32,
    pub derivations: Vec<String>,
    pub format: String,
    pub xpub_items: Vec<MultiSigXPubItem>,
    pub verify_code: String,
    pub config_text: String,
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
            config_text: String::new(),
        }
    }
}

impl MultiSigWalletConfig {
    pub fn get_network(&self) -> &Network {
        &self.xpub_items[0].network
    }

    pub fn get_network_u32(&self) -> u32 {
        match &self.xpub_items[0].network {
            Network::MainNet => 0,
            Network::TestNet => 1,
        }
    }

    pub fn get_wallet_path(&self) -> Result<&str, BitcoinError> {
        let path = match (self.format.as_str(), self.get_network()) {
            ("P2SH", _) => MULTI_P2SH_PATH,
            ("P2WSH-P2SH", Network::MainNet) => MULTI_P2WSH_P2SH_PATH,
            ("P2WSH-P2SH", Network::TestNet) => MULTI_P2WSH_P2SH_PATH_TEST,
            ("P2WSH", Network::MainNet) => MULTI_P2WSH_PATH,
            ("P2WSH", Network::TestNet) => MULTI_P2WSH_PATH_TEST,
            _ => {
                return Err(BitcoinError::MultiSigWalletParseError(format!(
                    "not support format {}",
                    self.format.as_str()
                )));
            }
        };
        Ok(path)
    }
}

pub fn create_wallet(
    creator: &str,
    name: &str,
    threshold: u32,
    total: u32,
    format: &str,
    xpub_infos: &Vec<MultiSigXPubInfo>,
    network: Network,
    xfp: &str,
) -> Result<MultiSigWalletConfig, BitcoinError> {
    if !is_valid_multi_sig_policy(threshold, total) {
        return Err(BitcoinError::MultiSigWalletCrateError(
            "not a valid policy".to_string(),
        ));
    }

    let mut wallet = MultiSigWalletConfig::default();

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
        .extend(xpub_infos.iter().map(|x| MultiSigXPubItem {
            network: network.clone(),
            xfp: x.xfp.to_string(),
            xpub: x.xpub.to_string(),
        }));

    wallet.creator = creator.to_string();
    wallet.total = total;
    wallet.threshold = threshold;
    wallet.format = format.to_string();
    wallet.name = name.to_string();

    verify_wallet_config(&wallet, xfp, &network)?;
    calculate_wallet_verify_code(&mut wallet)?;
    wallet.config_text = generate_config_data(&wallet, xfp, network)?;
    Ok(wallet)
}

fn _parse_plain_wallet_config(content: &str) -> Result<MultiSigWalletConfig, BitcoinError> {
    let mut wallet = MultiSigWalletConfig::default();

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
                _ => process_xpub_and_xfp(&mut wallet, label, value)?,
            }
        }
    }
    wallet.config_text = content.to_string();
    Ok(wallet)
}

pub fn parse_wallet_config(
    content: &str,
    xfp: &str,
    network: Network,
) -> Result<MultiSigWalletConfig, BitcoinError> {
    let mut wallet = _parse_plain_wallet_config(content)?;
    verify_wallet_config(&wallet, xfp, &network)?;
    calculate_wallet_verify_code(&mut wallet)?;
    Ok(wallet)
}

pub fn export_wallet_by_ur(
    config: &MultiSigWalletConfig,
    xfp: &str,
    network: Network,
) -> Result<Bytes, BitcoinError> {
    let config_data = generate_config_data(config, xfp, network)?;
    Ok(Bytes::new(config_data.into_bytes()))
}

pub fn generate_config_data(
    config: &MultiSigWalletConfig,
    xfp: &str,
    network: Network,
) -> Result<String, BitcoinError> {
    let mut config_data = String::new();

    let policy = format!("{} of {}", config.threshold, config.total);
    config_data.push_str(&format!(
        "# {} Multisig setup file (created on {})\n",
        config.creator, xfp
    ));
    config_data.push_str("#\n");
    config_data.push_str(&format!("Name: {}\n", config.name));
    config_data.push_str(&format!("Policy: {}\n", policy));

    if config.derivations.len() == 1 {
        config_data.push_str(&format!("Derivation: {}\n", config.derivations[0]));
        config_data.push_str(&format!("Format: {}\n", config.format));
        config_data.push_str("\n");
        let xpub_items = config
            .xpub_items
            .iter()
            .map(|item| {
                let xpub = if CHANGE_XPUB_PREFIX_BY_PATH {
                    convert_xpub(&config.derivations[0], &item.xpub, &network)?
                } else {
                    item.xpub.to_string()
                };
                Ok(format!("{}: {}", item.xfp, xpub))
            })
            .collect::<Result<Vec<String>, BitcoinError>>()?;

        config_data.push_str(&xpub_items.join("\n"));
    } else {
        config_data.push_str(&format!("Format: {}\n", config.format));

        for (index, item) in config.xpub_items.iter().enumerate() {
            config_data.push_str(&format!("\nDerivation: {}\n", config.derivations[index]));
            let xpub = if CHANGE_XPUB_PREFIX_BY_PATH {
                convert_xpub(&config.derivations[index], &item.xpub, &network)?
            } else {
                item.xpub.to_string()
            };
            // let xpub = convert_xpub(&config.derivations[index], &item.xpub, &network)?;
            config_data.push_str(&format!("{}: {}", item.xfp, xpub));
        }
    }

    Ok(config_data)
}

pub fn import_wallet_by_ur(
    bytes: &Bytes,
    xfp: &str,
    network: Network,
) -> Result<MultiSigWalletConfig, BitcoinError> {
    let data = String::from_utf8(bytes.get_bytes())
        .map_err(|e| BitcoinError::MultiSigWalletImportXpubError(e.to_string()))?;

    parse_wallet_config(&data, xfp, network)
}

pub fn is_valid_wallet_config(bytes: &Bytes) -> bool {
    if let Ok(d) = String::from_utf8(bytes.get_bytes())
        .map_err(|e| BitcoinError::MultiSigWalletImportXpubError(e.to_string()))
    {
        if let Ok(_) = _parse_plain_wallet_config(&d) {
            return true;
        }
    }
    false
}

fn parse_and_set_policy(
    wallet: &mut MultiSigWalletConfig,
    value: &str,
) -> Result<(), BitcoinError> {
    let policy = value.split(" of ").collect::<Vec<_>>();
    if policy.len() == 2 {
        let threshold: u32 = policy[0].parse().map_err(|_e| {
            BitcoinError::MultiSigWalletParseError("parse threshold error".to_string())
        })?;
        let total: u32 = policy[1].parse().map_err(|_e| {
            BitcoinError::MultiSigWalletParseError("parse total error".to_string())
        })?;

        if is_valid_multi_sig_policy(total, threshold) {
            wallet.threshold = threshold.clone();
            wallet.total = total.clone();
        } else {
            return Err(BitcoinError::MultiSigWalletParseError(
                "this is not a valid policy".to_string(),
            ));
        }
    }

    Ok(())
}

fn process_xpub_and_xfp(
    wallet: &mut MultiSigWalletConfig,
    label: &str,
    value: &str,
) -> Result<(), BitcoinError> {
    if is_valid_xfp(label) {
        if is_valid_xpub(value) {
            wallet.xpub_items.push(MultiSigXPubItem {
                network: detect_network(value),
                xfp: label.to_string(),
                xpub: value.to_string(),
            });
            Ok(())
        } else {
            Err(BitcoinError::MultiSigWalletParseError(
                "this is not a valid xpub".to_string(),
            ))
        }
    } else {
        Err(BitcoinError::MultiSigWalletParseError(
            "this is not a valid xfp".to_string(),
        ))
    }
}

fn is_valid_multi_sig_policy(total: u32, threshold: u32) -> bool {
    total <= 15 && total >= 2 && threshold <= total || threshold >= 1
}

fn is_valid_xfp(xfp: &str) -> bool {
    if xfp.len() != 8 {
        return false;
    }
    for c in xfp.chars() {
        if !c.is_digit(16) {
            return false;
        }
    }
    true
}

fn is_valid_xpub(xpub: &str) -> bool {
    Xpub::from_str(xpub).is_ok()
}

fn detect_network(xpub: &str) -> Network {
    if xpub.starts_with("tpub") || xpub.starts_with("Upub") || xpub.starts_with("Vpub") {
        Network::TestNet
    } else {
        Network::MainNet
    }
}

fn verify_wallet_config(
    wallet: &MultiSigWalletConfig,
    xfp: &str,
    network: &Network,
) -> Result<(), BitcoinError> {
    if wallet.xpub_items.is_empty() {
        return Err(BitcoinError::MultiSigWalletParseError(
            "have no xpub in config file".to_string(),
        ));
    }

    if !wallet
        .xpub_items
        .iter()
        .any(|item| item.xfp.eq_ignore_ascii_case(xfp))
    {
        return Err(BitcoinError::MultiSigWalletParseError(
            "have no matched xfp in config file".to_string(),
        ));
    }

    if wallet.derivations.len() != 1 && wallet.derivations.len() != wallet.xpub_items.len() {
        return Err(BitcoinError::MultiSigWalletParseError(
            "num of derivations is not right".to_string(),
        ));
    }

    for item in &wallet.xpub_items {
        if item.network != *network {
            return Err(BitcoinError::MultiSigWalletParseError(format!(
                "the network does not match the current mode, network is {}, current mode is {}",
                item.network, network
            )));
        }
    }
    Ok(())
}

fn calculate_wallet_verify_code(wallet: &mut MultiSigWalletConfig) -> Result<(), BitcoinError> {
    let join_xpubs = wallet
        .xpub_items
        .iter()
        .map(|x| xyzpub::convert_version(&x.xpub, &Version::Xpub))
        .collect::<Result<Vec<_>, _>>()?
        .iter()
        .sorted()
        .join(" ");

    let data = format!(
        "{}{}of{}{}",
        join_xpubs,
        wallet.threshold,
        wallet.xpub_items.len(),
        wallet.get_wallet_path()?,
    );
    wallet.verify_code = hex::encode(sha256(data.as_bytes()))[0..8].to_string();
    Ok(())
}

#[cfg(test)]
mod tests {
    use alloc::string::ToString;

    use crate::multi_sig::wallet::{
        create_wallet, export_wallet_by_ur, generate_config_data, parse_wallet_config,
    };
    use crate::multi_sig::{MultiSigXPubInfo, Network};
    use alloc::vec::Vec;
    use third_party::hex;
    use third_party::ur_registry::bytes::Bytes;

    #[test]
    fn test_parse_wallet_config() {
        // 2-3 single path
        {
            let config = r#"/*
            # Coldcard Multisig setup file (created on 5271C071)
            #
            Name: CC-2-of-3
            Policy: 2 of 3
            Derivation: m/48'/0'/0'/2'
            Format: P2WSH

            748CC6AA: xpub6F6iZVTmc3KMgAUkV9JRNaouxYYwChRswPN1ut7nTfecn6VPRYLXFgXar1gvPUX27QH1zaVECqVEUoA2qMULZu5TjyKrjcWcLTQ6LkhrZAj
            C2202A77: xpub6EiTGcKqBQy2uTat1QQPhYQWt8LGmZStNqKDoikedkB72sUqgF9fXLUYEyPthqLSb6VP4akUAsy19MV5LL8SvqdzvcABYUpKw45jA1KZMhm
            5271C071: xpub6EWksRHwPbDmXWkjQeA6wbCmXZeDPXieMob9hhbtJjmrmk647bWkh7om5rk2eoeDKcKG6NmD8nT7UZAFxXQMjTnhENTwTEovQw3MDQ8jJ16
            */"#;

            let config = parse_wallet_config(config, "748CC6AA", Network::MainNet).unwrap();

            assert_eq!("Coldcard", config.creator);
            assert_eq!("CC-2-of-3", config.name);
            assert_eq!(2, config.threshold);
            assert_eq!(3, config.total);
            assert_eq!("P2WSH", config.format);
            assert_eq!(1, config.derivations.len());
            assert_eq!("m/48'/0'/0'/2'", config.derivations[0]);
            assert_eq!(3, config.xpub_items.len());
            assert_eq!("xpub6F6iZVTmc3KMgAUkV9JRNaouxYYwChRswPN1ut7nTfecn6VPRYLXFgXar1gvPUX27QH1zaVECqVEUoA2qMULZu5TjyKrjcWcLTQ6LkhrZAj", config.xpub_items[0].xpub);
            assert_eq!("748CC6AA", config.xpub_items[0].xfp);
            assert_eq!(Network::MainNet, config.xpub_items[0].network);
            assert_eq!("xpub6EiTGcKqBQy2uTat1QQPhYQWt8LGmZStNqKDoikedkB72sUqgF9fXLUYEyPthqLSb6VP4akUAsy19MV5LL8SvqdzvcABYUpKw45jA1KZMhm", config.xpub_items[1].xpub);
            assert_eq!("C2202A77", config.xpub_items[1].xfp);
            assert_eq!(Network::MainNet, config.xpub_items[1].network);
            assert_eq!("xpub6EWksRHwPbDmXWkjQeA6wbCmXZeDPXieMob9hhbtJjmrmk647bWkh7om5rk2eoeDKcKG6NmD8nT7UZAFxXQMjTnhENTwTEovQw3MDQ8jJ16", config.xpub_items[2].xpub);
            assert_eq!("5271C071", config.xpub_items[2].xfp);
            assert_eq!(Network::MainNet, config.xpub_items[2].network);
            assert_eq!("d4637859", config.verify_code);
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
            Format: P2WSH-P2SH

            Derivation: m/48'/0'/0'/1'/12/32/5
            748cc6aa: xpub6KMfgiWkVW33LfMbZoGjk6M3CvdZtrzkn38RP2SjbGGU9E85JTXDaX6Jn6bXVqnmq2EnRzWTZxeF3AZ1ZLcssM4DT9GY5RSuJBt1GRx3xm2
            Derivation: m/48'/0'/0'/1'/5/6/7
            5271c071: xpub6LfFMiP3hcgrKeTrho9MgKj2zdKGPsd6ufJzrsQNaHSFZ7uj8e1vnSwibBVQ33VfXYJM5zn9G7E9VrMkFPVcdRtH3Brg9ndHLJs8v2QtwHa
            Derivation: m/48'/0'/0'/1'/110/8/9
            c2202a77: xpub6LZnaHgbbxyZpChT4w9V5NC91qaZC9rrPoebgH3qGjZmcDKvPjLivfZSKLu5R1PjEpboNsznNwtqBifixCuKTfPxDZVNVN9mnjfTBpafqQf
            */"#;

            let config = parse_wallet_config(config, "748cc6aa", Network::MainNet).unwrap();
            assert_eq!("Coldcard", config.creator);
            assert_eq!("My Multisig Wallet", config.name);
            assert_eq!(2, config.threshold);
            assert_eq!(3, config.total);
            assert_eq!("P2WSH-P2SH", config.format);
            assert_eq!(3, config.derivations.len());
            assert_eq!("m/48'/0'/0'/1'/12/32/5", config.derivations[0]);
            assert_eq!("m/48'/0'/0'/1'/5/6/7", config.derivations[1]);
            assert_eq!("m/48'/0'/0'/1'/110/8/9", config.derivations[2]);
            assert_eq!(3, config.xpub_items.len());
            assert_eq!("xpub6KMfgiWkVW33LfMbZoGjk6M3CvdZtrzkn38RP2SjbGGU9E85JTXDaX6Jn6bXVqnmq2EnRzWTZxeF3AZ1ZLcssM4DT9GY5RSuJBt1GRx3xm2", config.xpub_items[0].xpub);
            assert_eq!("748cc6aa", config.xpub_items[0].xfp);
            assert_eq!(Network::MainNet, config.xpub_items[0].network);
            assert_eq!("xpub6LfFMiP3hcgrKeTrho9MgKj2zdKGPsd6ufJzrsQNaHSFZ7uj8e1vnSwibBVQ33VfXYJM5zn9G7E9VrMkFPVcdRtH3Brg9ndHLJs8v2QtwHa", config.xpub_items[1].xpub);
            assert_eq!("5271c071", config.xpub_items[1].xfp);
            assert_eq!(Network::MainNet, config.xpub_items[1].network);
            assert_eq!("xpub6LZnaHgbbxyZpChT4w9V5NC91qaZC9rrPoebgH3qGjZmcDKvPjLivfZSKLu5R1PjEpboNsznNwtqBifixCuKTfPxDZVNVN9mnjfTBpafqQf", config.xpub_items[2].xpub);
            assert_eq!("c2202a77", config.xpub_items[2].xfp);
            assert_eq!(Network::MainNet, config.xpub_items[2].network);
            assert_eq!("7cb85a16", config.verify_code);
        }
    }

    #[test]
    fn test_generate_wallet_config() {
        let config_str = "# Keystone Multisig setup file (created on 73C5DA0A)
#
Name: testnet1
Policy: 2 of 2
Derivation: m/45'
Format: P2SH

C45358FA: tpubD9hphZzCi9u5Wcbtq3jQYTzbPv6igoaRWDuhxLUDv5VTffE3gEVovYaqwfVMCa6q8VMdwAcPpFgAdajgmLML6XgYrKBquyYEDQg1HnKm3wQ
73C5DA0A: tpubD97UxEEVXiRtzRBmHvR38R7QXNz6Dx3A7gKtoe9UgxepdJXExmJCd5Nxsv8YYLgHd3MEBKPzRwgVaJ62kvBSvMtntbkPnv6Pf8Zkny5rC89";
        let config = parse_wallet_config(config_str, "73C5DA0A", Network::TestNet).unwrap();
        let data = generate_config_data(&config, "73C5DA0A", Network::TestNet).unwrap();
        assert_eq!(data, config_str);
    }

    #[test]
    fn test_generate_wallet_config_ur() {
        let config_str = "# Keystone Multisig setup file (created on 73C5DA0A)
#
Name: testnet1
Policy: 2 of 2
Derivation: m/45'
Format: P2SH

C45358FA: tpubD9hphZzCi9u5Wcbtq3jQYTzbPv6igoaRWDuhxLUDv5VTffE3gEVovYaqwfVMCa6q8VMdwAcPpFgAdajgmLML6XgYrKBquyYEDQg1HnKm3wQ
73C5DA0A: tpubD97UxEEVXiRtzRBmHvR38R7QXNz6Dx3A7gKtoe9UgxepdJXExmJCd5Nxsv8YYLgHd3MEBKPzRwgVaJ62kvBSvMtntbkPnv6Pf8Zkny5rC89";
        let config = parse_wallet_config(config_str, "73C5DA0A", Network::TestNet).unwrap();
        let data = export_wallet_by_ur(&config, "73C5DA0A", Network::TestNet).unwrap();
        assert_eq!(
            "59016823204B657973746F6E65204D756C74697369672073657475702066696C65202863726561746564206F6E203733433544413041290A230A4E616D653A20746573746E6574310A506F6C6963793A2032206F6620320A44657269766174696F6E3A206D2F3435270A466F726D61743A20503253480A0A43343533353846413A207470756244396870685A7A43693975355763627471336A5159547A6250763669676F615257447568784C554476355654666645336745566F765961717766564D4361367138564D64774163507046674164616A676D4C4D4C36586759724B42717579594544516731486E4B6D3377510A37334335444130413A20747075624439375578454556586952747A52426D4876523338523751584E7A364478334137674B746F65395567786570644A5845786D4A4364354E7873763859594C674864334D45424B507A52776756614A36326B764253764D746E74626B506E76365066385A6B6E793572433839",
            hex::encode::<Vec<u8>>(data.clone().try_into().unwrap()).to_uppercase()
        );

        let bytes = Bytes::try_from(TryInto::<Vec<u8>>::try_into(data.clone()).unwrap()).unwrap();
        assert_eq!(data.get_bytes(), bytes.get_bytes());
    }

    #[test]
    fn test_create_wallet_config() {
        // single derivation
        {
            let config = "# test Multisig setup file (created on 73C5DA0A)
#
Name: test1
Policy: 2 of 2
Derivation: m/45'
Format: P2SH

C45358FA: tpubD9hphZzCi9u5Wcbtq3jQYTzbPv6igoaRWDuhxLUDv5VTffE3gEVovYaqwfVMCa6q8VMdwAcPpFgAdajgmLML6XgYrKBquyYEDQg1HnKm3wQ
73C5DA0A: tpubD97UxEEVXiRtzRBmHvR38R7QXNz6Dx3A7gKtoe9UgxepdJXExmJCd5Nxsv8YYLgHd3MEBKPzRwgVaJ62kvBSvMtntbkPnv6Pf8Zkny5rC89";
            let xpub_info = vec![
                MultiSigXPubInfo {
                    path: "m/45'".to_string(),
                    xfp: "C45358FA".to_string(),
                    xpub: "tpubD9hphZzCi9u5Wcbtq3jQYTzbPv6igoaRWDuhxLUDv5VTffE3gEVovYaqwfVMCa6q8VMdwAcPpFgAdajgmLML6XgYrKBquyYEDQg1HnKm3wQ".to_string(),
                },
                MultiSigXPubInfo {
                    path: "m/45'".to_string(),
                    xfp: "73C5DA0A".to_string(),
                    xpub: "tpubD97UxEEVXiRtzRBmHvR38R7QXNz6Dx3A7gKtoe9UgxepdJXExmJCd5Nxsv8YYLgHd3MEBKPzRwgVaJ62kvBSvMtntbkPnv6Pf8Zkny5rC89".to_string(),
                },
            ];

            let wallet = create_wallet(
                "test",
                "test1",
                2,
                2,
                "P2SH",
                &xpub_info,
                Network::TestNet,
                "73C5DA0A",
            )
            .unwrap();

            let data = generate_config_data(&wallet, "73C5DA0A", Network::TestNet).unwrap();
            assert_eq!(config, data);
        }

        // multi derivation
        {
            let config = "# test Multisig setup file (created on 748cc6aa)
#
Name: test1
Policy: 2 of 3
Format: P2WSH-P2SH

Derivation: m/48'/0'/0'/1'/12/32/5
748cc6aa: Ypub6q627cv7D98tcXi6LpXLnFnM6gpH3qfr1RJK5gcAM3UkpWWDKX54HhcNc1Wb4BfbU8Ra4478uqPJ3cnLaGBqp4rV9x5N4jjPTeDXEqJTiGs
Derivation: m/48'/0'/0'/1'/5/6/7
5271c071: Ypub6rPbncnQRFnhbWpMUpPxiVALtPVyYrJC93UtZXZoL4eYEQHs9hZmVdTnR6QTbPNVAeV8i4NpbyyCWJb5GK4aa9gYjzfW96umVmCetczSn8y
Derivation: m/48'/0'/0'/1'/110/8/9
c2202a77: Ypub6rJ91C5xKc5R653wqxQ67XdSubmGM8XwdBpVNwDG2Wn4HVi4QntZdr5W9Fp8yMGYsvnazwbTipdtCAu3y8UHQPCDvNJCUgSFxBzyAKgUjKs";

            let xpub_info = vec![
                MultiSigXPubInfo {
                    path: "m/48'/0'/0'/1'/12/32/5".to_string(),
                    xfp: "748cc6aa".to_string(),
                    xpub: "xpub6KMfgiWkVW33LfMbZoGjk6M3CvdZtrzkn38RP2SjbGGU9E85JTXDaX6Jn6bXVqnmq2EnRzWTZxeF3AZ1ZLcssM4DT9GY5RSuJBt1GRx3xm2".to_string(),
                },
                MultiSigXPubInfo {
                    path: "m/48'/0'/0'/1'/5/6/7".to_string(),
                    xfp: "5271c071".to_string(),
                    xpub: "xpub6LfFMiP3hcgrKeTrho9MgKj2zdKGPsd6ufJzrsQNaHSFZ7uj8e1vnSwibBVQ33VfXYJM5zn9G7E9VrMkFPVcdRtH3Brg9ndHLJs8v2QtwHa".to_string(),
                },
                MultiSigXPubInfo {
                    path: "m/48'/0'/0'/1'/110/8/9".to_string(),
                    xfp: "c2202a77".to_string(),
                    xpub: "xpub6LZnaHgbbxyZpChT4w9V5NC91qaZC9rrPoebgH3qGjZmcDKvPjLivfZSKLu5R1PjEpboNsznNwtqBifixCuKTfPxDZVNVN9mnjfTBpafqQf".to_string(),
                },
            ];

            let wallet = create_wallet(
                "test",
                "test1",
                2,
                3,
                "P2WSH-P2SH",
                &xpub_info,
                Network::MainNet,
                "748cc6aa",
            )
            .unwrap();

            let data = generate_config_data(&wallet, "748cc6aa", Network::MainNet).unwrap();

            assert_eq!(config, data);
        }
    }
}
