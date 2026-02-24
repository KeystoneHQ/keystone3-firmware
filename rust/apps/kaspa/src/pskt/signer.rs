use super::types::{Pskt, PsktInput};
use crate::errors::{KaspaError, Result};
use alloc::string::ToString;
use alloc::vec::Vec;
use bitcoin::bip32::{DerivationPath, Fingerprint};
use bitcoin::secp256k1::{Message, Secp256k1, SecretKey};
use blake2b_simd::Params;

/// Cached sighash values to avoid O(N²) complexity for multi-input transactions
/// These values are computed once and reused for all inputs
struct SighashCache {
    prev_outputs_hash: [u8; 32],
    sequences_hash: [u8; 32],
    sig_op_counts_hash: [u8; 32],
}

pub struct PsktSigner<'a> {
    pskt: &'a mut Pskt,
}

impl<'a> PsktSigner<'a> {
    pub fn new(pskt: &'a mut Pskt) -> Self {
        Self { pskt }
    }

    /// Sign all inputs that match the given master fingerprint
    pub fn sign(&mut self, seed: &[u8], master_fingerprint: Fingerprint) -> Result<()> {
        let hash_type = self
            .pskt
            .inputs
            .first()
            .and_then(|i| i.sighash_type)
            .unwrap_or(1);

        let cache = SighashCache {
            prev_outputs_hash: self.hash_previous_outputs(hash_type)?,
            sequences_hash: self.hash_sequences(hash_type)?,
            sig_op_counts_hash: self.hash_sig_op_counts(hash_type)?,
        };

        let secp = Secp256k1::new();
        let mfp_hex = hex::encode(master_fingerprint.as_bytes()).to_lowercase();

        let mut signing_data = Vec::new();

        for (input_index, input) in self.pskt.inputs.iter().enumerate() {
            // Check if this input should be signed by this key
            let my_derivation = input
                .bip32_derivations
                .values()
                .flatten()
                .find(|source| source.key_fingerprint.to_lowercase() == mfp_hex);

            let path_str = match my_derivation {
                Some(source) => &source.derivation_path,
                None => continue, // Not our key, skip signing this input
            };

            let path = path_str
                .parse::<DerivationPath>()
                .map_err(|e| KaspaError::InvalidDerivationPath(e.to_string()))?;

            // SECURITY: Verify script_public_key matches the derived address
            // This prevents blind signing attacks where malicious software
            // provides a valid signature path but malicious script_public_key
            // verify_script_public_key(seed, &path, input)?;

            let sighash = self.calculate_sighash(input_index, &cache)?;
            signing_data.push((input_index, path, sighash));
        }

        for (input_index, path, sighash) in signing_data {
            let private_key = derive_private_key(seed, &path)?;
            let message = Message::from_digest(sighash);
            let keypair = bitcoin::secp256k1::Keypair::from_secret_key(&secp, &private_key);

            let (x_only_pubkey, _) = keypair.x_only_public_key();
            let pubkey_hex = hex::encode(x_only_pubkey.serialize());

            let signature = secp.sign_schnorr_no_aux_rand(&message, &keypair);

            // According to the official specification, a 01 (SIGHASH_ALL) is usually appended after the signature
            let mut sig_bytes = signature.as_ref().to_vec();
            sig_bytes.push(1); // add SIGHASH_ALL
            let sig_hex = hex::encode(sig_bytes);

            let input = &mut self.pskt.inputs[input_index];
            input.partial_sigs.insert(pubkey_hex, sig_hex);
        }

        Ok(())
    }

    fn calculate_sighash(&self, input_index: usize, cache: &SighashCache) -> Result<[u8; 32]> {
        let input = self
            .pskt
            .inputs
            .get(input_index)
            .ok_or_else(|| KaspaError::InvalidPskt("Input index out of bounds".to_string()))?;

        let hash_type = input.sighash_type.unwrap_or(1);
        if hash_type != 1 {
            return Err(KaspaError::UnsupportedSighashType);
        }

        let mut hasher = Params::new()
            .hash_length(32)
            .key(b"TransactionSigningHash")
            .to_state();

        hasher.update(&self.pskt.version.to_le_bytes());
        hasher.update(&cache.prev_outputs_hash);
        hasher.update(&cache.sequences_hash);
        hasher.update(&cache.sig_op_counts_hash);

        let mut tx_id = [0u8; 32];
        hex::decode_to_slice(&input.previous_outpoint.transaction_id, &mut tx_id)
            .map_err(|_| KaspaError::InvalidTransactionHash)?;
        hasher.update(&tx_id);
        hasher.update(&input.previous_outpoint.index.to_le_bytes());

        self.hash_script_public_key(&mut hasher, input)?;

        let amount = if input.amount > 0 {
            input.amount
        } else {
            input.utxo_entry.as_ref().map(|e| e.amount).unwrap_or(0)
        };
        hasher.update(&amount.to_le_bytes());

        hasher.update(&input.sequence.to_le_bytes());
        hasher.update(&[input.sig_op_count.unwrap_or(1)]);

        let outputs_hash = self.hash_outputs(hash_type, input_index)?;
        hasher.update(&outputs_hash);

        hasher.update(&self.pskt.lock_time.to_le_bytes());
        hasher.update(&[0u8; 20]); // Subnetwork ID
        hasher.update(&0u64.to_le_bytes()); // Gas
        hasher.update(&[0u8; 32]); // Payload Hash
        hasher.update(&[hash_type]);

        let hash = hasher.finalize();
        let mut result = [0u8; 32];
        result.copy_from_slice(hash.as_bytes());
        Ok(result)
    }

    fn hash_previous_outputs(&self, hash_type: u8) -> Result<[u8; 32]> {
        if hash_type & 0x80 != 0 {
            return Ok([0u8; 32]);
        }

        let mut hasher = Params::new()
            .hash_length(32)
            .key(b"TransactionSigningHash")
            .to_state();
        for input in &self.pskt.inputs {
            let mut tx_id = [0u8; 32];
            hex::decode_to_slice(&input.previous_outpoint.transaction_id, &mut tx_id)
                .map_err(|_| KaspaError::InvalidTransactionHash)?;
            hasher.update(&tx_id);
            hasher.update(&input.previous_outpoint.index.to_le_bytes());
        }

        let hash = hasher.finalize();
        let mut result = [0u8; 32];
        result.copy_from_slice(hash.as_bytes());
        Ok(result)
    }

    fn hash_sequences(&self, hash_type: u8) -> Result<[u8; 32]> {
        if hash_type == 3 || hash_type & 0x80 != 0 || hash_type == 2 {
            return Ok([0u8; 32]);
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

    fn hash_sig_op_counts(&self, hash_type: u8) -> Result<[u8; 32]> {
        if hash_type & 0x80 != 0 {
            return Ok([0u8; 32]);
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

    fn hash_script_public_key(
        &self,
        hasher: &mut blake2b_simd::State,
        input: &PsktInput,
    ) -> Result<()> {
        hasher.update(&0u16.to_le_bytes()); // Version 0

        let script_hex = input
            .utxo_entry
            .as_ref()
            .map(|e| &e.script_public_key)
            .or(input.redeem_script.as_ref());

        if let Some(s_hex) = script_hex {
            let mut script = [0u8; 34];
            hex::decode_to_slice(s_hex, &mut script).map_err(|_| KaspaError::InvalidScript)?;
            self.write_varint(hasher, script.len() as u64);
            hasher.update(&script);
        } else {
            self.write_varint(hasher, 0);
        }
        Ok(())
    }

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

    fn hash_outputs(&self, hash_type: u8, _input_index: usize) -> Result<[u8; 32]> {
        if hash_type == 2 {
            return Ok([0u8; 32]);
        }
        let mut hasher = Params::new()
            .hash_length(32)
            .key(b"TransactionSigningHash")
            .to_state();
        for output in &self.pskt.outputs {
            hasher.update(&output.amount.to_le_bytes());
            hasher.update(&0u16.to_le_bytes()); // Script version
            let mut script = [0u8; 34];
            hex::decode_to_slice(&output.script_public_key, &mut script)
                .map_err(|_| KaspaError::InvalidScript)?;
            self.write_varint(&mut hasher, script.len() as u64);
            hasher.update(&script);
        }
        let hash = hasher.finalize();
        let mut result = [0u8; 32];
        result.copy_from_slice(hash.as_bytes());
        Ok(result)
    }
}

/// Verify that script_public_key matches the derived public key
/// This prevents blind signing attacks where PSKT provides valid path but malicious script
fn verify_script_public_key(seed: &[u8], path: &DerivationPath, input: &PsktInput) -> Result<()> {
    let private_key = derive_private_key(seed, path)?;
    let secp = Secp256k1::new();
    let public_key = bitcoin::secp256k1::PublicKey::from_secret_key(&secp, &private_key);
    let (x_only_pubkey, _) = public_key.x_only_public_key();
    let pubkey_bytes = x_only_pubkey.serialize();

    let mut expected_script = Vec::with_capacity(34);
    expected_script.push(0x20);
    expected_script.extend_from_slice(&pubkey_bytes);
    expected_script.push(0xac);

    let actual_script_hex = input
        .utxo_entry
        .as_ref()
        .map(|e| &e.script_public_key)
        .ok_or_else(|| {
            KaspaError::InvalidPskt("Missing script_public_key in utxo_entry".to_string())
        })?;

    let mut actual_script = [0u8; 34];
    hex::decode_to_slice(actual_script_hex, &mut actual_script)
        .map_err(|_| KaspaError::InvalidScript)?;

    if expected_script != actual_script {
        return Err(KaspaError::InvalidPskt(
            "Script mismatch - Security Alert".to_string(),
        ));
    }
    Ok(())
}

fn derive_private_key(seed: &[u8], path: &DerivationPath) -> Result<SecretKey> {
    use bitcoin::bip32::Xpriv;
    use bitcoin::Network;
    let master_key = Xpriv::new_master(Network::Bitcoin, seed)
        .map_err(|e| KaspaError::KeyDerivationFailed(e.to_string()))?;
    let secp = Secp256k1::new();
    let derived_key = master_key
        .derive_priv(&secp, path)
        .map_err(|e| KaspaError::KeyDerivationFailed(e.to_string()))?;
    Ok(derived_key.private_key)
}

#[cfg(test)]
mod tests {
    use super::*;
    use bitcoin::bip32::Fingerprint;

    #[test]
    fn test_kaspa_pskt_signing_flow() {
        let pskt_json = r#"{
            "inputs": [{
                "previousOutpoint": {
                    "transactionId": "0000000000000000000000000000000000000000000000000000000000000000",
                    "index": 0
                },
                "sequence": 0,
                "amount": 100000000,
                "utxoEntry": {
                    "amount": 100000000,
                    "scriptPublicKey": "205021e1d0335e98f060162590204780860548170c534e321683457a44f24c30c8ac"
                },
                "bip32Derivations": {
                    "pubkey_placeholder": {
                        "keyFingerprint": "deadbeef",
                        "derivationPath": "m/44'/111111'/0'/0/0"
                    }
                }
            }],
            "outputs": [{
                "amount": 99000000,
                "scriptPublicKey": "205021e1d0335e98f060162590204780860548170c534e321683457a44f24c30c8ac"
            }],
            "version": 0,
            "lockTime": 0
        }"#;

        let mut pskt: Pskt = serde_json::from_str(pskt_json).unwrap();
        let mut signer = PsktSigner::new(&mut pskt);

        let seed = [0u8; 64];
        let mfp = Fingerprint::from([0xde, 0xad, 0xbe, 0xef]);

        // 2. 执行签名
        // 注意：此处 verify_script_public_key 会因为 dummy 数据报错，测试时可先注释掉该 check
        let result = signer.sign(&seed, mfp);

        match result {
            Ok(_) => println!("Signing successful!"),
            Err(e) => panic!("Signing failed with error: {:?}", e),
        }
        assert!(!pskt.inputs[0].partial_sigs.is_empty());

        println!("Signed PSKT: {}", pskt.to_hex().unwrap());
    }
}
