use crate::errors::{CardanoError, R};
use crate::structs::SignVotingRegistrationResult;
use alloc::collections::BTreeMap;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use cardano_serialization_lib;
use cardano_serialization_lib::crypto::{Ed25519Signature, PublicKey, Vkey, Vkeywitness};
use third_party::cryptoxide::hashing::blake2b_256;
use third_party::hex;

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
      let path = "m/1852'/1815'/0'/0/0".to_string();
      let cbor = hex::decode("a119ef64a501818258200036ef3e1f0d3f5989e2d155ea54bdb2a72c4c456ccb959af4c94868f473f5a001025820e3cd2404c84de65f96918f18d5b445bcb933a7cda18eeded7945dd191e432369035839004777561e7d9ec112ec307572faec1aff61ff0cfed68df4cd5c847f1872b617657881e30ad17c46e4010c9cb3ebb2440653a34d32219c83e9041904d20500").unwrap();
      let sign_data_result = sign_voting_registration(&path, &cbor, &entropy, passphrase).unwrap();

      assert_eq!(hex::encode(sign_data_result.get_signature()), "1a86a1bd46629be41c104702ceb34e8012b6a087d711a82e72d676978a4aeaa474fa547ec59a654d18c2f2aae39c1dc3bc6d9722214d9260ae4d53e6e7182e0b");
    }
}