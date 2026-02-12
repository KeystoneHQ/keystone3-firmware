// PSKT Signer - for signing PSKT transactions with Schnorr signatures

use alloc::string::ToString;
use alloc::vec::Vec;
use bitcoin::bip32::{DerivationPath, Fingerprint};
use bitcoin::secp256k1::{Message, Secp256k1, SecretKey};
use blake2b_simd::Params;
use crate::errors::{KaspaError, Result};
use super::types::Pskt;

/// Cached sighash values to avoid O(N²) complexity for multi-input transactions
/// These values are computed once and reused for all inputs
struct SighashCache {
    prev_outputs_hash: [u8; 32],
    sequences_hash: [u8; 32],
    sig_op_counts_hash: [u8; 32],
}

/// PSKT Signer for signing inputs
pub struct PsktSigner<'a> {
    pskt: &'a mut Pskt,
}

impl<'a> PsktSigner<'a> {
    /// Create new PSKT signer
    pub fn new(pskt: &'a mut Pskt) -> Self {
        Self { pskt }
    }
    
    /// Sign all inputs that match the given master fingerprint
    pub fn sign(&mut self, seed: &[u8], master_fingerprint: Fingerprint) -> Result<()> {
        // OPTIMIZATION: Pre-compute global hashes once to avoid O(N²) complexity
        // For a transaction with N inputs, this saves (N-1) redundant hash calculations
        let hash_type = self.pskt.inputs.first()
            .and_then(|i| i.sighash_type)
            .unwrap_or(1);
        
        let cache = SighashCache {
            prev_outputs_hash: self.hash_previous_outputs(hash_type)?,
            sequences_hash: self.hash_sequences(hash_type)?,
            sig_op_counts_hash: self.hash_sig_op_counts(hash_type)?,
        };
        
        // Create local Secp256k1 context
        let secp = Secp256k1::new();

        // Collect signing data for inputs that need to be signed
        let mut signing_data = Vec::new();
        
        for (input_index, input) in self.pskt.inputs.iter().enumerate() {
            // Check if this input should be signed by this key
            if let Some(ref mfp_hex) = input.master_fingerprint {
                // OPTIMIZATION: Master fingerprint is always 4 bytes
                // Use stack-allocated array instead of heap allocation
                let mut input_mfp = [0u8; 4];
                hex::decode_to_slice(mfp_hex, &mut input_mfp)
                    .map_err(|_| KaspaError::InvalidMasterFingerprint)?;
                
                let input_fingerprint = Fingerprint::from(input_mfp);
                
                if input_fingerprint != master_fingerprint {
                    continue; // Not our key
                }
            } else {
                continue; // No fingerprint info
            }
            
            // Get derivation path
            let path = match &input.derivation_path {
                Some(p) => p.parse::<DerivationPath>()
                    .map_err(|e| KaspaError::InvalidDerivationPath(e.to_string()))?,
                None => continue, // No path info
            };
            
            // SECURITY: Verify script_public_key matches the derived address
            // This prevents blind signing attacks where malicious software
            // provides a valid signature path but malicious script_public_key
            verify_script_public_key(seed, &path, input)?;
            
            // Calculate sighash using cached global hashes
            let sighash = self.calculate_sighash(input_index, &cache)?;
            
            signing_data.push((input_index, path, sighash));
        }
        
        // Now sign all inputs
        for (input_index, path, sighash) in signing_data {
            // Derive private key
            let private_key = derive_private_key(seed, &path)?;

            // Derive public key
            let public_key = bitcoin::secp256k1::PublicKey::from_secret_key(&secp, &private_key);

            // Sign with Schnorr signature (BIP-340)
            let message = Message::from_digest(sighash);
            let keypair = bitcoin::secp256k1::Keypair::from_secret_key(&secp, &private_key);
            let signature = secp.sign_schnorr_no_aux_rand(&message, &keypair);
            
            // Store signature and public key
            let input = &mut self.pskt.inputs[input_index];
            input.public_key = Some(hex::encode(public_key.serialize()));
            input.signature = Some(hex::encode(signature.as_ref()));
        }
        
        Ok(())
    }
    
    /// Calculate sighash for input
    /// Uses Blake2b-256 personalized hash with "TransactionSigningHash"
    /// Following official Kaspa implementation:
    /// https://github.com/kaspanet/rusty-kaspa/blob/master/consensus/core/src/hashing/sighash.rs
    fn calculate_sighash(&self, input_index: usize, cache: &SighashCache) -> Result<[u8; 32]> {
        let input = self.pskt.inputs.get(input_index)
            .ok_or_else(|| KaspaError::InvalidPskt("Input index out of bounds".to_string()))?;
        
        let hash_type = input.sighash_type.unwrap_or(1); // SIG_HASH_ALL = 1
        
        // SECURITY: Hardware wallet ONLY signs SIG_HASH_ALL transactions
        // This prevents malicious apps from tricking users into signing dangerous transactions:
        // - SIG_HASH_NONE (0x02): Allows attacker to modify all outputs after signing
        // - SIG_HASH_SINGLE (0x03): Only protects one output, others can be modified
        // - SIG_HASH_ANYONECANPAY (0x80): Allows attacker to add inputs
        // All legitimate Kaspa wallets use SIG_HASH_ALL. Reject anything else.
        const SIG_HASH_ALL: u8 = 1;
        if hash_type != SIG_HASH_ALL {
            // FIRMWARE SAFETY: Use static string to avoid heap allocation in error path
            // The actual hash_type value can be logged/displayed by UI layer if needed
            return Err(KaspaError::UnsupportedSighashType);
        }
        
        // Create hasher with official domain separation
        // NOTE: Kaspa uses .key() instead of .personal() to avoid 16-byte limitation
        // Reference: rusty-kaspa/crypto/hashes/src/hashers.rs
        let mut hasher = Params::new()
            .hash_length(32)
            .key(b"TransactionSigningHash")
            .to_state();
        
        // 1. Version (u16 little-endian)
        hasher.update(&self.pskt.version.to_le_bytes());
        
        // 2. Previous outputs hash (use cached value)
        hasher.update(&cache.prev_outputs_hash);
        
        // 3. Sequences hash (use cached value)
        hasher.update(&cache.sequences_hash);
        
        // 4. Sig op counts hash (use cached value)
        hasher.update(&cache.sig_op_counts_hash);
        
        // 5. Current input's outpoint
        // OPTIMIZATION: Use stack-allocated array instead of heap allocation (Vec)
        let mut prev_hash = [0u8; 32];
        hex::decode_to_slice(&input.previous_outpoint_hash, &mut prev_hash)
            .map_err(|_| KaspaError::InvalidTransactionHash)?;
        hasher.update(&prev_hash);
        hasher.update(&input.previous_outpoint_index.to_le_bytes());
        
        // 6. Script public key
        self.hash_script_public_key(&mut hasher, input)?;
        
        // 7. Amount (u64 little-endian)
        hasher.update(&input.amount.to_le_bytes());
        
        // 8. Sequence (u64 little-endian)
        hasher.update(&input.sequence.to_le_bytes());
        
        // 9. Sig op count (u8)
        hasher.update(&[input.sig_op_count.unwrap_or(1)]);
        
        // 10. Outputs hash
        let outputs_hash = self.hash_outputs(hash_type, input_index)?;
        hasher.update(&outputs_hash);
        
        // 11. Lock time (u64 little-endian)
        hasher.update(&self.pskt.lock_time.to_le_bytes());
        
        // 12. Subnetwork ID (assuming native = all zeros, 20 bytes)
        // FUTURE: If Kaspa introduces sidechains/subnets, this must be read from PSKT
        // Currently all mainnet transactions use Native subnetwork (all zeros)
        hasher.update(&[0u8; 20]);
        
        // 13. Gas (u64 = 0 for native)
        hasher.update(&0u64.to_le_bytes());
        
        // 14. Payload hash (empty for native transactions)
        hasher.update(&[0u8; 32]); // ZERO_HASH
        
        // 15. Sighash type (u8)
        hasher.update(&[hash_type]);
        
        let hash = hasher.finalize();
        let mut result = [0u8; 32];
        result.copy_from_slice(hash.as_bytes());
        Ok(result)
    }
    
    /// Hash all previous outputs (outpoints)
    /// Returns ZERO_HASH if SIGHASH_ANYONECANPAY
    /// NOTE: This function should never be called with non-SIG_HASH_ALL due to validation in calculate_sighash
    fn hash_previous_outputs(&self, hash_type: u8) -> Result<[u8; 32]> {
        // SIG_HASH_ANY_ONE_CAN_PAY = 0x80
        // Note: Should be unreachable due to strict validation in calculate_sighash
        if hash_type & 0x80 != 0 {
            return Ok([0u8; 32]); // ZERO_HASH
        }
        
        let mut hasher = Params::new()
            .hash_length(32)
            .key(b"TransactionSigningHash")
            .to_state();
        
        for input in &self.pskt.inputs {
            // OPTIMIZATION: Use stack-allocated array to avoid heap allocation
            let mut tx_id = [0u8; 32];
            hex::decode_to_slice(&input.previous_outpoint_hash, &mut tx_id)
                .map_err(|_| KaspaError::InvalidTransactionHash)?;
            hasher.update(&tx_id);
            hasher.update(&input.previous_outpoint_index.to_le_bytes());
        }
        
        let hash = hasher.finalize();
        let mut result = [0u8; 32];
        result.copy_from_slice(hash.as_bytes());
        Ok(result)
    }
    
    /// Hash all sequences
    /// Returns ZERO_HASH if SIGHASH_SINGLE, SIGHASH_ANYONECANPAY, or SIGHASH_NONE
    /// NOTE: This function should never be called with non-SIG_HASH_ALL due to validation in calculate_sighash
    fn hash_sequences(&self, hash_type: u8) -> Result<[u8; 32]> {
        // SIG_HASH_SINGLE = 3, SIG_HASH_NONE = 2, SIG_HASH_ANY_ONE_CAN_PAY = 0x80
        // Note: Should be unreachable due to strict validation in calculate_sighash
        if hash_type == 3 || hash_type & 0x80 != 0 || hash_type == 2 {
            return Ok([0u8; 32]); // ZERO_HASH
        }
        
        let mut hasher = Params::new()
            .hash_length(32)
            .key(b"TransactionSigningHash")
            .to_state();
        
        for input in &self.pskt.inputs {
            hasher.update(&input.sequence.to_le_bytes());
        }
        
        let hash = hasher.finalize();
        let mut result = [0u8; 32];
        result.copy_from_slice(hash.as_bytes());
        Ok(result)
    }
    
    /// Hash all sig op counts
    /// Returns ZERO_HASH if SIGHASH_ANYONECANPAY
    /// NOTE: This function should never be called with non-SIG_HASH_ALL due to validation in calculate_sighash
    fn hash_sig_op_counts(&self, hash_type: u8) -> Result<[u8; 32]> {
        // SIG_HASH_ANY_ONE_CAN_PAY = 0x80
        // Note: Should be unreachable due to strict validation in calculate_sighash
        if hash_type & 0x80 != 0 {
            return Ok([0u8; 32]); // ZERO_HASH
        }
        
        let mut hasher = Params::new()
            .hash_length(32)
            .key(b"TransactionSigningHash")
            .to_state();
        
        for input in &self.pskt.inputs {
            hasher.update(&[input.sig_op_count.unwrap_or(1)]);
        }
        
        let hash = hasher.finalize();
        let mut result = [0u8; 32];
        result.copy_from_slice(hash.as_bytes());
        Ok(result)
    }
    
    /// Hash script public key
    /// Format: version (u16) + script_length (varint) + script
    fn hash_script_public_key(&self, hasher: &mut blake2b_simd::State, input: &super::types::PsktInput) -> Result<()> {
        // Default version = 0
        hasher.update(&0u16.to_le_bytes());
        
        if let Some(ref script_hex) = input.script_public_key {
            // OPTIMIZATION: Kaspa P2PK script is always 34 bytes (0x20 + 32-byte-pubkey + 0xac)
            // Use stack-allocated array instead of heap allocation
            let mut script = [0u8; 34];
            hex::decode_to_slice(script_hex, &mut script)
                .map_err(|_| KaspaError::InvalidScript)?;
            
            // Write variable length
            self.write_varint(hasher, script.len() as u64);
            hasher.update(&script);
        } else {
            // Empty script
            self.write_varint(hasher, 0);
        }
        
        Ok(())
    }
    
    /// Write variable-length integer
    fn write_varint(&self, hasher: &mut blake2b_simd::State, n: u64) {
        if n < 0xfd {
            hasher.update(&[n as u8]);
        } else if n <= 0xffff {
            hasher.update(&[0xfd]);
            hasher.update(&(n as u16).to_le_bytes());
        } else if n <= 0xffffffff {
            hasher.update(&[0xfe]);
            hasher.update(&(n as u32).to_le_bytes());
        } else {
            hasher.update(&[0xff]);
            hasher.update(&n.to_le_bytes());
        }
    }
    
    /// Hash all outputs
    /// Returns ZERO_HASH if SIGHASH_NONE
    /// Returns single output hash if SIGHASH_SINGLE
    /// NOTE: This function should never be called with non-SIG_HASH_ALL due to validation in calculate_sighash
    fn hash_outputs(&self, hash_type: u8, input_index: usize) -> Result<[u8; 32]> {
        // SIG_HASH_NONE = 2
        // Note: Should be unreachable due to strict validation in calculate_sighash
        if hash_type == 2 {
            return Ok([0u8; 32]); // ZERO_HASH
        }
        
        // SIG_HASH_SINGLE = 3
        // Note: Should be unreachable due to strict validation in calculate_sighash
        if hash_type == 3 {
            // If the relevant output exists - return its hash, otherwise return zero-hash
            if input_index >= self.pskt.outputs.len() {
                return Ok([0u8; 32]); // ZERO_HASH
            }
            
            let mut hasher = Params::new()
                .hash_length(32)
                .key(b"TransactionSigningHash")
                .to_state();
            
            self.hash_single_output(&mut hasher, &self.pskt.outputs[input_index])?;
            
            let hash = hasher.finalize();
            let mut result = [0u8; 32];
            result.copy_from_slice(hash.as_bytes());
            return Ok(result);
        }
        
        // SIG_HASH_ALL (default) - hash all outputs
        let mut hasher = Params::new()
            .hash_length(32)
            .key(b"TransactionSigningHash")
            .to_state();
        
        for output in &self.pskt.outputs {
            self.hash_single_output(&mut hasher, output)?;
        }
        
        let hash = hasher.finalize();
        let mut result = [0u8; 32];
        result.copy_from_slice(hash.as_bytes());
        Ok(result)
    }
    
    /// Hash a single output
    /// Format: amount (u64) + script_public_key
    fn hash_single_output(&self, hasher: &mut blake2b_simd::State, output: &super::types::PsktOutput) -> Result<()> {
        // Amount
        hasher.update(&output.amount.to_le_bytes());
        
        // Script public key - version (u16) + length (varint) + script
        hasher.update(&0u16.to_le_bytes()); // version = 0
        
        // OPTIMIZATION: Kaspa P2PK script is always 34 bytes
        // Use stack-allocated array instead of heap allocation
        let mut script = [0u8; 34];
        hex::decode_to_slice(&output.script_public_key, &mut script)
            .map_err(|_| KaspaError::InvalidScript)?;
        
        self.write_varint(hasher, script.len() as u64);
        hasher.update(&script);
        
        Ok(())
    }
}

// Note: PsktFinalizer removed
// Finalization (building signature_script from signature + pubkey)
// should be done by software wallet (Kaspium) using kaspa_consensus_core

// Note: extract_transaction removed
// Transaction extraction should be done by software wallet (Kaspium)
// The software wallet will:
// 1. Receive signed PSKT from hardware wallet
// 2. Finalize: build signature_script from signature + pubkey  
// 3. Extract: construct kaspa_consensus_core::Transaction
// 4. Broadcast to Kaspa network

/// Verify that script_public_key matches the derived public key
/// This prevents blind signing attacks where PSKT provides valid path but malicious script
fn verify_script_public_key(
    seed: &[u8],
    path: &DerivationPath,
    input: &super::types::PsktInput,
) -> Result<()> {
    // Derive the expected public key from the path
    let private_key = derive_private_key(seed, path)?;
    let secp = Secp256k1::new();
    let public_key = bitcoin::secp256k1::PublicKey::from_secret_key(&secp, &private_key);
    
    // Get x-only public key for Schnorr (BIP-340)
    let (x_only_pubkey, _parity) = public_key.x_only_public_key();
    let pubkey_bytes = x_only_pubkey.serialize();
    
    // Construct expected P2PK script for Kaspa
    // Format: OP_DATA_32 <32-byte-x-only-pubkey> OP_CHECKSIG
    // Opcodes: 0x20 (32 bytes) + pubkey + 0xac (OP_CHECKSIG)
    let mut expected_script = Vec::with_capacity(34);
    expected_script.push(0x20); // OP_DATA_32
    expected_script.extend_from_slice(&pubkey_bytes);
    expected_script.push(0xac); // OP_CHECKSIG
    
    // Get actual script from PSKT
    let actual_script_hex = input.script_public_key
        .as_ref()
        .ok_or_else(|| KaspaError::InvalidPskt("Missing script_public_key".to_string()))?;
    
    // OPTIMIZATION: Use stack-allocated array (34 bytes for P2PK script)
    let mut actual_script = [0u8; 34];
    hex::decode_to_slice(actual_script_hex, &mut actual_script)
        .map_err(|_| KaspaError::InvalidScript)?;
    
    // Compare scripts
    if expected_script[..] != actual_script[..] {
        return Err(KaspaError::InvalidPskt(
            "script_public_key does not match derived address - possible blind signing attack".to_string()
        ));
    }
    
    Ok(())
}

/// Derive private key from seed and path
fn derive_private_key(seed: &[u8], path: &DerivationPath) -> Result<SecretKey> {
    use bitcoin::bip32::Xpriv;
    use bitcoin::Network;
    
    // Create master key from seed
    // Note: Network parameter doesn't affect the derived private key itself.
    // BIP32 key derivation (HMAC-SHA512) is identical across all chains.
    // Network only affects the serialization format (xprv prefix), which we don't use.
    let master_key = Xpriv::new_master(Network::Bitcoin, seed)
        .map_err(|e| KaspaError::KeyDerivationFailed(e.to_string()))?;

    // Use a local Secp256k1 context for derivation
    let secp = Secp256k1::new();
    let derived_key = master_key.derive_priv(&secp, path)
        .map_err(|e| KaspaError::KeyDerivationFailed(e.to_string()))?;
    
    Ok(derived_key.private_key)
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_signer() {
        // This is a placeholder test
        // Real test would require valid seed and PSKT
        assert!(true);
    }
}
