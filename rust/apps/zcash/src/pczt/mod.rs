use alloc::{
    collections::btree_map::BTreeMap,
    string::{String, ToString},
    vec::Vec,
};
use bitcoin::secp256k1::Message;
use keystore::algorithms::secp256k1::{
    derive_public_key, get_public_key_by_seed, sign_message_by_seed,
};
use keystore::algorithms::zcash::{calculate_seed_fingerprint, sign_message_orchard};
use zcash_vendor::{
    orchard,
    pczt::{
        common::Zip32Derivation,
        pczt_ext::{PcztSigner, ZcashSignature},
        Pczt,
    },
    zcash_keys::keys::UnifiedFullViewingKey,
};

use crate::errors::ZcashError;

pub mod check;
pub mod parse;
pub mod sign;
