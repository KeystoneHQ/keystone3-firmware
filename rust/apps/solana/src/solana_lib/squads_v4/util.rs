use crate::solana_lib::squads_v4::hash::hash;
use alloc::format;
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
    fn test_cal_sighash() {
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
}
