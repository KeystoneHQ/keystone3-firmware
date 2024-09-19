use alloc::string::{String, ToString};
use alloc::vec::Vec;

use serde::{Serialize, Serializer};
use serde::ser::SerializeStruct;

use crate::solana_lib::jupiter_v6::instructions::{
    ExactOutRouteArgs, RouteArgs, SharedAccountsExactOutRouteArgs, SharedAccountsRouteArgs,
};
use crate::solana_lib::solana_program::clock::{Epoch, UnixTimestamp};
use crate::solana_lib::solana_program::vote::state::Lockout;
use crate::solana_lib::spl::token_lending::state::{ReserveConfig, ReserveFees};
use crate::solana_lib::spl::token_swap::curve::base::{CurveType, SwapCurve};
use crate::solana_lib::spl::token_swap::curve::fees::Fees;
use crate::solana_lib::spl::token_swap::instruction::{
    DepositAllTokenTypes, DepositSingleTokenTypeExactAmountIn, Initialize, Swap,
    WithdrawAllTokenTypes, WithdrawSingleTokenTypeExactAmountOut,
};
use crate::solana_lib::squads_v4::instructions::{
    MultisigCreateArgs, MultisigCreateArgsV2, ProposalCreateArgs, ProposalVoteArgs,
    VaultTransactionCreateArgs,
};

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailSystemTransfer {
    pub value: String,
    pub from: String,
    pub to: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailSystemTransferWithSeed {
    pub from: String,
    pub to: String,
    pub amount: String,
    pub from_base_pubkey: String,
    pub from_owner: String,
    pub from_seed: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailSystemCreateAccount {
    pub funding_account: String,
    pub new_account: String,
    pub amount: String,
    pub space: String,
    pub owner: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailSystemAssign {
    pub account: String,
    pub new_owner: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailSystemCreateAccountWithSeed {
    pub funding_account: String,
    pub new_account: String,
    pub base_account: String,
    pub base_pubkey: String,
    pub seed: String,
    pub amount: String,
    pub space: String,
    pub owner: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailSystemAdvanceNonceAccount {
    pub nonce_account: String,
    pub recent_blockhashes_sysvar: String,
    pub nonce_authority_pubkey: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailVote {
    pub input_accounts: Vec<String>,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailVoteInitAccount {
    pub vote_account: String,
    pub sysvar_rent: String,
    pub sysvar_clock: String,
    pub new_validator_identity: String,
    pub node_pubkey: String,
    pub authorized_voter: String,
    pub authorized_withdrawer: String,
    pub commission: u8,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenInitializeMint {
    pub mint: String,
    pub sysver_rent: String,
    pub mint_authority_pubkey: String,
    pub freeze_authority_pubkey: String,
    pub decimals: u8,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailSystemWithdrawNonceAccount {
    pub nonce_account: String,
    pub recipient: String,
    pub recent_blockhashes_sysvar: String,
    pub rent_sysvar: String,
    pub nonce_authority_pubkey: String,
    pub amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailSystemInitializeNonceAccount {
    pub nonce_account: String,
    pub sysvar_recent_blockhashes: String,
    pub sysvar_rent: String,
    pub nonce_authority_pubkey: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailSystemAuthorizeNonceAccount {
    pub nonce_account: String,
    pub old_nonce_authority_pubkey: String,
    pub new_nonce_authority_pubkey: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct CommonDetail {
    pub program: String,
    #[serde(skip_serializing_if = "String::is_empty")]
    pub method: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailSystemAllocate {
    pub new_account: String,
    pub space: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailSystemAllocateWithSeed {
    pub allocated_account: String,
    pub base_account: String,
    pub base_pubkey: String,
    pub seed: String,
    pub space: String,
    pub owner: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailSystemAssignWithSeed {
    pub assigned_account: String,
    pub base_account: String,
    pub base_pubkey: String,
    pub seed: String,
    pub owner: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailSystemUpgradeNonceAccount {
    pub nonce_account: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailVoteAuthorize {
    pub vote_account: String,
    pub sysvar_clock: String,
    pub old_authority_pubkey: String,
    pub new_authority_pubkey: String,
    pub authority_type: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailVoteVote {
    pub vote_account: String,
    pub sysvar_slot_hashes: String,
    pub sysvar_clock: String,
    pub vote_authority_pubkey: String,
    pub slots: Vec<String>,
    pub hash: String,
    pub timestamp: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailVoteWithdraw {
    pub vote_account: String,
    pub recipient: String,
    pub withdraw_authority_pubkey: String,
    pub amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailVoteUpdateValidatorIdentity {
    pub vote_account: String,
    pub new_validator_identity: String,
    pub withdraw_authority: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailVoteUpdateCommission {
    pub vote_account: String,
    pub withdraw_authority_pubkey: String,
    pub new_commission: u8,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailVoteVoteSwitch {
    pub vote_account: String,
    pub sysvar_slot_hashes: String,
    pub sysvar_clock: String,
    pub vote_authority_pubkey: String,
    pub slots: Vec<String>,
    pub hash: String,
    pub timestamp: String,
    pub proof_hash: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailVoteAuthorizeChecked {
    pub vote_account: String,
    pub sysvar_clock: String,
    pub old_authority_pubkey: String,
    pub new_authority_pubkey: String,
    pub authority_type: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailVoteUpdateVoteState {
    pub vote_account: String,
    pub vote_authority_pubkey: String,
    pub lockouts: Vec<Lockout>,
    pub root: String,
    pub hash: String,
    pub timestamp: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailVoteUpdateVoteStateSwitch {
    pub vote_account: String,
    pub vote_authority_pubkey: String,
    pub lockouts: Vec<Lockout>,
    pub root: String,
    pub hash: String,
    pub timestamp: String,
    pub proof_hash: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailVoteAuthorizeWithSeed {
    pub vote_account: String,
    pub sysvar_clock: String,
    pub old_base_pubkey: String,
    pub authorization_type: String,
    pub current_authority_derived_key_owner: String,
    pub current_authority_derived_key_seed: String,
    pub new_authority_pubkey: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailVoteAuthorizeCheckedWithSeed {
    pub vote_account: String,
    pub sysvar_clock: String,
    pub old_base_pubkey: String,
    pub new_authority_pubkey: String,
    pub authorization_type: String,
    pub current_authority_derived_key_owner: String,
    pub current_authority_derived_key_seed: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenCloseAccount {
    pub account: String,
    pub recipient: String,
    pub owner: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signers: Option<Vec<String>>,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenBurn {
    pub account: String,
    pub mint: String,
    pub owner: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signers: Option<Vec<String>>,
    pub amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenMintTo {
    pub mint: String,
    pub mint_to_account: String,
    pub mint_authority_pubkey: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signers: Option<Vec<String>>,
    pub amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenMintToChecked {
    pub mint: String,
    pub mint_to_account: String,
    pub mint_authority_pubkey: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signers: Option<Vec<String>>,
    pub decimals: u8,
    pub amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenSetAuthority {
    pub account: String,
    pub old_authority_pubkey: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signers: Option<Vec<String>>,
    pub authority_type: String,
    pub new_authority_pubkey: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenRevoke {
    pub source_account: String,
    pub owner: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signers: Option<Vec<String>>,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenApprove {
    pub source_account: String,
    pub delegate_account: String,
    pub owner: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signers: Option<Vec<String>>,
    pub amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenTransfer {
    pub source_account: String,
    pub recipient: String,
    pub owner: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signers: Option<Vec<String>>,
    pub amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenInitializeMultisig {
    pub multisig_account: String,
    pub sysvar_rent: String,
    pub attendees: Vec<String>,
    pub required_signatures: u8,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenFreezeAccount {
    pub account: String,
    pub mint: String,
    pub mint_freeze_authority_pubkey: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signers: Option<Vec<String>>,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenThawAccount {
    pub account: String,
    pub mint: String,
    pub mint_freeze_authority_pubkey: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signers: Option<Vec<String>>,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenTransferChecked {
    pub account: String,
    pub mint: String,
    pub recipient: String,
    pub owner: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signers: Option<Vec<String>>,
    pub decimals: u8,
    pub amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenInitializeAccount {
    pub account: String,
    pub mint: String,
    pub owner: String,
    pub sysver_rent: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenBurnChecked {
    pub account: String,
    pub mint: String,
    pub owner: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signers: Option<Vec<String>>,
    pub decimals: u8,
    pub amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenInitializeAccount2 {
    pub account: String,
    pub mint: String,
    pub sysver_rent: String,
    pub owner: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenInitializeAccount3 {
    pub account: String,
    pub mint: String,
    pub owner: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenSyncNative {
    pub account_to_sync: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenInitializeMultisig2 {
    pub multisig_account: String,
    pub attendees: Vec<String>,
    pub required_signatures: u8,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenApproveChecked {
    pub account: String,
    pub mint: String,
    pub delegate: String,
    pub owner: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub signers: Option<Vec<String>>,
    pub decimals: u8,
    pub amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailTokenInitializeMint2 {
    pub mint: String,
    pub mint_authority_pubkey: String,
    pub freeze_authority_pubkey: String,
    pub decimals: u8,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailLendingInitLendingMarket {
    pub lending_market_account: String,
    pub sysvar_rent: String,
    pub token_program_id: String,
    pub oracle_program_id: String,
    pub owner: String,
    pub quote_currency: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailLendingSetLendingMarketOwner {
    pub lending_market_account: String,
    pub current_owner: String,
    pub new_owner: String,
}

fn reserve_config_serialize<S>(x: &ReserveConfig, serializer: S) -> Result<S::Ok, S::Error>
where
    S: Serializer,
{
    let mut state = serializer.serialize_struct("ReserveConfig", 8)?;
    state.serialize_field("optimal_utilization_rate", &x.optimal_utilization_rate)?;
    state.serialize_field("loan_to_value_ratio", &x.loan_to_value_ratio)?;
    state.serialize_field("liquidation_bonus", &x.liquidation_bonus)?;
    state.serialize_field("liquidation_threshold", &x.liquidation_threshold)?;
    state.serialize_field("min_borrow_rate", &x.min_borrow_rate)?;
    state.serialize_field("optimal_borrow_rate", &x.optimal_borrow_rate)?;
    state.serialize_field("max_borrow_rate", &x.max_borrow_rate)?;
    state.serialize_field("fees", &x.fees)?;
    state.end()
}

impl Serialize for ReserveFees {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let mut s = serializer.serialize_struct("ReserveFees", 3)?;
        s.serialize_field("borrow_fee_wad", &self.borrow_fee_wad)?;
        s.serialize_field("flash_loan_fee_wad", &self.flash_loan_fee_wad)?;
        s.serialize_field("host_fee_percentage", &self.host_fee_percentage)?;
        s.end()
    }
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailLendingInitReserve {
    pub source_liquidity_account: String,
    pub destination_collateral_account: String,
    pub reserve_account: String,
    pub reserve_liquidity_mint: String,
    pub reserve_liquidity_supply_account: String,
    pub reserve_liquidity_fee_receiver: String,
    pub reserve_collateral_mint: String,
    pub reserve_collateral_supply_pubkey: String,
    pub pyth_product_account: String,
    pub pyth_price_account: String,
    pub lending_market_account: String,
    pub lending_market_authority_pubkey: String,
    pub lending_market_owner: String,
    pub user_transfer_authority_pubkey: String,
    pub sysvar_clock: String,
    pub sysvar_rent: String,
    pub token_program_id: String,
    pub liquidity_amount: String,
    #[serde(serialize_with = "reserve_config_serialize")]
    pub reserve_config: ReserveConfig,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailLendingRefreshReserve {
    pub reserve_account: String,
    pub reserve_liquidity_oracle_account: String,
    pub sysvar_clock: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailLendingDepositReserveLiquidity {
    pub source_liquidity_account: String,
    pub destination_collateral_account: String,
    pub reserve_account: String,
    pub reserve_liquidity_supply_account: String,
    pub reserve_collateral_mint: String,
    pub lending_market_account: String,
    pub lending_market_authority_pubkey: String,
    pub user_transfer_authority_pubkey: String,
    pub sysvar_clock: String,
    pub token_program_id: String,
    pub liquidity_amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailLendingRedeemReserveCollateral {
    pub source_collateral_account: String,
    pub destination_liquidity_account: String,
    pub reserve_account: String,
    pub reserve_liquidity_supply_account: String,
    pub reserve_collateral_mint: String,
    pub lending_market_account: String,
    pub lending_market_authority_pubkey: String,
    pub user_transfer_authority_pubkey: String,
    pub sysvar_clock: String,
    pub token_program_id: String,
    pub collateral_amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailLendingInitObligation {
    pub obligation_account: String,
    pub lending_market_account: String,
    pub obligation_owner: String,
    pub sysvar_clock: String,
    pub sysvar_rent: String,
    pub token_program_id: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailLendingRefreshObligation {
    pub obligation_account: String,
    pub sysvar_clock: String,
    pub keys: Vec<String>,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailLendingWithdrawObligationCollateral {
    pub source_collateral_account: String,
    pub destination_collateral_account: String,
    pub withdraw_reserve_account: String,
    pub obligation_account: String,
    pub lending_market_account: String,
    pub lending_market_authority_pubkey: String,
    pub obligation_owner: String,
    pub sysvar_clock: String,
    pub token_program_id: String,
    pub collateral_amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailLendingDepositObligationCollateral {
    pub source_collateral_account: String,
    pub destination_collateral_account: String,
    pub deposit_reserve_pubkey: String,
    pub obligation_account: String,
    pub lending_market_account: String,
    pub obligation_owner: String,
    pub user_transfer_authority_pubkey: String,
    pub sysvar_clock: String,
    pub token_program_id: String,
    pub collateral_amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailLendingRepayObligationLiquidity {
    pub source_liquidity_account: String,
    pub destination_liquidity_account: String,
    pub repay_reserve_account: String,
    pub obligation_account: String,
    pub lending_market_account: String,
    pub user_transfer_authority_pubkey: String,
    pub sysvar_clock: String,
    pub token_program_id: String,
    pub liquidity_amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailLendingLiquidateObligation {
    pub source_liquidity_account: String,
    pub destination_liquidity_account: String,
    pub repay_reserve_account: String,
    pub repay_reserve_liquidity_supply_pubkey: String,
    pub withdraw_reserve_account: String,
    pub withdraw_reserve_collateral_supply_pubkey: String,
    pub obligation_account: String,
    pub lending_market_account: String,
    pub lending_market_authority_pubkey: String,
    pub user_transfer_authority_pubkey: String,
    pub sysvar_clock: String,
    pub token_program_id: String,
    pub liquidity_amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailLendingBorrowObligationLiquidity {
    pub source_liquidity_account: String,
    pub destination_liquidity_account: String,
    pub borrow_reserve_account: String,
    pub borrow_reserve_liquidity_fee_receiver_pubkey: String,
    pub obligation_account: String,
    pub lending_market_account: String,
    pub lending_market_authority_pubkey: String,
    pub obligation_owner: String,
    pub sysvar_clock: String,
    pub token_program_id: String,
    pub host_fee_receiver: String,
    pub liquidity_amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailLendingFlashLoan {
    pub source_liquidity_account: String,
    pub destination_liquidity_account: String,
    pub reserve_account: String,
    pub reserve_liquidity_fee_receiver: String,
    pub host_fee_receiver: String,
    pub lending_market_account: String,
    pub lending_market_authority_pubkey: String,
    pub token_program_id: String,
    pub flash_loan_receiver_program_id: String,
    pub flash_loan_receiver_program_accounts: Vec<String>,
    pub amount: String,
}

#[derive(Debug, Clone, Serialize)]
pub struct ProgramDetailSwapV3Initialize {
    pub token_swap: String,
    pub swap_authority_pubkey: String,
    pub token_a: String,
    pub token_b: String,
    pub pool_mint: String,
    pub pool_token_account_1: String,
    pub pool_token_account_2: String,
    pub token_program_id: String,
    pub initialize: Initialize,
}

impl Clone for Initialize {
    fn clone(&self) -> Self {
        Self {
            fees: self.fees.clone(),
            swap_curve: self.swap_curve.clone(),
        }
    }
}

impl Clone for SwapCurve {
    fn clone(&self) -> Self {
        Self {
            curve_type: self.curve_type,
            calculator: self.calculator.clone(),
        }
    }
}

impl Serialize for Initialize {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let mut s = serializer.serialize_struct("Initialize", 2)?;
        s.serialize_field("fees", &self.fees)?;
        s.serialize_field("swap_curve", &self.swap_curve)?;
        s.end()
    }
}

impl Serialize for Fees {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let mut s = serializer.serialize_struct("Fees", 8)?;
        s.serialize_field("trade_fee_numerator", &self.trade_fee_numerator)?;
        s.serialize_field("trade_fee_denominator", &self.trade_fee_denominator)?;
        s.serialize_field("owner_trade_fee_numerator", &self.owner_trade_fee_numerator)?;
        s.serialize_field(
            "owner_trade_fee_denominator",
            &self.owner_trade_fee_denominator,
        )?;
        s.serialize_field(
            "owner_withdraw_fee_numerator",
            &self.owner_withdraw_fee_numerator,
        )?;
        s.serialize_field(
            "owner_withdraw_fee_denominator",
            &self.owner_withdraw_fee_denominator,
        )?;
        s.serialize_field("host_fee_numerator", &self.host_fee_numerator)?;
        s.serialize_field("host_fee_denominator", &self.host_fee_denominator)?;
        s.end()
    }
}

impl Serialize for SwapCurve {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let mut s = serializer.serialize_struct("SwapCurve", 1)?;
        s.serialize_field("curve_type", &self.curve_type)?;
        s.end()
    }
}

impl Serialize for CurveType {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let curve_type = match self {
            CurveType::Offset => "offset",
            CurveType::Stable => "stable",
            CurveType::ConstantPrice => "constant_price",
            CurveType::ConstantProduct => "constant_product",
        };
        serializer.serialize_str(curve_type)
    }
}

#[derive(Debug, Clone, Serialize)]
pub struct ProgramDetailSwapV3Swap {
    pub token_swap: String,
    pub swap_authority_pubkey: String,
    pub user_transfer_authority_pubkey: String,
    pub source_account: String,
    pub source_token: String,
    pub destination_token: String,
    pub destination_account: String,
    pub pool_mint: String,
    pub fee_account: String,
    pub token_program_id: String,
    pub host_fee_account: String,
    pub swap: Swap,
}
impl Serialize for Swap {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let mut s = serializer.serialize_struct("Swap", 2)?;
        s.serialize_field("amount_in", &self.amount_in)?;
        s.serialize_field("minimum_amount_out", &self.minimum_amount_out)?;
        s.end()
    }
}

#[derive(Debug, Clone, Serialize)]
pub struct ProgramDetailSwapV3DepositAllTokenTypes {
    pub token_swap: String,
    pub swap_authority_pubkey: String,
    pub user_transfer_authority_pubkey: String,
    pub token_a_user_transfer_authority_pubkey: String,
    pub token_b_user_transfer_authority_pubkey: String,
    pub token_a_base_account: String,
    pub token_b_base_account: String,
    pub pool_mint: String,
    pub pool_account: String,
    pub token_program_id: String,
    pub deposit_all_token_types: DepositAllTokenTypes,
}
impl Serialize for DepositAllTokenTypes {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let mut s = serializer.serialize_struct("DepositAllTokenTypes", 3)?;
        s.serialize_field("pool_token_amount", &self.pool_token_amount)?;
        s.serialize_field("maximum_token_a_amount", &self.maximum_token_a_amount)?;
        s.serialize_field("maximum_token_b_amount", &self.maximum_token_b_amount)?;
        s.end()
    }
}

#[derive(Debug, Clone, Serialize)]
pub struct ProgramDetailSwapV3WithdrawAllTokenTypes {
    pub token_swap: String,
    pub swap_authority_pubkey: String,
    pub user_transfer_authority_pubkey: String,
    pub pool_mint: String,
    pub source_pool_account: String,
    pub token_a_swap_account: String,
    pub token_b_swap_account: String,
    pub token_a_user_account: String,
    pub token_b_user_account: String,
    pub fee_account: String,
    pub token_program_id: String,
    pub withdraw_all_token_types: WithdrawAllTokenTypes,
}

impl Serialize for WithdrawAllTokenTypes {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let mut s = serializer.serialize_struct("WithdrawAllTokenTypes", 3)?;
        s.serialize_field("pool_token_amount", &self.pool_token_amount)?;
        s.serialize_field("minimum_token_a_amount", &self.minimum_token_a_amount)?;
        s.serialize_field("minimum_token_b_amount", &self.minimum_token_b_amount)?;
        s.end()
    }
}

#[derive(Debug, Clone, Serialize)]
pub struct ProgramDetailSwapV3DepositSingleTokenTypeExactAmountIn {
    pub token_swap: String,
    pub swap_authority_pubkey: String,
    pub user_transfer_authority_pubkey: String,
    pub token_source_account: String,
    pub token_a_swap_account: String,
    pub token_b_swap_account: String,
    pub pool_mint: String,
    pub pool_account: String,
    pub token_program_id: String,
    pub arguments: DepositSingleTokenTypeExactAmountIn,
}

impl Serialize for DepositSingleTokenTypeExactAmountIn {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let mut s = serializer.serialize_struct("DepositSingleTokenTypeExactAmountIn", 2)?;
        s.serialize_field("source_token_amount", &self.source_token_amount)?;
        s.serialize_field("minimum_pool_token_amount", &self.minimum_pool_token_amount)?;
        s.end()
    }
}

#[derive(Debug, Clone, Serialize)]
pub struct ProgramDetailSwapV3WithdrawSingleTokenTypeExactAmountOut {
    pub token_swap: String,
    pub swap_authority_pubkey: String,
    pub user_transfer_authority_pubkey: String,
    pub pool_mint: String,
    pub source_pool_account: String,
    pub token_a_swap_account: String,
    pub token_b_swap_account: String,
    pub token_user_account: String,
    pub fee_account: String,
    pub token_program_id: String,
    pub arguments: WithdrawSingleTokenTypeExactAmountOut,
}

impl Serialize for WithdrawSingleTokenTypeExactAmountOut {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let mut s = serializer.serialize_struct("WithdrawSingleTokenTypeExactAmountOut", 2)?;
        s.serialize_field("destination_token_amount", &self.destination_token_amount)?;
        s.serialize_field("maximum_pool_token_amount", &self.maximum_pool_token_amount)?;
        s.end()
    }
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeDelegateStake {
    pub stake_account: String,
    pub vote_account: String,
    pub sysvar_clock: String,
    pub sysvar_stake_history: String,
    pub config_account: String,
    pub stake_authority_pubkey: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeWithdraw {
    pub stake_account: String,
    pub recipient: String,
    pub sysvar_clock: String,
    pub sysvar_stake_history: String,
    pub withdraw_authority_pubkey: String,
    pub stake_authority_pubkey: String,
    pub amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeInitialize {
    pub stake_account: String,
    pub sysvar_rent: String,
    pub staker: String,
    pub withdrawer: String,
    pub timestamp: UnixTimestamp,
    pub epoch: Epoch,
    pub custodian: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeAuthorize {
    pub stake_account: String,
    pub sysvar_clock: String,
    pub old_authority_pubkey: String,
    pub lockup_authority_pubkey: String,
    pub new_authority_pubkey: String,
    pub authorize_type: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeSplit {
    pub stake_account: String,
    pub target_account: String,
    pub stake_authority_pubkey: String,
    pub amount: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeDeactivate {
    pub delegated_stake_account: String,
    pub sysvar_clock: String,
    pub stake_authority_pubkey: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeSetLockup {
    pub stake_account: String,
    pub lockup_authority_pubkey: String,
    pub unix_timestamp: UnixTimestamp,
    pub epoch: Epoch,
    pub custodian: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeMerge {
    pub destination_stake_account: String,
    pub source_stake_account: String,
    pub sysvar_clock: String,
    pub sysvar_stake_history: String,
    pub stake_authority_pubkey: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeAuthorizeWithSeed {
    pub stake_account: String,
    pub old_base_pubkey: String,
    pub sysvar_clock: String,
    pub lockup_authority_pubkey: String,
    pub new_authority_pubkey: String,
    pub authorize_type: String,
    pub authority_seed: String,
    pub authority_owner: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeInitializeChecked {
    pub stake_account: String,
    pub sysvar_rent: String,
    pub stake_authority_pubkey: String,
    pub withdraw_authority_pubkey: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeSetLockupChecked {
    pub stake_account: String,
    pub lockup_authority_pubkey: String,
    pub new_lockup_authority_pubkey: String,
    pub timestamp: UnixTimestamp,
    pub epoch: Epoch,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeAuthorizeChecked {
    pub stake_account: String,
    pub sysvar_clock: String,
    pub old_authority_pubkey: String,
    pub new_authority_pubkey: String,
    pub lockup_authority_pubkey: String,
    pub authority_type: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeAuthorizeCheckedWithSeed {
    pub stake_account: String,
    pub sysvar_clock: String,
    pub old_base_pubkey: String,
    pub new_authority_pubkey: String,
    pub lockup_authority: String,
    pub authority_type: String,
    pub authority_seed: String,
    pub authority_owner: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeGetMinimumDelegation {}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailStakeDeactivateDelinquent {}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailUnknown {
    pub program_index: u8,
    pub account_indexes: Vec<u8>,
    pub data: String,
    pub reason: String,
    pub accounts: String,
    pub program_account: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailInstruction {
    pub data: String,
    pub accounts: Vec<String>,
    pub program_account: String,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailGeneralUnknown {
    pub reason: String,
}
impl ProgramDetailGeneralUnknown {
    pub fn from_unknown_detail(detail: &ProgramDetailUnknown) -> Self {
        Self {
            reason: detail.reason.to_string(),
        }
    }
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct ProgramDetailRawUnknown {
    pub program_index: u8,
    pub account_indexes: Vec<u8>,
    pub data: String,
    pub accounts: String,
    pub program_account: String,
}

impl ProgramDetailRawUnknown {
    pub fn from_unknown_detail(detail: &ProgramDetailUnknown) -> Self {
        Self {
            program_index: detail.program_index,
            account_indexes: detail.account_indexes.to_vec(),
            data: detail.data.to_string(),
            accounts: detail.accounts.to_string(),
            program_account: detail.program_account.to_string(),
        }
    }
}
#[derive(Debug, Clone, Default, Serialize)]
pub struct JupiterV6SharedAccountsRouteDetail {
    pub accounts: Vec<String>,
    pub args: SharedAccountsRouteArgs,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct JupiterV6RouteDetail {
    pub accounts: Vec<String>,
    pub args: RouteArgs,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct JupiterV6ExactOutRouteDetail {
    pub accounts: Vec<String>,
    pub args: ExactOutRouteArgs,
}

#[derive(Debug, Clone, Default, Serialize)]
pub struct JupiterV6SharedAccountsExactOutRouteDetail {
    pub accounts: Vec<String>,
    pub args: SharedAccountsExactOutRouteArgs,
}

#[derive(Debug, Clone, Serialize)]
#[serde(untagged)]
pub enum ProgramDetail {
    // system
    SystemTransfer(ProgramDetailSystemTransfer),
    SystemTransferWithSeed(ProgramDetailSystemTransferWithSeed),
    SystemCreateAccount(ProgramDetailSystemCreateAccount),
    SystemAssign(ProgramDetailSystemAssign),
    SystemAssignWithSeed(ProgramDetailSystemAssignWithSeed),
    SystemCreateAccountWithSeed(ProgramDetailSystemCreateAccountWithSeed),
    SystemAdvanceNonceAccount(ProgramDetailSystemAdvanceNonceAccount),
    SystemWithdrawNonceAccount(ProgramDetailSystemWithdrawNonceAccount),
    SystemInitializeNonceAccount(ProgramDetailSystemInitializeNonceAccount),
    SystemAuthorizeNonceAccount(ProgramDetailSystemAuthorizeNonceAccount),
    SystemAllocate(ProgramDetailSystemAllocate),
    SystemAllocateWithSeed(ProgramDetailSystemAllocateWithSeed),
    SystemUpgradeNonceAccount(ProgramDetailSystemUpgradeNonceAccount),
    // vote
    VoteWithdraw(ProgramDetailVoteWithdraw),
    VoteUpdateValidatorIdentity(ProgramDetailVoteUpdateValidatorIdentity),
    VoteUpdateCommission(ProgramDetailVoteUpdateCommission),
    VoteVoteSwitch(ProgramDetailVoteVoteSwitch),
    VoteAuthorizeChecked(ProgramDetailVoteAuthorizeChecked),
    VoteUpdateVoteState(ProgramDetailVoteUpdateVoteState),
    VoteUpdateVoteStateSwitch(ProgramDetailVoteUpdateVoteStateSwitch),
    VoteAuthorizeWithSeed(ProgramDetailVoteAuthorizeWithSeed),
    VoteAuthorizeCheckedWithSeed(ProgramDetailVoteAuthorizeCheckedWithSeed),
    VoteInitializeAccount(ProgramDetailVoteInitAccount),
    VoteAuthorize(ProgramDetailVoteAuthorize),
    VoteVote(ProgramDetailVoteVote),

    // token
    TokenInitializeMint(ProgramDetailTokenInitializeMint),
    TokenInitializeAccount(ProgramDetailTokenInitializeAccount),
    TokenInitializeMultisig(ProgramDetailTokenInitializeMultisig),
    TokenTransfer(ProgramDetailTokenTransfer),
    TokenApprove(ProgramDetailTokenApprove),
    TokenRevoke(ProgramDetailTokenRevoke),
    TokenSetAuthority(ProgramDetailTokenSetAuthority),
    TokenMintTo(ProgramDetailTokenMintTo),
    TokenBurn(ProgramDetailTokenBurn),
    TokenCloseAccount(ProgramDetailTokenCloseAccount),
    TokenFreezeAccount(ProgramDetailTokenFreezeAccount),
    TokenThawAccount(ProgramDetailTokenThawAccount),
    TokenTransferChecked(ProgramDetailTokenTransferChecked),
    TokenApproveChecked(ProgramDetailTokenApproveChecked),
    TokenMintToChecked(ProgramDetailTokenMintToChecked),
    TokenBurnChecked(ProgramDetailTokenBurnChecked),
    TokenInitializeAccount2(ProgramDetailTokenInitializeAccount2),
    TokenSyncNative(ProgramDetailTokenSyncNative),
    TokenInitializeAccount3(ProgramDetailTokenInitializeAccount3),
    TokenInitializeMultisig2(ProgramDetailTokenInitializeMultisig2),
    TokenInitializeMint2(ProgramDetailTokenInitializeMint2),

    // token lending
    LendingInitLendingMarket(ProgramDetailLendingInitLendingMarket),
    LendingSetLendingMarketOwner(ProgramDetailLendingSetLendingMarketOwner),
    LendingInitReserve(ProgramDetailLendingInitReserve),
    LendingRefreshReserve(ProgramDetailLendingRefreshReserve),
    LendingDepositReserveLiquidity(ProgramDetailLendingDepositReserveLiquidity),
    LendingRedeemReserveCollateral(ProgramDetailLendingRedeemReserveCollateral),
    LendingInitObligation(ProgramDetailLendingInitObligation),
    LendingRefreshObligation(ProgramDetailLendingRefreshObligation),
    LendingDepositObligationCollateral(ProgramDetailLendingDepositObligationCollateral),
    LendingWithdrawObligationCollateral(ProgramDetailLendingWithdrawObligationCollateral),
    LendingBorrowObligationLiquidity(ProgramDetailLendingBorrowObligationLiquidity),
    LendingRepayObligationLiquidity(ProgramDetailLendingRepayObligationLiquidity),
    LendingLiquidateObligation(ProgramDetailLendingLiquidateObligation),
    LendingFlashLoan(ProgramDetailLendingFlashLoan),

    //token swap v3
    SwapV3Initialize(ProgramDetailSwapV3Initialize),
    SwapV3Swap(ProgramDetailSwapV3Swap),
    SwapV3DepositAllTokenTypes(ProgramDetailSwapV3DepositAllTokenTypes),
    SwapV3WithdrawAllTokenTypes(ProgramDetailSwapV3WithdrawAllTokenTypes),
    SwapV3DepositSingleTokenTypeExactAmountIn(
        ProgramDetailSwapV3DepositSingleTokenTypeExactAmountIn,
    ),
    SwapV3WithdrawSingleTokenTypeExactAmountOut(
        ProgramDetailSwapV3WithdrawSingleTokenTypeExactAmountOut,
    ),

    // stake
    StakeDelegateStake(ProgramDetailStakeDelegateStake),
    StakeWithdraw(ProgramDetailStakeWithdraw),
    StakeInitialize(ProgramDetailStakeInitialize),
    StakeAuthorize(ProgramDetailStakeAuthorize),
    StakeSplit(ProgramDetailStakeSplit),
    StakeDeactivate(ProgramDetailStakeDeactivate),
    StakeSetLockup(ProgramDetailStakeSetLockup),
    StakeMerge(ProgramDetailStakeMerge),
    StakeAuthorizeWithSeed(ProgramDetailStakeAuthorizeWithSeed),
    StakeInitializeChecked(ProgramDetailStakeInitializeChecked),
    StakeAuthorizeChecked(ProgramDetailStakeAuthorizeChecked),
    StakeAuthorizeCheckedWithSeed(ProgramDetailStakeAuthorizeCheckedWithSeed),
    StakeSetLockupChecked(ProgramDetailStakeSetLockupChecked),
    StakeGetMinimumDelegation(ProgramDetailStakeGetMinimumDelegation),
    StakeDeactivateDelinquent(ProgramDetailStakeDeactivateDelinquent),

    // squads v4
    SquadsV4MultisigCreate(MultisigCreateArgs),
    SquadsV4MultisigCreateV2(MultisigCreateArgsV2),
    SquadsV4ProposalActivate,
    SquadsV4ProposalCreate(ProposalCreateArgs),
    SquadsV4ProposalApprove(ProposalVoteArgs),
    SquadsV4ProposalCancel(ProposalVoteArgs),
    SquadsV4ProposalReject(ProposalVoteArgs),
    SquadsV4VaultTransactionCreate(VaultTransactionCreateArgs),
    SquadsV4VaultTransactionExecute,

    // jupiter v6
    JupiterV6SharedAccountsRoute(JupiterV6SharedAccountsRouteDetail),
    JupiterV6Route(JupiterV6RouteDetail),
    JupiterV6ExactOutRoute(JupiterV6ExactOutRouteDetail),
    JupiterV6SharedAccountsExactOutRoute(JupiterV6SharedAccountsExactOutRouteDetail),

    // raw
    Unknown(ProgramDetailUnknown),
    GeneralUnknown(ProgramDetailGeneralUnknown),
    RawUnknown(ProgramDetailRawUnknown),
    Instruction(ProgramDetailInstruction),
}

#[derive(Debug, Clone, Serialize)]
pub struct SolanaDetail {
    #[serde(flatten)]
    pub common: CommonDetail,
    #[serde(flatten)]
    pub kind: ProgramDetail,
}
