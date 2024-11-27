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

#[derive(Debug, Clone)]
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

pub fn test_sign_performance() {
    let sec_s_key = PrivateKey::from_bytes(
        &hex::decode("5385a6c16c84e4c6450ec4df4aad857de294e46abf56a09f5438a07f5e167202")
            .unwrap(),
    );
    let sec_v_key = PrivateKey::from_bytes(
        &hex::decode("9128db9621015042d5eb96078b7b86aec79e6fb63b75affbd33138ba25f10d02")
            .unwrap(),
    );
    let keypair = crate::key::KeyPair::new(sec_v_key.clone(), sec_s_key.clone());

    let unsigned_tx_cbor = hex::decode("4d6f6e65726f20756e7369676e656420747820736574059cf89997d568e3491f626085153a5653d038a3dd2aae8e30284d58415db5ca7328ca2f6334f814970830d877c46dd0aaa3a52dc78f737138e421ebbf96cea5ea659aff47209e1d445abbdd68638bf682e3db19992e67a304b42555d50c1779ff7a451d3c065f52a708f6544d1ddce50fd8698a93ecd99d830f9e1e9e78520a36bf02f5d3eb67589c374d076ca94b2da3c74b0caeb08ad5437bf224daa7fdb2645d469e2d9c98a05d5b80d3986cbec51daa6a558d02f692d3a57e21e0a8b94c626928d53ceab9053707c1decac435eed05f76f85280db4e58f8bf29d48ae8900753a4b49cd2af83e168c27c1031b559cfb9d6729fc33cb7146b2fd3030a5570056012972accde2da4e76d370296c26c4da7ce148a38a72bba59a1eb40dcdaeabbd72da6b9e89408390816cc8c0961792a48a6abb7b3b3ebb6ede624efb8240f31f9167df34d4d2eda89a4492120a11dc7aaa1d204ff2be44e510efc49d535e2c55679128bb1e3a8778ec417c0a0a2ae131702a85ab95a0612e707a03d394856123fd36a283c67e80bfc5626b7abb25ef8354c9fd28ce89a389e0d17e813ea82f27c15458b966cd68ba81c900eb1f62509596be947720ff9b258b086472f61d88865c2cf90894ad11b4864b6de61c664a9cabcbc5dbf03d6e2d677a7f058da488b0ed27656b30a91f5adfc6c0a64086d2bf8f08bc6d788044c2aabd6d6b8fe1eb80b6c38a1e00950be78f3fc036f849b2a85fed92d0f36d3771aaaa7dbc04812ce7f84c37cdbd2583dc43068cbcb581da13323ace4a74ddc7c633923d4ce02c07ffc728921cf3fde40be73e3ff6c1a8879d6e0b3e273e3c8aaff9b41e18c4fc809be6e0526ce03e97f3b9a0acc681418d1d38c794588acc88e5510484ea1d95a4f4ad49599d628629e6c8f56e9a2377988a08f8c72bef26ee608fa1e48dc6dacf5125e157de828f916e4d272b4215ae70cc52b754fe58051f759769f0669def4d8e22172bde071cd5b66539b112bd0fcafb847a7f113c2a2ef556e444ea80be16b37ecabee8f8e6e861efe6a405e66a122f1f20143348e8b2863501965769e9e3563bc5357c7e443ff11f551c8dc9f1f94292f049f1ea3a5c12e023edadf265e72acae7ded9f955ac5cb7d7ae832ef0c46f3843d5ee5477ea4d3d79962753b2c3ceb13c97cf0b9f84f4cd8a1ef43df88eaac68e00bb93f1f3b75826778ed1a6a7b5aae35ab9cfd7f74490a73f00c7a5401b08737fd1b4b9c5b8da79b0cec9f35d94ba713c10cc7a72d1b880ed22d790ef31753400ee56e78ca48e44cec0f62a80a8280141bb3a2a60098ab11c86abe35c7ca52495892003ce42c0f64ec1ae693cafb5a5590b2f139a7ab86a733d77a4abe0ca73a8698aba60f1666bddfc1fa05b417ac9dc77341cf00cacc1294ad76e944009ffbf9df20af37362b274acd6cf8c7385e5d019b0dbab69de8df4f18cd1eb83de528c8b147ed40269f2f0d2285d85d1e1dfcddd7eb6794d545d40db41091e2facfa4cc858f6a0dd63a3009b99d04835e788ed6262cca8a85978ae98bbdc3cc017f84e1e5bd030484549d8cc6f00427ddd901f99ab3fd60afd4cf4f8cc6cef2dffc9f01aef55b5eb0947e4a35e8ebf6b6ab2eeea68895c8d9487ad4d6124b367967cf7b791326d61d42f9a1b1b4075fea58ede9a8909fd7f93eb36f00d35531b8cd7fd644a1bccc260fa59a659728abfdcec7994fcb2353b5b01c929ff6642a69d056d45a76af597cfc528f596d19458b9d24f29857b4b6ef0d0ed94cdf2d508f6a28793021382b6271b3480e940cede206e3f4083734989bc29156536cbf4d6315f540ef519f37dd627812da3b4298630e4c37e3f795405d65ab8397f97a7408cc4b8b1ced9663e0a85e4b0155f253bcacf8e4c2383de9df396918a8d720168fdd8b3ab02de96bd778c1fdf9d7950e5c0175e97807e75ff8b056faca2536795b5fad91306c88aeec21fb358ef0178a38dabb2cdbb1b75c7d2ed95140960974886c48dd28c6af6fcbb9663acbe7ca06d0fae316825ae05ff450e0b991de3e772da1598947b1b0aa9456e4dc1bf61b9d4858292d31f308f555a42a263f4c9a89299a07570f13abc12eac3d2e54f3b71ff452f16f6f1988815add21434e063e16e573d52d438d885d3d24ed4af1cabb729091733fe65cd42b9d994d00aaf6f9c85b6b32079e87ff573d88f56ce52c8cac640bf2b280146779d8c5185fb5533c5de21cf917949448f7642e3982a9535ff9251da766ce529bb7d45adc4c5fe16cd54e79a8d5fe395bccce86551edc4937ede302ed7a0f566a1cfa61916f8d34a10423cc265568d20273571362c85a7c930422b17f117929655504867b4594003cbde895d2cde66cd298774378b13f24d99935362d12d1398f9df383ea0342251033f499c248b7daaf55c0b01dae5ed26d50bf25d521f86536e0b8085ae6deec2068c1cae5b911f877c09ff51377210502221cdba0c4b712c3beddb89d51f70a8b7bb337415740dddcfd831a2f79b4e137032afb99acffa9a634c38865963bebff9f73f8c1e1008151cf426f86129e024a10e5bf14013e2f2a92711321767262d0011ca692da9cd5b3c47e3aa650f3a17e1571c2ac93224f75a833aad93328756df1200113794ae752bc42b13e403e03530cd17e6dbc668e48de90072629fa689af6bb14255200349e12900d2f46ff029096e101db8341be614c1f7bc808858dfbac69aa6ad31af4f2f7ebb886c92a8f99e3d13a274033bd6f4473e4dc550158ce2f1651db6f37ccff9b5ea17215187e8f9852f7ec8938eee014ba59961b22985e76c2b3a434552bf9939923edcb41c29e82f1cc9010715272de9bdeecf23a4c5c0abb53bb2adc9048888c427116e49c129b001ee5772098bd14c40e42b49fa652889a49f7a13a7402b7278c0f5ac2d6084401f5e6dd75b50a03e1ebfe795c63a772e214ea0246fefce97b5ec90db82942aea60bac2a1ac6328491087a32d3036b0efae5554b52afa7cd22fb5076b56c0e463a36f2aa40061c5cbcbca1871c183c8f72b1f0075fa07610288ed2e2fa41c573e6257e71e090a073e11740304492854c5a25a92d7b90007de11da506a48d8485f6b6a2604b0869ce63b128be117cda3b730f3cff048b6e6f6750c58d66aea451f9c5cada819d7041b04abaee54a835f27cd33d58ef3b53c08134d2b5a4f3822d09167c3ebb99877483414e4bf616b6aad087a4a5a315bb7d264ab0825ea79b4474b5cb5fa68f9ef9749a8b8daf4ca91f66f9d5feb417f5a244be5aa3e96abe70cf1e19eab2b0c3b44cdf96222b3630d718136d9ae22948a63838dc46be26659fde59f9cd6927dff603dfd5580ccac7c56893a84b5192e0a02dff46345685eeb1e6e550d7498c12e42ee6c4344654a01b063d6eb6c7b8e680af5f9db09e9cf844e084108c9b01f9572e85742c15cfa4d0b3c2a424400a951c3df5ef9770aa0e7b298fb5d0c064a1d2ae91dcfa94f4f62b01fb0a7137885bca0e105138b82671519d63ea423a9510a2f94b41f1cb4d98ab7eff736eed19e162d9fa9d13d1d37a7a4b9adecec2a48d747e66fbf5d4d0cf0113a21c01f20e8b1e09143ca0d8df668f2a8024da015df9bc93f48ab74b91a97322d1b9ff4b91c834f7b2e0f0042a55c4f4eb3439e28d748c60c298de0a8286019c5de718f91d9d83cf36aad9f78e084f4bd809e7069d1bb7531edc125dc68894d1ff327895fdc94cbac5899e24dce4ad8d7e84dd3bbe7c974ae50462cc66d3cef7eeaed8893c2028529341a143e0020afc37d7b9ba2d7c497c337a9c9b3b0b6176a29c4b0a99ca7cfbccfbf284212d46b81e087791f26b8cd54a689c4cfed9c30a3915e771c9b4af73018b32a517403b73779f0d3de6349fa3634e2d37a9df827fa1c1a230a16fe92a835386a101829fdc6c31c5517b4af0cb79ab4d744b68336cd53754020603d9d0dc1f5b086341ed2704ceb5da2c41496de0d4749ef8bbe063af85a51e9a74e385bd3332edaff34d5d35c70b665e9c3fb133e75dd8fdd31db6d60d32001c61f5a23872fda8a6bd51e84f84e389e0fe9c211cd3c634a84f1553b0df5c64a19c1d70f8255e811368dd46cb14da39f9a5c138b95f34aa3d658f9c2c8c7594c47cbed768d0224f410a0f59a6aacfa6008249882d0b8c17e31f7f99bce65d1ab4d675f1f6db49b3e2e98863373c4ebae9c012a59860432a9854c706b3c87a13590dddb176e58eebaa0ab399a1591a46dc3f1146efb6e4597541c94a8662c265d7a5ef85bd44915164995faff7f8286e887e48e059ce1058ac4bf7dfd794c47432f37cb8c2f9f4c88edd33f5ad9d3713333ce78b1b9b71a7c8aae27c679167a6c24b5d20e1adfd546fa42c2a5a3fe04f8a64a5990f18ac027d0d269e7cecb28792afe002b8b55f1cddef56a11d53257c61d42f50b9c2e2aa60cb3f5224e7eebe92aadf773f9da1356b941c0ede7d45aeb4c7e97465d320fcd73bcdeae715a69730d991783751e8be965e9c09fcffa699072c8da6c64e6a19453176428e72143306040f9c1a0a8248d4591a44dc4e266da0a5ea09d53797deae473d7e61ab5cd3f59446a3856ed62844e1011d85450f74fb1d2e08").unwrap();

    let decrypt_data = decrypt_data_with_pvk(keypair.view.to_bytes(), unsigned_tx_cbor, UNSIGNED_TX_PREFIX).unwrap();

    assert_eq!(
        hex::encode(decrypt_data.data.clone()),
        "0201021002db8d940284f53a0edb9d0b32ebf0097876272ffd60485d2fe34a9c0f21da63b028b9b6125e40f426a90991994c59c85a61c40c04e505731cffb73479c38e5e02ea7a8744028183b337232a67eea5b53479fb530d25a260fc02a4c59c6161b9c1ac647d6a7a6e53687a147f5beff390ee1e55149a9bd0eb429f9f62530884980c25d72ece632b0a102a02f1d2c7372695952224f94637cb3fbdb71d10cfab834cc95978cc5e761f72e71e9cf7c6f49dc8df81bc594652f31521a03850a4b0e81f8886cf083b2ff48bf1770999f8530290c0a638f95f7f233123c370f0bd0d5470ba119ba8ac7ce484c489dd4a2cfc020af0dfe63cad3ec3b22928b4f76c9904763807d059c41e15c9933e3fcf635370617d7ee60293c8bf386807148a905d51c47665edd78cc691f8965470f95bd49c1b3df02d2f708dca0ec1212ed51c9436dd10eafc85a336b9eaadf8f5fc1adef3378ad41cea4aa889e502a8ee82390093d8e3d1f864357e62ba9cfe38808f104a2247360789065c4ed5145c27a5399a63432d4d7028f15f70599ef9249113e75e70f96e65dfb02803fc7e4b5a64150298fb85397d65ee891b27dab50a023885b7614c7018013f5f31f4cdf1a11c72b8072d210ddbeb7339bfe21f29504d7ba036e33c0e2d3c06c378f37c205e2d89e98a9d3e2102bd9b8739266064a09b7709dfe454559ab9010a09c9f6d2822b26c2429528fc5d1b43e3e96d683573f4a1e9cf72f860a6cde29a1814cecaa40ec38368875057a5b1d6f7f302aac687397f052d45f19104817ef25548a4144a60011f836ff63b5f7d8f2b8d76a44b2cde89e76b41d62799afb82cf127ee12679fed694c2c941076695f3bc6799a58386902a2b9883940d40079817024c5b4a6a9fd18b5e77e717edbde7bbc524e1d0a7d48cb376a8514a0d291b73052284f62976047598d13fc8489c9fc3c2bb20d731e9d3cbaa57602fc9a893929fe02a3b4a13ae4df894f2275e2553d07980a8200e22fc189dbfbb6fe2da7b2da2e03cefefb2a2d91564bb7554a4f3a6ee589768d6d2fa0c957e75943c7e3a40287b0893937655b00673f61644b4456e9e73c16bb626d7fa22c90198e20b65f2a2e2cd944c6560d8b2d27347dcd62e6c9e1c92eb8a2ae9eb239baff2c24b6fd11d19461cc02eace8939a9040dab3f68100937dd6567f6b4806e237e3bfeb5d8a07340817cfcfb1890a3765850f9a50ca1d640129b8d290ba14aeb13cedacfcfdc40d5dbc08a17d8986702c9d58939a1a694a905c3a5717d7190611e8d841bc8296c3571f7cfb08bac1d8720d72873f201b91d420d99923cda74800172c19941fd9133bb74328ddf16be99bcd5d3570290a68a392b76de31ee550fd7ef0ad4e37c8737854ef0498ccfa05478bc72a21b9385c5dcb665f04baacbf16265fb6bc6b159d3c8e8abed0f2ebff6b4f585504c9d36590702e1b88a397d151ec6427f3b684c75faeae28076d7f3bb15ea0d01305783b5c5bcab62b6fbd23acd2e28f45f328df48730fef1743b31e84597eb31c1913680beaaa79a53020e000000000000008d8c3014bd4ea82e9ee7fbeaa09cd902cd70efb54f91b538b2dd82baa031c34e000100000000000000403c9830100000000147fb8e0a6d505ddccc03b917f98b8d7e22a57312ff825acee31d78ef6571270d0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000100289e59e0d7c0dcfca82262cd168ed8c427b929316894bf5d1ce158dd9eadf5743c43e14d7b81c5fa649be784d238ff59599e19c7990c2ef89423eca2b1a3aae9f75f91bcf0291abee2e39aeae2d97cce0b30ad879d4e39e1b6e905dffd3b2585ab119fc88239266367d0f878594579818da7b7ce11410acac13cdb202548746ae0783608cecea92d416029599a536e967b239179b25c9028b101a4608466bf5fdcd89ff7f735eacae74eda7b5769fdc6dc3d15314a9a71518faccc54c51c7d196402d714ff9eb1510121f1eece9680292e0d5369d57eb6f88bdb772690d314afeff1493c1ebceef7b0fc1c895531a435d27419a0bb46d6eea776ab5c6366d1cc1f9d0b2873884a3295b2ccf91f5c943924ccda702bedef1361d2ff75567699ba24f42743dfa81358e368dba2ffb52a9f6d6482bc78170b8fdef9b380ff27f5dc1812830195740caed3dd019bfe38323b2e57b3c7e558dc47702acf0e037f10d76a690cada32899579cd07d070aa26f50e2d0509dce15ac7171b10f6ded0b18d8551609ef5d7552fb01189116958033d9a20ab978c88af731b3cd9ca8f7a0297939638652de4b442fe5e1ea3fc696d2606277becb2f70d8f280674774e11f90b2ebdbc3f8444eef36286d426c7cdb559aa356e4e4c298c0dce33cd5502a9b4929378dc02bafbf1382a81a3cbcedc6db0e9730d6d087c95cdd37a5113d690b6f187330a675b264ee9d75a2b11604ffc77fba26da820472f55501c0e0337ec505fd074c6497d14918d02e2a8fc3848e484bb994428ced2f01fefc3cbf22172fa9d072bada460ea8bf21f18764bcf4a7e2f332636208210b7d9d8053898f4400477ba3bbaebcabef40011dd2bad110282b7ff388399fdb34f9e6548a61f934f0700badf8cc65dd3888964229f40186e2bae0592229ac08b22dcef5e49b3c9f6e054ebc59a07fd9c03f4f66b9465cc755ba7efe002cca483397677219c23f9b35c59ef6378c2dc939091becee2f9bfa26cf70f93f399132e3cbe2ffae58613b1321e021875107c26cd3fc8048f65726548796427dcd2093240028db088393892710ba719a6b6327cf7e2c71341290f625f341c6170132a7c61c5df6d5cbb310751574ea8e48d90d81053c64a49c06458f1c64430e5e27f3be11cf90dc3e002e8b68939a8a914e14baf095437311a894581811115601af693ef377e3e3ef5ffc069377f67f5c979af23cc7a24583f5582096ee06e7e3ea186a9178da42c177347cb51c60292d689392780be13a7b9bdbf85855ad59387e14284565a83ffca47a59a5af5a9f8c78c7691716c7c466c20ffbe68dedcb53309ccceb78145109f07a8e7b8509e5eec14b802a2a48a39234a1901e6f86a51c2ee9c4a3c35b266f17ed615c13072c548e3d59e936b95258af74fc2fa6f343919460b056bd6d428ae9bb77726f47955463a7fb12bd200fc02b0b68a395c578dd7534f67cf71020292d94e0fcd6871778c2335f76383ec822cb5c061c54a0c567b775eb9dd0bc8a74ad4b00d390c4f8f86fe6915aa6efcb1ffce8c98820f00000000000000f9ee387ab40cfea6b829569582a0a1faac25deead9517c52e04acc2fd5a15d4e00000000000000000080e8f334000000000168af954455934531400f00668a7a21abbeb7492253a4173014a34c520e21f505000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000e0b1e7f6e00149263fae1a3d475f30d0ac1a9094598e9310b80d72a261e202665d12c45a00b04e5167244e697022150a9f2761c8c8f23f680c39bb33c3dfa12923ebbb532d340000025f383545394a42524b42746d3565525851337875393941514a386d617850717466453576755047474e4c52506d426a4e32616b385a6f6f444256624c56763473445459626350524e566450644835487262507173705a7237643253525459626980c8afa025493379d1c9e6f21bc59f5d2d3f89898b463039682b6c551d7882ecc9ef92f040271e05edf7a3003ebbbc51e6219953ceeb9bcb2390fb2464c50fa3a45ed8940c010000e0b1e7f6e00149263fae1a3d475f30d0ac1a9094598e9310b80d72a261e202665d12c45a00b04e5167244e697022150a9f2761c8c8f23f680c39bb33c3dfa12923ebbb532d3400000204052c01be01cdba7288a7b8aa1b580b5bb55a2e3578ebd5de9f5125251101ee25a44a910209010000000000000000000000000000000003000304015f383545394a42524b42746d3565525851337875393941514a386d617850717466453576755047474e4c52506d426a4e32616b385a6f6f444256624c56763473445459626350524e566450644835487262507173705a7237643253525459626980c8afa025493379d1c9e6f21bc59f5d2d3f89898b463039682b6c551d7882ecc9ef92f040271e05edf7a3003ebbbc51e6219953ceeb9bcb2390fb2464c50fa3a45ed8940c010000000000010003070700"
    );

    let unsigned_tx = UnsignedTx::deserialize(&decrypt_data.data);

    let signed_txes = unsigned_tx.sign(&keypair);

    assert_eq!(
        hex::encode(signed_txes.serialize()),
        "020002020010db8d94028183b337f1d2c73790c0a63893c8bf38a8ee823998fb8539bd9b8739aac68739a2b98839fc9a893987b08939eace8939c9d5893990a68a39e1b88a3941d84315a66787a918eac17a621d56b635c4162b6be80c67f364d7848424d5ae02001089e59e0d91abee2e9599a53692e0d536bedef136acf0e03797939638bafbf138e2a8fc3882b7ff38cca483398db08839e8b6893992d68939a2a48a39b0b68a39026a22ff8a3471fa80aec5b5eae4a65b004ac47b4c378c01e2088489dee61f740200038c71f767e255802c1b2bbbb02fece94ac14fc593ba034a3fa60e4eb266a7052dd300039a550f29a3986cfcd5fa0f61935ad41fdf632b669198c339bf8c2feffc867897252c01be01cdba7288a7b8aa1b580b5bb55a2e3578ebd5de9f5125251101ee25a44a91020901000000000000000005e0cf9915d6699223e9de6279584ef8f780cfb58b4201992be8d37c177cf3357405b49c098bed5d05cef272fab69137bbb55bc1e188e239bd05e3a493d68a07677ea27a391a23ed5c0d4b76692c5c4405ae7b730e01f31dd8d10a0070cc29028bba7368890ea50da1078adb1dd26002f1ee85331effc11755d9a602fe156b5b98b85f1c3b681fe9f041d17b67ec332c0669f6078f51dc8341d5292c8c813809d02ecf06b86395dd58f4ce259146f53aabc88ff8caf86dcea0ef7f37e55ee2048a57854ac030b719b8a4346f5c6ea16b1e08570675b67e24a38b978f2ab79c2d80ba18aae0bdf2e5a01475c9649f6632df7459bd5b0acedf20601a81c5b611cbb9d26486302527f0005104df644e8f02e850f531d7000799380c1aaf77696c3eb4d4b5c54168054b96a6b127fb8a0a83cab7f44b9c820f9fdbaf6b492ff1e553d34d930a5f5dc48c4ece65953f9fad05e8094dea1b549521f6d2eedb9fe91d38034a670d26b19ee50d6bbe069c64316b9becc06cc374f5932db331edd64a8d0e61682ff52dba7696bc871f655be9006e9d5d281e08350b280862c683a5b839ee59b9e5274ecdf3e1164250f384ede523f65f2b0abf7f42541d7c4978eb6b254a9fb19e5f05423e470d0a62480fbe281284d057510af7ddd24a8fa513fd9a4c928d28f93a968094f0b61a3132e4d391ccf546805645831d0760c67de983fb4dd3ad6736ead41ba6d570ea374aa1fe57da25748d299a4e7c3e69e3a19a45957cdb0af77bcfe0ac229ce4181607edc949eb4a6e18bc56eaacaac7f46da2d4653dcb4b66d8ae9a2f9325d64b8e4bc203cdf537e067677339390b646ba21ab9f3edee7457c0b906fa4b45f45fb22ed349898d83e34d74b2902c419426ea15264ef2ad28a9a59a27df62b3406f4a293f220191f4ef1436fcc6d5f647eed4a03781d8bdb0396e67364c0ce76f4d208566d0d390e193059a06a895fd0e0eb28d751bcb4ac92aa4b09cccb9c0db628432454ad894769c4c4125846b5f5ef3d1050cd869df2304b92b82707aee18eb7c53b568a03ebfcd2f7dcb729302b8510cdd29005e6b24d2d5ff80b4658be1320372b2a0c435054a3959eb0275041da14a4dee73f455b1e8c15a4979803516931763011376212832cd906a687e0cead302e340e3df60cd22e12fceabbacbf41f00fb8707874b91230fe1ca0cbf0211f716245738fab6be319af1d952a80ceb837528fec594f076a3c8aad2835e0721abbb4aab91a65fac0030cd8cf021b70399800177fafe6b7dd6b8520903820cb5d2e5fd9c76f68d25d26148355d0b74b2804945d15daa9ae539c4777a24f90260287ac51397fe52b89131aa36ffdd44d04d8961bf2f6cdaf377c44c09340b001bc49fa1ba9c4be88d49bed1a0ed20b0ce3529ca055746513534a0b154d581025878689807f2042a89ef79ec81a75ac1f0478ec308a15917b9b936f64cfbce07583029efdbcc89d82de0cb1cea7733f5a322990f4bda22a009070bca6f34a802655fdecdbe84b04dffc18d80212068901714c40883a4c1f190490f2e5573e103dfb5d45af63b8c15bba4681bf11467dcc2ff1e3a599c90c72d4d9a1731836e02477493d1d4c25f4e3d2329378a70190d01139a74720d6fc71190075ff4cc670da25c94bc5fb902be9d20c026ce28e603586cbf6de89db1b782ff4f8e67e87106dd42cf999053fbe049d5db94244a5f4a652bfd45ba23509f2408a5985165e905f18a7e3fd20551e079ef837977bab8ffe2b0a6e089dc7efa157fbe1406f20304d491ef57ba01205ec73efc946f208f0bb615097ea526c5652f079cf3b9980109eb62d399091aa88ff1c61981b1ecdd2958820291bb8a0b62f063eb634d939a0e947b5097f378fd4b1ee32818d768426bba1a080b278164b3adbe0b9adf8cd0032ac00c604bd8c7d127ed401adba186a928931212576109bb48c4f08bd7ea42a576b29a1e0c8a8a25dc2bd7e2ab6adb93b8318957aa3f2d7d3994fd79f801f60b3e78ddce5ae3e0744c9bb2cab5ad30b253d052337cedaaf350e6c974ce7c93060782dfd4b7eadb82f1e267c4c8e90dcebc590d2dc0e97abcffe6f0024799310237912cc90837ef30d0fd464b4e1dec1c763d1cd4fa7e54c137e51eea1afa91068893f329ec4dcd944425b999401dfe5123fb1d27a7277d4b2e8282433d0d250765699a72e70cc70df900c3549c01887f7d8efbaff31e2236c41a134cdfb4a5044e49be4557bc050abb1bbbc4d1bd3cce8e1d968d7d0ddf960e1773b0a6ab7e0a86d90725bedf39d48c1c9ea8103a203014dafd66be0d5e15d85f9f44f69412044386b67f219d0d32723f2a528266d43d92b453ba6e27b32e0771a56eeca8820c3cb25271a432eb6032d3377b2028f9f189032af442496ea81a56bf47a502b4092f40b7d577c7f27965df225aba44bfe954fe1ef54cef0beb673e049eecf3980180d23a2361ab21e60897ad4b5ea4755c662b463ce2415ffb13de7248e1b9460a0318a68ec862eab6c2b1473bd176627aebe743da4ab99136c2a1aca2c1971806f1aa28721d52eb01d94dc96dc42b06e4208c433a2c73d685815c1d01c35e010e38e68932c60bdb6af3ec7c80d6d3b1c7cc47a7b23e46923fa8589c91874723095f1a447c1ae43f15c47a4fd0fac67ada57e616b78486a78c5809c3bbad283b0b33696179302040dadff7f22bafccddedfea5d8620405f41b8f1e9a6c2859b2090b25e03ba337967b30a6650a397f3de6c054469db23f2cee89d3fa4d58a27584bf28bf40503d6d67d591dd025029c81184ecd6fa3a9c11a10b19c998635b4e2069b387132e4422cff54674e1be5803cba775f711a9dfe13d9ad281e4735a89d9"
    );
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::key::PrivateKey;
    use crate::utils::constants::PUBKEY_LEH;
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

        let unsigned_tx_cbor = hex::decode("4d6f6e65726f20756e7369676e656420747820736574059d0387bc6ceba5911fcea00da365d4b98c2d9e534c1c9c877ab61b5582c6b4554bd9ac8dbf9bd1fe82ad248eada917dfad4f91be9cd7b71a295df6f2216ef0a6a40d151f8c72f78a4c4abc2b07c70a5c42a84011c331c4e31d5b4e3dff6689b343f1b6963ab55c2418ce6e63121253da44c323a8636b96d6e876e2cea060048a72e49eb856f034252641f61a71523e4fe1ac36b926e1bca823a8676c7d84b8264013b71db78a54ffe95c546e65609ae502f74e90ddabfcda0fd58018fa30fa370c6f18a88cc9623083140eca9f148bab7289972cb33332c8adcd17c3ab187cee448e1e10822931c0b72282c7ab257570e1c035553573936c57f3f77a6f00e228bb5d6d9deff90e371768183e8a113a46cff6b8b2ba56993f5cd8e47971e252a625b3364c49ef2730b0660ee6baa676f5b24c0db949d309f353237133757191aec6c0dc44c6fb00d1a87682e65dc0681645b06510053d46d61a9761b48106815e163d35080720015d0b030bac0f108aab860530c5af5fe9f2d110ff8b82e6e2dade190ab5a3db7c104fedea59e54beaed2bde183881b4257cd575cf71c248398787d0d9930d1a380d203d09e8197692ed3eddefb1f9f233a3b856509c2731bbc9c7b5cb709f92eb157118ccdd8e849d51ff8cb765781f5afb3fdd6dab9554bb21b00a01b7685325094bc66d0632013c31a467462f7894d13690bd22788a15fa7f5a47561d8e3d90f600b932417b6b67fbc8347eceec86b14dd07b6665b63bdaecba2cc12fc98b7737ac065ee2087ea6c4df381e6fe8ea562ccb34398966435f85b878418d8b6cd2d89db0658d3a7f0664b728ad13f8f5fea7a5eb4be0fc82bb5ce658bf3e6ca2883be9408a2d1740d62c658137b6591e8c0bf4b319c66a3722c621b3dd0fcdbd321a07e7055a3d824a1560cc95e64a87625325508366a98006bbf6990d7dbc35836f712cca3d67e6aba44db6cffdb929b9cb2c5680b5fb3de210d62be16f2749e83b36735a2e1a832333d2333209e7f86c22bbc2591e098f3d9b70d788398ef3e540f4a78708402c8272b0a068e78a8f1335c8ab1520104e3adf28bdddd8ec6e72896aa117362c61873542b3e71520262a1ff17cfa73e1d5da50c898b89a67e2a61be327132dd658c68164842e44d00e20908574768e3fa8202a172c43e284f3e893592d004c1a8adcae84fc69e564bc96b9590300fb932c641d70905877a3c5e6223698983dfb90c30dc7b7e8dd67e3bfb9b31272e249c2f0fa39e609db2ef1496f89f8b02d42a2cb76ded4ac7b8a1503276cbacb649c65bd87a853623eed1cf5fceda8b3acd3ca1ae05d4bbd5512e1d5d6fb4b4948980b06f6b3d5695620f931ea5e736e50c20c0b287dda41a52cd479be821b85249c3a73d44d938ef62bcf861e470f2c31d31659890d43bff9392f34deabf26feb481d88334b5e0a5ff5c0af48ee71876cd9745142b6b7b42648ddcd280f9da12351b2c581cf3aba4508e0628e3d811f20f6e8ef8feaea2a518cf02c2b65abc032e57729ef419e347b2d446f465be6afde3123ee4ac7b3840e508d2c4f5bb8c7fff867c6452707d8403effcd10dbde6dfc4badbb0f67addd1c9c426cb1056e33ca090ae370c6e03d26470030796f851e40eff77d4a014a8f116bb13caf30252a5d2def43e7f6116b313a3595d9be0c91ba71cab9fcd842bf43248bfe1808cbf3f545b064ae475f0949b76c84e83089854022cc1e11de5c280725c1cd51d4b4f2ec89b5ae7e1990fe9f10a78cc775fb4048cb0bcd76e98ce747431c280825ef551a5918cc98ec5ff19b7c3afe45773663e38e874b8f81ba23c4f2c201e28d0787d4e52d35b74a71852cdc46305009f708502636386a7bff77366af8a6c959a3470e7892e44c6fded4f4f2433b0dd368591e5615b556edd0c14063a603e7fca190dde6846c2b4377eebb3e4575545764762130f58c3d8295660fd2c87c81f9c18a9a8ec77634c22ff993bbd9d666b0cbda0014acb819fcb5f108a34bc7bed3237577344a11588d301cddf013ac68714ad1a61790bf2e2d63f99abb3f6784cd31ee54bc7174de07583103b2aef04a3fe3f310b691c088e221a1ce6597b686befa961a1957e2945b9a763574084f14a01102e30011e7eefda6e47e012e5f0c167c31fe0355c49504c3ea259b70c7ae0bcbb4d8cf872c04b4b9f8dd3ed15725e343e81cbdd9b2005d8a5bfaddadd63e989dc30d1c4dda857763244463d0b56f066370700f63cd0e5893b3aa63db2fa15865ce959c31cb8cfbb8974dddfd2b7100ea49cdf1c799c46fdfb64f54c79001c3df18613b5a536a52103dc9bc20148ae0cde6f4ab6a0d1d5c2f5e2aa48964c9c607bbcd9347292a65845bf1886fa4ed04aa6e0537d8ccb838701a2c6c9fba653433346944e3c9feb40a108be7afbbb428496a0bc60c876ae5f771cf84f0a1cce77b6961b27f518bb3c6895e65f5aa06dfe844a24103bd657d706a6d7128c14cd4e5f2c49dfe5720bc41a1138bd109f9b5568efeac2b1e0c784fa0736cc8123119ef2aa93a6ee7ccdfdf569c653999867696d2e01cb66eaaeb05f95f15d822d0b0c7c68ee74d7518317818b61d1f696b928d570780fb4c9768cf2ae7de5f3ca0c0d142da60d91dd8e24808f20fe07d9b8ccb9013fd3b5563a8b22a24423e8828787353df5f15e4f0c9db48e162df30d1634d1e48df122c7df975c2952d6a87aa583241ba5a7b72d2f910721156b58ad0314e1c5da4bcbb2c16b5ef6b0900f810f4531f0ea8c64c4d247d217e94db9963c471615863f4c669103a0240920c2045d015aea4738ac71b4dca91497fe4ca13b24bcee89f02d4a6c25a8060cb066d137693bb7b560bc2591e8449cfde1d39d0ce164e2839692f3ef5e7fb6cd7a0d1315a26675f54e8f901bbed079648912393575afc0c32a5c6eae876bc4238c41d860e7eecf3ce4f78142f994c6e224d5e80e594d65f74b63a105f98a790bdcc888937534422a6523fa5c7091168f33843ce19ed0dcf0ad511cb105d185ab97e62ddd6bba96a23dc0ea4da49a0735f919514c960074cc8ed4e7557aa035337430bcd31b0469d19bcfc43a560aebfbfa623dc47911116fa4b1b3dbf1c979788c5ea31cdd949b57b5bd2929a83fce311e5ab49ac50e7e81098856dd3105c18f68080b859cf1da842d6d964d2b3a5378ac09d7375710250d3c95e6d1f720ca8d6104acc3bd9bd09b6649acf039fc2cd35d7c032ff55ecd57bdb924b0d10ffab2e87d0c9177535fe493d8890760d675f36e9320af04dcf4a503ca15e126fb6f0e3a964488643f17623b6fa3ec29c6f5148a2646853159dc992fd30ed1ed65471ee5f185f7cf8da9432cafe704e0bfa19f999a50442f67ee731d813c50972d2a58bba1034599e2643a8898d69287aa47e8e7d961ad4bc4ae594980efa61a0201e40307d81774ccfa8d6c1f8fa24060b29813e15c836ae16df366faa417e4597c0909b4debcf273efdbcce50e456a6e4f6a609b5fb233826f981ae0732802da637cdf9d573f65ee79497dabb2faa79283bec7061b0345ccaf03f10fedd550abcfb259101837486f0da01208ced031c54fe0730904b87680843965637006c444d8e725c40095829700fba4c40968bc0984203e4269a601d740c5699f4b3e181626bb1c2a5c75d735c30d00280d3fb7d8480c5a92d7ec2659e051ec0c511d7f3175d60fe0a801abe691c3208766c9c78a2df62027edda04c8a348e0cffd1e9c9bbc1e3b996f5ebaea2794a3e7065ede1620d1223930d04f146772a1b821263518f22e9320bcf44a294214df453b2535b70e007bf685a0f2aa6f6240eafc8717bee26ad6e3adc523f442338ca7c0e277c514af9706c30a34875cdc00bdf535b0072b31ad98dafe4ea0621c6c87e1d9df174bb040655899f7cc2e3e5239abf2e9d386b978fcf13c49751d8f8825535440dbeddd68d7531478e0e6fc18bb7570e6389d09897b44ea45581d85782e0e46ef2506143a3e9f6388eefd0ec375ee0fe9bb63aa16ca897513c04dd5499191a436c1a2b45e23d2efa39b3aa3a558abc7cf41af5a62428454530b64aa0d77c3d098fb3cf00a8b8436887067eef721c9ffe4815302053b7bc6fbbc389da2732aa4fcb4cb99223004c308fc9cbeccb0917d324e79adf090151d2c3569c1b5e3de3df9b6b8b538f7d5704d164bda89220969a2806c583dc4a088becdfe48aa3e2e5db0415529e0fd8c8265ae9b1031d994d7b0ab0e459d791bb0c7264f8ab0a9ba42ede4403c01e2a12fb7b58e8907b7b2d75864589a0bd74f13f0bfe0e4888e6c345ffd1de13da7f5ed2eff84dd18fb9df542b4aa2e46ec0b6a8d3e58138d2def19f6462e2178eb1a160ed23b4c164d5065257160bf251d7a9cf35d8bebeee3f1d1d85b844adfc8ba19db2ba300503834f6bbcb4029179bec40f55d1b4584f49fbf61f2e4c2aa47da0e5523a1de29769d83fc8d48d063f2fc8b246589cc0fca2ec56b72ebd7acabb0f3c58e948b96a692e2550951061fb99e240c8789513da95f7f3056aa793002fa7990e4def0ece0f263239d1d48cd7a129701f72f3a3961d9d927a8edccbc8e6f6455c86dfbba37e4c3c1d41dc2d6f57cacce289ed7f0bca2c2c9bd2e0ab7a79f7b754cf4fc94f4013190c318f4ac791bfebb144e5189199de0b2acb2d95dbcc8d66438c521beddc8b92a6e0c3214d49ddb658e84fd79a59853ac8965b2ef1f06a6428189506002c1592e31aaf8a01abb82cc91f7ffc0d1c505f0370ec6c63ad3fce0e8108b987c5468d4f7d4951d3f30257b6c4e8f9f2e101f33db054e8f2140d553cc907d8a9e9654c0b4fe6152a511ee9afc92c85ee0a4f379c4f7348f8c665e3cb35a6c82e7167641b5178ceef78809595bb1269c8a4e4efdc462097bf08f77873528fc41028cdb54c2bdc061c57e1fea05934376e7ccff40a0f3a93434d5e15f5b3dcfdf0ddce37d23db1b801dcbdfa2f4a04ec5e29c01ff560920761a2ad0e67cbac64970caa0a7bfa5a1917a52ccd7bfff73ec2c25c67a7f81fead14d4a9469d286350e6b884f7458b734337d03718f0a9efa036e31d2f6fb0936819adcacc1d8452112842b9b85e44638c4c5a1549472346648719cdb54fec6bf7ecae6005223fc240d84e4038cbadad1662d71f41261f5682021db2e34d43e9805f337ee859cb4f1c2b5b18c9b1417de773de42d271194f983716975957478f4472d157bfd6d8571cb07296ec3f4ac201a914fb99d1b9083ae770d64baa13bb0f8cb21e3f08433caa478ca4f05b06ba383dab6726d240927073d022a398e894d13a565b56f179909b351f5a53a5549cd0d40b9b036bb0e731d538729175a438f01fe06c28e3c460dbfcdc897e91576285f8d60e8c872ff1a0f038ec5482d786b87512bf701b359bf61c0b64a8176149e1a1b87330c2c9f5ccf0564d5c09eb2204ed744e2ed48f07933c28401d1be95cecc701e1aa642510202cdde49cff9c6203f6024eae51568b79b379b2630a8ddc514c3d52f8035785773095977f4537aae600d88faa3c109f2d249faa1b0516fcbadbaf4b8ae914d73e69bfb8414038f66c284c0e39a7650d8d4757b8d07d724017daccaa2b21921b3d9dbee096374a8730faded7b0c6855db59902580d314e054ea99e3f099073b4bdcd530a58cd1b6e1d0ad4ac83ade01faa8d43a43ea6666c87a438b0d529638d3e3da04231455a8112fdd7e53972f1ba7fe8157b2e041cae6b826ad89ea3f4d47809a311c68be08fe13b64c958fc88c7431c17023ee7d06038a014b339095f2dbbec3ac8f73ee76ca2eaff5fee5826489919f646e52fd5eeb542a5989cc86e3ceed24109275841e891760433d15e9d9562532088e577b32181c238ea9ca2056990f81bc6e3ffdf69bceb57301a698696cd2dc8684bb2e6fdaa44edf4e5f808d2cdba96abedbefabf7b202406facc46f0c1ad58410122ff4d605b0be981f029719d7fd328669f9d732f899eed35e31b3bbace0ceb34b08673be81548a2d8a61a7695b88ea8386c9e9f0e8803d5497dfa69c4532e3a4fb029d71d8a0dc44afa4105175187d1140e124b8239539312b38f46f7219ad15ee66a591ace32c9140099e7b41cce692f12dca77c6b39fefa8b6a0041567bc96696d629c662cb4ab594ee2c53ffb94a1cf203085544310f26d8f52bf7a57683274540858ae582a1ca7274f641bd413426d557e8d70afc54c1888d19b06848d0beecb91637ad6ece1a37031529adae2cc66fff72613cf91518fcc395fd3236939624a60f2c28c843ae138deb8032e996a14f689e175106b92bedd955cc003c39c16483514d6970f4ef541985bff8b8ee1135e434234bfdc830306ae4391ba2434455ef74f7fac5372511c8226b96b6b23df0775179ae8fd0b3827c6b9be81e08ae6b75a428afc5c1e2eef0ee0b160b118eecfacfc4ac2c5658885e81b02bd239836921a14be4b6c52346845805ae7b43875de4d08de2836af7ab4fa44b034fbe24ac11042c48cdc0455f72ab35190174441b246c93a10940720b87d7f005").unwrap();

        let decrypt_data = decrypt_data_with_pvk(keypair.view.to_bytes(), unsigned_tx_cbor, UNSIGNED_TX_PREFIX).unwrap();

        let unsigned_tx = UnsignedTx::deserialize(&decrypt_data.data);

        let display_info = unsigned_tx.display_info();

        extern crate std;
        std::print!("[display_info]:\n{}", display_info);

        let signed_txes = unsigned_tx.sign(&keypair);

        let signed_ur_data = encrypt_data_with_pvk(keypair, signed_txes.serialize(), SIGNED_TX_PREFIX);

        let ur_fmt = |cbor: Vec<u8>| {
            let mut ur = vec![0x59];
            let len = cbor.len();
            ur.push((len >> 8) as u8);
            ur.push((len & 0xff) as u8);
            ur.extend(cbor);
            ur
        };

        assert_eq!(
            hex::encode(ur_fmt(signed_ur_data)),
            "591f994d6f6e65726f207369676e65642074782073657405a9bfcc44d2528037bb31d2c93a4f2cd0cff36965002476588f76642df482e6144729d9ab9816368967580f3af3fcf28d80004cd011a2d26a6cc04ed2b630913333b54d2cbcf34eb95f6577f1c42a6e7083ad978d6cb9a40de0f979a57a8657e197e8471449199416b28512209e5301666281051d1f1ade38a6735939a3bb1adb10d6e68bd0eb77ee8f874fa18201433fbc305d107062776b7ead71a10a25c592e5a64c36366ab762b132d04337c2a0f1f441eac9ac77fe563e0924ae90d2f9c7378bce7e4b020fb8ffd7da4e7b720d7c391bdc6f15dafa6655eb3e3b5dd4da647283e771298389acb133f180094d5df6884b86bc82672cee12b1199b645a0429c14c904b400c3f4c4beda872346ae0264cc1a3e3d062354f6d5c5762a55714413245045729481af7ac49a512619682a28d65c46e1aa3b0dd8d152f20b10d857edb2e09e92fbdafcadaf1635ea514ffc02b1df950da03a2ac00b56b321114f23bc3eab557b72dfac5bcb1aa59b473f129bd8790d4a04547cb10e8255cde62fb18b155da1c7cd2c1542d2f6389123fd416ca6eb91a63a3cdae5e2ea6813f542570dcc985bc44b751475968d2cbbe0a1ab7b546e6e0f8bd41a492618d0565bd429f8e22c2cba569b77cbdb823b786d10629852aba52db067f07fc6edef7de6b1398da775ba3aad4c9b28907d741917f87ca8a52020483d01b7aac71cc918a2dde2a2940a385c93a18dc9411b984865d2a9c0837b03de26b8b60c8ef556626ddac4f1c76dd3a50757cb15257a2b86d117d013baeda21c4178ffade5975c31fa041989b45781fc426c39106561b1f9787b8c183114595b043392a3396e7011ced03cd8e952461aeb4ed477fe6013178bd8b87924aa2a8b94c28435c1b6090cf983e25f72d2ed77b690b58402056352ce60caf31e03a8d66609fbe3baa35c8126107a71bd9da5736bf11d8f69e0ead0d7953c42fce5d7be82fff7033daa4a4d58d59f48bbe8329133e2312189f30847e36290021197cc020491cd195531ae02cc8becb3d5e1dbf49b6b0112ae370c17c857f1b015e7a81a3acccbeb98ab05ec8fda3cc2ca69a31afe69b6b11bcb352e68401015f45ae61639c3ae5e459c7cb8b9ca6273eb8aadb5ca2bd7d1bb1aca9649edde7bd46f6f44096e707fb7174e74d19c5d9337fa2146ad624a256d5a1ef98055db03d5cc014d73c3644d917c6687c333aa063bbd9413f69bf9264ed10b00b10bbe4e504fc0c02823118b0decc98e4d594280c57d6bc2c914e4768ab0d99f0e86adb98d9186feabe0ec576c8164820b237e187a8b77a16a24a6bb90e7476329429cf56f30fab2561f6d4fb4dbee9215ee90fe4255367f8493076670c49b317139db697bad867420c7cbbe46b4f10376c50d2c33e6d446f99df765650f53d627245b4b5a16c1406cc67998d756d81f93f6f0bb2fcfb298ea5e40f7f3d81092d922c8bce14dfbb0a3cf94c800641afcb6785a0d2f19131c91bd22b7db8b055e7aff9b1cb693c1fb3c667d3e6bb60143abdd51585ccf562c814d45096a420d6749b8cd4e9ff3df78c025f66dca4a67e1ce5a9ee471283de8fc238382a827b416bafaf05789424e892f6c1647b9b216a455b74fa79ff84260984a10cad7369b61435cfd69295462e32707e84d1c560d70b0fa2c636e0f8458ecb2ac90edc6ddba09bb4400826e2ded3c303ecbaaf3bf3700cecb5554855c8aea2cda6a2d35ecfc320277b4a8938a3b2b964336ca492c99ebb97ca33ec527c4f941c4b9763e562853b81dd39e0dd8f561dac3defae8cba6e59f89ee563d1eca1764df1ed14fceee75a52b6d3f7974d7842164d34ab4287c1f3d8557cc186b37bb11956a2cf60cc7294e7c04ceadf04b5a5e4621b8d20364e5093d708c3f9c2f60a71dbf3b85066fae42d3842eb64dc6602b31ac446425cfd22dbfdc02278de4825c8a5f4c1f7137d84e19fa13edd2919f107b5793425438420d2bc220b245f9a6d7b40599e16794dc3958a0a0d3a4ac4a2c755ce6911a8f664ecc443a0a9301115b46aff6f967bd5e6438cd2513d9d21f2d313ece95fc12921693965e3076ba38af0d576d8796db763aa1b9686ed4e3a029fb5dbce64f66bbb4a19d345bde534a44e0dd4057c91ff48a35de7ceaeef4e017b656d03919300a7ff4bc72d946032aaffb6ed02a042bb98bdb66fd4b0aa65eba2223937b7e341e98075d2dfcf673df110b40a2cf70f002cf2dbf0cef0c818ba1a86bd7fc30603c24dbbae57dcf5741fe02dcdf2d3aa382ff23049837f63cc81a6c32ac2bbbe6c67069eb3264398ffd8953eb6e269541c54dd4f5bbc2a0af5fa951500bb3bac228d6b139854bdce102136c35b1c6f79f00f806ca43187336536f68eda8969570ce48241f7f7eb5f1a87d06e8129854fec39812cad7351e0da54cb79ddd0d0a18f8b0334bcf2bbfa5c4799a7a8d8a0ed8418bb83d90981f2ccef8f2010e7f95fb20fe3588ea5d08c7fd276a435109e1632931dd236b9c1f1b841a467c93d8985a48c27b51210b10557598ffd5b153ccc1613d7e33452063818097298274d7dfdab172bf916f1b2188d5e611788d6effcc90ebeeaa20d138b7fc4682c069403a9aa8495a084dc0ae5e1ede3e0ae73ae07007f186ec0ed039604c3208aeefae3907144831dffb60595419f4a264d8fe8e68d0844f2e35682e590606936397d91df559cb04d61c6d79d73b9518184b4a8254848d5549f9cd056da7fcd8266bd32df4b4952b662d0ee29dbad329b91ccdb2a8880b23285492dcefc700af1525039c03113cf92175d1b418d4cc09a915c854459290396e28056816f3601920e7424a449d469abd2e8310f28d909777d5fa2b3b25562ea96ce30a546825ba7df87f16136df9457702613775173e0c5a260e94fe9a0959259ab8837c06ae1be983a3059a575d33de97ba10e6ab17baff8cb50f81d2e282e1c989ce6297ddea6cfc33b865963750c84eb54e4e4ec4295d999e7ebb64d5ebe926e1957754794f7051e9d42668c456ca1509d0aec6a54f60595a2a1975f2148579a52a5cc08cb4e5b0da3418a81bb275a7fe4ae700299c49918af0c624d3232f22f8d47bbe3530f00ff9307731c81ebde0a6c5b1812171d11d77d80e886736699aff702c8e12c1e82a633889b24b6a09aa86c4e90f258a1d6c803618b0a6f80ea892a71062f6416119a926fc750357293f58991c9ec700adca182518a2bc399636285c40015c3b4281757a0a05dde2181b66135a194c550a7ea862a9c0981545a7085346af69f037fbb0d4bdc282d73cb72d530fd772d895def2b7e35a1917bfb3ee78ca315f8c086aa95b0f897746ba676974c4a4d993605e911614e64239577af0d9b0042c9f4ee1cdbf5cadb92f748ab571de5f3f785dd1818d3a902061b9d6502f3d2705d1540d5431402acbe29090bf0cc2fc3e41a1a43936144392b17f61891fa82b7c1a55f3e771836c446b05401c06dd7410e1608bcfd761cd1ee77c7774ff9427ae64dea13d11c4c33843e489774c4f6a5ce387b56e8f7313b714f8024f732d118428f5b5bc650a1ea8bdcd6732c78cc53f4fa50ce2e5bfb3cda0e6629184eb9da2658babfd3df90eeaf08cf70239cfdfe2992ceb2a163ef244cbb25fda6f1cd4460be8b549849e700303ce5627697f67349f50f8c26c4483ffedba9ea161482f2f648de32a58eebba1fdf962fbc057ff15337b95dc56a4900458ac2c5001972e2236812a2e86d4b07a87c3095666cdcf9194e2a6f68b90c1bba4a4a75f32896ba69ae1574914b89533a74498c338139c0998aff6ed8f67f8a3b9b0cc5ad330e8eaccabd2bfb6f050804f3eb7817d26906061f5559bc5adeddc9e7209abda09fda3b5070c64fe6c27db032009a08f7ee0089ac4f0fb7b0e87445d440d367e9441a3ffea544405123330fe51224c69386454aad25a74da29ac3a57a8816ac93901b5c3fd3dc98ff1335d94394f12a5e40291a72c85dbac3afd958a0da6ad20bae55c6b53fc4b96e5e46bfb018e7101979e14d61d5e1644f08a018eaf686dc48b68de7af7b1da1f28730f849e5b02c32049581af0eb2f191e361be0a8c8e6044f64553130d37cc743d2aea779742bff6c04bc964540af6e115f896a9229359e1a5fc7e6c18212dc8d982a6f6f1604ce9c5761d6496fbc39ddeacf402d96d9baa15e6406debbf52ba7764c4bc8f55c4bc6bf93ec3c97cd6b9277eb133ee0e2046f0c08c8c0606c2e735ca3f53d0ba245c196c8566b47ee9348669f600196efc877cfeaddff7801455e83b4fea77f3d4c7b902d573ea1e5b9bd68d33f691d7ee710ef4826d73a58ea7a8e767f16a62f64c647a7dbe1f4b54136dd7e49c53a8ed01687bf233bd89aa806ffd316432402026602041cc6d42fab2936205e050598400475303c7b139788fe650749db3e7f6b520d17c1e1ccce25bc7f3bdb9d8199135a29b255121cd924ad6b9341ee75e01c64f3f606654f720d3dfd9a6af98bc19865f1adfe88a4385f4a111c920b755ae28d40712fef5a4957e2e520564f44d11848b097416efc2ceb82782f3380720da9a2bc6e44c1378d2d25ca7fd7d00a212f0f10e215b087431ac6f3d77d2499d5eca2ad322c1698fe422be0d1baf25e15646f438a2616781f3bde2950a646c43e86863b8d0883ca06c711755726d545405aaf5e1f698579836c249e7cf6e7f63c1343920ba6bf00326866b239d0a5d68359cff0633bea459a667b1ddf09254494944b1462f6dff74dd9b3216c5613209037d418cd6e21c8159b62502acdee6cdc6e0a855159011ddc3a1f87ee0d1b27224d7ef2cf714210a2b0dc3e9f0268139dde9d8e56bd99ca56438b3c601219fffa0910f5897d026dc0bf79a79173048e597f91a19a4880096c143c7bdbc6e862784e8b6be4759fa528871b7811656aafadb4b2b2191b9090e0e3b6d133ce3cb27d386f7d3db3100d2564c6a21a046e5bcad598ad0ca7534ee3f6d1620d10719c02a16ed1f3726f15f41eb98e1e21b7658e3eb490b729358fb08a72c96a4dbc39382ed4ef862328e534dce3b4b390aa28778e335af45467457cc514c3f2e305e2b32f8d6febcd263e569fcf8d69726b3a7ebbd8b886599dd85411ee4fee4aa3097a909e4357badcb0a0d7ffe47019b0ff29ad10f1700574313adefa514dc18d740a32f32ffc28725bddb91fcb03f15b91c17f4dedc568ef7dc2e5cfdd23fe45e4713834c135936b46dd71423fad5d853e745c7fc22f752e7a3eecb5611a42f54d305c16a6cd59fb8976af704eb39099ee06b1cdd5dc8592d7862aa64bd2e12b728a0e36d66fe272319df4e20889340bb1a3d11b5cbb177d24b40da21fcabd7906783eb6ab5755a5e2f87f1b49061278b0e02c9370941530848509192a4f083ed972bec779ea89b9b2f9367ad19c8957b3b056e1c3bb2cfa60f30d505258fcde4a8fd0d41da41edd09647a6fd3e33222fb93d94d7711a6cc94f6b6aed8b622b36a24eb219035ab4d9aafe39c8cda3540b77a566b1909d002880a8259f2208569402279115df55baa21bbb2f3c6f0c09039b43e802d85a61d40ecf72a9a4fc20dcf261de7fef09ce164557cab1cfd52f8b19a1f2e453fd6e8ad8fae299626afd57652a90d23e1a021247b7cb43b08c48107fa602b5606b804bd412c929073e720f08e86c1b95f8bcbcd1f41b702c7c0aae8e61c6e6d302654417cd3d52fa31f78975c573472be3c711b08ce3b5b2a9a34c1d9dd5e40dcb4f81782822f40c603dd3be844e3f02c3ad88b6c8ee154b5ecd164ce773508994de1e02c79059a341cc80253770fd974f82e6e90f7e992a89972135015f83632639c7a9632ddaeeb7bab1ad226a9cbfcc73299f212a0adb28ae9b5af0f2c4dd81d00c0234d8f7e7afc0a055d7238c4cd9aa06c01905bd88df4ef1c7f7447c02c04f01c4cebc5be32eb448360f7407ab2f01ec2540ef5104075794a1b7cce46daa7bcdfb1616f742debab0571dcdfaa92e4e0f7d581fced9335babc3651ecb6d4be201207e23f4db893bdea17a0f595c9b742e96e6aff384b29e920a2ad37eab04231987648cadee6e31c61389b9a095fffb4cccc0d14bbbe1382f13afbb401f80fd8eb9da1427e77429647487ebd253bbc5ac41b666ee262951e5a6e3d840362c59ec3d497108cc8a43e694fe06567eb1c57b05326b65bacae05e279517e2169d8f53d7881cb4619cc2871c0695496b9aaed0a3e75102397e1299e80fa6dd511fc874726d1f7f8a7aed323db1f20634e89a909873b582bdf05857441e7033a837f78e782451683aeeb635ce70b5f2d26d13662563819fc3024226505a892809584cd96fa9fab304f419e171b0c1ca1f38301ff2d3697021f3841780c2aee0878ba73b2e973ff5bdeb3fabc2c8ca3eca8c6bf9f6671b06d3efe26b5791ee042e40c373253a3b2a585257aea1e3d1d383cd9affbe73ae6aab210d801eed6cfbb5c33d4c2f58a6e0f6c4031f21b57b49b49c24b51ffcb0d554833b965372047e5a1eae8e37e0e4dcfeb44bb11c68a36c7c5db1982fa886acf109d62b87c2e988423b3349e53a2462e1ae691b67da17a83f49301c8e03a9e562053ca33b3319a47e56b84c23f1a5d3022cccd3781a1387b94832277d0296a17a204faa9f5bc665370f6f3b189c5bc37ed16c3cfb1ecf5d998134b54039e0f6427e4fad8024811990c7eb6947f71f1fb62195b784e9cf9c32718d55e44285f9c736129dd6fe0b27795cf68f721a11ce73639023d008809833add0acc2618973a39df24d0724038174c4811d6596ab95ab380c90ee9f20bb8eaed5078ca5bc681d675c33ca284ed1a4729e6510b819f54a3a27c534cf41b13256efbf6444ef0e0f9107ec7c8b3413f40ea627315b27e17c5454b36c3ba291fc9c866edee1a36d49bd7f4d1342158b8a702a583f68cb193c2c972262a28a97b981e1c30ab11ec646c31a21ad57951b3410c67b0fe6980bad4cefa3ec150f1be4b6a7ea1b37a9553836cbe32596ffc6703f91252dbc9efe807ad390efaafd5cacc78a3d8e98a9284869b0a58482596ba0a56c4b01f160ed4563e50dedd9e7b0b2d7f62c47c2f3d6346aad77eab36bee7f9dfdcf6173e2dba256d5d1194c4647363cb7d645abd39fce8df9d7aeadd7ed9112f82be9b8c46af46791d493be958436047c1f81390a7013d2f4ba4288e8686c927cfa79aa00ef3fd65b8864bbef1be8cb6b0eed507b899e05f7d40794bfc2c072b4ba1fb9a907f727b1308c26fbfccbf640ba0c3abf69e8ddf11bfc8a2a1f9329bbb73cb2d28e5f1e9a1e30e1bb74637a28dd036ad1dc7cabb97210f25356af260f8e259d80cdf969e7dbc5b180b54e45127d30ef3cec292459f44eebf77c8fc692e88c88e39c89184408abad5381a55aa7e40927e0c24064fcad77b343d2dd793704bb60847f4c3493a4b6e9ee934ab7f46cce17c214e8dc014fb6f4242159784059168144ada5e8474ce2614f04a6aba7b81b76fa39a15ee8fbfe1f6b93518cbd455770b1da0f26e30c84122f92329aebfc760406976835c558801d13cb53098dda54378f2a6d9ce6e9e37fd21b1926e983e32cf0f9b9fe30b2241ae437af12648dfd0cb2f4f88b0910f9dac7a94606ec714382aa42850910f71adb4106a3812be4f8afeda12de2e8253f286a0028a996911a722c9bdd88ca2d25cd463bb5708bb1bc14247381194ce8be941828fa4152878efc377625c15973a710698a9b02bcead2edcb87db2af080f7319decabf8bfcab4d7c3fa68c00e29cd01310b7ba63e9d675412b563b4a80c212e88c8af43327d790b95bbb88d6728a6fb571885dfb98842d995b3cbe283c082b38bf2a0f78a765d3ed6f9d3d1647766ea943134f03976256641fc3f2961686e1eaba4dc961c18e1baaf6d015fbc7c5d947ed877756ef3c4c3a41452140566b38ea29fc43edeed4ba229e426b98e49efc1e5f7299a6000216ae6fae1de33419626dd5042965478907191f6e61e469a76d46e100189bda7e02e1acd139987f77547b44fad18ee8f0d26538ee4d5c74f6b7ee5c97fa4c714e825f62cbdb18e7ef8bd281a028936b42206025b85d081ff7662dd37235a662e9c85da9b71af03ce74a45edd6faf9774485acc0c4ddcbca07744bbfd194323b68bfa277aee5545390e4469f9e35b6d2d94335801f80f4736d7004e7e6cb27808590220bb1a4f32932f93cdddfec5839d287801b169be2c9ee710ce7ba6ee1ff624d1296b6befade597a9246d561d4ad2732cf22354b7b698f08622a713a3dd95d2ad4ab50e31b797eefa6975e7e84ca8439a80b2e950de3f643becd7a37374b18c8dd4b60da7fd4c7afca7b0d8ac4f32357789603b0882b1c1d76a633cd5357f707c1ffcd6855f9150b734eb2667042048bb8e8a4794ce0f8ed38ca615fb87ec9df8c3043ce8de41618e3b3ca7ec546a00fa776c2e25bd98eafb4dd32cd31df6e7d01678f94e8b7c52b05a1b64a400a94570656942e9fbcd45c0398817b4b9e6162f428bb4ef3e390c0332027539fb3ecf969cd9e8a8b95923b42cee6e4c3bce4351a2737b87f759d08d09b3ce74b67c7b4bc76cf90ad14a73d5a2b9d9ecedc40bc7639845555bb3baf3bc195fcd2d147730cbae71b39fe0f323c950ac56e1ff51576c8ecaf24e9219a1b211a23c863455cb609305e8d0d028ac1b23864249052e8496c55d599f24719b5adcbd0ed05991cd47d593cfcb88fe0fbcf94ceff971252ba003633db13177ffb023cf6e8e88500e8cb6eebf6781aa8f05838439b94591dfb445be8e20fd5a66db4c110e791bb19b3639bdb2af27c9549f6f395bc199aaa1739aa8d7ecfe1fb13f9693424137d394df4cf6f1c6b24209b4547c761c13a71943ea48e95ffcbd1ca5359866b224886c6794a40a4d7b89fafab917bb3be68d1d35a96def479d456ed84b0790a89f29371702cce3c341a51cf7e2edc56065561e3091925d6f62365333589cf375f48ea4f602c8dcfb43929423034d5a9eb20a391a33433f0e193be74eda76caeb22c8d5b645d24866ba65a697b759f7723b36cc09cc13f05a67244ec4b0df29d7dab223ed3216ddfaf83b0f83c34a4dda3275ed7068782380d8121d977f377e61a213beb56b3b5cc350bafac6a75358a8ed3457c6195cd47d88f101b9bc58d25fc9503a7b5f60b51e92534f457d173c8c8faf8c3d81448b47817565a48d622772e757336cbb2653b3e996d4ec280aeb64ffe01171c56679990e972b8df3d80232b24b1e172665d3e8c7f07533cc5308f8e06ee17228ad716fcbf1739745cfedf63d3fc9c7ee729d08750bf2d11c63b13c8c927dbad7bf6b2c02e690a4fa4c7d253608c8abe9974060aa4c5e0dcad4e9ef2118d26306c31745f0f225af1ff69fb29a060c61a0edf7c7660ec1006ffb458aa1dbc35453b8c5dc3be05824358efc9e2677fe2f70c6b0b31d636d208ff3d256dd0f27963905295999f807a021d42e10cd235674f0406aec6977a853be0ebb65dc6efd5361cc22adaf7aaf259673e9b9aedc6d324c6a976204bd9368e70ec4280a975d804c945a02c90949d8de946e569675b7653600cc9dbb37b804af4486ac01eda4ba850c40857d0395cdd0ac53e895b02dc82152c1af4a2d6da4e2bd342fd781d6a9c0c284004d34a6ff941bca3fccc943ef5c7410120a6106443774ed3c8ab4265991cfbdf0adc56176e9662a640a53ef9649434a57ab8d94d0bca8023cc52fd016a89a4a83f5d6c56c87c205ed8a52c44191971798f6d322f4c29784abcee7386720f64d3f8312444ab78e53d79f53116df4887326e23ced1eaebfa21b1d2cde29681a2d6e2ebeffd93f055152c3cd58ed721c9443be49a5efe0d13629e206728e3041ca2df58b9e629d3b01516820736caae25c2ab84a234c865157eb341d60b8bb549fd0c93b2aef749da0d40feb6dfdeb372d662a96a4077bb8316bcfb118b830aa57c6cb3332a5eaeecf80b6c3cac5cc7e8f8c817d339abe4a8937e8e141d5519b61b2f5a775a0334891ee62b37b70accddaafadb6537dfdf0067d074a4b17abd394cf69010698bf33d2f4423725f2d7fc5292fd4109b7f698377a792f2bc8d47f325670afef09c0e47fecf54253c04680ad375b6c276a219a8b0a462b397e5244cfbbcf5da3d98f22b325455ccf553325949f021e62f9eb6e7ccd9c22d22c4df567d17e4c2986944879cc0c9425136f71f120533192707574911891f26b2cd421c3e13c5f43a21679664530b62a8406f0bbf90beba29f2febdb5076df815c4b98947c01971c7c30f6fb7985fa2e64054a70c46efd8f7270b1066a77e91031c32652f4df587529e36b21ecd44cf72034c255fdca189832825f242c7301e827007ddcf9d0c8ebba9604e29e44229a4c6bc92281bf7cd210fb9897aca6dcb758e26a53faab5132910348dd09f3b7db58567ea1e15ac8ebbe1fa095aa40586e7db5953a65ede3f4b005cf9174baad6cef325657992a59c995a898c853c8c78fad3474882da4f1747c75aafa5d6ed2902c146dfd6dbb831a2dbdf88661e79449fc7bf4a5ccdcc3829280beaa458d113e3bac82a7acdd08abd5fa3e9737ba1505915f2b5288c17c8e411fd0e2e5e08b97c7a528cd1f989e14862b3c6a6dc5542b56ab8fc34789cc10d6630b5cf21c7c30f687bfdf2c552672976c3b6191c627f6d155f54e1d87b18e954232d0c935bcc455c25bc1a67bbba60937042ba1d933a1b4de1b74666b5064c3506e63dc5931ddd604c2d040ac7284c952e5e4e9a2b8c827aa5fb942198d36d8144be04b6cc4383dabd7be83be51c60257eead7619bca10a7ff262bedcf7f56614e049c0a148b952f2984790459207b1b4937926640abafef90538f35a35110409f6645465a27b1b8f27b4132c923a9ee3e97aa13f705952e86ec3de49c28e741cd2706d262bb366e08a4e1171b6a86d1798227ddcc15df225225826a7d29212e42211f87803c95a1d9d38e413253a9b76f14eb646b9a3000c1fce4cf328963ec8c516c756d910dba927d2695d48975f5433d1715372c744f248ff4dbea4a578b52af30a8d3fe17b22c872a6c020c1d1ea5d3eefebea3b9a65aa5880a1aced91d1ff51fbcab836a1d6796f5ac30fee2a10526534ea556fa32f04a7632211966069168d95033a059c43ba74e11cb8c98b463ba1c2cdf4ac0a7e53ff1c7c182b06fa9b1c0a6f7b8b20d14891eb7925eeddbfc10a793f89d5acca99b64c738be596b4bc70345fdfa302a63a7bba6dca78c604c67e57eee20bcb84030bba3892c77b995a2c74bc38203da8abf3bdcad1c14746e4eece484a3b1d4e52c1086cbcee4d21ff00465d698d40112c798c8208cf0e66a648632c46c81ed4dd4d93132453094cfa2e7b668f51f0c90a6b43f4664004"
        );
    }
}
