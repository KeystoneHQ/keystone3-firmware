use crate::errors::{Result, SolanaError};
use crate::parser::detail::{
    CommonDetail, ProgramDetail, ProgramDetailSystemAdvanceNonceAccount,
    ProgramDetailSystemAllocate, ProgramDetailSystemAllocateWithSeed, ProgramDetailSystemAssign,
    ProgramDetailSystemAssignWithSeed, ProgramDetailSystemAuthorizeNonceAccount,
    ProgramDetailSystemCreateAccount, ProgramDetailSystemCreateAccountWithSeed,
    ProgramDetailSystemInitializeNonceAccount, ProgramDetailSystemTransfer,
    ProgramDetailSystemTransferWithSeed, ProgramDetailSystemUpgradeNonceAccount,
    ProgramDetailSystemWithdrawNonceAccount, SolanaDetail,
};
use crate::resolvers::format_amount;
use crate::solana_lib::solana_program::pubkey::Pubkey;
use crate::solana_lib::solana_program::system_instruction::SystemInstruction;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;

static PROGRAM_NAME: &str = "System";

pub fn resolve(instruction: SystemInstruction, accounts: Vec<String>) -> Result<SolanaDetail> {
    match instruction {
        SystemInstruction::CreateAccount {
            lamports,
            space,
            owner,
        } => resolve_create_account(accounts, lamports, space, owner),
        SystemInstruction::Assign { owner } => resolve_assign(accounts, owner),
        SystemInstruction::Transfer { lamports } => resolve_transfer(accounts, lamports),
        SystemInstruction::CreateAccountWithSeed {
            base,
            seed,
            lamports,
            space,
            owner,
        } => resolve_create_account_with_seed(accounts, base, seed, lamports, space, owner),
        SystemInstruction::AdvanceNonceAccount => resolve_advance_nonce_account(accounts),
        SystemInstruction::WithdrawNonceAccount(lamports) => {
            resolve_withdraw_nonce_account(accounts, lamports)
        }
        SystemInstruction::InitializeNonceAccount(pubkey) => {
            resolve_initialize_nonce_account(accounts, pubkey)
        }
        SystemInstruction::AuthorizeNonceAccount(pubkey) => {
            resolve_authorize_nonce_account(accounts, pubkey)
        }
        SystemInstruction::Allocate { space } => resolve_allocate(accounts, space),
        SystemInstruction::AllocateWithSeed {
            owner,
            base,
            seed,
            space,
        } => resolve_allocate_with_seed(accounts, owner, base, seed, space),
        SystemInstruction::AssignWithSeed { owner, seed, base } => {
            resolve_assign_with_seed(accounts, owner, seed, base)
        }
        SystemInstruction::TransferWithSeed {
            lamports,
            from_seed,
            from_owner,
        } => resolve_transfer_with_seed(accounts, lamports, from_seed, from_owner),
        SystemInstruction::UpgradeNonceAccount => resolve_upgrade_nonce_account(accounts),
    }
}

fn resolve_create_account(
    accounts: Vec<String>,
    lamports: u64,
    space: u64,
    owner: Pubkey,
) -> Result<SolanaDetail> {
    let method_name = "CreateAccount".to_string();
    let funding_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(
            "CreateAccount.funding_account".to_string(),
        ))?
        .to_string();
    let new_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(
            "CreateAccount.new_account".to_string(),
        ))?
        .to_string();
    let amount = lamports.to_string();
    let space = space.to_string();
    let owner = owner.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            program: PROGRAM_NAME.to_string(),
            method: method_name,
        },
        kind: ProgramDetail::SystemCreateAccount(ProgramDetailSystemCreateAccount {
            funding_account,
            new_account,
            amount,
            space,
            owner,
        }),
    })
}

fn resolve_assign(accounts: Vec<String>, owner: Pubkey) -> Result<SolanaDetail> {
    let method_name = "Assign".to_string();
    let account = accounts.get(0).ok_or(SolanaError::AccountNotFound(format!(
        "{}.account",
        method_name
    )))?;
    let new_owner = owner.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            program: PROGRAM_NAME.to_string(),
            method: method_name,
        },
        kind: ProgramDetail::SystemAssign(ProgramDetailSystemAssign {
            account: account.to_string(),
            new_owner,
        }),
    })
}

fn resolve_transfer(accounts: Vec<String>, lamports: u64) -> Result<SolanaDetail> {
    let method_name = "Transfer".to_string();
    let from = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.from",
            method_name
        )))?
        .to_string();
    let to = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.recipient",
            method_name
        )))?
        .to_string();
    let value = format_amount(lamports.to_string())?;
    Ok(SolanaDetail {
        common: CommonDetail {
            program: PROGRAM_NAME.to_string(),
            method: method_name,
        },
        kind: ProgramDetail::SystemTransfer(ProgramDetailSystemTransfer { value, from, to }),
    })
}

fn resolve_create_account_with_seed(
    accounts: Vec<String>,
    base: Pubkey,
    seed: String,
    lamports: u64,
    space: u64,
    owner: Pubkey,
) -> Result<SolanaDetail> {
    let method_name = "CreateAccountWithSeed".to_string();
    let funding_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.funding_account",
            method_name
        )))?
        .to_string();
    let new_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.new_account",
            method_name
        )))?
        .to_string();
    let base_account = accounts.get(2).unwrap_or(&"".to_string()).to_string();
    let amount = lamports.to_string();
    let space = space.to_string();
    let owner = owner.to_string();
    let base_pubkey = base.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            program: PROGRAM_NAME.to_string(),
            method: method_name,
        },
        kind: ProgramDetail::SystemCreateAccountWithSeed(
            ProgramDetailSystemCreateAccountWithSeed {
                funding_account,
                new_account,
                base_account,
                base_pubkey,
                seed,
                amount,
                space,
                owner,
            },
        ),
    })
}

fn resolve_advance_nonce_account(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "AdvanceNonceAccount".to_string();
    let nonce_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.nonce_account",
            method_name
        )))?
        .to_string();
    let recent_blockhashes_sysvar = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.recent_blockhashes_sysvar",
            method_name
        )))?
        .to_string();
    let nonce_authority_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.nonce_authority_pubkey",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name,
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SystemAdvanceNonceAccount(ProgramDetailSystemAdvanceNonceAccount {
            nonce_account,
            recent_blockhashes_sysvar,
            nonce_authority_pubkey,
        }),
    })
}

fn resolve_withdraw_nonce_account(accounts: Vec<String>, lamports: u64) -> Result<SolanaDetail> {
    let method_name = "WithdrawNonceAccount".to_string();
    let nonce_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.nonce_account",
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
    let recent_blockhashes_sysvar = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.recent_blockhashes_sysvar",
            method_name
        )))?
        .to_string();
    let rent_sysvar = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.rent_sysvar",
            method_name
        )))?
        .to_string();
    let nonce_authority_pubkey = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.nonce_authority_pubkey",
            method_name
        )))?
        .to_string();
    let amount = lamports.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name,
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SystemWithdrawNonceAccount(ProgramDetailSystemWithdrawNonceAccount {
            nonce_account,
            recipient,
            recent_blockhashes_sysvar,
            rent_sysvar,
            nonce_authority_pubkey,
            amount,
        }),
    })
}

fn resolve_initialize_nonce_account(accounts: Vec<String>, pubkey: Pubkey) -> Result<SolanaDetail> {
    let method_name = "InitializeNonceAccount".to_string();
    let nonce_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.nonce_account",
            method_name
        )))?
        .to_string();
    let sysvar_recent_blockhashes = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_recent_blockhashes",
            method_name
        )))?
        .to_string();
    let sysvar_rent = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_rent",
            method_name
        )))?
        .to_string();
    let nonce_authority_pubkey = pubkey.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name,
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SystemInitializeNonceAccount(
            ProgramDetailSystemInitializeNonceAccount {
                nonce_account,
                sysvar_recent_blockhashes,
                sysvar_rent,
                nonce_authority_pubkey,
            },
        ),
    })
}

fn resolve_authorize_nonce_account(accounts: Vec<String>, pubkey: Pubkey) -> Result<SolanaDetail> {
    let method_name = "AuthorizeNonceAccount".to_string();
    let nonce_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.nonce_account",
            method_name
        )))?
        .to_string();
    let old_nonce_authority_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.old_nonce_authority_pubkey",
            method_name
        )))?
        .to_string();
    let new_nonce_authority_pubkey = pubkey.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name,
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SystemAuthorizeNonceAccount(
            ProgramDetailSystemAuthorizeNonceAccount {
                nonce_account,
                old_nonce_authority_pubkey,
                new_nonce_authority_pubkey,
            },
        ),
    })
}

fn resolve_allocate(accounts: Vec<String>, space: u64) -> Result<SolanaDetail> {
    let method_name = "Allocate".to_string();
    let new_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.account",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name,
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SystemAllocate(ProgramDetailSystemAllocate {
            new_account,
            space: space.to_string(),
        }),
    })
}

fn resolve_allocate_with_seed(
    accounts: Vec<String>,
    owner: Pubkey,
    base_pubkey: Pubkey,
    seed: String,
    space: u64,
) -> Result<SolanaDetail> {
    let method_name = "AllocateWithSeed".to_string();
    let allocated_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.allocated_account",
            method_name
        )))?
        .to_string();
    let base_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.base_account",
            method_name
        )))?
        .to_string();
    let owner = owner.to_string();
    let base = base_pubkey.to_string();
    let space = space.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name,
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SystemAllocateWithSeed(ProgramDetailSystemAllocateWithSeed {
            allocated_account,
            base_account,
            base_pubkey: base,
            seed,
            space,
            owner,
        }),
    })
}

fn resolve_assign_with_seed(
    accounts: Vec<String>,
    owner: Pubkey,
    seed: String,
    base_pubkey: Pubkey,
) -> Result<SolanaDetail> {
    let method_name = "AssignWithSeed".to_string();
    let assigned_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.assigned_account",
            method_name
        )))?
        .to_string();
    let base_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.base_account",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name,
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SystemAssignWithSeed(ProgramDetailSystemAssignWithSeed {
            assigned_account,
            base_account,
            base_pubkey: base_pubkey.to_string(),
            seed,
            owner: owner.to_string(),
        }),
    })
}

fn resolve_transfer_with_seed(
    accounts: Vec<String>,
    lamports: u64,
    from_seed: String,
    from_owner: Pubkey,
) -> Result<SolanaDetail> {
    let method_name = "TransferWithSeed".to_string();
    let from = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.from",
            method_name
        )))?
        .to_string();
    let from_base_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.from_base_pubkey",
            method_name
        )))?
        .to_string();
    let recipient = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.recipient",
            method_name
        )))?
        .to_string();
    let amount = format_amount(lamports.to_string())?;
    let from_owner = from_owner.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name,
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SystemTransferWithSeed(ProgramDetailSystemTransferWithSeed {
            from,
            to: recipient,
            amount,
            from_base_pubkey,
            from_owner,
            from_seed,
        }),
    })
}

fn resolve_upgrade_nonce_account(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "UpgradeNonceAccount".to_string();
    let nonce_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(
            "UpgradeNonceAccount.nonce_account".to_string(),
        ))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name,
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SystemUpgradeNonceAccount(ProgramDetailSystemUpgradeNonceAccount {
            nonce_account,
        }),
    })
}
