use alloc::{string::ToString, vec::Vec};

use keystore::algorithms::secp256k1::{get_public_key_by_seed};
use keystore::algorithms::zcash::{calculate_seed_fingerprint, sign_message_orchard};
use zcash_vendor::{orchard, zcash_keys::keys::UnifiedFullViewingKey};

use crate::errors::ZcashError;

pub mod check;
pub mod parse;
pub mod sign;
pub mod structs;
