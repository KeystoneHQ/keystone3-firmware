use alloc::boxed::Box;
use alloc::string::String;
use core::{fmt, str::FromStr};

pub mod borsh;
mod errors;
pub mod serde;

pub use errors::{ParseAccountError, ParseErrorKind};

#[derive(Eq, Ord, Hash, Clone, Debug, PartialEq, PartialOrd)]
pub struct AccountId(Box<str>);

impl AccountId {
    pub const MIN_LEN: usize = 2;

    pub const MAX_LEN: usize = 64;

    pub fn as_str(&self) -> &str {
        self
    }

    pub fn is_top_level(&self) -> bool {
        !self.is_system() && !self.contains('.')
    }

    pub fn is_sub_account_of(&self, parent: &AccountId) -> bool {
        self.strip_suffix(parent.as_str())
            .map_or(false, |s| !s.is_empty() && s.find('.') == Some(s.len() - 1))
    }

    pub fn is_implicit(&self) -> bool {
        self.len() == 64
            && self
                .as_bytes()
                .iter()
                .all(|b| matches!(b, b'a'..=b'f' | b'0'..=b'9'))
    }

    pub fn is_system(&self) -> bool {
        self.as_str() == "system"
    }

    pub fn validate(account_id: &str) -> Result<(), ParseAccountError> {
        if account_id.len() < AccountId::MIN_LEN {
            Err(ParseAccountError {
                kind: ParseErrorKind::TooShort,
                char: None,
            })
        } else if account_id.len() > AccountId::MAX_LEN {
            Err(ParseAccountError {
                kind: ParseErrorKind::TooLong,
                char: None,
            })
        } else {
            // Adapted from https://github.com/near/near-sdk-rs/blob/fd7d4f82d0dfd15f824a1cf110e552e940ea9073/near-sdk/src/environment/env.rs#L819

            // NOTE: We don't want to use Regex here, because it requires extra time to compile it.
            // The valid account ID regex is /^(([a-z\d]+[-_])*[a-z\d]+\.)*([a-z\d]+[-_])*[a-z\d]+$/
            // Instead the implementation is based on the previous character checks.

            // We can safely assume that last char was a separator.
            let mut last_char_is_separator = true;

            let mut this = None;
            for (i, c) in account_id.chars().enumerate() {
                this.replace((i, c));
                let current_char_is_separator = match c {
                    'a'..='z' | '0'..='9' => false,
                    '-' | '_' | '.' => true,
                    _ => {
                        return Err(ParseAccountError {
                            kind: ParseErrorKind::InvalidChar,
                            char: this,
                        });
                    }
                };
                if current_char_is_separator && last_char_is_separator {
                    return Err(ParseAccountError {
                        kind: ParseErrorKind::RedundantSeparator,
                        char: this,
                    });
                }
                last_char_is_separator = current_char_is_separator;
            }

            if last_char_is_separator {
                return Err(ParseAccountError {
                    kind: ParseErrorKind::RedundantSeparator,
                    char: this,
                });
            }
            Ok(())
        }
    }

    #[doc(hidden)]
    #[cfg(feature = "internal_unstable")]
    #[deprecated(
        since = "#4440",
        note = "AccountId construction without validation is illegal"
    )]
    pub fn new_unvalidated(account_id: String) -> Self {
        Self(account_id.into_boxed_str())
    }
}

impl core::ops::Deref for AccountId {
    type Target = str;

    fn deref(&self) -> &Self::Target {
        self.0.as_ref()
    }
}

impl AsRef<str> for AccountId {
    fn as_ref(&self) -> &str {
        self
    }
}

impl core::borrow::Borrow<str> for AccountId {
    fn borrow(&self) -> &str {
        self
    }
}

impl FromStr for AccountId {
    type Err = ParseAccountError;

    fn from_str(account_id: &str) -> Result<Self, Self::Err> {
        Self::validate(account_id)?;
        Ok(Self(account_id.into()))
    }
}

impl TryFrom<Box<str>> for AccountId {
    type Error = ParseAccountError;

    fn try_from(account_id: Box<str>) -> Result<Self, Self::Error> {
        Self::validate(&account_id)?;
        Ok(Self(account_id))
    }
}

impl TryFrom<String> for AccountId {
    type Error = ParseAccountError;

    fn try_from(account_id: String) -> Result<Self, Self::Error> {
        Self::validate(&account_id)?;
        Ok(Self(account_id.into_boxed_str()))
    }
}

impl fmt::Display for AccountId {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        fmt::Display::fmt(&self.0, f)
    }
}

impl From<AccountId> for String {
    fn from(account_id: AccountId) -> Self {
        account_id.0.into_string()
    }
}

impl From<AccountId> for Box<str> {
    fn from(value: AccountId) -> Box<str> {
        value.0
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    pub const OK_ACCOUNT_IDS: [&str; 24] = [
        "aa",
        "a-a",
        "a-aa",
        "100",
        "0o",
        "com",
        "near",
        "bowen",
        "b-o_w_e-n",
        "b.owen",
        "bro.wen",
        "a.ha",
        "a.b-a.ra",
        "system",
        "over.9000",
        "google.com",
        "illia.cheapaccounts.near",
        "0o0ooo00oo00o",
        "alex-skidanov",
        "10-4.8-2",
        "b-o_w_e-n",
        "no_lols",
        "0123456789012345678901234567890123456789012345678901234567890123",
        // Valid, but can't be created
        "near.a",
    ];

    pub const BAD_ACCOUNT_IDS: [&str; 24] = [
        "a",
        "A",
        "Abc",
        "-near",
        "near-",
        "-near-",
        "near.",
        ".near",
        "near@",
        "@near",
        "неар",
        "@@@@@",
        "0__0",
        "0_-_0",
        "0_-_0",
        "..",
        "a..near",
        "nEar",
        "_bowen",
        "hello world",
        "abcdefghijklmnopqrstuvwxyz.abcdefghijklmnopqrstuvwxyz.abcdefghijklmnopqrstuvwxyz",
        "01234567890123456789012345678901234567890123456789012345678901234",
        // `@` separators are banned now
        "some-complex-address@gmail.com",
        "sub.buy_d1gitz@atata@b0-rg.c_0_m",
    ];

    #[test]
    fn test_is_valid_account_id() {
        for account_id in OK_ACCOUNT_IDS.iter().cloned() {
            if let Err(err) = AccountId::validate(account_id) {
                panic!(
                    "Valid account id {:?} marked invalid: {}",
                    account_id,
                    err.kind()
                );
            }
        }

        for account_id in BAD_ACCOUNT_IDS.iter().cloned() {
            if let Ok(_) = AccountId::validate(account_id) {
                panic!("Invalid account id {:?} marked valid", account_id);
            }
        }
    }

    #[test]
    fn test_err_kind_classification() {
        let id = "ErinMoriarty.near".parse::<AccountId>();
        debug_assert!(
            matches!(
                id,
                Err(ParseAccountError {
                    kind: ParseErrorKind::InvalidChar,
                    char: Some((0, 'E'))
                })
            ),
            "{:?}",
            id
        );

        let id = "-KarlUrban.near".parse::<AccountId>();
        debug_assert!(
            matches!(
                id,
                Err(ParseAccountError {
                    kind: ParseErrorKind::RedundantSeparator,
                    char: Some((0, '-'))
                })
            ),
            "{:?}",
            id
        );

        let id = "anthonystarr.".parse::<AccountId>();
        debug_assert!(
            matches!(
                id,
                Err(ParseAccountError {
                    kind: ParseErrorKind::RedundantSeparator,
                    char: Some((12, '.'))
                })
            ),
            "{:?}",
            id
        );

        let id = "jack__Quaid.near".parse::<AccountId>();
        debug_assert!(
            matches!(
                id,
                Err(ParseAccountError {
                    kind: ParseErrorKind::RedundantSeparator,
                    char: Some((5, '_'))
                })
            ),
            "{:?}",
            id
        );
    }

    #[test]
    fn test_is_valid_top_level_account_id() {
        let ok_top_level_account_ids = &[
            "aa",
            "a-a",
            "a-aa",
            "100",
            "0o",
            "com",
            "near",
            "bowen",
            "b-o_w_e-n",
            "0o0ooo00oo00o",
            "alex-skidanov",
            "b-o_w_e-n",
            "no_lols",
            "0123456789012345678901234567890123456789012345678901234567890123",
        ];
        for account_id in ok_top_level_account_ids {
            assert!(
                account_id
                    .parse::<AccountId>()
                    .map_or(false, |account_id| account_id.is_top_level()),
                "Valid top level account id {:?} marked invalid",
                account_id
            );
        }

        let bad_top_level_account_ids = &[
            "ƒelicia.near", // fancy ƒ!
            "near.a",
            "b.owen",
            "bro.wen",
            "a.ha",
            "a.b-a.ra",
            "some-complex-address@gmail.com",
            "sub.buy_d1gitz@atata@b0-rg.c_0_m",
            "over.9000",
            "google.com",
            "illia.cheapaccounts.near",
            "10-4.8-2",
            "a",
            "A",
            "Abc",
            "-near",
            "near-",
            "-near-",
            "near.",
            ".near",
            "near@",
            "@near",
            "неар",
            "@@@@@",
            "0__0",
            "0_-_0",
            "0_-_0",
            "..",
            "a..near",
            "nEar",
            "_bowen",
            "hello world",
            "abcdefghijklmnopqrstuvwxyz.abcdefghijklmnopqrstuvwxyz.abcdefghijklmnopqrstuvwxyz",
            "01234567890123456789012345678901234567890123456789012345678901234",
            // Valid regex and length, but reserved
            "system",
        ];
        for account_id in bad_top_level_account_ids {
            assert!(
                !account_id
                    .parse::<AccountId>()
                    .map_or(false, |account_id| account_id.is_top_level()),
                "Invalid top level account id {:?} marked valid",
                account_id
            );
        }
    }

    #[test]
    fn test_is_valid_sub_account_id() {
        let ok_pairs = &[
            ("test", "a.test"),
            ("test-me", "abc.test-me"),
            ("gmail.com", "abc.gmail.com"),
            ("gmail.com", "abc-lol.gmail.com"),
            ("gmail.com", "abc_lol.gmail.com"),
            ("gmail.com", "bro-abc_lol.gmail.com"),
            ("g0", "0g.g0"),
            ("1g", "1g.1g"),
            ("5-3", "4_2.5-3"),
        ];
        for (signer_id, sub_account_id) in ok_pairs {
            assert!(
                matches!(
                    (signer_id.parse::<AccountId>(), sub_account_id.parse::<AccountId>()),
                    (Ok(signer_id), Ok(sub_account_id)) if sub_account_id.is_sub_account_of(&signer_id)
                ),
                "Failed to create sub-account {:?} by account {:?}",
                sub_account_id,
                signer_id
            );
        }

        let bad_pairs = &[
            ("test", ".test"),
            ("test", "test"),
            ("test", "a1.a.test"),
            ("test", "est"),
            ("test", ""),
            ("test", "st"),
            ("test5", "ббб"),
            ("test", "a-test"),
            ("test", "etest"),
            ("test", "a.etest"),
            ("test", "retest"),
            ("test-me", "abc-.test-me"),
            ("test-me", "Abc.test-me"),
            ("test-me", "-abc.test-me"),
            ("test-me", "a--c.test-me"),
            ("test-me", "a_-c.test-me"),
            ("test-me", "a-_c.test-me"),
            ("test-me", "_abc.test-me"),
            ("test-me", "abc_.test-me"),
            ("test-me", "..test-me"),
            ("test-me", "a..test-me"),
            ("gmail.com", "a.abc@gmail.com"),
            ("gmail.com", ".abc@gmail.com"),
            ("gmail.com", ".abc@gmail@com"),
            ("gmail.com", "abc@gmail@com"),
            ("test", "a@test"),
            ("test_me", "abc@test_me"),
            ("gmail.com", "abc@gmail.com"),
            ("gmail@com", "abc.gmail@com"),
            ("gmail.com", "abc-lol@gmail.com"),
            ("gmail@com", "abc_lol.gmail@com"),
            ("gmail@com", "bro-abc_lol.gmail@com"),
            (
                "gmail.com",
                "123456789012345678901234567890123456789012345678901234567890@gmail.com",
            ),
            (
                "123456789012345678901234567890123456789012345678901234567890",
                "1234567890.123456789012345678901234567890123456789012345678901234567890",
            ),
            ("aa", "ъ@aa"),
            ("aa", "ъ.aa"),
        ];
        for (signer_id, sub_account_id) in bad_pairs {
            assert!(
                !matches!(
                    (signer_id.parse::<AccountId>(), sub_account_id.parse::<AccountId>()),
                    (Ok(signer_id), Ok(sub_account_id)) if sub_account_id.is_sub_account_of(&signer_id)
                ),
                "Invalid sub-account {:?} created by account {:?}",
                sub_account_id,
                signer_id
            );
        }
    }

    #[test]
    fn test_is_account_id_64_len_hex() {
        let valid_64_len_hex_account_ids = &[
            "0000000000000000000000000000000000000000000000000000000000000000",
            "6174617461746174617461746174617461746174617461746174617461746174",
            "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
            "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
            "20782e20662e64666420482123494b6b6c677573646b6c66676a646b6c736667",
        ];
        for valid_account_id in valid_64_len_hex_account_ids {
            assert!(
                matches!(
                    valid_account_id.parse::<AccountId>(),
                    Ok(account_id) if account_id.is_implicit()
                ),
                "Account ID {} should be valid 64-len hex",
                valid_account_id
            );
        }

        let invalid_64_len_hex_account_ids = &[
            "000000000000000000000000000000000000000000000000000000000000000",
            "6.74617461746174617461746174617461746174617461746174617461746174",
            "012-456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
            "fffff_ffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
            "oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo",
            "00000000000000000000000000000000000000000000000000000000000000",
        ];
        for invalid_account_id in invalid_64_len_hex_account_ids {
            assert!(
                !matches!(
                    invalid_account_id.parse::<AccountId>(),
                    Ok(account_id) if account_id.is_implicit()
                ),
                "Account ID {} is not an implicit account",
                invalid_account_id
            );
        }
    }
}
