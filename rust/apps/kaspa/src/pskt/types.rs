// Kaspa PSKT (Partially Signed Kaspa Transaction) Types
// Custom lightweight implementation for embedded hardware wallet

use alloc::string::{String, ToString};
use alloc::vec::Vec;
use serde::{Deserialize, Serialize};
use bitcoin::bip32::{DerivationPath, Fingerprint};
use crate::errors::{KaspaError, Result};

/// PSKT format prefix
pub const PSKT_PREFIX: &str = "PSKT";

/// Kaspa PSKT - Partially Signed Kaspa Transaction
/// Kaspa's native format for collaborative transaction construction (not Bitcoin PSBT)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Pskt {
    /// Transaction inputs
    pub inputs: Vec<PsktInput>,
    
    /// Transaction outputs
    pub outputs: Vec<PsktOutput>,
    
    /// Transaction version
    pub version: u16,
    
    /// Transaction lock time
    pub lock_time: u64,
}

/// PSKT Input
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PsktInput {
    /// Previous transaction ID
    pub previous_outpoint_hash: String,
    
    /// Previous output index
    pub previous_outpoint_index: u32,
    
    /// Signature script (scriptSig)
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signature_script: Option<String>,
    
    /// Sequence number
    pub sequence: u64,
    
    /// Amount in sompi (1 KAS = 100,000,000 sompi)
    pub amount: u64,
    
    /// Script public key from the UTXO being spent
    #[serde(skip_serializing_if = "Option::is_none")]
    pub script_public_key: Option<String>,
    
    /// Signature operation count
    #[serde(skip_serializing_if = "Option::is_none")]
    pub sig_op_count: Option<u8>,
    
    /// Public key
    #[serde(skip_serializing_if = "Option::is_none")]
    pub public_key: Option<String>,
    
    /// BIP32 derivation path
    #[serde(skip_serializing_if = "Option::is_none")]
    pub derivation_path: Option<String>,
    
    /// Master key fingerprint
    #[serde(skip_serializing_if = "Option::is_none")]
    pub master_fingerprint: Option<String>,
    
    /// Signature (Schnorr signature)
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signature: Option<String>,
    
    /// SigHash type
    #[serde(skip_serializing_if = "Option::is_none")]
    pub sighash_type: Option<u8>,
}

/// PSKT Output
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PsktOutput {
    /// Amount in sompi
    pub amount: u64,
    
    /// Script public key (scriptPubKey)
    pub script_public_key: String,
    
    /// Kaspa address (optional, for display)
    #[serde(skip_serializing_if = "Option::is_none")]
    pub address: Option<String>,
    
    /// BIP32 derivation path (for change address identification)
    #[serde(skip_serializing_if = "Option::is_none")]
    pub derivation_path: Option<String>,
    
    /// Master key fingerprint (for change address identification)
    #[serde(skip_serializing_if = "Option::is_none")]
    pub master_fingerprint: Option<String>,
}

impl Pskt {
    /// Create new PSKT
    pub fn new(version: u16, lock_time: u64) -> Self {
        Self {
            inputs: Vec::new(),
            outputs: Vec::new(),
            version,
            lock_time,
        }
    }
    
    /// Parse PSKT from hex string
    /// Format: "PSKT{hex_encoded_json}"
    pub fn from_hex(hex_str: &str) -> Result<Self> {
        // Check prefix
        if !hex_str.starts_with(PSKT_PREFIX) {
            return Err(KaspaError::InvalidPskt("Missing PSKT prefix".to_string()));
        }
        
        // Extract hex part
        let hex_data = &hex_str[PSKT_PREFIX.len()..];
        
        // Decode hex
        let json_bytes = hex::decode(hex_data)
            .map_err(|_| KaspaError::HexDecodeError)?;
        
        // Parse JSON
        let json_str = core::str::from_utf8(&json_bytes)
            .map_err(|_| KaspaError::Utf8DecodeError)?;
        
        serde_json::from_str(json_str)
            .map_err(|_| KaspaError::JsonParseError)
    }
    
    /// Serialize PSKT to hex string
    /// Format: "PSKT{hex_encoded_json}"
    pub fn to_hex(&self) -> Result<String> {
        // Serialize to JSON
        let json_str = serde_json::to_string(self)
            .map_err(|e| KaspaError::SerializationFailed(e.to_string()))?;
        
        // Encode to hex
        let hex_data = hex::encode(json_str.as_bytes());
        
        Ok(format!("{}{}", PSKT_PREFIX, hex_data))
    }
    
    /// Add input to PSKT
    pub fn add_input(&mut self, input: PsktInput) {
        self.inputs.push(input);
    }
    
    /// Add output to PSKT
    pub fn add_output(&mut self, output: PsktOutput) {
        self.outputs.push(output);
    }
    
    /// Get total input amount
    pub fn total_input_amount(&self) -> u64 {
        self.inputs.iter().map(|i| i.amount).sum()
    }
    
    /// Get total output amount
    pub fn total_output_amount(&self) -> u64 {
        self.outputs.iter().map(|o| o.amount).sum()
    }
    
    /// Get fee amount
    pub fn fee_amount(&self) -> u64 {
        self.total_input_amount().saturating_sub(self.total_output_amount())
    }
    
    /// Check if all inputs have signatures
    pub fn is_finalized(&self) -> bool {
        self.inputs.iter().all(|i| i.signature.is_some())
    }
}

impl PsktInput {
    /// Create new PSKT input
    pub fn new(
        previous_outpoint_hash: String,
        previous_outpoint_index: u32,
        sequence: u64,
        amount: u64,
    ) -> Self {
        Self {
            previous_outpoint_hash,
            previous_outpoint_index,
            signature_script: None,
            sequence,
            amount,
            script_public_key: None,
            sig_op_count: Some(1), // Default to 1
            public_key: None,
            derivation_path: None,
            master_fingerprint: None,
            signature: None,
            sighash_type: Some(1), // SIGHASH_ALL
        }
    }
    
    /// Set BIP32 derivation info
    pub fn set_derivation_info(
        &mut self,
        path: DerivationPath,
        fingerprint: Fingerprint,
    ) {
        self.derivation_path = Some(path.to_string());
        self.master_fingerprint = Some(hex::encode(fingerprint.as_bytes()));
    }
}

impl PsktOutput {
    /// Create new PSKT output
    pub fn new(amount: u64, script_public_key: String, address: Option<String>) -> Self {
        Self {
            amount,
            script_public_key,
            address,
            derivation_path: None,
            master_fingerprint: None,
        }
    }
    
    /// Set BIP32 derivation info (for change addresses)
    pub fn set_derivation_info(
        &mut self,
        path: DerivationPath,
        fingerprint: Fingerprint,
    ) {
        self.derivation_path = Some(path.to_string());
        self.master_fingerprint = Some(hex::encode(fingerprint.as_bytes()));
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_pskt_serialization() {
        let mut pskt = Pskt::new(1, 0);
        
        pskt.add_input(PsktInput::new(
            "aabbccdd".to_string(),
            0,
            0,
            100_000_000, // 1 KAS
        ));
        
        pskt.add_output(PsktOutput::new(
            99_900_000,
            "script_pubkey".to_string(),
            Some("kaspa:qrtest".to_string()),
        ));
        
        let hex = pskt.to_hex().unwrap();
        assert!(hex.starts_with("PSKT"));
        
        let decoded = Pskt::from_hex(&hex).unwrap();
        assert_eq!(decoded.inputs.len(), 1);
        assert_eq!(decoded.outputs.len(), 1);
        assert_eq!(decoded.fee_amount(), 100_000);
    }
}
