use crate::errors::{Result, SolanaError};
use crate::parser::detail::{
    CommonDetail, ProgramDetail, ProgramDetailStakeAuthorize, ProgramDetailStakeAuthorizeChecked,
    ProgramDetailStakeAuthorizeCheckedWithSeed, ProgramDetailStakeAuthorizeWithSeed,
    ProgramDetailStakeDeactivate, ProgramDetailStakeDeactivateDelinquent,
    ProgramDetailStakeDelegateStake, ProgramDetailStakeGetMinimumDelegation,
    ProgramDetailStakeInitialize, ProgramDetailStakeInitializeChecked, ProgramDetailStakeMerge,
    ProgramDetailStakeSetLockup, ProgramDetailStakeSetLockupChecked, ProgramDetailStakeSplit,
    ProgramDetailStakeWithdraw, SolanaDetail,
};
use crate::solana_lib::solana_program::clock::{Epoch, UnixTimestamp};
use crate::solana_lib::solana_program::pubkey::Pubkey;
use crate::solana_lib::solana_program::stake::instruction::{
    AuthorizeCheckedWithSeedArgs, AuthorizeWithSeedArgs, LockupArgs, LockupCheckedArgs,
    StakeInstruction,
};
use crate::solana_lib::solana_program::stake::state::{Authorized, Lockup, StakeAuthorize};
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;

static PROGRAM_NAME: &str = "Stake";

pub fn resolve(instruction: StakeInstruction, accounts: Vec<String>) -> Result<SolanaDetail> {
    match instruction {
        StakeInstruction::Initialize(authorized, lockup) => {
            resolve_initialize(accounts, authorized, lockup)
        }
        StakeInstruction::Authorize(pubkey, stake_authorize) => {
            resolve_authorize(accounts, pubkey, stake_authorize)
        }
        StakeInstruction::DelegateStake => resolve_delegate_stake(accounts),
        StakeInstruction::Split(lamports) => resolve_split(accounts, lamports),
        StakeInstruction::Withdraw(lamports) => resolve_withdraw(accounts, lamports),
        StakeInstruction::Deactivate => resolve_deactivate(accounts),
        StakeInstruction::SetLockup(lockup) => resolve_set_lockup(accounts, lockup),
        StakeInstruction::Merge => resolve_merge(accounts),
        StakeInstruction::AuthorizeWithSeed(args) => resolve_authorize_with_seed(accounts, args),
        StakeInstruction::InitializeChecked => resolve_initialize_checked(accounts),
        StakeInstruction::AuthorizeChecked(stake_authorize) => {
            resolve_authorize_checked(accounts, stake_authorize)
        }
        StakeInstruction::AuthorizeCheckedWithSeed(args) => {
            resolve_authorize_checked_with_seed(accounts, args)
        }
        StakeInstruction::SetLockupChecked(args) => resolve_set_lockup_checked(accounts, args),
        StakeInstruction::GetMinimumDelegation => Ok(SolanaDetail {
            common: CommonDetail {
                method: "GetMinimumDelegation".to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::StakeGetMinimumDelegation(
                ProgramDetailStakeGetMinimumDelegation {},
            ),
        }),
        StakeInstruction::DeactivateDelinquent => Ok(SolanaDetail {
            common: CommonDetail {
                method: "DeactivateDelinquent".to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::StakeDeactivateDelinquent(
                ProgramDetailStakeDeactivateDelinquent {},
            ),
        }),
    }
}

fn resolve_initialize(
    accounts: Vec<String>,
    authorized: Authorized,
    lockup: Lockup,
) -> Result<SolanaDetail> {
    let method_name = "Initialize".to_string();
    let stake_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_account",
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
    let staker = authorized.staker.to_string();
    let withdrawer = authorized.withdrawer.to_string();
    let timestamp = lockup.unix_timestamp;
    let epoch = lockup.epoch;
    let custodian = lockup.custodian.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::StakeInitialize(ProgramDetailStakeInitialize {
            stake_account,
            sysvar_rent,
            staker,
            withdrawer,
            timestamp,
            epoch,
            custodian,
        }),
    })
}

fn resolve_authorize(
    accounts: Vec<String>,
    pubkey: Pubkey,
    stake_authorize: StakeAuthorize,
) -> Result<SolanaDetail> {
    let method_name = "Authorize".to_string();
    let stake_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_account",
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
            "Authorize.old_authority_pubkey"
        )))?
        .to_string();
    let lockup_authority_pubkey = accounts.get(3).unwrap_or(&"".to_string()).to_string();
    let new_authority_pubkey = pubkey.to_string();
    let authorize_type = match stake_authorize {
        StakeAuthorize::Staker => "staker",
        StakeAuthorize::Withdrawer => "withdrawer",
    }
    .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::StakeAuthorize(ProgramDetailStakeAuthorize {
            stake_account,
            sysvar_clock,
            old_authority_pubkey,
            lockup_authority_pubkey,
            new_authority_pubkey,
            authorize_type,
        }),
    })
}

fn resolve_delegate_stake(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "DelegateStake".to_string();
    let stake_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_account",
            method_name
        )))?
        .to_string();
    let vote_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.vote_account",
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
    let sysvar_stake_history = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_stake_history",
            method_name
        )))?
        .to_string();
    let config_account = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.config_account",
            method_name
        )))?
        .to_string();
    let stake_authority_pubkey = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_authority_pubkey",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            program: PROGRAM_NAME.to_string(),
            method: method_name,
        },
        kind: ProgramDetail::StakeDelegateStake(ProgramDetailStakeDelegateStake {
            stake_account,
            vote_account,
            sysvar_clock,
            sysvar_stake_history,
            config_account,
            stake_authority_pubkey,
        }),
    })
}

fn resolve_split(accounts: Vec<String>, lamports: u64) -> Result<SolanaDetail> {
    let method_name = "Split".to_string();
    let stake_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_account",
            method_name
        )))?
        .to_string();
    let target_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.target_account",
            method_name
        )))?
        .to_string();
    let stake_authority_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_authority_pubkey",
            method_name
        )))?
        .to_string();
    let amount = lamports.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::StakeSplit(ProgramDetailStakeSplit {
            stake_account,
            target_account,
            stake_authority_pubkey,
            amount,
        }),
    })
}

fn resolve_withdraw(accounts: Vec<String>, lamports: u64) -> Result<SolanaDetail> {
    let method_name = "Withdraw".to_string();
    let stake_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_account",
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
    let sysvar_clock = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
            method_name
        )))?
        .to_string();
    let sysvar_stake_history = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_stake_history",
            method_name
        )))?
        .to_string();
    let withdraw_authority_pubkey = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.withdraw_authority_pubkey",
            method_name
        )))?
        .to_string();
    let stake_authority_pubkey = accounts.get(5).unwrap_or(&"".to_string()).to_string();
    let amount = lamports.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            program: PROGRAM_NAME.to_string(),
            method: method_name,
        },
        kind: ProgramDetail::StakeWithdraw(ProgramDetailStakeWithdraw {
            stake_account,
            recipient,
            sysvar_clock,
            sysvar_stake_history,
            withdraw_authority_pubkey,
            stake_authority_pubkey,
            amount,
        }),
    })
}

fn resolve_deactivate(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "Deactivate".to_string();
    let delegated_stake_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.delegated_stake_account",
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
    let stake_authority_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_authority_pubkey",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::StakeDeactivate(ProgramDetailStakeDeactivate {
            delegated_stake_account,
            sysvar_clock,
            stake_authority_pubkey,
        }),
    })
}

fn resolve_set_lockup(accounts: Vec<String>, lockup: LockupArgs) -> Result<SolanaDetail> {
    let method_name = "SetLockup".to_string();
    let stake_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_account",
            method_name
        )))?
        .to_string();
    let lockup_authority_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lockup_authority_pubkey",
            method_name
        )))?
        .to_string();
    let unix_timestamp = lockup.unix_timestamp.unwrap_or(UnixTimestamp::default());
    let epoch = lockup.epoch.unwrap_or(Epoch::default());
    let custodian = lockup
        .custodian
        .map_or_else(|| "".to_string(), |v| v.to_string());
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::StakeSetLockup(ProgramDetailStakeSetLockup {
            stake_account,
            lockup_authority_pubkey,
            unix_timestamp,
            epoch,
            custodian,
        }),
    })
}

fn resolve_merge(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "Merge".to_string();
    let destination_stake_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.destination_stake_account",
            method_name
        )))?
        .to_string();
    let source_stake_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_stake_account",
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
    let sysvar_stake_history = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_stake_history",
            method_name
        )))?
        .to_string();
    let stake_authority_pubkey = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_authority_pubkey",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::StakeMerge(ProgramDetailStakeMerge {
            destination_stake_account,
            source_stake_account,
            sysvar_clock,
            sysvar_stake_history,
            stake_authority_pubkey,
        }),
    })
}

fn resolve_authorize_with_seed(
    accounts: Vec<String>,
    args: AuthorizeWithSeedArgs,
) -> Result<SolanaDetail> {
    let method_name = "AuthorizeWithSeed".to_string();
    let stake_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_account",
            method_name
        )))?
        .to_string();
    let old_base_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.old_base_pubkey",
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
    let lockup_authority_pubkey = accounts.get(3).unwrap_or(&"".to_string()).to_string();

    let new_authority_pubkey = args.new_authorized_pubkey.to_string();
    let stake_authorize = match args.stake_authorize {
        StakeAuthorize::Staker => "staker",
        StakeAuthorize::Withdrawer => "withdrawer",
    }
    .to_string();
    let authority_owner = args.authority_owner.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::StakeAuthorizeWithSeed(ProgramDetailStakeAuthorizeWithSeed {
            stake_account,
            old_base_pubkey,
            sysvar_clock,
            lockup_authority_pubkey,
            new_authority_pubkey,
            authorize_type: stake_authorize,
            authority_seed: args.authority_seed,
            authority_owner,
        }),
    })
}

fn resolve_initialize_checked(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "InitializeChecked".to_string();
    let stake_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_account",
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
    let stake_authority_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_authority_pubkey",
            method_name
        )))?
        .to_string();
    let withdraw_authority_pubkey = accounts
        .get(3)
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
        kind: ProgramDetail::StakeInitializeChecked(ProgramDetailStakeInitializeChecked {
            stake_account,
            sysvar_rent,
            stake_authority_pubkey,
            withdraw_authority_pubkey,
        }),
    })
}

fn resolve_authorize_checked(
    accounts: Vec<String>,
    stake_authorize: StakeAuthorize,
) -> Result<SolanaDetail> {
    let method_name = "AuthorizeChecked".to_string();
    let stake_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_account",
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
    let lockup_authority_pubkey = accounts.get(4).unwrap_or(&"".to_string()).to_string();
    let authority_type = match stake_authorize {
        StakeAuthorize::Staker => "staker",
        StakeAuthorize::Withdrawer => "withdrawer",
    }
    .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::StakeAuthorizeChecked(ProgramDetailStakeAuthorizeChecked {
            stake_account,
            sysvar_clock,
            old_authority_pubkey,
            new_authority_pubkey,
            lockup_authority_pubkey,
            authority_type,
        }),
    })
}

fn resolve_authorize_checked_with_seed(
    accounts: Vec<String>,
    args: AuthorizeCheckedWithSeedArgs,
) -> Result<SolanaDetail> {
    let method_name = "AuthorizeCheckedWithSeed".to_string();
    let stake_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_account",
            method_name
        )))?
        .to_string();
    let old_base_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.old_base_pubkey",
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
    let new_authority_pubkey = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.new_authority_pubkey",
            method_name
        )))?
        .to_string();
    let lockup_authority = accounts.get(4).unwrap_or(&"".to_string()).to_string();
    let authority_type = match args.stake_authorize {
        StakeAuthorize::Staker => "staker",
        StakeAuthorize::Withdrawer => "withdrawer",
    }
    .to_string();
    let authority_seed = args.authority_seed;
    let authority_owner = args.authority_owner.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::StakeAuthorizeCheckedWithSeed(
            ProgramDetailStakeAuthorizeCheckedWithSeed {
                stake_account,
                sysvar_clock,
                old_base_pubkey,
                new_authority_pubkey,
                lockup_authority,
                authority_type,
                authority_seed,
                authority_owner,
            },
        ),
    })
}

fn resolve_set_lockup_checked(
    accounts: Vec<String>,
    args: LockupCheckedArgs,
) -> Result<SolanaDetail> {
    let method_name = "SetLockupChecked".to_string();
    let stake_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.stake_account",
            method_name
        )))?
        .to_string();
    let lockup_authority_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.authority_pubkey",
            method_name
        )))?
        .to_string();
    let new_lockup_authority_pubkey = accounts.get(2).unwrap_or(&"".to_string()).to_string();

    let timestamp = args.unix_timestamp.unwrap_or(UnixTimestamp::default());
    let epoch = args.epoch.unwrap_or(Epoch::default());
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::StakeSetLockupChecked(ProgramDetailStakeSetLockupChecked {
            stake_account,
            lockup_authority_pubkey,
            new_lockup_authority_pubkey,
            timestamp,
            epoch,
        }),
    })
}
