extern crate alloc;

use alloc::string::{String, ToString};

use bitcoin::secp256k1::{ecdsa, SecretKey};
use core::str::FromStr;
use secp256k1::PublicKey;

use third_party::bitcoin::bip32::{DerivationPath, Fingerprint, Xpriv, Xpub};
use third_party::bitcoin::Network;
use third_party::secp256k1::Message;
use third_party::{bitcoin, secp256k1};

use crate::algorithms::utils::normalize_path;

use crate::errors::{KeystoreError, Result};

pub fn derive_public_key(xpub: &String, path: &String) -> Result<PublicKey> {
    derive_extend_public_key(xpub, path).map(|e| e.public_key)
}

pub fn derive_extend_public_key(xpub: &String, path: &String) -> Result<Xpub> {
    let p = normalize_path(path);
    let extended_key =
        Xpub::from_str(xpub.as_str()).map_err(|e| KeystoreError::DerivePubKey(e.to_string()))?;
    let derivation_path = DerivationPath::from_str(p.as_str())
        .map_err(|e| KeystoreError::InvalidDerivationPath(e.to_string()))?;
    extended_key
        .derive_pub(&secp256k1::Secp256k1::new(), &derivation_path)
        .map_err(|e| KeystoreError::DerivationError(e.to_string()))
}

pub fn get_private_key_by_seed(seed: &[u8], path: &String) -> Result<SecretKey> {
    Ok(get_extended_private_key_by_seed(seed, path)?.private_key)
}

pub fn get_public_key_by_seed(seed: &[u8], path: &String) -> Result<PublicKey> {
    Ok(get_private_key_by_seed(seed, path)?.public_key(&secp256k1::Secp256k1::new()))
}

fn get_extended_private_key_by_seed(seed: &[u8], path: &String) -> Result<Xpriv> {
    let p = normalize_path(path);
    let derivation_path = DerivationPath::from_str(p.as_str())
        .map_err(|e| KeystoreError::InvalidDerivationPath(e.to_string()))?;
    let root = Xpriv::new_master(Network::Bitcoin, seed)
        .map_err(|e| KeystoreError::SeedError(e.to_string()))?;
    root.derive_priv(&secp256k1::Secp256k1::new(), &derivation_path)
        .map_err(|e| KeystoreError::DerivationError(e.to_string()))
}

pub fn get_extended_public_key_by_seed(seed: &[u8], path: &String) -> Result<Xpub> {
    let prik = get_extended_private_key_by_seed(seed, path)?;
    Ok(Xpub::from_priv(&secp256k1::Secp256k1::new(), &prik))
}

pub fn get_master_fingerprint_by_seed(seed: &[u8]) -> Result<Fingerprint> {
    let root = Xpriv::new_master(Network::Bitcoin, seed)
        .map_err(|e| KeystoreError::SeedError(e.to_string()))?;
    Ok(root.fingerprint(&secp256k1::Secp256k1::new()))
}

pub fn sign_message_by_seed(
    seed: &[u8],
    path: &String,
    message: &Message,
) -> Result<(i32, [u8; 64])> {
    let key = get_private_key_by_seed(seed, path)?;
    let secp = secp256k1::Secp256k1::new();
    let (rec_id, signature) = secp
        .sign_ecdsa_recoverable(message, &key)
        .serialize_compact();
    Ok((rec_id.to_i32(), signature))
}

pub fn sign_message_hash_by_private_key(
    message_hash: &[u8],
    private_key: &[u8],
) -> Result<[u8; 64]> {
    let secp = secp256k1::Secp256k1::signing_only();
    let message = Message::from_digest_slice(message_hash)
        .map_err(|e| KeystoreError::InvalidDataError(e.to_string()))?;

    let private_key = SecretKey::from_slice(private_key)
        .map_err(|e| KeystoreError::InvalidDataError(e.to_string()))?;

    let sig = secp.sign_ecdsa(&message, &private_key);
    Ok(sig.serialize_compact())
}

pub fn verify_signature(
    signature: &[u8],
    message_hash: &[u8],
    public_key_bytes: &[u8],
) -> Result<bool> {
    let secp = secp256k1::Secp256k1::verification_only();
    let public_key = PublicKey::from_slice(public_key_bytes)
        .map_err(|e| KeystoreError::InvalidDataError(e.to_string()))?;
    let message = Message::from_digest_slice(message_hash)
        .map_err(|e| KeystoreError::InvalidDataError(e.to_string()))?;
    let mut sig = ecdsa::Signature::from_compact(signature)
        .map_err(|e| KeystoreError::InvalidDataError(e.to_string()))?;
    sig.normalize_s();
    let result = secp.verify_ecdsa(&message, &sig, &public_key).is_ok();
    Ok(result)
}

#[cfg(test)]
mod tests {
    use crate::algorithms::crypto::hmac_sha512;
    use crate::algorithms::secp256k1::derive_public_key;
    use alloc::string::{String, ToString};
    use secp256k1::hashes;
    use third_party::hex;
    use third_party::hex::ToHex;

    use super::*;

    extern crate std;

    use std::println;

    #[test]
    fn test() {
        let result = hmac_sha512(
            b"Bitcoin seed",
            &hex::decode("dbc4ac53fcc6d33e38c63fde60dae89f").unwrap(),
        );
        println!("{}", hex::encode(result));
    }

    #[test]
    fn test_get_key() {
        let path = "m/84'/0'/0'/0/0";
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let key = get_private_key_by_seed(&seed, &path.to_string()).unwrap();
        assert_eq!(
            "0330d54fd0dd420a6e5f8d3624f5f3482cae350f79d5f0753bf5beef9c2d91af3c",
            key.public_key(&secp256k1::Secp256k1::new())
                .serialize()
                .encode_hex::<String>()
        );
    }

    #[test]
    fn test_sign() {
        let data = [0x01u8];
        let path = "m/84'/0'/0'/0/0";
        let message = Message::from_hashed_data::<hashes::sha256::Hash>(&data);
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let (rec_id, signature) = sign_message_by_seed(&seed, &path.to_string(), &message).unwrap();
        assert_eq!(0, rec_id);
        assert_eq!("4df58f401bb8bc22226f4e26e2f1d9b4758b45db6c4336eee4da3a8b15cfce0477e67f8132abaa84b543df45cdfa381a38e405be4b9dddbb02f5810ed6064dc0", signature.encode_hex::<String>())
    }

    #[test]
    fn test_derive_public_key() {
        let extended_pubkey_str = "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYJVVhhawA7d4R5WSWGFNbi8Aw6ZRc1brxMyWMzG3DSSSSoekkudhUd9yLb6qx39T9nMdj";
        let pub_key =
            derive_public_key(&extended_pubkey_str.to_string(), &String::from("m/0/0")).unwrap();
        assert_eq!(
            pub_key.serialize().encode_hex::<String>(),
            "03aaeb52dd7494c361049de67cc680e83ebcbbbdbeb13637d92cd845f70308af5e"
        )
    }

    #[test]
    fn test_get_master_fingerprint() {
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let master_fingerprint = get_master_fingerprint_by_seed(&seed).unwrap();
        assert_eq!("73c5da0a", master_fingerprint.encode_hex::<String>())
    }

    #[test]
    fn test_derive_extend_public_key() {
        let extended_pubkey_str = "xpub6GPqFm1j3L6SppDC1WSMRcUSB4Rt5oPkiHfTsbRTk9o1pzTFN2SrAvbX8a42j48vNrxRbVG8s7RZcNBWBo89yp7iohDAZAuszvnUo7DvJdx";
        let extended_pubkey =
            derive_extend_public_key(&extended_pubkey_str.to_string(), &String::from("m/0/0"))
                .unwrap()
                .to_string();
        assert_eq!(
            extended_pubkey,
            "xpub6L9VwNZddk9Z9JC4MRJfoGKJZFX4Sk3kAUjkfsMwMUFJFsoBTEmGJhAb74aD4JEpabkGX2xZR3rXX1E8tokxdDpcyFbeXieePeeHNjajGxt"
        );
    }

    #[test]
    fn it_should_sign_and_verify_the_signature() {
        let test_key = "f254b030d04cdd902e9219d8390e1deb5a585f3c25bacf5c74bd07803a8dd873";
        let test_key_bytes = hex::decode(test_key).unwrap();
        let tmp_private_key = SecretKey::from_slice(&test_key_bytes).unwrap();
        let secp = secp256k1::Secp256k1::new();
        let test_pubkey = tmp_private_key.public_key(&secp).serialize_uncompressed();
        let message_hash =
            hex::decode("0D94D045A7E0D4547E161AC360C73581A95383435A48D8869AB08FF34A8DB5E7")
                .unwrap();

        let signature = sign_message_hash_by_private_key(&message_hash, &test_key_bytes).unwrap();
        let result = verify_signature(&signature, &message_hash, &test_pubkey).unwrap();
        assert_eq!(result, true);
    }
}
