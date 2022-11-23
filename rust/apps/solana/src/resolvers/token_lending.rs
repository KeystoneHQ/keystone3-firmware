use crate::errors::{Result, SolanaError};
use crate::parser::detail::{
    CommonDetail, ProgramDetail, ProgramDetailLendingBorrowObligationLiquidity,
    ProgramDetailLendingDepositObligationCollateral, ProgramDetailLendingDepositReserveLiquidity,
    ProgramDetailLendingFlashLoan, ProgramDetailLendingInitLendingMarket,
    ProgramDetailLendingInitObligation, ProgramDetailLendingInitReserve,
    ProgramDetailLendingLiquidateObligation, ProgramDetailLendingRedeemReserveCollateral,
    ProgramDetailLendingRefreshObligation, ProgramDetailLendingRefreshReserve,
    ProgramDetailLendingRepayObligationLiquidity, ProgramDetailLendingSetLendingMarketOwner,
    ProgramDetailLendingWithdrawObligationCollateral, SolanaDetail,
};
use crate::solana_lib::solana_program::pubkey::Pubkey;
use crate::solana_lib::spl::token_lending::instruction::LendingInstruction;
use crate::solana_lib::spl::token_lending::state::ReserveConfig;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::str;

static PROGRAM_NAME: &str = "TokenLending";

pub fn resolve(instruction: LendingInstruction, accounts: Vec<String>) -> Result<SolanaDetail> {
    match instruction {
        LendingInstruction::InitLendingMarket {
            owner,
            quote_currency,
        } => init_lending_market(accounts, owner, quote_currency),
        LendingInstruction::SetLendingMarketOwner { new_owner } => {
            set_lending_market_owner(accounts, new_owner)
        }
        LendingInstruction::InitReserve {
            liquidity_amount,
            config,
        } => init_reserve(accounts, liquidity_amount, config),
        LendingInstruction::RefreshReserve => refresh_reserve(accounts),
        LendingInstruction::DepositReserveLiquidity { liquidity_amount } => {
            deposit_reserve_liquidity(accounts, liquidity_amount)
        }
        LendingInstruction::RedeemReserveCollateral { collateral_amount } => {
            redeem_reserve_collateral(accounts, collateral_amount)
        }
        LendingInstruction::InitObligation => init_obligation(accounts),
        LendingInstruction::RefreshObligation => refresh_obligation(accounts),
        LendingInstruction::DepositObligationCollateral { collateral_amount } => {
            deposit_obligation_collateral(accounts, collateral_amount)
        }
        LendingInstruction::WithdrawObligationCollateral { collateral_amount } => {
            withdraw_obligation_collateral(accounts, collateral_amount)
        }
        LendingInstruction::BorrowObligationLiquidity { liquidity_amount } => {
            borrow_obligation_liquidity(accounts, liquidity_amount)
        }
        LendingInstruction::RepayObligationLiquidity { liquidity_amount } => {
            repay_obligation_liquidity(accounts, liquidity_amount)
        }
        LendingInstruction::LiquidateObligation { liquidity_amount } => {
            liquidate_obligation(accounts, liquidity_amount)
        }
        LendingInstruction::FlashLoan { amount } => flash_loan(accounts, amount),
    }
}

fn init_lending_market(
    accounts: Vec<String>,
    owner: Pubkey,
    quote_currency: [u8; 32],
) -> Result<SolanaDetail> {
    let method_name = "InitLendingMarket";
    let lending_market_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_account",
            method_name,
        )))?
        .to_string();
    let sysvar_rent = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.account",
            method_name
        )))?
        .to_string();
    let token_program_id = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_program_id",
            method_name
        )))?
        .to_string();
    let oracle_program_id = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.oracle_program_id",
            method_name
        )))?
        .to_string();
    let owner = owner.to_string();
    let quote_currency = str::from_utf8(&quote_currency)
        .map_err(|_| SolanaError::InvalidData(format!("{}.quote_currency", method_name)))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::LendingInitLendingMarket(ProgramDetailLendingInitLendingMarket {
            lending_market_account,
            sysvar_rent,
            token_program_id,
            oracle_program_id,
            owner,
            quote_currency,
        }),
    })
}

fn set_lending_market_owner(accounts: Vec<String>, new_owner: Pubkey) -> Result<SolanaDetail> {
    let method_name = "SetLendingMarketOwner";
    let lending_market_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_account",
            method_name,
        )))?
        .to_string();
    let current_owner = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.current_owner",
            method_name
        )))?
        .to_string();
    let new_owner = new_owner.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::LendingSetLendingMarketOwner(
            ProgramDetailLendingSetLendingMarketOwner {
                lending_market_account,
                current_owner,
                new_owner,
            },
        ),
    })
}

fn init_reserve(
    accounts: Vec<String>,
    liquidity_amount: u64,
    config: ReserveConfig,
) -> Result<SolanaDetail> {
    let method_name = "SetLendingMarketOwner";
    let source_liquidity_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_liquidity_account",
            method_name,
        )))?
        .to_string();
    let destination_collateral_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.destination_collateral_account",
            method_name
        )))?
        .to_string();
    let reserve_account = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_account",
            method_name
        )))?
        .to_string();
    let reserve_liquidity_mint = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_liquidity_mint",
            method_name
        )))?
        .to_string();
    let reserve_liquidity_supply_account = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_liquidity_supply_account",
            method_name
        )))?
        .to_string();
    let reserve_liquidity_fee_receiver = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_liquidity_fee_receiver",
            method_name
        )))?
        .to_string();
    let reserve_collateral_mint = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_collateral_mint",
            method_name
        )))?
        .to_string();
    let reserve_collateral_supply_pubkey = accounts
        .get(7)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_collateral_supply_pubkey",
            method_name
        )))?
        .to_string();
    let pyth_product_account = accounts
        .get(8)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.pyth_product_account",
            method_name
        )))?
        .to_string();
    let pyth_price_account = accounts
        .get(9)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.pyth_price_account",
            method_name
        )))?
        .to_string();
    let lending_market_account = accounts
        .get(10)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_account",
            method_name
        )))?
        .to_string();
    let lending_market_authority_pubkey = accounts
        .get(11)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_authority_pubkey",
            method_name
        )))?
        .to_string();
    let lending_market_owner = accounts
        .get(12)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_owner",
            method_name
        )))?
        .to_string();
    let user_transfer_authority_pubkey = accounts
        .get(13)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.user_transfer_authority_pubkey",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(14)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
            method_name
        )))?
        .to_string();
    let sysvar_rent = accounts
        .get(15)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_rent",
            method_name
        )))?
        .to_string();
    let token_program_id = accounts
        .get(16)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.token_program_id",
            method_name
        )))?
        .to_string();
    let liquidity_amount = liquidity_amount.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::LendingInitReserve(ProgramDetailLendingInitReserve {
            source_liquidity_account,
            destination_collateral_account,
            reserve_account,
            reserve_liquidity_mint,
            reserve_liquidity_supply_account,
            reserve_liquidity_fee_receiver,
            reserve_collateral_mint,
            reserve_collateral_supply_pubkey,
            pyth_product_account,
            pyth_price_account,
            lending_market_account,
            lending_market_authority_pubkey,
            lending_market_owner,
            user_transfer_authority_pubkey,
            sysvar_clock,
            sysvar_rent,
            token_program_id,
            liquidity_amount,
            reserve_config: config,
        }),
    })
}

fn refresh_reserve(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "RefreshReserve";
    let reserve_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_account",
            method_name,
        )))?
        .to_string();
    let reserve_liquidity_oracle_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_liquidity_oracle_account",
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
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::LendingRefreshReserve(ProgramDetailLendingRefreshReserve {
            reserve_account,
            reserve_liquidity_oracle_account,
            sysvar_clock,
        }),
    })
}

fn deposit_reserve_liquidity(accounts: Vec<String>, liquidity_amount: u64) -> Result<SolanaDetail> {
    let method_name = "DepositReserveLiquidity";
    let source_liquidity_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_liquidity_account",
            method_name,
        )))?
        .to_string();
    let destination_collateral_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.destination_collateral_account",
            method_name
        )))?
        .to_string();
    let reserve_account = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_account",
            method_name
        )))?
        .to_string();
    let reserve_liquidity_supply_account = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_liquidity_supply_account",
            method_name
        )))?
        .to_string();
    let reserve_collateral_mint = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_collateral_mint",
            method_name
        )))?
        .to_string();
    let lending_market_account = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_account",
            method_name
        )))?
        .to_string();
    let lending_market_authority_pubkey = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_authority_pubkey",
            method_name
        )))?
        .to_string();
    let user_transfer_authority_pubkey = accounts
        .get(7)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.user_transfer_authority_pubkey",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(8)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
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
    let liquidity_amount = liquidity_amount.to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::LendingDepositReserveLiquidity(
            ProgramDetailLendingDepositReserveLiquidity {
                source_liquidity_account,
                destination_collateral_account,
                reserve_account,
                reserve_liquidity_supply_account,
                reserve_collateral_mint,
                lending_market_account,
                lending_market_authority_pubkey,
                user_transfer_authority_pubkey,
                sysvar_clock,
                token_program_id,
                liquidity_amount,
            },
        ),
    })
}

fn redeem_reserve_collateral(
    accounts: Vec<String>,
    collateral_amount: u64,
) -> Result<SolanaDetail> {
    let method_name = "RedeemReserveCollateral";
    let source_collateral_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_collateral_account",
            method_name,
        )))?
        .to_string();
    let destination_liquidity_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.destination_liquidity_account",
            method_name
        )))?
        .to_string();
    let reserve_account = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_account",
            method_name
        )))?
        .to_string();
    let reserve_collateral_mint = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_collateral_mint",
            method_name
        )))?
        .to_string();
    let reserve_liquidity_supply_account = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_liquidity_supply_account",
            method_name
        )))?
        .to_string();
    let lending_market_account = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_account",
            method_name
        )))?
        .to_string();
    let lending_market_authority_pubkey = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_authority_pubkey",
            method_name
        )))?
        .to_string();
    let user_transfer_authority_pubkey = accounts
        .get(7)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.user_transfer_authority_pubkey",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(8)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
            method_name
        )))?
        .to_string();
    let token_program_id = accounts
        .get(9)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
            method_name
        )))?
        .to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::LendingRedeemReserveCollateral(
            ProgramDetailLendingRedeemReserveCollateral {
                source_collateral_account,
                destination_liquidity_account,
                reserve_account,
                reserve_liquidity_supply_account,
                reserve_collateral_mint,
                lending_market_account,
                lending_market_authority_pubkey,
                user_transfer_authority_pubkey,
                sysvar_clock,
                token_program_id,
                collateral_amount: collateral_amount.to_string(),
            },
        ),
    })
}

fn init_obligation(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "InitObligation";
    let obligation_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.obligation_account",
            method_name,
        )))?
        .to_string();
    let lending_market_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_account",
            method_name
        )))?
        .to_string();
    let obligation_owner = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.obligation_owner",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
            method_name
        )))?
        .to_string();
    let sysvar_rent = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_rent",
            method_name
        )))?
        .to_string();
    let token_program_id = accounts
        .get(5)
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
        kind: ProgramDetail::LendingInitObligation(ProgramDetailLendingInitObligation {
            obligation_account,
            lending_market_account,
            obligation_owner,
            sysvar_clock,
            sysvar_rent,
            token_program_id,
        }),
    })
}

fn refresh_obligation(accounts: Vec<String>) -> Result<SolanaDetail> {
    let method_name = "RefreshObligation";
    let obligation_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.obligation_account",
            method_name,
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
            method_name
        )))?
        .to_string();
    let keys = accounts[2..].to_vec();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::LendingRefreshObligation(ProgramDetailLendingRefreshObligation {
            obligation_account,
            sysvar_clock,
            keys,
        }),
    })
}

fn deposit_obligation_collateral(
    accounts: Vec<String>,
    collateral_amount: u64,
) -> Result<SolanaDetail> {
    let method_name = "DepositObligationCollateral";
    let source_collateral_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_collateral_account",
            method_name,
        )))?
        .to_string();
    let destination_collateral_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.destination_collateral_account",
            method_name
        )))?
        .to_string();
    let deposit_reserve_pubkey = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.deposit_reserve_pubkey",
            method_name
        )))?
        .to_string();
    let obligation_account = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.obligation_account",
            method_name
        )))?
        .to_string();
    let lending_market_account = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_account",
            method_name
        )))?
        .to_string();
    let obligation_owner = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.obligation_owner",
            method_name
        )))?
        .to_string();
    let user_transfer_authority_pubkey = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.user_transfer_authority_pubkey",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(7)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
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
        kind: ProgramDetail::LendingDepositObligationCollateral(
            ProgramDetailLendingDepositObligationCollateral {
                source_collateral_account,
                destination_collateral_account,
                deposit_reserve_pubkey,
                obligation_account,
                lending_market_account,
                obligation_owner,
                user_transfer_authority_pubkey,
                sysvar_clock,
                token_program_id,
                collateral_amount: collateral_amount.to_string(),
            },
        ),
    })
}

fn withdraw_obligation_collateral(
    accounts: Vec<String>,
    collateral_amount: u64,
) -> Result<SolanaDetail> {
    let method_name = "WithdrawObligationCollateral";
    let source_collateral_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_collateral_account",
            method_name,
        )))?
        .to_string();
    let destination_collateral_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.destination_collateral_account",
            method_name
        )))?
        .to_string();
    let withdraw_reserve_account = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.deposit_reserve_pubkey",
            method_name
        )))?
        .to_string();
    let obligation_account = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.obligation_account",
            method_name
        )))?
        .to_string();
    let lending_market_account = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_account",
            method_name
        )))?
        .to_string();
    let lending_market_authority_pubkey = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_authority_pubkey",
            method_name
        )))?
        .to_string();
    let obligation_owner = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.obligation_owner",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(7)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
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
        kind: ProgramDetail::LendingWithdrawObligationCollateral(
            ProgramDetailLendingWithdrawObligationCollateral {
                source_collateral_account,
                destination_collateral_account,
                withdraw_reserve_account,
                obligation_account,
                lending_market_account,
                lending_market_authority_pubkey,
                obligation_owner,
                sysvar_clock,
                token_program_id,
                collateral_amount: collateral_amount.to_string(),
            },
        ),
    })
}

fn borrow_obligation_liquidity(
    accounts: Vec<String>,
    liquidity_amount: u64,
) -> Result<SolanaDetail> {
    let method_name = "BorrowObligationLiquidity";
    let source_liquidity_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_liquidity_account",
            method_name,
        )))?
        .to_string();
    let destination_liquidity_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.destination_liquidity_account",
            method_name
        )))?
        .to_string();
    let borrow_reserve_account = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.borrow_reserve_account",
            method_name
        )))?
        .to_string();
    let borrow_reserve_liquidity_fee_receiver_pubkey = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.borrow_reserve_liquidity_fee_receiver_pubkey",
            method_name
        )))?
        .to_string();
    let obligation_account = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.obligation_account",
            method_name
        )))?
        .to_string();
    let lending_market_account = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_account",
            method_name
        )))?
        .to_string();
    let lending_market_authority_pubkey = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_authority_pubkey",
            method_name
        )))?
        .to_string();
    let obligation_owner = accounts
        .get(7)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.obligation_owner",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(9)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
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
    let host_fee_receiver = accounts.get(10).unwrap_or(&"".to_string()).to_string();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::LendingBorrowObligationLiquidity(
            ProgramDetailLendingBorrowObligationLiquidity {
                source_liquidity_account,
                destination_liquidity_account,
                borrow_reserve_account,
                borrow_reserve_liquidity_fee_receiver_pubkey,
                obligation_account,
                lending_market_account,
                lending_market_authority_pubkey,
                obligation_owner,
                sysvar_clock,
                token_program_id,
                host_fee_receiver,
                liquidity_amount: liquidity_amount.to_string(),
            },
        ),
    })
}

fn repay_obligation_liquidity(
    accounts: Vec<String>,
    liquidity_amount: u64,
) -> Result<SolanaDetail> {
    let method_name = "RepayObligationLiquidity";

    let source_liquidity_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_liquidity_account",
            method_name,
        )))?
        .to_string();
    let destination_liquidity_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.destination_liquidity_account",
            method_name
        )))?
        .to_string();
    let repay_reserve_account = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.repay_reserve_account",
            method_name
        )))?
        .to_string();
    let obligation_account = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.obligation_account",
            method_name
        )))?
        .to_string();
    let lending_market_account = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_account",
            method_name
        )))?
        .to_string();
    let user_transfer_authority_pubkey = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_authority_pubkey",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
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
        kind: ProgramDetail::LendingRepayObligationLiquidity(
            ProgramDetailLendingRepayObligationLiquidity {
                source_liquidity_account,
                destination_liquidity_account,
                repay_reserve_account,
                obligation_account,
                lending_market_account,
                user_transfer_authority_pubkey,
                sysvar_clock,
                token_program_id,
                liquidity_amount: liquidity_amount.to_string(),
            },
        ),
    })
}

fn liquidate_obligation(accounts: Vec<String>, liquidity_amount: u64) -> Result<SolanaDetail> {
    let method_name = "LiquidateObligation";
    let source_liquidity_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_liquidity_account",
            method_name,
        )))?
        .to_string();
    let destination_collateral_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.destination_collateral_account",
            method_name
        )))?
        .to_string();
    let repay_reserve_account = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.repay_reserve_account",
            method_name
        )))?
        .to_string();
    let repay_reserve_liquidity_supply_pubkey = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.repay_reserve_liquidity_supply_pubkey",
            method_name
        )))?
        .to_string();
    let withdraw_reserve_account = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.withdraw_reserve_account",
            method_name
        )))?
        .to_string();
    let withdraw_reserve_collateral_supply_pubkey = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.withdraw_reserve_collateral_supply_pubkey",
            method_name
        )))?
        .to_string();
    let obligation_account = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.obligation_account",
            method_name
        )))?
        .to_string();
    let lending_market_account = accounts
        .get(7)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_account",
            method_name
        )))?
        .to_string();
    let lending_market_authority_pubkey = accounts
        .get(8)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_authority_pubkey",
            method_name
        )))?
        .to_string();
    let user_transfer_authority_pubkey = accounts
        .get(9)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.user_transfer_authority_pubkey",
            method_name
        )))?
        .to_string();
    let sysvar_clock = accounts
        .get(10)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.sysvar_clock",
            method_name
        )))?
        .to_string();
    let token_program_id = accounts
        .get(11)
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
        kind: ProgramDetail::LendingLiquidateObligation(ProgramDetailLendingLiquidateObligation {
            source_liquidity_account,
            destination_liquidity_account: destination_collateral_account,
            repay_reserve_account,
            repay_reserve_liquidity_supply_pubkey,
            withdraw_reserve_account,
            withdraw_reserve_collateral_supply_pubkey,
            obligation_account,
            lending_market_account,
            lending_market_authority_pubkey,
            user_transfer_authority_pubkey,
            sysvar_clock,
            token_program_id,
            liquidity_amount: liquidity_amount.to_string(),
        }),
    })
}

fn flash_loan(accounts: Vec<String>, amount: u64) -> Result<SolanaDetail> {
    let method_name = "FlashLoan";
    let source_liquidity_account = accounts
        .get(0)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.source_liquidity_account",
            method_name,
        )))?
        .to_string();
    let destination_liquidity_account = accounts
        .get(1)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.destination_liquidity_account",
            method_name
        )))?
        .to_string();
    let reserve_account = accounts
        .get(2)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_account",
            method_name
        )))?
        .to_string();
    let reserve_liquidity_fee_receiver = accounts
        .get(3)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.reserve_liquidity_fee_receiver",
            method_name
        )))?
        .to_string();
    let host_fee_receiver = accounts
        .get(4)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.host_fee_receiver",
            method_name
        )))?
        .to_string();
    let lending_market_account = accounts
        .get(5)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_account",
            method_name
        )))?
        .to_string();
    let lending_market_authority_pubkey = accounts
        .get(6)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.lending_market_authority_pubkey",
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
    let flash_loan_receiver_program_id = accounts
        .get(8)
        .ok_or(SolanaError::AccountNotFound(format!(
            "{}.flash_loan_receiver_program_id",
            method_name
        )))?
        .to_string();
    let flash_loan_receiver_program_accounts = accounts[9..].to_vec();
    Ok(SolanaDetail {
        common: CommonDetail {
            method: method_name.to_string(),
            program: PROGRAM_NAME.to_string(),
        },
        kind: ProgramDetail::LendingFlashLoan(ProgramDetailLendingFlashLoan {
            source_liquidity_account,
            destination_liquidity_account,
            reserve_account,
            reserve_liquidity_fee_receiver,
            host_fee_receiver,
            lending_market_account,
            lending_market_authority_pubkey,
            token_program_id,
            flash_loan_receiver_program_id,
            flash_loan_receiver_program_accounts,
            amount: amount.to_string(),
        }),
    })
}
