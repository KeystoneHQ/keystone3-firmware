use crate::address::public_key_to_address;
use crate::errors::Result;
use alloc::string::String;

use bitcoin::secp256k1::PublicKey;

#[derive(Clone, Debug)]
pub struct PersonalMessage {
    pub raw_message: String,
    pub utf8_message: String,
    pub from: Option<String>,
}

impl PersonalMessage {
    pub fn from(
        raw_message: String,
        utf8_message: String,
        from: Option<PublicKey>,
    ) -> Result<Self> {
        Ok(Self {
            raw_message,
            utf8_message,
            from: from.map(|key| public_key_to_address(&key)),
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;

    #[test]
    fn test_personal_message_without_from() {
        let msg = PersonalMessage::from(
            "0x48656c6c6f".to_string(),
            "Hello".to_string(),
            None,
        )
        .unwrap();

        assert_eq!(msg.raw_message, "0x48656c6c6f");
        assert_eq!(msg.utf8_message, "Hello");
        assert_eq!(msg.from, None);
    }

    #[test]
    fn test_personal_message_with_from() {
        let pubkey_hex = "0479be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8";
        let pubkey_bytes = hex::decode(pubkey_hex).unwrap();
        let pubkey = PublicKey::from_slice(&pubkey_bytes).unwrap();

        let msg = PersonalMessage::from(
            "0x48656c6c6f".to_string(),
            "Hello".to_string(),
            Some(pubkey),
        )
        .unwrap();

        assert_eq!(msg.raw_message, "0x48656c6c6f");
        assert_eq!(msg.utf8_message, "Hello");
        assert!(msg.from.is_some());
    }
}
