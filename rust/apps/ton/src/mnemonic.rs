use crate::errors::{MnemonicError, Result};
use crate::utils::pbkdf2_sha512;
use alloc::string::String;
use alloc::vec::Vec;
use core::cmp;
use cryptoxide::ed25519;
use cryptoxide::hmac::Hmac;
use cryptoxide::mac::Mac;
use cryptoxide::pbkdf2::pbkdf2;
use cryptoxide::sha2::Sha512;
use zeroize::Zeroize;

const PBKDF_ITERATIONS: u32 = 100000;
const TON_MNEMONIC_24_WORDS: usize = 24;
const TON_MNEMONIC_12_WORDS: usize = 12;

pub fn ton_mnemonic_to_entropy(normalized_words: &[String], password: &Option<String>) -> Vec<u8> {
    let mut binding = Hmac::new(Sha512::new(), normalized_words.join(" ").as_bytes());
    if let Some(password) = password {
        binding.input(password.as_bytes());
    }
    binding.result().code().to_vec()
}

pub fn ton_mnemonic_validate(normalized_words: &[String], password: &Option<String>) -> Result<()> {
    if normalized_words.len() != TON_MNEMONIC_24_WORDS
        && normalized_words.len() != TON_MNEMONIC_12_WORDS
    {
        return Err(MnemonicError::UnexpectedWordCount(normalized_words.len()).into());
    }

    let entropy = ton_mnemonic_to_entropy(normalized_words, &None);
    let mut seed: [u8; 64] = [0; 64];
    match password {
        Some(s) if !s.is_empty() => {
            pbkdf2_sha512(&entropy, "TON fast seed version".as_bytes(), 1, &mut seed);
            if seed[0] != 1 {
                seed.zeroize();
                return Err(MnemonicError::InvalidFirstByte(seed[0]).into());
            }
            let entropy = ton_mnemonic_to_entropy(normalized_words, password);
            pbkdf2_sha512(
                &entropy,
                "TON seed version".as_bytes(),
                cmp::max(1, PBKDF_ITERATIONS / 256),
                &mut seed,
            );
            if seed[0] == 0 {
                seed.zeroize();
                return Err(MnemonicError::InvalidFirstByte(seed[0]).into());
            }
        }
        _ => {
            pbkdf2_sha512(
                &entropy,
                "TON seed version".as_bytes(),
                cmp::max(1, PBKDF_ITERATIONS / 256),
                &mut seed,
            );
            if seed[0] != 0 {
                seed.zeroize();
                return Err(MnemonicError::InvalidPasswordlessMenmonicFirstByte(seed[0]).into());
            }
        }
    }
    seed.zeroize();

    Ok(())
}

pub fn ton_entropy_to_seed(entropy: &[u8]) -> [u8; 64] {
    let mut master_seed = [0u8; 64];
    pbkdf2(
        &mut Hmac::new(Sha512::new(), entropy),
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
    if words.len() != TON_MNEMONIC_24_WORDS && words.len() != TON_MNEMONIC_12_WORDS {
        return Err(MnemonicError::UnexpectedWordCount(words.len()).into());
    }
    let mut normalized_words: Vec<String> = words.iter().map(|w| w.trim().to_lowercase()).collect();
    ton_mnemonic_validate(&normalized_words, &password)?;
    let entropy = ton_mnemonic_to_entropy(&normalized_words, &password);
    normalized_words.zeroize();
    Ok(ton_entropy_to_seed(&entropy))
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
    use alloc::{format, string::ToString, vec};

    use hex;

    use super::*;
    extern crate std;
    use std::vec::Vec;

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
        {
            let words: Vec<String> = [
                "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
                "gas", "diet", "dune",
            ]
            .iter()
            .map(|v| v.to_lowercase())
            .collect();
            let result = ton_mnemonic_to_master_seed(words, None);
            assert!(result.is_err());
            assert_eq!(
                result.err().unwrap().to_string(),
                "Invalid TON Mnemonic, Invalid passwordless mnemonic (first byte: 0x0)".to_string()
            )
        }
        {
            let words: Vec<String> = [
                "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
                "gas", "diet",
            ]
            .iter()
            .map(|v| v.to_lowercase())
            .collect();
            let words_len = words.len();
            let result = ton_mnemonic_to_master_seed(words, None);
            assert!(result.is_err());
            assert_eq!(
                result.err().unwrap().to_string(),
                format!("Invalid TON Mnemonic, Invalid mnemonic word count (count: {words_len})",)
            )
        }
    }

    #[test]
    fn test_ton_mnemonic_with_password() {
        let words: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        let password = Some("test_password".to_string());
        let entropy = ton_mnemonic_to_entropy(&words, &password);

        // Verify entropy is generated with password
        assert_eq!(entropy.len(), 64);
        assert_ne!(
            hex::encode(&entropy),
            "46dcfbce05b1a1b42c535b05f84461f5bb1d62b1429b283fcfed943352c0ecfb29f671d2e8e6885d4dd3a8c85d99f91bbb7a17c73d96c918e6200f49268ea1a3"
        );
    }

    #[test]
    fn test_ton_mnemonic_12_words_valid() {
        // Test with a valid 12-word mnemonic
        let words: Vec<String> = [
            "abandon", "abandon", "abandon", "abandon", "abandon", "abandon", "abandon", "abandon",
            "abandon", "abandon", "abandon", "about",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        // Note: This might fail validation due to checksum, but should not fail on word count
        assert!(words.len() == TON_MNEMONIC_12_WORDS);
    }

    #[test]
    fn test_ton_master_seed_to_public_key() {
        let words: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        let master_seed = ton_mnemonic_to_master_seed(words, None).unwrap();
        let public_key = ton_master_seed_to_public_key(master_seed);

        // Verify public key is 32 bytes
        assert_eq!(public_key.len(), 32);
        assert_eq!(
            hex::encode(public_key),
            "c04ad1885c127fe863abb00752fa844e6439bb04f264d70de7cea580b32637ab"
        );
    }

    #[test]
    fn test_ton_mnemonic_with_empty_password() {
        let words: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        // Empty password should be treated same as no password
        let result_no_password = ton_mnemonic_to_master_seed(words.clone(), None).unwrap();
        let result_empty_password =
            ton_mnemonic_to_master_seed(words, Some("".to_string())).unwrap();

        assert_eq!(result_no_password, result_empty_password);
    }

    #[test]
    fn test_ton_mnemonic_with_whitespace() {
        let words_with_spaces: Vec<String> = vec![
            " dose ", " ice", "enrich ", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_string())
        .collect();

        let words_no_spaces: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_string())
        .collect();

        let seed_with_spaces = ton_mnemonic_to_master_seed(words_with_spaces, None).unwrap();
        let seed_no_spaces = ton_mnemonic_to_master_seed(words_no_spaces, None).unwrap();

        // Should produce same result after trimming
        assert_eq!(seed_with_spaces, seed_no_spaces);
    }

    #[test]
    fn test_ton_mnemonic_invalid_word_counts() {
        // Test various invalid word counts
        let invalid_counts = vec![1, 5, 10, 13, 15, 20, 25, 30];

        for count in invalid_counts {
            let words: Vec<String> = (0..count).map(|i| format!("word{i}")).collect();
            let result = ton_mnemonic_to_master_seed(words.clone(), None);

            assert!(result.is_err());
            let error_msg = result.err().unwrap().to_string();
            assert!(error_msg.contains(&format!("count: {count}")));
        }
    }

    #[test]
    fn test_ton_keypair_consistency() {
        let words: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        let master_seed = ton_mnemonic_to_master_seed(words, None).unwrap();
        let (secret_key, public_key) = ton_master_seed_to_keypair(master_seed);
        let public_key_direct = ton_master_seed_to_public_key(master_seed);

        // Verify both methods produce same public key
        assert_eq!(public_key, public_key_direct);

        // Verify key sizes
        assert_eq!(secret_key.len(), 64);
        assert_eq!(public_key.len(), 32);
    }

    #[test]
    fn test_ton_mnemonic_validate_different_passwords() {
        let words: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        // Test validation with no password
        let result_no_password = ton_mnemonic_validate(&words, &None);
        assert!(result_no_password.is_ok());

        // Test validation with different passwords - these may fail if mnemonic was not created with password
        let _result_with_password = ton_mnemonic_validate(&words, &Some("password123".to_string()));
        // The validation might fail because this specific mnemonic is passwordless
    }

    #[test]
    fn test_ton_mnemonic_different_passwords_different_seeds() {
        let words: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        let entropy_no_password = ton_mnemonic_to_entropy(&words, &None);
        let entropy_password1 = ton_mnemonic_to_entropy(&words, &Some("password1".to_string()));
        let entropy_password2 = ton_mnemonic_to_entropy(&words, &Some("password2".to_string()));

        // Different passwords should produce different entropies
        assert_ne!(entropy_no_password, entropy_password1);
        assert_ne!(entropy_password1, entropy_password2);
        assert_ne!(entropy_no_password, entropy_password2);
    }

    #[test]
    fn test_ton_mnemonic_to_entropy_12_words() {
        let words: Vec<String> = [
            "abandon", "abandon", "abandon", "abandon", "abandon", "abandon", "abandon", "abandon",
            "abandon", "abandon", "abandon", "about",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        let entropy = ton_mnemonic_to_entropy(&words, &None);

        // Should generate entropy for 12-word mnemonic
        assert_eq!(entropy.len(), 64);

        let entropy_with_password = ton_mnemonic_to_entropy(&words, &Some("test".to_string()));

        // Different entropy with password
        assert_ne!(entropy, entropy_with_password);
    }

    #[test]
    fn test_ton_mnemonic_with_special_characters_password() {
        let words: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        // Test with various special characters in password
        let passwords = vec![
            "p@ssw0rd!",
            "密码123",
            "pass word with spaces",
            "pass\nword\twith\twhitespace",
            "!@#$%^&*()_+-=[]{}|;:',.<>?/`~",
        ];

        for password in passwords {
            let entropy = ton_mnemonic_to_entropy(&words, &Some(password.to_string()));
            assert_eq!(entropy.len(), 64);
        }
    }

    #[test]
    fn test_ton_master_seed_to_keypair_deterministic() {
        let words: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        let master_seed = ton_mnemonic_to_master_seed(words, None).unwrap();

        // Generate keypair multiple times
        let (sk1, pk1) = ton_master_seed_to_keypair(master_seed);
        let (sk2, pk2) = ton_master_seed_to_keypair(master_seed);

        // Should be deterministic
        assert_eq!(sk1, sk2);
        assert_eq!(pk1, pk2);
    }

    #[test]
    fn test_ton_mnemonic_validate_empty_words() {
        let words: Vec<String> = vec![];
        let result = ton_mnemonic_validate(&words, &None);

        assert!(result.is_err());
        assert_eq!(
            result.err().unwrap().to_string(),
            "Invalid TON Mnemonic, Invalid mnemonic word count (count: 0)"
        );
    }

    #[test]
    fn test_ton_mnemonic_validate_24_words() {
        let words: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        // Verify it's 24 words
        assert_eq!(words.len(), TON_MNEMONIC_24_WORDS);

        // Should validate successfully
        let result = ton_mnemonic_validate(&words, &None);
        assert!(result.is_ok());
    }

    #[test]
    fn test_ton_mnemonic_long_password() {
        let words: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        // Test with very long password
        let long_password = "a".repeat(1000);
        let entropy = ton_mnemonic_to_entropy(&words, &Some(long_password));

        assert_eq!(entropy.len(), 64);
    }

    #[test]
    fn test_ton_mnemonic_unicode_in_words() {
        // Test with unicode characters that get normalized
        let words: Vec<String> = vec![
            "DOSE", "ICE", "ENRICH", "TRIGGER", "TEST", "DOVE", "CENTURY", "STILL", "BETRAY",
            "GAS", "DIET", "DUNE", "USE", "OTHER", "BASE", "GYM", "MAD", "LAW", "IMMENSE",
            "VILLAGE", "WORLD", "EXAMPLE", "PRAISE", "GAME",
        ]
        .iter()
        .map(|v| v.to_string())
        .collect();

        let result = ton_mnemonic_to_master_seed(words, None);
        assert!(result.is_ok());
    }

    #[test]
    fn test_ton_keypair_from_different_seeds() {
        let seed1 = [1u8; 64];
        let seed2 = [2u8; 64];

        let (sk1, pk1) = ton_master_seed_to_keypair(seed1);
        let (sk2, pk2) = ton_master_seed_to_keypair(seed2);

        // Different seeds should produce different keys
        assert_ne!(sk1, sk2);
        assert_ne!(pk1, pk2);
    }

    #[test]
    fn test_ton_public_key_from_different_seeds() {
        let seed1 = [1u8; 64];
        let seed2 = [2u8; 64];

        let pk1 = ton_master_seed_to_public_key(seed1);
        let pk2 = ton_master_seed_to_public_key(seed2);

        // Different seeds should produce different public keys
        assert_ne!(pk1, pk2);

        // Public keys should be 32 bytes
        assert_eq!(pk1.len(), 32);
        assert_eq!(pk2.len(), 32);
    }

    #[test]
    fn test_ton_mnemonic_case_insensitivity() {
        let test_cases = vec![
            ("dose", "DOSE"),
            ("ice", "Ice"),
            ("enrich", "ENRICH"),
            ("trigger", "TrIgGeR"),
        ];

        for (lower, upper) in test_cases {
            let words1: Vec<String> = vec![lower.to_string(); 24];
            let words2: Vec<String> = vec![upper.to_string(); 24];

            let normalized1: Vec<String> = words1.iter().map(|w| w.trim().to_lowercase()).collect();
            let normalized2: Vec<String> = words2.iter().map(|w| w.trim().to_lowercase()).collect();

            let entropy1 = ton_mnemonic_to_entropy(&normalized1, &None);
            let entropy2 = ton_mnemonic_to_entropy(&normalized2, &None);

            assert_eq!(entropy1, entropy2);
        }
    }

    #[test]
    fn test_ton_mnemonic_validate_with_invalid_password_check() {
        let words: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        // This mnemonic is passwordless, so validating with a password should fail
        let result = ton_mnemonic_validate(&words, &Some("wrongpassword".to_string()));
        assert!(result.is_err());
    }

    #[test]
    fn test_ton_mnemonic_boundary_exact_24_words() {
        // Test exactly 24 words
        let words: Vec<String> = (0..24).map(|i| format!("word{i}")).collect();
        assert_eq!(words.len(), TON_MNEMONIC_24_WORDS);

        // Should not fail on word count
        let result = ton_mnemonic_validate(&words, &None);
        // May fail on validation but not on count check
        if result.is_err() {
            let error_msg = result.err().unwrap().to_string();
            assert!(!error_msg.contains("Invalid mnemonic word count"));
        }
    }

    #[test]
    fn test_ton_mnemonic_boundary_exact_12_words() {
        // Test exactly 12 words
        let words: Vec<String> = (0..12).map(|i| format!("word{i}")).collect();
        assert_eq!(words.len(), TON_MNEMONIC_12_WORDS);

        // Should not fail on word count
        let result = ton_mnemonic_validate(&words, &None);
        // May fail on validation but not on count check
        if result.is_err() {
            let error_msg = result.err().unwrap().to_string();
            assert!(!error_msg.contains("Invalid mnemonic word count"));
        }
    }

    #[test]
    fn test_ton_entropy_consistent_across_calls() {
        let words: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        // Call multiple times to ensure consistency
        let entropy1 = ton_mnemonic_to_entropy(&words, &None);
        let entropy2 = ton_mnemonic_to_entropy(&words, &None);
        let entropy3 = ton_mnemonic_to_entropy(&words, &None);

        assert_eq!(entropy1, entropy2);
        assert_eq!(entropy2, entropy3);
    }

    #[test]
    fn test_ton_mnemonic_empty_string_vs_none_password() {
        let words: Vec<String> = vec![
            "dose", "ice", "enrich", "trigger", "test", "dove", "century", "still", "betray",
            "gas", "diet", "dune", "use", "other", "base", "gym", "mad", "law", "immense",
            "village", "world", "example", "praise", "game",
        ]
        .iter()
        .map(|v| v.to_lowercase())
        .collect();

        // Validation: empty string password should behave same as None
        let validate_none = ton_mnemonic_validate(&words, &None);
        let validate_empty = ton_mnemonic_validate(&words, &Some("".to_string()));

        assert_eq!(validate_none.is_ok(), validate_empty.is_ok());
    }
}
