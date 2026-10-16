pub mod errors;
pub mod program_option;
pub mod program_pack;
pub mod stake;
pub mod system_instruction;
pub mod vote;

pub mod clock {
    pub type Slot = u64;

    /// Uniquely distinguishes every version of a slot, even if the
    /// slot number is the same, i.e. duplicate slots
    pub type BankId = u64;

    /// Epoch is a unit of time a given leader schedule is honored,
    ///  some number of Slots.
    pub type Epoch = u64;

    /// SlotIndex is an index to the slots of a epoch
    pub type SlotIndex = u64;

    /// SlotCount is the number of slots in a epoch
    pub type SlotCount = u64;

    /// UnixTimestamp is an approximate measure of real-world time,
    /// expressed as Unix time (ie. seconds since the Unix epoch)
    pub type UnixTimestamp = i64;
}

pub mod program_utils {
    use crate::solana_lib::solana_program::errors::InstructionError;

    /// Deserialize with a limit based the maximum amount of data a program can expect to get.
    /// This function should be used in place of direct deserialization to help prevent OOM errors
    pub fn limited_deserialize<T, const LIMIT: usize>(
        instruction_data: &[u8],
    ) -> Result<T, InstructionError>
    where
        T: serde::de::DeserializeOwned,
    {
        if instruction_data.len() > LIMIT {
            return Err(InstructionError::InvalidInstructionData);
        }
        let config = bincode::config::legacy()
            .with_limit::<LIMIT>()
            .with_fixed_int_encoding(); // As per https://github.com/servo/bincode/issues/333, these two options are needed
        let (result, consumed) = bincode::serde::decode_from_slice(instruction_data, config)
            .map_err(|_| InstructionError::InvalidInstructionData)?;
        if consumed != instruction_data.len() {
            return Err(InstructionError::InvalidInstructionData);
        }
        Ok(result)
    }

    #[cfg(test)]
    mod tests {
        use super::*;
        use alloc::{string::String, vec::Vec};

        #[test]
        fn rejects_oversized_declared_lengths() {
            let oversized_length = u64::MAX.to_le_bytes();
            assert!(limited_deserialize::<String, 1232>(&oversized_length).is_err());
            assert!(limited_deserialize::<Vec<u8>, 1232>(&oversized_length).is_err());
        }

        #[test]
        fn vote_sequences_reject_huge_hints_and_preserve_valid_wire_data() {
            use crate::solana_lib::solana_program::vote::state::{Vote, VoteStateUpdate};
            let huge = u64::MAX.to_le_bytes();
            assert!(limited_deserialize::<Vote, 1232>(&huge).is_err());
            assert!(limited_deserialize::<VoteStateUpdate, 1232>(&huge).is_err());
            let mut vote = 1u64.to_le_bytes().to_vec();
            vote.extend_from_slice(&42u64.to_le_bytes());
            vote.extend_from_slice(&[0; 32]);
            vote.push(0);
            assert_eq!(
                limited_deserialize::<Vote, 1232>(&vote).unwrap().slots,
                [42]
            );
            let mut update = 1u64.to_le_bytes().to_vec();
            update.extend_from_slice(&42u64.to_le_bytes());
            update.extend_from_slice(&3u32.to_le_bytes());
            update.push(0);
            update.extend_from_slice(&[0; 32]);
            update.push(0);
            let parsed = limited_deserialize::<VoteStateUpdate, 1232>(&update).unwrap();
            assert_eq!(parsed.lockouts[0].slot, 42);
            assert_eq!(parsed.lockouts[0].confirmation_count, 3);
        }

        #[test]
        fn enforces_decode_limit_and_exact_consumption() {
            let mut data = 3u64.to_le_bytes().to_vec();
            data.extend_from_slice(b"abc");
            assert_eq!(limited_deserialize::<String, 11>(&data).unwrap(), "abc");
            assert!(limited_deserialize::<String, 10>(&data).is_err());
            data.push(0);
            assert!(limited_deserialize::<String, 1232>(&data).is_err());
            assert!(limited_deserialize::<String, 1232>(&1233u64.to_le_bytes()).is_err());
        }
    }
}

pub mod pubkey {
    use core::fmt;

    use bitcoin::base58;
    use serde_derive::{Deserialize, Serialize};

    use borsh::BorshDeserialize;

    pub const PUBKEY_BYTES: usize = 32;

    #[derive(
        Clone,
        Copy,
        Default,
        Deserialize,
        Eq,
        Hash,
        Ord,
        PartialEq,
        PartialOrd,
        Serialize,
        BorshDeserialize,
    )]
    pub struct Pubkey(pub(crate) [u8; 32]);

    impl fmt::Debug for Pubkey {
        fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
            write!(f, "{}", base58::encode(&self.0))
        }
    }

    impl fmt::Display for Pubkey {
        fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
            write!(f, "{}", base58::encode(&self.0))
        }
    }

    impl Pubkey {
        pub fn new(pubkey_vec: &[u8]) -> Self {
            Self(
                <[u8; 32]>::try_from(<&[u8]>::clone(&pubkey_vec))
                    .expect("Slice must be the same length as a Pubkey"),
            )
        }
    }
}

pub mod hash {
    use core::fmt;

    use bitcoin::base58;
    use serde_derive::{Deserialize, Serialize};

    pub const HASH_BYTES: usize = 32;

    #[derive(
        Serialize, Deserialize, Clone, Copy, Default, Eq, PartialEq, Ord, PartialOrd, Hash,
    )]
    pub struct Hash(pub(crate) [u8; HASH_BYTES]);

    impl fmt::Debug for Hash {
        fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
            write!(f, "{}", base58::encode(&self.0))
        }
    }

    impl fmt::Display for Hash {
        fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
            write!(f, "{}", base58::encode(&self.0))
        }
    }
}
