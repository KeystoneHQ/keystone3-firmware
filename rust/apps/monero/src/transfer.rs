use crate::address::*;
use crate::errors::{MoneroError, Result};
use crate::key::*;
use crate::key_images::{try_to_generate_image, Keyimage};
use crate::outputs::ExportedTransferDetails;
use crate::signed_transaction::{PendingTx, SignedTxSet};
use crate::structs::*;
use crate::utils::*;
use crate::utils::{constants::*, hash::*, io::*};
use alloc::borrow::ToOwned;
use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;
use core::fmt::Display;
use curve25519_dalek::scalar::Scalar;
use curve25519_dalek::EdwardsPoint;
use monero_serai::primitives::Commitment;
use monero_serai::primitives::Decoys;
use monero_serai::ringct::bulletproofs::Bulletproof;
use monero_serai::ringct::clsag::{Clsag, ClsagContext};
use monero_serai::ringct::{RctBase, RctProofs, RctPrunable};
use monero_serai::transaction::{
    Input, Output, Timelock, Transaction, TransactionPrefix,
};
use rand_core::OsRng;
use zeroize::Zeroizing;

#[derive(Debug, Clone)]
pub struct DisplayTransactionInfo {
    pub outputs: Vec<(Address, String, bool)>,
    pub inputs: Vec<(PublicKey, String)>,
    pub input_amount: String,
    pub output_amount: String,
    pub fee: String,
}

impl Display for DisplayTransactionInfo {
    fn fmt(&self, f: &mut core::fmt::Formatter) -> core::fmt::Result {
        writeln!(f, "Inputs:")?;
        for (public_key, amount) in self.inputs.iter() {
            writeln!(f, "  {:?} - {}", hex::encode(public_key.as_bytes()), amount)?;
        }
        writeln!(f, "Outputs:")?;
        for (address, amount, _) in self.outputs.iter() {
            writeln!(f, "  {:?} - {}", address.to_string(), amount)?;
        }
        writeln!(f, "Input amount: {}", self.input_amount)?;
        writeln!(f, "Output amount: {}", self.output_amount)?;
        writeln!(f, "Fee: {}", self.fee)
    }
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum RctType {
    RCTTypeNull = 0,
    RCTTypeFull = 1,
    RCTTypeSimple = 2,
    RCTTypeBulletproof = 3,
    RCTTypeBulletproof2 = 4,
    RCTTypeCLSAG = 5,
    RCTTypeBulletproofPlus = 6,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum RangeProofType {
    Null = 0,
    Full = 1,
    Simple = 2,
    Bulletproof = 3,
    Bulletproof2 = 4,
    CLSAG = 5,
    BulletproofPlus = 6,
}

#[derive(Debug, Clone, Copy)]
pub struct RCTConfig {
    pub version: u32,
    pub range_proof_type: RangeProofType,
    pub bp_version: RctType,
}

#[derive(Debug, Clone, Copy)]
pub struct CtKey {
    pub dest: [u8; 32],
    pub mask: [u8; 32],
}

#[derive(Debug, Clone, Copy)]
pub struct OutputEntry {
    pub index: u64,
    pub key: CtKey,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct AccountPublicAddress {
    pub spend_public_key: [u8; 32],
    pub view_public_key: [u8; 32],
}

impl AccountPublicAddress {
    pub fn to_address(&self, network: Network, is_subaddress: bool) -> Address {
        Address {
            network,
            addr_type: if is_subaddress {
                AddressType::Subaddress
            } else {
                AddressType::Standard
            },
            public_spend: PublicKey::from_bytes(&self.spend_public_key).unwrap(),
            public_view: PublicKey::from_bytes(&self.view_public_key).unwrap(),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TxDestinationEntry {
    pub original: String,
    pub amount: u64,
    pub addr: AccountPublicAddress,
    pub is_subaddress: bool,
    pub is_integrated: bool,
}

#[derive(Debug, Clone, Copy)]
#[allow(non_camel_case_types)]
#[allow(non_snake_case)]

pub struct Multisig_kLRki {
    pub k: [u8; 32],
    pub L: [u8; 32],
    pub R: [u8; 32],
    pub ki: [u8; 32],
}

#[derive(Debug, Clone)]
#[allow(non_snake_case)]
pub struct TxSourceEntry {
    pub outputs: Vec<OutputEntry>,
    pub real_output: u64,
    pub real_out_tx_key: [u8; 32],
    pub real_out_additional_tx_keys: Vec<[u8; 32]>,
    pub real_output_in_tx_index: u64,
    pub amount: u64,
    pub rct: bool,
    pub mask: [u8; 32],
    pub multisig_kLRki: Multisig_kLRki,
}

#[derive(Debug, Clone)]
pub struct TxConstructionData {
    pub sources: Vec<TxSourceEntry>,
    pub change_dts: TxDestinationEntry,
    pub splitted_dsts: Vec<TxDestinationEntry>,
    pub selected_transfers: Vec<usize>,
    pub extra: Vec<u8>,
    pub unlock_time: u64,
    pub use_rct: u8,
    pub rct_config: RCTConfig,
    pub dests: Vec<TxDestinationEntry>,
    pub subaddr_account: u32,
    pub subaddr_indices: Vec<u32>,
}

pub struct InnerInput {
    pub source: TxSourceEntry,
    pub key_offsets: Vec<u64>,
    pub input: Input,
    pub key_offset: Scalar,
}

pub struct InnerInputs(Vec<InnerInput>);

impl Default for InnerInputs {
    fn default() -> Self {
        Self::new()
    }
}

impl InnerInputs {
    pub fn new() -> InnerInputs {
        InnerInputs(vec![])
    }

    pub fn push(&mut self, input: InnerInput) {
        self.0.push(input);
    }

    pub fn get_inputs(&self) -> Vec<Input> {
        self.0
            .iter()
            .map(|inner_input| inner_input.input.clone())
            .collect()
    }

    pub fn get_key_offsets(&self, index: usize) -> Vec<u64> {
        self.0[index].key_offsets.clone()
    }

    pub fn get_sources(&self) -> Vec<TxSourceEntry> {
        self.0
            .iter()
            .map(|inner_input| inner_input.source.clone())
            .collect()
    }
}

pub struct InnerOutput {
    output: Output,
    key_image: Keyimage,
}

pub struct InnerOutputs(Vec<InnerOutput>);

impl Default for InnerOutputs {
    fn default() -> Self {
        Self::new()
    }
}

impl InnerOutputs {
    pub fn new() -> InnerOutputs {
        InnerOutputs(vec![])
    }

    pub fn push(&mut self, output: InnerOutput) {
        self.0.push(output);
    }

    pub fn get_outputs(&self) -> Vec<Output> {
        self.0
            .iter()
            .map(|inner_output| inner_output.output.clone())
            .collect()
    }

    pub fn get_key_images(&self) -> Vec<Keyimage> {
        self.0
            .iter()
            .map(|inner_output| inner_output.key_image)
            .collect()
    }
}

impl TxConstructionData {
    pub fn outputs_amount(&self) -> u64 {
        self.sources.iter().map(|source| source.amount).sum()
    }

    pub fn inputs_amount(&self) -> u64 {
        self.splitted_dsts.iter().map(|dest| dest.amount).sum()
    }

    pub fn fee(&self) -> u64 {
        self.outputs_amount() - self.inputs_amount()
    }

    fn absolute_output_offsets_to_relative(&self, off: Vec<u64>) -> Vec<u64> {
        let mut res = off;
        if res.is_empty() {
            return res;
        }
        res.sort();
        for i in (1..res.len()).rev() {
            res[i] -= res[i - 1];
        }
        res
    }

    pub fn inputs(&self, keypair: &KeyPair) -> Result<InnerInputs> {
        let mut res = InnerInputs::new();
        for (index, source) in self.sources.iter().enumerate() {
            let key_offsets = self.absolute_output_offsets_to_relative(
                source.outputs.iter().map(|output| output.index).collect(),
            );
            let (key_image, key_offset) = self.calc_key_image_by_index(keypair, index)?;
            let input = Input::ToKey {
                amount: None,
                key_offsets: key_offsets.clone(),
                key_image: key_image.to_point(),
            };
            res.push(InnerInput {
                key_offsets,
                input,
                source: source.clone(),
                key_offset,
            });
        }

        let key_image_sort = |x: &EdwardsPoint, y: &EdwardsPoint| -> core::cmp::Ordering {
            x.compress()
                .to_bytes()
                .cmp(&y.compress().to_bytes())
                .reverse()
        };
        res.0.sort_by(|a, b| {
            key_image_sort(
                &get_key_image_from_input(a.input.clone())
                    .unwrap()
                    .to_point(),
                &get_key_image_from_input(b.input.clone())
                    .unwrap()
                    .to_point(),
            )
        });

        Ok(res)
    }

    pub fn derive_view_tag(&self, derivation: &EdwardsPoint, output_index: u64) -> u8 {
        let mut buffer = vec![];
        buffer.extend_from_slice(b"view_tag");
        buffer.extend_from_slice(&derivation.compress().to_bytes());
        buffer.extend_from_slice(&write_varinteger(output_index));

        keccak256(&buffer)[0]
    }

    pub fn outputs(&self, keypair: &KeyPair, tx_key: &PrivateKey, additional_keys: &Vec<PrivateKey>, tx_key_pub: &EdwardsPoint) -> InnerOutputs {
        let shared_key_derivations = self.shared_key_derivations(keypair, tx_key, additional_keys, tx_key_pub);
        let mut res = InnerOutputs::new();
        for (dest, shared_key_derivation) in self.splitted_dsts.iter().zip(shared_key_derivations) {
            let image = generate_key_image_from_priavte_key(&PrivateKey::new(
                shared_key_derivation.shared_key,
            ));

            let key = PublicKey::from_bytes(&dest.addr.spend_public_key)
                .unwrap()
                .point
                .decompress()
                .unwrap()
                + EdwardsPoint::mul_base(&shared_key_derivation.shared_key);

            res.push(InnerOutput {
                output: Output {
                    key: key.compress(),
                    amount: None,
                    view_tag: (match self.rct_config.bp_version {
                        RctType::RCTTypeFull => false,
                        RctType::RCTTypeNull | RctType::RCTTypeBulletproof2 => true,
                        _ => panic!("unsupported RctType"),
                    })
                    .then_some(shared_key_derivation.view_tag),
                },
                key_image: Keyimage::new(image.compress().to_bytes()),
            })
        }

        res
    }

    fn calc_key_image_by_index(
        &self,
        keypair: &KeyPair,
        sources_index: usize,
    ) -> Result<(Keyimage, Scalar)> {
        let source = self.sources[sources_index].clone();
        let output_entry = source.outputs[source.real_output as usize];
        let ctkey = output_entry.key;

        let additional_tx_keys: Vec<PublicKey> = source
            .real_out_additional_tx_keys
            .iter()
            .map(|key_bytes| PublicKey::from_bytes(key_bytes).unwrap())
            .collect();

        match try_to_generate_image(
            keypair,
            &source.real_out_tx_key,
            &ctkey.dest,
            additional_tx_keys,
            source.real_output_in_tx_index,
            self.subaddr_account,
            self.subaddr_indices.clone(),
        ) {
            Ok((key_image, key_offset)) => Ok((key_image, key_offset)),
            Err(e) => Err(e),
        }
    }

    fn calc_key_images(&self, keypair: &KeyPair) -> Result<Vec<Keyimage>> {
        let mut key_images = vec![];
        let tx = self;

        for index in 0..tx.sources.iter().len() {
            key_images.push(tx.calc_key_image_by_index(keypair, index)?.0);
        }

        Ok(key_images)
    }

    pub fn serialize(&self) -> Vec<u8> {
        let mut res = vec![];
        res.push(self.sources.len() as u8);

        res
    }
}

#[derive(Debug, Clone, Default)]
pub struct UnsignedTx {
    pub txes: Vec<TxConstructionData>,
    transfers: ExportedTransferDetails,
}

impl UnsignedTx {
    pub fn display_info(&self) -> DisplayTransactionInfo {
        let mut inputs = vec![];
        let mut outputs = vec![];
        for tx in self.txes.iter() {
            for source in tx.sources.iter() {
                let output = source.outputs[source.real_output as usize];
                let amount = source.amount;
                inputs.push((
                    PublicKey::from_bytes(&output.key.dest).unwrap(),
                    fmt_monero_amount(amount),
                ));
            }
            for dest in tx.splitted_dsts.iter() {
                let address = dest.addr.to_address(Network::Mainnet, dest.is_subaddress);
                let amount = dest.amount;
                let is_change = address
                    == tx
                        .change_dts
                        .addr
                        .to_address(Network::Mainnet, tx.change_dts.is_subaddress);
                outputs.push((address, fmt_monero_amount(amount), is_change));
            }
        }

        DisplayTransactionInfo {
            inputs,
            outputs,
            input_amount: fmt_monero_amount(self.txes.iter().map(|tx| tx.inputs_amount()).sum()),
            output_amount: fmt_monero_amount(self.txes.iter().map(|tx| tx.outputs_amount()).sum()),
            fee: fmt_monero_amount(self.txes.iter().map(|tx| tx.fee()).sum()),
        }
    }

    pub fn deserialize(bytes: &[u8]) -> UnsignedTx {
        let mut offset = 0;
        read_varinteger(bytes, &mut offset); // version: should be 0x02
        let txes_len = read_varinteger(bytes, &mut offset);
        let mut txes = vec![];
        for _ in 0..txes_len {
            let sources_len = read_varinteger(bytes, &mut offset);
            let mut sources = vec![];
            for _ in 0..sources_len {
                let outputs_len = read_varinteger(bytes, &mut offset);
                let mut outputs = vec![];
                for _ in 0..outputs_len {
                    read_varinteger(bytes, &mut offset); // should be 0x02
                    let index = read_varinteger(bytes, &mut offset);
                    let dest = read_next_u8_32(bytes, &mut offset);
                    let mask = read_next_u8_32(bytes, &mut offset);
                    outputs.push(OutputEntry {
                        index,
                        key: CtKey { dest, mask },
                    });
                }
                let real_output = read_next_u64(bytes, &mut offset);
                let real_out_tx_key = read_next_u8_32(bytes, &mut offset);
                let real_out_additional_tx_keys_len = read_varinteger(bytes, &mut offset);
                let mut real_out_additional_tx_keys = vec![];
                for _ in 0..real_out_additional_tx_keys_len {
                    let key = read_next_u8_32(bytes, &mut offset);
                    real_out_additional_tx_keys.push(key);
                }
                let real_output_in_tx_index = read_next_u64(bytes, &mut offset);
                let amount = read_next_u64(bytes, &mut offset);
                let rct = read_next_bool(bytes, &mut offset);
                let mask = read_next_u8_32(bytes, &mut offset);
                #[allow(non_snake_case)]
                let multisig_kLRki = Multisig_kLRki {
                    k: read_next_u8_32(bytes, &mut offset),
                    L: read_next_u8_32(bytes, &mut offset),
                    R: read_next_u8_32(bytes, &mut offset),
                    ki: read_next_u8_32(bytes, &mut offset),
                };
                sources.push(TxSourceEntry {
                    outputs,
                    real_output,
                    real_out_tx_key,
                    real_out_additional_tx_keys,
                    real_output_in_tx_index,
                    amount,
                    rct,
                    mask,
                    multisig_kLRki,
                });
            }
            let change_dts = read_next_tx_destination_entry(bytes, &mut offset);
            let splitted_dsts_len = read_varinteger(bytes, &mut offset);
            let mut splitted_dsts = vec![];
            for _ in 0..splitted_dsts_len {
                splitted_dsts.push(read_next_tx_destination_entry(bytes, &mut offset));
            }
            let selected_transfers_len = read_varinteger(bytes, &mut offset);
            let mut selected_transfers = vec![];
            for _ in 0..selected_transfers_len {
                selected_transfers.push(read_varinteger(bytes, &mut offset) as usize);
            }
            let extra_len = read_varinteger(bytes, &mut offset);
            let extra = read_next_vec_u8(bytes, &mut offset, extra_len as usize);
            let unlock_time = read_next_u64(bytes, &mut offset);
            let use_rct = read_next_u8(bytes, &mut offset);
            // RCTConfig
            let version = read_varinteger(bytes, &mut offset);
            let range_proof_type = read_varinteger(bytes, &mut offset);
            let range_proof_type = match range_proof_type {
                0 => RangeProofType::Null,
                1 => RangeProofType::Full,
                2 => RangeProofType::Simple,
                3 => RangeProofType::Bulletproof,
                4 => RangeProofType::Bulletproof2,
                5 => RangeProofType::CLSAG,
                6 => RangeProofType::BulletproofPlus,
                _ => panic!("Invalid range_proof_type"),
            };
            let bp_version = read_varinteger(bytes, &mut offset);
            let bp_version = match bp_version {
                0 => RctType::RCTTypeNull,
                1 => RctType::RCTTypeFull,
                2 => RctType::RCTTypeSimple,
                3 => RctType::RCTTypeBulletproof,
                4 => RctType::RCTTypeBulletproof2,
                5 => RctType::RCTTypeCLSAG,
                6 => RctType::RCTTypeBulletproofPlus,
                _ => panic!("Invalid bp_version"),
            };
            let rct_config = RCTConfig {
                version: version as u32,
                range_proof_type,
                bp_version,
            };
            let dests_len = read_varinteger(bytes, &mut offset);
            let mut dests = vec![];
            for _ in 0..dests_len {
                dests.push(read_next_tx_destination_entry(bytes, &mut offset));
            }
            let subaddr_account = read_next_u32(bytes, &mut offset);
            let subaddr_indices_len = read_varinteger(bytes, &mut offset);
            let mut subaddr_indices = vec![];
            for _ in 0..subaddr_indices_len {
                subaddr_indices.push(read_varinteger(bytes, &mut offset) as u32);
            }
            txes.push(TxConstructionData {
                sources,
                change_dts,
                splitted_dsts,
                selected_transfers,
                extra,
                unlock_time,
                use_rct,
                rct_config,
                dests,
                subaddr_account,
                subaddr_indices,
            });
        }
        let transfers = ExportedTransferDetails::from_bytes(&bytes[offset..]).unwrap();

        UnsignedTx { txes, transfers }
    }


    pub fn construct_tx(&self, tx: &TxConstructionData, keypair: &KeyPair, tx_key: &PrivateKey, additional_keys: &Vec<PrivateKey>, tx_key_pub: &EdwardsPoint, additional_keys_pub: &Vec<EdwardsPoint>) -> Result<Transaction> {
        let commitments_and_encrypted_amounts = tx.commitments_and_encrypted_amounts(keypair, tx_key, additional_keys, tx_key_pub);
        let mut commitments = Vec::with_capacity(tx.splitted_dsts.len());
        let mut bp_commitments = Vec::with_capacity(tx.splitted_dsts.len());
        let mut encrypted_amounts = Vec::with_capacity(tx.splitted_dsts.len());
        for (commitment, encrypted_amount) in commitments_and_encrypted_amounts {
            commitments.push(commitment.calculate());
            bp_commitments.push(commitment);
            encrypted_amounts.push(encrypted_amount);
        }
        let bulletproof = {
            (match tx.rct_config.bp_version {
                RctType::RCTTypeFull => Bulletproof::prove(&mut OsRng, bp_commitments),
                RctType::RCTTypeNull | RctType::RCTTypeBulletproof2 => {
                    Bulletproof::prove_plus(&mut OsRng, bp_commitments)
                }
                _ => panic!("unsupported RctType"),
            })
            .expect(
                "couldn't prove BP(+)s for this many payments despite checking in constructor?",
            )
        };
        let tx = Transaction::V2 {
            prefix: TransactionPrefix {
                additional_timelock: Timelock::None,
                inputs: tx.inputs(keypair)?.get_inputs(),
                outputs: tx.outputs(keypair, tx_key, additional_keys, tx_key_pub).get_outputs(),
                extra: tx.extra(keypair, tx_key, additional_keys, tx_key_pub, additional_keys_pub),
            },
            proofs: Some(RctProofs {
                base: RctBase {
                    fee: tx.fee(),
                    encrypted_amounts,
                    pseudo_outs: vec![],
                    commitments,
                },
                prunable: RctPrunable::Clsag {
                    bulletproof,
                    clsags: vec![],
                    pseudo_outs: vec![],
                },
            }),
        };
        Ok(tx)
    }

    pub fn sign(&self, keypair: &KeyPair) -> Result<SignedTxSet> {
        let mut penging_tx = vec![];
        let mut tx_key_images = vec![];
        for unsigned_tx in self.txes.iter() {
            let (tx_key, additional_keys, tx_key_pub, additional_keys_pub) = unsigned_tx.transaction_keys();

            let tx = self.construct_tx(unsigned_tx, keypair, &tx_key, &additional_keys, &tx_key_pub, &additional_keys_pub)?;

            let mask_sum = unsigned_tx.sum_output_masks(keypair, &tx_key, &additional_keys, &tx_key_pub);
            let inputs = unsigned_tx.inputs(keypair)?;
            let mut clsag_signs = Vec::with_capacity(inputs.0.len());
            for (i, input) in inputs.0.iter().enumerate() {
                let ring: Vec<[EdwardsPoint; 2]> = input
                    .source
                    .outputs
                    .iter()
                    .map(|output| {
                        [
                            PublicKey::from_bytes(&output.key.dest)
                                .unwrap()
                                .point
                                .decompress()
                                .unwrap(),
                            PublicKey::from_bytes(&output.key.mask)
                                .unwrap()
                                .point
                                .decompress()
                                .unwrap(),
                        ]
                    })
                    .collect();
                clsag_signs.push((
                    Zeroizing::new(keypair.spend.scalar + input.key_offset),
                    ClsagContext::new(
                        Decoys::new(
                            unsigned_tx.inputs(keypair)?.get_key_offsets(i),
                            input.source.real_output as u8,
                            ring.clone(),
                        )
                        .unwrap(),
                        Commitment {
                            mask: Scalar::from_bytes_mod_order(input.source.mask),
                            amount: input.source.amount,
                        },
                    )
                    .unwrap(),
                ));
            }

            let msg = tx.signature_hash().unwrap();
            let clsags_and_pseudo_outs =
                Clsag::sign(&mut OsRng, clsag_signs, mask_sum, msg).unwrap();

            let mut tx = tx.clone();
            let inputs_len = tx.prefix().inputs.len();
            let Transaction::V2 {
                proofs:
                    Some(RctProofs {
                        prunable:
                            RctPrunable::Clsag {
                                ref mut clsags,
                                ref mut pseudo_outs,
                                ..
                            },
                        ..
                    }),
                ..
            } = tx
            else {
                panic!("not signing clsag?")
            };
            *clsags = Vec::with_capacity(inputs_len);
            *pseudo_outs = Vec::with_capacity(inputs_len);
            for (clsag, pseudo_out) in clsags_and_pseudo_outs.iter() {
                clsags.push(clsag.to_owned());
                pseudo_outs.push(*pseudo_out);
            }

            let key_images = unsigned_tx.calc_key_images(keypair)?;
            let key_images_str = if key_images.is_empty() {
                "".to_owned()
            } else {
                let mut key_images_str = "".to_owned();
                for key_image in key_images {
                    key_images_str.push('<');
                    key_images_str.push_str(&key_image.to_string());
                    key_images_str.push('>');
                    key_images_str.push(' ');
                }
                key_images_str
            };

            for item in unsigned_tx.outputs(keypair, &tx_key, &additional_keys, &tx_key_pub).0.iter() {
                tx_key_images.push((PublicKey::new(item.output.key), item.key_image));
            }

            penging_tx.push(PendingTx::new(
                tx.clone(),
                0,
                unsigned_tx.fee(),
                false,
                unsigned_tx.change_dts.clone(),
                unsigned_tx.selected_transfers.clone(),
                key_images_str,
                tx_key,
                additional_keys,
                unsigned_tx.dests.clone(),
                // vec![],
                unsigned_tx.clone(),
                // PrivateKey::default(),
            ));

        }

        for transfer in self.transfers.details.iter() {
            tx_key_images.push(transfer.generate_key_image_without_signature(keypair));
        }

        Ok(SignedTxSet::new(penging_tx, vec![], tx_key_images))
    }
}

pub fn parse_unsigned(
    request_data: Vec<u8>,
    decrypt_key: [u8; 32],
    pvk: [u8; 32],
) -> Result<DisplayTransactionInfo> {
    let decrypted_data = match decrypt_data_with_decrypt_key(
        decrypt_key,
        pvk,
        request_data.clone(),
        UNSIGNED_TX_PREFIX,
    ) {
        Ok(data) => data,
        Err(e) => match e {
            MoneroError::DecryptInvalidSignature => return Err(MoneroError::MismatchedMfp),
            _ => return Err(e),
        },
    };

    let unsigned_tx = UnsignedTx::deserialize(&decrypted_data.data);

    Ok(unsigned_tx.display_info())
}

pub fn sign_tx(keypair: KeyPair, request_data: Vec<u8>) -> Result<Vec<u8>> {
    let decrypted_data = match decrypt_data_with_pvk(
        keypair.view.to_bytes(),
        request_data.clone(),
        UNSIGNED_TX_PREFIX,
    ) {
        Ok(data) => data,
        Err(e) => match e {
            MoneroError::DecryptInvalidSignature => return Err(MoneroError::MismatchedMfp),
            _ => return Err(e),
        },
    };

    let unsigned_tx = UnsignedTx::deserialize(&decrypted_data.data);

    let signed_txes = unsigned_tx.sign(&keypair)?;

    Ok(encrypt_data_with_pvk(
        keypair,
        signed_txes.serialize(),
        SIGNED_TX_PREFIX,
    ))
}

#[cfg(test)]
mod tests {
    use super::*;
    
    use alloc::vec;
    use core::ops::Deref;
    use curve25519_dalek::edwards::EdwardsPoint;
    use curve25519_dalek::scalar::Scalar;
    use monero_serai::generators::hash_to_point;
    use rand_core::{RngCore, SeedableRng};

    #[test]
    fn test_clsag_signature() {
        const RING_LEN: u64 = 11;
        const AMOUNT: u64 = 1337;

        let rng_seed = [1; 32];
        let mut rng = rand_chacha::ChaCha20Rng::from_seed(rng_seed);

        for real in 0..RING_LEN {
            let msg = [1; PUBKEY_LEH];

            let mut secrets = (Zeroizing::new(Scalar::ZERO), Scalar::ZERO);
            let mut ring = vec![];
            for i in 0..RING_LEN {
                let dest = Zeroizing::new(generate_random_scalar(&mut rng));
                let mask = generate_random_scalar(&mut rng);
                let amount;
                if i == real {
                    secrets = (dest.clone(), mask);
                    amount = AMOUNT;
                } else {
                    amount = rng.next_u64();
                }
                let point = EdwardsPoint::mul_base(dest.deref());
                ring.push([
                    point,
                    monero_serai::primitives::Commitment::new(mask, amount).calculate(),
                ]);
            }

            let sum_outputs = generate_random_scalar(&mut rng);

            let (clsag, pseudo_out) = Clsag::sign(
                &mut rng,
                vec![(
                    secrets.0.clone(),
                    ClsagContext::new(
                        monero_serai::primitives::Decoys::new(
                            (1..=RING_LEN).collect(),
                            u8::try_from(real).unwrap(),
                            ring.clone(),
                        )
                        .unwrap(),
                        monero_serai::primitives::Commitment::new(secrets.1, AMOUNT),
                    )
                    .unwrap(),
                )],
                sum_outputs,
                msg,
            )
            .unwrap()
            .swap_remove(0);

            let image = hash_to_point((EdwardsPoint::mul_base(secrets.0.deref())).compress().0)
                * secrets.0.deref();

            assert_eq!(clsag.verify(&ring, &image, &pseudo_out, &msg), Ok(()));
        }
    }

    // #[test]
    // fn test_sign_transaction() {
    //     let sec_s_key = PrivateKey::from_bytes(
    //         &hex::decode("5385a6c16c84e4c6450ec4df4aad857de294e46abf56a09f5438a07f5e167202")
    //             .unwrap(),
    //     );
    //     let sec_v_key = PrivateKey::from_bytes(
    //         &hex::decode("9128db9621015042d5eb96078b7b86aec79e6fb63b75affbd33138ba25f10d02")
    //             .unwrap(),
    //     );
    //     let keypair = crate::key::KeyPair::new(sec_v_key.clone(), sec_s_key.clone());

    //     let unsigned_tx_cbor = hex::decode("4d6f6e65726f20756e7369676e65642074782073657405543582a66d85aaa08a665445b51cad7c13ce24ded1fdc6d21226d8d0b4615331cb80434adbe0f4e516aad44a0a13015396a1945601a9e3eab1c4315ae4695ea73e28f3428cf66a1cd344191f1d85c97a70620ffc77ab9e4478cca0d7e3c9492aacbc1625b462ff3d396e4478d4608173810c714d7b2bca717d55e41b183702432198ea217c9fb2a962ab0e5189a0302f7774377afa890b7851b268a16a263f0daa0715ebad83959cd067a472a7174471874be870f38c8af5af20c680eec99fecc1f23397bcb9ff4e63f5fa28c7f18a277b797a1173ab0e1eda3ccef5376204c364c50731c63d63f312bdc0f2a50e6b726c42de07d84fa19f2bfcc0133a4ed24239e191e3562c6612211b4d78994c23711331975dd16bcde6991862b7a354212d1547f4f7edabaa57feb3d87b01bd8687ea65446f73119f85fcdab0d3fff3a4c60bcb87442f3fcc016f6dd46a6babc56c22b5ac6eb0eaefb17fe7d750b6b2a1e1990ce5f07b3292a60db37cafb851886f1653b41dbfcfbbfa55a1916ad23598739699b8265b02b4ae68a86d4d17fa77428b4017506ee640aed29d36532fdeec7f7b329a8edebb54bf2245b6c8d4d0c1f0aca00d4c2ed71ff0a5c57a8364382d03afbe4a48e3136ab816c9705095fee4892ace2a6b4eb920079527b2394baf3fa5d1d8f7ff08618a1934e7e34f38bf3f69d15139492c664e317f8d916d0b0cdf3fc8dbaa6ce73d9a08e27e641b5e2e3c543a551dc7ceca3bd7f54c228c63672c0d4fdf1004135e1f10e3243ae701a0f134efcd874e0d32d8f52cb4cfae1972e9ebe358ff69ff2427cdb43220c2b7992f6d4b1ba882c3d91ca91e0655c3e6c36b3765a3f41e75e5747203fae2077a33df7f2015c39a5ad069301d52fbf95d2fbc1e1c45aedc0253992c8211494212f6ddc942e7b0256e36aed8da044b7096f7e768e042216898ff6550b5245086fb20cacc0af9d53fd1f0cafba025d26c69d898e06fecd423c7961727c3cf109d4c429f049f23b791c39b6868b65b1dc699aa97d21fd54d4d1cb891d880685528e57796943b89cad902107824a0de386cc4c94e5fc1d7d45aca2db5ad2526ec72170554c9edc0ec73693712ff21712965539f32925bc4345c0f62867f4fdd21f98451c223a67c75bf7cf56fbdef82f762f48052c2fd8f922616e779884c2f533f36063c0a3bf350f53c3990eb815335cc6925dda858123648e2b83ee040e016a618158558f106412b7d99094431508ac7d2eb32b3e495faccaec0aade80a6e6b5958a79faadbe53f21540721362a6d99bc38f3e5d8e10ce049770d763d1669f764c297b44f91232ef2d286ea450a8c07f2caee5ebfe6c5b65c498d54343fe0980d8da872a7ec8eb15ec3ebdfe60482d85a1b52dc5136495c7d9e88ae2652bc6c8c8e765be605667c102804fb2bb118ca9d800b2c5649aacdc9cd591cbad79370f200dfce1d204e2031711f00d7503d613bfd3674b144911d7c9d7a07f1ba7d841587dd41f76dadd5967b329bca7d347570a87f483a770d5e74587d17b36a0313c44e4322f1cff801524d645df1d504ab8e7ac2f5d9fd41fe26d7188c1089f596668624b2a1e755b63e4ad456d466eddcf5f8837a868c26ccf9f3fd3088d544b8ebf113ee0de0254683808d269c8bbefe9b2e29f8d3f839c59216bcbb40ba4b79350e4d9e7d283953da6c89b7d226213fc53a73fd138c10a2e17b5dec4b864d17bb0dd3a7184ea8afbe3c26d5f2abd7bdf71fcfb12b35ef81b5d5d92f4391c26ac927f75475731fda3e0cd72196c0b97b445cf6a6ea5e8dd4cb44463445537e989e4b8afe47d98ffdd7efdec2f57725f3a5dbabb8e2f464bfe3f6931600681eecc28a120adcca788ba91208943c44cd98c8d70e7d0c3100530a01bf5a8eea88742857be2aa69aea0e08d3076efd8c71635c4e42d533f9ced9fc2d491589c48992a7d51f0e39b664aee8c22a64538b0b4db2b57f1595a3f372779c1c773b75f7e9ede737e44998270e0b2bcc267d5e728ef9100f52f89e9a4287d4a214da03f9e9e667b4facb4e3a85c6b18eb5d039b40f68d5be76e3a28d1fda09e32ed8740589e0e371179af44ab257e72fec909440245fe3ff3483ba6d70d1fe4d51be4a440f966a720fc999eddee8a1eae684081b48b44a53d9f9db16ae660540afeb2cead4d9a5049e3ff2fc9f60dac662d9f6102cb0d266150c9d4fe760f5851ebb5e82a654ef21e9a8b1684a98fe2d95a238c6a6bac53498e1d196939c92fe06cf40052d541f0ee33d00e6d07d55331eed1c9de08c66fb255c58bd50fef1277bcf4f58d1a2b43d3858da56af0c8a6e18fcab82aad00250c7e2f656a2597efb9170386f553aac412d2cd00c9ce588f10dadd2d570b82757b1a3aaaba08e5d17305a6e21fabacd8f32be120631dd6b247736e221028e82faa443e796d4129ea08e50b1d238b90c05e389cd5a0f02f3cd676ff5f3ee93c27f5b6ca65e403081f0c79192f14df529b80e13857b521b3b63a1dafd9de9d1403fcd9a0521f4af48589298ec68c9772c010a9e603a91c950c9197b3ee86ce98a6f20ffa4cd7622590051c22a78370944c2f6ccd1dbe47f0f84b95bbdc4172fe85b3ba02612bf10cd060bfc3f9c53194f3b4b89b0565f5720cde69fdb5c07d3a62ae0db9150ba9c9e83ba98eaed08ebee28eb19f7f9819809c054049407b7c60f665ca4d001a71a86ede08a2506aa15a7eb7b2cc3f748b3dde7e577689365232c8762786af9c90260040e504f32467bdbf655800cda481de448fc587f2088e32b816c913cee5c2d65baa05e27668b56485a128b2f5ba1876e9c9df8a61453ab2b46a17adf22b400fe7143124b45fd5a9b09f0b025efed20c1140dca51246148fb3e2379862196256b29d8c96052bcface2f0caeae96dafb169adea2135cb69c1cbfe7aa04db0ee1173e1b8d8f094a5ef2e972be12be60852f82ba3524a011492b02675716d27473575b90704bcb0378091d4b60ea687938008c481a6a7ecd508de2e733a694ec4b8d20c315f4fae9bd3edbd33a38c40d50be9aedd37ee08aa70d2cd4abc5f0844730c3fd93b22b8af6a5f8730d229b577c911aec8400540a28f188cb702febf68c8f180a77f8a6c9e1e5526a5eb89387173716675b49a6d6117f6bab93fc55a3d88d4adc41e05e88d2d8e4bd4becb11b9f30800d22df4dc4a004d2e4c00ccc26d4fb594e98b06f3f17dda8e7a322a906a96bb799f3dfbb76c584fd547e880b51fd2d13a598ef142b8b9d069f8d00fbb8f0b5c1581a8d8e9a80efe66176168fa46e456eaf7d9a2628527e20a5ce72f69c86a86ea4ef19c6d995db14f0d8fea655e2200eec5e7121fecdff6066070d1e652591fab3a2cba4165a5971cdf933a57ba3d55c277e7dbf48ee8eff6177d22d9ccd8a7b1b77e8f026796a1e8610fc3f24210975cea576e3c22414a70f1a255e1d17b667eb552b1a9bc262925adb4c3eacd15bc5ecd51157f6408f92b8ac172c3cc3329dd55bd7a11da9c7d6aab6e631158ca090372c18ec35c734522985ea2f3059ed0021748e71d058252ba73820855f609794a3488be81a5b6ae25dec13bb276da805494603921073dfd60246494e168f7af8468cdbc961ace62c33b830cbe081731fdaaa28731ffcca328dc2fb1986495139b329e80e14794f72504e3a94971b54982361837d1e94f3c3aaa7625a6e28312af6bdc5c2a1b64fa6ed398d1b59ac2d08b6d98d3f3698c54fe75852353c0bc3f14290ce4df070f825aa6a8dd4f1e070177ee88d732e860b2f7bc1cea4aef5c34643b470ecab392045cf454a826f2381d6f23881f53c7e09ef60070c55740baf32c9c56c61f3768a4456bd072c5fd93d3359cb4539fe85762fa28e5118840e3c6600c7795ca1bd0f64f654d9d09c1ef2c9720cf8a1c386b27dd642689592d9dbe68b4a9604a208ed95d2acc9149f36b6dbf7e0a53072c13d28520361edaeeef8cc7ff2e78f50e8e0dd131435fd496e74048b24c21a6367c5b46182e22e454950e5187269c089f13252f8ce225036ddb682dda4957bfa85c005365ff7f2eaee16b29ac2d32d64aa10438a80bf660421cc8266751d1a6ba1361c0161145722b5763ba0bb48c4454b61ef91200c3180438fd7715138fdf8fd7ce3448d7f55dc12d3f05357ed4de69eff1c37a9242cbf94f362e5bab8a90ffcf851c3b27f1002db0f04510da11898596666e81dd1999a035768e934c8b17bc688079bfde98cd0fb409ccc24714258b4bf19e2a6ee45945252252a0679c59a254114e22ff9f8872cac6b076dca95fc7008e6ccc5cbb0a91aef6a4ddead594b49e65410936b35b3e8034920d01cadd4b2730c9efdd1760f50f4dc511cd1110bf9148ace40fc779999dbbab19cb69d6e15ffadb1d7f56d708258a0347e63af31494bb01f5acd6e8790245efab07b946194b72dc4e71a4920a").unwrap();

    //     let signed_tx = sign_tx(keypair, unsigned_tx_cbor).unwrap();

    //     assert_eq!(
    //         hex::encode(signed_tx),
    //         "4d6f6e65726f207369676e65642074782073657405ac90bff4a8a3e5ec8c3388ab705ed1b92a54e56255367593895a619297b5a514eff74cc1a4d25e1885e954a49f0fe32d17c9a1f645676f9da9784e4c20dbdb8dc55674bca8b0b5d7789fec1a60e856e72f9eec005394ead1aa35b3aed93c1407f111094e9acd823c8521041a2befced0af23d53910c157254f9ef8176e7747117c8d95cd00a33f1ce731d0960bf17a0fe5c1017e72c0d37aa99e14a74f164226e09ea7ab6303632d45eac0cb2450f98d41e26853831ff00614e053c177c44981e3baa3d4aa5643899fd7cb948887ca2b089d867f187678f0d41aca3dc62f7b4235091b9cb8bc42895e330c67255be3977c81e934d59b0a814c7b905f9c8e315bdb8c12d3d1884fa0af55eb1c50e862a046d844049404fea59b115f5351460d60cc09f7ccfc0558ad03b9de8121465b27361e3745b7ee19022538c781f7e60f37aa9494c60351452c1812c987e5279eb60bdc07ac073c2a4ac39c812929cacbcbf7b3e9e53f92c7bb8bed39b53c330652b7ff217ebbc153d1e55c76d35c5ed8ebc3d949109602553d18059af8951f0133da347dc8ea656352b5bad5a1092565ae60cae35c8899edbdc7e782768b40d04c8968f4c80b84f43ab3ab005a7e7d9f3acbb2b59d35313151a5b20e80da0e055d402e1a4492d239d7b9583a72c67303e299e1673a21d219d36b0f318669166b8ee22c18487174a771462815cb0bf94b7335fb77d56497299e5cbc222fe265359149a82a49bf222ecc7f501f8b7d1c7ef314acd9ca11ac1d88695baf616bd27192be6408b99b8ffa9b6bd29aa175d6d1cb9fa11ebb210bbb8c8e66313fa7e39b6c0eee511e674fb74d56a4e5583fa7de2dcf59048833989b4614c452a8d069284340e62285e2c3d6e5b3c9aba6eecf808c927b58d5350c28c2400aa1c39127c9315a57a8155f31f6274a371e971cd7d680896500bf2c4750be115b17296c6c8c5c6be72b1fc8877f7a1a0de3cc7b5a529a63f1a9bfd2a191e6fc1b50a40936eb8e7d102d5f84d4d8faa93587c7dd5c76abf81274ee0d76d1eb193eb735e394ad658b2f722aa27cf4975fad45a3958b8fc1f0e0af1cfe97929109938206512cce615421e738532072e1552e07794a3d415ab702ddb2bc52b4be0b06d163d9d9fb88d946ab06434420092e793746d5f32a274af9373eb33d3390e12be1f13ca25e71813d1a5a249cf21f9a92c6d4b51a3cd4bdebc9ae198479503ffb55880775ff85faf557c0b99440d174b7ce78c04e97bb133805100f1144d70427128f31bbafd2d995a1cb8e74bb4132438a33598279d131661f6868fb8e6410cb3c1c9605fb67624c198bbf0d0891df1f1d1614e5009e3171125062d2dc44900074c02773e3d8d4a8433329955e82bd9c583d7bce7c1fe0fa1402fc579a1106f3a30eaff9ff3119831f7d19b1b45dc48d678db8a94419099dc8dd1ddd7223bd90489af1169e5aefec1dbaa5cb3d976bfd0ce5276b421f6c994e9219248e38da639bd79de74d4c798c4fa5ee520d5f14e35dc678492ddb1c885d199344190892f19f1844538c10f40267c5d61dd1efea0c19025789125e68d5c4f6b97b8f4324ba76e3e8c303b93174e7c96eb3a07a833c737217a16a7721ff0a25efc28df8202b716237daed9dc27231828dcdb7f1c7eb26dcad925f36d2584f4009aa3fec7b6016a02f270a9171d09a6dff72bf2629948117ca3438fa4b4e2fd4f5a0da1c126586c56fb61ffbc17099b691c7488e490a63fb9ff4751526f8c450c53febe6f38018b872bd273cbe730e0053918d02eff4d6cfd7b5a4499fa3ed7c5067ed8d6c5632b8ff5aa6bc319585d5ea7f8065420c24fa1b4fb693a24ad1acdb8075022fc00c3f138fc750df59ffa7d2e87c100be2adccba73a3175f7961217af8b212687bfc2c33da36a88e279c9c57fd2678451c9352819f0ef4b394994095579ce51547a477485cb8f276064bd5cd8e3dc190f9365b33dcc86ad4faf29530e740d07e57130c035da356a81e151acbe1c85739d489c416a44b9139cb94a7aae8f9ad8cf2eab2084fa101f9466f80312ebe4c579faca2389feeef9bcedce3d71acdbeea3576ab16031ebe18193217dcfc1f9a9d2054192d1a3752d4e5e399a71eba8ba159bfc01c353ccf283eded9f257bb1b3829f22504c8ed2589a55cf893a4a995ea39e8d924593ad330809e3acb100efdec97eca442baf6a860571e8200406664ece78807501c1c7b10b92e351f473a8e9753a50edc7c77c967c3e851895bdb016315c516e2b45fb1f1274d4bd3b780f322dacdf4b77e252ff5e1b83d8e31f38ff5158bc88fa860b64bd870deb2de94aad7bc2dc40f87097cc5c4cffc50a5358f38662414f1429786e6e1e10d97612d1d845749984386391443d475115315ae55ff1927b5f0fedc640bca0f238dbc9df2ba854a8d34857078776cd93a361137d9b855a4b4059a1f4d4dc10588ab840899392cac38eb1b8fa1bd09ae9f279ba724f6ddd740be6092bff5f9b14d3a836295e7e084c71b6056189bc62c65a4ae419554971f2aab584a78cc36485524206737b1c1f5336259c866018316081f079c78acd0592376f3f4cb864959e24cca1f4340aca2c09edcbae4e6114527f83e4b377dcf71d40c67bf41a021487387ed4c5166c029888b039e55728a8ca8ab37cfe53e425bda50ec3b194cc062484f1092058308550117582aa9798bb240d1e0edb35125b8701bc630e44fedfe2902dceb4ca75057479fdebc9ead9b7883138b56bea1dad40db063b8e72cbf0d8f39725858f6545f0c4dae55d1f7a136d8aa59557359250105931a22f4dfe2b87d2155e48a8c89efc6939d7755d62858a43b66e933a7a6520cefd4542993e81a133c70177a876050ea2ca30a5dcce281c062973c88a0bd05f6b2bbd57b0fa19a0ea55d64ed43b73e280a048108fe43f8955abd13a7656d1bfd7524cedd978a576023794245a117ba3ad2899e3f8f02e33e23232f16ac7e075cebb1aa29d46894e7352bebe754f4236c6eab678050c7180b57163773017828b4ed8a9481c5b231a8d30f2ed5f158a4814bd5dcc7b8e222be4f77b03198b78b35c30bd1b0b22f83e6fd9340a965006b834580915d1b5e8ad249fbb6402694d531130fc629ad701bad45be31c47af8b66e9a9691ff8d9b54998dc772999262217f689f6f672e3546914ecd0d89dcb07cdfc3ab07c0f2ac678722a5c2f02301b5fd9dae5f93036357c41037d3140d9917ae14c048ab7a2314711f86b74dd129bc659ed547a9056987ee976cf86933714d5e63545355666d32c64b697ac10693953ce5b7e108903ef20f466a3862f659ae6a7f927e7d26590e306401bdcf1bcb667cab5ecc2e385dcab3398417ed0d95261c2f2389611662eceb6362c99fbb3defbee748e57899f01e51c7cf55690919dc846ee48f37d2f6dcc202cffb2985f63ee060c444beb69316c822dce8cb6c44faa5ce947ffdcff4062d8dbb6a8ffa827917d75c02cf0aa41b27808116b715f9ea78a827476fec0e78d40996771494d4db282db77c6751cbcd955f25bf84ef2e1b6875a0fc8980c3d55f1cbbf1803f27a7178792fc0db3951251b655f8cba7ea01e0e303f0d3c960e0229283969220a128ffe7865bcb724b8823f50c89ce5c3298f37cf6a64aac08b9b01435fb9f2d7f6b9c3791b2664a0b1f5b04f41f89b21fbf88c876b591494270a2ca4526296a4c3bcf67e24872093ad89afa2464a2ef7a74756cf7d453f3b0d0677dcbf83a94ddb2f88072c057f3c8ffb1bd99639f0f0bb52ba24937df23c84d1179e4046fc1e2d666cc96fbb55974a92511b5a594ff0223402687a5d21851efe6851bc5afcbad0e1b780df867f3e11c9e7bfd25936fb38886d72b02f65a248391d4a6f46dec0ae09622f95834efa5f78a17ba5be566d7c42b3fbd3a8bbb7240567dcb320516f1694471402f809d4d1ca1e6d1b0365e5fd75ad11efddfcb9136654657bebd600e4ef1d13cfacd9095bda027d38c19c1c5f602427781d723a85950bfc085b8d83de2c8370e32dfc707f8767990d1d6fcc23c4825a01f345702bfc4cdf30e703be4f463825fd1eebd854a5d63a946ec835ae3cd205e928e2b7bfcd98d4526e0b8467c004412e2cae4b1ffd30c571ecd52fa67b9c0d5db82e80a28ce5f47058befafb25c9bb598627f33d5fc818efb29ef7447d67d3ed8a014bfadca41f3a4ed63e3d369edd17382e788e8f6d73b100c85c3a8ecbfd4a6f5c21c69d224157435953d7ddbc77f39772881402d0a717a55c05b850b3678a3cb9f876c9aa1e922b31166519a2eaa830549d709c1ed25cd9d9b1fff23139587887a2d54057104a97d001cde35728f924d20d3aa7a196e6ca2282e2f3422920de550604218c28aff4a6dbad37645eeed7ec0290ee1e7babe837bc5d2e593702165a704263ddb7de9f2f2ae17d71f443c2579f937b7b56cfd0d6ef0e6940ebf5b531ef2b29561426939d559ae7874ae305b1dee9d21217346a719b284c60e4637eb7889935593d75dd0a421fabc7b65fa8abc2b879e12afb9877f5616a64007f79efdc762ead667ac99692ef1198ba2d17de9760e4ec7f09c6ff271c357d32eb845af52da15c1ee258d5c270d74e4773748965ef082185604164214597479bb51bfeba928c63975bf1aef1ca23761510c6b2420ebd639d8c310cc4b0ed09fac4f72b14d68cc911b5ceaa93c32a6b581e1649a18824b7080d1947afd4b16d51b4254a296bb343d029dbb9467bd2c6a698caf74242de7d0c6bf0fcc8ad25d128f26883686d8edad42b72923ea8e8af531381baab0d9319827d4e8137962dba94b621a9660bbb6a10f949c213c24534baefe46d1f586604c6265d5d71b070076ee2f1036948b50324f30493b46142300f7d8492f9e5eb4fecc9b1c032c320ca470ffb3047da3a0293cd303546189d6a3e1101968b71d77698392210adb89689856af96d796959e263a07978e9f10f2980e40cfcb79cdbb70dd1e54e87a61706728150dca3024658c32e527885b268704e62d081af5566ee219272c408b6081beb044113ab13a6708bda7bf7149b41dec869ca1f3f6709dabd08bac6bf3e97ede9aaa315b3d01917bc4c281e5b28f50fa30bbef4c4e8642e0d4ba6fbfb7ff7f2e70c523a144795a32e0787f05ad90025fce964490ef5270bd8bc9f0a1b4a6ea6e735643d766afd2934d290deeb35b9757f2d35947ebcd83cba3122d1bdc1596bc93ae6daae61777af35cee6dfe45d51175e0d391d092ed994f1f2c90a450e9a8f451b4a0ec75534658999f5f1c16e234f98598402f227c5dd7acf9a8fa9ba901563f1c0866d32b37641b564153560e6306038d0c3e3aef112a88bc48e579ed3792a90780e55c221c0532cdaabcc14fadff633b72f587ca1cc32a9097598b2be24b1f36b41937c029596a42f6b8f947fada7183bf3ff24d6a7ec8f37836231908745c547cf093c9090c47e6c61a087fa85c17ed57190035ab39ce30ff224e9ce6a45086476216155af903420c7159869e896f42d311a3d6495f1dc5d45b4dc8cc51298c4fabed9b95d5857c10d1f791da493becac0773317e8a586257e446e219c66b2e667d347b20be93c4c5549c5e7af869489ab3b52fa056b3dba7ea63daadfb5103ad435ae159a2a175ef4d0cd5af8e0f38f8a73649b8db545b0eef449bf52759e371da64482d04b1bc66c1edc5d7e11a6c529d058822b86ed9e4900613ad38c099236e2aa8bee79293e532845651466f35a1954e737bcf3c9fc0cde02ba06ad6eacfea9431c19a2d8685c059812f706b5921e048cf60f848d88435f721ab126f7738eb6024ca25a601404121957f901546d447da172cc626c6c3cbf45aa51f99ef7d16b25212395c59f5fe6e466dd3edaf9fbb07ce0d71027b5a0337a65b12fbe9caf879126e86679a20a57cec4098906b5bdb5a4553904634af6cb4cbebf40ac03bf2d7004d3dc0f889a63d7abbf24f5d9309b6d181f1693d864ffdc67a14225a0701d9cffa9392d64d2ae02172a51a4822cc914ce925eb279f0455c969e64704afaa0475970ba9aeac4c647fbcaea1384c75451cea5a72f5c5a8110f31cd60312ec7a36859a141a08b4a502149a3186cd452a80d7f9ee93243a263ab6429f72f7c344d59a0179924f3a55e103a1adabbcc10bae64b84a1dbbab9ef67c726f8e902be985f252ffdea4370893d0143387188a382fbfd4b086f7d4ff16dd4aebc7d0ec28c824d43d19cce1ed623f7fd94f9ebc835a9dfde1307fa260f9d0bff6d8b2a78cc9a6783b137db9ff385adc12772fd6cae611838f0b795717840b8dd80efe8f134adbffc22777287b7558ec51ae01911f9cb762543e41a495b9f061bf38fa5fff1aa831997e9d31d6e587f01f4938cbd84d8c26a015082266b7873db304f9b7cca844aef81208b81f73ecbc4a01aab473c4b5ec32e31026b6369c5ffa602d0bfd3c5e675a581a9072e391db4e44eec9523f3f142dca4143f587177197c8cc594a289e8c98666f851b2df87c78d897176d11df13af5b76ed09fc9c6de2ab0ee8aa9fedc10a47a0f4c814db3d8bf2ac63fe325a683427c48479d5f89e90c2b47c625c81d2081184f9cb3890ef380250ddac04ccb093a1d5f3955ff2ea9fa1e0eb782203c4b8175c837706f5713ab80e34d39a5bc76cb10a22de8f9b5ad36e16859a06812651150b858531cf310a731c54f3adab14f74760617694b19ae15107eb62dfe234c1751b7866310b9b0a1340962d81e4445aca7e8dd4f6421785e4d22c2e181a31ca6db8e233fb80533351395dea08813f2158149ddd27e8eee8bc2915eb2dfebbc6d52a330109f2089097cbc35c40fb7bb4b0e0cfbc0920eebb32927120d3da89a9c22b613dbe9d474f92b56ceac0cd60569174971106083ea665c773f6c09888f9c198eff8f364a83f09fffee3dd9e34d248beab04411f5c0025861ed7196f877a192c4f8cdf9b67e39db8c6132244e2979063e3187271fc3b9bcd56b20a9b92b88a668e57230e0b4745ae0a7a4ae5f797b8a110b0fb8f1b34f4a325acdf08ac8db88c0cbe9a538b3c044416fa4b5cf85efec4abc07d989b11bb379f6251cd1a1bfde416319287f03e37ba079dfdfe1e32050874408c46d2edb18a4321c6b9ae1c562611b4bd1631d657b5de45a91fd22e72be955b2fce8c03ff5f9dd345cd846239eb98cee0e4bc526a3f3dd783f1f6c246482f46241fc16e273f32a218c3a9dd9d083c6ec3df1240ad99807bad8f3dde238875965f0b64ed728a1aa6d9d28bd7339a585d440d3908c7b9c1a69e077f60518629c942d68744012669ad5fe44ac84ddae1c8f4968db9f360da4afa062656d99600d33b1cb3e89887cf0a28b7182babfac7f2cb2e0a765ba6ddd0aaf14423e191381a03b8eb110b34ba5b670e3949940393cb916823d4d924226e43110f5c0990ed6241c1b954e38facf076f54baf4f78032db6626942ebf4c77425afa035456d1125ea5a9e70dea24bff1f38fbdc21390b6c988effd244d5ccc33c8164b03114c9d616efa2cf677b5136d4109a3178315b675ac88e7b6a544b079c7439453ad44e667b980cfea26f4119f22280328f5bfa9e304eb50ebabd70012c18260de0937a58fee8d1fef2637c9b5e6e450d49fe6ddc2f8a800ed85b2275f4f4716eb78616b96e31e91cb5146f6711f3ac1371029e1389a16ddfd4ef53561409d5f713539877d1f40a94ac39f76a678d5aef7ca65f95b784e4d5f19ba7d6d514332fb2cfed018cc932fe6f957966b3c2611fde40772b5365d08a444b93cc9a65175adbdcb3b87c7bf878963e5b15a6a07414f4d7c732b38e1473f25ec785a041d053a0cf6a8f97007130aabfe380bef2cad95a0d6c432531e46c5207b3cef2a28ba5589433bfa78647d7b4a5075a2d6408ca19c926a16ae797368e3a1ec1a75d6799a83a5c4fe1d820f9a651e1754f9771833b5363a24121015cb1a686049ef314b6ed841b9dcf0f047defcaa31947ea42195af1f1aa5db609b82c47c5a9573137cc04f129be9a0a5c30c01607d39c275396dd6555ceaa8e3d5a53bab21204ba8e404de6886c178cef85d97ff28abe433aaf8075b15af4a3ad65a7cdd7d099158a5845e0c5c4f0342f53e653517eed5f33957db2ef312e841eb745f5affe69619222fb70ff9f2b7526f63b3bd930b7aa7113a9aead2e4e226f67b00262a2838c995f16f3df245d3b012f7673110b064c622afac05e68178f509362f440996fc704a63cb3ece1a49f7a9c9c9d690f"
    //     );
    // }
}
