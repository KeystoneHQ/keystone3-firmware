use crate::errors::{Result, RustCError};
use alloc::string::ToString;
use secp256k1::{ecdsa, Message, PublicKey, Secp256k1, SecretKey};


pub fn verify_signature(signature: &[u8], message_hash: &[u8], pubkey: &[u8]) -> Result<bool> {
    let secp = Secp256k1::verification_only();
    let public_key = PublicKey::from_slice(pubkey)
        .map_err(|e| RustCError::FormatTypeError(e.to_string()))?;
    let message = Message::from_slice(message_hash)
        .map_err(|e| RustCError::FormatTypeError(e.to_string()))?;
    let mut sig = ecdsa::Signature::from_compact(signature)
        .map_err(|e| RustCError::FormatTypeError(e.to_string()))?;
    sig.normalize_s();
    let result = secp.verify_ecdsa(&message, &sig, &public_key).is_ok();
    Ok(result)
}

pub fn sign_message_by_key(message_hash: &[u8], private_key: &[u8]) -> Result<[u8; 64]> {
    let secp = Secp256k1::signing_only();
    let message = Message::from_slice(message_hash)
        .map_err(|e| RustCError::FormatTypeError(e.to_string()))?;

    let private_key = SecretKey::from_slice(private_key)
        .map_err(|e| RustCError::FormatTypeError(e.to_string()))?;

    let sig = secp.sign_ecdsa(&message, &private_key);
    Ok(sig.serialize_compact())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_should_sign_and_verify_the_signature() {
        let test_key = "f254b030d04cdd902e9219d8390e1deb5a585f3c25bacf5c74bd07803a8dd873";
        let private_key = hex::decode(test_key).unwrap();
        let message_hash =
            hex::decode("0D94D045A7E0D4547E161AC360C73581A95383435A48D8869AB08FF34A8DB5E7")
                .unwrap();

        let sig = hex::decode("168c267d21968b1447a676276d7ee7055810d58ac5524457361a09647bf19d2b108dd831a9d590019c93151d700e1c20eaf95fef24c60e645c04178227880e94").unwrap();
        let pubkey_bytes = hex::decode(PUBKEY_STRING).unwrap();
        // test pubkey
        const PUBKEY_STRING: &str = "04e3003fa1467452743ed7b97cc8c0786f3b9c255d31ccca9e6dc59915b17fa8ed5933cf74ce8ec3614a503422f0e77b495a07567e29256858d6282f63c6dbfebd";
        let result = verify_signature(&sig, &message_hash, &pubkey_bytes).unwrap();
        assert_eq!(result, true);
    }
}
