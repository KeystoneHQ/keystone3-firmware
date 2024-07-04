use crate::errors::{CardanoError, R};
use crate::structs::{CIP36VoteKeyDerivationPath, SignVotingRegistrationResult};
use alloc::collections::BTreeMap;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use cardano_serialization_lib;
use cardano_serialization_lib::crypto::{Ed25519Signature, PublicKey, Vkey, Vkeywitness};
use cardano_serialization_lib::metadata;
use third_party::cryptoxide::hashing::blake2b_256;
use third_party::ed25519_bip32_core::XPub;
use third_party::hex;

pub fn generate_vote_key(account_index: u32, entropy: &[u8], passphrase: &[u8]) -> R<XPub> {
    let vote_key_path = CIP36VoteKeyDerivationPath::new(account_index);
    let icarus_master_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_icarus_master_key_by_entropy(
            entropy, passphrase,
        )
        .map_err(|e| CardanoError::SigningFailed(e.to_string()))?;
    let bip32_signing_key =
        keystore::algorithms::ed25519::bip32_ed25519::derive_extended_privkey_by_xprv(
            &icarus_master_key,
            &vote_key_path.to_string(),
        )
        .unwrap();
    Ok(bip32_signing_key.public())
}

pub fn sign_voting_registration(
    path: &String,
    unsigned: &[u8],
    entropy: &[u8],
    passphrase: &[u8],
) -> R<SignVotingRegistrationResult> {
    let icarus_master_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_icarus_master_key_by_entropy(
            entropy, passphrase,
        )
        .map_err(|e| CardanoError::SigningFailed(e.to_string()))?;
    let bip32_signing_key =
        keystore::algorithms::ed25519::bip32_ed25519::derive_extended_privkey_by_xprv(
            &icarus_master_key,
            path,
        )
        .unwrap();
    let signed_data = bip32_signing_key.sign::<Vec<u8>>(&blake2b_256(unsigned));
    Ok(SignVotingRegistrationResult::new(
        signed_data.to_bytes().to_vec(),
    ))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_sign_voting_registration() {
        let entropy = hex::decode("7a4362fd9792e60d97ee258f43fd21af").unwrap();
        let passphrase = b"";
        let path = "m/1852'/1815'/0'/2/0".to_string();
        let cbor = hex::decode("a119ef64a50181825820248aba8dce1e4b0a5e53509d07c42ac34f970ec452293a84763bb77359b5263f01025820ca0e65d9bb8d0dca5e88adc5e1c644cc7d62e5a139350330281ed7e3a6938d2c0358390069fa1bd9338574702283d8fb71f8cce1831c3ea4854563f5e4043aea33a4f1f468454744b2ff3644b2ab79d48e76a3187f902fe8a1bcfaad0418640500").unwrap();
        let sign_data_result =
            sign_voting_registration(&path, &cbor, &entropy, passphrase).unwrap();

        let hash = hex::encode(blake2b_256(&cbor));
        assert_eq!(
            hash,
            "d2c40028745e3aee415523cc492986147d39530a9bfdf60a15f54f1c023ce266".to_string()
        );
        assert_eq!(hex::encode(sign_data_result.get_signature()), "6adc7ca65cab8d2a7e4a918a95cde7bfe0a0f07c5a738de7476defe0389778a8708cb31c3f39db80c486532cc7437a4c5f299e9af2ce2f468723f793c5012609");
    }

    #[test]
    fn test_generate_vote_key() {
        let entropy = hex::decode("7a4362fd9792e60d97ee258f43fd21af").unwrap();
        let passphrase = b"";
        let account_index = 0;
        let xpub = generate_vote_key(account_index, &entropy, passphrase).unwrap();

        assert_eq!(
            hex::encode(xpub.public_key()),
            "248aba8dce1e4b0a5e53509d07c42ac34f970ec452293a84763bb77359b5263f",
        );
    }
}
