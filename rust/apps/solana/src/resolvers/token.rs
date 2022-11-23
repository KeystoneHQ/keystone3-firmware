use crate::errors::{Result, SolanaError};
use crate::parser::detail::{
    CommonDetail, ProgramDetail, ProgramDetailTokenApprove, ProgramDetailTokenApproveChecked,
    ProgramDetailTokenBurn, ProgramDetailTokenBurnChecked, ProgramDetailTokenCloseAccount,
    ProgramDetailTokenFreezeAccount, ProgramDetailTokenInitializeAccount,
    ProgramDetailTokenInitializeAccount2, ProgramDetailTokenInitializeAccount3,
    ProgramDetailTokenInitializeMint, ProgramDetailTokenInitializeMint2,
    ProgramDetailTokenInitializeMultisig, ProgramDetailTokenInitializeMultisig2,
    ProgramDetailTokenMintTo, ProgramDetailTokenMintToChecked, ProgramDetailTokenRevoke,
    ProgramDetailTokenSetAuthority, ProgramDetailTokenSyncNative, ProgramDetailTokenThawAccount,
    ProgramDetailTokenTransfer, ProgramDetailTokenTransferChecked, SolanaDetail,
};
use crate::solana_lib::solana_program::program_option::COption;
use crate::solana_lib::solana_program::pubkey::Pubkey;
use crate::solana_lib::spl::token::instruction::{AuthorityType, TokenInstruction};
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;

fn map_coption_to_option<T>(value: COption<T>) -> Option<T> {
    match value {
        COption::Some(t) => Some(t),
        COption::None => None,
    }
}

fn is_multisig(accounts: &Vec<String>, point: u8) -> bool {
    (accounts.len() as u8) > point
}

static PROGRAM_NAME: &str = "Token";

pub fn resolve(instruction: TokenInstruction, accounts: Vec<String>) -> Result<SolanaDetail> {
    match instruction {
        TokenInstruction::InitializeMint {
            mint_authority,
            decimals,
            freeze_authority,
        } => initialize_mint(accounts, mint_authority, decimals, freeze_authority),
        TokenInstruction::InitializeAccount => initialize_account(accounts),
        TokenInstruction::InitializeMultisig { m } => initialize_multisig(accounts, m),
        TokenInstruction::Transfer { amount } => transfer(accounts, amount),
        TokenInstruction::Approve { amount } => approve(accounts, amount),
        TokenInstruction::Revoke => revoke(accounts),
        TokenInstruction::SetAuthority {
            authority_type,
            new_authority,
        } => set_authority(accounts, authority_type, new_authority),
        TokenInstruction::MintTo { amount } => mint_to(accounts, amount),
        TokenInstruction::Burn { amount } => burn(accounts, amount),
        TokenInstruction::CloseAccount => close_account(accounts),
        TokenInstruction::FreezeAccount => freeze_account(accounts),
        TokenInstruction::ThawAccount => thaw_account(accounts),
        TokenInstruction::TransferChecked { decimals, amount } => {
            transfer_checked(accounts, decimals, amount)
        }
        TokenInstruction::ApproveChecked { decimals, amount } => {
            approve_checked(accounts, decimals, amount)
        }
        TokenInstruction::MintToChecked { decimals, amount } => {
            mint_to_checked(accounts, decimals, amount)
        }
        TokenInstruction::BurnChecked { decimals, amount } => {
            burn_checked(accounts, decimals, amount)
        }
        TokenInstruction::InitializeAccount2 { owner } => initialize_account_2(accounts, owner),
        TokenInstruction::SyncNative => sync_native(accounts),
        TokenInstruction::InitializeAccount3 { owner } => initialize_account_3(accounts, owner),
        TokenInstruction::InitializeMultisig2 { m } => initialize_multisig_2(accounts, m),
        TokenInstruction::InitializeMint2 {
            mint_authority,
            decimals,
            freeze_authority,
        } => initialize_mint_2(accounts, mint_authority, decimals, freeze_authority),
    }
}

fn initialize_mint(
    accounts: Vec<String>,
    mint_authority_pubkey: Pubkey,
    decimals: u8,
    freeze_authority_pubkey: COption<Pubkey>,
) -> Result<SolanaDetail> {
    let method_name = "InitializeMint";
    let mint = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.mint",
            method_name
        )))?
        .to_string();
    let sysver_rent = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysver_rent",
            method_name
        )))?
        .to_string();
    let mint_authority_pubkey = mint_authority_pubkey.to_string().to_string();
    let freeze_authority_pubkey =
        map_coption_to_option(freeze_authority_pubkey.map(|v| v.to_string()))
            .unwrap_or("".to_string());
    Ok(SolanaDetail {
        common: CommonDetail {
            program: PROGRAM_NAME.to_string(),
            method: method_name.to_string(),
        },
        kind: ProgramDetail::TokenInitializeMint(ProgramDetailTokenInitializeMint {
            mint,
            sysver_rent,
            mint_authority_pubkey,
            freeze_authority_pubkey,
            decimals,
        }),
    })
}

fn initialize_account(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "InitializeAccount";

    let account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.account",
            method_name
        )))?
        .to_string();
    let mint = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.mint",
            method_name
        )))?
        .to_string();
    let owner = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.owner",
            method_name
        )))?
        .to_string();
    let sysver_rent = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysver_rent",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::TokenInitializeAccount(ProgramDetailTokenInitializeAccount {
            account,
            mint,
            owner,
            sysver_rent,
        }),
    })
}

fn initialize_multisig(accounts: Vec<String>, m: u8) -> Result<SolanaDetail> {
    let method_name = "InitializeMultisig";

    let multisig_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.multisig_account",
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
    let attendees = accounts[2..].to_vec();
    let required_signatures = m;
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::TokenInitializeMultisig(ProgramDetailTokenInitializeMultisig {
            multisig_account,
            sysvar_rent,
            attendees,
            required_signatures,
        }),
    })
}

fn transfer(accounts: Vec<String>, amount: u64) -> Result<SolanaDetail> {
    let method_name = "Transfer";

    let source_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_account",
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
    let amount = amount.to_string();
    if is_multisig(&accounts, 3) {
        let owner = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.owner",
                method_name
            )))?
            .to_string();
        let signers = Some(accounts[3..].to_vec());
        return Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenTransfer(ProgramDetailTokenTransfer {
                source_account,
                recipient,
                owner,
                signers,
                amount,
            }),
        });
    }
    let owner = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!("Transfer.owner")))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::TokenTransfer(ProgramDetailTokenTransfer {
            source_account,
            recipient,
            owner,
            amount,
            signers: None,
        }),
    })
}

fn approve(accounts: Vec<String>, amount: u64) -> Result<SolanaDetail> {
    let method_name = "Approve";

    let source_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_account",
            method_name
        )))?
        .to_string();
    let delegate_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.delegate_account",
            method_name
        )))?
        .to_string();
    let amount = amount.to_string();
    if is_multisig(&accounts, 3) {
        let owner = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.owner",
                method_name
            )))?
            .to_string();
        let signers = Some(accounts[3..].to_vec());
        return Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenApprove(ProgramDetailTokenApprove {
                source_account,
                delegate_account,
                owner,
                signers,
                amount,
            }),
        });
    }
    let owner = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.owner",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::TokenApprove(ProgramDetailTokenApprove {
            source_account,
            delegate_account,
            owner,
            signers: None,
            amount,
        }),
    })
}

fn revoke(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "Revoke";

    let source_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_account",
            method_name
        )))?
        .to_string();
    if is_multisig(&accounts, 2) {
        let owner = accounts
            .get(1)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.owner",
                method_name
            )))?
            .to_string();
        let signers = Some(accounts[2..].to_vec());
        return Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenRevoke(ProgramDetailTokenRevoke {
                source_account,
                owner,
                signers,
            }),
        });
    }
    let owner = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.owner",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::TokenRevoke(ProgramDetailTokenRevoke {
            source_account,
            owner,
            signers: None,
        }),
    })
}

fn set_authority(
    accounts: Vec<String>,
    authority_type: AuthorityType,
    new_authority: COption<Pubkey>,
) -> Result<SolanaDetail> {
    let method_name = "SetAuthority";

    let account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.account",
            method_name
        )))?
        .to_string();
    let authority_type = match authority_type {
        AuthorityType::AccountOwner => "account owner",
        AuthorityType::CloseAccount => "close account",
        AuthorityType::MintTokens => "mint tokens",
        AuthorityType::FreezeAccount => "freeze account",
    }
    .to_string();
    let new_authority_pubkey =
        map_coption_to_option(new_authority.map(|v| v.to_string())).unwrap_or("".to_string());
    if is_multisig(&accounts, 2) {
        let old_authority_pubkey = accounts
            .get(1)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.old_authority_pubkey",
                method_name
            )))?
            .to_string();
        let signers = Some(accounts[2..].to_vec());
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenSetAuthority(ProgramDetailTokenSetAuthority {
                account,
                old_authority_pubkey,
                signers,
                authority_type,
                new_authority_pubkey,
            }),
        })
    } else {
        let old_authority_pubkey = accounts
            .get(1)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.old_authority_pubkey",
                method_name
            )))?
            .to_string();
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenSetAuthority(ProgramDetailTokenSetAuthority {
                account,
                old_authority_pubkey,
                signers: None,
                authority_type,
                new_authority_pubkey,
            }),
        })
    }
}

fn mint_to(accounts: Vec<String>, amount: u64) -> Result<SolanaDetail> {
    let method_name = "MintTo";

    let mint = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.mint",
            method_name
        )))?
        .to_string();
    let mint_to_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.mint_to_account",
            method_name
        )))?
        .to_string();
    let amount = amount.to_string();
    if is_multisig(&accounts, 3) {
        let mint_authority_pubkey = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.mint_authority_pubkey",
                method_name
            )))?
            .to_string();
        let signers = Some(accounts[3..].to_vec());
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenMintTo(ProgramDetailTokenMintTo {
                mint,
                mint_to_account,
                mint_authority_pubkey,
                signers,
                amount,
            }),
        })
    } else {
        let mint_authority_pubkey = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.mint_authority_pubkey",
                method_name
            )))?
            .to_string();
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenMintTo(ProgramDetailTokenMintTo {
                mint,
                mint_to_account,
                mint_authority_pubkey,
                signers: None,
                amount,
            }),
        })
    }
}

fn burn(accounts: Vec<String>, amount: u64) -> Result<SolanaDetail> {
    let method_name = "Burn";

    let account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.account",
            method_name
        )))?
        .to_string();
    let mint = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.mint",
            method_name
        )))?
        .to_string();
    let amount = amount.to_string();
    if is_multisig(&accounts, 3) {
        let owner = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.owner",
                method_name
            )))?
            .to_string();
        let signers = Some(accounts[3..].to_vec());
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenBurn(ProgramDetailTokenBurn {
                account,
                mint,
                owner,
                signers,
                amount,
            }),
        })
    } else {
        let owner = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.owner",
                method_name
            )))?
            .to_string();
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenBurn(ProgramDetailTokenBurn {
                account,
                mint,
                owner,
                signers: None,
                amount,
            }),
        })
    }
}

fn close_account(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "CloseAccount";
    let account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.account",
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
    if is_multisig(&accounts, 3) {
        let owner = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.owner",
                method_name
            )))?
            .to_string();
        let signers = Some(accounts[3..].to_vec());
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenCloseAccount(ProgramDetailTokenCloseAccount {
                account,
                recipient,
                owner,
                signers,
            }),
        })
    } else {
        let owner = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!("CloseAccount.owner")))?
            .to_string();
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenCloseAccount(ProgramDetailTokenCloseAccount {
                account,
                recipient,
                owner,
                signers: None,
            }),
        })
    }
}

fn freeze_account(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "FreezeAccount";
    let account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.account",
            method_name
        )))?
        .to_string();
    let mint = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.mint",
            method_name
        )))?
        .to_string();
    if is_multisig(&accounts, 3) {
        let mint_freeze_authority_pubkey = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.mint_freeze_authority_pubkey",
                method_name
            )))?
            .to_string();
        let signers = Some(accounts[3..].to_vec());
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenFreezeAccount(ProgramDetailTokenFreezeAccount {
                account,
                mint,
                mint_freeze_authority_pubkey,
                signers,
            }),
        })
    } else {
        let mint_freeze_authority_pubkey = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.mint_freeze_authority_pubkey",
                method_name
            )))?
            .to_string();
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenFreezeAccount(ProgramDetailTokenFreezeAccount {
                account,
                mint,
                mint_freeze_authority_pubkey,
                signers: None,
            }),
        })
    }
}

fn thaw_account(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "ThawAccount";
    let account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.account",
            method_name
        )))?
        .to_string();
    let mint = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.mint",
            method_name
        )))?
        .to_string();
    if is_multisig(&accounts, 3) {
        let mint_freeze_authority_pubkey = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.mint_freeze_authority_pubkey",
                method_name
            )))?
            .to_string();
        let signers = Some(accounts[3..].to_vec());
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenThawAccount(ProgramDetailTokenThawAccount {
                account,
                mint,
                mint_freeze_authority_pubkey,
                signers,
            }),
        })
    } else {
        let mint_freeze_authority_pubkey = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.mint_freeze_authority_pubkey",
                method_name
            )))?
            .to_string();
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenThawAccount(ProgramDetailTokenThawAccount {
                account,
                mint,
                mint_freeze_authority_pubkey,
                signers: None,
            }),
        })
    }
}

fn transfer_checked(accounts: Vec<String>, decimals: u8, amount: u64) -> Result<SolanaDetail> {
    let method_name = "TransferChecked";
    if is_multisig(&accounts, 4) {
        let account = accounts
            .get(0)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.account",
                method_name
            )))?
            .to_string();
        let mint = accounts
            .get(1)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.mint",
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
        let owner = accounts
            .get(3)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.owner",
                method_name
            )))?
            .to_string();
        let signers = Some(accounts[4..].to_vec());
        let amount = amount.to_string();
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenTransferChecked(ProgramDetailTokenTransferChecked {
                account,
                mint,
                recipient,
                owner,
                signers,
                decimals,
                amount,
            }),
        })
    } else {
        let account = accounts
            .get(0)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.account",
                method_name
            )))?
            .to_string();
        let mint = accounts
            .get(1)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.mint",
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
        let owner = accounts
            .get(3)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.owner",
                method_name
            )))?
            .to_string();
        let amount = amount.to_string();
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenTransferChecked(ProgramDetailTokenTransferChecked {
                account,
                mint,
                recipient,
                owner,
                signers: None,
                decimals,
                amount,
            }),
        })
    }
}

fn approve_checked(accounts: Vec<String>, decimals: u8, amount: u64) -> Result<SolanaDetail> {
    let method_name = "ApproveChecked";
    if is_multisig(&accounts, 4) {
        let account = accounts
            .get(0)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.account",
                method_name
            )))?
            .to_string();
        let mint = accounts
            .get(1)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.mint",
                method_name
            )))?
            .to_string();
        let delegate = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.delegate",
                method_name
            )))?
            .to_string();
        let owner = accounts
            .get(3)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.owner",
                method_name
            )))?
            .to_string();
        let signers = Some(accounts[4..].to_vec());
        let amount = amount.to_string();
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenApproveChecked(ProgramDetailTokenApproveChecked {
                account,
                mint,
                delegate,
                owner,
                signers,
                decimals,
                amount,
            }),
        })
    } else {
        let account = accounts
            .get(0)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.account",
                method_name
            )))?
            .to_string();
        let mint = accounts
            .get(1)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.mint",
                method_name
            )))?
            .to_string();
        let delegate = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.delegate",
                method_name
            )))?
            .to_string();
        let owner = accounts
            .get(3)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.owner",
                method_name
            )))?
            .to_string();
        let amount = amount.to_string();
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenApproveChecked(ProgramDetailTokenApproveChecked {
                account,
                mint,
                delegate,
                owner,
                signers: None,
                decimals,
                amount,
            }),
        })
    }
}

fn mint_to_checked(accounts: Vec<String>, decimals: u8, amount: u64) -> Result<SolanaDetail> {
    let method_name = "MintToChecked";
    let mint = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.mint",
            method_name
        )))?
        .to_string();
    let mint_to_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.mint_to_account",
            method_name
        )))?
        .to_string();
    let amount = amount.to_string();
    if is_multisig(&accounts, 3) {
        let mint_authority_pubkey = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.mint_authority_pubkey",
                method_name
            )))?
            .to_string();
        let signers = Some(accounts[3..].to_vec());
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenMintToChecked(ProgramDetailTokenMintToChecked {
                mint,
                mint_to_account,
                mint_authority_pubkey,
                signers,
                decimals,
                amount,
            }),
        })
    } else {
        let mint_authority_pubkey = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.mint_authority_pubkey",
                method_name
            )))?
            .to_string();
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenMintToChecked(ProgramDetailTokenMintToChecked {
                mint,
                mint_to_account,
                mint_authority_pubkey,
                signers: None,
                decimals,
                amount,
            }),
        })
    }
}

fn burn_checked(accounts: Vec<String>, decimals: u8, amount: u64) -> Result<SolanaDetail> {
    let method_name = "BurnChecked";
    let account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.account",
            method_name
        )))?
        .to_string();
    let mint = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.mint",
            method_name
        )))?
        .to_string();
    let amount = amount.to_string();
    if is_multisig(&accounts, 3) {
        let owner = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.owner",
                method_name
            )))?
            .to_string();
        let signers = Some(accounts[3..].to_vec());
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenBurnChecked(ProgramDetailTokenBurnChecked {
                account,
                mint,
                owner,
                signers,
                decimals,
                amount,
            }),
        })
    } else {
        let owner = accounts
            .get(2)
            .ok_or(SolanaError::AccountNotFound(format!(
                "{}.owner",
                method_name
            )))?
            .to_string();
        Ok(SolanaDetail {
            common: CommonDetail {
                method: method_name.to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::TokenBurnChecked(ProgramDetailTokenBurnChecked {
                account,
                mint,
                owner,
                signers: None,
                decimals,
                amount,
            }),
        })
    }
}

fn initialize_account_2(accounts: Vec<String>, owner: Pubkey) -> Result<SolanaDetail> {
    let method_name = "InitializeAccount2";
    let account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.account",
            method_name
        )))?
        .to_string();
    let mint = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.mint",
            method_name
        )))?
        .to_string();
    let sysver_rent = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysver_rent",
            method_name
        )))?
        .to_string();
    let owner = owner.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::TokenInitializeAccount2(ProgramDetailTokenInitializeAccount2 {
            account,
            mint,
            sysver_rent,
            owner,
        }),
    })
}

fn sync_native(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "SyncNative";
    let account_to_sync = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.account_to_sync",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::TokenSyncNative(ProgramDetailTokenSyncNative { account_to_sync }),
    })
}

fn initialize_account_3(accounts: Vec<String>, owner: Pubkey) -> Result<SolanaDetail> {
    let method_name = "InitializeAccount3";
    let account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.account",
            method_name
        )))?
        .to_string();
    let mint = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.mint",
            method_name
        )))?
        .to_string();
    let owner = owner.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::TokenInitializeAccount3(ProgramDetailTokenInitializeAccount3 {
            account,
            mint,
            owner,
        }),
    })
}

fn initialize_multisig_2(accounts: Vec<String>, m: u8) -> Result<SolanaDetail> {
    let method_name = "InitializeMultisig2";
    let multisig_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.multisig_account",
            method_name
        )))?
        .to_string();
    let attendees = accounts[1..].to_vec();
    let required_signatures = m;
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::TokenInitializeMultisig2(ProgramDetailTokenInitializeMultisig2 {
            multisig_account,
            attendees,
            required_signatures,
        }),
    })
}

fn initialize_mint_2(
    accounts: Vec<String>,
    mint_authority_pubkey: Pubkey,
    decimals: u8,
    freeze_authority_pubkey: COption<Pubkey>,
) -> Result<SolanaDetail> {
    let method_name = "InitializeMint2";
    let mint = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.mint",
            method_name
        )))?
        .to_string();
    let mint_authority_pubkey = mint_authority_pubkey.to_string();
    let freeze_authority_pubkey =
        map_coption_to_option(freeze_authority_pubkey.map(|v| v.to_string()))
            .unwrap_or("".to_string())
            .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::TokenInitializeMint2(ProgramDetailTokenInitializeMint2 {
            mint,
            mint_authority_pubkey,
            freeze_authority_pubkey,
            decimals,
        }),
    })
}
