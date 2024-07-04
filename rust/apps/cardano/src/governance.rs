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
        let path = "m/1694'/1815'/0'/0/0".to_string();
        let cbor = hex::decode("a119ef64a501818258200036ef3e1f0d3f5989e2d155ea54bdb2a72c4c456ccb959af4c94868f473f5a001025820e3cd2404c84de65f96918f18d5b445bcb933a7cda18eeded7945dd191e432369035839004777561e7d9ec112ec307572faec1aff61ff0cfed68df4cd5c847f1872b617657881e30ad17c46e4010c9cb3ebb2440653a34d32219c83e9041904d20500").unwrap();
        let sign_data_result =
            sign_voting_registration(&path, &cbor, &entropy, passphrase).unwrap();

        assert_eq!(hex::encode(sign_data_result.get_signature()), "08266b498ff85bfae97f8d50a6e356f231f683bbdc7573c3726c8e521735315b3626bea99b20a6f31bdcff3d00009c800369efe02e43774bd47eed4ce99bcf07");
    }

    #[test]
    fn test_generate_vote_key() {
        let entropy = hex::decode("7a4362fd9792e60d97ee258f43fd21af").unwrap();
        let passphrase = b"";
        let account_index = 0;
        let xpub = generate_vote_key(account_index, &entropy, passphrase).unwrap();

        assert_eq!(
            hex::encode(xpub.public_key()),
            "4254bcba9304e951c348c3bf1d6cfa867b5f452c05e3c9d33f4517d78a069520",
        );
    }
}
