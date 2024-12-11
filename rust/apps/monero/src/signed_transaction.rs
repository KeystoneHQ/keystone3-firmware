use crate::key::*;
use crate::key_images::Keyimage;
use crate::transfer::{TxConstructionData, TxDestinationEntry};
use crate::utils::io::*;
use alloc::{string::String, vec, vec::Vec};
use curve25519_dalek::Scalar;
use monero_serai_mirror::transaction::{NotPruned, Transaction};

#[derive(Debug, Clone)]
pub struct PendingTx {
    pub tx: Transaction<NotPruned>,
    dust: u64,
    fee: u64,
    dust_added_to_fee: bool,
    change_dts: TxDestinationEntry,
    selected_transfers: Vec<usize>,
    key_images: String,
    tx_key: PrivateKey,
    additional_tx_keys: Vec<PrivateKey>,
    dests: Vec<TxDestinationEntry>,
    multisig_sigs: Vec<String>,
    construction_data: TxConstructionData,
    multisig_tx_key_entropy: PrivateKey,
}

impl PendingTx {
    pub fn new(
        tx: Transaction<NotPruned>,
        dust: u64,
        fee: u64,
        dust_added_to_fee: bool,
        change_dts: TxDestinationEntry,
        selected_transfers: Vec<usize>,
        key_images: String,
        tx_key: PrivateKey,
        additional_tx_keys: Vec<PrivateKey>,
        dests: Vec<TxDestinationEntry>,
        // TODO: always empty, unsupport multisig
        // multisig_sigs: Vec<u8>,
        construction_data: TxConstructionData,
        // TODO: always zero, unsupport multisig
        // multisig_tx_key_entropy: PrivateKey,
    ) -> Self {
        Self {
            tx,
            dust,
            fee,
            dust_added_to_fee,
            change_dts,
            selected_transfers,
            key_images,
            tx_key,
            additional_tx_keys,
            dests,
            multisig_sigs: vec![],
            construction_data,
            multisig_tx_key_entropy: PrivateKey::default(),
        }
    }
}

#[derive(Debug, Clone)]
pub struct SignedTxSet {
    pub ptx: Vec<PendingTx>,
    key_images: Vec<Keyimage>,
    tx_key_images: Vec<(PublicKey, Keyimage)>,
}

impl SignedTxSet {
    pub fn new(
        ptx: Vec<PendingTx>,
        key_images: Vec<Keyimage>,
        tx_key_images: Vec<(PublicKey, Keyimage)>,
    ) -> Self {
        Self {
            ptx,
            key_images,
            tx_key_images,
        }
    }
}

impl SignedTxSet {
    pub fn serialize(&self) -> Vec<u8> {
        let signed_tx_set = self;
        let mut res = vec![];
        // signed_tx_set version 00
        res.push(0u8);
        // ptx len
        res.extend_from_slice(write_varinteger(signed_tx_set.ptx.len() as u64).as_slice());
        for ptx in signed_tx_set.ptx.clone() {
            // ptx version 1
            res.push(1u8);
            res.extend_from_slice(&ptx.tx.serialize());
            res.extend_from_slice(&ptx.dust.to_le_bytes());
            res.extend_from_slice(&ptx.fee.to_le_bytes());
            res.push(ptx.dust_added_to_fee as u8);
            res.extend_from_slice(&write_tx_destination_entry(&ptx.change_dts));
            res.extend_from_slice(write_varinteger(ptx.selected_transfers.len() as u64).as_slice());
            for transfer in ptx.selected_transfers {
                res.push(transfer as u8);
            }
            let key_images_bytes = ptx.key_images.as_bytes();
            res.extend_from_slice(write_varinteger(key_images_bytes.len() as u64).as_slice());
            if key_images_bytes.len() > 0 {
                // res.push(0x01);
                res.extend_from_slice(&key_images_bytes);
            }
            // tx_key ZERO
            res.extend_from_slice(&Scalar::ONE.to_bytes());
            res.extend_from_slice(write_varinteger(ptx.additional_tx_keys.len() as u64).as_slice());
            for key in ptx.additional_tx_keys {
                res.extend_from_slice(&key.to_bytes());
            }
            res.extend_from_slice(write_varinteger(ptx.dests.len() as u64).as_slice());
            for dest in ptx.dests {
                res.extend_from_slice(&write_tx_destination_entry(&dest));
            }
            res.extend_from_slice(&write_tx_construction_data(&ptx.construction_data));
            // multisig_sigs
            res.push(0u8);
            res.extend_from_slice(&ptx.multisig_tx_key_entropy.to_bytes());
        }
        res.extend_from_slice(write_varinteger(signed_tx_set.key_images.len() as u64).as_slice());
        for key_image in signed_tx_set.key_images.clone() {
            res.extend_from_slice(&key_image.to_bytes());
        }
        res.extend_from_slice(
            write_varinteger(signed_tx_set.tx_key_images.len() as u64).as_slice(),
        );
        for (public_key, key_image) in signed_tx_set.tx_key_images.clone() {
            res.push(2u8);
            res.extend_from_slice(&public_key.point.to_bytes());
            res.extend_from_slice(&key_image.to_bytes());
        }
        res
    }
}
