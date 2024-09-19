use alloc::format;
use alloc::str::FromStr;
use core::convert::TryFrom;
use core::fmt;
use core::mem;

// Utility hashing module copied from `solana_program::program::hash`, since we
// can't import solana_program for compile time hashing for some reason.
// https://github.com/coral-xyz/anchor/blob/2a07d841c65d6f303aa9c2b0c68a6e69c4739aab/lang/syn/src/hash.rs
use sha2::{Digest, Sha256};
use third_party::bitcoin;
use thiserror::Error;

pub const HASH_BYTES: usize = 32;
#[derive(Clone, Copy, Default, Eq, PartialEq, Ord, PartialOrd, Hash)]
#[repr(transparent)]
pub struct Hash(pub [u8; HASH_BYTES]);

#[derive(Clone, Default)]
pub struct Hasher {
    hasher: Sha256,
}

impl Hasher {
    pub fn hash(&mut self, val: &[u8]) {
        self.hasher.update(val);
    }
    pub fn hashv(&mut self, vals: &[&[u8]]) {
        for val in vals {
            self.hash(val);
        }
    }
    pub fn result(self) -> Hash {
        // At the time of this writing, the sha2 library is stuck on an old version
        // of generic_array (0.9.0). Decouple ourselves with a clone to our version.
        Hash(<[u8; HASH_BYTES]>::try_from(self.hasher.finalize().as_slice()).unwrap())
    }
}

impl AsRef<[u8]> for Hash {
    fn as_ref(&self) -> &[u8] {
        &self.0[..]
    }
}

impl fmt::Debug for Hash {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", bitcoin::base58::encode(&self.0))
    }
}

impl fmt::Display for Hash {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", bitcoin::base58::encode(&self.0))
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Error)]
pub enum ParseHashError {
    #[error("string decoded to wrong size for hash")]
    WrongSize,
    #[error("failed to decoded string to hash")]
    Invalid,
}

impl FromStr for Hash {
    type Err = ParseHashError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let bytes = bitcoin::base58::decode(s).map_err(|_| ParseHashError::Invalid)?;
        if bytes.len() != mem::size_of::<Hash>() {
            Err(ParseHashError::WrongSize)
        } else {
            Ok(Hash::new(&bytes))
        }
    }
}

impl Hash {
    pub fn new(hash_slice: &[u8]) -> Self {
        Hash(<[u8; HASH_BYTES]>::try_from(hash_slice).unwrap())
    }

    pub fn to_bytes(self) -> [u8; HASH_BYTES] {
        self.0
    }
}

/// Return a Sha256 hash for the given data.
pub fn hashv(vals: &[&[u8]]) -> Hash {
    let mut hasher = Hasher::default();
    hasher.hashv(vals);
    hasher.result()
}

/// Return a Sha256 hash for the given data.
pub fn hash(val: &[u8]) -> Hash {
    hashv(&[val])
}

// Namespace for calculating instruction sighash signatures for any instruction
// not affecting program state.
pub const SIGHASH_GLOBAL_NAMESPACE: &str = "global";

// We don't technically use sighash, because the input arguments aren't given.
// Rust doesn't have method overloading so no need to use the arguments.
// However, we do namespace methods in the preeimage so that we can use
// different traits with the same method name.
pub fn sighash(namespace: &str, name: &str) -> [u8; 8] {
    let preimage = format!("{}:{}", namespace, name);
    let mut sighash = [0u8; 8];
    sighash.copy_from_slice(&hash(preimage.as_bytes()).to_bytes()[..8]);
    sighash
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_calculate_instruction_sighash() {
        let multisig_create_sighash = sighash(SIGHASH_GLOBAL_NAMESPACE, "multisig_create");
        assert_eq!("7a4d509f54585ac5", hex::encode(multisig_create_sighash));
    }

    #[test]
    fn test_cal_squads_sighash() {
        let sig_hash = sighash(SIGHASH_GLOBAL_NAMESPACE, "multisig_create");
        assert_eq!("7a4d509f54585ac5", hex::encode(sig_hash));
        let sig_h = sighash(SIGHASH_GLOBAL_NAMESPACE, "multisig_create_v2");
        assert_eq!("32ddc75d28f58be9", hex::encode(sig_h));
        let sig_h = sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_activate");
        assert_eq!("0b225cf89a1b336a", hex::encode(sig_h));
        let sig_h = sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_create");
        assert_eq!("dc3c49e01e6c4f9f", hex::encode(sig_h));
        let sig_h = sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_approve");
        assert_eq!("9025a488bcd82af8", hex::encode(sig_h));
        let sig_h = sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_cancel");
        assert_eq!("1b2a7fed26a354cb", hex::encode(sig_h));
        let sig_h = sighash(SIGHASH_GLOBAL_NAMESPACE, "proposal_reject");
        assert_eq!("f33e869ce66af687", hex::encode(sig_h));
        let sig_h = sighash(SIGHASH_GLOBAL_NAMESPACE, "vault_transaction_create");
        assert_eq!("30fa4ea8d0e2dad3", hex::encode(sig_h));
        let sig_h = sighash(SIGHASH_GLOBAL_NAMESPACE, "vault_transaction_execute");
        assert_eq!("c208a15799a419ab", hex::encode(sig_h));
    }

    #[test]
    fn test_cal_jupiter_v6_sighash() {
        let sig_hash = sighash(SIGHASH_GLOBAL_NAMESPACE, "shared_accounts_route");
        assert_eq!("c1209b3341d69c81", hex::encode(sig_hash));

        let sig_hash = sighash(SIGHASH_GLOBAL_NAMESPACE, "route");
        assert_eq!("e517cb977ae3ad2a", hex::encode(sig_hash));

        let sig_hash = sighash(SIGHASH_GLOBAL_NAMESPACE, "exact_out_route");
        assert_eq!("d033ef977b2bed5c", hex::encode(sig_hash));

        let sig_hash = sighash(SIGHASH_GLOBAL_NAMESPACE, "shared_accounts_exact_out_route");
        assert_eq!("b0d169a89a7d453e", hex::encode(sig_hash));
    }
}
