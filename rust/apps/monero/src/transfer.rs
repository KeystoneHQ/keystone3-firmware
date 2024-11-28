use alloc::borrow::ToOwned;
use alloc::vec;
use alloc::vec::Vec;
use alloc::string::{String, ToString};
use curve25519_dalek::EdwardsPoint;
use monero_primitives_mirror::Decoys;
use crate::key_images::{Keyimage, try_to_generate_image};
use crate::utils::*;
use crate::utils::{hash::*, io::*, constants::*};
use crate::address::*;
use crate::structs::*;
use crate::key::*;
use crate::errors::{MoneroError, Result};
use curve25519_dalek::scalar::Scalar;
use monero_serai_mirror::transaction::{Transaction, NotPruned, TransactionPrefix, Input, Output, Timelock};
use monero_serai_mirror::ringct::bulletproofs::Bulletproof;
use monero_serai_mirror::ringct::{RctPrunable, RctBase, RctProofs};
use monero_serai_mirror::primitives::Commitment;
use rand_core::SeedableRng;
use monero_clsag_mirror::{Clsag, ClsagContext};
use zeroize::Zeroizing;
use crate::signed_transaction::{SignedTxSet, PendingTx};
use alloc::format;
use core::fmt::Display;
use crate::outputs::ExportedTransferDetails;

#[derive(Debug, Clone)]
pub struct DisplayTransactionInfo {
    pub outputs: Vec<(Address, String)>,
    pub inputs: Vec<(PublicKey, String)>,
    pub input_amount: String,
    pub output_amount: String,
    pub fee: String,
}

fn fmt_monero_amount(amount: u64) -> String {
    format!("{:.12}", amount as f64 / 1_000_000_000_000.0)
}

impl Display for DisplayTransactionInfo {
    fn fmt(&self, f: &mut core::fmt::Formatter) -> core::fmt::Result {
        writeln!(f, "Inputs:")?;
        for (public_key, amount) in self.inputs.iter() {
            writeln!(f, "  {:?} - {}", hex::encode(public_key.as_bytes()), amount)?;
        }
        writeln!(f, "Outputs:")?;
        for (address, amount) in self.outputs.iter() {
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
    pub fn to_address(
        &self,
        network: Network,
        is_subaddress: bool,
    ) -> Address {
        let address = Address {
            network,
            addr_type: if is_subaddress { AddressType::Subaddress } else { AddressType::Standard },
            public_spend: PublicKey::from_bytes(&self.spend_public_key).unwrap(),
            public_view: PublicKey::from_bytes(&self.view_public_key).unwrap(),
        };

        address
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
pub struct Multisig_kLRki {
    pub k: [u8; 32],
    pub L: [u8; 32],
    pub R: [u8; 32],
    pub ki: [u8; 32],
}

#[derive(Debug, Clone)]
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

impl InnerInputs {
    pub fn new() -> InnerInputs {
        InnerInputs(vec![])
    }

    pub fn push(&mut self, input: InnerInput) {
        self.0.push(input);
    }

    pub fn get_inputs(&self) -> Vec<Input> {
        self.0.iter().map(|inner_input| inner_input.input.clone()).collect()
    }

    pub fn get_key_offsets(&self, index: usize) -> Vec<u64> {
        self.0[index].key_offsets.clone()
    }

    pub fn get_sources(&self) -> Vec<TxSourceEntry> {
        self.0.iter().map(|inner_input| inner_input.source.clone()).collect()
    }
}

pub struct InnerOutput {
    output: Output,
    key_image: Keyimage,
}

pub struct InnerOutputs(Vec<InnerOutput>);

impl InnerOutputs {
    pub fn new() -> InnerOutputs {
        InnerOutputs(vec![])
    }

    pub fn push(&mut self, output: InnerOutput) {
        self.0.push(output);
    }

    pub fn get_outputs(&self) -> Vec<Output> {
        self.0.iter().map(|inner_output| inner_output.output.clone()).collect()
    }

    pub fn get_key_images(&self) -> Vec<Keyimage> {
        self.0.iter().map(|inner_output| inner_output.key_image.clone()).collect()
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
        if res.len() == 0 {
            return res;
        }
        res.sort();
        for i in (1..res.len()).rev() {
            res[i] -= res[i - 1];
        }
        res
    }

    pub fn inputs(&self, keypair: &KeyPair) -> InnerInputs {
        let mut res = InnerInputs::new();
        for (index, source) in self.sources.iter().enumerate() {
            let key_offsets = self.absolute_output_offsets_to_relative(
                source.outputs.iter().map(|output| output.index).collect()
            );
            let key_image = self.calc_key_image_by_index(keypair, index);
            let input = Input::ToKey {
                amount: None,
                key_offsets: key_offsets.clone(),
                key_image: key_image.0.to_point(),
            };
            res.push(InnerInput {
                key_offsets,
                input,
                source: source.clone(),
                key_offset: key_image.1,
            });
        }

        let key_image_sort = |x: &EdwardsPoint, y: &EdwardsPoint| -> core::cmp::Ordering {
            x.compress().to_bytes().cmp(&y.compress().to_bytes()).reverse()
        };
        res.0.sort_by(|a, b| key_image_sort(
            &get_key_image_from_input(a.input.clone()).unwrap().to_point(),
            &get_key_image_from_input(b.input.clone()).unwrap().to_point(),
        ));

        res
    }

    pub fn derive_view_tag(&self, derivation: &EdwardsPoint, output_index: u64) -> u8 {
        let mut buffer = vec![];
        buffer.extend_from_slice(b"view_tag");
        buffer.extend_from_slice(&derivation.compress().to_bytes());
        buffer.extend_from_slice(&write_varinteger(output_index));
    
        keccak256(&buffer)[0]
    }

    pub fn outputs(&self, keypair: &KeyPair) -> InnerOutputs {
        let shared_key_derivations = self.shared_key_derivations(keypair);
        let mut res = InnerOutputs::new();
        for (dest, shared_key_derivation) in self.splitted_dsts.iter().zip(shared_key_derivations) {
            let image = generate_key_image_from_priavte_key(&PrivateKey::new(shared_key_derivation.shared_key));

            let key =
                PublicKey::from_bytes(&dest.addr.spend_public_key).unwrap().point.decompress().unwrap() +
                EdwardsPoint::mul_base(&shared_key_derivation.shared_key);

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

    fn calc_key_image_by_index(&self, keypair: &KeyPair, sources_index: usize) -> (Keyimage, Scalar) {
        let source = self.sources[sources_index].clone();
        let output_entry = source.outputs[source.real_output as usize];
        let ctkey = output_entry.key;

        try_to_generate_image(
            keypair,
            &source.real_out_tx_key,
            &ctkey.dest,
            source.real_output_in_tx_index,
            self.subaddr_account,
            self.subaddr_indices.clone(),
        ).unwrap()
    }

    fn calc_key_images(&self, keypair: &KeyPair) -> Vec<Keyimage> {
        let mut key_images = vec![];
        let tx = self;

        for index in 0 .. tx.sources.iter().len() {
            key_images.push(tx.calc_key_image_by_index(keypair, index).0);
        }

        key_images
    }

    pub fn serialize(&self) -> Vec<u8> {
        let mut res = vec![];
        res.push(self.sources.len() as u8);

        res
    }
}

#[derive(Debug, Clone, Default)]
pub struct  UnsignedTx {
    pub txes: Vec<TxConstructionData>,
    transfers: ExportedTransferDetails,
}

impl UnsignedTx {
    pub fn display_info(&self) -> DisplayTransactionInfo {
        let mut inputs = vec![];
        let mut outputs = vec![];
        for tx in self.txes.iter() {
            for source in tx.sources.iter() {
                let output = source.outputs[source.real_output as usize].clone();
                let amount = source.amount;
                inputs.push((PublicKey::from_bytes(&output.key.dest).unwrap(), fmt_monero_amount(amount)));
            }
            for dest in tx.splitted_dsts.iter() {
                let address = dest.addr.to_address(Network::Mainnet, dest.is_subaddress);
                let amount = dest.amount;
                outputs.push((address, fmt_monero_amount(amount)));
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
                    outputs.push(OutputEntry { index, key: CtKey { dest, mask } });
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

        UnsignedTx {
            txes,
            transfers,
        }
    }

    pub fn transaction_without_signatures(&self, keypair: &KeyPair) -> Vec<Transaction> {
        let mut txes = vec![];
        for tx in self.txes.iter() {
            let commitments_and_encrypted_amounts =
                tx.commitments_and_encrypted_amounts(keypair);
            let mut commitments = Vec::with_capacity(tx.splitted_dsts.len());
            let mut bp_commitments = Vec::with_capacity(tx.splitted_dsts.len());
            let mut encrypted_amounts = Vec::with_capacity(tx.splitted_dsts.len());
            for (commitment, encrypted_amount) in commitments_and_encrypted_amounts {
                commitments.push(commitment.calculate());
                bp_commitments.push(commitment);
                encrypted_amounts.push(encrypted_amount);
            }
            let bulletproof = {
                let mut seed = vec![];
                seed.extend_from_slice(b"bulletproof");
                seed.extend_from_slice(&tx.extra);
                let mut bp_rng = rand_chacha::ChaCha20Rng::from_seed(keccak256(&seed));
                (match tx.rct_config.bp_version {
                    RctType::RCTTypeFull => Bulletproof::prove(&mut bp_rng, bp_commitments),
                    RctType::RCTTypeNull | RctType::RCTTypeBulletproof2 => Bulletproof::prove_plus(&mut bp_rng, bp_commitments),
                    _ => panic!("unsupported RctType"),
                })
                .expect("couldn't prove BP(+)s for this many payments despite checking in constructor?")
            };

            let tx: Transaction::<NotPruned> = Transaction::V2 {
                prefix: TransactionPrefix {
                    additional_timelock: Timelock::None,
                    inputs: tx.inputs(keypair).get_inputs(),
                    outputs: tx.outputs(keypair).get_outputs(),
                    extra: tx.extra(keypair),
                },
                proofs: Some(RctProofs {
                    base: RctBase {
                        fee: tx.fee(),
                        encrypted_amounts,
                        pseudo_outs: vec![],
                        commitments,
                    },
                    prunable: RctPrunable::Clsag { bulletproof, clsags: vec![], pseudo_outs: vec![] },
                })
            };
            txes.push(tx);
        }

        txes
    }

    pub fn sign(&self, keypair: &KeyPair) -> SignedTxSet {
        let mut penging_tx = vec![];
        let txes = self.transaction_without_signatures(keypair);
        let mut tx_key_images = vec![];
        let seed = keccak256(&txes[0].serialize());
        let mut rng = rand_chacha::ChaCha20Rng::from_seed(seed);
        for (tx, unsigned_tx) in txes.iter().zip(self.txes.iter()) {

            let mask_sum = unsigned_tx.sum_output_masks(keypair);
            let inputs = unsigned_tx.inputs(keypair);
            let mut clsag_signs = Vec::with_capacity(inputs.0.len());
            for (i, input) in inputs.0.iter().enumerate() {
                let ring: Vec<[EdwardsPoint; 2]> = input.source.outputs.iter().map(|output| {
                    [
                        PublicKey::from_bytes(&output.key.dest).unwrap().point.decompress().unwrap(),
                        PublicKey::from_bytes(&output.key.mask).unwrap().point.decompress().unwrap(),
                    ]
                }).collect();
                clsag_signs.push((
                    Zeroizing::new(keypair.spend.scalar + input.key_offset),
                    ClsagContext::new(
                        Decoys::new(
                            unsigned_tx.inputs(keypair).get_key_offsets(i),
                            input.source.real_output as u8,
                            ring.clone(),
                        ).unwrap(),
                        Commitment {
                            mask: Scalar::from_bytes_mod_order(input.source.mask),
                            amount: input.source.amount,
                        }
                    ).unwrap(),
                ));
            }

            let msg = tx.signature_hash().unwrap();
            let clsags_and_pseudo_outs =
                Clsag::sign(&mut rng, clsag_signs, mask_sum, msg)
                    .unwrap();

            let mut tx = tx.clone();
            let inputs_len = tx.prefix().inputs.len();
            let Transaction::V2 {
                proofs:
                    Some(RctProofs {
                        prunable: RctPrunable::Clsag { ref mut clsags, ref mut pseudo_outs, .. },
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

            let key_images = unsigned_tx.calc_key_images(keypair);
            let key_images_str = if key_images.len() == 0 {
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

            let keys = unsigned_tx.transaction_keys();

            penging_tx.push(PendingTx::new(
                tx.clone(),
                0,
                unsigned_tx.fee(),
                false,
                unsigned_tx.change_dts.clone(),
                unsigned_tx.selected_transfers.clone(),
                key_images_str,
                keys.0,
                keys.1,
                unsigned_tx.dests.clone(),
                // vec![],
                unsigned_tx.clone(),
                // PrivateKey::default(),
            ));

            for item in unsigned_tx.outputs(keypair).0.iter() {
                tx_key_images.push((
                    PublicKey::new(item.output.key),
                    item.key_image,
                ));
            }
        }

        for transfer in self.transfers.details.iter() {
            tx_key_images.push(
                transfer.generate_key_image_without_signature(keypair)
            );
        }

        SignedTxSet::new(penging_tx, vec![], tx_key_images)
    }
}

pub fn sign_tx(keypair: KeyPair, request_data: Vec<u8>) -> Result<Vec<u8>> {
    let decrypted_data = match decrypt_data_with_pvk(
        keypair.view.to_bytes().try_into().unwrap(),
        request_data.clone(),
        UNSIGNED_TX_PREFIX,
    ) {
        Ok(data) => data,
        Err(e) => match e {
            MoneroError::DecryptInvalidSignature => return Err(MoneroError::MismatchedMfp),
            _ => return Err(e)
        },
    };

    let unsigned_tx = UnsignedTx::deserialize(&decrypted_data.data);

    let signed_txes = unsigned_tx.sign(&keypair);

    Ok(encrypt_data_with_pvk(keypair, signed_txes.serialize(), SIGNED_TX_PREFIX))
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::key::PrivateKey;
    use rand_core::{RngCore, SeedableRng};
    use alloc::vec;
    use core::ops::Deref;
    use curve25519_dalek::edwards::EdwardsPoint;
    use curve25519_dalek::scalar::Scalar;

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
                    monero_primitives_mirror::Commitment::new(mask, amount).calculate(),
                ]);
            }

            let sum_outputs = generate_random_scalar(&mut rng);

            let (clsag, pseudo_out) = Clsag::sign(
                &mut rng,
                vec![(
                    secrets.0.clone(),
                    ClsagContext::new(
                        monero_primitives_mirror::Decoys::new(
                            (1..=RING_LEN).collect(),
                            u8::try_from(real).unwrap(),
                            ring.clone(),
                        )
                        .unwrap(),
                        monero_primitives_mirror::Commitment::new(secrets.1, AMOUNT),
                    )
                    .unwrap(),
                )],
                sum_outputs,
                msg,
            )
            .unwrap()
            .swap_remove(0);

            let image = monero_generators_mirror::hash_to_point(
                (EdwardsPoint::mul_base(secrets.0.deref())).compress().0,
            ) * secrets.0.deref();

            assert_eq!(clsag.verify(&ring, &image, &pseudo_out, &msg), Ok(()));
        }
    }

    #[test]
    fn test_sign_transaction() {
        let sec_s_key = PrivateKey::from_bytes(
            &hex::decode("5385a6c16c84e4c6450ec4df4aad857de294e46abf56a09f5438a07f5e167202")
                .unwrap(),
        );
        let sec_v_key = PrivateKey::from_bytes(
            &hex::decode("9128db9621015042d5eb96078b7b86aec79e6fb63b75affbd33138ba25f10d02")
                .unwrap(),
        );
        let keypair = crate::key::KeyPair::new(sec_v_key.clone(), sec_s_key.clone());

        let unsigned_tx_cbor = hex::decode("4d6f6e65726f20756e7369676e65642074782073657405543582a66d85aaa08a665445b51cad7c13ce24ded1fdc6d21226d8d0b4615331cb80434adbe0f4e516aad44a0a13015396a1945601a9e3eab1c4315ae4695ea73e28f3428cf66a1cd344191f1d85c97a70620ffc77ab9e4478cca0d7e3c9492aacbc1625b462ff3d396e4478d4608173810c714d7b2bca717d55e41b183702432198ea217c9fb2a962ab0e5189a0302f7774377afa890b7851b268a16a263f0daa0715ebad83959cd067a472a7174471874be870f38c8af5af20c680eec99fecc1f23397bcb9ff4e63f5fa28c7f18a277b797a1173ab0e1eda3ccef5376204c364c50731c63d63f312bdc0f2a50e6b726c42de07d84fa19f2bfcc0133a4ed24239e191e3562c6612211b4d78994c23711331975dd16bcde6991862b7a354212d1547f4f7edabaa57feb3d87b01bd8687ea65446f73119f85fcdab0d3fff3a4c60bcb87442f3fcc016f6dd46a6babc56c22b5ac6eb0eaefb17fe7d750b6b2a1e1990ce5f07b3292a60db37cafb851886f1653b41dbfcfbbfa55a1916ad23598739699b8265b02b4ae68a86d4d17fa77428b4017506ee640aed29d36532fdeec7f7b329a8edebb54bf2245b6c8d4d0c1f0aca00d4c2ed71ff0a5c57a8364382d03afbe4a48e3136ab816c9705095fee4892ace2a6b4eb920079527b2394baf3fa5d1d8f7ff08618a1934e7e34f38bf3f69d15139492c664e317f8d916d0b0cdf3fc8dbaa6ce73d9a08e27e641b5e2e3c543a551dc7ceca3bd7f54c228c63672c0d4fdf1004135e1f10e3243ae701a0f134efcd874e0d32d8f52cb4cfae1972e9ebe358ff69ff2427cdb43220c2b7992f6d4b1ba882c3d91ca91e0655c3e6c36b3765a3f41e75e5747203fae2077a33df7f2015c39a5ad069301d52fbf95d2fbc1e1c45aedc0253992c8211494212f6ddc942e7b0256e36aed8da044b7096f7e768e042216898ff6550b5245086fb20cacc0af9d53fd1f0cafba025d26c69d898e06fecd423c7961727c3cf109d4c429f049f23b791c39b6868b65b1dc699aa97d21fd54d4d1cb891d880685528e57796943b89cad902107824a0de386cc4c94e5fc1d7d45aca2db5ad2526ec72170554c9edc0ec73693712ff21712965539f32925bc4345c0f62867f4fdd21f98451c223a67c75bf7cf56fbdef82f762f48052c2fd8f922616e779884c2f533f36063c0a3bf350f53c3990eb815335cc6925dda858123648e2b83ee040e016a618158558f106412b7d99094431508ac7d2eb32b3e495faccaec0aade80a6e6b5958a79faadbe53f21540721362a6d99bc38f3e5d8e10ce049770d763d1669f764c297b44f91232ef2d286ea450a8c07f2caee5ebfe6c5b65c498d54343fe0980d8da872a7ec8eb15ec3ebdfe60482d85a1b52dc5136495c7d9e88ae2652bc6c8c8e765be605667c102804fb2bb118ca9d800b2c5649aacdc9cd591cbad79370f200dfce1d204e2031711f00d7503d613bfd3674b144911d7c9d7a07f1ba7d841587dd41f76dadd5967b329bca7d347570a87f483a770d5e74587d17b36a0313c44e4322f1cff801524d645df1d504ab8e7ac2f5d9fd41fe26d7188c1089f596668624b2a1e755b63e4ad456d466eddcf5f8837a868c26ccf9f3fd3088d544b8ebf113ee0de0254683808d269c8bbefe9b2e29f8d3f839c59216bcbb40ba4b79350e4d9e7d283953da6c89b7d226213fc53a73fd138c10a2e17b5dec4b864d17bb0dd3a7184ea8afbe3c26d5f2abd7bdf71fcfb12b35ef81b5d5d92f4391c26ac927f75475731fda3e0cd72196c0b97b445cf6a6ea5e8dd4cb44463445537e989e4b8afe47d98ffdd7efdec2f57725f3a5dbabb8e2f464bfe3f6931600681eecc28a120adcca788ba91208943c44cd98c8d70e7d0c3100530a01bf5a8eea88742857be2aa69aea0e08d3076efd8c71635c4e42d533f9ced9fc2d491589c48992a7d51f0e39b664aee8c22a64538b0b4db2b57f1595a3f372779c1c773b75f7e9ede737e44998270e0b2bcc267d5e728ef9100f52f89e9a4287d4a214da03f9e9e667b4facb4e3a85c6b18eb5d039b40f68d5be76e3a28d1fda09e32ed8740589e0e371179af44ab257e72fec909440245fe3ff3483ba6d70d1fe4d51be4a440f966a720fc999eddee8a1eae684081b48b44a53d9f9db16ae660540afeb2cead4d9a5049e3ff2fc9f60dac662d9f6102cb0d266150c9d4fe760f5851ebb5e82a654ef21e9a8b1684a98fe2d95a238c6a6bac53498e1d196939c92fe06cf40052d541f0ee33d00e6d07d55331eed1c9de08c66fb255c58bd50fef1277bcf4f58d1a2b43d3858da56af0c8a6e18fcab82aad00250c7e2f656a2597efb9170386f553aac412d2cd00c9ce588f10dadd2d570b82757b1a3aaaba08e5d17305a6e21fabacd8f32be120631dd6b247736e221028e82faa443e796d4129ea08e50b1d238b90c05e389cd5a0f02f3cd676ff5f3ee93c27f5b6ca65e403081f0c79192f14df529b80e13857b521b3b63a1dafd9de9d1403fcd9a0521f4af48589298ec68c9772c010a9e603a91c950c9197b3ee86ce98a6f20ffa4cd7622590051c22a78370944c2f6ccd1dbe47f0f84b95bbdc4172fe85b3ba02612bf10cd060bfc3f9c53194f3b4b89b0565f5720cde69fdb5c07d3a62ae0db9150ba9c9e83ba98eaed08ebee28eb19f7f9819809c054049407b7c60f665ca4d001a71a86ede08a2506aa15a7eb7b2cc3f748b3dde7e577689365232c8762786af9c90260040e504f32467bdbf655800cda481de448fc587f2088e32b816c913cee5c2d65baa05e27668b56485a128b2f5ba1876e9c9df8a61453ab2b46a17adf22b400fe7143124b45fd5a9b09f0b025efed20c1140dca51246148fb3e2379862196256b29d8c96052bcface2f0caeae96dafb169adea2135cb69c1cbfe7aa04db0ee1173e1b8d8f094a5ef2e972be12be60852f82ba3524a011492b02675716d27473575b90704bcb0378091d4b60ea687938008c481a6a7ecd508de2e733a694ec4b8d20c315f4fae9bd3edbd33a38c40d50be9aedd37ee08aa70d2cd4abc5f0844730c3fd93b22b8af6a5f8730d229b577c911aec8400540a28f188cb702febf68c8f180a77f8a6c9e1e5526a5eb89387173716675b49a6d6117f6bab93fc55a3d88d4adc41e05e88d2d8e4bd4becb11b9f30800d22df4dc4a004d2e4c00ccc26d4fb594e98b06f3f17dda8e7a322a906a96bb799f3dfbb76c584fd547e880b51fd2d13a598ef142b8b9d069f8d00fbb8f0b5c1581a8d8e9a80efe66176168fa46e456eaf7d9a2628527e20a5ce72f69c86a86ea4ef19c6d995db14f0d8fea655e2200eec5e7121fecdff6066070d1e652591fab3a2cba4165a5971cdf933a57ba3d55c277e7dbf48ee8eff6177d22d9ccd8a7b1b77e8f026796a1e8610fc3f24210975cea576e3c22414a70f1a255e1d17b667eb552b1a9bc262925adb4c3eacd15bc5ecd51157f6408f92b8ac172c3cc3329dd55bd7a11da9c7d6aab6e631158ca090372c18ec35c734522985ea2f3059ed0021748e71d058252ba73820855f609794a3488be81a5b6ae25dec13bb276da805494603921073dfd60246494e168f7af8468cdbc961ace62c33b830cbe081731fdaaa28731ffcca328dc2fb1986495139b329e80e14794f72504e3a94971b54982361837d1e94f3c3aaa7625a6e28312af6bdc5c2a1b64fa6ed398d1b59ac2d08b6d98d3f3698c54fe75852353c0bc3f14290ce4df070f825aa6a8dd4f1e070177ee88d732e860b2f7bc1cea4aef5c34643b470ecab392045cf454a826f2381d6f23881f53c7e09ef60070c55740baf32c9c56c61f3768a4456bd072c5fd93d3359cb4539fe85762fa28e5118840e3c6600c7795ca1bd0f64f654d9d09c1ef2c9720cf8a1c386b27dd642689592d9dbe68b4a9604a208ed95d2acc9149f36b6dbf7e0a53072c13d28520361edaeeef8cc7ff2e78f50e8e0dd131435fd496e74048b24c21a6367c5b46182e22e454950e5187269c089f13252f8ce225036ddb682dda4957bfa85c005365ff7f2eaee16b29ac2d32d64aa10438a80bf660421cc8266751d1a6ba1361c0161145722b5763ba0bb48c4454b61ef91200c3180438fd7715138fdf8fd7ce3448d7f55dc12d3f05357ed4de69eff1c37a9242cbf94f362e5bab8a90ffcf851c3b27f1002db0f04510da11898596666e81dd1999a035768e934c8b17bc688079bfde98cd0fb409ccc24714258b4bf19e2a6ee45945252252a0679c59a254114e22ff9f8872cac6b076dca95fc7008e6ccc5cbb0a91aef6a4ddead594b49e65410936b35b3e8034920d01cadd4b2730c9efdd1760f50f4dc511cd1110bf9148ace40fc779999dbbab19cb69d6e15ffadb1d7f56d708258a0347e63af31494bb01f5acd6e8790245efab07b946194b72dc4e71a4920a").unwrap();

        let signed_tx = sign_tx(keypair, unsigned_tx_cbor).unwrap();

        assert_eq!(
            hex::encode(signed_tx),
            "4d6f6e65726f207369676e65642074782073657405cf55731f284aa4c10ee438d9eb26062d5a79a0f53038b54edbcb7d8dd7782ceb4355449a0bd5b54dfd85a1884af8d74861c6db4a7c81da97b9f6e0c14f2d6f408729db4cfafec76303ccb7716b98119f1d6796e16ef05dd3182b503cca25e9ed541713e9dae0863bc629db187888e094995a529ffa71d866ae53e51d5feba94e669995cf773309dcae2bd7b95a4c88a553f97f7cc344c252fda1ba8dafa7dc84774bcb8552cd9e1f113eb49bc2fd0d45eead2bc62be572ae82969bffdf00a49467da7fe8d31c26ba025d4fcaf8f1ecf8e835d356011b24e66b9e7c694fc478ac7cce3af83af1b0830d45e7c7dbd59bd425da3b05e7362a272dfab2deae01ab56f36007fdbec5e4e7af1ca0b317a6d91728bd56f7db84e421038bb043e1d0cc2699cbbddd39b73fbc367ed5a799e7798f2f65e3b20b6c6217e0b6efd7ca71771b030d92038233ebbebc87b7719d9485093cc32e7533acd03ceea5d4ff45109d7f148288c36e03539448359eba52cd4c1c6676599b4c93eb3d9b7e6f4a3809bdd75cdd5de18f2323b18bb06786c28b4bc640d108ed1edc04feb919f000ebe52613be472cef230b30426c3c8941823032e03446cf5bdd7b6d5e79bb4e1ef954ab3d123d338846528036f6befb330f1cfe2c4547ea21695486294afff56db0df84dbdfc832679aaa0760c56733347f734c70c596d7dace6de4cb68f0a78b2e979d3f9bdd8f9f85d316e4c6382b4f0058baf5894a3919df8e54bc9505758ec3f18ad15bb5cdc4ad3f76727a0c41e5a3bcd72380883869831c61d87a07847a2a0e9beb143515f1f731cfe727dbfde367e7967e9e8ff8bc722f985fbed20db4954030dad5d30e7628ef395920c49433e2aba740bb228fba5cc352876c9b1cbb442250ceb9002833265279ed669a174ab3452ecfe6927307ceb25552420a1fc01bedafea98a4ccbdfee045f8ceeef13819a99e202b45ee6f9e8100cf56e5abc6b5b0251c60e4bdb8258ac874b3c41be995496b4cd55134c9594527f5f3450f3188f44122b9261acc7499d0fac00a11667d6f493f54f9649137ddc26de2747f528dcab53e22ccc5449299e6fac3e5e8be86b454cd30870bc0c137ed31cbab1e4947f9924d014f9b169875d3fecce5e746c33e4d27f507fa3974b6fe6accf423233022e93dc0f4a4e1dcc7fcd3778723b107501d2725841d579528275308566a208b67a6fcb7d55e49b9d7a18db13813676c51df960046c5b698b46bb7c8749a8a5a9ae306c42ef4fdeeae31012b6c63ac009efb086207aaf1cd5e401616f079cb472745b64d172a841aac670af1147e256b9d58db823045213e4523bcbca2d492a5213786bcfa22315311d98eafe3da91eff38157d952879bec8a92561ecb35938b3b31d4d70da84e496b10117031e0a62f7625bbd7c44d2147aeff543b6874b2bd4d5f0724b1b5f467ff4db89c31f803deee56561d738594998b17fdddad7b590f81c68edfc590409bc9d4ed4d4aba08f9a184489f09041a0f91af7768fb8aea790f2d4108e4945b237bcd6990e89c1096118267e5f615b784a75899460707b6d15126a1731258d08ed825e20b57800d5e57ba90ec65b01117d6fb6856f247de292e6ec75c62f8b0e9e43947bfc7cde0705eca0b4bfcaa712f0871f3a902497d56f226b9cde7d5f095b15774301bfe76846517291064e4fcd3423678c31502925c115946823c4e01db7524a8f0837d9fb585c13f9c1cc2c3aa3cf6e166bd198286a5b956b62e6d995bb4b2ce0c2edfad6299ba6ab4316c41e028224286a67889680250665281add76c720922997fd2691c419eef636cf6e8068a0fb29398bd760fa6f4fa8aaff0999549684ad9deadc4de329e88d5a555fc89bc40783a4e16f804cb2fbd86fd2700c5510d9ec13eac352c822736a44e7f9d6751b3b673470ab217f774ce62c693beaaeca556093a13e4d18b0377111ba221a6175978a2635affa8794924ab9963e6afd738c013eabbd8dcd27f2477ffc58f3b625bfde5e71ee2efb8df243c2efa8f7f199812cd448735ac33043bdb26e31d600faad0e09323f18abfc4880489cf32f0b44e5d2f9c4fd780ba831a14cc7849f0d751cb56fbf1906758c563aae7dfbd2a44d561c1cde47a3beb220b577ae6367fd9b1a49f2d25c1458e7542737a7c7099fc7bed14c23c5a2dab5acc14aef2f34552f95f3db305cde052b070b11228d3bb59cf6173cf148e2f5276274e6ddf68a74fae8e175f7a1952c0eb536821cf9c5fab3553b4e70c463c750c7eb010f106a0b059577aeff8b5f11dfdeef5f60263639f11e319653a91bb48da5f57bf47dd83f671b1b75502052ac8dfd5ae67f1edd177719a5946e6807938511a897d0b23aa99798ab4135781e114e8a027bca689fc00021a5b8cef5f24959f744ada3d8bc46aa8bec55f5b7c12bc08a085a47e4b7f15b5ea7a132dc1188db48c2cee11c8ac597b02a9343c7e7e205f36631b7c9477c793c906d9a450c80710c212248cb1dfb47f44104bf45136627ae4de912392b67da536521bb994d44227cfa3940e41148fc74ebb42d0dc04b6182943f2133a9c0144f42892d294bf8692cff4070ddbfab577694d8a23743b208d2ac59e6e9fdb87828702db94f568cd7e9f9383c64bd082bccc032155a2f0effc34eb23bb409a15e5e31b63ac6f701019acd7e9f8b61ac5820a396e36775f62589aee3c4bef02cfae71cd0ff332a6aa0539791e4e5c14849bc0e22157bafb7f9a781f66a6061ee33135f0f4cd6fbc8b3229d3f12cff288751b6d03415a83b529edae7369a5faae2326d29c8abe9e24f3c2de62e2b5786e39f93bc3cffca9568b77a110a80e3af0fc83a510aef15fd2a186a4fdb1b9a0f7cf77fea384833c89bf465d899fb55862ae02d23e67bd733953ae54e849fdd68d18c793681a0bfc651582a824868d4f32dad3e15ade0c2efce62c0d37e7e48bae05860653858dc0d9b806144ba5a02d2149c9194b6a62294049dbb47a405fde8f5cde6a8bea0469d6bfe195bb51ff91c68c0e73cec67ae4ddee14d6e007576dce5e99081c8eec5eb86380df3dff9f1ec3659a77db2d7b3a1025c9b450a266e4a00925c34be3ad08fd712552819b42a92ddeefc9411fbcbf9de54cc92c1210d36759ed971162ae67e445c2c2b23134cf473cb0a447ae3702b1126c6ab2cb056e7b996dfc766ddbba7c319225d67a0fcd28e21867e8dbe86ea789ab1548922775f0516881698d2dbdd86e6becfe96261822f40704419ef823f2d603e5837c16600a33ff1636cc4e21623afb5626f3acb30cf57aaf782278edc65d50f047e90582170243aa4458b4a185210a1fc0cd495b067584e4c505ea721361c35eb4101b2dd7358b105771eed5277263b5e78e122ac4ed5d2f4555211f31c934409d30304001ca02e94b82242c84884737996517b96a0a61fd558236ea201902c146d40320d8d9b8f60976c6c31ac2d6c0a915f1c5b70621a6c00611ee34118ee4ff023a1b74039ecf5304bfd00e89f4398a5837d5cb37ffc218a55ceec9331af415b0d080b25fea6a827d3fc66b67f6bbee5d4059a338c3a7762e76c253f87635066a45b11a11cf6c750886a459da41cae0ee50e374be2b1e173007416995c38937d790fd04e944962d12626f62ceeed863b189687cb958c1269c6e6b5d617708b9bd095de09fe24efd3f34e6970a429725d4ee72486f8afbcfc41de3227d352bec306de0347139ac84cfa5c042615ba4801695ee2f22dc4d95103caa896fa3a6d5f6f811802eb9757a68f376901766fbb85f79d323d2a9c0ffe323061980369fb327fbc7b7498da8fdd5311559e1b768f46a62b6c1206501004924e24abfea80e54f26965261bc3d1acb20ef0a1b832b10cef458c1c1c0d164a364c11b6fc236456ca3064b97fb47ded3c0552320dafe1a21c8a85df60128a0a227a14e4ceb438dd6e6f0220797f09e3a26f56db846afc47919c7cf585b4c0b44400519d38f17a77b004373fed67b907d6706b3e38783397df83cdf43d7e7df781997d7a00d6290b5acb4d51abec73a02536516a0cbbf5a9cdccc565add2d0da36c989095e26a7faedb4c0358636c962a6b9e78c353e4197cd4c5541b93fcbb99c56814d37c0f82422a33e515054629f3bf6509e64450644c58de96a05e5d0c2653972f89abbbec69456dfa6b8812e1ae913d6ab80719872da2e8414a29b318ccd3ec1ebcb4a941fa21d586df89ad5da95b1429ced26277f3960cb90432e9d1eef35d298eeffa8c87482ed996c750a212285873ec4c67ed44466c966b6e7afbb9241bb6a35545e48a417bb0ade3d9348aeb85afddbd4287cdc3bf0f6466cd1a5012d8407eaa7ff843fe1ac3a8d51455164da3c2533fb1fea0a77a50d86baf9179971e8b6a1faee7f1c4f61c17fd2100f8ff06564990227fcd784a9530560b47c40b5987d68b3498e05f75c1813d213d32e749cbf85d4808942442ad68a8295a120a084767e1ab351839fd15f55548ad477f8e66f446091b69ab4225b88e1031f93850d73401b7db17fdcccea19f931ee0e4dcfa59ed9fcd39e370902848ac82a26919162334ae6102ca1dff22a64c2515e130cf341c21d68c44a64efcb83f44a64023e9aa48e0696a55f72829088bd1dbe70efc55f81b6359e9da0d9a0465082c1184f729c800a49739f1c7444ecb89d1e4c372e3f9cdea4325fda431834e262e6050e797250c63f46cbd790aa5137221e7847a62dff7a4e8681fe6dc6baf20a36fa12806a5351e65b7745d294791383d02524616bedcff5e5587d2058c6f4d27e11ff6f153d4491a02299c1019451bb0a7c8c0dc9e2b726d64c7e6a6ba4507d61da54379a5666671768220684c3785f7814bc44abf154d61a19eb8a851a5d6cf4d5411ad876c9f68be1bfbb38ce179d6dd40eb1a98cac6bb500a81f294e7ca85bc21db938dabca7374419a65a4b1b0dc01a12860903e32f44d796a7323508b234a81ea2166610cc39ba5a807a3ea5aee19d6d5d089bc7caf30af19d32a7c994ed2ab40a9b919ba827d3c419d2a241b9403cb54dc8e41d118398731aa19640dd7e803baf418f9d7808979ee18913b5d5c794a565a2e191dfd49bd4da87db25e39e3ad9e71a38e28934a6f9ce449152d595a6557efffa2824426e3190beaa2651e4ef4816bab2276d4ad344bf6fcfcd0fb5df63f21fa6a85b759bf259d55303528ad12d517856eeae9d85c5e3a20d4ed782d36b27fd4ae817829db5c550ee0f8330325f9bf8b8d3f96ab073c51f76b64d1a1df2a699743610882b802f7328ab9340402ed29c6d15ffe5e8c38f79105d1901dc56d171b6f10a3fda98559d79ff49846298cfe2816a57fa1cc256075c1086fc9cd0853d1159ca46c47b089bfe8cfcaa1dee4c23921ba0222c7024a585d4a98423f54086801cfca59d7f9ef08e9595887df5136facec28955f758bc95ff1c5cd2aa0a5f5cc10adcbbd7131943995a54ceeb6083d3ef348f67f03543d9840d5f8a351acf332cea413df369c716bc38c8e041ef262c84e10d6ce0dacc6b10fbe475cbc23646e4c74c7018b5c6590e7438a682fd420138b64b1f6f7b3ba5f7f6937cc518ae23f9b774d6d23bf8499497ca700ca94f66c97245f075bf15bf3c2811bb61e3fd894375425d3432163971c579495a9d5672b7ed3a3f5984e40ba6bc872f3f2ea678339d2fd2bd504cdf0e7bdafe99b7a301d09815fb3cedb03a8228e3a283692001a03df8539f5cb045e8b7cc31a70b352ebcbbd2ca194cb09c5a780b3bfc2917b9280fe8fbca9625fe359dfba02f9956b5e358efb5650888ebb888a6dbfdcde30b67da7ebc47e8a621f9ae41e87e59545d090af25c1a237c89f7a877db8e283d00e4ebca2cafa77086570e23d986db757ccd45cee1042a6e050f71d07ea83935150555e2f817363002d2e1f28a43d6b6b65c8d6a54f82eb891ff226cbb13c4258c99c71ead878b04a2b7ba543091c8c3dad6a7fe4ea1186e248779178c675dff20765f3722e34baa27809c6bd1acd6d3fbed1311406afa6d54813d13a52bedfacdd2466e11bb3ad21a38799241105cd191c001804daa5d73e641d9333e2273ae317194329b9cca8ca2478f9e7184957d298dd57075803c6063008de96677e41bafe40a83416c2b289a6df5aafd918c12da81b1c5eee91bf7df9c5573f0f39e86a95e45f85a22d16881a78ba179b6e731081f3e788f8bdb3e255862afdf15194796907be1a38ea3b0093fcac73e030bacfb15c8b04f8fa7e551012f0684ff17b8e2c21915c3ba46c1de1be33aeb418f62ad8cac179b3ad5a157a984d8fff9fa012f9a95c31769521f6d93c7ef6718981d1f7f6f23148394b49b7bf371f53d58402c191483d9688ccef9b316d12ceee5c3e2adcf6e34ec6756e2b99844199a1c9cf0b9d1968cfee040f1545bc93b98cbb83a95e7fcd3b10b3c9dcd825805d43ad280d2d5031160b0262c47624f825b22047475306e9a3ce8659570d52d18edd39e2c36be4da454ed757497dccf11ff381fe22286836ce85c720823483372112df45de40987bf43173fe847ddf1a28eab5eda170c44fa0f32a0869a3a40c36d4c6b630d354f37c8110ba62deb90cf3de8797f5af5a1817f8f7f17c6ebfcc21664ca582b7c7dad6224863e641298e6dd02ebc4394a10cb912e5fa94b51b7ef351aa2e91ba95900b008f271f6a8ca414d9e3d8719f62601e773f1fc1d74fc3f0b214cf8b62b80b84db5cfb1567fb9b582c4d309da6a980e921e1563a014360e4d51b966d9ebac7fee64c6a710f5601b9ef4bbe83c0ea1a27dd31a8aebe0521266324d6af36888a5ba312afaca388b70a96fad2c6e97b3c633c7cc21fc54cdb1cffa91e6fe7d0ab9c334c25778cb3b32c522989dc12ab5b43c2ea78ca64d835fbddc731369fdaeb78d5109b1793cdc4dfea0b7157ddc8bbf09249c475fa290023884ffc23cf2cfcfcdb59f2f4aa6227b00160a5a0c551f0f1c62c5b644612fc3e28558897b1f5a6319f4a7878c88d19b92b71ae6d0e9bb48aee0a9808a6dd62be048c6aa4e9fa4ccc646afd9450fdae0e69d11a9698580221ead48b3aa2284d74ffb5319cd1b0ab5b84f13959b06d144123438144b1adc5ad027f3e402fc6ed9987717cb0045b65aa064d46dc42d24026739b282dff2c2bb1e5772bdaf7a18a091b1da6cbe9263c75ec869deef70f1cf26013040d53dce677e743ff5054c75555f527feff3a945bbb423d3bc5048c4688785797c0fd9f228662929f20cf2c336ca8484743cab454a5bc9fc6b1f85c095d2f77c015cb161535eb45bdc4b0f4ab04f3a0d7bbaf10ea2f1d360cff8ccfbd1a37c856327d4b8fa2f08d6f12423bcb4233dd10b6e8a6a3df59d6fdc3bf2fd55e7aa9ec5e38a7c240d137f7ae86f9fbf9aa6bde0d9fb1da72def2bb5181159e3aea675d39121cf587061a76bf33b4bc76e35744a3d3586de3b1340d5e7c8b7c435862e3aaa6807cfd18677e762e07e4eb6dd7cb10f4e8ca4f0e621cbf1ad4a2b4bcf8cfec2be02c438f5351712b88fb7bbebd0302caed051e4fb19e1c9dfa4c0d59bf2807a03edc58c14dd6855eb283c9f5ee788160dcd4defaec600835f9c3e4b9873d86382740d153beea5c40cc99d67a4ce76a8d8a5b3161df62d829e3f37944221d56368691566aadf704a0bb651dbe3c04672930fd79eb0ed647a35dec87303edefc85aa4d0d46b390896f1fd50c022c131e7258fe839de8540f7e3b8190c33bfd5cb3a2fdc57efc8d8aa5f76b2ff69564bf76f2a12f31d0695d10ecd0e5fad6569becc8bdba6ff8496ed9b4af6508e9694be758ed6fc1885c3bf540f8fc627720bd914f0ad68d8a2dd7aac10928f9e36b86d735a51b15cdfe4c50d4dde32e30106056711f422bf7b48b652b2470d6175d4488d3384774433ecbdf56d9275aa9ddfd68f73ac1cdef2ccbacbc938029ed79ea6f246e569c7756c19d468e82975eb64b9cec6f7456b9135162b34958f4701aad0ddebc11c836e4d5f1c50fdab76c8c800a26e5cce2a4583e9fb2c9ef0ba5e20f88e16b8db10867d32259589d92184bb676d9d696e53ca263785ee4b77ba1253dbd669264e3751566aa23d0ba5b21d65b6f92ce0af69706a601b94dac1dabb3ca87cf52480a747823f49e8e9474f18d8f44fa7d6d777ec7fb4ea15cbc0df049346cde9f35c9ea0ebb55eb44a678453effc3f2f78b47de27a2f5f6ad2fb8b10e41a30464863697f92cb66de534129277992657534cbf205a8ab2f983a8cd93e2366807"
        );
    }
}
