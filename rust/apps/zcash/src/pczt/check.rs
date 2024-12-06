use super::*;

use orchard::{
    commitment::ExtractedNoteCommitment,
    keys::{FullViewingKey, Scope},
    note::{Memo, Note, Nullifier, Rho},
    note_ext::{calculate_note_commitment, calculate_nullifier},
    Address,
};
use parse::decode_output_enc_ciphertext;
use zcash_vendor::{
    bip32::ChildNumber, pasta_curves::group::GroupEncoding, pczt,
    zcash_primitives::legacy::keys::AccountPubKey,
};

pub fn check_pczt(
    seed_fingerprint: &[u8; 32],
    ufvk: &UnifiedFullViewingKey,
    pczt: &Pczt,
) -> Result<(), ZcashError> {
    let xpub = ufvk.transparent().ok_or(ZcashError::InvalidDataError(
        "transparent xpub is not present".to_string(),
    ))?;
    let orchard = ufvk.orchard().ok_or(ZcashError::InvalidDataError(
        "orchard fvk is not present".to_string(),
    ))?;
    check_orchard(&seed_fingerprint, &orchard, &pczt.orchard())?;
    check_transparent(&seed_fingerprint, &xpub, &pczt.transparent())?;
    Ok(())
}

fn check_transparent(
    seed_fingerprint: &[u8; 32],
    xpub: &AccountPubKey,
    bundle: &pczt::transparent::Bundle,
) -> Result<(), ZcashError> {
    bundle.inputs().iter().try_for_each(|input| {
        check_transparent_input(seed_fingerprint, xpub, input)?;
        Ok(())
    })?;
    bundle.outputs().iter().try_for_each(|output| {
        check_transparent_output(seed_fingerprint, xpub, output)?;
        Ok(())
    })?;
    Ok(())
}

fn check_transparent_input(
    seed_fingerprint: &[u8; 32],
    xpub: &AccountPubKey,
    input: &pczt::transparent::Input,
) -> Result<(), ZcashError> {
    let pubkey: [u8; 33] = input.script_pubkey().clone().try_into().unwrap();
    match input.bip32_derivation().get(&pubkey) {
        Some(bip32_derivation) => {
            if seed_fingerprint == &bip32_derivation.seed_fingerprint {
                //verify public key
                let xpub = xpub.to_inner();
                let target = bip32_derivation
                    .derivation_path
                    .iter()
                    .try_fold(xpub, |acc, n| {
                        acc.derive_child(ChildNumber::from(*n)).map_err(|_| {
                            ZcashError::InvalidPczt(
                                "transparent input bip32 derivation path invalid".to_string(),
                            )
                        })
                    })?;
                if target.public_key().serialize().to_vec() != input.script_pubkey().clone() {
                    return Err(ZcashError::InvalidPczt(
                        "transparent input script pubkey mismatch".to_string(),
                    ));
                }
                Ok(())
            } else {
                //not my input, pass
                Ok(())
            }
        }
        //not my input, pass
        None => Ok(()),
    }
}

fn check_transparent_output(
    seed_fingerprint: &[u8; 32],
    xpub: &AccountPubKey,
    output: &pczt::transparent::Output,
) -> Result<(), ZcashError> {
    let pubkey: [u8; 33] = output.script_pubkey().clone().try_into().unwrap();
    match output.bip32_derivation().get(&pubkey) {
        Some(bip32_derivation) => {
            if seed_fingerprint == &bip32_derivation.seed_fingerprint {
                //verify public key
                let xpub = xpub.to_inner();
                let target = bip32_derivation
                    .derivation_path
                    .iter()
                    .try_fold(xpub, |acc, n| {
                        acc.derive_child(ChildNumber::from(*n)).map_err(|_| {
                            ZcashError::InvalidPczt(
                                "transparent output bip32 derivation path invalid".to_string(),
                            )
                        })
                    })?;
                if target.public_key().serialize().to_vec() != output.script_pubkey().clone() {
                    return Err(ZcashError::InvalidPczt(
                        "transparent output script pubkey mismatch".to_string(),
                    ));
                }
                Ok(())
            } else {
                //not my output, pass
                Ok(())
            }
        }
        //not my output, pass
        None => Ok(()),
    }
}

fn check_orchard(
    seed_fingerprint: &[u8; 32],
    fvk: &FullViewingKey,
    bundle: &pczt::orchard::Bundle,
) -> Result<(), ZcashError> {
    bundle.actions().iter().try_for_each(|action| {
        check_action(seed_fingerprint, fvk, action)?;
        Ok(())
    })?;
    Ok(())
}

fn check_action(
    seed_fingerprint: &[u8; 32],
    fvk: &FullViewingKey,
    action: &pczt::orchard::Action,
) -> Result<(), ZcashError> {
    check_action_spend(seed_fingerprint, fvk, &action.spend())?;
    check_action_output(fvk, action)
}

// check spend nullifier
fn check_action_spend(
    seed_fingerprint: &[u8; 32],
    fvk: &FullViewingKey,
    spend: &pczt::orchard::Spend,
) -> Result<(), ZcashError> {
    if let Some(zip32_derivation) = spend.zip32_derivation().as_ref() {
        if zip32_derivation.seed_fingerprint == *seed_fingerprint {
            let nullifier = spend.nullifier();
            let rho = spend.rho().ok_or(ZcashError::InvalidPczt(
                "spend.rho is not present".to_string(),
            ))?;
            let rseed = spend.rseed().ok_or(ZcashError::InvalidPczt(
                "spend.rseed is not present".to_string(),
            ))?;
            let value = spend.value().ok_or(ZcashError::InvalidPczt(
                "spend.value is not present".to_string(),
            ))?;
            let nk = fvk.nk();

            let recipient = spend.recipient().ok_or(ZcashError::InvalidPczt(
                "spend.recipient is not present".to_string(),
            ))?;

            let note_commitment = calculate_note_commitment(&recipient, value, &rho, &rseed);

            let derived_nullifier = calculate_nullifier(&nk, &rho, &rseed, note_commitment);

            if nullifier.clone() != derived_nullifier {
                return Err(ZcashError::InvalidPczt(
                    "orchard action nullifier wrong".to_string(),
                ));
            }
        }
    }
    Ok(())
}

//check output cmx
fn check_action_output(
    fvk: &FullViewingKey,
    action: &pczt::orchard::Action,
) -> Result<(), ZcashError> {
    let result = decode_action_output(fvk, action)?;
    if let Some((note, _address, _memo, _)) = result {
        let node_commitment = note.commitment();
        let cmx: ExtractedNoteCommitment = node_commitment.into();
        if cmx.to_bytes() != action.output().cmx().clone() {
            return Err(ZcashError::InvalidPczt(
                "orchard action cmx wrong".to_string(),
            ));
        }
    }
    Ok(())
}

pub fn decode_action_output(
    fvk: &FullViewingKey,
    action: &pczt::orchard::Action,
) -> Result<Option<(Note, Address, Memo, bool)>, ZcashError> {
    let nullifier = Nullifier::from_bytes(&action.spend().nullifier()).unwrap();

    let rho = Rho::from_nf_old(nullifier);

    let epk = action.output().ephemeral_key();

    let cmx = action.output().cmx();

    let cv = action.cv_net();

    let enc_ciphertext = action.output().enc_ciphertext().clone().try_into().unwrap();

    let out_ciphertext = action.output().out_ciphertext().clone().try_into().unwrap();

    let external_ovk = fvk.to_ovk(Scope::External);

    let internal_ovk = fvk.to_ovk(Scope::Internal);

    if let Some((note, address, memo)) = decode_output_enc_ciphertext(
        &external_ovk,
        &rho,
        &epk,
        &cmx,
        &cv,
        &enc_ciphertext,
        &out_ciphertext,
    )? {
        return Ok(Some((note, address, memo, true)));
    }

    if let Some((note, address, memo)) = decode_output_enc_ciphertext(
        &internal_ovk,
        &rho,
        &epk,
        &cmx,
        &cv,
        &enc_ciphertext,
        &out_ciphertext,
    )? {
        return Ok(Some((note, address, memo, false)));
    }

    Ok(None)
}

#[cfg(test)]
mod tests {
    use alloc::vec;
    use keystore::algorithms::zcash::derive_ufvk;
    use pczt::common::HARDENED_MASK;
    use zcash_vendor::{
        pczt::{
            common::{Global, Zip32Derivation},
            orchard::{self, Action},
            sapling, transparent, Pczt, V5_TX_VERSION, V5_VERSION_GROUP_ID,
        },
        zcash_protocol::consensus::MAIN_NETWORK,
    };

    use super::*;

    extern crate std;
    use std::println;

    #[test]
    fn test_decode_output_enc_ciphertext() {
        {
            let fingerprint =
                hex::decode("a833c2361e2d72d8fef1ec19071a6433b5f3c0b8aafb82ce2930b2349ad985c5")
                    .unwrap();

            let ufvk = "uview10zf3gnxd08cne6g7ryh6lln79duzsayg0qxktvyc3l6uutfk0agmyclm5g82h5z0lqv4c2gzp0eu0qc0nxzurxhj4ympwn3gj5c3dc9g7ca4eh3q09fw9kka7qplzq0wnauekf45w9vs4g22khtq57sc8k6j6s70kz0rtqlyat6zsjkcqfrlm9quje8vzszs8y9mjvduf7j2vx329hk2v956g6svnhqswxfp3n760mw233w7ffgsja2szdhy5954hsfldalf28wvav0tctxwkmkgrk43tq2p7sqchzc6";

            let unified_fvk = UnifiedFullViewingKey::decode(&MAIN_NETWORK, ufvk).unwrap();

            // let fingerprint = fingerprint.try_into().unwrap();

            // let result = check_pczt(&fingerprint, &unified_fvk, &pczt);

            // assert_eq!(false, result.is_ok());
        }
    }
    //TODO: add test for happy path
}
