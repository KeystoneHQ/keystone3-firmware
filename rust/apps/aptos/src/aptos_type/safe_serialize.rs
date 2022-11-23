//! Custom serializers which track recursion nesting with a thread local,
//! and otherwise delegate to the derived serializers.
//!
//! This is currently only implemented for type tags, but can be easily
//! generalized, as the the only type-tag specific thing is the allowed nesting.

use serde::{Deserialize, Deserializer, Serialize, Serializer};

pub(crate) const MAX_TYPE_TAG_NESTING: u8 = 9;

pub(crate) fn type_tag_recursive_serialize<S, T>(t: &T, s: S) -> Result<S::Ok, S::Error>
where
    S: Serializer,
    T: Serialize,
{
    let res = t.serialize(s);
    res
}

pub(crate) fn type_tag_recursive_deserialize<'de, D, T>(d: D) -> Result<T, D::Error>
where
    D: Deserializer<'de>,
    T: Deserialize<'de>,
{
    let res = T::deserialize(d);
    res
}
