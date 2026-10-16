use alloc::string::String;
use alloc::vec::Vec;

use borsh::{from_slice, BorshDeserialize};
use serde_derive::Serialize;

use crate::solana_lib::solana_program::errors::ProgramError;
use crate::solana_lib::solana_program::pubkey::Pubkey;
use crate::solana_lib::squads_v4::errors::SquadsV4Error;
use crate::solana_lib::traits::Dispatch;

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

#[derive(BorshDeserialize, Serialize, Debug, Clone)]
pub struct ConfigTransactionCreateArgs {
    pub actions: Vec<ConfigAction>,
    pub memo: Option<String>,
}

fn serialize_pubkey<S: serde::Serializer>(
    key: &Pubkey,
    serializer: S,
) -> core::result::Result<S::Ok, S::Error> {
    serializer.collect_str(key)
}

fn serialize_pubkeys<S: serde::Serializer>(
    keys: &[Pubkey],
    serializer: S,
) -> core::result::Result<S::Ok, S::Error> {
    use alloc::string::ToString;
    serde::Serialize::serialize(
        &keys.iter().map(ToString::to_string).collect::<Vec<_>>(),
        serializer,
    )
}

fn serialize_optional_pubkey<S: serde::Serializer>(
    key: &Option<Pubkey>,
    serializer: S,
) -> core::result::Result<S::Ok, S::Error> {
    use alloc::string::ToString;
    serde::Serialize::serialize(&key.as_ref().map(ToString::to_string), serializer)
}

#[derive(BorshDeserialize, Serialize, Debug, Clone)]
pub struct ConfigMember {
    #[serde(serialize_with = "serialize_pubkey")]
    pub key: Pubkey,
    pub permissions: Permissions,
}

#[derive(BorshDeserialize, Serialize, Debug, Clone)]
pub enum ConfigAction {
    AddMember {
        new_member: ConfigMember,
    },
    RemoveMember {
        #[serde(serialize_with = "serialize_pubkey")]
        old_member: Pubkey,
    },
    ChangeThreshold {
        new_threshold: u16,
    },
    SetTimeLock {
        new_time_lock: u32,
    },
    AddSpendingLimit {
        #[serde(serialize_with = "serialize_pubkey")]
        create_key: Pubkey,
        vault_index: u8,
        #[serde(serialize_with = "serialize_pubkey")]
        mint: Pubkey,
        amount: u64,
        period: SpendingLimitPeriod,
        #[serde(serialize_with = "serialize_pubkeys")]
        members: Vec<Pubkey>,
        #[serde(serialize_with = "serialize_pubkeys")]
        destinations: Vec<Pubkey>,
    },
    RemoveSpendingLimit {
        #[serde(serialize_with = "serialize_pubkey")]
        spending_limit: Pubkey,
    },
    SetRentCollector {
        #[serde(serialize_with = "serialize_optional_pubkey")]
        new_rent_collector: Option<Pubkey>,
    },
}

#[derive(BorshDeserialize, Serialize, Debug, Clone)]
pub enum SpendingLimitPeriod {
    OneTime,
    Day,
    Week,
    Month,
}

#[derive(Debug)]
pub enum SquadsInstructions {
    ConfigTransactionCreate(ConfigTransactionCreateArgs),
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

impl Dispatch for SquadsInstructions {
    fn dispatch(instrucion_data: &[u8]) -> Result<Self, ProgramError> {
        let data = instrucion_data;
        if data.len() < 8 {
            return Err(SquadsV4Error::InvalidInstruction.into());
        }
        let ix_type = &data[..8];
        let ix_data = &data[8..];
        match hex::encode(ix_type).as_str() {
            "9bec57e4894b5127" => {
                let args = from_slice::<ConfigTransactionCreateArgs>(ix_data)
                    .map_err(|_| SquadsV4Error::InvalidInstruction)?;
                if args.actions.is_empty()
                    || args
                        .actions
                        .iter()
                        .filter(|action| matches!(action, ConfigAction::ChangeThreshold { .. }))
                        .count()
                        > 1
                {
                    return Err(SquadsV4Error::InvalidInstruction.into());
                }
                Ok(SquadsInstructions::ConfigTransactionCreate(args))
            }
            "7a4d509f54585ac5" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE, "multisig_create")
                Ok(SquadsInstructions::MultisigCreate(
                    from_slice::<MultisigCreateArgs>(ix_data)
                        .map_err(|_| SquadsV4Error::InvalidInstruction)?,
                ))
            }
            "32ddc75d28f58be9" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE, "multisig_create_v2")
                Ok(SquadsInstructions::MultisigCreateV2(
                    from_slice::<MultisigCreateArgsV2>(ix_data)
                        .map_err(|_| SquadsV4Error::InvalidInstruction)?,
                ))
            }
            "dc3c49e01e6c4f9f" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_create")
                Ok(SquadsInstructions::ProposalCreate(
                    from_slice::<ProposalCreateArgs>(ix_data)
                        .map_err(|_| SquadsV4Error::InvalidInstruction)?,
                ))
            }
            "0b225cf89a1b336a" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_activate")
                if !ix_data.is_empty() {
                    return Err(SquadsV4Error::InvalidInstruction.into());
                }
                Ok(SquadsInstructions::ProposalActivate)
            }
            "9025a488bcd82af8" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_approve")
                Ok(SquadsInstructions::ProposalApprove(
                    from_slice::<ProposalVoteArgs>(ix_data)
                        .map_err(|_| SquadsV4Error::InvalidInstruction)?,
                ))
            }
            "1b2a7fed26a354cb" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_cancel")
                Ok(SquadsInstructions::ProposalCancel(
                    from_slice::<ProposalVoteArgs>(ix_data)
                        .map_err(|_| SquadsV4Error::InvalidInstruction)?,
                ))
            }

            "f33e869ce66af687" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_reject")
                Ok(SquadsInstructions::ProposalReject(
                    from_slice::<ProposalVoteArgs>(ix_data)
                        .map_err(|_| SquadsV4Error::InvalidInstruction)?,
                ))
            }
            "30fa4ea8d0e2dad3" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE,vault_transaction_create
                Ok(SquadsInstructions::VaultTransactionCreate(
                    from_slice::<VaultTransactionCreateArgs>(ix_data)
                        .map_err(|_| SquadsV4Error::InvalidInstruction)?,
                ))
            }
            "c208a15799a419ab" => {
                // sighash(SIGHASH_GLOBAL_NAMESPACE,VaultTransactionExecute)
                if !ix_data.is_empty() {
                    return Err(SquadsV4Error::InvalidInstruction.into());
                }
                Ok(SquadsInstructions::VaultTransactionExecute)
            }
            _ => Err(SquadsV4Error::UnknownSquadV4Instruction.into()),
        }
    }
}
#[cfg(test)]
mod tests {
    use std::prelude::rust_2024::ToString;

    use crate::solana_lib::utils::{sighash, SIGHASH_GLOBAL_NAMESPACE};

    use super::*;

    #[test]
    fn rejects_repeated_threshold_changes_even_when_equal_or_separated() {
        for actions in [
            vec![2, 3, 0, 2, 1, 0],
            vec![2, 3, 0, 2, 3, 0],
            vec![2, 3, 0, 3, 60, 0, 0, 0, 2, 1, 0],
        ] {
            let count = if actions.len() == 6 { 2u32 } else { 3u32 };
            let mut data = hex::decode("9bec57e4894b5127").unwrap();
            data.extend_from_slice(&count.to_le_bytes());
            data.extend_from_slice(&actions);
            data.push(0);
            assert!(SquadsInstructions::dispatch(&data).is_err());
        }
        let data = hex::decode("9bec57e4894b512702000000020300033c00000000").unwrap();
        assert!(SquadsInstructions::dispatch(&data).is_ok());
    }

    #[test]
    fn parses_config_transaction_create_and_rejects_malformed_fields() {
        let data = hex::decode("9bec57e4894b51270100000001902cce4c0210897aa154b0529cdf3759e940e07449f7176e4656ca468ace8fbf010400000074657374").unwrap();
        let SquadsInstructions::ConfigTransactionCreate(args) =
            SquadsInstructions::dispatch(&data).unwrap()
        else {
            panic!("wrong instruction");
        };
        assert_eq!(args.memo.as_deref(), Some("test"));
        assert_eq!(args.actions.len(), 1);
        assert!(matches!(args.actions[0], ConfigAction::RemoveMember { .. }));
        let json = serde_json::to_value(&args).unwrap();
        assert!(json["actions"][0]["RemoveMember"]["old_member"].is_string());
        for length in 0..data.len() {
            assert!(SquadsInstructions::dispatch(&data[..length]).is_err());
        }
        let mut padded = data.clone();
        padded.push(0);
        assert!(SquadsInstructions::dispatch(&padded).is_err());
        for action in [7u8, 255] {
            let mut malformed = data.clone();
            malformed[12] = action;
            assert!(SquadsInstructions::dispatch(&malformed).is_err());
        }
        let mut empty = data[..8].to_vec();
        empty.extend_from_slice(&[0, 0, 0, 0, 0]);
        assert!(SquadsInstructions::dispatch(&empty).is_err());
    }

    #[test]
    fn decodes_all_config_action_wire_variants() {
        let key = [42u8; 32];
        let mut add_member = vec![0];
        add_member.extend_from_slice(&key);
        add_member.push(7);
        let mut remove_member = vec![1];
        remove_member.extend_from_slice(&key);
        let mut spending = vec![4];
        spending.extend_from_slice(&key);
        spending.push(0);
        spending.extend_from_slice(&key);
        spending.extend_from_slice(&100u64.to_le_bytes());
        spending.push(1);
        spending.extend_from_slice(&1u32.to_le_bytes());
        spending.extend_from_slice(&key);
        spending.extend_from_slice(&0u32.to_le_bytes());
        let mut remove_spending = vec![5];
        remove_spending.extend_from_slice(&key);
        let mut rent = vec![6, 1];
        rent.extend_from_slice(&key);
        for (action, name) in [
            (add_member, "AddMember"),
            (remove_member, "RemoveMember"),
            (vec![2, 2, 0], "ChangeThreshold"),
            (vec![3, 60, 0, 0, 0], "SetTimeLock"),
            (spending, "AddSpendingLimit"),
            (remove_spending, "RemoveSpendingLimit"),
            (rent, "SetRentCollector"),
            (vec![6, 0], "SetRentCollector"),
        ] {
            let mut data = hex::decode("9bec57e4894b5127").unwrap();
            data.extend_from_slice(&1u32.to_le_bytes());
            data.extend_from_slice(&action);
            data.push(0);
            let SquadsInstructions::ConfigTransactionCreate(args) =
                SquadsInstructions::dispatch(&data).unwrap()
            else {
                panic!("wrong instruction");
            };
            let json = serde_json::to_value(&args).unwrap();
            assert!(json["actions"][0].get(name).is_some());
        }
    }

    #[test]
    fn no_argument_instructions_reject_trailing_bytes() {
        for discriminator in ["0b225cf89a1b336a", "c208a15799a419ab"] {
            let data = hex::decode(discriminator).unwrap();
            assert!(SquadsInstructions::dispatch(&data).is_ok());
            for length in 0..8 {
                assert!(SquadsInstructions::dispatch(&data[..length]).is_err());
            }
            for suffix in [&[0][..], &[1, 2, 3][..]] {
                let mut malformed = data.clone();
                malformed.extend(suffix);
                assert!(SquadsInstructions::dispatch(&malformed).is_err());
            }
        }
    }

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
        let multisig_create_sighash_v2 = sighash(SIGHASH_GLOBAL_NAMESPACE, "multisig_create_v2");
        let proposal_create_sighash = sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_create");
        assert_eq!("7a4d509f54585ac5", hex::encode(multisig_create_sighash));
        assert_eq!("32ddc75d28f58be9", hex::encode(multisig_create_sighash_v2));
        assert_eq!("dc3c49e01e6c4f9f", hex::encode(proposal_create_sighash));
    }
}
