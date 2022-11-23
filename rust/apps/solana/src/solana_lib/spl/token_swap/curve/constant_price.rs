use crate::solana_lib::solana_program::errors::ProgramError;
use crate::solana_lib::solana_program::program_pack::{IsInitialized, Pack, Sealed};
use crate::solana_lib::spl::token_swap::curve::calculator::{
    map_zero_to_none, CurveCalculator, DynPack, RoundDirection, SwapWithoutFeesResult,
    TradeDirection, TradingTokenResult,
};
use crate::solana_lib::spl::token_swap::curve::errors::SwapError;
use crate::solana_lib::spl::token_swap::curve::math::{CheckedCeilDiv, PreciseNumber, U256};
use arrayref::{array_mut_ref, array_ref};

/// Get the amount of pool tokens for the given amount of token A or B.
///
/// The constant product implementation uses the Balancer formulas found at
/// <https://balancer.finance/whitepaper/#single-asset-deposit>, specifically
/// in the case for 2 tokens, each weighted at 1/2.
pub fn trading_tokens_to_pool_tokens(
    token_b_price: u64,
    source_amount: u128,
    swap_token_a_amount: u128,
    swap_token_b_amount: u128,
    pool_supply: u128,
    trade_direction: TradeDirection,
    round_direction: RoundDirection,
) -> Option<u128> {
    let token_b_price = U256::from(token_b_price);
    let given_value = match trade_direction {
        TradeDirection::AtoB => U256::from(source_amount),
        TradeDirection::BtoA => U256::from(source_amount).checked_mul(token_b_price)?,
    };
    let total_value = U256::from(swap_token_b_amount)
        .checked_mul(token_b_price)?
        .checked_add(U256::from(swap_token_a_amount))?;
    let pool_supply = U256::from(pool_supply);
    match round_direction {
        RoundDirection::Floor => Some(
            pool_supply
                .checked_mul(given_value)?
                .checked_div(total_value)?
                .as_u128(),
        ),
        RoundDirection::Ceiling => Some(
            pool_supply
                .checked_mul(given_value)?
                .checked_ceil_div(total_value)?
                .0
                .as_u128(),
        ),
    }
}

/// ConstantPriceCurve struct implementing CurveCalculator
#[derive(Clone, Debug, Default, PartialEq)]
pub struct ConstantPriceCurve {
    /// Amount of token A required to get 1 token B
    pub token_b_price: u64,
}

impl CurveCalculator for ConstantPriceCurve {
    /// Constant price curve always returns 1:1
    fn swap_without_fees(
        &self,
        source_amount: u128,
        _swap_source_amount: u128,
        _swap_destination_amount: u128,
        trade_direction: TradeDirection,
    ) -> Option<SwapWithoutFeesResult> {
        let token_b_price = self.token_b_price as u128;

        let (source_amount_swapped, destination_amount_swapped) = match trade_direction {
            TradeDirection::BtoA => (source_amount, source_amount.checked_mul(token_b_price)?),
            TradeDirection::AtoB => {
                let destination_amount_swapped = source_amount.checked_div(token_b_price)?;
                let mut source_amount_swapped = source_amount;

                // if there is a remainder from buying token B, floor
                // token_a_amount to avoid taking too many tokens, but
                // don't recalculate the fees
                let remainder = source_amount_swapped.checked_rem(token_b_price)?;
                if remainder > 0 {
                    source_amount_swapped = source_amount.checked_sub(remainder)?;
                }

                (source_amount_swapped, destination_amount_swapped)
            }
        };
        let source_amount_swapped = map_zero_to_none(source_amount_swapped)?;
        let destination_amount_swapped = map_zero_to_none(destination_amount_swapped)?;
        Some(SwapWithoutFeesResult {
            source_amount_swapped,
            destination_amount_swapped,
        })
    }

    /// Get the amount of trading tokens for the given amount of pool tokens,
    /// provided the total trading tokens and supply of pool tokens.
    /// For the constant price curve, the total value of the pool is weighted
    /// by the price of token B.
    fn pool_tokens_to_trading_tokens(
        &self,
        pool_tokens: u128,
        pool_token_supply: u128,
        swap_token_a_amount: u128,
        swap_token_b_amount: u128,
        round_direction: RoundDirection,
    ) -> Option<TradingTokenResult> {
        let token_b_price = self.token_b_price as u128;
        let total_value = self
            .normalized_value(swap_token_a_amount, swap_token_b_amount)?
            .to_imprecise()?;

        let (token_a_amount, token_b_amount) = match round_direction {
            RoundDirection::Floor => {
                let token_a_amount = pool_tokens
                    .checked_mul(total_value)?
                    .checked_div(pool_token_supply)?;
                let token_b_amount = pool_tokens
                    .checked_mul(total_value)?
                    .checked_div(token_b_price)?
                    .checked_div(pool_token_supply)?;
                (token_a_amount, token_b_amount)
            }
            RoundDirection::Ceiling => {
                let (token_a_amount, _) = pool_tokens
                    .checked_mul(total_value)?
                    .checked_ceil_div(pool_token_supply)?;
                let (pool_value_as_token_b, _) = pool_tokens
                    .checked_mul(total_value)?
                    .checked_ceil_div(token_b_price)?;
                let (token_b_amount, _) =
                    pool_value_as_token_b.checked_ceil_div(pool_token_supply)?;
                (token_a_amount, token_b_amount)
            }
        };
        Some(TradingTokenResult {
            token_a_amount,
            token_b_amount,
        })
    }

    /// Get the amount of pool tokens for the given amount of token A and B
    /// For the constant price curve, the total value of the pool is weighted
    /// by the price of token B.
    fn deposit_single_token_type(
        &self,
        source_amount: u128,
        swap_token_a_amount: u128,
        swap_token_b_amount: u128,
        pool_supply: u128,
        trade_direction: TradeDirection,
    ) -> Option<u128> {
        trading_tokens_to_pool_tokens(
            self.token_b_price,
            source_amount,
            swap_token_a_amount,
            swap_token_b_amount,
            pool_supply,
            trade_direction,
            RoundDirection::Floor,
        )
    }

    fn withdraw_single_token_type_exact_out(
        &self,
        source_amount: u128,
        swap_token_a_amount: u128,
        swap_token_b_amount: u128,
        pool_supply: u128,
        trade_direction: TradeDirection,
    ) -> Option<u128> {
        trading_tokens_to_pool_tokens(
            self.token_b_price,
            source_amount,
            swap_token_a_amount,
            swap_token_b_amount,
            pool_supply,
            trade_direction,
            RoundDirection::Ceiling,
        )
    }

    fn validate(&self) -> Result<(), SwapError> {
        if self.token_b_price == 0 {
            Err(SwapError::InvalidCurve)
        } else {
            Ok(())
        }
    }

    fn validate_supply(&self, token_a_amount: u64, _token_b_amount: u64) -> Result<(), SwapError> {
        if token_a_amount == 0 {
            return Err(SwapError::EmptySupply);
        }
        Ok(())
    }

    /// The total normalized value of the constant price curve adds the total
    /// value of the token B side to the token A side.
    ///
    /// Note that since most other curves use a multiplicative invariant, ie.
    /// `token_a * token_b`, whereas this one uses an addition,
    /// ie. `token_a + token_b`.
    ///
    /// At the end, we divide by 2 to normalize the value between the two token
    /// types.
    fn normalized_value(
        &self,
        swap_token_a_amount: u128,
        swap_token_b_amount: u128,
    ) -> Option<PreciseNumber> {
        let swap_token_b_value = swap_token_b_amount.checked_mul(self.token_b_price as u128)?;
        // special logic in case we're close to the limits, avoid overflowing u128
        let value = if swap_token_b_value.saturating_sub(u64::MAX.into())
            > (u128::MAX.saturating_sub(u64::MAX.into()))
        {
            swap_token_b_value
                .checked_div(2)?
                .checked_add(swap_token_a_amount.checked_div(2)?)?
        } else {
            swap_token_a_amount
                .checked_add(swap_token_b_value)?
                .checked_div(2)?
        };
        PreciseNumber::new(value)
    }
}

/// IsInitialized is required to use `Pack::pack` and `Pack::unpack`
impl IsInitialized for ConstantPriceCurve {
    fn is_initialized(&self) -> bool {
        true
    }
}
impl Sealed for ConstantPriceCurve {}
impl Pack for ConstantPriceCurve {
    const LEN: usize = 8;
    fn pack_into_slice(&self, output: &mut [u8]) {
        (self as &dyn DynPack).pack_into_slice(output);
    }

    fn unpack_from_slice(input: &[u8]) -> Result<ConstantPriceCurve, ProgramError> {
        let token_b_price = array_ref![input, 0, 8];
        Ok(Self {
            token_b_price: u64::from_le_bytes(*token_b_price),
        })
    }
}

impl DynPack for ConstantPriceCurve {
    fn pack_into_slice(&self, output: &mut [u8]) {
        let token_b_price = array_mut_ref![output, 0, 8];
        *token_b_price = self.token_b_price.to_le_bytes();
    }
}
