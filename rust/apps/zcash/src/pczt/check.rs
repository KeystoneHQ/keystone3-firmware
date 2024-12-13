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
    bip32::ChildNumber,
    pczt,
    ripemd::Ripemd160,
    sha2::{Digest, Sha256},
    zcash_primitives::legacy::{keys::AccountPubKey, Script, TransparentAddress},
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
    let script = Script(input.script_pubkey().clone());
    match script.address() {
        Some(TransparentAddress::PublicKeyHash(hash)) => {
            let pubkey = input.bip32_derivation().keys().find(|pubkey| {
                return hash[..] == Ripemd160::digest(Sha256::digest(pubkey))[..];
            });
            match pubkey {
                Some(pubkey) => {
                    match input.bip32_derivation().get(pubkey) {
                        Some(bip32_derivation) => {
                            if seed_fingerprint == &bip32_derivation.seed_fingerprint {
                                //verify public key
                                let xpub = xpub.to_inner();
                                let target = bip32_derivation.derivation_path.iter().try_fold(
                                    xpub,
                                    |acc, n| {
                                        acc.derive_child(ChildNumber::from(*n)).map_err(|_| {
                                            ZcashError::InvalidPczt(
                                                "transparent input bip32 derivation path invalid"
                                                    .to_string(),
                                            )
                                        })
                                    },
                                )?;
                                if target.public_key().serialize().to_vec()
                                    != input.script_pubkey().clone()
                                {
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
                None => Ok(()),
            }
        }
        _ => Err(ZcashError::InvalidPczt(
            "transparent input script pubkey is not a public key hash".to_string(),
        )),
    }
}

fn check_transparent_output(
    seed_fingerprint: &[u8; 32],
    xpub: &AccountPubKey,
    output: &pczt::transparent::Output,
) -> Result<(), ZcashError> {
    let script = Script(output.script_pubkey().clone());
    match script.address() {
        Some(TransparentAddress::PublicKeyHash(hash)) => {
            let pubkey = output
                .bip32_derivation()
                .keys()
                .find(|pubkey| hash[..] == Ripemd160::digest(Sha256::digest(pubkey))[..]);
            match pubkey {
                Some(pubkey) => {
                    match output.bip32_derivation().get(pubkey) {
                        Some(bip32_derivation) => {
                            if seed_fingerprint == &bip32_derivation.seed_fingerprint {
                                //verify public key
                                let xpub = xpub.to_inner();
                                let target = bip32_derivation.derivation_path.iter().try_fold(
                                    xpub,
                                    |acc, n| {
                                        acc.derive_child(ChildNumber::from(*n)).map_err(|_| {
                                            ZcashError::InvalidPczt(
                                                "transparent output bip32 derivation path invalid"
                                                    .to_string(),
                                            )
                                        })
                                    },
                                )?;
                                if target.public_key().serialize().to_vec()
                                    != output.script_pubkey().clone()
                                {
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
                //not my output, pass
                None => Ok(()),
            }
        }
        _ => Err(ZcashError::InvalidPczt(
            "transparent output script pubkey is not a public key hash".to_string(),
        )),
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
    if let Some(value) = spend.value() {
        if *value == 0 {
            //ignore dummy spend
            return Ok(());
        }
    }
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
        if note.value().inner() == 0 {
            //ignore dummy output
            return Ok(());
        }
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
                hex::decode("2fac20755b7bb7c99d302b782ea36f62a1b0cfe8d7d4d09a58e8ba5da26f4578")
                    .unwrap();

            let ufvk = "uview10zf3gnxd08cne6g7ryh6lln79duzsayg0qxktvyc3l6uutfk0agmyclm5g82h5z0lqv4c2gzp0eu0qc0nxzurxhj4ympwn3gj5c3dc9g7ca4eh3q09fw9kka7qplzq0wnauekf45w9vs4g22khtq57sc8k6j6s70kz0rtqlyat6zsjkcqfrlm9quje8vzszs8y9mjvduf7j2vx329hk2v956g6svnhqswxfp3n760mw233w7ffgsja2szdhy5954hsfldalf28wvav0tctxwkmkgrk43tq2p7sqchzc6";

            let unified_fvk = UnifiedFullViewingKey::decode(&MAIN_NETWORK, ufvk).unwrap();

            let hex_str = "50435a5401000000058ace9cb502d5a09cc70c0100e5dda70185010001227a636173685f636c69656e745f6261636b656e643a70726f706f73616c5f696e666f140cd612b51afd462f8b19e3edcaf79157bddda7010000000000fbc2f4300c01f0b7820d00e3347c8da4ee614674376cbc45359daa54f9b5493e010000000000000000000000000000000000000000000000000000000000000000027ccaa1746a7e4f233b0e8b1e0ba1100599646dea4823707e388f56fe0553783f8651dba92ed02d4b1ae37f059ecebb55d19f136e1775d44f05d97e84ced6c90a6a8fec0b2f85315e3cbc44e6354d23d6e0f29aa323411bba869c3f87befe8b82000114b0cac3e14c4e1a4a4fd22c98563b9cbe859c67768a39f67abd5dbe55d3d16ff58e5c45b21d8c573f54b901f0c3f3010116f6b3245e0edd4c9bbbca1fade86fe002f5013484befd3ec14489e8439c853601c5ee514905a1665cf4732252c7f101895c604a65413d9d2592bdea6d3f95dbf6017d6d7535f6e3eade13042662d05eefa91d1f4757fcd0b4b844554e77366f4b36a7068e15c2f06aaa11f6bcfd2e9e36231530069f7fb99a2771050689f24f332b427fdd05d1d5f3219230a1406f0955e3ddc0ef5ce2c7f160d797f3e54ef5782001f4a3ab17cb9fd050a3d41632aa292b83b5ad3d8ad48c4c04266accdb07521b79b0d02a172dfc6fa69f67b161755daf4158577ecf3699c44886cb1301c455405a5427b42e2613292ad8ecf30170e0b481b06d5444c0ee136e4e44a887f7a83e87b1883f34abf821b8774c8335addfb4e78002c1078a4b0fce3c245ac02e219bca314dcb31826125625f323b832c2fcb450dee46c7538c3f9bc5d51e99dff6853d94c05817f4c87055cca5619e4fa814844aca850662c404673ba3c1091a946ba64dccc22f3100c0297533ab75c22b113db4693d8cf61b21570c1f21edf89477a2e7bf740cb1e779f3088da9819a590c5474384ed04d724606a76ddf80036e2ac52a8fa7057aa0667da7c4ef890f2eab3c5892e39be29b020793a5d3f0f6f3942981c26a38007ab14443f4b375536142b59f222f5337bf4c671bd6f57aa3fd5e050726dc3c53fd99c2e7156a1092c95bce18d9e2a5cc568a96556430c1785958b26fb22b12eec609dbfb205dd0c6675c576ea301bf0aa397c4cd383f6ace064434e405b5093c1d81175c618596a01ecadad95d75fee2eb4390fef14a084ac781b79e487f0f1dbb6c1523d9a86316044757219311a23d5ad6e71d34b8f6a2832494d2b98d3d7597692c9b1a0b7d4bc8e93db1c90c8331e9709ac9973ca3fbf38b2aa955382b0c104faa2b1148b1b627090aa939e321b82edcb0ce70aa4eee298997556b4f3548878ed93d489a456128cf0407d3aa22a00f815e52b4cf752ab6826cdd93943012278dfeae9949f887b70ae81e084f8897a5054627acef3efd01c8b29793d522ca2ced953b7fb95e3ba986333da9e69cd355223c929731094b6c2174c7638d2e60040850b766b126a2b4843fcdfdffa5d5cab3f53bc860a3bef68958b5f066177097b04c2aa045a0deffcaca41c5ac92e694466578f5909e72bb78d33310f705cc2dcaa338b312112db04b435a706d63244dd435238f0aa1e9e1598d354708102dcc4273c8a0ed2337ecf7879380a07e7d427c7f9d82e538002bd1442978402cdaf63debf5b40df902dae98dadc029f281474d190cddecef1b10653248a234151f91982912012669f74d0cfa1030ff37b152324e5b8346b3335a0aaeb63a0a2de2bca6a8d987d668defba89dc082196a922634ed88e065c669e526bb8815ee1be8ae2ad91d463bab75ee941d33cc5817b613c63cda943a4c07f600591b088a25d53fdee371cef596766823f4a518a583b1158243afe89700f0da76da46d0060f15d2444cefe7914c9a61e829c730eceb216288fee825f6b3b6298f6f6b6bd62e4c57a617a0aa10ea7a83aa6b6b0ed685b6a3d9e5b8fd14f56cdc18021b12253f3fd4915c19bd831a7920be55d969b2ac23359e2559da77de2373f06ca014ba2787d063cd07ee4944222b7762840eb94c688bec743fa8bdf7715c8fe29f104c2a012db4073cc29d5a2e0524f885ab386d56f640e10ce64213a6ffb6421fd2205934012fac20755b7bb7c99d302b782ea36f62a1b0cfe8d7d4d09a58e8ba5da26f457803a080808008858180800880808080080000615dc43debb52ac32763d67fb8b7045f9f06c3648ad89e6b771a444f1688802c8e898b0e59097032de086dc0b2e5d6f4ae8087ef0499bd263f302083121d8c38c4045f1c1b5767977b8e53d80a72424f01b74c82116535190cbe90310626d3ae1ecc9637628e270c30cc259a3add824ba7e46ec055cb06a8abb5da0042a65be9614ed2aa9739458902b02c156008ec0286acfdb5c7c00c909951a3bf9bce61d2d2784f7b3a2bd2ac56b5f4b655a7e0139f17b71d0da50bfa3cca849ae1159fdcbcc6536f8142fa42c9a9afcf5c00662dbbd75e45bed1a037cff6032311137285d8a93e4aa0573933d73894b559816295f512f4a0d7e5e7e2727e37716ac0a3daf618e26e381414a0cb72f5d6dcd2e33744f32ba95026be07371555e01cc75f1fdcb186ea1d83bc00e7d49c21a17ed8b1808844ca6528844d461a51074aaa75069548c561eca8db399fc98ea1835ffa2158b33c99fc6cf4c3ea7b371f672c35773f72d85ddad58495174ff13691464157d711fc25e76fbd41281195f16d47253116ba8d655f6cdf2dec9ce0d86894f4b032a44bdfa280bc1f41075f3927e35cb87f5d1f7f248e1632cf67b17c9ae7955278bf69748739a54a7734bd3d1edd8afd3fe7c0ff3782e4161fd7332214e1b5c46e5865e7109e137f8f4a4d1fefee1059834f2e9051a1e24d99fe985f248c082164a57bce56b3b7c8b5dab31838c3cf3185d5103911af0b9fdf99deda6406730ef0f1fb08b7364edd0bcfab93201190ff3235fc3f6c9e071bc203f3184f203af30a195b6f1d975b2946b9ad00d5a2dc359b6d36b135e0aa5a0d0d371dff27d633b1dd79da08422d6c06932901cb00896f5482d48d2cc6b2fdbf295d6b53589c9fa2e89e99c8e6a08f60445cc0f9ec9a37dfe9cb1fb9bb5029a1368b83f3351a88dc179ce755a6c81c3d745ed697532eda0dce2aa127c86c9dbc839a3ed3d1183b119f31959193d70bf48dee76c88136bd843f34589d8b91bf735fc841c7a4d2bfe78248c4be64200114b0cac3e14c4e1a4a4fd22c98563b9cbe859c67768a39f67abd5dbe55d3d16ff58e5c45b21d8c573f54b901d0a7f2010172babb4ff26fb841ee873c78916260c937a892c4a0eff29ff7720bd2b467d749000001207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666f12020cd612b51afd462f8b19e3edcaf791570001bd5c6c6b14dc35cf4ef022ea6dc50acae3feb45c5db4cf1a2eb29070685a4714b46f4a373ab7608f00516d1351e3865f05df962b6322f9b775ab758b66f855bac7257dddbce49dba62f9e3da4cbb45a23c74e56fe27748c9bd65e87a13a1341825ba16c3a2bf3dcfafcf87d3c3a7817701715549aaac369de4f61f6b410d383e01ef9fd97365aaec402256d39dfccf07d4aa6fe632abda1277c2fd5c952d4516a8b54e0870d75ae397841bcbb0683a8fce8874ca7983630c152f45cc4e245044390153c50c554d797a27bec83fe85cd2d84152d4d142b5f2881dc487268e5509ffa7f2bbab1351a0098f7df31d010001dc97669d3c6d0efcd829c4a973c01745618a1e34af77cd504ea7a523c326922901ca36adfedc332f498e8fc32a7d081b8804b4052e369797a2592b1cf6cfd1d4530174e6ab053cd555d3813a4837292694cdae7a0c2a6051ae40e0b375a4e136e4190fb6185ee54822c29784e1ef573cf84f9ce829c4228815bef835675881a54b1e2b09447015d6f6430fb1f7008dea36f25d6f3c7e58e2a899903f8a2f0d868d2301c0dda1b601f97ac918842fafe231b4317e2cbf17d3851d8d4d3d2d90886068993473b6a70f41e15581902bbc7c9b9dfddd618203ee224054312e4d0a04ba7d0557faf2dd1f989f1fa6cf8c71cbe3bddba251dd18d5b13b8080c82a1c5aa8efff30a06a52150c9866cd4f0f5d62036651c7bee5785c34bf53bf718db7f6df0a3d228c774e24c10985ce4921cc1baf85fa4a7b0ba9f7020503d3514b5d225390ecd75676f81456e615a45702e6909833f00ce30e4ac9ed7f6caef953567f5c341613b6c35d2b8bf768a7b2c90d4d6fccc261ce31db37deb86812b50cf21be36434ada33d79015f936c8fa7e9025a8e668c949336b64555395ab31606e35b399abce61215812b04d2b48fbd880fe25d99d894295ca5e9d181c45f3ea69cc0539345f81ce3403bd2a84167737d6329a3733818d6b501fa1954c46027aeb79c3ad060bf7e62e53c6dd808ee7c6d9e27ccd5a416407af8cf0b440846d62c028a6b92283a1153be3e1380298e5c8d2103ec2649809c82758cacde4dc01312d7fe2faee911de6c3f0c517e6000d120c5190cf9172e603fc50bbecab2aa53efcd1b61fc1dba12d10d1bcf32f2ff234cec2097564bcd80a3a3333c4904a1ac8a56dfa594b8048a6b570631be88c9229290ce00e418cda1d3f5d5586461cc3b17226d589afa2ed5d3e0132dd62b6eb863a64536259499cb3caae1c625f7b24618071b97414657b301172e2e02cdb5293dd6ea830725da2a175f4676f39572974ceed978c7f3350c23713338e152c058f1e547d2028670c6f0c6af77339e5b67bf23babe8cb50a54bd7a25dd4b8644edf79aa3bd0d170a10e0476403cdc3fd01fa774f14958f831e7c12338aaf7dc0f4b18d9c2667f5855db8b968ddb11113e81527b8dcb276ff22a6d92aed7d2fa4066a5261ee02cba50f1131fe311538d174aa48effa951d14dd70ca0b0f82468300bca991118a185383e34b8c60231c6afa15e642bdbc64f35bba963878ff9662751ea9208fee8ac1660697742b8ef7a58a68f51c2de72e2507ef503adc41514372a8c9b954c3fe51cc5cfbcbede75899443147cde3a1622bd0dd4a3fd045e180b87da6858588db602ff882f25212ac80374c1a50bc2256b13dfcd90dc1f6e436da878d67b2ba288160f4436a056ebd0130b4fc405e43945c590fcf360140025a98b4c0ac7145dd4986a7d7f5aeb4f964b3c6deada336a87f45b4821eba17c95a6dddbcfe65cd2ee80271e3a9911be6149142c56380bf0f8524319906c4095fa96bda83b4129ef7603a907f3be1cca1ad95e22efaa57542edc4d4a6196f6094e7f918403c957de278b872883d529ef63328dc432ba4d7f7a9e231f61c3ab066c4fac4dc5e89808b6d9bfefa6098ac39e2fb9a80bffc19efa12edb09026cee7bd4ad26ee35714e28bffd499fbc7e010beeed6572a67e1fd052d21afb1d018b05fa588a7d4978a2d30d4e4c916baa644f16fa3d6bac76d1729fe055343306012fac20755b7bb7c99d302b782ea36f62a1b0cfe8d7d4d09a58e8ba5da26f457803a080808008858180800880808080080000c525233b5c108ffea501eb86355e21102df5f271dc184bdd7c70a4cf4be4db0e622eef078c57b376011a105303d86bb4ac31aeb97838acec729b1a0ab1869907c404c4480357e5b959760b8af9e60f692c70d3001f657142310e2e57462a7b67db965848d3afd70d34df667d6d06d2578c328cc83247fdcac86efff812d7da7ea8f9e7a9b272ef45342bbb61c514a568907c8514cabd889ac32aeff178505ded741f5322a75aece7f092bb26b78523b1b07e7a113da6db201c5a99ec289084ee2abbf802cf2bea95ad240eadcc3a64911b8061a106b771c59cb03d1603164984f523a2d5dcf30ace0d8edc5690f76663c9e126c2c24a7d79549d6c0380520af978cefcc354f9dbfb92b432b733caae37a1239848246da9ed55bb90fbec88dcee00cda132e42bf4da1b0d36d3e17760e48d8b4830296307ba2d120bdbc965866a8a2ce84575b2be5e2e5c1c2850e1c28286e8967a5c82adc0ba91b81e4fefbd822fd7ab21262c29a5a196244bb1731a888194868df4037e723a18a17c6261628a776ede0bd6747cd2a333f1042b5824e49476915b8e989ff281ac3cadd23c617f07d5d814b438c15f502ab00a9fb980709e1793f38ea52cfee2ba24380c8ac14a67fd6a7ebc2a2be2058dc3cd8efaaaaae6cff23d805721ae8725a83cf6a84a305518510c19a0c4f62ce1533568dbb3b10a50a98447c249a4ce7cdbedfa600e0a53c0467200375d45df2ed6c47b8320ee3316da1a766435d60cae5fcf41e0d7fa58b5aa207ba3be4bc37e7cb376ea1870630add795a64871386d13a4982fb742a5fb02721a8222679ecb87867ca82e920ecd83989b57a325c742d933255192ab0122178a2f41777d3aeced835bcfc8d1742e6dd0e67b93ebfce7b62e82f55b699dba8df66b24d506d4c0f82eb1bed6df445502ea9ea0aebdc01d622afe25bf8eceb9faea0c7f507eca7a41e6f10e9cf54948b54922c8c3cb4da1eda29b2ed0e1539b564524ba62b0e2cc2e2ba8594e9ed51e743386f94300199a66f4d2212d15f6b9ddf1d738b41226e3954130fee9e1654409d3eb52a82e0e18f62bb13bab4f37db92901904e0175bbcbaff297f42ec9fd5bc9e1a40b83c5f7bf42282b0fc2fc1e7c3b54414be4000001207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666fd80100d50175316677787573387a63687136343772346a61717833327537736a32777479706c7376756e336a6563613272633477353575387572736e337132773438377634756d6b686738776c727465726e686e3064776761707471683537327566306b357078736d717a75726633336671677868353375616a6b7472683377733735677766786c65677537686e676839726c6c743635636e7568393675766d65336366346b663366357938737936396a39356e6c7370726176396d3977336d7a6c336b757663306d6874716d3074746c71716a6d786735357501b1ba2d7a090e9d9be8fd07bcc8de2b5894285ce5b8ec13933761ade41654c21e03904e00a16f0cf2345c7df7646039daef8f2ddc84b0d7325498d9b89086ff5643bb080d00016e179ae51dead26a37ee2aa636a436227827114216a1e3ad65133e557fae0933";
            let pczt_hex = hex::decode(hex_str).unwrap();
            let pczt = Pczt::parse(&pczt_hex).unwrap();

            let fingerprint = fingerprint.try_into().unwrap();

            let result = check_pczt(&fingerprint, &unified_fvk, &pczt);

            println!("result: {:?}", result);

            assert_eq!(true, result.is_ok());
        }
    }
    //TODO: add test for happy path
}
