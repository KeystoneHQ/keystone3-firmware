//! Swap calculations

use crate::solana_lib::spl::token_swap::curve::errors::SwapError;
use crate::solana_lib::spl::token_swap::curve::math::PreciseNumber;
#[cfg(feature = "fuzz")]
use arbitrary::Arbitrary;
use core::fmt::Debug;

/// Initial amount of pool tokens for swap contract, hard-coded to something
/// "sensible" given a maximum of u128.
/// Note that on Ethereum, Uniswap uses the geometric mean of all provided
/// input amounts, and Balancer uses 100 * 10 ^ 18.
pub const INITIAL_SWAP_POOL_AMOUNT: u128 = 1_000_000_000;

/// Hardcode the number of token types in a pool, used to calculate the
/// equivalent pool tokens for the owner trading fee.
pub const TOKENS_IN_POOL: u128 = 2;

/// Helper function for mapping to SwapError::CalculationFailure
pub fn map_zero_to_none(x: u128) -> Option<u128> {
    if x == 0 {
        None
    } else {
        Some(x)
    }
}

/// The direction of a trade, since curves can be specialized to treat each
/// token differently (by adding offsets or weights)
#[cfg_attr(feature = "fuzz", derive(Arbitrary))]
#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum TradeDirection {
    /// Input token A, output token B
    AtoB,
    /// Input token B, output token A
    BtoA,
}

/// The direction to round.  Used for pool token to trading token conversions to
/// avoid losing value on any deposit or withdrawal.
#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum RoundDirection {
    /// Floor the value, ie. 1.9 => 1, 1.1 => 1, 1.5 => 1
    Floor,
    /// Ceiling the value, ie. 1.9 => 2, 1.1 => 2, 1.5 => 2
    Ceiling,
}

impl TradeDirection {
    /// Given a trade direction, gives the opposite direction of the trade, so
    /// A to B becomes B to A, and vice versa
    pub fn opposite(&self) -> TradeDirection {
        match self {
            TradeDirection::AtoB => TradeDirection::BtoA,
            TradeDirection::BtoA => TradeDirection::AtoB,
        }
    }
}

/// Encodes all results of swapping from a source token to a destination token
#[derive(Debug, PartialEq)]
pub struct SwapWithoutFeesResult {
    /// Amount of source token swapped
    pub source_amount_swapped: u128,
    /// Amount of destination token swapped
    pub destination_amount_swapped: u128,
}

/// Encodes results of depositing both sides at once
#[derive(Debug, PartialEq)]
pub struct TradingTokenResult {
    /// Amount of token A
    pub token_a_amount: u128,
    /// Amount of token B
    pub token_b_amount: u128,
}

/// Trait for packing of trait objects, required because structs that implement
/// `Pack` cannot be used as trait objects (as `dyn Pack`).
pub trait DynPack {
    /// Only required function is to pack given a trait object
    fn pack_into_slice(&self, dst: &mut [u8]);
}

/// Trait representing operations required on a swap curve
pub trait CurveCalculator: Debug + DynPack {
    /// Calculate how much destination token will be provided given an amount
    /// of source token.
    fn swap_without_fees(
        &self,
        source_amount: u128,
        swap_source_amount: u128,
        swap_destination_amount: u128,
        trade_direction: TradeDirection,
    ) -> Option<SwapWithoutFeesResult>;

    /// Get the supply for a new pool
    /// The default implementation is a Balancer-style fixed initial supply
    fn new_pool_supply(&self) -> u128 {
        INITIAL_SWAP_POOL_AMOUNT
    }

    /// Get the amount of trading tokens for the given amount of pool tokens,
    /// provided the total trading tokens and supply of pool tokens.
    fn pool_tokens_to_trading_tokens(
        &self,
        pool_tokens: u128,
        pool_token_supply: u128,
        swap_token_a_amount: u128,
        swap_token_b_amount: u128,
        round_direction: RoundDirection,
    ) -> Option<TradingTokenResult>;

    /// Get the amount of pool tokens for the deposited amount of token A or B.
    ///
    /// This is used for single-sided deposits.  It essentially performs a swap
    /// followed by a deposit.  Because a swap is implicitly performed, this will
    /// change the spot price of the pool.
    ///
    /// See more background for the calculation at:
    ///
    /// <https://balancer.finance/whitepaper/#single-asset-deposit-withdrawal>
    fn deposit_single_token_type(
        &self,
        source_amount: u128,
        swap_token_a_amount: u128,
        swap_token_b_amount: u128,
        pool_supply: u128,
        trade_direction: TradeDirection,
    ) -> Option<u128>;

    /// Get the amount of pool tokens for the withdrawn amount of token A or B.
    ///
    /// This is used for single-sided withdrawals and owner trade fee
    /// calculation. It essentially performs a withdrawal followed by a swap.
    /// Because a swap is implicitly performed, this will change the spot price
    /// of the pool.
    ///
    /// See more background for the calculation at:
    ///
    /// <https://balancer.finance/whitepaper/#single-asset-deposit-withdrawal>
    fn withdraw_single_token_type_exact_out(
        &self,
        source_amount: u128,
        swap_token_a_amount: u128,
        swap_token_b_amount: u128,
        pool_supply: u128,
        trade_direction: TradeDirection,
    ) -> Option<u128>;

    /// Validate that the given curve has no invalid parameters
    fn validate(&self) -> Result<(), SwapError>;

    /// Validate the given supply on initialization. This is useful for curves
    /// that allow zero supply on one or both sides, since the standard constant
    /// product curve must have a non-zero supply on both sides.
    fn validate_supply(&self, token_a_amount: u64, token_b_amount: u64) -> Result<(), SwapError> {
        if token_a_amount == 0 {
            return Err(SwapError::EmptySupply);
        }
        if token_b_amount == 0 {
            return Err(SwapError::EmptySupply);
        }
        Ok(())
    }

    /// Some curves function best and prevent attacks if we prevent deposits
    /// after initialization.  For example, the offset curve in `offset.rs`,
    /// which fakes supply on one side of the swap, allows the swap creator
    /// to steal value from all other depositors.
    fn allows_deposits(&self) -> bool {
        true
    }

    /// Calculates the total normalized value of the curve given the liquidity
    /// parameters.
    ///
    /// This value must have the dimension of `tokens ^ 1` For example, the
    /// standard Uniswap invariant has dimension `tokens ^ 2` since we are
    /// multiplying two token values together.  In order to normalize it, we
    /// also need to take the square root.
    ///
    /// This is useful for testing the curves, to make sure that value is not
    /// lost on any trade.  It can also be used to find out the relative value
    /// of pool tokens or liquidity tokens.
    fn normalized_value(
        &self,
        swap_token_a_amount: u128,
        swap_token_b_amount: u128,
    ) -> Option<PreciseNumber>;
}
