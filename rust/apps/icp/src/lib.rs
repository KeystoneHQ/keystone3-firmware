#![no_std]
#![feature(error_in_core)]
#[macro_use]
extern crate alloc;


pub mod errors;
pub mod account_id;
pub mod principal_id;
pub mod types;
pub mod principal;

use third_party::secp256k1::PublicKey;
use alloc::string::String;
use alloc::string::ToString;
use alloc::vec::Vec;
use third_party::secp256k1::Message;
use crate::types::ICPPublicKey;
use pkcs8::EncodePublicKey;
use crate::principal_id::PrincipalId;
use crate::errors::Result;
use crate::account_id::AccountIdentifier;

pub fn generate_principal_id(public_key : PublicKey) ->Result<PrincipalId> {
    let icp_public_key = ICPPublicKey::try_from(public_key)?;
    let der_public_key = icp_public_key.to_public_key_der().map_err(|e| errors::ICPError::PublicKeyToDerError(format!("{}", e)))?;
    let principal_id: PrincipalId =
    PrincipalId::new_self_authenticating(&der_public_key.as_bytes());
    Ok(principal_id)
}

pub fn generate_account_id(principal_id: PrincipalId) -> Result<String> {
    let account_id: AccountIdentifier = AccountIdentifier::new(principal_id.0, None);
    Ok(account_id.to_hex())
}


pub fn generate_address(public_key:PublicKey) -> Result<String> {
    let principal_id = generate_principal_id(public_key)?;
    let account_id = generate_account_id(principal_id)?;
    Ok(account_id)
}


pub fn sign(message: Message, hd_path: &String, seed: &[u8]) -> Result<(i32, [u8; 64])> {
    keystore::algorithms::secp256k1::sign_message_by_seed(&seed, hd_path, &message)
        .map_err(|e| errors::ICPError::KeystoreError(format!("sign failed {:?}", e.to_string())))
}


#[cfg(test)]
mod tests {
    use super::*;

    use crate::principal_id::PrincipalId;
    use crate::types::ICPPublicKey;
    use pkcs8::EncodePublicKey;
    use alloc::string::ToString;
    use alloc::str::FromStr;
    use third_party::bitcoin::secp256k1::hashes;
    use third_party::bitcoin::secp256k1::{ecdsa, SecretKey,Secp256k1};
    use third_party::cryptoxide::hashing::sha256;
    #[test]
    fn test_public_key_principal_id () {
        let public_key_bytes = [3, 59, 101, 243, 36, 181, 142, 146, 174, 252, 195, 17, 7, 16, 168, 230, 66, 167, 206, 106, 94, 101, 65, 150, 148, 1, 255, 96, 112, 12, 5, 244, 57];
        let public_key = PublicKey::from_slice(&public_key_bytes).unwrap();
        let principal_id = generate_principal_id(public_key).unwrap();
        assert_eq!(
            "7rtqo-ah3ki-saurz-utzxq-o4yhl-so2yx-iardd-mktej-x4k24-ijen6-dae".to_string(),
            principal_id.0.to_text()
        );
    }

    #[test]
    fn test_from_principal_id_to_account_id() {
        let principal_id_str = "7rtqo-ah3ki-saurz-utzxq-o4yhl-so2yx-iardd-mktej-x4k24-ijen6-dae";
        let principal_id = PrincipalId::from_str(&principal_id_str).unwrap();
        let account_id = generate_account_id(principal_id).unwrap();
        assert_eq!(
            "33a807e6078195d2bbe1904b0ed0fc65b8a3a437b43831ccebba2b7b6d393bd6".to_string(),
            account_id
        );
    }

    #[test]
    fn test_from_public_key_to_address() {
        let public_key_bytes = [3, 59, 101, 243, 36, 181, 142, 146, 174, 252, 195, 17, 7, 16, 168, 230, 66, 167, 206, 106, 94, 101, 65, 150, 148, 1, 255, 96, 112, 12, 5, 244, 57];
        let public_key = PublicKey::from_slice(&public_key_bytes).unwrap();
        let address = generate_address(public_key).unwrap();
        assert_eq!(
            "33a807e6078195d2bbe1904b0ed0fc65b8a3a437b43831ccebba2b7b6d393bd6".to_string(),
            address
        );
    }

    #[test]
    fn test_sign_from_seed() {
        let message_hash =
        hex::decode("0D94D045A7E0D4547E161AC360C73581A95383435A48D8869AB08FF34A8DB5E7")
            .unwrap();
        let message = Message::from_hashed_data::<hashes::sha256::Hash>(&message_hash);
        let hd_path = "m/44'/223'/0'/0/0".to_string();
        let seed = "8AC41B663FC02C7E6BCD961989A5E03B23325509089C3F9984F9D86169B918E0967AB3CC894D10C8A66824B0163E445EBBE639CD126E37AAC16591C278425FF9".to_string();
        let public_key_bytes = [3, 59, 101, 243, 36, 181, 142, 146, 174, 252, 195, 17, 7, 16, 168, 230, 66, 167, 206, 106, 94, 101, 65, 150, 148, 1, 255, 96, 112, 12, 5, 244, 57];
        let public_key = PublicKey::from_slice(&public_key_bytes).unwrap();
        let (v, sig) = sign(message, &hd_path, &hex::decode(seed).unwrap()).unwrap();
        let result = keystore::algorithms::secp256k1::verify_signature(&sig, message.as_ref(), &public_key.serialize()).unwrap();
        assert_eq!(result, true);
    }


    #[test]
    fn test_sig_message_from_private_key_str() {
        let priv_key_str =
            "833fe62409237b9d62ec77587520911e9a759cec1d19755b7da901b96dca3d42".to_string();
        let priv_key_bytes = hex::decode(priv_key_str).unwrap();
        let private_key = SecretKey::from_slice(&priv_key_bytes).unwrap();
        let public_key = PublicKey::from_secret_key(&Secp256k1::new(), &private_key);
        let message = "0a69632d72657175657374957985b77f030ee246db6a464dc8c90bac5e50a40da8d5a2edf27ef6a7a91806";
        let message_vec = hex::decode(message).unwrap();
        assert_eq!(
            [
                10, 105, 99, 45, 114, 101, 113, 117, 101, 115, 116, 149, 121, 133, 183, 127, 3, 14,
                226, 70, 219, 106, 70, 77, 200, 201, 11, 172, 94, 80, 164, 13, 168, 213, 162, 237,
                242, 126, 246, 167, 169, 24, 6
            ],
            message_vec.as_slice()
        );
        let message_vec = sha256(message_vec.as_slice());
        let signature = keystore::algorithms::secp256k1::sign_message_hash_by_private_key(&message_vec, &priv_key_bytes).unwrap();
        
        let result = keystore::algorithms::secp256k1::verify_signature(&signature, &message_vec, &public_key.serialize()).unwrap();
        assert_eq!(result, true);
    }
}