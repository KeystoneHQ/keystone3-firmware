use crate::errors::{CardanoError, R};
use crate::structs::{ParseContext, ParsedCardanoTx};
use alloc::string::ToString;
use alloc::vec::Vec;
use cardano_serialization_lib;
use cardano_serialization_lib::crypto::{Ed25519Signature, PublicKey, Vkey, Vkeywitness};
use third_party::cryptoxide::hashing::blake2b_256;

pub fn parse_tx(tx: Vec<u8>, context: ParseContext) -> R<ParsedCardanoTx> {
    let cardano_tx = cardano_serialization_lib::Transaction::from_bytes(tx)?;
    ParsedCardanoTx::from_cardano_tx(cardano_tx, context)
}

pub fn check_tx(tx: Vec<u8>, context: ParseContext) -> R<()> {
    let cardano_tx = cardano_serialization_lib::Transaction::from_bytes(tx)?;
    ParsedCardanoTx::verify(cardano_tx, context)
}

pub fn sign_tx(tx: Vec<u8>, context: ParseContext, entropy: &[u8]) -> R<Vec<u8>> {
    let cardano_tx = cardano_serialization_lib::Transaction::from_bytes(tx)?;
    let hash = blake2b_256(cardano_tx.body().to_bytes().as_ref());
    let mut witness_set = cardano_serialization_lib::TransactionWitnessSet::new();
    let mut vkeys = cardano_serialization_lib::crypto::Vkeywitnesses::new();
    let mut utxo_signatures: Vec<([u8; 32], [u8; 64])> = context
        .get_utxos()
        .iter()
        .filter(|v| {
            v.get_master_fingerprint()
                .eq(&context.get_master_fingerprint())
        })
        .map(|v| {
            let pubkey =
                keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
                    &v.get_path().to_string(),
                    entropy,
                )
                .map(|v| v.public_key())
                .map_err(|e| CardanoError::SigningFailed(e.to_string()));
            let signature = keystore::algorithms::ed25519::bip32_ed25519::sign_message_by_entropy(
                &hash,
                &v.get_path().to_string(),
                entropy,
            )
            .map_err(|e| CardanoError::SigningFailed(e.to_string()));
            pubkey.and_then(|_pubkey| signature.map(|_signature| (_pubkey, _signature)))
        })
        .collect::<R<Vec<([u8; 32], [u8; 64])>>>()?;
    let mut cert_signatures = context
        .get_cert_keys()
        .iter()
        .filter(|v| {
            v.get_master_fingerprint()
                .eq(&context.get_master_fingerprint())
        })
        .map(|v| {
            let pubkey =
                keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
                    &v.get_path().to_string(),
                    entropy,
                )
                .map(|v| v.public_key())
                .map_err(|e| CardanoError::SigningFailed(e.to_string()));
            let signature = keystore::algorithms::ed25519::bip32_ed25519::sign_message_by_entropy(
                &hash,
                &v.get_path().to_string(),
                entropy,
            )
            .map_err(|e| CardanoError::SigningFailed(e.to_string()));
            pubkey.and_then(|v| signature.map(|_v| (v, _v)))
        })
        .collect::<R<Vec<([u8; 32], [u8; 64])>>>()?;
    utxo_signatures.append(&mut cert_signatures);
    for (pubkey, signature) in utxo_signatures {
        let v = Vkeywitness::new(
            &Vkey::new(
                &PublicKey::from_bytes(&pubkey)
                    .map_err(|e| CardanoError::SigningFailed(e.to_string()))?,
            ),
            &Ed25519Signature::from_bytes(signature.to_vec())
                .map_err(|e| CardanoError::SigningFailed(e.to_string()))?,
        );
        vkeys.add(&v);
    }

    witness_set.set_vkeys(&vkeys);

    Ok(witness_set.to_bytes())
}
