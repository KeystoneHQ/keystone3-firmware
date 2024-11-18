

use super::*;

use orchard::{
    commitment::ExtractedNoteCommitment,
    keys::{FullViewingKey, Scope},
    note::{Memo, Note, Nullifier, Rho},
    note_ext::{
        calculate_note_commitment, calculate_nullifier,
    },
    Address,
};
use parse::decode_output_enc_ciphertext;
use zcash_vendor::{
    bip32::ChildNumber,
    pasta_curves::{
        group::{
            GroupEncoding,
        },
    },
    pczt,
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
    check_orchard(&seed_fingerprint, &orchard, &pczt.orchard)?;
    check_transparent(&seed_fingerprint, &xpub, &pczt.transparent)?;
    Ok(())
}

fn check_transparent(
    seed_fingerprint: &[u8; 32],
    xpub: &AccountPubKey,
    bundle: &pczt::transparent::Bundle,
) -> Result<(), ZcashError> {
    bundle.inputs.iter().try_for_each(|input| {
        check_transparent_input(seed_fingerprint, xpub, input)?;
        Ok(())
    })?;
    bundle.outputs.iter().try_for_each(|output| {
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
    match input.bip32_derivation.get(&input.script_pubkey) {
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
                if target.public_key().serialize().to_vec() != input.script_pubkey {
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
    match output.bip32_derivation.get(&output.script_pubkey) {
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
                if target.public_key().serialize().to_vec() != output.script_pubkey {
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
    bundle.actions.iter().try_for_each(|action| {
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
    check_action_spend(seed_fingerprint, fvk, &action.spend)?;
    check_action_output(fvk, action)
}

// check spend nullifier
fn check_action_spend(
    seed_fingerprint: &[u8; 32],
    fvk: &FullViewingKey,
    spend: &pczt::orchard::Spend,
) -> Result<(), ZcashError> {
    if let Some(zip32_derivation) = spend.zip32_derivation.as_ref() {
        if zip32_derivation.seed_fingerprint == *seed_fingerprint {
            let nullifier = spend.nullifier;
            let rho = spend.rho.ok_or(ZcashError::InvalidPczt(
                "spend.rho is not present".to_string(),
            ))?;
            let rseed = spend.rseed.ok_or(ZcashError::InvalidPczt(
                "spend.rseed is not present".to_string(),
            ))?;
            let value = spend.value.ok_or(ZcashError::InvalidPczt(
                "spend.value is not present".to_string(),
            ))?;
            let nk = fvk.nk();

            let recipient = spend.recipient.ok_or(ZcashError::InvalidPczt(
                "spend.recipient is not present".to_string(),
            ))?;

            let note_commitment = calculate_note_commitment(&recipient, value, &rho, &rseed);

            let derived_nullifier = calculate_nullifier(&nk, &rho, &rseed, note_commitment);

            if nullifier != derived_nullifier {
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
        if cmx.to_bytes() != action.output.cmx {
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
    let nullifier = Nullifier::from_bytes(&action.spend.nullifier).unwrap();

    let rho = Rho::from_nf_old(nullifier);

    let epk = action.output.ephemeral_key;

    let cmx = action.output.cmx;

    let cv = action.cv;

    let enc_ciphertext = action.output.enc_ciphertext.clone().try_into().unwrap();

    let out_ciphertext = action.output.out_ciphertext.clone().try_into().unwrap();

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
            sapling, transparent, Pczt, Version, V5_TX_VERSION, V5_VERSION_GROUP_ID,
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

            let pczt = Pczt {
                version: Version::V0,
                transparent: transparent::Bundle {
                    inputs: vec![],
                    outputs: vec![],
                },
                sapling: sapling::Bundle {
                    anchor: [0; 32],
                    spends: vec![],
                    outputs: vec![],
                    value_balance: 0,
                    bsk: None,
                },
                orchard: orchard::Bundle {
                    anchor: hex::decode("a6c1ad5befd98da596ebe78491d76f76402f3400bf921f73a3b176bd70ab5000").unwrap().try_into().unwrap(),
                    actions: vec![
                        Action {
                            cv: hex::decode("4ac2480c13624d2b8aabf82ee808b4e4965d6c26efd9cfc9070f69e1a9a69609").unwrap().try_into().unwrap(),
                            spend: orchard::Spend {
                                value: None,
                                witness: None,
                                alpha: Some(hex::decode("105dd4f80b149ee6a8d5f11b2d0f7d0caa7cece6d0dce3ce494ce14977def354").unwrap().try_into().unwrap()),
                                fvk: None,
                                proprietary: BTreeMap::new(),
                                recipient: None,
                                rho: None,
                                rseed: None,
                                nullifier: hex::decode("ef870733c09572b274782e32e28809c201a90c1e179ad78e88eb1477c7bd9631").unwrap().try_into().unwrap(),
                                rk: hex::decode("7fe9364e043a92f893100dc09fc70f1a4faad022687767f8c3495a83a57e6726").unwrap().try_into().unwrap(),
                                spend_auth_sig: None,
                                zip32_derivation: Some(Zip32Derivation {
                                    seed_fingerprint: fingerprint.clone().try_into().unwrap(),
                                    derivation_path: vec![HARDENED_MASK + 44, HARDENED_MASK + 133, HARDENED_MASK + 0],
                                }),
                            },
                            output: orchard::Output {
                                cmx: hex::decode("dbc7cc05319a4c70a6792ec195e99f1f3028194338953ee28f2f9426e06e1039").unwrap().try_into().unwrap(),
                                ephemeral_key: hex::decode("c7a4a801f5a0cf4380263eed1acc4952ecc61805a6bc4c17ce0fe783aa8f582e").unwrap().try_into().unwrap(),
                                enc_ciphertext: hex::decode("212c8799c5cc99400f3c1685820fd7d6b86a155f43b35d803aab357ef65f95430c922192e3a5c9d0e23d34a39d4257d2361c6f7ffbe386e4573ace688854f1c45ff08644d1ec4de6ad877104f9172cffbbcaf97993eac6a54b426c492697dcfe15461a661a0dd770696f1e6a59b3d280034b38f96cecfb8bb8c3ee642640887e021cc06406c1dc94a1d0c1e71bdb864bd97e1ca6beb25bcdb5bb756ca209da24aea0cfe45f65e159ad395e78133bd56c227da05778df4368fbb5247bb33cddafc7fefd67a8cb26d7b8841896f3e7ca57e218273335851e980ee470a7995e7ff179eb4a566ca8a7aca67cee124b8d8fd32072804d288f9db115edabb90b2cb3121dcb6069f8cb0809e1d53e1b71182f6a903436fe6685706d0e089e2cef276b27e7cd0a32230e1da7f5ded3edd136dc263f4913b1fd519eefd7f4a23dbfa8c530807e2c352b1b2e4d69cce2ffc506e85bba1dbf0daf212bc5ec204964aa26329071ae19cfc2614b7e6f2b5f0b831b1dafaa91e1b2e0e46f7d6b7e6870209cf10fc13908b88d079802f3e2fa2a62a3a88dd7fba600655948ff716ee6e7ee76f2deeb32a2cac8726168dcedad7584c9b42a4938b617b605f3e7acde18f8f5b5495ecd5ccbb7c9d86888b8f6a236cb79eced16eb41dbe884382d78dd26768d7110843aa3e3804c0757768458d4556f69e8887d1cbbd3f3ab0c9eb0b66319052a6089a94fb769ecf80930ae87cf04d282eb4fcaa24e5959c3b535ff99ba2ecb0f71931035f37f9875c944bdbd741adb95d5fed3ddbb78585c21f58c3d74a6cc18418e8537e1b8").unwrap().try_into().unwrap(),
                                out_ciphertext: hex::decode("6295187eb1d8dc74a065d46ae2bc235a47e5914b4320419e1312157ca16f153269e44278ad6f999a3899dfa6d004ce685cd7759a33112b26e5359dc7fe7ec3d81429854b4bbf767857120d14019353e5").unwrap().try_into().unwrap(),
                                ock: None,
                                proprietary: BTreeMap::new(),
                                recipient: None,
                                rseed: None,
                                shared_secret: None,
                                value: None,
                                zip32_derivation: Some(Zip32Derivation {
                                    seed_fingerprint: fingerprint.clone().try_into().unwrap(),
                                    derivation_path: vec![HARDENED_MASK + 44, HARDENED_MASK + 133, HARDENED_MASK + 0],
                                }),
                            },
                            rcv: None,
                        },
                        Action {
                            cv: hex::decode("6c78ee94ced314a28898218fb3d9594ff97b96d7d92c71f9e1866731eddd3ca8").unwrap().try_into().unwrap(),
                            spend: orchard::Spend {
                                value: None,
                                witness: None,
                                alpha: Some(hex::decode("15716c9c0c9af80201b63a1b0a329fd9579824cfae4cd9f086848d729dd37cff").unwrap().try_into().unwrap()),
                                fvk: None,
                                proprietary: BTreeMap::new(),
                                recipient: None,
                                rho: None,
                                rseed: None,
                                nullifier: hex::decode("0e65a80237a3d3e1dcede4fe7632eec67254e0e1af721cd20fa8b9800263f508").unwrap().try_into().unwrap(),
                                rk: hex::decode("afa2899a1fc1f5d16639e162979b29bedbf84aeb0987a2d8143d10134a47f722").unwrap().try_into().unwrap(),
                                spend_auth_sig: None,
                                zip32_derivation: Some(Zip32Derivation {
                                    seed_fingerprint: fingerprint.clone().try_into().unwrap(),
                                    derivation_path: vec![HARDENED_MASK + 44, HARDENED_MASK + 133, HARDENED_MASK + 0],
                                }),
                            },
                            output: orchard::Output {
                                cmx: hex::decode("c7e391d7deb77891735e12be5f63e8821a79636a774578706bf495bef678072b").unwrap().try_into().unwrap(),
                                ephemeral_key: hex::decode("b306bb760dc8b8018db27ee58072969d1665b98095b41034615d4ff62410800f").unwrap().try_into().unwrap(),
                                enc_ciphertext: hex::decode("572b855e2c2b872eb6d9e375831a34b486833a015d0d42876c8b8b47409aa67d238903a931539466b12645d0e7f18ad6637fc81152b145585511245a48b9e4a20069dcf3d10aef699388f6a1855567c5312d66a94724db45c10ae0bc4a6af7fa508b4184859a1bfc38dbed7258b39406a64af9a401ab9d921f74fd8fb2f44893458d9a0fd67c773a8d65ecffe2f0868755b8e359ab3b7bf6ebce2553745b96d31bdcea662188691f9fc12fd652b8528e6339924c66f12e39e4b1b3041fde91cef49b7c9f4c0201e22f712ecc599219acdc4d5c77b795ebe3c80a701e22780274c2f88298fa40af2bcba9b78b258e80bebf5bf962c82e020e7444aafa3c857f8fefe5c2c79627873e334af336defc71c772c472840228cd6a7a870ff16efc2204d232b1f4da4b17ca5c9dee367c7aecd0f1deb9ec65f6c03f26ec95c6e9f03f5da0419260be47703ac2a56467883a272858625cb64bd3c0e388a15197665493377984c78aee751bab65971ea0b511879b6339856d724780250843a34af9462c765ac5200b22b6a35341c73ef4da9fc82087f3fa9dfb6ce6434a1e60b6c15beedfa3a8ca2feeeb249fb73154d541c4ced12936fe5ab6b0ac989eee5d045a36659d31d7352f77db6c32b8a827a456ce93bcd8f69e9b3b17ba2f44016107a886392af6c413e54d3707008573d2b393693616dae726e2e1ae52e437a0a5bab14ffe5ea26df3be381770fa9fce263a0adfbf4bf5182826c573da06d83011e85dbb16866099de5dc79465f46b29552565eede84f36ae0b443ea05e46a97362be8796bb635549108").unwrap().try_into().unwrap(),
                                out_ciphertext: hex::decode("f60df073061724815f4ae663a99a6781fc5ca797390541172c5cf8b4fece3d45a07d97636853bdaec1758fa8ba339b935462ff4bc23ced395990a6551fcee705d092bcd33a0a68c41f2cd15d59128060").unwrap().try_into().unwrap(),
                                ock: None,
                                proprietary: BTreeMap::new(),
                                recipient: None,
                                rseed: None,
                                shared_secret: None,
                                value: None,
                                zip32_derivation: Some(Zip32Derivation {
                                    seed_fingerprint: fingerprint.clone().try_into().unwrap(),
                                    derivation_path: vec![HARDENED_MASK + 44, HARDENED_MASK + 133, HARDENED_MASK + 0],
                                }),
                            },
                            rcv: None,
                        }
                    ],
                    flags: 3,
                    value_balance: 10000,
                    zkproof: None,
                    bsk: None,
                },
                global: Global {
                    tx_version: V5_TX_VERSION,
                    version_group_id: V5_VERSION_GROUP_ID,
                    consensus_branch_id: 0xc2d6_d0b4,
                    lock_time: 0,
                    expiry_height: 2705733,
                    proprietary: BTreeMap::new(),
                },
            };

            let fingerprint = fingerprint.try_into().unwrap();

            let result = check_pczt(&fingerprint, &unified_fvk, &pczt);

            assert_eq!(false, result.is_ok());
        }
    }
    //TODO: add test for happy path
}
