use alloc::{string::ToString, vec::Vec};

use keystore::algorithms::secp256k1::get_public_key_by_seed;
use keystore::algorithms::zcash::calculate_seed_fingerprint;
use zcash_vendor::zcash_keys::keys::UnifiedFullViewingKey;

#[cfg(feature = "cypherpunk")]
use zcash_vendor::orchard;

#[cfg(feature = "cypherpunk")]
use keystore::algorithms::zcash::sign_message_orchard;

use crate::errors::ZcashError;

pub mod check;
pub mod parse;
pub mod sign;
pub mod structs;
