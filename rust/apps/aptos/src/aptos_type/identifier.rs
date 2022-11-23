use alloc::borrow::ToOwned;
use alloc::boxed::Box;
use alloc::format;
use alloc::string::String;
use alloc::vec::Vec;

use crate::errors::AptosError;
use core::{borrow::Borrow, fmt, ops::Deref, str::FromStr};
use ref_cast::RefCast;
use serde::{Deserialize, Serialize};

#[inline]
pub const fn is_valid_identifier_char(c: char) -> bool {
    matches!(c, '_' | 'a'..='z' | 'A'..='Z' | '0'..='9')
}

const fn all_bytes_valid(b: &[u8], start_offset: usize) -> bool {
    let mut i = start_offset;

    while i < b.len() {
        if !is_valid_identifier_char(b[i] as char) {
            return false;
        }
        i += 1;
    }
    true
}

pub const fn is_valid(s: &str) -> bool {
    let b = s.as_bytes();
    match b {
        b"<SELF>" => true,
        [b'a'..=b'z', ..] | [b'A'..=b'Z', ..] => all_bytes_valid(b, 1),
        [b'_', ..] if b.len() > 1 => all_bytes_valid(b, 1),
        _ => false,
    }
}

#[cfg(any(test, feature = "fuzzing"))]
#[allow(dead_code)]
pub(crate) static ALLOWED_IDENTIFIERS: &str =
    r"(?:[a-zA-Z][a-zA-Z0-9_]*)|(?:_[a-zA-Z0-9_]+)|(?:<SELF>)";
#[cfg(any(test, feature = "fuzzing"))]
pub(crate) static ALLOWED_NO_SELF_IDENTIFIERS: &str =
    r"(?:[a-zA-Z][a-zA-Z0-9_]*)|(?:_[a-zA-Z0-9_]+)";

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, Deserialize)]
#[cfg_attr(feature = "fuzzing", derive(arbitrary::Arbitrary))]
pub struct Identifier(Box<str>);

impl Identifier {
    pub fn new(s: impl Into<Box<str>>) -> crate::errors::Result<Self> {
        let s = s.into();
        if Self::is_valid(&s) {
            Ok(Self(s))
        } else {
            Err(AptosError::ParseTxError(format!(
                "Invalid identifier '{}'",
                s
            )))
        }
    }

    pub fn is_valid(s: impl AsRef<str>) -> bool {
        is_valid(s.as_ref())
    }

    pub fn is_self(&self) -> bool {
        &*self.0 == "<SELF>"
    }

    pub fn from_utf8(vec: Vec<u8>) -> crate::errors::Result<Self> {
        let s = String::from_utf8(vec)?;
        Self::new(s)
    }

    pub fn as_ident_str(&self) -> &IdentStr {
        self
    }

    pub fn into_string(self) -> String {
        self.0.into()
    }

    pub fn into_bytes(self) -> Vec<u8> {
        self.into_string().into_bytes()
    }
}

impl FromStr for Identifier {
    type Err = crate::errors::AptosError;

    fn from_str(data: &str) -> crate::errors::Result<Self> {
        Self::new(data)
    }
}

impl From<&IdentStr> for Identifier {
    fn from(ident_str: &IdentStr) -> Self {
        ident_str.to_owned()
    }
}

impl AsRef<IdentStr> for Identifier {
    fn as_ref(&self) -> &IdentStr {
        self
    }
}

impl Deref for Identifier {
    type Target = IdentStr;

    fn deref(&self) -> &IdentStr {
        IdentStr::ref_cast(&self.0)
    }
}

impl fmt::Display for Identifier {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", &self.0)
    }
}

#[derive(Debug, Eq, Hash, Ord, PartialEq, PartialOrd, ref_cast::RefCast)] //
#[repr(transparent)]
pub struct IdentStr(str);

impl IdentStr {
    pub fn new(s: &str) -> crate::errors::Result<&IdentStr> {
        if Self::is_valid(s) {
            Ok(IdentStr::ref_cast(s))
        } else {
            Err(AptosError::ParseTxError(format!(
                "Invalid identifier '{}'",
                s
            )))
        }
    }

    pub fn is_valid(s: impl AsRef<str>) -> bool {
        is_valid(s.as_ref())
    }

    pub fn len(&self) -> usize {
        self.0.len()
    }

    pub fn is_empty(&self) -> bool {
        self.0.is_empty()
    }

    pub fn as_str(&self) -> &str {
        &self.0
    }

    pub fn as_bytes(&self) -> &[u8] {
        self.0.as_bytes()
    }
}

impl Borrow<IdentStr> for Identifier {
    fn borrow(&self) -> &IdentStr {
        self
    }
}

impl ToOwned for IdentStr {
    type Owned = Identifier;

    fn to_owned(&self) -> Identifier {
        Identifier(self.0.into())
    }
}

impl fmt::Display for IdentStr {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", &self.0)
    }
}

#[macro_export]
macro_rules! ident_str {
    ($ident:expr) => {{
        let s: &'static str = $ident;

        let is_valid = $crate::identifier::is_valid(s);
        ["String is not a valid Move identifier"][!is_valid as usize];

        #[allow(clippy::transmute_ptr_to_ptr)]
        unsafe {
            ::std::mem::transmute::<&'static str, &'static $crate::identifier::IdentStr>(s)
        }
    }};
}
