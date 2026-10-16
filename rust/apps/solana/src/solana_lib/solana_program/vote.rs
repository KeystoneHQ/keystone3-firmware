pub mod state {
    use crate::solana_lib::solana_program::clock::{Slot, UnixTimestamp};
    use crate::solana_lib::solana_program::hash::Hash;
    use crate::solana_lib::solana_program::pubkey::Pubkey;
    use alloc::collections::VecDeque;
    use alloc::string::String;
    use alloc::vec::Vec;
    use serde_derive::{Deserialize, Serialize};

    fn bounded_sequence<'de, D, T, const MAX: usize>(deserializer: D) -> Result<Vec<T>, D::Error>
    where
        D: serde::Deserializer<'de>,
        T: serde::Deserialize<'de>,
    {
        struct Visitor<T, const MAX: usize>(core::marker::PhantomData<T>);
        impl<'de, T: serde::Deserialize<'de>, const MAX: usize> serde::de::Visitor<'de>
            for Visitor<T, MAX>
        {
            type Value = Vec<T>;
            fn expecting(&self, formatter: &mut core::fmt::Formatter) -> core::fmt::Result {
                formatter.write_str("a bounded vote sequence")
            }
            fn visit_seq<A: serde::de::SeqAccess<'de>>(
                self,
                mut seq: A,
            ) -> Result<Vec<T>, A::Error> {
                if seq.size_hint().is_some_and(|size| size > MAX) {
                    return Err(serde::de::Error::custom(
                        "vote sequence exceeds instruction limit",
                    ));
                }
                let mut values = Vec::new();
                while let Some(value) = seq.next_element()? {
                    if values.len() == MAX {
                        return Err(serde::de::Error::custom(
                            "vote sequence exceeds instruction limit",
                        ));
                    }
                    values.push(value);
                }
                Ok(values)
            }
        }
        deserializer.deserialize_seq(Visitor::<T, MAX>(core::marker::PhantomData))
    }

    fn deserialize_slots<'de, D: serde::Deserializer<'de>>(
        deserializer: D,
    ) -> Result<Vec<Slot>, D::Error> {
        bounded_sequence::<D, Slot, { 1232 / 8 }>(deserializer)
    }

    fn deserialize_lockouts<'de, D: serde::Deserializer<'de>>(
        deserializer: D,
    ) -> Result<VecDeque<Lockout>, D::Error> {
        bounded_sequence::<D, Lockout, { 1232 / 12 }>(deserializer).map(VecDeque::from)
    }

    #[derive(Debug, Serialize, Deserialize, PartialEq, Eq, Clone)]
    pub struct VoteInit {
        pub node_pubkey: Pubkey,
        pub authorized_voter: Pubkey,
        pub authorized_withdrawer: Pubkey,
        pub commission: u8,
    }

    #[derive(Debug, Serialize, Deserialize, PartialEq, Eq, Clone)]
    pub enum VoteAuthorize {
        Voter,
        Withdrawer,
    }

    #[derive(Debug, Serialize, Deserialize, PartialEq, Eq, Clone)]
    pub struct VoteAuthorizeCheckedWithSeedArgs {
        pub authorization_type: VoteAuthorize,
        pub current_authority_derived_key_owner: Pubkey,
        pub current_authority_derived_key_seed: String,
    }

    #[derive(Debug, Serialize, Deserialize, PartialEq, Eq, Clone)]
    pub struct VoteAuthorizeWithSeedArgs {
        pub authorization_type: VoteAuthorize,
        pub current_authority_derived_key_owner: Pubkey,
        pub current_authority_derived_key_seed: String,
        pub new_authority: Pubkey,
    }

    #[derive(Debug, Serialize, Deserialize, PartialEq, Eq, Clone)]
    pub struct BlockTimestamp {
        pub slot: Slot,
        pub timestamp: UnixTimestamp,
    }

    #[derive(Debug, Serialize, Deserialize, PartialEq, Eq, Clone)]
    pub struct Vote {
        /// A stack of votes starting with the oldest vote
        #[serde(deserialize_with = "deserialize_slots")]
        pub slots: Vec<Slot>,
        /// signature of the bank's state at the last slot
        pub hash: Hash,
        /// processing timestamp of last slot
        pub timestamp: Option<UnixTimestamp>,
    }

    #[derive(Debug, Serialize, Deserialize, PartialEq, Eq, Clone)]
    pub struct Lockout {
        pub slot: Slot,
        pub confirmation_count: u32,
    }

    #[derive(Debug, Serialize, Deserialize, PartialEq, Eq, Clone)]
    pub struct VoteStateUpdate {
        /// The proposed tower
        #[serde(deserialize_with = "deserialize_lockouts")]
        pub lockouts: VecDeque<Lockout>,
        /// The proposed root
        pub root: Option<Slot>,
        /// signature of the bank's state at the last slot
        pub hash: Hash,
        /// processing timestamp of last slot
        pub timestamp: Option<UnixTimestamp>,
    }
}

pub mod instruction {
    use crate::solana_lib::solana_program::hash::Hash;
    use crate::solana_lib::solana_program::pubkey::Pubkey;
    use crate::solana_lib::solana_program::vote::state::{
        Vote, VoteAuthorize, VoteAuthorizeCheckedWithSeedArgs, VoteAuthorizeWithSeedArgs, VoteInit,
        VoteStateUpdate,
    };
    use serde_derive::{Deserialize, Serialize};

    #[derive(Serialize, Deserialize, Debug, Clone, PartialEq, Eq)]
    pub enum VoteInstruction {
        /// Initialize a vote account
        ///
        /// # Account references
        ///   0. `[WRITE]` Uninitialized vote account
        ///   1. `[]` Rent sysvar
        ///   2. `[]` Clock sysvar
        ///   3. `[SIGNER]` New validator identity (node_pubkey)
        InitializeAccount(VoteInit),

        /// Authorize a key to send votes or issue a withdrawal
        ///
        /// # Account references
        ///   0. `[WRITE]` Vote account to be updated with the Pubkey for authorization
        ///   1. `[]` Clock sysvar
        ///   2. `[SIGNER]` Vote or withdraw authority
        Authorize(Pubkey, VoteAuthorize),

        /// A Vote instruction with recent votes
        ///
        /// # Account references
        ///   0. `[WRITE]` Vote account to vote with
        ///   1. `[]` Slot hashes sysvar
        ///   2. `[]` Clock sysvar
        ///   3. `[SIGNER]` Vote authority
        Vote(Vote),

        /// Withdraw some amount of funds
        ///
        /// # Account references
        ///   0. `[WRITE]` Vote account to withdraw from
        ///   1. `[WRITE]` Recipient account
        ///   2. `[SIGNER]` Withdraw authority
        Withdraw(u64),

        /// Update the vote account's validator identity (node_pubkey)
        ///
        /// # Account references
        ///   0. `[WRITE]` Vote account to be updated with the given authority public key
        ///   1. `[SIGNER]` New validator identity (node_pubkey)
        ///   2. `[SIGNER]` Withdraw authority
        UpdateValidatorIdentity,

        /// Update the commission for the vote account
        ///
        /// # Account references
        ///   0. `[WRITE]` Vote account to be updated
        ///   1. `[SIGNER]` Withdraw authority
        UpdateCommission(u8),

        /// A Vote instruction with recent votes
        ///
        /// # Account references
        ///   0. `[WRITE]` Vote account to vote with
        ///   1. `[]` Slot hashes sysvar
        ///   2. `[]` Clock sysvar
        ///   3. `[SIGNER]` Vote authority
        VoteSwitch(Vote, Hash),

        /// Authorize a key to send votes or issue a withdrawal
        ///
        /// This instruction behaves like `Authorize` with the additional requirement that the new vote
        /// or withdraw authority must also be a signer.
        ///
        /// # Account references
        ///   0. `[WRITE]` Vote account to be updated with the Pubkey for authorization
        ///   1. `[]` Clock sysvar
        ///   2. `[SIGNER]` Vote or withdraw authority
        ///   3. `[SIGNER]` New vote or withdraw authority
        AuthorizeChecked(VoteAuthorize),

        /// Update the onchain vote state for the signer.
        ///
        /// # Account references
        ///   0. `[Write]` Vote account to vote with
        ///   1. `[SIGNER]` Vote authority
        UpdateVoteState(VoteStateUpdate),

        /// Update the onchain vote state for the signer along with a switching proof.
        ///
        /// # Account references
        ///   0. `[Write]` Vote account to vote with
        ///   1. `[SIGNER]` Vote authority
        UpdateVoteStateSwitch(VoteStateUpdate, Hash),

        /// Given that the current Voter or Withdrawer authority is a derived key,
        /// this instruction allows someone who can sign for that derived key's
        /// base key to authorize a new Voter or Withdrawer for a vote account.
        ///
        /// # Account references
        ///   0. `[Write]` Vote account to be updated
        ///   1. `[]` Clock sysvar
        ///   2. `[SIGNER]` Base key of current Voter or Withdrawer authority's derived key
        AuthorizeWithSeed(VoteAuthorizeWithSeedArgs),

        /// Given that the current Voter or Withdrawer authority is a derived key,
        /// this instruction allows someone who can sign for that derived key's
        /// base key to authorize a new Voter or Withdrawer for a vote account.
        ///
        /// This instruction behaves like `AuthorizeWithSeed` with the additional requirement
        /// that the new vote or withdraw authority must also be a signer.
        ///
        /// # Account references
        ///   0. `[Write]` Vote account to be updated
        ///   1. `[]` Clock sysvar
        ///   2. `[SIGNER]` Base key of current Voter or Withdrawer authority's derived key
        ///   3. `[SIGNER]` New vote or withdraw authority
        AuthorizeCheckedWithSeed(VoteAuthorizeCheckedWithSeedArgs),
    }
}
