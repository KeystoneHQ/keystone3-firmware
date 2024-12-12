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
                hex::decode("1b290d0b4449383e1f899dab47bfa689794f15673c4bba95eed10e14ca5aa953")
                    .unwrap();

            let ufvk = "uview1s2e0495jzhdarezq4h4xsunfk4jrq7gzg22tjjmkzpd28wgse4ejm6k7yfg8weanaghmwsvc69clwxz9f9z2hwaz4gegmna0plqrf05zkeue0nevnxzm557rwdkjzl4pl4hp4q9ywyszyjca8jl54730aymaprt8t0kxj8ays4fs682kf7prj9p24dnlcgqtnd2vnskkm7u8cwz8n0ce7yrwx967cyp6dhkc2wqprt84q0jmwzwnufyxe3j0758a9zgk9ssrrnywzkwfhu6ap6cgx3jkxs3un53n75s3";

            let unified_fvk = UnifiedFullViewingKey::decode(&MAIN_NETWORK, ufvk).unwrap();

            let hex_str = "50435a5401000000058ace9cb502d5a09cc70c0100b2d5a70185010001227a636173685f636c69656e745f6261636b656e643a70726f706f73616c5f696e666f14cfde7c339d494f19940b66c0bbbe5eec8ad5a7010000000000fbc2f4300c01f0b7820d00e3347c8da4ee614674376cbc45359daa54f9b5493e01000000000000000000000000000000000000000000000000000000000000000002cf0a2640ff5d5f8b152e7784b30ee9b49a920c68d27efd3c0442065088c718028abc7880ed738a251f3cfc77cb28c4305d87951c38d8c9d42687ec6af5794c28d0f4a827518f6756033fb0f51a48b2de885a8f146a8250f02acb3f445cfe648101ec2faad03fafc19e8ec17343f4016a0fcb0e3cddb8e1a25665d6940b637d7dadf2944d2cebdd45af63e9e84052f9754826a769559015137d5238c187dd5a3e1e015f233e4aedb2d201204635d5db9929a7015148de647c0c39eb79c1872b537dd9f7a8dd84ec991fa8e8be180100013fd67b588f6e3957a56b5b6700f8e779f6ae4e74b726ec2133f8bda5c24b121e01ee68cbcb5efefb79694af15ddf3ca49fbb5b75a6dbe8a4c62b966faf5832f767014d6524e8de6a59b37df3143ea86366c264fb011419748c46abfe1a2cd455390dedbd18b91d95d7dcc98881cceb8f0bf22cb4b82d662d3e0313501854f98fbd13e9f94fb697e669fd071aeacfe18e3bfd4057527c7ad379745bd2447baac89f1201d8a29cfd082162927f676b9d440e663795d3cc8aa5c1acfe7a3bfe5c6c7fac50fd412e8a040436b7f5776fcfe193ebb7b5cca7ecbf8b35b0606a4f0f2fa98f486d2c757400c18bc0d94599b23414680b9ab01f410f28fdf55206baf25dbbc1500488abca3b01645111c62fb42da53125d98a4c874b8116b69e9729e72cd4983f9887906b3830c719d67f04811eae47016f61c1937bae9d19199c3fea50f70ea7e64af2042f1b555231e0337164c5f7185c757d7ba9514028209efa636ab35ea911a226482cd8fa6c534bfadaed490c189e251df2dd9f1e534026b61f2dde82a5653cb57f08b60c11b4e566ea75bed01b54c216c49344b8893b8e6ec7869176c245de6fe90712c3cc165027a9f99b3bfabebbde7a320b3cf4b882ffeb5caaa71f6674455f2fbda368f572b6fc748e67d271a5ed4f43f241d6eb75e00e863fb00905499e8e0342b4f5833a074b232cedfabe023ae83dcc0c8e74d82d483b5f810519713cd12ff4c29c9a0bcabbca9af73af558cfe4dc3cd2302320369866325e78b95da1721f5db7d5810c0c1acecc730bcec4d0fdd720802970a5f7eac6716d825b6e1fb619cb5016d59cd8bf38052b7fd6bd79f7d89d34648c24c3ac60826b4cf0501d311c1897ca26a407ca8228b5ce15b82cb95b415db08e9e787ca0af597fa02882643bb2b285a6c54701e7d437f3330c6de2cea1a3932788a0a9d57345c11b62e2c31d843eb1c65b62ad11fa98693df93edae814db68af73c754e6e11853be8f9660112f89fd0ceac45dfbbcbc8dd8799756cc53b68418128860e375df3e25cd4597054afa50750146e23d06972eb64eb2835e6ae5520674ff1cc86ed364fc9f39d6292a8cbfb6fa0e1000ede8cd1722a45127b7fed74626e8863fbec6cb505d7c8b05470253a5915ba7d565fc25cc9afd87ca1c554a162f143457f82df5311ee4cd262e7b21d9031c677317e730dea080b398a1e1b00b48ca3770c548eefa3276df12550655f669d3638e9a00ad4016ab62cf17148005b08639d491bdc1c45bb9de2294ceae0b8f21fe643a8bed8403aab09008c462f7718da8f2df120a29c6361b0cf4192289bc721845e4d2666b2d2ad6fcb5c638706dcdc592a3818e040bb9ea267053bfbb670c6b436941c3c4771c047ac768c84786d7c948bf6215d1eb16f7313fe26806de43530f6fcf05ecb596636013df89bd926054e047a91dff9d08721f9b3801efc34209958784ebd79f3a9d61a25d399ff9ca02b50c7ec46edf45c93e781206b73b8f1d76ba04da5066135c5d64d2ed944a79a7805fd2955eb0db1819e96109d5f43eaca87b6be732e04aa0b7c31869d4a36eb8c8530827b1d1142f347c24e0d4470b5309a120d57325e270b594d5dea77b4c601cfa10bb7b3f49a5033a5d799e2e6243348b0e32c36f7239d17850ac9b36373302f63c162251e6a42201dfae0d9f6a288194c518f4272c841a91fa9aa46b13bd5a1cabd4c530b20c2531011b290d0b4449383e1f899dab47bfa689794f15673c4bba95eed10e14ca5aa95303a08080800885818080088080808008000060fe62bd5d29131d67f0e027443704708565c5c59bf8c3747b8860001b6856149d3513acd45aa1fe4b2a3e40f41837778c655eec92a5c9debace1b9031525d9ec4045e155561e2ad060fcc6ae5aff62a4fd4bd238ebcf9afc41903732507c550c7b050fd9d9b4f7e845c3fa66304b13aacfd5c567bdd604ad2fee0ab6932ab38f682e317e397e2aa04be18a9574bffea2fde4eea6f5d53f35c57b7a6b01aa739b0a96706fd19d5b85f568df085777c28ef5feaeb44e0e328c727e017aa22ad5d809c74f9700ec8ca98d33a00488e1bf5ec17500882f24b8885483fb5c21c3059493d6b250c9091230ece8268c44328e08a7da4b6b7184e4a3f00e58fc5908ad462d2dc5bc1bef83bfd9e897ebf5fd90d4a247d4ae8c11d5abced9a33938ed0713d54395cd11ce19d23337012cb9042099440e5bc4c95a139f076f28612abcae7ced073ecc89c86872406187dabd7db58fc8f2509dd1219789d1536181eafaddde4fe41e32028c426456880ec14b331331a5507797074ddd791efa7071b432adeebf22b4adefc28fbaf9a509c9a17bf625da962c1ef5f3193fa33de1f608bed130be408423c6e2a9fe636c9f289d6e4d54d43c735c95a58243ee0270a5515774fd45976a1d75dc132b0ec0d017b2333a7b13067690444d1e35a271fa9238f7d21afe309317f9e507a67538dfd4ae95a8155c2b80e8fb4541a6982982fa084241300fb927fc3d23048c3bbf098cd6f07c475f6849ca005228f0842b1c79d022d3f09512f3327da3332f9c2799e048e17e71065f9c40a5bd8963ccfc8a6ee3139f8132b595260609e25460f9a358cd7ecef6324f64bd936e07770c477c270ac6bcc908680ff5a86a760bdf917c3e1a08c702340565b0a0f8bd9b190f91ad15a0f35173db5ff7c5c505f6d9f3edba039d304431624aaa33552a699823bcace93b1425d08291accfdcb889810d40c8c27c1e37afa6256941cb8886c0fd816a071e5eceebd9e79d3c791854941bb571b207a649b899305e83eae016c260344c3fb60a31ac6ab541060069e25c9bdf9ebcbeca3e3d82744acdc529529940a519711d093eddc9701a08d0601276fa1b4717f4f56bd32aa942ef184f8ca8973b4dfadf0d7bfe798bcbd2b5260000001207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666fd80100d5017531767a7335796134633771787537656a7732393438346b616372736b7974766b6b37366c736333336c6b6c78777679787771746d346572796e7678326c7175776e75787a79326c3774747a3838366a366133617037776a3963647a79647a6b71387339356d766c6333786477633477386175377374723667793932686464657872716a686d6b7933337666346b7430646e6e33396737736a6d79326b7a7835667861397a613836326d666e356136666739686d6330706a757374746e366b757a736164767865396e75323568303570746564676d01089473dddfb28273ba280af954a9ff66af216fce512ab4fe03a2264e6a54850e8c87ca7230dd5ef7acf525347e2ae27a9782a46748e1436ff655a7398e6e992a6f190f06213fc66da6d0a25fbd84a914a7ffc099b90eb34a519dfcd00a4a0f210c8f6ca51bdbaef4a8ea424bbe94d86b1a6803636dbb6fe86e18941c7b03703400015bb146d7493c853fd0020fd02a0e2663a613bdadb33b1e202462d1ebfa818e2de84ea38185721813a6402f01c096b10201d3a77387fdc748ffccc19af66e6ec2e6484e656fc21333ad4fd993d1c39d590101dfacde28d67d6852e209a5201341a8e5777c3ae159f19913d198bc12b92a21f401677c12dfc237d005428692f80f9bd8f35993d0437f4955d55cf64b4a027ec92059ac34cdb5b89e2db8130175daf8e5d89cb65eeff6a1a21261aabb60ed6c1012e0657719566e48433da7a9fa319b23bf9e783b11047f1ed691b7164da32fe1120183c7ac173f1f169c1c42cafa7f4b009f62f5abf691d36eca01894ec45ca32c9171bc5f304a3408822821b080e36488cc4909908b3d99cf2d536823fa0da72040afb0523b7b690a73a1b46fdf78e4f5168800546dad3f1cf1e72567704d7074d2b33c46242111fc397753e5fd50ec74816df27d6ada7ed2a9ac3816aab2573c8fac794204806afbfeb45c64d4f2384c51eff30764b84599ae56a7ab3d4a46d9ce3aeab431873e4157f2c0f0c645e899360069fcc9d2ed9bc11bf59827af0230ed52edab1827ab1320953ae1ad70c8c15a1253a0a86fbc8a0aa36a84207293f8a495ffc402d512cd870759a7de2786222d7604be32f05bfb8ede90689279dfa6998674103981b7b81a8a5fed4558434ba9943b107bf71cf173919f2caec264d33c714c5e1af11d35df91a7ba195f7597e87ff43f7daf55570e54e735bebbf0a467d171440ba3c02568acebf5ca1ec30d6a7d7cd217a47d6a1b8311bf9462a5f939c6b743073ef9b30bae6122da1605bad6ec5d49b41d4d40caa96c1cf6302b66c5d2d10d3922ae2800cb93abe63b70c172de70362d9830e53800398884a7a64ff68ed99e0b9d2e26bdef115ced7bd36305a54386996133c4e65759f3731637a40eba67da103f98adbe364f148b0cc2042cafc6be1166fae39090ab4b354bfb6217b964453b63f8dbd10df936f1734973e0b3bd25f4ed440566c923085903f696bc6347ec0f6f3f63aab58e63b6449583df5658a91972a20291c6311b5b3e5240aff8d7d00212278dfeae9949f887b70ae81e084f8897a5054627acef3efd01c8b29793d522ca2ced953b7fb95e3ba986333da9e69cd355223c929731094b6c2174c7638d2e60040850b766b126a2b4843fcdfdffa5d5cab3f53bc860a3bef68958b5f066177097b04c2aa045a0deffcaca41c5ac92e694466578f5909e72bb78d33310f705cc2dcaa338b312112db04b435a706d63244dd435238f0aa1e9e1598d354708102dcc4273c8a0ed2337ecf7879380a07e7d427c7f9d82e538002bd1442978402cdaf63debf5b40df902dae98dadc029f281474d190cddecef1b10653248a234151f91982912012669f74d0cfa1030ff37b152324e5b8346b3335a0aaeb63a0a2de2bca6a8d987d668defba89dc082196a922634ed88e065c669e526bb8815ee1be8ae2ad91d463bab75ee941d33cc5817b613c63cda943a4c07f600591b088a25d53fdee371cef596766823f4a518a583b1158243afe89700f0da76da46d0060f15d2444cefe7914c9a61e829c730eceb216288fee825f6b3b6298f6f6b6bd62e4c57a617a0aa10ea7a83aa6b6b0ed685b6a3d9e5b8fd14f56cdc18021b12253f3fd4915c19bd831a7920be55d969b2ac23359e2559da77de2373f06ca014ba2787d063cd07ee4944222b7762840eb94c688bec743fa8bdf7715c8fe29f104c2a0130fdc7364135613b83123b633379a8fce1bc3894605bd8d9d2b191b64fa0fe2a011b290d0b4449383e1f899dab47bfa689794f15673c4bba95eed10e14ca5aa95303a080808008858180800880808080080000538736c40ac5b5e9b88e4c1847e079cf28291ae31ba84ecff9742261982f74236dd141f7f3c78e4e55a9ecf804b98b650a10583dfd6375bb68d10cf82d0b5d8dc40473d6e151a742f6408a71ffe27f76215365edd4b124b755483abcb0022f51d382c6d9355a909c02816bdbabedafeaa7429ee6e0b45e482db827aa1dc31a86b8691d3de6f32e7e604ad9d7b81a43a1e301895188b2ba940ae3ad35dfa513cba3563da62af39afd69f074d0b42350d78231206ddb8457d07996516a5aa3d947f1c46f5df2c338c340e1580f42de00aba46091be75ea62c18c58dafab4513a8d2ddde4e32f31b52ca29cccee2eee927ea0d43a15efab967e488889edb2e64b545df15856131fc9260ec069e4a6d009ac5cc63fa8887fd5a81af5dd015c3d76cf8962304ae5779fbae10fa5ed0ba5b0e019ca6faed5423003ad431df4ad6483a3996cfbfccf3ebe5fe4b45526aece70c2676d47906d434cdfa7fde39bb474043b17a3a32e4639cd5d84e21af9e6f902b9dcc48346dffc62a420c6211123bee403e23ba531ad838c348278b11441c15e1f0a1c2a4c3c58a93cf2843473ca83be87e80d06c6bf9e1fdc26c7f50a32a1a5858bb575187ff6903ccc085204a43655b4642d77c6fbb6274b7f28ee924fac017d96bcdb5c905c09536547aa4f64b12d047c87b33e03bf6ff94c338d7e98cf9048661d42910f8ace26be4b5237f07f6aff0a28a50a238b90d6c10b958d4e8ff8d8e24b5ecf84c6ec35d5c25d53c37152425e16907fe66351732fac0f7ca32c0f325394260823cc95286c56ac7079dbf7c022e7116b6bbd5e8b47d1eb9b59be172fca603fce52acd834494d5af541acf3f624da1e63a019d3989c0cb73fc91c64ab23318b392f690e067335a0900281f46d2f2c65c955f450e9f0dd32315465d302d4d739883ec333ff0d7bad1ff12a0c6002a99ad89a47384293ac0d1d2e112bad751026219a27dcbf8580b57770c741b1097dcf0639c8bd0e46aa8deb6ccb5f4596dc8928b6ccd401a6e4300a17750f9a93b123a762950633fe0e507065c3443b147b14642b21823b57f468fc7d7579fe8bf6840190bbaa02017ec89aa14e2b8fbd951f66652b195ee13f0753ebb793b2ec33b60ccf12390fd4000001207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666f1202cfde7c339d494f19940b66c0bbbe5eec000124fec7775d59fd60729eef775fdbd17fccf34d2f199ad5329c9e00566ee3c31a03904e00bab113d7c75c1218350f8b162b1084c7cd8c0dcbbabe1c560249b8d289390e2000012c923b553d0c80d42cc7f970b484d1e67b15bdfd6ac48931a04027a4d8374929";
            let pczt_hex = hex::decode(hex_str).unwrap();
            let pczt = Pczt::parse(&pczt_hex).unwrap();

            let fingerprint = fingerprint.try_into().unwrap();

            let result = check_pczt(&fingerprint, &unified_fvk, &pczt);

            assert_eq!(true, result.is_ok());
        }
    }
    //TODO: add test for happy path
}
