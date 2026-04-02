use crate::errors::{BitcoinError, Result};
use alloc::collections::{BTreeMap, BTreeSet};
use alloc::format;

use bitcoin::script::Instruction;
use itertools::Itertools;

use crate::addresses::address::Address;
use crate::network::{self, CustomNewNetwork};
use crate::transactions::parsed_tx::{ParseContext, ParsedInput, ParsedOutput, TxParser};
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::ops::Index;
use core::str::{Chars, FromStr};
use keystore::algorithms::secp256k1::derive_public_key;

use crate::multi_sig::address::calculate_multi_address;
use crate::multi_sig::wallet::calculate_multi_sig_verify_code;
use crate::multi_sig::MultiSigFormat;
use bitcoin::bip32::{ChildNumber, DerivationPath, Fingerprint, KeySource, Xpub};
use bitcoin::psbt::{GetKey, KeyRequest, Psbt};
use bitcoin::psbt::{Input, Output};
use bitcoin::secp256k1::{Secp256k1, Signing, XOnlyPublicKey};
use bitcoin::taproot::TapLeafHash;
use bitcoin::{secp256k1, NetworkKind};
use bitcoin::{Network, PrivateKey};
use bitcoin::{PublicKey, ScriptBuf, TxOut};

pub struct WrappedPsbt {
    pub(crate) psbt: Psbt,
}

//TODO: use it later
#[allow(unused)]
pub enum SignStatus {
    Completed,
    PartlySigned,
    IdenticalPartlySigned(u32, u32),
    Unsigned,
    Invalid,
}

struct Keystore {
    mfp: Fingerprint,
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
            KeyRequest::Bip32((fingerprint, path)) => {
                if self.mfp == fingerprint {
                    let key = keystore::algorithms::secp256k1::get_private_key_by_seed(
                        &self.seed,
                        &path.to_string(),
                    )
                    .map_err(|e| BitcoinError::GetKeyError(e.to_string()))?;
                    Ok(Some(PrivateKey::new(key, Network::Bitcoin)))
                } else {
                    Err(BitcoinError::GetKeyError("mfp is not match".to_string()))
                }
            }
            _ => Err(BitcoinError::GetKeyError(
                "get private key by public key is not supported".to_string(),
            )),
        }
    }
}

impl WrappedPsbt {
    pub fn sign(&mut self, seed: &[u8], mfp: Fingerprint) -> Result<Psbt> {
        let k = Keystore {
            mfp,
            seed: seed.to_vec(),
        };
        self.psbt
            .sign(&k, &secp256k1::Secp256k1::new())
            .map_err(|_| BitcoinError::SignFailure("unknown error".to_string()))?;
        Ok(self.psbt.clone())
    }

    pub fn parse_input(
        &self,
        input: &Input,
        index: usize,
        context: &ParseContext,
        network: &network::Network,
    ) -> Result<ParsedInput> {
        let address = if self.is_taproot_input(input) {
            self.calculate_address_for_taproot_input(input, network)?
        } else {
            self.calculate_address_for_input(input, network)?
        };
        let unsigned_tx = &self.psbt.unsigned_tx;
        let mut value = 0u64;
        if let Some(prev_tx) = &input.non_witness_utxo {
            let tx_in = unsigned_tx
                .input
                .get(index)
                .ok_or(BitcoinError::InvalidInput)?;
            if !tx_in.previous_output.txid.eq(&prev_tx.compute_txid()) {
                return Err(BitcoinError::InvalidInput);
            }
            let prevout = prev_tx.output.get(tx_in.previous_output.vout as usize);
            if let Some(out) = prevout {
                value = out.value.to_sat();
            }
        }
        if let Some(utxo) = &input.witness_utxo {
            value = utxo.value.to_sat();
        }
        let path = self.get_my_input_path(input, index, context)?;
        let sign_status = self.get_input_sign_status(input);
        let is_multisig = sign_status.1 > 1;
        let mut need_sign = false;
        if path.is_some() {
            need_sign = self.need_sign_input(input, context)?;
        }
        Ok(ParsedInput {
            address,
            amount: Self::format_amount(value, network),
            value,
            path: path.clone().map(|v| v.0),
            sign_status,
            is_multisig,
            is_external: path.is_some_and(|(_, external)| external),
            need_sign,
            ecdsa_sighash_type: input
                .sighash_type
                .map(|v| v.ecdsa_hash_ty().unwrap_or(bitcoin::EcdsaSighashType::All))
                .unwrap_or(bitcoin::EcdsaSighashType::All)
                .to_u32() as u8,
        })
    }

    pub fn check_inputs(&self, context: &ParseContext) -> Result<()> {
        if self.psbt.inputs.is_empty() {
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
        self.check_my_input_script(input, index)?;
        self.check_my_input_signature(input, index, context)?;
        self.check_my_input_value_tampered(input, index)?;
        self.check_my_wallet_type(input, context)?;
        Ok(true)
    }

    fn get_my_input_verify_code(&self, input: &Input) -> Option<String> {
        if input.bip32_derivation.len() > 1 {
            self.get_multi_sig_input_verify_code(input).ok()
        } else {
            None
        }
    }

    fn check_my_wallet_type(&self, input: &Input, context: &ParseContext) -> Result<()> {
        let input_verify_code = self.get_my_input_verify_code(input);
        match &context.verify_code {
            //single sig
            None => {
                if input_verify_code.is_none() {
                    return Ok(());
                }
            }
            //multi sig
            Some(verify_code) => {
                if let Some(inner_verify_code) = &input_verify_code {
                    if verify_code == inner_verify_code {
                        return Ok(());
                    }
                }
            }
        };

        let input_verify_code = match input_verify_code {
            None => "null".to_string(),
            Some(verify_code) => verify_code,
        };

        let wallet_verify_code = match &context.verify_code {
            None => "null".to_string(),
            Some(verify_code) => verify_code.to_string(),
        };
        Err(BitcoinError::WalletTypeError(format!(
            "wallet type mismatch wallet verify code is {wallet_verify_code} input verify code is {input_verify_code}"
        )))
    }

    pub fn check_my_input_script(&self, _input: &Input, _index: usize) -> Result<()> {
        Ok(())
    }

    pub fn check_my_input_derivation(&self, _input: &Input, _index: usize) -> Result<()> {
        Ok(())
    }

    pub fn check_my_input_signature(
        &self,
        input: &Input,
        index: usize,
        context: &ParseContext,
    ) -> Result<()> {
        if input.bip32_derivation.len() > 1 {
            // we consider this situation to be OK currently.

            // for key in input.bip32_derivation.keys() {
            //     let (fingerprint, _) = input
            //         .bip32_derivation
            //         .get(key)
            //         .ok_or(BitcoinError::InvalidInput)?;

            //     if fingerprint.eq(&context.master_fingerprint) {
            //         if input.partial_sigs.contains_key(&PublicKey::new(*key)) {
            //             return Err(BitcoinError::InvalidTransaction(format!(
            //                 "input #{} has already been signed",
            //                 index
            //             )));
            //         }
            //     }
            // }
            return Ok(());
        }
        if self.is_taproot_input(input) {
            if let Some((x_only_pubkey, leasfhashes)) =
                self.get_my_key_and_leafhashes_pair_for_taproot(input, context)
            {
                for leasfhash in leasfhashes.iter() {
                    if input
                        .tap_script_sigs
                        .contains_key(&(x_only_pubkey, *leasfhash))
                    {
                        return Err(BitcoinError::InvalidTransaction(format!(
                            "input #{index} has already been signed"
                        )));
                    }
                }
            }
            if input.tap_key_sig.is_some() {
                return Err(BitcoinError::InvalidTransaction(format!(
                    "input #{index} has already been signed"
                )));
            }
        } else if !input.partial_sigs.is_empty() {
            return Err(BitcoinError::InvalidTransaction(format!(
                "input #{index} has already been signed"
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
            if !prev_tx_out_value.eq(utxo) {
                return Err(BitcoinError::InputValueTampered(format!(
                    "input #{index}'s value does not match the value in previous transaction"
                )));
            }
        }
        Ok(())
    }

    pub fn check_outputs(&self, context: &ParseContext) -> Result<()> {
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

    pub fn calculate_address_for_taproot_input(
        &self,
        input: &Input,
        network: &network::Network,
    ) -> Result<Option<String>> {
        if let Some(internal_key) = input.tap_internal_key {
            return Ok(Some(
                Address::p2tr(&internal_key, input.tap_merkle_root, network.clone())?.to_string(),
            ));
        }
        Ok(None)
    }

    fn get_multi_sig_script_and_format<'a>(
        &'a self,
        input: &'a Input,
    ) -> Result<(&'a ScriptBuf, MultiSigFormat)> {
        match (&input.redeem_script, &input.witness_script) {
            (Some(script), None) => Ok((script, MultiSigFormat::P2sh)),
            (Some(_), Some(script)) => Ok((script, MultiSigFormat::P2wshP2sh)),
            (None, Some(script)) => Ok((script, MultiSigFormat::P2wsh)),
            (_, _) => Err(BitcoinError::MultiSigWalletAddressCalError(
                "invalid multi sig input".to_string(),
            )),
        }
    }

    // fn get_input_sign_status_text(&self, input: &Input) -> String {
    //     //this might be a multisig input;
    //     if input.bip32_derivation.len() > 1 {
    //         let result = self.get_multi_sig_script_and_format(input);
    //         if let Ok((script, _)) = result {
    //             if script.is_multisig() {
    //                 let mut instructions = script.instructions();
    //                 if let Some(Ok(Instruction::Op(op))) = instructions.next() {
    //                     //OP_PUSHNUM_1 to OP_PUSHNUM_16
    //                     if op.to_u8() >= 0x51 && op.to_u8() <= 0x60 {
    //                         let required_sigs = op.to_u8() - 0x50;
    //                         return Some(format!(
    //                             "{}/{} Signed",
    //                             input.partial_sigs.len(),
    //                             required_sigs,
    //                         ));
    //                     }
    //                 }
    //             }
    //         }
    //     }
    //     None
    // }

    //return: (sigs, required_sigs)
    fn get_input_sign_status(&self, input: &Input) -> (u32, u32) {
        //this might be a multisig input;
        if input.bip32_derivation.len() > 1 {
            let result = self.get_multi_sig_script_and_format(input);
            if let Ok((script, _)) = result {
                if script.is_multisig() {
                    let mut instructions = script.instructions();
                    if let Some(Ok(Instruction::Op(op))) = instructions.next() {
                        //OP_PUSHNUM_1 to OP_PUSHNUM_16
                        if op.to_u8() >= 0x51 && op.to_u8() <= 0x60 {
                            let required_sigs = op.to_u8() - 0x50;
                            return (input.partial_sigs.len() as u32, required_sigs as u32);
                        }
                    }
                }
            }
        }
        if self.is_taproot_input(input) {
            return self.get_taproot_sign_status(input);
        }
        //there might be a (x, 0) forms of sign status which we don't care;
        (
            input.partial_sigs.len() as u32,
            input.bip32_derivation.len() as u32,
        )
    }

    // assume this is my input because we checked it before;
    fn need_sign_input(&self, input: &Input, context: &ParseContext) -> Result<bool> {
        if input.bip32_derivation.len() > 1 {
            let (cur, req) = self.get_input_sign_status(input);
            //already collected needed signatures
            if cur >= req {
                return Ok(false);
            }
            // or I have signed this input
            for key in input.bip32_derivation.keys() {
                let (fingerprint, _) = input
                    .bip32_derivation
                    .get(key)
                    .ok_or(BitcoinError::InvalidInput)?;

                if fingerprint.eq(&context.master_fingerprint)
                    && input.partial_sigs.contains_key(&PublicKey::new(*key))
                {
                    return Ok(false);
                }
            }
            Ok(true)
        } else if self.is_taproot_input(input) {
            Ok(!self.has_signed_taproot_input(input, context)?)
        } else {
            Ok(input.partial_sigs.is_empty())
        }
    }

    fn get_multi_sig_input_threshold_and_total(&self, script: &ScriptBuf) -> Result<(u8, u8)> {
        if !script.is_multisig() {
            return Err(BitcoinError::MultiSigInputError(
                "it's not a multi sig script".to_string(),
            ));
        }
        let mut required_sigs = 0;
        let mut total = 0;

        let mut instructions = script.instructions();

        if let Some(Ok(Instruction::Op(op))) = instructions.next() {
            if op.to_u8() >= 0x51 && op.to_u8() <= 0x60 {
                required_sigs = op.to_u8() - 0x50;
            }
        }
        while let Some(Ok(instruction)) = instructions.next() {
            if let Instruction::Op(op) = instruction {
                if op.to_u8() >= 0x51 && op.to_u8() <= 0x60 {
                    total = op.to_u8() - 0x50;
                }
            }
        }
        Ok((required_sigs, total))
    }

    fn get_xpub_from_psbt_by_fingerprint(
        &self,
        fp: &Fingerprint,
    ) -> Result<(&Xpub, &DerivationPath)> {
        for key in self.psbt.xpub.keys() {
            let (fingerprint, path) = self.psbt.xpub.get(key).ok_or(BitcoinError::InvalidInput)?;
            if fp == fingerprint {
                return Ok((key, path));
            }
        }
        Err(BitcoinError::MultiSigInputError(
            "have no match fingerprint in xpub".to_string(),
        ))
    }

    fn get_multi_sig_input_verify_code(&self, input: &Input) -> Result<String> {
        if self.psbt.xpub.is_empty() {
            return Err(BitcoinError::MultiSigInputError(
                "xpub is empty".to_string(),
            ));
        }

        let mut xpubs = vec![];
        for key in input.bip32_derivation.keys() {
            let (fingerprint, path) = input
                .bip32_derivation
                .get(key)
                .ok_or(BitcoinError::InvalidInput)?;
            let (xpub, derivation_path) = self.get_xpub_from_psbt_by_fingerprint(fingerprint)?;
            let public_key = derive_public_key_by_path(xpub, derivation_path, path)?;
            if public_key == *key {
                xpubs.push(xpub.to_string());
            }
        }

        if xpubs.is_empty() || xpubs.len() != input.bip32_derivation.len() {
            return Err(BitcoinError::MultiSigInputError(
                "xpub does not match bip32_derivation".to_string(),
            ));
        }

        let (script, format) = self.get_multi_sig_script_and_format(input)?;
        let (threshold, total) = self.get_multi_sig_input_threshold_and_total(script)?;

        let network = if let Some((xpub, _)) = self.psbt.xpub.first_key_value() {
            match xpub.network {
                NetworkKind::Main => network::Network::Bitcoin,
                _ => network::Network::BitcoinTestnet,
            }
        } else {
            return Err(BitcoinError::MultiSigNetworkError(
                "can not get network type".to_string(),
            ));
        };

        calculate_multi_sig_verify_code(
            &xpubs,
            threshold,
            total,
            format,
            &crate::multi_sig::Network::try_from(&network)?,
            None,
        )
    }

    pub fn get_overall_sign_status(&self) -> Option<String> {
        if self.psbt.inputs.is_empty() {
            return None;
        }
        let (first_sigs, first_requires) = self.get_input_sign_status(&self.psbt.inputs[0]);
        let all_inputs_status: Vec<(u32, u32)> = self
            .psbt
            .inputs
            .iter()
            .map(|v| self.get_input_sign_status(v))
            .collect();
        //none of inputs is signed
        if all_inputs_status.iter().all(|(sigs, _)| sigs.eq(&0)) {
            Some(String::from("Unsigned"))
        }
        //or some inputs are signed and completed
        else if all_inputs_status
            .iter()
            .all(|(sigs, requires)| sigs.ge(requires))
        {
            return Some(String::from("Completed"));
        }
        //or inputs are partially signed and all of them are multisig inputs
        else if all_inputs_status
            .iter()
            .all(|(sigs, requires)| sigs.eq(&first_sigs) && requires.eq(&first_requires))
        {
            return Some(format!("{first_sigs}/{first_requires} Signed"));
        } else {
            return Some(String::from("Partly Signed"));
        }
    }

    pub fn is_sign_completed(&self) -> bool {
        if let Some(res) = self.get_overall_sign_status() {
            return res.eq("Completed");
        }
        false
    }

    fn calculate_address_by_pubkey_and_path(
        &self,
        pubkey: &secp256k1::PublicKey,
        path: &DerivationPath,
        network: &network::Network,
    ) -> Result<Option<String>> {
        match path.index(0) {
            ChildNumber::Hardened { index: _i } => match _i {
                44 => match path.index(1) {
                    ChildNumber::Hardened { index: _i } => match _i {
                        0 | 3 => Ok(Some(
                            Address::p2pkh(&bitcoin::PublicKey::new(*pubkey), network.clone())?
                                .to_string(),
                        )),
                        60 => Ok(Some(
                            Address::p2wpkh(&bitcoin::PublicKey::new(*pubkey), network.clone())?
                                .to_string(),
                        )),
                        _ => Ok(None),
                    },
                    _ => Ok(None),
                },
                49 => Ok(Some(
                    Address::p2shp2wpkh(&bitcoin::PublicKey::new(*pubkey), network.clone())?
                        .to_string(),
                )),
                84 => Ok(Some(
                    Address::p2wpkh(&bitcoin::PublicKey::new(*pubkey), network.clone())?
                        .to_string(),
                )),
                _ => Ok(None),
            },
            _ => Ok(None),
        }
    }

    pub fn calculate_address_for_input(
        &self,
        input: &Input,
        network: &network::Network,
    ) -> Result<Option<String>> {
        if input.bip32_derivation.len() > 1 {
            let (script, foramt) = self.get_multi_sig_script_and_format(input)?;
            return Ok(Some(calculate_multi_address(
                script,
                foramt,
                &crate::multi_sig::Network::try_from(network)?,
            )?));
        }

        match input.bip32_derivation.first_key_value() {
            Some((pubkey, (_, derivation_path))) => {
                self.calculate_address_by_pubkey_and_path(pubkey, derivation_path, network)
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
        let path = self.parse_my_output(&output.bip32_derivation, tx_out, context)?;
        Ok(ParsedOutput {
            address: self.calculate_address_for_output(tx_out, network)?,
            amount: Self::format_amount(tx_out.value.to_sat(), network),
            value: tx_out.value.to_sat(),
            path: path.clone().map(|v| v.0),
            is_mine: path.is_some(),
            is_external: path.clone().is_some_and(|v| v.1),
        })
    }

    pub fn calculate_address_for_output(
        &self,
        tx_out: &TxOut,
        network: &network::Network,
    ) -> Result<String> {
        match Address::from_script(&tx_out.script_pubkey, network.clone()) {
            Ok(address) => Ok(address.to_string()),
            Err(_) => Ok(tx_out.script_pubkey.to_string()),
        }
    }

    pub fn get_my_input_path(
        &self,
        input: &Input,
        index: usize,
        context: &ParseContext,
    ) -> Result<Option<(String, bool)>> {
        if context.multisig_wallet_config.is_some() && self.is_taproot_input(input) {
            return Err(BitcoinError::InvalidPsbt(
                "multisig with taproot is not supported".to_string(),
            ));
        }
        if self.is_taproot_input(input) {
            self.get_my_key_path_for_taproot(&input.tap_key_origins, index, "input", context)
        } else {
            self.get_my_key_path(&input.bip32_derivation, index, "input", context)
        }
    }

    pub fn get_my_output_path(
        &self,
        output: &Output,
        index: usize,
        context: &ParseContext,
    ) -> Result<Option<(String, bool)>> {
        let path = self.get_my_key_path(&output.bip32_derivation, index, "output", context)?;
        if path.is_some() {
            return Ok(path);
        }
        self.get_my_key_path_for_taproot(&output.tap_key_origins, index, "output", context)
    }

    // copy and modify from get_my_key_path
    // TODO: refactor multisig and singlesig to different workflows
    pub fn parse_my_output(
        &self,
        bip32_derivation: &BTreeMap<secp256k1::PublicKey, KeySource>,
        tx_out: &TxOut,
        context: &ParseContext,
    ) -> Result<Option<(String, bool)>> {
        if let Some(config) = &context.multisig_wallet_config {
            let total = config.total;
            // not my key
            if bip32_derivation.keys().len() as u32 != total {
                return Ok(None);
            }

            let wallet_xfps = config
                .xpub_items
                .iter()
                .map(|v| v.xfp.clone())
                .sorted()
                .fold("".to_string(), |acc, cur| format!("{acc}{cur}"));
            let xfps = bip32_derivation
                .values()
                .map(|(fp, _)| fp.to_string())
                .sorted()
                .fold("".to_string(), |acc, cur| format!("{acc}{cur}"));
            // not my multisig key
            if !wallet_xfps.eq_ignore_ascii_case(&xfps) {
                return Ok(None);
            }

            let mut child_path = None;
            let mut matched_parent_path = None;
            for key in bip32_derivation.keys() {
                let (fingerprint, path) = bip32_derivation.get(key).unwrap();
                for (i, item) in config.xpub_items.iter().enumerate() {
                    if item.xfp.eq_ignore_ascii_case(&fingerprint.to_string()) {
                        if let Some(parent_path) = config.get_derivation_by_index(i) {
                            let path_str = path.to_string();
                            if path_str.starts_with(&parent_path) {
                                child_path = Some(path_str);
                                matched_parent_path = Some(parent_path);
                                break;
                            }
                        }
                    }
                }
                if child_path.is_some() {
                    break;
                }
            }

            if let Some(child) = child_path {
                let parent = matched_parent_path.unwrap();
                let suffix = child[parent.len()..].to_string();
                let mut pubkeys = Vec::new();
                for item in config.xpub_items.iter() {
                    let derived_pk = keystore::algorithms::secp256k1::derive_public_key(
                        &item.xpub,
                        &format!("m{suffix}"),
                    )
                    .map_err(|e| BitcoinError::DerivePublicKeyError(e.to_string()))?;
                    pubkeys.push(bitcoin::PublicKey::new(derived_pk));
                }
                pubkeys.sort();
                let pubkeys_len = pubkeys.len();
                let mut builder = bitcoin::script::Builder::new().push_int(config.threshold as i64);
                for key in pubkeys {
                    builder = builder.push_key(&key);
                }
                let p2ms = builder
                    .push_int(pubkeys_len as i64)
                    .push_opcode(bitcoin::opcodes::all::OP_CHECKMULTISIG)
                    .into_script();

                let format = MultiSigFormat::from(&config.format)?;
                let expected_script = match format {
                    MultiSigFormat::P2sh => ScriptBuf::new_p2sh(&p2ms.script_hash()),
                    MultiSigFormat::P2wshP2sh => {
                        let p2wsh = ScriptBuf::new_p2wsh(&p2ms.wscript_hash());
                        ScriptBuf::new_p2sh(&p2wsh.script_hash())
                    }
                    MultiSigFormat::P2wsh => ScriptBuf::new_p2wsh(&p2ms.wscript_hash()),
                };

                if expected_script != tx_out.script_pubkey {
                    return Ok(None);
                } else {
                    return Ok(Some((
                        child.to_uppercase(),
                        Self::judge_external_key(child, parent),
                    )));
                }
            }
        }

        for key in bip32_derivation.keys() {
            let (fingerprint, path) = bip32_derivation
                .get(key)
                .ok_or(BitcoinError::InvalidInput)?;
            if fingerprint.eq(&context.master_fingerprint) {
                let child = path.to_string();

                for (_i, (parent_path, xpub)) in context.extended_public_keys.iter().enumerate() {
                    if child.starts_with(&parent_path.to_string()) {
                        let _child_path = match DerivationPath::from_str(&child) {
                            Ok(p) => p,
                            Err(_) => {
                                return Err(BitcoinError::InvalidTransaction(format!(
                                    "invalid output, cannot parse child derivation path"
                                )));
                            }
                        };
                        let public_key =
                            match derive_public_key_by_path(xpub, parent_path, &_child_path) {
                                Ok(pk) => pk,
                                Err(_) => {
                                    return Err(BitcoinError::InvalidTransaction(format!(
                                        "invalid output, cannot derive associated public key"
                                    )));
                                }
                            };

                        let expected_script = match parent_path.into_iter().next() {
                            Some(ChildNumber::Hardened { index: 44 }) => ScriptBuf::new_p2pkh(
                                &bitcoin::PublicKey::new(public_key).pubkey_hash(),
                            ),
                            Some(ChildNumber::Hardened { index: 49 }) => {
                                let p2wpkh = ScriptBuf::new_p2wpkh(
                                    &bitcoin::PublicKey::new(public_key).wpubkey_hash().map_err(
                                        |e| BitcoinError::DerivePublicKeyError(e.to_string()),
                                    )?,
                                );
                                ScriptBuf::new_p2sh(&p2wpkh.script_hash())
                            }
                            Some(ChildNumber::Hardened { index: 84 }) => ScriptBuf::new_p2wpkh(
                                &bitcoin::PublicKey::new(public_key).wpubkey_hash().map_err(
                                    |e| BitcoinError::DerivePublicKeyError(e.to_string()),
                                )?,
                            ),
                            _ => return Ok(None),
                        };

                        if expected_script != tx_out.script_pubkey {
                            return Ok(None);
                        }

                        return Ok(Some((
                            child.to_uppercase(),
                            Self::judge_external_key(child, parent_path.to_string()),
                        )));
                    }
                }
                return Err(BitcoinError::InvalidTransaction(format!(
                    "invalid output, fingerprint matched but cannot derive associated public key"
                )));
            }
        }
        Ok(None)
    }

    pub fn get_my_key_path(
        &self,
        bip32_derivation: &BTreeMap<secp256k1::PublicKey, KeySource>,
        index: usize,
        purpose: &str,
        context: &ParseContext,
    ) -> Result<Option<(String, bool)>> {
        if let Some(config) = &context.multisig_wallet_config {
            let total = config.total;
            // not my key
            if bip32_derivation.keys().len() as u32 != total {
                return Ok(None);
            }

            let wallet_xfps = config
                .xpub_items
                .iter()
                .map(|v| v.xfp.clone())
                .sorted()
                .fold("".to_string(), |acc, cur| format!("{acc}{cur}"));
            let xfps = bip32_derivation
                .values()
                .map(|(fp, _)| fp.to_string())
                .sorted()
                .fold("".to_string(), |acc, cur| format!("{acc}{cur}"));
            // not my multisig key
            if !wallet_xfps.eq_ignore_ascii_case(&xfps) {
                return Ok(None);
            }
        }
        //it's a singlesig or it's my multisig input
        for key in bip32_derivation.keys() {
            let (fingerprint, path) = bip32_derivation
                .get(key)
                .ok_or(BitcoinError::InvalidInput)?;
            if fingerprint.eq(&context.master_fingerprint) {
                let child = path.to_string();
                match &context.multisig_wallet_config {
                    Some(config) => {
                        for (i, xpub_item) in config.xpub_items.iter().enumerate() {
                            if xpub_item.xfp.eq_ignore_ascii_case(&fingerprint.to_string()) {
                                if let Some(parent_path) = config.get_derivation_by_index(i) {
                                    if child.starts_with(&parent_path) {
                                        return Ok(Some((
                                            child.to_uppercase(),
                                            Self::judge_external_key(child, parent_path.clone()),
                                        )));
                                    }
                                }
                            }
                        }
                        return Err(BitcoinError::InvalidTransaction(format!(
                            "invalid {purpose} #{index}, fingerprint matched but cannot derive associated public key"
                        )));
                    }
                    None => {
                        for (_i, (parent_path, xpub)) in
                            context.extended_public_keys.iter().enumerate()
                        {
                            if child.starts_with(&parent_path.to_string()) {
                                let _child_path = match DerivationPath::from_str(&child) {
                                    Ok(p) => p,
                                    Err(_) => {
                                        return Err(BitcoinError::InvalidTransaction(format!(
                                            "invalid {purpose} #{index}, cannot parse child derivation path"
                                        )));
                                    }
                                };
                                return Ok(Some((
                                    child.to_uppercase(),
                                    Self::judge_external_key(child, parent_path.to_string()),
                                )));
                            }
                        }
                        return Err(BitcoinError::InvalidTransaction(format!(
                            "invalid {purpose} #{index}, fingerprint matched but cannot derive associated public key"
                        )));
                    }
                }
            }
        }
        Ok(None)
    }

    fn judge_external_key(child: String, parent: String) -> bool {
        let sub_path = &child[parent.len()..];
        fn judge(v: &mut Chars) -> bool {
            match v.next() {
                Some('/') => judge(v),
                Some('0') => true,
                _ => false,
            }
        }
        judge(&mut sub_path.chars())
    }

    pub fn get_my_key_path_for_taproot(
        &self,
        tap_key_origins: &BTreeMap<XOnlyPublicKey, (Vec<TapLeafHash>, KeySource)>,
        index: usize,
        purpose: &str,
        context: &ParseContext,
    ) -> Result<Option<(String, bool)>> {
        for key in tap_key_origins.keys() {
            let (_, (fingerprint, path)) =
                tap_key_origins.get(key).ok_or(BitcoinError::InvalidInput)?;
            if fingerprint.eq(&context.master_fingerprint) {
                let child = path.to_string();
                for parent_path in context.extended_public_keys.keys() {
                    if child.starts_with(&parent_path.to_string()) {
                        return Ok(Some((
                            child.to_uppercase(),
                            Self::judge_external_key(child, parent_path.to_string()),
                        )));
                    }
                }
                return Err(BitcoinError::InvalidTransaction(format!(
                    "invalid {purpose} #{index}, fingerprint matched but cannot derive associated public key"
                )));
            }
        }
        Ok(None)
    }

    pub fn get_my_key_and_leafhashes_pair_for_taproot(
        &self,
        input: &Input,
        context: &ParseContext,
    ) -> Option<(XOnlyPublicKey, Vec<TapLeafHash>)> {
        for (pk, (leaf_hashes, (fingerprint, _))) in input.tap_key_origins.iter() {
            if *fingerprint == context.master_fingerprint && !leaf_hashes.is_empty() {
                return Some((*pk, leaf_hashes.clone()));
            }
        }
        None
    }

    fn has_signed_taproot_input(&self, input: &Input, context: &ParseContext) -> Result<bool> {
        for (pk, (leaf_hashes, (fingerprint, _))) in input.tap_key_origins.iter() {
            if *fingerprint != context.master_fingerprint {
                continue;
            }
            if leaf_hashes.is_empty() {
                if input.tap_key_sig.is_some() {
                    return Ok(true);
                }
            } else {
                for leaf_hash in leaf_hashes {
                    if input.tap_script_sigs.contains_key(&(*pk, *leaf_hash)) {
                        return Ok(true);
                    }
                }
            }
        }
        Ok(false)
    }

    fn get_taproot_sign_status(&self, input: &Input) -> (u32, u32) {
        let mut signed = 0u32;
        // for key path spend, it requires only 1 signature.
        let mut required = 1u32;

        if input.tap_key_sig.is_some() {
            signed += 1;
        }

        if !input.tap_script_sigs.is_empty() {
            // every key generates a signature for each leaf script, see https://github.com/rust-bitcoin/rust-bitcoin/blob/master/bitcoin/src/psbt/mod.rs#L461
            // but the actual requirements depends on the script content.
            // we don't parse the taproot script content for now, so we use a max required value here.
            let max_required = input
                .tap_key_origins
                .iter()
                .fold(0, |acc, (_, (leaf_hashes, _))| {
                    acc + leaf_hashes.len() as u32
                });
            let mut unique_signers: BTreeSet<[u8; 32]> = BTreeSet::new();
            for &(pk, _) in input.tap_script_sigs.keys() {
                unique_signers.insert(pk.serialize());
            }
            signed += unique_signers.len() as u32;
            // for script path spend, we don't know the required number, so we set it to 0xff.
            required = max_required;
        }
        (signed, required)
    }

    fn is_taproot_input(&self, input: &Input) -> bool {
        if let Some(witness_utxo) = &input.witness_utxo {
            return witness_utxo.script_pubkey.is_p2tr();
        }
        false
    }

    // use global unknown for some custom usage
    // current use it for identify whether it is the fractal bitcoin tx
    pub fn identify_fractal_bitcoin_tx(&self) -> Option<CustomNewNetwork> {
        self.psbt.unknown.iter().find_map(|(item_key, item_value)| {
            (String::from_utf8(item_key.key.clone()).ok()? == "chain")
                .then(|| String::from_utf8(item_value.clone()).ok())
                .flatten()
                .and_then(|value| match value.as_str() {
                    "fb" => Some(CustomNewNetwork::FractalBitcoin),
                    "tfb" => Some(CustomNewNetwork::FractalBitcoinTest),
                    _ => None,
                })
        })
    }
}

fn derive_public_key_by_path(
    xpub: &Xpub,
    root_path: &DerivationPath,
    hd_path: &DerivationPath,
) -> Result<secp256k1::PublicKey> {
    let root_path = if !root_path.to_string().ends_with('/') {
        root_path.to_string() + "/"
    } else {
        root_path.to_string()
    };
    let sub_path = hd_path
        .to_string()
        .strip_prefix(&root_path)
        .ok_or(BitcoinError::InvalidPsbt(hd_path.to_string()))?
        .to_string();

    let public_key = derive_public_key(&xpub.to_string(), &format!("m/{sub_path}"))
        .map_err(|e| BitcoinError::DerivePublicKeyError(e.to_string()))?;

    Ok(public_key)
}

#[cfg(test)]
mod tests {
    use alloc::string::String;
    use alloc::vec::Vec;
    use core::str::FromStr;

    use crate::TxChecker;
    use bitcoin::absolute::LockTime;
    use bitcoin::ecdsa::Signature;
    use bitcoin::psbt::Input;
    use bitcoin::secp256k1::{Secp256k1, SecretKey};
    use bitcoin::transaction::Version;
    use bitcoin::{ScriptBuf, Transaction};
    use either::Left;
    use hex::{self, FromHex, ToHex};

    use super::*;

    fn empty_psbt() -> Psbt {
        let tx = Transaction {
            version: Version::TWO,
            lock_time: LockTime::ZERO,
            input: Vec::new(),
            output: Vec::new(),
        };
        Psbt::from_unsigned_tx(tx).unwrap()
    }

    fn dummy_pubkey() -> secp256k1::PublicKey {
        let sk =
            SecretKey::from_str("0000000000000000000000000000000000000000000000000000000000000001")
                .unwrap();
        secp256k1::PublicKey::from_secret_key(&Secp256k1::new(), &sk)
    }

    #[test]
    fn test_new_sign() {
        {
            let psbt_hex = "70736274ff01005202000000016d41e6873468f85aff76d7709a93b47180ea0784edaac748228d2c474396ca550000000000fdffffff01a00f0000000000001600146623828c1f87be7841a9b1cc360d38ae0a8b6ed0000000000001011f6817000000000000160014d0c4a3ef09e997b6e99e397e518fe3e41a118ca1220602e7ab2537b5d49e970309aae06e9e49f36ce1c9febbd44ec8e0d1cca0b4f9c3191873c5da0a54000080010000800000008000000000000000000000";
            let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
            let mut wrapper = WrappedPsbt { psbt };
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let master_fingerprint = bitcoin::bip32::Fingerprint::from_str("73c5da0a").unwrap();
            let result = wrapper.sign(&seed, master_fingerprint).unwrap();
            let psbt_result = result.serialize().encode_hex::<String>();
            assert_eq!("70736274ff01005202000000016d41e6873468f85aff76d7709a93b47180ea0784edaac748228d2c474396ca550000000000fdffffff01a00f0000000000001600146623828c1f87be7841a9b1cc360d38ae0a8b6ed0000000000001011f6817000000000000160014d0c4a3ef09e997b6e99e397e518fe3e41a118ca1220202e7ab2537b5d49e970309aae06e9e49f36ce1c9febbd44ec8e0d1cca0b4f9c319483045022100e2b9a7963bed429203bbd73e5ea000bfe58e3fc46ef8c1939e8cf8d1cf8460810220587ba791fc2a42445db70e2b3373493a19e6d5c47a2af0447d811ff479721b0001220602e7ab2537b5d49e970309aae06e9e49f36ce1c9febbd44ec8e0d1cca0b4f9c3191873c5da0a54000080010000800000008000000000000000000000", psbt_result);
        }
    }

    #[test]
    fn test_identify_fractal_bitcoin_tx() {
        {
            // no value in global
            let psbt_hex = "70736274ff01005202000000016d41e6873468f85aff76d7709a93b47180ea0784edaac748228d2c474396ca550000000000fdffffff01a00f0000000000001600146623828c1f87be7841a9b1cc360d38ae0a8b6ed0000000000001011f6817000000000000160014d0c4a3ef09e997b6e99e397e518fe3e41a118ca1220602e7ab2537b5d49e970309aae06e9e49f36ce1c9febbd44ec8e0d1cca0b4f9c3191873c5da0a54000080010000800000008000000000000000000000";
            let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
            let wrapper = WrappedPsbt { psbt };
            let result = wrapper.identify_fractal_bitcoin_tx();
            assert_eq!(result.is_none(), true);

            // globalkey: ff chain fb (utf-8)
            let psbt_hex = "70736274ff01005202000000016d41e6873468f85aff76d7709a93b47180ea0784edaac748228d2c474396ca550000000000fdffffff01a00f0000000000001600146623828c1f87be7841a9b1cc360d38ae0a8b6ed00000000006ff636861696e0266620001011f6817000000000000160014d0c4a3ef09e997b6e99e397e518fe3e41a118ca1220602e7ab2537b5d49e970309aae06e9e49f36ce1c9febbd44ec8e0d1cca0b4f9c3191873c5da0a54000080010000800000008000000000000000000000";
            let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
            let wrapper = WrappedPsbt { psbt };
            let result = wrapper.identify_fractal_bitcoin_tx().unwrap();
            assert_eq!(result, CustomNewNetwork::FractalBitcoin);

            // globalkey: ff chain tfb (utf-8)
            let psbt_hex = "70736274ff01005202000000016d41e6873468f85aff76d7709a93b47180ea0784edaac748228d2c474396ca550000000000fdffffff01a00f0000000000001600146623828c1f87be7841a9b1cc360d38ae0a8b6ed00000000006ff636861696e037466620001011f6817000000000000160014d0c4a3ef09e997b6e99e397e518fe3e41a118ca1220602e7ab2537b5d49e970309aae06e9e49f36ce1c9febbd44ec8e0d1cca0b4f9c3191873c5da0a54000080010000800000008000000000000000000000";
            let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
            let wrapper = WrappedPsbt { psbt };
            let result = wrapper.identify_fractal_bitcoin_tx().unwrap();
            assert_eq!(result, CustomNewNetwork::FractalBitcoinTest)
        }
    }

    #[test]
    fn test_taproot_sign() {
        {
            let psbt_hex = "70736274ff01005e02000000013aee4d6b51da574900e56d173041115bd1e1d01d4697a845784cf716a10c98060000000000ffffffff0100190000000000002251202258f2d4637b2ca3fd27614868b33dee1a242b42582d5474f51730005fa99ce8000000000001012bbc1900000000000022512022f3956cc27a6a9b0e0003a0afc113b04f31b95d5cad222a65476e8440371bd1010304000000002116b68df382cad577d8304d5a8e640c3cb42d77c10016ab754caa4d6e68b6cb296d190073c5da0a5600008001000080000000800000000002000000011720b68df382cad577d8304d5a8e640c3cb42d77c10016ab754caa4d6e68b6cb296d011820c913dc9a8009a074e7bbc493b9d8b7e741ba137f725f99d44fbce99300b2bb0a0000";
            let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
            let mut wrapper = WrappedPsbt { psbt };
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let master_fingerprint = bitcoin::bip32::Fingerprint::from_str("73c5da0a").unwrap();
            let result = wrapper.sign(&seed, master_fingerprint).unwrap();
            let psbt_result = result.serialize().encode_hex::<String>();
            assert_eq!("70736274ff01005e02000000013aee4d6b51da574900e56d173041115bd1e1d01d4697a845784cf716a10c98060000000000ffffffff0100190000000000002251202258f2d4637b2ca3fd27614868b33dee1a242b42582d5474f51730005fa99ce8000000000001012bbc1900000000000022512022f3956cc27a6a9b0e0003a0afc113b04f31b95d5cad222a65476e8440371bd10103040000000001134092864dc9e56b6260ecbd54ec16b94bb597a2e6be7cca0de89d75e17921e0e1528cba32dd04217175c237e1835b5db1c8b384401718514f9443dce933c6ba9c872116b68df382cad577d8304d5a8e640c3cb42d77c10016ab754caa4d6e68b6cb296d190073c5da0a5600008001000080000000800000000002000000011720b68df382cad577d8304d5a8e640c3cb42d77c10016ab754caa4d6e68b6cb296d011820c913dc9a8009a074e7bbc493b9d8b7e741ba137f725f99d44fbce99300b2bb0a0000", psbt_result);
        }
    }

    #[test]
    fn test_check_psbt() {
        // single sig wallet and single sig input
        {
            let psbt_hex = "70736274ff01005e02000000013aee4d6b51da574900e56d173041115bd1e1d01d4697a845784cf716a10c98060000000000ffffffff0100190000000000002251202258f2d4637b2ca3fd27614868b33dee1a242b42582d5474f51730005fa99ce8000000000001012bbc1900000000000022512022f3956cc27a6a9b0e0003a0afc113b04f31b95d5cad222a65476e8440371bd1010304000000002116b68df382cad577d8304d5a8e640c3cb42d77c10016ab754caa4d6e68b6cb296d190073c5da0a5600008001000080000000800000000002000000011720b68df382cad577d8304d5a8e640c3cb42d77c10016ab754caa4d6e68b6cb296d011820c913dc9a8009a074e7bbc493b9d8b7e741ba137f725f99d44fbce99300b2bb0a0000";
            let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
            let wpsbt = WrappedPsbt { psbt };
            let master_fingerprint = Fingerprint::from_str("73c5da0a").unwrap();
            let extended_pubkey = Xpub::from_str("tpubDDfvzhdVV4unsoKt5aE6dcsNsfeWbTgmLZPi8LQDYU2xixrYemMfWJ3BaVneH3u7DBQePdTwhpybaKRU95pi6PMUtLPBJLVQRpzEnjfjZzX").unwrap();
            let path = DerivationPath::from_str("m/86'/1'/0'").unwrap();
            let mut keys = BTreeMap::new();
            keys.insert(path, extended_pubkey);

            let reust = wpsbt.check(Left(&ParseContext {
                master_fingerprint: master_fingerprint.clone(),
                extended_public_keys: keys.clone(),
                verify_code: None,
                multisig_wallet_config: None,
            }));
            assert_eq!(Ok(()), reust);
        }

        // single sig wallet and multi sig input
        {
            let psbt_hex= "70736274ff01005e0200000001d8d89245a905abe9e2ab7bb834ebbc50a75947c82f96eeec7b38e0b399a62c490000000000fdffffff0158070000000000002200202b9710701f5c944606bb6bab82d2ef969677d8b9d04174f59e2a631812ae739bf76c27004f01043587cf0473f7e9418000000147f2d1b4bef083e346eb1949bcd8e2b59f95d8391a9eb4e1ea9005df926585480365fd7b1eca553df2c4e17bc5b88384ceda3d0d98fa3145cff5e61e471671a0b214c45358fa300000800100008000000080010000804f01043587cf04bac1483980000001a73adbe2878487634dcbfc3f7ebde8b1fc994f1ec06860cf01c3fe2ea791ddb602e62a2a9973ee6b3a7af47c229a5bde70bca59bd04bbb297f5693d7aa256b976d1473c5da0a3000008001000080000000800100008000010120110800000000000017a914980ec372495334ee232575505208c0b2e142dbb5872202032ed737f53936afb128247fc71a0b0b5be4d9348e9a48bfda9ef31efe3e45fa2e47304402203109d97095c61395881d6f75093943b16a91e1a4fff73bf193fcfe6e7689a35c02203bd187fed5bba45ee2c322911b8abb07f1d091520f5598259047d0dee058a75e01010304010000000104220020ffac81e598dd9856d08bd6c55b712fd23ea8522bd075fcf48ed467ced2ee015601054752210267ea4562439356307e786faf40503730d8d95a203a0e345cb355a5dfa03fce0321032ed737f53936afb128247fc71a0b0b5be4d9348e9a48bfda9ef31efe3e45fa2e52ae2206032ed737f53936afb128247fc71a0b0b5be4d9348e9a48bfda9ef31efe3e45fa2e1cc45358fa30000080010000800000008001000080000000000000000022060267ea4562439356307e786faf40503730d8d95a203a0e345cb355a5dfa03fce031c73c5da0a3000008001000080000000800100008000000000000000000000";
            let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
            let wpsbt = WrappedPsbt { psbt };

            let master_fingerprint = Fingerprint::from_str("73c5da0a").unwrap();
            let extended_pubkey = Xpub::from_str("tpubDFH9dgzveyD8yHQb8VrpG8FYAuwcLMHMje2CCcbBo1FpaGzYVtJeYYxcYgRqSTta5utUFts8nPPHs9C2bqoxrey5jia6Dwf9mpwrPq7YvcJ").unwrap();
            let path = DerivationPath::from_str("m/48'/1'/0'/1'").unwrap();
            let mut keys = BTreeMap::new();
            keys.insert(path, extended_pubkey);

            let reust = wpsbt.check(Left(&ParseContext {
                master_fingerprint: master_fingerprint.clone(),
                extended_public_keys: keys.clone(),
                verify_code: None,
                multisig_wallet_config: None,
            }));
            assert_eq!(true, reust.is_err());
        }

        // multi sig wallet and single sig input
        {
            let psbt_hex = "70736274ff01005e02000000013aee4d6b51da574900e56d173041115bd1e1d01d4697a845784cf716a10c98060000000000ffffffff0100190000000000002251202258f2d4637b2ca3fd27614868b33dee1a242b42582d5474f51730005fa99ce8000000000001012bbc1900000000000022512022f3956cc27a6a9b0e0003a0afc113b04f31b95d5cad222a65476e8440371bd1010304000000002116b68df382cad577d8304d5a8e640c3cb42d77c10016ab754caa4d6e68b6cb296d190073c5da0a5600008001000080000000800000000002000000011720b68df382cad577d8304d5a8e640c3cb42d77c10016ab754caa4d6e68b6cb296d011820c913dc9a8009a074e7bbc493b9d8b7e741ba137f725f99d44fbce99300b2bb0a0000";
            let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
            let wpsbt = WrappedPsbt { psbt };

            let master_fingerprint = Fingerprint::from_str("73c5da0a").unwrap();
            let extended_pubkey = Xpub::from_str("tpubDFH9dgzveyD8yHQb8VrpG8FYAuwcLMHMje2CCcbBo1FpaGzYVtJeYYxcYgRqSTta5utUFts8nPPHs9C2bqoxrey5jia6Dwf9mpwrPq7YvcJ").unwrap();
            let path = DerivationPath::from_str("m/48'/1'/0'/1'").unwrap();
            let mut keys = BTreeMap::new();
            keys.insert(path, extended_pubkey);

            let reust = wpsbt.check(Left(&ParseContext {
                master_fingerprint: master_fingerprint.clone(),
                extended_public_keys: keys.clone(),
                verify_code: Some("03669e02".to_string()),
                multisig_wallet_config: None,
            }));
            assert_eq!(true, reust.is_err());
        }

        // multi sig wallet and multi sig input, verify code is equal
        {
            let psbt_hex= "70736274ff01005e0200000001d8d89245a905abe9e2ab7bb834ebbc50a75947c82f96eeec7b38e0b399a62c490000000000fdffffff0158070000000000002200202b9710701f5c944606bb6bab82d2ef969677d8b9d04174f59e2a631812ae739bf76c27004f01043587cf0473f7e9418000000147f2d1b4bef083e346eb1949bcd8e2b59f95d8391a9eb4e1ea9005df926585480365fd7b1eca553df2c4e17bc5b88384ceda3d0d98fa3145cff5e61e471671a0b214c45358fa300000800100008000000080010000804f01043587cf04bac1483980000001a73adbe2878487634dcbfc3f7ebde8b1fc994f1ec06860cf01c3fe2ea791ddb602e62a2a9973ee6b3a7af47c229a5bde70bca59bd04bbb297f5693d7aa256b976d1473c5da0a3000008001000080000000800100008000010120110800000000000017a914980ec372495334ee232575505208c0b2e142dbb5872202032ed737f53936afb128247fc71a0b0b5be4d9348e9a48bfda9ef31efe3e45fa2e47304402203109d97095c61395881d6f75093943b16a91e1a4fff73bf193fcfe6e7689a35c02203bd187fed5bba45ee2c322911b8abb07f1d091520f5598259047d0dee058a75e01010304010000000104220020ffac81e598dd9856d08bd6c55b712fd23ea8522bd075fcf48ed467ced2ee015601054752210267ea4562439356307e786faf40503730d8d95a203a0e345cb355a5dfa03fce0321032ed737f53936afb128247fc71a0b0b5be4d9348e9a48bfda9ef31efe3e45fa2e52ae2206032ed737f53936afb128247fc71a0b0b5be4d9348e9a48bfda9ef31efe3e45fa2e1cc45358fa30000080010000800000008001000080000000000000000022060267ea4562439356307e786faf40503730d8d95a203a0e345cb355a5dfa03fce031c73c5da0a3000008001000080000000800100008000000000000000000000";
            let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
            let wpsbt = WrappedPsbt { psbt };

            let master_fingerprint = Fingerprint::from_str("73c5da0a").unwrap();
            let extended_pubkey = Xpub::from_str("tpubDFH9dgzveyD8yHQb8VrpG8FYAuwcLMHMje2CCcbBo1FpaGzYVtJeYYxcYgRqSTta5utUFts8nPPHs9C2bqoxrey5jia6Dwf9mpwrPq7YvcJ").unwrap();
            let path = DerivationPath::from_str("m/48'/1'/0'/1'").unwrap();
            let mut keys = BTreeMap::new();
            keys.insert(path, extended_pubkey);

            let reust = wpsbt.check(Left(&ParseContext {
                master_fingerprint: master_fingerprint.clone(),
                extended_public_keys: keys.clone(),
                verify_code: Some("03669e02".to_string()),
                multisig_wallet_config: None,
            }));
            assert_eq!(Ok(()), reust);
        }

        // multi sig wallet and multi sig input, verify code is not equal
        {
            let psbt_hex= "70736274ff01005e0200000001d8d89245a905abe9e2ab7bb834ebbc50a75947c82f96eeec7b38e0b399a62c490000000000fdffffff0158070000000000002200202b9710701f5c944606bb6bab82d2ef969677d8b9d04174f59e2a631812ae739bf76c27004f01043587cf0473f7e9418000000147f2d1b4bef083e346eb1949bcd8e2b59f95d8391a9eb4e1ea9005df926585480365fd7b1eca553df2c4e17bc5b88384ceda3d0d98fa3145cff5e61e471671a0b214c45358fa300000800100008000000080010000804f01043587cf04bac1483980000001a73adbe2878487634dcbfc3f7ebde8b1fc994f1ec06860cf01c3fe2ea791ddb602e62a2a9973ee6b3a7af47c229a5bde70bca59bd04bbb297f5693d7aa256b976d1473c5da0a3000008001000080000000800100008000010120110800000000000017a914980ec372495334ee232575505208c0b2e142dbb5872202032ed737f53936afb128247fc71a0b0b5be4d9348e9a48bfda9ef31efe3e45fa2e47304402203109d97095c61395881d6f75093943b16a91e1a4fff73bf193fcfe6e7689a35c02203bd187fed5bba45ee2c322911b8abb07f1d091520f5598259047d0dee058a75e01010304010000000104220020ffac81e598dd9856d08bd6c55b712fd23ea8522bd075fcf48ed467ced2ee015601054752210267ea4562439356307e786faf40503730d8d95a203a0e345cb355a5dfa03fce0321032ed737f53936afb128247fc71a0b0b5be4d9348e9a48bfda9ef31efe3e45fa2e52ae2206032ed737f53936afb128247fc71a0b0b5be4d9348e9a48bfda9ef31efe3e45fa2e1cc45358fa30000080010000800000008001000080000000000000000022060267ea4562439356307e786faf40503730d8d95a203a0e345cb355a5dfa03fce031c73c5da0a3000008001000080000000800100008000000000000000000000";
            let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
            let wpsbt = WrappedPsbt { psbt };

            let master_fingerprint = Fingerprint::from_str("73c5da0a").unwrap();
            let extended_pubkey = Xpub::from_str("tpubDFH9dgzveyD8yHQb8VrpG8FYAuwcLMHMje2CCcbBo1FpaGzYVtJeYYxcYgRqSTta5utUFts8nPPHs9C2bqoxrey5jia6Dwf9mpwrPq7YvcJ").unwrap();
            let path = DerivationPath::from_str("m/48'/1'/0'/1'").unwrap();
            let mut keys = BTreeMap::new();
            keys.insert(path, extended_pubkey);

            let reust = wpsbt.check(Left(&ParseContext {
                master_fingerprint: master_fingerprint.clone(),
                extended_public_keys: keys.clone(),
                verify_code: Some("12345678".to_string()),
                multisig_wallet_config: None,
            }));
            assert!(reust.is_err());
        }
    }

    #[test]
    fn test_get_multi_sig_input_threshold_and_total() {
        let mut pk1 = vec![0x03];
        pk1.extend([0x11; 32]);
        let mut pk2 = vec![0x03];
        pk2.extend([0x22; 32]);
        let mut pk3 = vec![0x03];
        pk3.extend([0x33; 32]);
        let pk1 = bitcoin::script::PushBytesBuf::try_from(pk1).unwrap();
        let pk2 = bitcoin::script::PushBytesBuf::try_from(pk2).unwrap();
        let pk3 = bitcoin::script::PushBytesBuf::try_from(pk3).unwrap();
        let script = ScriptBuf::builder()
            .push_opcode(bitcoin::opcodes::all::OP_PUSHNUM_2)
            .push_slice(pk1.clone())
            .push_slice(pk2.clone())
            .push_slice(pk3.clone())
            .push_opcode(bitcoin::opcodes::all::OP_PUSHNUM_3)
            .push_opcode(bitcoin::opcodes::all::OP_CHECKMULTISIG)
            .into_script();
        let wrapper = WrappedPsbt { psbt: empty_psbt() };
        let (threshold, total) = wrapper
            .get_multi_sig_input_threshold_and_total(&script)
            .unwrap();
        assert_eq!(threshold, 2);
        assert_eq!(total, 3);

        let invalid_script = ScriptBuf::builder()
            .push_opcode(bitcoin::opcodes::all::OP_RETURN)
            .into_script();
        let err = wrapper
            .get_multi_sig_input_threshold_and_total(&invalid_script)
            .unwrap_err();
        assert!(matches!(
            err,
            BitcoinError::MultiSigInputError(message) if message == "it's not a multi sig script"
        ));
    }

    #[test]
    fn test_judge_external_key() {
        assert!(WrappedPsbt::judge_external_key(
            "M/84'/0'/0'/0/15".to_string(),
            "M/84'/0'/0'/".to_string()
        ));
        assert!(!WrappedPsbt::judge_external_key(
            "M/84'/0'/0'/1/3".to_string(),
            "M/84'/0'/0'/".to_string()
        ));
    }

    #[test]
    fn test_get_overall_sign_status_variants() {
        let wrapper_empty = WrappedPsbt { psbt: empty_psbt() };
        assert!(wrapper_empty.get_overall_sign_status().is_none());

        let mut psbt_unsigned = empty_psbt();
        let mut unsigned_input = Input::default();
        unsigned_input.bip32_derivation.insert(
            dummy_pubkey(),
            (
                Fingerprint::from_str("73c5da0a").unwrap(),
                DerivationPath::from_str("m/84'/0'/0'/0/0").unwrap(),
            ),
        );
        psbt_unsigned.inputs.push(unsigned_input);
        let wrapper_unsigned = WrappedPsbt {
            psbt: psbt_unsigned,
        };
        assert_eq!(
            wrapper_unsigned.get_overall_sign_status(),
            Some("Unsigned".to_string())
        );

        let mut psbt_signed = empty_psbt();
        let mut signed_input = Input::default();
        let key = dummy_pubkey();
        signed_input.bip32_derivation.insert(
            key,
            (
                Fingerprint::from_str("73c5da0a").unwrap(),
                DerivationPath::from_str("m/84'/0'/0'/0/0").unwrap(),
            ),
        );
        let signature = Signature::from_str("3045022100e2b9a7963bed429203bbd73e5ea000bfe58e3fc46ef8c1939e8cf8d1cf8460810220587ba791fc2a42445db70e2b3373493a19e6d5c47a2af0447d811ff479721b0001").unwrap();
        signed_input
            .partial_sigs
            .insert(bitcoin::PublicKey::new(key), signature);
        psbt_signed.inputs.push(signed_input);
        let wrapper_signed = WrappedPsbt { psbt: psbt_signed };
        assert_eq!(
            wrapper_signed.get_overall_sign_status(),
            Some("Completed".to_string())
        );
    }
}
