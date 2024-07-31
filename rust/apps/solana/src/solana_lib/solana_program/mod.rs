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

    const DECODE_LIMIT: usize = u64::MAX as usize;

    /// Deserialize with a limit based the maximum amount of data a program can expect to get.
    /// This function should be used in place of direct deserialization to help prevent OOM errors
    pub fn limited_deserialize<T>(
        instruction_data: &[u8],
        _limit: u64,
    ) -> Result<T, InstructionError>
    where
        T: serde::de::DeserializeOwned,
    {
        let config = bincode::config::legacy()
            .with_limit::<DECODE_LIMIT>()
            .with_fixed_int_encoding(); // As per https://github.com/servo/bincode/issues/333, these two options are needed
        let (result, _) = bincode::serde::decode_from_slice(instruction_data, config)
            .map_err(|_| InstructionError::InvalidInstructionData)?;
        Ok(result)
    }
}

pub mod pubkey {
    use core::fmt;

    use serde_derive::{Deserialize, Serialize};
    use third_party::base58;

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

    use serde_derive::{Deserialize, Serialize};
    use third_party::base58;

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
