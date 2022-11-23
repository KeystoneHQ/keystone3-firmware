use crate::errors::{Result, SolanaError};
use crate::parser::detail::{
    CommonDetail, ProgramDetail, ProgramDetailVoteAuthorize, ProgramDetailVoteAuthorizeChecked,
    ProgramDetailVoteAuthorizeCheckedWithSeed, ProgramDetailVoteAuthorizeWithSeed,
    ProgramDetailVoteInitAccount, ProgramDetailVoteUpdateCommission,
    ProgramDetailVoteUpdateValidatorIdentity, ProgramDetailVoteUpdateVoteState,
    ProgramDetailVoteUpdateVoteStateSwitch, ProgramDetailVoteVote, ProgramDetailVoteVoteSwitch,
    ProgramDetailVoteWithdraw, SolanaDetail,
};
use crate::solana_lib::solana_program::hash::Hash;
use crate::solana_lib::solana_program::pubkey::Pubkey;
use crate::solana_lib::solana_program::vote::instruction::VoteInstruction;
use crate::solana_lib::solana_program::vote::state::{
    Lockout, Vote, VoteAuthorize, VoteAuthorizeCheckedWithSeedArgs, VoteAuthorizeWithSeedArgs,
    VoteInit, VoteStateUpdate,
};
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;

static PROGRAM_NAME: &str = "Vote";

pub fn resolve(instruction: VoteInstruction, accounts: Vec<String>) -> Result<SolanaDetail> {
    match instruction {
        VoteInstruction::InitializeAccount(vote_init) => {
            resolve_initialize_account(accounts, vote_init)
        }
        VoteInstruction::Authorize(pubkey, vote_authority) => {
            resolve_authorize(accounts, pubkey, vote_authority)
        }
        VoteInstruction::Vote(vote) => resolve_vote(accounts, vote),
        VoteInstruction::Withdraw(lamports) => resolve_withdraw(accounts, lamports),
        VoteInstruction::UpdateValidatorIdentity => resolve_update_validator_identity(accounts),
        VoteInstruction::UpdateCommission(new_commission) => {
            resolve_update_commission(accounts, new_commission)
        }
        VoteInstruction::VoteSwitch(vote, proof_hash) => {
            resolve_vote_switch(accounts, vote, proof_hash)
        }
        VoteInstruction::AuthorizeChecked(vote_authority) => {
            resolve_authorize_checked(accounts, vote_authority)
        }
        VoteInstruction::UpdateVoteState(state) => resolve_update_vote_state(accounts, state),
        VoteInstruction::UpdateVoteStateSwitch(state, proof_hash) => {
            resolve_update_vote_state_switch(accounts, state, proof_hash)
        }
        VoteInstruction::AuthorizeWithSeed(args) => resolve_authorize_with_seed(accounts, args),
        VoteInstruction::AuthorizeCheckedWithSeed(args) => {
            resolve_authorize_checked_with_seed(accounts, args)
        }
    }
}

fn resolve_initialize_account(accounts: Vec<String>, vote_init: VoteInit) -> Result<SolanaDetail> {
    let method_name = "InitializeAccount".to_string();
    let account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.account",
            method_name
        )))?
        .to_string();
    let sysvar_rent = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_rent",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
            method_name
        )))?
        .to_string();
    let new_validator_identity = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.new_validator_identity",
            method_name
        )))?
        .to_string();
    let node_pubkey = vote_init.node_pubkey.to_string();
    let authorized_voter = vote_init.authorized_voter.to_string();
    let authorized_withdrawer = vote_init.authorized_withdrawer.to_string();
    let commission = vote_init.commission;
    Ok(SolanaDetail {
        common: CommonDetail {
            program: PROGRAM_NAME.to_string(),
            method: method_name,
        },
        kind: ProgramDetail::VoteInitializeAccount(ProgramDetailVoteInitAccount {
            vote_account: account,
            sysvar_rent,
            sysvar_clock,
            new_validator_identity,
            node_pubkey,
            authorized_voter,
            authorized_withdrawer,
            commission,
        }),
    })
}

fn resolve_authorize(
    accounts: Vec<String>,
    pubkey: Pubkey,
    vote_authority: VoteAuthorize,
) -> Result<SolanaDetail> {
    let method_name = "Authorize".to_string();
    let vote_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_account",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
            method_name
        )))?
        .to_string();
    let old_authority_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.old_authority_pubkey",
            method_name
        )))?
        .to_string();
    let authority_type = match vote_authority {
        VoteAuthorize::Voter => "voter",
        VoteAuthorize::Withdrawer => "withdrawer",
    }
    .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            program: PROGRAM_NAME.to_string(),
            method: method_name,
        },
        kind: ProgramDetail::VoteAuthorize(ProgramDetailVoteAuthorize {
            vote_account,
            sysvar_clock,
            old_authority_pubkey,
            new_authority_pubkey: pubkey.to_string(),
            authority_type,
        }),
    })
}

fn resolve_vote(accounts: Vec<String>, vote: Vote) -> Result<SolanaDetail> {
    let method_name = "Vote".to_string();

    let vote_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_account",
            method_name
        )))?
        .to_string();
    let sysvar_slot_hashes = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_slot_hashes",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
            method_name
        )))?
        .to_string();
    let vote_authority_pubkey = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_authority_pubkey",
            method_name
        )))?
        .to_string();
    let vote_slots = vote
        .slots
        .iter()
        .map(|v| v.to_string())
        .collect::<Vec<String>>();
    let vote_hash = vote.hash.to_string();
    let timestamp = vote
        .timestamp
        .map_or_else(|| "".to_string(), |v| v.to_string());
    Ok(SolanaDetail {
        common: CommonDetail {
            program: PROGRAM_NAME.to_string(),
            method: method_name,
        },
        kind: ProgramDetail::VoteVote(ProgramDetailVoteVote {
            vote_account,
            sysvar_slot_hashes,
            sysvar_clock,
            vote_authority_pubkey,
            slots: vote_slots,
            hash: vote_hash,
            timestamp,
        }),
    })
}

fn resolve_withdraw(accounts: Vec<String>, lamports: u64) -> Result<SolanaDetail> {
    let method_name = "Withdraw";

    let vote_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_account",
            method_name
        )))?
        .to_string();
    let recipient = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.recipient",
            method_name
        )))?
        .to_string();
    let withdraw_authority_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.withdraw_authority_pubkey",
            method_name
        )))?
        .to_string();
    let amount = lamports.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::VoteWithdraw(ProgramDetailVoteWithdraw {
            vote_account,
            recipient,
            withdraw_authority_pubkey,
            amount,
        }),
    })
}

fn resolve_update_validator_identity(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "UpdateValidatorIdentity".to_string();

    let vote_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_account",
            method_name
        )))?
        .to_string();
    let new_validator_identity = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.new_validator_identity",
            method_name
        )))?
        .to_string();
    let withdraw_authority_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.withdraw_authority_pubkey",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name,
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::VoteUpdateValidatorIdentity(
            ProgramDetailVoteUpdateValidatorIdentity {
                vote_account,
                new_validator_identity,
                withdraw_authority: withdraw_authority_pubkey,
            },
        ),
    })
}

fn resolve_update_commission(accounts: Vec<String>, new_commission: u8) -> Result<SolanaDetail> {
    let method_name = "UpdateCommission".to_string();

    let vote_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_account",
            method_name
        )))?
        .to_string();
    let withdraw_authority_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.withdraw_authority_pubkey",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::VoteUpdateCommission(ProgramDetailVoteUpdateCommission {
            vote_account,
            withdraw_authority_pubkey,
            new_commission,
        }),
    })
}

fn resolve_vote_switch(
    accounts: Vec<String>,
    vote: Vote,
    proof_hash: Hash,
) -> Result<SolanaDetail> {
    let method_name = "VoteSwitch".to_string();

    let vote_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_account",
            method_name
        )))?
        .to_string();
    let sysvar_slot_hashes = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_slot_hashes",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
            method_name
        )))?
        .to_string();
    let vote_authority_pubkey = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_authority_pubkey",
            method_name
        )))?
        .to_string();
    let vote_slots = vote
        .slots
        .iter()
        .map(|v| v.to_string())
        .collect::<Vec<String>>();
    let vote_hash = vote.hash.to_string();
    let proof_hash = proof_hash.to_string();
    let timestamp = vote
        .timestamp
        .map_or_else(|| "".to_string(), |v| v.to_string());
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::VoteVoteSwitch(ProgramDetailVoteVoteSwitch {
            vote_account,
            sysvar_slot_hashes,
            sysvar_clock,
            vote_authority_pubkey,
            slots: vote_slots,
            hash: vote_hash,
            timestamp,
            proof_hash,
        }),
    })
}

fn resolve_authorize_checked(
    accounts: Vec<String>,
    vote_authority: VoteAuthorize,
) -> Result<SolanaDetail> {
    let method_name = "AuthorizeChecked".to_string();

    let vote_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_account",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
            method_name
        )))?
        .to_string();
    let old_authority_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.old_authority_pubkey",
            method_name
        )))?
        .to_string();
    let new_authority_pubkey = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.new_authority_pubkey",
            method_name
        )))?
        .to_string();
    let authority_type = match vote_authority {
        VoteAuthorize::Voter => "voter",
        VoteAuthorize::Withdrawer => "withdrawer",
    }
    .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::VoteAuthorizeChecked(ProgramDetailVoteAuthorizeChecked {
            vote_account,
            sysvar_clock,
            old_authority_pubkey,
            new_authority_pubkey,
            authority_type,
        }),
    })
}

fn resolve_update_vote_state(
    accounts: Vec<String>,
    state: VoteStateUpdate,
) -> Result<SolanaDetail> {
    let method_name = "UpdateVoteState".to_string();

    let vote_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_account",
            method_name
        )))?
        .to_string();
    let vote_authority_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_authority_pubkey",
            method_name
        )))?
        .to_string();

    let lockouts = state
        .lockouts
        .iter()
        .map(|v| Lockout {
            slot: v.slot,
            confirmation_count: v.confirmation_count,
        })
        .collect::<Vec<Lockout>>();
    let root = state.root.map_or_else(|| "".to_string(), |v| v.to_string());
    let hash = state.hash.to_string();
    let timestamp = state
        .timestamp
        .map_or_else(|| "".to_string(), |v| v.to_string());
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::VoteUpdateVoteState(ProgramDetailVoteUpdateVoteState {
            vote_account,
            vote_authority_pubkey,
            lockouts,
            root,
            hash,
            timestamp,
        }),
    })
}

fn resolve_update_vote_state_switch(
    accounts: Vec<String>,
    state: VoteStateUpdate,
    proof_hash: Hash,
) -> Result<SolanaDetail> {
    let method_name = "UpdateVoteStateSwitch".to_string();

    let vote_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_account",
            method_name
        )))?
        .to_string();
    let vote_authority_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_authority_pubkey",
            method_name
        )))?
        .to_string();
    let lockouts = state
        .lockouts
        .iter()
        .map(|v| Lockout {
            slot: v.slot,
            confirmation_count: v.confirmation_count,
        })
        .collect::<Vec<Lockout>>();
    let root = state.root.map_or_else(|| "".to_string(), |v| v.to_string());
    let hash = state.hash.to_string();
    let timestamp = state
        .timestamp
        .map_or_else(|| "".to_string(), |v| v.to_string());
    let proof_hash = proof_hash.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::VoteUpdateVoteStateSwitch(ProgramDetailVoteUpdateVoteStateSwitch {
            vote_account,
            vote_authority_pubkey,
            lockouts,
            root,
            hash,
            timestamp,
            proof_hash,
        }),
    })
}

fn resolve_authorize_with_seed(
    accounts: Vec<String>,
    args: VoteAuthorizeWithSeedArgs,
) -> Result<SolanaDetail> {
    let method_name = "AuthorizeWithSeed".to_string();

    let vote_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_account",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
            method_name
        )))?
        .to_string();
    let old_base_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.old_base_pubkey",
            method_name
        )))?
        .to_string();

    let authorization_type = match args.authorization_type {
        VoteAuthorize::Voter => "voter",
        VoteAuthorize::Withdrawer => "withdrawer",
    }
    .to_string();
    let current_authority_derived_key_owner = args.current_authority_derived_key_owner.to_string();
    let current_authority_derived_key_seed = args.current_authority_derived_key_seed;
    let new_authority_pubkey = args.new_authority.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::VoteAuthorizeWithSeed(ProgramDetailVoteAuthorizeWithSeed {
            vote_account,
            sysvar_clock,
            old_base_pubkey,
            authorization_type,
            current_authority_derived_key_owner,
            current_authority_derived_key_seed,
            new_authority_pubkey,
        }),
    })
}

fn resolve_authorize_checked_with_seed(
    accounts: Vec<String>,
    args: VoteAuthorizeCheckedWithSeedArgs,
) -> Result<SolanaDetail> {
    let method_name = "AuthorizeCheckedWithSeed".to_string();

    let vote_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_account",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
            method_name
        )))?
        .to_string();
    let old_base_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.old_base_pubkey",
            method_name
        )))?
        .to_string();
    let new_authority_pubkey = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.new_authority_pubkey",
            method_name
        )))?
        .to_string();

    let authorization_type = match args.authorization_type {
        VoteAuthorize::Voter => "voter",
        VoteAuthorize::Withdrawer => "withdrawer",
    }
    .to_string();
    let current_authority_derived_key_owner = args.current_authority_derived_key_owner.to_string();
    let current_authority_derived_key_seed = args.current_authority_derived_key_seed;
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::VoteAuthorizeCheckedWithSeed(
            ProgramDetailVoteAuthorizeCheckedWithSeed {
                vote_account,
                sysvar_clock,
                old_base_pubkey,
                new_authority_pubkey,
                authorization_type,
                current_authority_derived_key_owner,
                current_authority_derived_key_seed,
            },
        ),
    })
}
