use crate::errors::{CardanoError, R};
use crate::structs::{ParseContext, ParsedCardanoTx};
use alloc::format;
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

pub fn sign_tx(
    tx: Vec<u8>,
    context: ParseContext,
    entropy: &[u8],
    passphrase: &[u8],
) -> R<Vec<u8>> {
    let cardano_tx = cardano_serialization_lib::Transaction::from_bytes(tx)?;
    let hash = blake2b_256(cardano_tx.body().to_bytes().as_ref());
    let mut witness_set = cardano_serialization_lib::TransactionWitnessSet::new();
    let mut vkeys = cardano_serialization_lib::crypto::Vkeywitnesses::new();
    rust_tools::debug!(format!("before generate key"));
    let icarus_master_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_icarus_master_key_by_entropy(
            entropy, passphrase,
        )
        .map_err(|e| CardanoError::SigningFailed(e.to_string()))?;
    rust_tools::debug!(format!("after generate key"));
    let mut utxo_signatures: Vec<([u8; 32], [u8; 64])> = context
        .get_utxos()
        .iter()
        .filter(|v| {
            v.get_master_fingerprint()
                .eq(&context.get_master_fingerprint())
        })
        .map(|v| {
            let pubkey =
                keystore::algorithms::ed25519::bip32_ed25519::derive_extended_pubkey_by_xprv(
                    &icarus_master_key,
                    &v.get_path().to_string(),
                )
                .map(|v| v.public_key())
                .map_err(|e| CardanoError::SigningFailed(e.to_string()));
            let signature = keystore::algorithms::ed25519::bip32_ed25519::sign_message_by_xprv(
                &icarus_master_key,
                &hash,
                &v.get_path().to_string(),
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
                keystore::algorithms::ed25519::bip32_ed25519::derive_extended_pubkey_by_xprv(
                    &icarus_master_key,
                    &v.get_path().to_string(),
                )
                .map(|v| v.public_key())
                .map_err(|e| CardanoError::SigningFailed(e.to_string()));
            let signature = keystore::algorithms::ed25519::bip32_ed25519::sign_message_by_xprv(
                &icarus_master_key,
                &hash,
                &v.get_path().to_string(),
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
    rust_tools::debug!(format!("after sign"));
    witness_set.set_vkeys(&vkeys);

    Ok(witness_set.to_bytes())
}

#[cfg(test)]
mod test {
    use cardano_serialization_lib::Transaction;

    extern crate std;

    use std::println;

    #[test]
    fn spike_transaction() {
        let tx_hex = "84a60081825820b779155710df9bc90946345e5813132e1169f63e2405ea67d6eeafebfa3584d7010181825839014b892f4424343e546ecd3fae5bad84d403f8405a842b4f4ca65beef9a2803a2739522d725d93ab5ea68a76ac05c18ff7b12d468e6ef871701a01041c06021a0002a35d031a065a61bb048183028200581ca2803a2739522d725d93ab5ea68a76ac05c18ff7b12d468e6ef87170581c62c337b473a2e2548ed3cb76002a56bc1305bad95c26c6efd89085af0800a0f5f6";
        let tx = Transaction::from_hex(tx_hex).unwrap();
        println!("{}", tx.to_json().unwrap());
    }
}
