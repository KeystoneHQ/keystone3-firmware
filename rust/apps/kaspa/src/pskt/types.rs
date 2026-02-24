// apps/kaspa/src/pskt/types.rs

use crate::errors::{KaspaError, Result};
use alloc::collections::BTreeMap;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use bitcoin::bip32::{DerivationPath, Fingerprint};
use serde::{Deserialize, Serialize};

pub const PSKT_PREFIX: &str = "PSKT";

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Pskt {
    pub inputs: Vec<PsktInput>,
    pub outputs: Vec<PsktOutput>,
    #[serde(default)]
    pub version: u16,
    #[serde(default)]
    pub lock_time: u64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct PsktInput {
    pub previous_outpoint: KaspaOutpoint,
    pub sequence: u64,
    /// The hardware wallet will prioritize reading this amount for display purposes
    #[serde(default)]
    pub amount: u64,
    
    /// Also retains utxo_entry to adapt to the official structure
    /// If Kaspium passes a nested utxo_entry, serde will parse it here
    pub utxo_entry: Option<KaspaUtxoEntry>,

    #[serde(default)]
    pub bip32_derivations: BTreeMap<String, Option<KeySource>>,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub sig_op_count: Option<u8>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub redeem_script: Option<String>,
    #[serde(default)]
    pub partial_sigs: BTreeMap<String, String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signature: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub sighash_type: Option<u8>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct PsktOutput {
    pub amount: u64,
    pub script_public_key: String,

    #[serde(skip_serializing_if = "Option::is_none")]
    pub address: Option<String>,

    #[serde(default)]
    pub bip32_derivations: BTreeMap<String, Option<KeySource>>,

    #[serde(flatten)]
    pub unknowns: BTreeMap<String, serde_json::Value>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct KaspaOutpoint {
    pub transaction_id: String,
    pub index: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct KaspaUtxoEntry {
    pub amount: u64,
    pub script_public_key: String,
    #[serde(default)]
    pub block_daa_score: u64,
    #[serde(default)]
    pub is_coinbase: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct KeySource {
    pub key_fingerprint: String,
    pub derivation_path: String,
}


impl Pskt {
    pub fn from_hex(hex_str: &str) -> Result<Self> {
        //If the UR decoded from Kaspium contains a prefix, remove it; otherwise, parse directly
        let clean_hex = if hex_str.starts_with(PSKT_PREFIX) {
            &hex_str[PSKT_PREFIX.len()..]
        } else {
            hex_str
        };

        let json_bytes = hex::decode(clean_hex).map_err(|_| KaspaError::HexDecodeError)?;
        let json_str =
            core::str::from_utf8(&json_bytes).map_err(|_| KaspaError::Utf8DecodeError)?;

        serde_json::from_str(json_str).map_err(|_| KaspaError::JsonParseError)
    }

    pub fn to_hex(&self) -> Result<String> {
        let json_str = serde_json::to_string(self)
            .map_err(|e| KaspaError::SerializationFailed(e.to_string()))?;
        Ok(format!(
            "{}{}",
            PSKT_PREFIX,
            hex::encode(json_str.as_bytes())
        ))
    }

    pub fn total_input_amount(&self) -> u64 {
        // Prefer amount from utxo_entry if available, otherwise use input.amount
        self.inputs.iter().map(|i| {
            if i.amount > 0 { i.amount } 
            else { i.utxo_entry.as_ref().map(|e| e.amount).unwrap_or(0) }
        }).sum()
    }

    pub fn total_output_amount(&self) -> u64 {
        self.outputs.iter().map(|o| o.amount).sum()
    }
}

impl PsktInput {
    pub fn new(transaction_id: String, index: u32, sequence: u64, amount: u64) -> Self {
        Self {
            previous_outpoint: KaspaOutpoint {
                transaction_id,
                index,
            },
            sequence,
            amount,
            utxo_entry: None,
            bip32_derivations: BTreeMap::new(),
            sig_op_count: Some(1),
            redeem_script: None,
            partial_sigs: BTreeMap::new(),
            signature: None,
            sighash_type: Some(1),
        }
    }

    pub fn set_derivation_info(
        &mut self,
        pubkey_hex: String,
        path: DerivationPath,
        fingerprint: Fingerprint,
    ) {
        let source = KeySource {
            key_fingerprint: hex::encode(fingerprint.as_bytes()),
            derivation_path: path.to_string(),
        };
        self.bip32_derivations.insert(pubkey_hex, Some(source));
    }
}

impl PsktOutput {
    pub fn new(amount: u64, script_public_key: String, address: Option<String>) -> Self {
        Self {
            amount,
            script_public_key,
            address,
            bip32_derivations: BTreeMap::new(),
            unknowns: BTreeMap::new(),
        }
    }

    pub fn set_derivation_info(
        &mut self,
        pubkey_hex: String,
        path: DerivationPath,
        fingerprint: Fingerprint,
    ) {
        let source = KeySource {
            key_fingerprint: hex::encode(fingerprint.as_bytes()),
            derivation_path: path.to_string(),
        };
        self.bip32_derivations.insert(pubkey_hex, Some(source));
    }
}
