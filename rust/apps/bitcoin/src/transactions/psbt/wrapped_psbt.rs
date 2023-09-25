use crate::errors::{BitcoinError, Result};
use alloc::format;

use crate::addresses::address::Address;
use crate::network;
use crate::transactions::parsed_tx::{ParseContext, ParsedInput, ParsedOutput, TxParser};
use alloc::collections::BTreeMap;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::ops::Index;

use third_party::bitcoin::bip32::{ChildNumber, KeySource};
use third_party::bitcoin::psbt::{GetKey, KeyRequest, Psbt};
use third_party::bitcoin::psbt::{Input, Output};
use third_party::bitcoin::TxOut;
use third_party::bitcoin::{Network, PrivateKey};
use third_party::secp256k1::{Secp256k1, Signing};
use third_party::{bitcoin, secp256k1};

pub struct WrappedPsbt {
    pub(crate) psbt: Psbt,
}

struct Keystore {
    seed: Vec<u8>,
}

impl GetKey for Keystore {
    type Error = BitcoinError;

    fn get_key<C: Signing>(
        &self,
        key_request: KeyRequest,
        _: &Secp256k1<C>,
    ) -> core::result::Result<Option<PrivateKey>, Self::Error> {
        match key_request {
            KeyRequest::Bip32((_, path)) => {
                let key = keystore::algorithms::secp256k1::get_private_key_by_seed(
                    &self.seed,
                    &path.to_string(),
                )
                .map_err(|e| BitcoinError::GetKeyError(e.to_string()))?;
                Ok(Some(PrivateKey::new(key, Network::Bitcoin)))
            }
            _ => Err(BitcoinError::GetKeyError(format!(
                "get private key by public key is not supported"
            ))),
        }
    }
}

impl WrappedPsbt {
    pub fn sign(&mut self, seed: &[u8]) -> Result<Psbt> {
        let k = Keystore {
            seed: seed.to_vec(),
        };
        self.psbt
            .sign(&k, &third_party::secp256k1::Secp256k1::new())
            .map_err(|_| BitcoinError::SignFailure(format!("unknown error")))?;
        Ok(self.psbt.clone())
    }

    pub fn parse_input(
        &self,
        input: &Input,
        index: usize,
        context: &ParseContext,
        network: &network::Network,
    ) -> Result<ParsedInput> {
        let address = self.calculate_address_for_input(input, network)?;
        let unsigned_tx = &self.psbt.unsigned_tx;
        let mut value = 0u64;
        if let Some(prev_tx) = &input.non_witness_utxo {
            let tx_in = unsigned_tx
                .input
                .get(index)
                .ok_or(BitcoinError::InvalidInput)?;
            if !tx_in.previous_output.txid.eq(&prev_tx.txid()) {
                return Err(BitcoinError::InvalidInput);
            }
            let prevout = prev_tx.output.get(tx_in.previous_output.vout as usize);
            match prevout {
                Some(out) => {
                    value = out.value;
                }
                None => {}
            }
        }
        if let Some(utxo) = &input.witness_utxo {
            value = utxo.value;
        }
        if value <= 0 {
            return Err(BitcoinError::InvalidTransaction(format!(
                "input #{} has negative value",
                index
            )));
        }
        let path = self.get_my_input_path(input, index, context)?;
        Ok(ParsedInput {
            address,
            amount: Self::format_amount(value, network),
            value,
            path,
        })
    }

    pub fn check_inputs(&self, context: &ParseContext) -> Result<()> {
        if self.psbt.inputs.len() == 0 {
            return Err(BitcoinError::NoInputs);
        }
        let has_my_input = self
            .psbt
            .inputs
            .iter()
            .enumerate()
            .map(|(i, inp)| self.judge_my_input_then_check(inp, i, context))
            .fold(Ok(false), |acc, cur| match (acc, cur) {
                (Ok(b1), Ok(b2)) => Ok(b1 | b2),
                (a, b) => a.and(b),
            })?;
        if !has_my_input {
            return Err(BitcoinError::NoMyInputs);
        }
        Ok(())
    }

    pub fn judge_my_input_then_check(
        &self,
        input: &Input,
        index: usize,
        context: &ParseContext,
    ) -> Result<bool> {
        let path = self.get_my_input_path(input, index, context)?;
        if path.is_none() {
            // not my input
            return Ok(false);
        }
        self.check_my_input_derivation(input, index)?;
        self.check_my_input_signature(input, index)?;
        self.check_my_input_value_tampered(input, index)?;
        Ok(true)
    }

    pub fn check_my_input_derivation(&self, input: &Input, index: usize) -> Result<()> {
        if input.bip32_derivation.len() > 1 {
            return Err(BitcoinError::UnsupportedTransaction(format!(
                "multisig input #{} is not supported yet",
                index
            )));
        }
        Ok(())
    }

    pub fn check_my_input_signature(&self, input: &Input, index: usize) -> Result<()> {
        if input.bip32_derivation.len() > 1 {
            // TODO check multisig signatures
            return Ok(());
        }
        if input.partial_sigs.len() > 0 {
            return Err(BitcoinError::InvalidTransaction(format!(
                "input #{} has already been signed",
                index
            )));
        }
        Ok(())
    }

    // https://blog.trezor.io/details-of-firmware-updates-for-trezor-one-version-1-9-1-and-trezor-model-t-version-2-3-1-1eba8f60f2dd
    pub fn check_my_input_value_tampered(&self, input: &Input, index: usize) -> Result<()> {
        if let (Some(prev_tx), Some(utxo)) = (&input.non_witness_utxo, &input.witness_utxo) {
            let tx = &self.psbt.unsigned_tx;
            let this_tx_in = tx.input.get(index).ok_or(BitcoinError::InvalidInput)?;
            let prev_tx_out_value = prev_tx
                .output
                .get(this_tx_in.previous_output.vout as usize)
                .ok_or(BitcoinError::InvalidInput)?;
            if !prev_tx_out_value.eq(&utxo) {
                return Err(BitcoinError::InputValueTampered(format!(
                    "input #{}'s value does not match the value in previous transaction",
                    index
                )));
            }
        }
        Ok(())
    }

    pub fn check_outputs(&self, context: &ParseContext) -> Result<()> {
        if self.psbt.outputs.len() == 0 {
            return Err(BitcoinError::NoOutputs);
        }
        self.psbt
            .outputs
            .iter()
            .enumerate()
            .map(|(index, output)| self.judge_then_check_my_output(output, index, context))
            .fold(Ok(()), |acc, cur| acc.and(cur))
    }

    pub fn judge_then_check_my_output(
        &self,
        output: &Output,
        index: usize,
        context: &ParseContext,
    ) -> Result<()> {
        let _ = self.get_my_output_path(output, index, context)?;
        Ok(())
    }

    pub fn calculate_address_for_input(
        &self,
        input: &Input,
        network: &network::Network,
    ) -> Result<Option<String>> {
        // TODO: multisig support
        match input.bip32_derivation.first_key_value() {
            Some((pubkey, (_, derivation_path))) => {
                //call generate address here
                match derivation_path.index(0) {
                    ChildNumber::Hardened { index: _i } => match _i {
                        44 => Ok(Some(
                            Address::p2pkh(
                                &bitcoin::PublicKey::new(pubkey.clone()),
                                network.clone(),
                            )?
                            .to_string(),
                        )),
                        49 => Ok(Some(
                            Address::p2shp2wpkh(
                                &bitcoin::PublicKey::new(pubkey.clone()),
                                network.clone(),
                            )?
                            .to_string(),
                        )),
                        84 => Ok(Some(
                            Address::p2wpkh(
                                &bitcoin::PublicKey::new(pubkey.clone()),
                                network.clone(),
                            )?
                            .to_string(),
                        )),
                        _ => Ok(None),
                    },
                    _ => Ok(None),
                }
            }
            _ => Ok(None),
        }
    }

    pub fn parse_output(
        &self,
        output: &Output,
        index: usize,
        context: &ParseContext,
        network: &network::Network,
    ) -> Result<ParsedOutput> {
        let unsigned_tx = &self.psbt.unsigned_tx;
        let tx_out = unsigned_tx
            .output
            .get(index)
            .ok_or(BitcoinError::InvalidOutput)?;
        let path = self.get_my_output_path(output, index, context)?;
        Ok(ParsedOutput {
            address: self.calculate_address_for_output(tx_out, network)?,
            amount: Self::format_amount(tx_out.value, network),
            value: tx_out.value,
            path,
        })
    }

    pub fn calculate_address_for_output(
        &self,
        tx_out: &TxOut,
        network: &network::Network,
    ) -> Result<String> {
        let address = Address::from_script(&tx_out.script_pubkey, network.clone())
            .map_err(|_v| BitcoinError::InvalidOutput)?;
        Ok(address.to_string())
    }

    pub fn get_my_input_path(
        &self,
        input: &Input,
        index: usize,
        context: &ParseContext,
    ) -> Result<Option<String>> {
        self.get_my_key_path(&input.bip32_derivation, index, "input", context)
    }

    pub fn get_my_output_path(
        &self,
        output: &Output,
        index: usize,
        context: &ParseContext,
    ) -> Result<Option<String>> {
        self.get_my_key_path(&output.bip32_derivation, index, "output", context)
    }

    pub fn get_my_key_path(
        &self,
        bip32_derivation: &BTreeMap<secp256k1::PublicKey, KeySource>,
        index: usize,
        purpose: &str,
        context: &ParseContext,
    ) -> Result<Option<String>> {
        for key in bip32_derivation.keys() {
            let (fingerprint, path) = bip32_derivation
                .get(key)
                .ok_or(BitcoinError::InvalidInput)?;
            if fingerprint.eq(&context.master_fingerprint) {
                let child = path.to_string();
                for parent_path in context.extended_public_keys.keys() {
                    if child.starts_with(&parent_path.to_string()) {
                        return Ok(Some(child.to_uppercase()));
                        // let extended_key = context.extended_public_keys.get(parent_path).unwrap();
                        // let xpub = bip32::XPub::from_str(extended_key.to_string().as_str())
                        //     .map_err(|e| BitcoinError::GetKeyError(e.to_string()))?;
                        // let sub = child
                        //     .strip_prefix(&parent_path.to_string())
                        //     .map(|v| {
                        //         v.split('/').fold("m".to_string(), |acc, cur| {
                        //             match cur.is_empty() {
                        //                 true => acc,
                        //                 false => acc + "/" + cur,
                        //             }
                        //         })
                        //     })
                        //     .unwrap();
                        // let sub_path = bip32::DerivationPath::from_str(sub.as_str())
                        //     .map_err(|_e| BitcoinError::InvalidInput)?;
                        // let child_key =
                        //     sub_path
                        //         .into_iter()
                        //         .fold(Ok(xpub), |acc: Result<XPub>, cur| {
                        //             acc.and_then(|v| {
                        //                 v.derive_child(cur)
                        //                     .map_err(|e| BitcoinError::GetKeyError(e.to_string()))
                        //             })
                        //         })?;
                        // let lk = child_key.public_key().to_bytes();
                        // let rk = key.serialize();
                        // if lk.eq(&rk) {
                        //     return Ok(Some(child.to_uppercase()));
                        // }
                    }
                }
                return Err(BitcoinError::InvalidTransaction(format!(
                    "invalid {} #{}, fingerprint matched but cannot derive associated public key",
                    purpose, index
                )));
            }
        }
        Ok(None)
    }
}

#[cfg(test)]
mod tests {
    use alloc::string::String;
    use alloc::vec::Vec;

    use third_party::bitcoin::hashes::hex::FromHex;
    use third_party::hex::{self, ToHex};

    use super::*;

    #[test]
    fn test_new_sign() {
        {
            let psbt_hex = "70736274ff01005202000000016d41e6873468f85aff76d7709a93b47180ea0784edaac748228d2c474396ca550000000000fdffffff01a00f0000000000001600146623828c1f87be7841a9b1cc360d38ae0a8b6ed0000000000001011f6817000000000000160014d0c4a3ef09e997b6e99e397e518fe3e41a118ca1220602e7ab2537b5d49e970309aae06e9e49f36ce1c9febbd44ec8e0d1cca0b4f9c3191873c5da0a54000080010000800000008000000000000000000000";
            let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
            let mut wrapper = WrappedPsbt { psbt };
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let result = wrapper.sign(&seed).unwrap();
            let psbt_result = result.serialize().encode_hex::<String>();
            assert_eq!("70736274ff01005202000000016d41e6873468f85aff76d7709a93b47180ea0784edaac748228d2c474396ca550000000000fdffffff01a00f0000000000001600146623828c1f87be7841a9b1cc360d38ae0a8b6ed0000000000001011f6817000000000000160014d0c4a3ef09e997b6e99e397e518fe3e41a118ca1220202e7ab2537b5d49e970309aae06e9e49f36ce1c9febbd44ec8e0d1cca0b4f9c319483045022100e2b9a7963bed429203bbd73e5ea000bfe58e3fc46ef8c1939e8cf8d1cf8460810220587ba791fc2a42445db70e2b3373493a19e6d5c47a2af0447d811ff479721b0001220602e7ab2537b5d49e970309aae06e9e49f36ce1c9febbd44ec8e0d1cca0b4f9c3191873c5da0a54000080010000800000008000000000000000000000", psbt_result);
        }
    }
}
