use alloc::string::String;
use alloc::vec::Vec;

use borsh::{from_slice, BorshDeserialize};
use serde_derive::Serialize;

use crate::solana_lib::solana_program::errors::ProgramError;
use crate::solana_lib::solana_program::pubkey::Pubkey;
use crate::solana_lib::squads_v4::errors::SquadsV4Error;
use crate::solana_lib::squads_v4::util::{sighash, SIGHASH_GLOBAL_NAMESPACE};

#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub struct MultisigCreateArgs {
    /// The authority that can configure the multisig: add/remove members, change the threshold, etc.
    /// Should be set to `None` for autonomous multisigs.
    pub config_authority: Option<Pubkey>,
    /// The number of signatures required to execute a transaction.
    pub threshold: u16,
    /// The members of the multisig.
    pub members: Vec<Member>,
    /// How many seconds must pass between transaction voting, settlement, and execution.
    pub time_lock: u32,
    /// Memo is used for indexing only.
    pub memo: Option<String>,
}

#[derive(BorshDeserialize, Serialize, Debug, Clone)]
#[borsh(crate = "borsh")]
pub struct Member {
    pub key: Pubkey,
    pub permissions: Permissions,
}
#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub struct Permissions {
    pub mask: u8,
}

#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub struct MultisigCreateArgsV2 {
    /// The authority that can configure the multisig: add/remove members, change the threshold, etc.
    /// Should be set to `None` for autonomous multisigs.
    pub config_authority: Option<Pubkey>,
    /// The number of signatures required to execute a transaction.
    pub threshold: u16,
    /// The members of the multisig.
    pub members: Vec<Member>,
    /// How many seconds must pass between transaction voting, settlement, and execution.
    pub time_lock: u32,
    /// The address where the rent for the accounts related to executed, rejected, or cancelled
    /// transactions can be reclaimed. If set to `None`, the rent reclamation feature is turned off.
    pub rent_collector: Option<Pubkey>,
    /// Memo is used for indexing only.
    pub memo: Option<String>,
}
#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub struct ProposalCreateArgs {
    /// Index of the multisig transaction this proposal is associated with.
    pub transaction_index: u64,
    /// Whether the proposal should be initialized with status `Draft`.
    pub draft: bool,
}
#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub struct VaultTransactionCreateArgs {
    /// Index of the vault this transaction belongs to.
    pub vault_index: u8,
    /// Number of ephemeral signing PDAs required by the transaction.
    pub ephemeral_signers: u8,
    pub transaction_message: Vec<u8>,
    pub memo: Option<String>,
}

#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
// #[borsh(crate = "borsh")]
pub struct ProposalVoteArgs {
    pub memo: Option<String>,
}

#[derive(Debug)]
pub enum SquadsInstructions {
    MultisigCreate(MultisigCreateArgs),
    MultisigCreateV2(MultisigCreateArgsV2),
    ProposalActivate,
    ProposalCreate(ProposalCreateArgs),
    ProposalApprove(ProposalVoteArgs),
    ProposalCancel(ProposalVoteArgs),
    ProposalReject(ProposalVoteArgs),
    VaultTransactionCreate(VaultTransactionCreateArgs),
    VaultTransactionExecute,
}
pub trait Dispatch {
    fn dispatch(data: &[u8]) -> Result<Self, ProgramError>
    where
        Self: Sized;
}
impl Dispatch for SquadsInstructions {
    fn dispatch(instrucion_data: &[u8]) -> Result<Self, ProgramError> {
        let data = instrucion_data;
        let ix_type = &data[..8];
        let ix_data = &data[8..];
        match hex::encode(ix_type).as_str() {
            "7a4d509f54585ac5" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE, "multisig_create")
                Ok(SquadsInstructions::MultisigCreate(
                    from_slice::<MultisigCreateArgs>(ix_data).unwrap(),
                ))
            }
            "32ddc75d28f58be9" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE, "multisig_create_v2")
                Ok(SquadsInstructions::MultisigCreateV2(
                    from_slice::<MultisigCreateArgsV2>(ix_data).unwrap(),
                ))
            }
            "dc3c49e01e6c4f9f" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_create")
                Ok(SquadsInstructions::ProposalCreate(
                    from_slice::<ProposalCreateArgs>(ix_data).unwrap(),
                ))
            }
            "0b225cf89a1b336a" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_activate")
                Ok(SquadsInstructions::ProposalActivate)
            }
            "9025a488bcd82af8" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_approve")
                Ok(SquadsInstructions::ProposalApprove(
                    from_slice::<ProposalVoteArgs>(ix_data).unwrap(),
                ))
            }
            "1b2a7fed26a354cb" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_cancel")
                Ok(SquadsInstructions::ProposalCancel(
                    from_slice::<ProposalVoteArgs>(ix_data).unwrap(),
                ))
            }

            "f33e869ce66af687" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_reject")
                Ok(SquadsInstructions::ProposalReject(
                    from_slice::<ProposalVoteArgs>(ix_data).unwrap(),
                ))
            }
            "30fa4ea8d0e2dad3" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE,vault_transaction_create
                Ok(SquadsInstructions::VaultTransactionCreate(
                    from_slice::<VaultTransactionCreateArgs>(ix_data).unwrap(),
                ))
            }
            "c208a15799a419ab" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE,VaultTransactionExecute)
                Ok(SquadsInstructions::VaultTransactionExecute)
            }
            _ => Err(SquadsV4Error::UnknownSquadV4Instruction.into()),
        }
    }
}
#[cfg(test)]
mod tests {
    use std::prelude::rust_2024::ToString;

    use super::*;

    #[test]
    fn test_parse_instruction_data() {
        let data = "7a4d509f54585ac5000200050000002b8d8b3addd92759f55b840c2852f5bd50aee3552fe987ee0d4fe24b9043df8e07f219076b2850cbb770c807661d874e09a7224de024d6579e43cc1df392a12244073b08df2ea93b9fc9ecb8a965773e18c6c8c4f66696dda8eb6ea61ca420700c5607f8b770467b0eaae4e081f7e4b66db848c91d63a4f3de46092fe5ccff4427dec50793479bb7ee58060b82e4bdba7ec1a026bacabbb96a8d1c72f21f2a1dd98ad8de070000000001300000007b226e223a22544553544d554c5449534947222c2264223a2254455354204d554c544920534947222c2269223a22227d";
        let data = hex::decode(data).unwrap();
        let instr = SquadsInstructions::dispatch(&data).unwrap();
        match instr {
            SquadsInstructions::MultisigCreate(multisig_args) => {
                assert_eq!(multisig_args.config_authority, None);
                assert_eq!(multisig_args.threshold, 2);
                assert_eq!(multisig_args.time_lock, 0);
                assert_eq!(multisig_args.members.len(), 5);
                assert_eq!(
                    multisig_args.memo,
                    Some(
                        "{\"n\":\"TESTMULTISIG\",\"d\":\"TEST MULTI SIG\",\"i\":\"\"}".to_string()
                    )
                );
            }
            SquadsInstructions::ProposalCreate(proposal_args) => {
                println!("{:?}", proposal_args);
            }
            SquadsInstructions::VaultTransactionCreate(vault_tx_args) => {
                println!("{:?}", vault_tx_args);
            }
            _ => {}
        }
    }
    #[test]
    fn test_calculate_instruction_sighash() {
        let multisig_create_sighash = sighash(SIGHASH_GLOBAL_NAMESPACE, "multisig_create");
        let proposal_create_sighash = sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_create");
        assert_eq!("7a4d509f54585ac5", hex::encode(multisig_create_sighash));
        assert_eq!("dc3c49e01e6c4f9f", hex::encode(proposal_create_sighash));
    }
}
