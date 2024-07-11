use crate::errors::{MnemonicError, Result};
use crate::utils::pbkdf2_sha512;
use alloc::string::String;
use alloc::vec::Vec;
use core::cmp;
use third_party::cryptoxide::ed25519;
use third_party::cryptoxide::hmac::Hmac;
use third_party::cryptoxide::mac::Mac;
use third_party::cryptoxide::pbkdf2::pbkdf2;
use third_party::cryptoxide::sha2::Sha512;

const PBKDF_ITERATIONS: u32 = 100000;

pub fn ton_mnemonic_to_entropy(
    normalized_words: &Vec<String>,
    password: &Option<String>,
) -> Vec<u8> {
    let mut binding = Hmac::new(Sha512::new(), normalized_words.join(" ").as_bytes());
    if let Some(password) = password {
        binding.input(password.as_bytes());
    }
    binding.result().code().to_vec()
}

pub fn ton_mnemonic_validate(
    normalized_words: &Vec<String>,
    password: &Option<String>,
) -> Result<()> {
    let entropy = ton_mnemonic_to_entropy(normalized_words, &None);
    match password {
        Some(s) if !s.is_empty() => {
            let mut seed: [u8; 64] = [0; 64];
            pbkdf2_sha512(&entropy, "TON fast seed version".as_bytes(), 1, &mut seed);
            if seed[0] != 1 {
                return Err(MnemonicError::InvalidFirstByte(seed[0]).into());
            }
            let entropy = ton_mnemonic_to_entropy(&normalized_words, password);
            pbkdf2_sha512(
                &entropy,
                "TON seed version".as_bytes(),
                cmp::max(1, PBKDF_ITERATIONS / 256),
                &mut seed,
            );
            if seed[0] == 0 {
                return Err(MnemonicError::InvalidFirstByte(seed[0]).into());
            }
        }
        _ => {
            let mut seed: [u8; 64] = [0; 64];
            pbkdf2_sha512(
                &entropy,
                "TON seed version".as_bytes(),
                cmp::max(1, PBKDF_ITERATIONS / 256),
                &mut seed,
            );
            if seed[0] != 0 {
                return Err(MnemonicError::InvalidPasswordlessMenmonicFirstByte(seed[0]).into());
            }
        }
    }

    Ok(())
}

pub fn ton_entropy_to_seed(entropy: Vec<u8>) -> [u8; 64] {
    let mut master_seed = [0u8; 64];
    pbkdf2(
        &mut Hmac::new(Sha512::new(), &entropy),
        b"TON default seed",
        PBKDF_ITERATIONS,
        &mut master_seed,
    );
    master_seed
}

pub fn ton_mnemonic_to_master_seed(
    words: Vec<String>,
    password: Option<String>,
) -> Result<[u8; 64]> {
    if words.len() != 24 {
        return Err(MnemonicError::UnexpectedWordCount(words.len()).into());
    }
    let normalized_words: Vec<String> = words.iter().map(|w| w.trim().to_lowercase()).collect();
    ton_mnemonic_validate(&normalized_words, &password)?;
    let entropy = ton_mnemonic_to_entropy(&normalized_words, &password);
    Ok(ton_entropy_to_seed(entropy))
}

pub fn ton_master_seed_to_keypair(master_seed: [u8; 64]) -> ([u8; 64], [u8; 32]) {
    let mut key = [0u8; 32];
    key.copy_from_slice(&master_seed[..32]);
    ed25519::keypair(&key)
}

pub fn ton_master_seed_to_public_key(master_seed: [u8; 64]) -> [u8; 32] {
    let mut key = [0u8; 32];
    key.copy_from_slice(&master_seed[..32]);
    ed25519::keypair(&key).1
}

#[cfg(test)]
mod tests {
    use alloc::{string::ToString, vec};
    use base64::Engine;
    use third_party::hex;

    use super::*;
    extern crate std;
    use std::{println, vec::Vec};

    #[test]
    fn test_ton_mnemonic_to_entropy() {
        let words: Vec<&str> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ];
        let normalized_words: Vec<String> = words.iter().map(|w| w.trim().to_lowercase()).collect();
        let result = ton_mnemonic_to_entropy(&normalized_words, &None);
        assert_eq!(hex::encode(result), "46dcfbce05b1a1b42c535b05f84461f5bb1d62b1429b283fcfed943352c0ecfb29f671d2e8e6885d4dd3a8c85d99f91bbb7a17c73d96c918e6200f49268ea1a3");
    }

    #[test]
    fn test_ton_mnemonic_to_master_seed() {
        let words: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();
        let result = ton_mnemonic_to_master_seed(words, None);
        let result = ton_master_seed_to_keypair(result.unwrap());
        assert_eq!(hex::encode(result.0), "119dcf2840a3d56521d260b2f125eedc0d4f3795b9e627269a4b5a6dca8257bdc04ad1885c127fe863abb00752fa844e6439bb04f264d70de7cea580b32637ab");
    }

    #[test]
    fn test_ton_mnemonic_invalid_mnemonic() {
        let words = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();
        let result = ton_mnemonic_to_master_seed(words, None);
        assert_eq!(result.is_err(), true);
        assert_eq!(
            result.err().unwrap().to_string(),
            "Invalid TON Mnemonic, Invalid mnemonic word count (count: 12)"
        )
    }
}
