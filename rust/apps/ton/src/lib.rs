#![no_std]
#![cfg_attr(coverage_nightly, feature(coverage_attribute))]

use core::str::FromStr;

use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use errors::Result;

extern crate alloc;

use vendor::{address::TonAddress, wallet::TonWallet};

pub mod errors;
mod jettons;
mod messages;
pub mod mnemonic;
pub mod structs;
pub mod transaction;
mod utils;

#[cfg_attr(coverage_nightly, coverage(off))]
#[allow(warnings)]
mod vendor;

pub fn ton_public_key_to_address(pk: Vec<u8>) -> Result<String> {
    TonWallet::derive_default(vendor::wallet::WalletVersion::V4R2, pk)
        .map(|v| v.address.to_base64_url_flags(true, false))
        .map_err(|e| errors::TonError::AddressError(e.to_string()))
}

pub fn ton_compare_address_and_public_key(pk: Vec<u8>, address: String) -> bool {
    match TonWallet::derive_default(vendor::wallet::WalletVersion::V4R2, pk) {
        Ok(wallet) => match TonAddress::from_str(&address) {
            Ok(address) => wallet.address.eq(&address),
            Err(_e) => false,
        },
        Err(_e) => false,
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    extern crate std;
    use alloc::vec;

    #[test]
    fn test_generate_address() {
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = super::ton_public_key_to_address(pk).unwrap();
        assert_eq!(address, "UQC4FC01K66rElokeYTPeEnWStITQDxGiK8RhkMXpT88FY5b")
    }

    #[test]
    fn test_generate_address_different_key() {
        // Test with a different public key
        let pk = hex::decode("82e594257c8c42f193ecef1f7d61f261e817211e2f8033c3e97de8647ddf7855")
            .unwrap();
        let address = ton_public_key_to_address(pk).unwrap();
        // Verify it returns a valid base64 URL address
        assert!(!address.is_empty());
        assert!(address.starts_with("UQ") || address.starts_with("EQ"));
    }

    #[test]
    fn test_generate_address_invalid_key_length() {
        // Test with invalid key length (not 32 bytes)
        let pk = vec![0u8; 16]; // Only 16 bytes instead of 32
        let result = ton_public_key_to_address(pk);
        assert!(result.is_err());
    }

    #[test]
    fn test_generate_address_empty_key() {
        // Test with empty public key
        let pk = vec![];
        let result = ton_public_key_to_address(pk);
        assert!(result.is_err());
    }

    #[test]
    fn test_generate_address_oversized_key() {
        // Test with oversized key (more than 32 bytes)
        let pk = vec![0u8; 64];
        let result = ton_public_key_to_address(pk);
        assert!(result.is_err());
    }

    #[test]
    fn test_compare_address_and_public_key_match() {
        // Test that matching public key and address return true
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = "UQC4FC01K66rElokeYTPeEnWStITQDxGiK8RhkMXpT88FY5b".to_string();
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(result);
    }

    #[test]
    fn test_compare_address_and_public_key_mismatch() {
        // Test that non-matching public key and address return false
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = "UQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAJKZ".to_string();
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_compare_address_and_public_key_invalid_address() {
        // Test with invalid address format
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = "invalid_address_format".to_string();
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_compare_address_and_public_key_empty_address() {
        // Test with empty address
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = "".to_string();
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_compare_address_and_public_key_invalid_key() {
        // Test with invalid public key length
        let pk = vec![0u8; 16];
        let address = "UQC4FC01K66rElokeYTPeEnWStITQDxGiK8RhkMXpT88FY5b".to_string();
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_compare_address_and_public_key_empty_key() {
        // Test with empty public key
        let pk = vec![];
        let address = "UQC4FC01K66rElokeYTPeEnWStITQDxGiK8RhkMXpT88FY5b".to_string();
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_compare_address_with_different_formats() {
        // Test that UQ (bounceable) and EQ (non-bounceable) are treated as different addresses
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let generated_address = ton_public_key_to_address(pk.clone()).unwrap();
        // The generated address should be UQ (bounceable) format
        assert!(generated_address.starts_with("UQ"));

        // Comparing with the correct UQ format should match
        assert!(ton_compare_address_and_public_key(
            pk.clone(),
            generated_address
        ));

        // Try with EQ prefix (non-bounceable) - this should not match
        // as the comparison is strict about the format
        let eq_address = "EQC4FC01K66rElokeYTPeEnWStITQDxGiK8RhkMXpT88FVuL".to_string();
        let result = ton_compare_address_and_public_key(pk, eq_address);
        // Different format flags mean different addresses for comparison purposes
        assert!(!result);
    }

    #[test]
    fn test_generate_address_consistency() {
        // Test that generating address multiple times gives same result
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address1 = ton_public_key_to_address(pk.clone()).unwrap();
        let address2 = ton_public_key_to_address(pk.clone()).unwrap();
        assert_eq!(address1, address2);
    }

    #[test]
    fn test_generate_address_different_keys_different_addresses() {
        // Test that different keys generate different addresses
        let pk1 = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let pk2 = hex::decode("82e594257c8c42f193ecef1f7d61f261e817211e2f8033c3e97de8647ddf7855")
            .unwrap();

        let address1 = ton_public_key_to_address(pk1).unwrap();
        let address2 = ton_public_key_to_address(pk2).unwrap();

        assert_ne!(address1, address2);
    }

    #[test]
    fn test_compare_with_generated_address() {
        // Integration test: generate address and then compare it
        let pk = hex::decode("82e594257c8c42f193ecef1f7d61f261e817211e2f8033c3e97de8647ddf7855")
            .unwrap();
        let generated_address = ton_public_key_to_address(pk.clone()).unwrap();
        let result = ton_compare_address_and_public_key(pk, generated_address);
        assert!(result);
    }

    #[test]
    fn test_compare_address_case_sensitivity() {
        // Addresses are case-sensitive in base64
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        // Use lowercase which should be invalid
        let address = "uqc4fc01k66relokeytpeenwstiitqdxgik8rhkmxpt88fy5b".to_string();
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_generate_address_all_zeros_key() {
        // Test with all zeros key (edge case)
        let pk = vec![0u8; 32];
        let result = ton_public_key_to_address(pk);
        // Should succeed as it's a valid 32-byte key
        assert!(result.is_ok());
    }

    #[test]
    fn test_generate_address_all_ones_key() {
        // Test with all 0xFF key (edge case)
        let pk = vec![0xFFu8; 32];
        let result = ton_public_key_to_address(pk);
        // Should succeed as it's a valid 32-byte key
        assert!(result.is_ok());
    }

    #[test]
    fn test_compare_address_with_whitespace() {
        // Test that address with whitespace doesn't match
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = " UQC4FC01K66rElokeYTPeEnWStITQDxGiK8RhkMXpT88FY5b ".to_string();
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_address_format_validation() {
        // Verify generated addresses have correct format
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = ton_public_key_to_address(pk).unwrap();

        // Address should start with UQ or EQ
        assert!(address.starts_with("UQ") || address.starts_with("EQ"));
        // Address should be base64 characters
        assert!(address
            .chars()
            .all(|c| c.is_alphanumeric() || c == '_' || c == '-'));
        // Address should have reasonable length (typically 48 characters)
        assert!(address.len() >= 40 && address.len() <= 50);
    }

    #[test]
    fn test_multiple_known_address_pairs() {
        // Test multiple known public key and address pairs
        let test_cases = vec![(
            "15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1",
            "UQC4FC01K66rElokeYTPeEnWStITQDxGiK8RhkMXpT88FY5b",
        )];

        for (pk_hex, expected_address) in test_cases {
            let pk = hex::decode(pk_hex).unwrap();
            let address = ton_public_key_to_address(pk.clone()).unwrap();
            assert_eq!(address, expected_address);
            assert!(ton_compare_address_and_public_key(
                pk,
                expected_address.to_string()
            ));
        }
    }

    #[test]
    fn test_key_length_boundary_31_bytes() {
        // Test with exactly 31 bytes (one less than required)
        let pk = vec![0u8; 31];
        let result = ton_public_key_to_address(pk);
        assert!(result.is_err());
    }

    #[test]
    fn test_key_length_boundary_33_bytes() {
        // Test with exactly 33 bytes (one more than required)
        let pk = vec![0u8; 33];
        let result = ton_public_key_to_address(pk);
        assert!(result.is_err());
    }

    #[test]
    fn test_compare_with_truncated_address() {
        // Test with truncated address
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = "UQC4FC01K66rEloke".to_string(); // Truncated
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_compare_with_extra_characters() {
        // Test with address having extra characters at the end
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = "UQC4FC01K66rElokeYTPeEnWStITQDxGiK8RhkMXpT88FY5bEXTRA".to_string();
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_address_with_invalid_base64() {
        // Test with invalid base64 characters
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = "UQC4FC01K66rEloke@#$%^&*()+=FY5b".to_string();
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_sequential_key_bytes() {
        // Test with sequential bytes (0x00, 0x01, 0x02, ... 0x1F)
        let pk: Vec<u8> = (0..32).collect();
        let result = ton_public_key_to_address(pk);
        assert!(result.is_ok());
        assert!(!result.unwrap().is_empty());
    }

    #[test]
    fn test_alternating_bit_pattern() {
        // Test with alternating bit pattern (0xAA repeated)
        let pk = vec![0xAAu8; 32];
        let result = ton_public_key_to_address(pk);
        assert!(result.is_ok());
    }

    #[test]
    fn test_alternating_bit_pattern_2() {
        // Test with alternating bit pattern (0x55 repeated)
        let pk = vec![0x55u8; 32];
        let result = ton_public_key_to_address(pk);
        assert!(result.is_ok());
    }

    #[test]
    fn test_compare_same_address_different_keys() {
        // Ensure different keys don't match the same address
        let pk1 = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let pk2 = hex::decode("82e594257c8c42f193ecef1f7d61f261e817211e2f8033c3e97de8647ddf7855")
            .unwrap();
        let address = "UQC4FC01K66rElokeYTPeEnWStITQDxGiK8RhkMXpT88FY5b".to_string();

        let result1 = ton_compare_address_and_public_key(pk1, address.clone());
        let result2 = ton_compare_address_and_public_key(pk2, address);

        // Only one should match
        assert_ne!(result1, result2);
    }

    #[test]
    fn test_address_with_only_prefix() {
        // Test with only address prefix
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = "UQ".to_string();
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_address_with_wrong_prefix() {
        // Test with wrong prefix (not UQ or EQ)
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = "ABC4FC01K66rElokeYTPeEnWStITQDxGiK8RhkMXpT88FY5b".to_string();
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_high_entropy_key() {
        // Test with high entropy key (pseudo-random bytes)
        let pk = hex::decode("a3f4b1c2d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2")
            .unwrap();
        let result = ton_public_key_to_address(pk);
        assert!(result.is_ok());
        let address = result.unwrap();
        assert!(address.starts_with("UQ") || address.starts_with("EQ"));
    }

    #[test]
    fn test_low_entropy_key() {
        // Test with low entropy key (mostly zeros with few ones)
        let mut pk = vec![0u8; 32];
        pk[0] = 1;
        pk[31] = 1;
        let result = ton_public_key_to_address(pk);
        assert!(result.is_ok());
    }

    #[test]
    fn test_middle_bit_pattern_key() {
        // Test with alternating bytes pattern
        let pk: Vec<u8> = (0..32)
            .map(|i| if i % 2 == 0 { 0xFF } else { 0x00 })
            .collect();
        let result = ton_public_key_to_address(pk);
        assert!(result.is_ok());
    }

    #[test]
    fn test_compare_address_consistency() {
        // Test that comparison is consistent across multiple calls
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = "UQC4FC01K66rElokeYTPeEnWStITQDxGiK8RhkMXpT88FY5b".to_string();

        for _ in 0..5 {
            let result = ton_compare_address_and_public_key(pk.clone(), address.clone());
            assert!(result);
        }
    }

    #[test]
    fn test_numeric_string_as_address() {
        // Test with numeric string (invalid address)
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = "123456789012345678901234567890".to_string();
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_special_characters_in_address() {
        // Test with special characters
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = "UQC4FC01K66r!@#$%^&*()".to_string();
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_very_long_address_string() {
        // Test with excessively long address string
        let pk = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let address = "UQ".to_string() + &"A".repeat(1000);
        let result = ton_compare_address_and_public_key(pk, address);
        assert!(!result);
    }

    #[test]
    fn test_address_generation_deterministic() {
        // Verify that address generation is deterministic
        let pk = hex::decode("82e594257c8c42f193ecef1f7d61f261e817211e2f8033c3e97de8647ddf7855")
            .unwrap();

        let mut addresses = Vec::new();
        for _ in 0..10 {
            addresses.push(ton_public_key_to_address(pk.clone()).unwrap());
        }

        // All addresses should be identical
        let first = &addresses[0];
        for addr in &addresses {
            assert_eq!(addr, first);
        }
    }

    #[test]
    fn test_single_bit_difference_keys() {
        // Test that keys differing by a single bit produce different addresses
        let pk1 = hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
            .unwrap();
        let mut pk2 = pk1.clone();
        pk2[0] ^= 0x01; // Flip the least significant bit

        let addr1 = ton_public_key_to_address(pk1).unwrap();
        let addr2 = ton_public_key_to_address(pk2).unwrap();

        assert_ne!(addr1, addr2);
    }

    #[test]
    fn test_first_byte_variations() {
        // Test variations in the first byte
        for byte_val in [0x00, 0x7F, 0x80, 0xFF] {
            let mut pk =
                hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
                    .unwrap();
            pk[0] = byte_val;
            let result = ton_public_key_to_address(pk);
            assert!(result.is_ok());
        }
    }

    #[test]
    fn test_last_byte_variations() {
        // Test variations in the last byte
        for byte_val in [0x00, 0x7F, 0x80, 0xFF] {
            let mut pk =
                hex::decode("15556a2d93ab1471eb34e1d6873fc637e6a4b5a9cb2638148c12dc4bac1651f1")
                    .unwrap();
            pk[31] = byte_val;
            let result = ton_public_key_to_address(pk);
            assert!(result.is_ok());
        }
    }
}
