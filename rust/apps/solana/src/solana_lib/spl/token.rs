pub mod instruction {
    use crate::solana_lib::solana_program::errors::ProgramError;
    use crate::solana_lib::solana_program::program_option::COption;
    use crate::solana_lib::solana_program::pubkey::Pubkey;
    use crate::solana_lib::spl::errors::TokenError;

    /// Specifies the authority type for SetAuthority instructions
    #[repr(u8)]
    #[derive(Clone, Debug, PartialEq)]
    pub enum AuthorityType {
        /// Authority to mint new tokens
        MintTokens,
        /// Authority to freeze any account associated with the Mint
        FreezeAccount,
        /// Owner of a given token account
        AccountOwner,
        /// Authority to close a token account
        CloseAccount,
    }

    impl AuthorityType {
        fn into(&self) -> u8 {
            match self {
                AuthorityType::MintTokens => 0,
                AuthorityType::FreezeAccount => 1,
                AuthorityType::AccountOwner => 2,
                AuthorityType::CloseAccount => 3,
            }
        }

        fn from(index: u8) -> Result<Self, ProgramError> {
            match index {
                0 => Ok(AuthorityType::MintTokens),
                1 => Ok(AuthorityType::FreezeAccount),
                2 => Ok(AuthorityType::AccountOwner),
                3 => Ok(AuthorityType::CloseAccount),
                _ => Err(TokenError::InvalidInstruction.into()),
            }
        }
    }

    /// Instructions supported by the token program.
    #[repr(C)]
    #[derive(Clone, Debug, PartialEq)]
    pub enum TokenInstruction {
        /// Initializes a new mint and optionally deposits all the newly minted
        /// tokens in an account.
        ///
        /// The `InitializeMint` instruction requires no signers and MUST be
        /// included within the same Transaction as the system program's
        /// `CreateAccount` instruction that creates the account being initialized.
        /// Otherwise another party can acquire ownership of the uninitialized
        /// account.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   0. `[writable]` The mint to initialize.
        ///   1. `[]` Rent sysvar
        ///
        InitializeMint {
            /// Number of base 10 digits to the right of the decimal place.
            decimals: u8,
            /// The authority/multisignature to mint tokens.
            mint_authority: Pubkey,
            /// The freeze authority/multisignature of the mint.
            freeze_authority: COption<Pubkey>,
        },
        /// Initializes a new account to hold tokens.  If this account is associated
        /// with the native mint then the token balance of the initialized account
        /// will be equal to the amount of SOL in the account. If this account is
        /// associated with another mint, that mint must be initialized before this
        /// command can succeed.
        ///
        /// The `InitializeAccount` instruction requires no signers and MUST be
        /// included within the same Transaction as the system program's
        /// `CreateAccount` instruction that creates the account being initialized.
        /// Otherwise another party can acquire ownership of the uninitialized
        /// account.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   0. `[writable]`  The account to initialize.
        ///   1. `[]` The mint this account will be associated with.
        ///   2. `[]` The new account's owner/multisignature.
        ///   3. `[]` Rent sysvar
        InitializeAccount,
        /// Initializes a multisignature account with N provided signers.
        ///
        /// Multisignature accounts can used in place of any single owner/delegate
        /// accounts in any token instruction that require an owner/delegate to be
        /// present.  The variant field represents the number of signers (M)
        /// required to validate this multisignature account.
        ///
        /// The `InitializeMultisig` instruction requires no signers and MUST be
        /// included within the same Transaction as the system program's
        /// `CreateAccount` instruction that creates the account being initialized.
        /// Otherwise another party can acquire ownership of the uninitialized
        /// account.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   0. `[writable]` The multisignature account to initialize.
        ///   1. `[]` Rent sysvar
        ///   2. ..2+N. `[]` The signer accounts, must equal to N where 1 <= N <=
        ///      11.
        InitializeMultisig {
            /// The number of signers (M) required to validate this multisignature
            /// account.
            m: u8,
        },
        /// Transfers tokens from one account to another either directly or via a
        /// delegate.  If this account is associated with the native mint then equal
        /// amounts of SOL and Tokens will be transferred to the destination
        /// account.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   * Single owner/delegate
        ///   0. `[writable]` The source account.
        ///   1. `[writable]` The destination account.
        ///   2. `[signer]` The source account's owner/delegate.
        ///
        ///   * Multisignature owner/delegate
        ///   0. `[writable]` The source account.
        ///   1. `[writable]` The destination account.
        ///   2. `[]` The source account's multisignature owner/delegate.
        ///   3. ..3+M `[signer]` M signer accounts.
        Transfer {
            /// The amount of tokens to transfer.
            amount: u64,
        },
        /// Approves a delegate.  A delegate is given the authority over tokens on
        /// behalf of the source account's owner.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   * Single owner
        ///   0. `[writable]` The source account.
        ///   1. `[]` The delegate.
        ///   2. `[signer]` The source account owner.
        ///
        ///   * Multisignature owner
        ///   0. `[writable]` The source account.
        ///   1. `[]` The delegate.
        ///   2. `[]` The source account's multisignature owner.
        ///   3. ..3+M `[signer]` M signer accounts
        Approve {
            /// The amount of tokens the delegate is approved for.
            amount: u64,
        },
        /// Revokes the delegate's authority.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   * Single owner
        ///   0. `[writable]` The source account.
        ///   1. `[signer]` The source account owner.
        ///
        ///   * Multisignature owner
        ///   0. `[writable]` The source account.
        ///   1. `[]` The source account's multisignature owner.
        ///   2. ..2+M `[signer]` M signer accounts
        Revoke,
        /// Sets a new authority of a mint or account.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   * Single authority
        ///   0. `[writable]` The mint or account to change the authority of.
        ///   1. `[signer]` The current authority of the mint or account.
        ///
        ///   * Multisignature authority
        ///   0. `[writable]` The mint or account to change the authority of.
        ///   1. `[]` The mint's or account's current multisignature authority.
        ///   2. ..2+M `[signer]` M signer accounts
        SetAuthority {
            /// The type of authority to update.
            authority_type: AuthorityType,
            /// The new authority
            new_authority: COption<Pubkey>,
        },
        /// Mints new tokens to an account.  The native mint does not support
        /// minting.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   * Single authority
        ///   0. `[writable]` The mint.
        ///   1. `[writable]` The account to mint tokens to.
        ///   2. `[signer]` The mint's minting authority.
        ///
        ///   * Multisignature authority
        ///   0. `[writable]` The mint.
        ///   1. `[writable]` The account to mint tokens to.
        ///   2. `[]` The mint's multisignature mint-tokens authority.
        ///   3. ..3+M `[signer]` M signer accounts.
        MintTo {
            /// The amount of new tokens to mint.
            amount: u64,
        },
        /// Burns tokens by removing them from an account.  `Burn` does not support
        /// accounts associated with the native mint, use `CloseAccount` instead.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   * Single owner/delegate
        ///   0. `[writable]` The account to burn from.
        ///   1. `[writable]` The token mint.
        ///   2. `[signer]` The account's owner/delegate.
        ///
        ///   * Multisignature owner/delegate
        ///   0. `[writable]` The account to burn from.
        ///   1. `[writable]` The token mint.
        ///   2. `[]` The account's multisignature owner/delegate.
        ///   3. ..3+M `[signer]` M signer accounts.
        Burn {
            /// The amount of tokens to burn.
            amount: u64,
        },
        /// Close an account by transferring all its SOL to the destination account.
        /// Non-native accounts may only be closed if its token amount is zero.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   * Single owner
        ///   0. `[writable]` The account to close.
        ///   1. `[writable]` The destination account.
        ///   2. `[signer]` The account's owner.
        ///
        ///   * Multisignature owner
        ///   0. `[writable]` The account to close.
        ///   1. `[writable]` The destination account.
        ///   2. `[]` The account's multisignature owner.
        ///   3. ..3+M `[signer]` M signer accounts.
        CloseAccount,
        /// Freeze an Initialized account using the Mint's freeze_authority (if
        /// set).
        ///
        /// Accounts expected by this instruction:
        ///
        ///   * Single owner
        ///   0. `[writable]` The account to freeze.
        ///   1. `[]` The token mint.
        ///   2. `[signer]` The mint freeze authority.
        ///
        ///   * Multisignature owner
        ///   0. `[writable]` The account to freeze.
        ///   1. `[]` The token mint.
        ///   2. `[]` The mint's multisignature freeze authority.
        ///   3. ..3+M `[signer]` M signer accounts.
        FreezeAccount,
        /// Thaw a Frozen account using the Mint's freeze_authority (if set).
        ///
        /// Accounts expected by this instruction:
        ///
        ///   * Single owner
        ///   0. `[writable]` The account to freeze.
        ///   1. `[]` The token mint.
        ///   2. `[signer]` The mint freeze authority.
        ///
        ///   * Multisignature owner
        ///   0. `[writable]` The account to freeze.
        ///   1. `[]` The token mint.
        ///   2. `[]` The mint's multisignature freeze authority.
        ///   3. ..3+M `[signer]` M signer accounts.
        ThawAccount,

        /// Transfers tokens from one account to another either directly or via a
        /// delegate.  If this account is associated with the native mint then equal
        /// amounts of SOL and Tokens will be transferred to the destination
        /// account.
        ///
        /// This instruction differs from Transfer in that the token mint and
        /// decimals value is checked by the caller.  This may be useful when
        /// creating transactions offline or within a hardware wallet.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   * Single owner/delegate
        ///   0. `[writable]` The source account.
        ///   1. `[]` The token mint.
        ///   2. `[writable]` The destination account.
        ///   3. `[signer]` The source account's owner/delegate.
        ///
        ///   * Multisignature owner/delegate
        ///   0. `[writable]` The source account.
        ///   1. `[]` The token mint.
        ///   2. `[writable]` The destination account.
        ///   3. `[]` The source account's multisignature owner/delegate.
        ///   4. ..4+M `[signer]` M signer accounts.
        TransferChecked {
            /// The amount of tokens to transfer.
            amount: u64,
            /// Expected number of base 10 digits to the right of the decimal place.
            decimals: u8,
        },
        /// Approves a delegate.  A delegate is given the authority over tokens on
        /// behalf of the source account's owner.
        ///
        /// This instruction differs from Approve in that the token mint and
        /// decimals value is checked by the caller.  This may be useful when
        /// creating transactions offline or within a hardware wallet.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   * Single owner
        ///   0. `[writable]` The source account.
        ///   1. `[]` The token mint.
        ///   2. `[]` The delegate.
        ///   3. `[signer]` The source account owner.
        ///
        ///   * Multisignature owner
        ///   0. `[writable]` The source account.
        ///   1. `[]` The token mint.
        ///   2. `[]` The delegate.
        ///   3. `[]` The source account's multisignature owner.
        ///   4. ..4+M `[signer]` M signer accounts
        ApproveChecked {
            /// The amount of tokens the delegate is approved for.
            amount: u64,
            /// Expected number of base 10 digits to the right of the decimal place.
            decimals: u8,
        },
        /// Mints new tokens to an account.  The native mint does not support
        /// minting.
        ///
        /// This instruction differs from MintTo in that the decimals value is
        /// checked by the caller.  This may be useful when creating transactions
        /// offline or within a hardware wallet.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   * Single authority
        ///   0. `[writable]` The mint.
        ///   1. `[writable]` The account to mint tokens to.
        ///   2. `[signer]` The mint's minting authority.
        ///
        ///   * Multisignature authority
        ///   0. `[writable]` The mint.
        ///   1. `[writable]` The account to mint tokens to.
        ///   2. `[]` The mint's multisignature mint-tokens authority.
        ///   3. ..3+M `[signer]` M signer accounts.
        MintToChecked {
            /// The amount of new tokens to mint.
            amount: u64,
            /// Expected number of base 10 digits to the right of the decimal place.
            decimals: u8,
        },
        /// Burns tokens by removing them from an account.  `BurnChecked` does not
        /// support accounts associated with the native mint, use `CloseAccount`
        /// instead.
        ///
        /// This instruction differs from Burn in that the decimals value is checked
        /// by the caller. This may be useful when creating transactions offline or
        /// within a hardware wallet.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   * Single owner/delegate
        ///   0. `[writable]` The account to burn from.
        ///   1. `[writable]` The token mint.
        ///   2. `[signer]` The account's owner/delegate.
        ///
        ///   * Multisignature owner/delegate
        ///   0. `[writable]` The account to burn from.
        ///   1. `[writable]` The token mint.
        ///   2. `[]` The account's multisignature owner/delegate.
        ///   3. ..3+M `[signer]` M signer accounts.
        BurnChecked {
            /// The amount of tokens to burn.
            amount: u64,
            /// Expected number of base 10 digits to the right of the decimal place.
            decimals: u8,
        },
        /// Like InitializeAccount, but the owner pubkey is passed via instruction data
        /// rather than the accounts list. This variant may be preferable when using
        /// Cross Program Invocation from an instruction that does not need the owner's
        /// `AccountInfo` otherwise.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   0. `[writable]`  The account to initialize.
        ///   1. `[]` The mint this account will be associated with.
        ///   3. `[]` Rent sysvar
        InitializeAccount2 {
            /// The new account's owner/multisignature.
            owner: Pubkey,
        },
        /// Given a wrapped / native token account (a token account containing SOL)
        /// updates its amount field based on the account's underlying `lamports`.
        /// This is useful if a non-wrapped SOL account uses `system_instruction::transfer`
        /// to move lamports to a wrapped token account, and needs to have its token
        /// `amount` field updated.
        ///
        /// Accounts expected by this instruction:
        ///
        ///   0. `[writable]`  The native token account to sync with its underlying lamports.
        SyncNative,
        /// Like InitializeAccount2, but does not require the Rent sysvar to be provided
        ///
        /// Accounts expected by this instruction:
        ///
        ///   0. `[writable]`  The account to initialize.
        ///   1. `[]` The mint this account will be associated with.
        InitializeAccount3 {
            /// The new account's owner/multisignature.
            owner: Pubkey,
        },
        /// Like InitializeMultisig, but does not require the Rent sysvar to be provided
        ///
        /// Accounts expected by this instruction:
        ///
        ///   0. `[writable]` The multisignature account to initialize.
        ///   1. ..1+N. `[]` The signer accounts, must equal to N where 1 <= N <=
        ///      11.
        InitializeMultisig2 {
            /// The number of signers (M) required to validate this multisignature
            /// account.
            m: u8,
        },
        /// Like InitializeMint, but does not require the Rent sysvar to be provided
        ///
        /// Accounts expected by this instruction:
        ///
        ///   0. `[writable]` The mint to initialize.
        ///
        InitializeMint2 {
            /// Number of base 10 digits to the right of the decimal place.
            decimals: u8,
            /// The authority/multisignature to mint tokens.
            mint_authority: Pubkey,
            /// The freeze authority/multisignature of the mint.
            freeze_authority: COption<Pubkey>,
        },
    }

    impl TokenInstruction {
        /// Unpacks a byte buffer into a [TokenInstruction](enum.TokenInstruction.html).
        pub fn unpack(input: &[u8]) -> Result<Self, ProgramError> {
            use TokenError::InvalidInstruction;

            let (&tag, rest) = input.split_first().ok_or(InvalidInstruction)?;
            Ok(match tag {
                0 => {
                    let (&decimals, rest) = rest.split_first().ok_or(InvalidInstruction)?;
                    let (mint_authority, rest) = Self::unpack_pubkey(rest)?;
                    let (freeze_authority, _rest) = Self::unpack_pubkey_option(rest)?;
                    Self::InitializeMint {
                        mint_authority,
                        freeze_authority,
                        decimals,
                    }
                }
                1 => Self::InitializeAccount,
                2 => {
                    let &m = rest.get(0).ok_or(InvalidInstruction)?;
                    Self::InitializeMultisig { m }
                }
                3 | 4 | 7 | 8 => {
                    let amount = rest
                        .get(..8)
                        .and_then(|slice| slice.try_into().ok())
                        .map(u64::from_le_bytes)
                        .ok_or(InvalidInstruction)?;
                    match tag {
                        3 => Self::Transfer { amount },
                        4 => Self::Approve { amount },
                        7 => Self::MintTo { amount },
                        8 => Self::Burn { amount },
                        _ => unreachable!(),
                    }
                }
                5 => Self::Revoke,
                6 => {
                    let (authority_type, rest) = rest
                        .split_first()
                        .ok_or_else(|| ProgramError::from(InvalidInstruction))
                        .and_then(|(&t, rest)| Ok((AuthorityType::from(t)?, rest)))?;
                    let (new_authority, _rest) = Self::unpack_pubkey_option(rest)?;

                    Self::SetAuthority {
                        authority_type,
                        new_authority,
                    }
                }
                9 => Self::CloseAccount,
                10 => Self::FreezeAccount,
                11 => Self::ThawAccount,
                12 => {
                    let (amount, rest) = rest.split_at(8);
                    let amount = amount
                        .try_into()
                        .ok()
                        .map(u64::from_le_bytes)
                        .ok_or(InvalidInstruction)?;
                    let (&decimals, _rest) = rest.split_first().ok_or(InvalidInstruction)?;

                    Self::TransferChecked { amount, decimals }
                }
                13 => {
                    let (amount, rest) = rest.split_at(8);
                    let amount = amount
                        .try_into()
                        .ok()
                        .map(u64::from_le_bytes)
                        .ok_or(InvalidInstruction)?;
                    let (&decimals, _rest) = rest.split_first().ok_or(InvalidInstruction)?;

                    Self::ApproveChecked { amount, decimals }
                }
                14 => {
                    let (amount, rest) = rest.split_at(8);
                    let amount = amount
                        .try_into()
                        .ok()
                        .map(u64::from_le_bytes)
                        .ok_or(InvalidInstruction)?;
                    let (&decimals, _rest) = rest.split_first().ok_or(InvalidInstruction)?;

                    Self::MintToChecked { amount, decimals }
                }
                15 => {
                    let (amount, rest) = rest.split_at(8);
                    let amount = amount
                        .try_into()
                        .ok()
                        .map(u64::from_le_bytes)
                        .ok_or(InvalidInstruction)?;
                    let (&decimals, _rest) = rest.split_first().ok_or(InvalidInstruction)?;

                    Self::BurnChecked { amount, decimals }
                }
                16 => {
                    let (owner, _rest) = Self::unpack_pubkey(rest)?;
                    Self::InitializeAccount2 { owner }
                }
                17 => Self::SyncNative,
                18 => {
                    let (owner, _rest) = Self::unpack_pubkey(rest)?;
                    Self::InitializeAccount3 { owner }
                }
                19 => {
                    let &m = rest.get(0).ok_or(InvalidInstruction)?;
                    Self::InitializeMultisig2 { m }
                }
                20 => {
                    let (&decimals, rest) = rest.split_first().ok_or(InvalidInstruction)?;
                    let (mint_authority, rest) = Self::unpack_pubkey(rest)?;
                    let (freeze_authority, _rest) = Self::unpack_pubkey_option(rest)?;
                    Self::InitializeMint2 {
                        mint_authority,
                        freeze_authority,
                        decimals,
                    }
                }
                _ => return Err(TokenError::InvalidInstruction.into()),
            })
        }

        fn unpack_pubkey(input: &[u8]) -> Result<(Pubkey, &[u8]), ProgramError> {
            if input.len() >= 32 {
                let (key, rest) = input.split_at(32);
                let pk = Pubkey::new(key);
                Ok((pk, rest))
            } else {
                Err(TokenError::InvalidInstruction.into())
            }
        }

        fn unpack_pubkey_option(input: &[u8]) -> Result<(COption<Pubkey>, &[u8]), ProgramError> {
            match input.split_first() {
                Option::Some((&0, rest)) => Ok((COption::None, rest)),
                Option::Some((&1, rest)) if rest.len() >= 32 => {
                    let (key, rest) = rest.split_at(32);
                    let pk = Pubkey::new(key);
                    Ok((COption::Some(pk), rest))
                }
                _ => Err(TokenError::InvalidInstruction.into()),
            }
        }
    }
}
