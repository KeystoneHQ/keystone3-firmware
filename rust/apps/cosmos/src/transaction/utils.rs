use crate::errors::Result;
use alloc::collections::BTreeMap;
use alloc::string::{String, ToString};
use alloc::vec::Vec;

pub fn detect_msg_type(msg_type: Option<&str>) -> &str {
    let msg_type_parts: Vec<&str> = msg_type
        .unwrap_or(&"")
        .split("/")
        .flat_map(|s| s.split("."))
        .collect();
    msg_type_parts[msg_type_parts.len() - 1]
}

pub fn get_network_by_chain_id(chain_id: &str) -> Result<String> {
    // Registered chains https://github.com/cosmos/chain-registry
    let mut map: BTreeMap<&str, &str> = BTreeMap::new();
    map.insert("cosmoshub", "Cosmos Hub");
    map.insert("osmosis", "Osmosis");
    map.insert("secret", "Secret Network");
    map.insert("akashnet", "Akash");
    map.insert("crypto-org-chain-mainnet", "Crypto.org Chain");
    map.insert("iov-mainnet", "Starname");
    map.insert("sifchain", "Sifchain");
    map.insert("shentu", "Shentu");
    map.insert("irishub", "IRISnet");
    map.insert("regen", "Regen");
    map.insert("core", "Persistence");
    map.insert("sentinelhub", "Sentinel");
    map.insert("ixo", "ixo");
    map.insert("emoney", "e-Money");
    map.insert("agoric", "Agoric");
    map.insert("bostrom", "bostrom");
    map.insert("juno", "Juno");
    map.insert("stargaze", "Stargaze");
    map.insert("axelar-dojo", "Axelar");
    map.insert("sommelier", "Sommelier");
    map.insert("umee", "Umee");
    map.insert("gravity-bridge", "Gravity Bridge");
    map.insert("tgrade-mainnet", "Tgrade");
    map.insert("stride", "Stride");
    map.insert("kava_2222", "Kava");
    map.insert("evmos_9001", "Evmos");
    map.insert("evmos_9000", "Evmos Testnet");
    let chain_id_parts: Vec<&str> = chain_id.split("-").collect();
    let chain_id_prefix = chain_id_parts[..chain_id_parts.len() - 1].join("-");
    Ok(map
        .get(chain_id_prefix.as_str())
        .and_then(|v| Some(v.to_string()))
        .unwrap_or("Cosmos Hub".to_string()))
}
