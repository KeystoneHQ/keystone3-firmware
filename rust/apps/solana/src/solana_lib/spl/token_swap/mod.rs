pub mod curve;

pub mod instruction {
    use crate::solana_lib::solana_program::errors::ProgramError;
    use crate::solana_lib::solana_program::program_pack::Pack;
    use crate::solana_lib::spl::token_swap::curve::base::SwapCurve;
    use crate::solana_lib::spl::token_swap::curve::errors::SwapError;
    use crate::solana_lib::spl::token_swap::curve::fees::Fees;

    /// Initialize instruction data
    #[repr(C)]
    #[derive(Debug)]
    pub struct Initialize {
        /// all swap fees
        pub fees: Fees,
        /// swap curve info for pool, including CurveType and anything
        /// else that may be required
        pub swap_curve: SwapCurve,
    }

    /// Swap instruction data
    #[cfg_attr(feature = "fuzz", derive(Arbitrary))]
    #[repr(C)]
    #[derive(Clone, Debug, PartialEq)]
    pub struct Swap {
        /// SOURCE amount to transfer, output to DESTINATION is based on the exchange rate
        pub amount_in: u64,
        /// Minimum amount of DESTINATION token to output, prevents excessive slippage
        pub minimum_amount_out: u64,
    }

    /// DepositAllTokenTypes instruction data
    #[cfg_attr(feature = "fuzz", derive(Arbitrary))]
    #[repr(C)]
    #[derive(Clone, Debug, PartialEq)]
    pub struct DepositAllTokenTypes {
        /// Pool token amount to transfer. token_a and token_b amount are set by
        /// the current exchange rate and size of the pool
        pub pool_token_amount: u64,
        /// Maximum token A amount to deposit, prevents excessive slippage
        pub maximum_token_a_amount: u64,
        /// Maximum token B amount to deposit, prevents excessive slippage
        pub maximum_token_b_amount: u64,
    }

    /// WithdrawAllTokenTypes instruction data
    #[cfg_attr(feature = "fuzz", derive(Arbitrary))]
    #[repr(C)]
    #[derive(Clone, Debug, PartialEq)]
    pub struct WithdrawAllTokenTypes {
        /// Amount of pool tokens to burn. User receives an output of token a
        /// and b based on the percentage of the pool tokens that are returned.
        pub pool_token_amount: u64,
        /// Minimum amount of token A to receive, prevents excessive slippage
        pub minimum_token_a_amount: u64,
        /// Minimum amount of token B to receive, prevents excessive slippage
        pub minimum_token_b_amount: u64,
    }

    /// Deposit one token type, exact amount in instruction data
    #[cfg_attr(feature = "fuzz", derive(Arbitrary))]
    #[repr(C)]
    #[derive(Clone, Debug, PartialEq)]
    pub struct DepositSingleTokenTypeExactAmountIn {
        /// Token amount to deposit
        pub source_token_amount: u64,
        /// Pool token amount to receive in exchange. The amount is set by
        /// the current exchange rate and size of the pool
        pub minimum_pool_token_amount: u64,
    }

    /// WithdrawSingleTokenTypeExactAmountOut instruction data
    #[cfg_attr(feature = "fuzz", derive(Arbitrary))]
    #[repr(C)]
    #[derive(Clone, Debug, PartialEq)]
    pub struct WithdrawSingleTokenTypeExactAmountOut {
        /// Amount of token A or B to receive
        pub destination_token_amount: u64,
        /// Maximum amount of pool tokens to burn. User receives an output of token A
        /// or B based on the percentage of the pool tokens that are returned.
        pub maximum_pool_token_amount: u64,
    }

    /// Instructions supported by the token swap program.
    #[repr(C)]
    #[derive(Debug)]
    pub enum SwapInstruction {
        ///   Initializes a new swap
        ///
        ///   0. `[writable, signer]` New Token-swap to create.
        ///   1. `[]` swap authority derived from `create_program_address(&[Token-swap account])`
        ///   2. `[]` token_a Account. Must be non zero, owned by swap authority.
        ///   3. `[]` token_b Account. Must be non zero, owned by swap authority.
        ///   4. `[writable]` Pool Token Mint. Must be empty, owned by swap authority.
        ///   5. `[]` Pool Token Account to deposit trading and withdraw fees.
        ///   Must be empty, not owned by swap authority
        ///   6. `[writable]` Pool Token Account to deposit the initial pool token
        ///   supply.  Must be empty, not owned by swap authority.
        ///   7. `[]` Token program id
        Initialize(Initialize),

        ///   Swap the tokens in the pool.
        ///
        ///   0. `[]` Token-swap
        ///   1. `[]` swap authority
        ///   2. `[]` user transfer authority
        ///   3. `[writable]` token_(A|B) SOURCE Account, amount is transferable by user transfer authority,
        ///   4. `[writable]` token_(A|B) Base Account to swap INTO.  Must be the SOURCE token.
        ///   5. `[writable]` token_(A|B) Base Account to swap FROM.  Must be the DESTINATION token.
        ///   6. `[writable]` token_(A|B) DESTINATION Account assigned to USER as the owner.
        ///   7. `[writable]` Pool token mint, to generate trading fees
        ///   8. `[writable]` Fee account, to receive trading fees
        ///   9. `[]` Token program id
        ///   10. `[optional, writable]` Host fee account to receive additional trading fees
        Swap(Swap),

        ///   Deposit both types of tokens into the pool.  The output is a "pool"
        ///   token representing ownership in the pool. Inputs are converted to
        ///   the current ratio.
        ///
        ///   0. `[]` Token-swap
        ///   1. `[]` swap authority
        ///   2. `[]` user transfer authority
        ///   3. `[writable]` token_a user transfer authority can transfer amount,
        ///   4. `[writable]` token_b user transfer authority can transfer amount,
        ///   5. `[writable]` token_a Base Account to deposit into.
        ///   6. `[writable]` token_b Base Account to deposit into.
        ///   7. `[writable]` Pool MINT account, swap authority is the owner.
        ///   8. `[writable]` Pool Account to deposit the generated tokens, user is the owner.
        ///   9. `[]` Token program id
        DepositAllTokenTypes(DepositAllTokenTypes),

        ///   Withdraw both types of tokens from the pool at the current ratio, given
        ///   pool tokens.  The pool tokens are burned in exchange for an equivalent
        ///   amount of token A and B.
        ///
        ///   0. `[]` Token-swap
        ///   1. `[]` swap authority
        ///   2. `[]` user transfer authority
        ///   3. `[writable]` Pool mint account, swap authority is the owner
        ///   4. `[writable]` SOURCE Pool account, amount is transferable by user transfer authority.
        ///   5. `[writable]` token_a Swap Account to withdraw FROM.
        ///   6. `[writable]` token_b Swap Account to withdraw FROM.
        ///   7. `[writable]` token_a user Account to credit.
        ///   8. `[writable]` token_b user Account to credit.
        ///   9. `[writable]` Fee account, to receive withdrawal fees
        ///   10. `[]` Token program id
        WithdrawAllTokenTypes(WithdrawAllTokenTypes),

        ///   Deposit one type of tokens into the pool.  The output is a "pool" token
        ///   representing ownership into the pool. Input token is converted as if
        ///   a swap and deposit all token types were performed.
        ///
        ///   0. `[]` Token-swap
        ///   1. `[]` swap authority
        ///   2. `[]` user transfer authority
        ///   3. `[writable]` token_(A|B) SOURCE Account, amount is transferable by user transfer authority,
        ///   4. `[writable]` token_a Swap Account, may deposit INTO.
        ///   5. `[writable]` token_b Swap Account, may deposit INTO.
        ///   6. `[writable]` Pool MINT account, swap authority is the owner.
        ///   7. `[writable]` Pool Account to deposit the generated tokens, user is the owner.
        ///   8. `[]` Token program id
        DepositSingleTokenTypeExactAmountIn(DepositSingleTokenTypeExactAmountIn),

        ///   Withdraw one token type from the pool at the current ratio given the
        ///   exact amount out expected.
        ///
        ///   0. `[]` Token-swap
        ///   1. `[]` swap authority
        ///   2. `[]` user transfer authority
        ///   3. `[writable]` Pool mint account, swap authority is the owner
        ///   4. `[writable]` SOURCE Pool account, amount is transferable by user transfer authority.
        ///   5. `[writable]` token_a Swap Account to potentially withdraw from.
        ///   6. `[writable]` token_b Swap Account to potentially withdraw from.
        ///   7. `[writable]` token_(A|B) User Account to credit
        ///   8. `[writable]` Fee account, to receive withdrawal fees
        ///   9. `[]` Token program id
        WithdrawSingleTokenTypeExactAmountOut(WithdrawSingleTokenTypeExactAmountOut),
    }

    impl SwapInstruction {
        /// Unpacks a byte buffer into a [SwapInstruction](enum.SwapInstruction.html).
        pub fn unpack(input: &[u8]) -> Result<Self, ProgramError> {
            let (&tag, rest) = input.split_first().ok_or(SwapError::InvalidInstruction)?;
            Ok(match tag {
                0 => {
                    if rest.len() >= Fees::LEN {
                        let (fees, rest) = rest.split_at(Fees::LEN);
                        let fees = Fees::unpack_unchecked(fees)?;
                        let swap_curve = SwapCurve::unpack_unchecked(rest)?;
                        Self::Initialize(Initialize { fees, swap_curve })
                    } else {
                        return Err(SwapError::InvalidInstruction.into());
                    }
                }
                1 => {
                    let (amount_in, rest) = Self::unpack_u64(rest)?;
                    let (minimum_amount_out, _rest) = Self::unpack_u64(rest)?;
                    Self::Swap(Swap {
                        amount_in,
                        minimum_amount_out,
                    })
                }
                2 => {
                    let (pool_token_amount, rest) = Self::unpack_u64(rest)?;
                    let (maximum_token_a_amount, rest) = Self::unpack_u64(rest)?;
                    let (maximum_token_b_amount, _rest) = Self::unpack_u64(rest)?;
                    Self::DepositAllTokenTypes(DepositAllTokenTypes {
                        pool_token_amount,
                        maximum_token_a_amount,
                        maximum_token_b_amount,
                    })
                }
                3 => {
                    let (pool_token_amount, rest) = Self::unpack_u64(rest)?;
                    let (minimum_token_a_amount, rest) = Self::unpack_u64(rest)?;
                    let (minimum_token_b_amount, _rest) = Self::unpack_u64(rest)?;
                    Self::WithdrawAllTokenTypes(WithdrawAllTokenTypes {
                        pool_token_amount,
                        minimum_token_a_amount,
                        minimum_token_b_amount,
                    })
                }
                4 => {
                    let (source_token_amount, rest) = Self::unpack_u64(rest)?;
                    let (minimum_pool_token_amount, _rest) = Self::unpack_u64(rest)?;
                    Self::DepositSingleTokenTypeExactAmountIn(DepositSingleTokenTypeExactAmountIn {
                        source_token_amount,
                        minimum_pool_token_amount,
                    })
                }
                5 => {
                    let (destination_token_amount, rest) = Self::unpack_u64(rest)?;
                    let (maximum_pool_token_amount, _rest) = Self::unpack_u64(rest)?;
                    Self::WithdrawSingleTokenTypeExactAmountOut(
                        WithdrawSingleTokenTypeExactAmountOut {
                            destination_token_amount,
                            maximum_pool_token_amount,
                        },
                    )
                }
                _ => return Err(SwapError::InvalidInstruction.into()),
            })
        }

        fn unpack_u64(input: &[u8]) -> Result<(u64, &[u8]), ProgramError> {
            if input.len() >= 8 {
                let (amount, rest) = input.split_at(8);
                let amount = amount
                    .get(..8)
                    .and_then(|slice| slice.try_into().ok())
                    .map(u64::from_le_bytes)
                    .ok_or(SwapError::InvalidInstruction)?;
                Ok((amount, rest))
            } else {
                Err(SwapError::InvalidInstruction.into())
            }
        }
    }
}
