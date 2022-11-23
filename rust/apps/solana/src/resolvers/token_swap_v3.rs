use crate::errors::{Result, SolanaError};
use crate::parser::detail::{
    CommonDetail, ProgramDetail, ProgramDetailSwapV3DepositAllTokenTypes,
    ProgramDetailSwapV3DepositSingleTokenTypeExactAmountIn, ProgramDetailSwapV3Initialize,
    ProgramDetailSwapV3Swap, ProgramDetailSwapV3WithdrawAllTokenTypes,
    ProgramDetailSwapV3WithdrawSingleTokenTypeExactAmountOut, SolanaDetail,
};
use crate::solana_lib::spl::token_swap::instruction::{
    DepositAllTokenTypes, DepositSingleTokenTypeExactAmountIn, Initialize, Swap, SwapInstruction,
    WithdrawAllTokenTypes, WithdrawSingleTokenTypeExactAmountOut,
};
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;

static PROGRAM_NAME: &str = "TokenSwapV3";

pub fn resolve(instruction: SwapInstruction, accounts: Vec<String>) -> Result<SolanaDetail> {
    match instruction {
        SwapInstruction::Initialize(args) => initialize(accounts, args),
        SwapInstruction::Swap(args) => swap(accounts, args),
        SwapInstruction::DepositAllTokenTypes(args) => deposit_all_token_types(accounts, args),
        SwapInstruction::WithdrawAllTokenTypes(args) => withdraw_all_token_types(accounts, args),
        SwapInstruction::DepositSingleTokenTypeExactAmountIn(args) => {
            deposit_single_token_type_exact_amount_in(accounts, args)
        }
        SwapInstruction::WithdrawSingleTokenTypeExactAmountOut(args) => {
            withdraw_single_token_type_exact_amount_out(accounts, args)
        }
    }
}

fn initialize(accounts: Vec<String>, initialize: Initialize) -> Result<SolanaDetail> {
    let method_name = "Initialize";
    let token_swap = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_swap",
            method_name
        )))?
        .to_string();
    let swap_authority_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.swap_authority_pubkey",
            method_name
        )))?
        .to_string();
    let token_a = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_a",
            method_name
        )))?
        .to_string();
    let token_b = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_b",
            method_name
        )))?
        .to_string();
    let pool_mint = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.pool_mint",
            method_name
        )))?
        .to_string();
    let pool_token_account_1 = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.pool_token_account_1",
            method_name
        )))?
        .to_string();
    let pool_token_account_2 = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.pool_token_account_2",
            method_name
        )))?
        .to_string();
    let token_program_id = accounts
        .get(7)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_program_id",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SwapV3Initialize(ProgramDetailSwapV3Initialize {
            token_swap,
            swap_authority_pubkey,
            token_a,
            token_b,
            pool_mint,
            pool_token_account_1,
            pool_token_account_2,
            token_program_id,
            initialize,
        }),
    })
}

fn swap(accounts: Vec<String>, swap: Swap) -> Result<SolanaDetail> {
    let method_name = "Swap";
    let token_swap = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_swap",
            method_name
        )))?
        .to_string();
    let swap_authority_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.swap_authority_pubkey",
            method_name
        )))?
        .to_string();
    let user_transfer_authority_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.user_transfer_authority_pubkey",
            method_name
        )))?
        .to_string();
    let source_account = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_account",
            method_name
        )))?
        .to_string();
    let source_token = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_token",
            method_name
        )))?
        .to_string();
    let destination_token = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.destination_token",
            method_name
        )))?
        .to_string();
    let destination_account = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.destination_account",
            method_name
        )))?
        .to_string();
    let pool_mint = accounts
        .get(7)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.pool_mint",
            method_name
        )))?
        .to_string();
    let fee_account = accounts
        .get(8)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.fee_account",
            method_name
        )))?
        .to_string();
    let token_program_id = accounts
        .get(9)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_program_id",
            method_name
        )))?
        .to_string();
    let host_fee_account = accounts.get(10).unwrap_or(&"".to_string()).to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SwapV3Swap(ProgramDetailSwapV3Swap {
            token_swap,
            swap_authority_pubkey,
            user_transfer_authority_pubkey,
            source_account,
            source_token,
            destination_token,
            destination_account,
            pool_mint,
            fee_account,
            token_program_id,
            host_fee_account,
            swap,
        }),
    })
}

fn deposit_all_token_types(
    accounts: Vec<String>,
    args: DepositAllTokenTypes,
) -> Result<SolanaDetail> {
    let method_name = "DepositAllTokenTypes";
    let token_swap = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_swap",
            method_name
        )))?
        .to_string();
    let swap_authority_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.swap_authority_pubkey",
            method_name
        )))?
        .to_string();
    let user_transfer_authority_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.user_transfer_authority_pubkey",
            method_name
        )))?
        .to_string();
    let token_a_user_transfer_authority_pubkey = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_a_user_transfer_authority_pubkey",
            method_name
        )))?
        .to_string();
    let token_b_user_transfer_authority_pubkey = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_b_user_transfer_authority_pubkey",
            method_name
        )))?
        .to_string();
    let token_a_base_account = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_a_base_account",
            method_name
        )))?
        .to_string();
    let token_b_base_account = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_b_base_account",
            method_name
        )))?
        .to_string();
    let pool_mint = accounts
        .get(7)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.pool_mint",
            method_name
        )))?
        .to_string();
    let pool_account = accounts
        .get(8)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.pool_account",
            method_name
        )))?
        .to_string();
    let token_program_id = accounts
        .get(9)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_program_id",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SwapV3DepositAllTokenTypes(ProgramDetailSwapV3DepositAllTokenTypes {
            token_swap,
            swap_authority_pubkey,
            user_transfer_authority_pubkey,
            token_a_user_transfer_authority_pubkey,
            token_b_user_transfer_authority_pubkey,
            token_a_base_account,
            token_b_base_account,
            pool_mint,
            pool_account,
            token_program_id,
            deposit_all_token_types: args,
        }),
    })
}

fn withdraw_all_token_types(
    accounts: Vec<String>,
    args: WithdrawAllTokenTypes,
) -> Result<SolanaDetail> {
    let method_name = "WithdrawAllTokenTypes";
    let token_swap = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_swap",
            method_name
        )))?
        .to_string();
    let swap_authority_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.swap_authority_pubkey",
            method_name
        )))?
        .to_string();
    let user_transfer_authority_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.user_transfer_authority_pubkey",
            method_name
        )))?
        .to_string();
    let pool_mint = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.pool_mint",
            method_name
        )))?
        .to_string();
    let source_pool_account = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_pool_account",
            method_name
        )))?
        .to_string();
    let token_a_swap_account = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_a_swap_account",
            method_name
        )))?
        .to_string();
    let token_b_swap_account = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_b_swap_account",
            method_name
        )))?
        .to_string();
    let token_a_user_account = accounts
        .get(7)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_a_user_account",
            method_name
        )))?
        .to_string();
    let token_b_user_account = accounts
        .get(8)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_b_user_account",
            method_name
        )))?
        .to_string();
    let fee_account = accounts
        .get(9)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.fee_account",
            method_name
        )))?
        .to_string();
    let token_program_id = accounts
        .get(10)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_program_id",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SwapV3WithdrawAllTokenTypes(
            ProgramDetailSwapV3WithdrawAllTokenTypes {
                token_swap,
                swap_authority_pubkey,
                user_transfer_authority_pubkey,
                pool_mint,
                source_pool_account,
                token_a_swap_account,
                token_b_swap_account,
                token_a_user_account,
                token_b_user_account,
                fee_account,
                token_program_id,
                withdraw_all_token_types: args,
            },
        ),
    })
}

fn deposit_single_token_type_exact_amount_in(
    accounts: Vec<String>,
    args: DepositSingleTokenTypeExactAmountIn,
) -> Result<SolanaDetail> {
    let method_name = "DepositSingleTokenTypeExactAmountIn";
    let token_swap = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_swap",
            method_name
        )))?
        .to_string();
    let swap_authority_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.swap_authority_pubkey",
            method_name
        )))?
        .to_string();
    let user_transfer_authority_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.user_transfer_authority_pubkey",
            method_name
        )))?
        .to_string();
    let token_source_account = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_source_account",
            method_name
        )))?
        .to_string();
    let token_a_swap_account = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_a_swap_account",
            method_name
        )))?
        .to_string();
    let token_b_swap_account = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_b_swap_account",
            method_name
        )))?
        .to_string();
    let pool_mint = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.pool_mint",
            method_name
        )))?
        .to_string();
    let pool_account = accounts
        .get(7)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.pool_account",
            method_name
        )))?
        .to_string();
    let token_program_id = accounts
        .get(8)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_program_id",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SwapV3DepositSingleTokenTypeExactAmountIn(
            ProgramDetailSwapV3DepositSingleTokenTypeExactAmountIn {
                token_swap,
                swap_authority_pubkey,
                user_transfer_authority_pubkey,
                token_source_account,
                token_a_swap_account,
                token_b_swap_account,
                pool_mint,
                pool_account,
                token_program_id,
                arguments: args,
            },
        ),
    })
}

fn withdraw_single_token_type_exact_amount_out(
    accounts: Vec<String>,
    args: WithdrawSingleTokenTypeExactAmountOut,
) -> Result<SolanaDetail> {
    let method_name = "WithdrawSingleTokenTypeExactAmountOut";
    let token_swap = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_swap",
            method_name
        )))?
        .to_string();
    let swap_authority_pubkey = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.swap_authority_pubkey",
            method_name
        )))?
        .to_string();
    let user_transfer_authority_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.user_transfer_authority_pubkey",
            method_name
        )))?
        .to_string();
    let pool_mint = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_source_account",
            method_name
        )))?
        .to_string();
    let source_pool_account = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_a_swap_account",
            method_name
        )))?
        .to_string();
    let token_a_swap_account = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_b_swap_account",
            method_name
        )))?
        .to_string();
    let token_b_swap_account = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.pool_mint",
            method_name
        )))?
        .to_string();
    let token_user_account = accounts
        .get(7)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.pool_account",
            method_name
        )))?
        .to_string();
    let fee_account = accounts
        .get(8)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_program_id",
            method_name
        )))?
        .to_string();
    let token_program_id = accounts
        .get(9)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_program_id",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::SwapV3WithdrawSingleTokenTypeExactAmountOut(
            ProgramDetailSwapV3WithdrawSingleTokenTypeExactAmountOut {
                token_swap,
                swap_authority_pubkey,
                user_transfer_authority_pubkey,
                pool_mint,
                source_pool_account,
                token_a_swap_account,
                token_b_swap_account,
                token_user_account,
                fee_account,
                token_program_id,
                arguments: args,
            },
        ),
    })
}
