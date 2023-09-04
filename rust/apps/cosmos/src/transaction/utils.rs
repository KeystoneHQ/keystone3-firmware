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
    map.insert("crypto-org-chain-mainnet", "Crypto.org");
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
    map.insert("evmos_9001", "Evmos");
    map.insert("evmos_9000", "Evmos Testnet");
    map.insert("injective", "Injective");
    map.insert("kava_2222", "Kava");
    map.insert("quicksilver", "Quicksilver");
    map.insert("phoenix", "Terra");
    map.insert("columbus", "Terra Classic");
    let chain_id_parts: Vec<&str> = chain_id.split("-").collect();
    let chain_id_prefix = chain_id_parts[..chain_id_parts.len() - 1].join("-");
    Ok(map
        .get(chain_id_prefix.as_str())
        .and_then(|v| Some(v.to_string()))
        .unwrap_or("Cosmos Hub".to_string()))
}

pub fn get_chain_id_by_address(address: &str) -> String {
    let mut map: BTreeMap<&str, &str> = BTreeMap::new();
    map.insert("cosmos", "cosmoshub-4");
    map.insert("osmo", "osmosis-1");
    map.insert("secret", "secret-4");
    map.insert("akash", "akashnet-2");
    map.insert("cro", "crypto-org-chain-mainnet-1");
    map.insert("sif", "sifchain-1");
    map.insert("certik", "shentu-2.2");
    map.insert("iaa", "irishub-1");
    map.insert("regen", "regen-1");
    map.insert("persistence", "core-1");
    map.insert("sent", "sentinelhub-2");
    map.insert("ixo", "ixo-4");
    map.insert("emoney", "emoney-3");
    map.insert("agoric", "agoric-3");
    map.insert("bostrom", "bostrom");
    map.insert("juno", "juno-1");
    map.insert("stars", "stargaze-1");
    map.insert("star", "iov-mainnet-ibc");
    map.insert("axelar", "axelar-dojo-1");
    map.insert("somm", "sommelier-3");
    map.insert("umee", "umee-1");
    map.insert("gravity", "gravity-bridge-3");
    map.insert("tgrade", "tgrade-mainnet-1");
    map.insert("stride", "stride-1");
    map.insert("evmos", "evmos_9001-2");
    map.insert("inj", "injective-1");
    map.insert("kava", "kava_2222-10");
    map.insert("quick", "quicksilver-1");
    map.insert("terra", "phoenix-1");
    for (k, v) in map {
        if address.starts_with(k) {
            return v.to_string();
        }
    }
    "cosmoshub-4".to_string()
}
