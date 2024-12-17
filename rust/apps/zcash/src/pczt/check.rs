use super::*;

use orchard::{keys::FullViewingKey, value::ValueSum};
use zcash_vendor::{
    pczt::{self, roles::verifier::Verifier, Pczt},
    ripemd::Ripemd160,
    sha2::{Digest, Sha256},
    transparent::{self, address::TransparentAddress, keys::AccountPubKey},
    zcash_address::{ToAddress, ZcashAddress},
    zcash_protocol::consensus::{self, NetworkConstants},
    zip32,
};

pub fn check_pczt<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    ufvk: &UnifiedFullViewingKey,
    pczt: &Pczt,
) -> Result<(), ZcashError> {
    let xpub = ufvk.transparent().ok_or(ZcashError::InvalidDataError(
        "transparent xpub is not present".to_string(),
    ))?;
    let orchard = ufvk.orchard().ok_or(ZcashError::InvalidDataError(
        "orchard fvk is not present".to_string(),
    ))?;
    Verifier::new(pczt.clone())
        .with_orchard(|bundle| {
            check_orchard(params, &seed_fingerprint, account_index, &orchard, bundle)
                .map_err(pczt::roles::verifier::OrchardError::Custom)
        })
        .map_err(|e| ZcashError::InvalidDataError(alloc::format!("{:?}", e)))?
        .with_transparent(|bundle| {
            check_transparent(params, seed_fingerprint, account_index, &xpub, bundle)
                .map_err(pczt::roles::verifier::TransparentError::Custom)
        })
        .map_err(|e| ZcashError::InvalidDataError(alloc::format!("{:?}", e)))?;
    Ok(())
}

fn check_transparent<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    xpub: &AccountPubKey,
    bundle: &transparent::pczt::Bundle,
) -> Result<(), ZcashError> {
    bundle.inputs().iter().try_for_each(|input| {
        check_transparent_input(params, seed_fingerprint, account_index, xpub, input)?;
        Ok::<_, ZcashError>(())
    })?;
    bundle.outputs().iter().try_for_each(|output| {
        check_transparent_output(params, seed_fingerprint, account_index, xpub, output)?;
        Ok::<_, ZcashError>(())
    })?;
    Ok(())
}

fn check_transparent_input<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    xpub: &AccountPubKey,
    input: &transparent::pczt::Input,
) -> Result<(), ZcashError> {
    let script = input.script_pubkey().clone();
    match script.address() {
        Some(TransparentAddress::PublicKeyHash(hash)) => {
            let pubkey = input.bip32_derivation().keys().find(|pubkey| {
                return hash[..] == Ripemd160::digest(Sha256::digest(pubkey))[..];
            });
            match pubkey {
                Some(pubkey) => {
                    match input.bip32_derivation().get(pubkey) {
                        Some(bip32_derivation) => {
                            if seed_fingerprint == bip32_derivation.seed_fingerprint() {
                                //verify public key
                                let target = xpub
                                    .derive_pubkey_at_bip32_path(
                                        params,
                                        account_index,
                                        &bip32_derivation.derivation_path(),
                                    )
                                    .map_err(|_| {
                                        ZcashError::InvalidPczt(
                                            "transparent input bip32 derivation path invalid"
                                                .to_string(),
                                        )
                                    })?;
                                if &target.serialize() != pubkey {
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

fn check_transparent_output<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    xpub: &AccountPubKey,
    output: &transparent::pczt::Output,
) -> Result<(), ZcashError> {
    let script = output.script_pubkey().clone();
    match script.address() {
        Some(TransparentAddress::PublicKeyHash(hash)) => {
            //check user_address and script_pubkey
            match output.user_address() {
                Some(user_address) => {
                    let ta =
                        ZcashAddress::from_transparent_p2pkh(params.network_type(), hash).encode();
                    if user_address != &ta {
                        return Err(ZcashError::InvalidPczt(
                            "transparent output user_address mismatch".to_string(),
                        ));
                    }
                }
                None => {
                    return Err(ZcashError::InvalidPczt(
                        "transparent output user_address is None".to_string(),
                    ))
                }
            }

            let pubkey = output
                .bip32_derivation()
                .keys()
                .find(|pubkey| hash[..] == Ripemd160::digest(Sha256::digest(pubkey))[..]);
            match pubkey {
                Some(pubkey) => {
                    match output.bip32_derivation().get(pubkey) {
                        Some(bip32_derivation) => {
                            if seed_fingerprint == bip32_derivation.seed_fingerprint() {
                                //verify public key
                                let target = xpub
                                    .derive_pubkey_at_bip32_path(
                                        params,
                                        account_index,
                                        bip32_derivation.derivation_path(),
                                    )
                                    .map_err(|_| {
                                        ZcashError::InvalidPczt(
                                            "transparent input bip32 derivation path invalid"
                                                .to_string(),
                                        )
                                    })?;
                                if &target.serialize() != pubkey {
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

fn check_orchard<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    fvk: &FullViewingKey,
    bundle: &orchard::pczt::Bundle,
) -> Result<(), ZcashError> {
    bundle.actions().iter().try_for_each(|action| {
        check_action(params, seed_fingerprint, account_index, fvk, action)?;
        Ok::<_, ZcashError>(())
    })?;

    // At this point, we know that every `value` field in the Orchard bundle is present.
    // Check that `value_sum` is correct so we can use it for fee calculations later.
    let calculated_value_balance = bundle
        .actions()
        .iter()
        .map(|action| {
            action.spend().value().expect("present") - action.output().value().expect("present")
        })
        .sum::<Result<ValueSum, _>>();

    match calculated_value_balance {
        Ok(value_balance) if &value_balance == bundle.value_sum() => Ok(()),
        _ => Err(ZcashError::InvalidPczt(
            "invalid Orchard bundle value balance".into(),
        )),
    }
}

fn check_action<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    fvk: &FullViewingKey,
    action: &orchard::pczt::Action,
) -> Result<(), ZcashError> {
    // Check `cv_net` first so we know that the `value` fields for both the spend and the
    // output are present and correct.
    action.verify_cv_net().map_err(|e| {
        ZcashError::InvalidPczt(alloc::format!("invalid cv_net in Orchard action: {:?}", e))
    })?;

    check_action_spend(
        params,
        seed_fingerprint,
        account_index,
        fvk,
        &action.spend(),
    )?;
    check_action_output(action)
}

// check spend nullifier
fn check_action_spend<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    fvk: &FullViewingKey,
    spend: &orchard::pczt::Spend,
) -> Result<(), ZcashError> {
    // We can only verify the `nullifier` and `rk` fields of a spend if we know its FVK.
    let can_verify_nf_rk = match (spend.value(), spend.fvk(), spend.zip32_derivation()) {
        // If the spend is marked as matching the accounts's FVK, verify with it.
        (_, _, Some(zip32_derivation))
            if zip32_derivation.seed_fingerprint() == seed_fingerprint
                && zip32_derivation.derivation_path()
                    == &[
                        zip32::ChildIndex::hardened(32),
                        zip32::ChildIndex::hardened(params.network_type().coin_type()),
                        account_index.into(),
                    ] =>
        {
            Some(Some(fvk))
        }
        // Dummy notes use randomly-generated FVKs, so if one is already present then
        // don't validate using the account's FVK.
        (Some(value), Some(_), _) if value.inner() == 0 => Some(None),
        // Don't verify `nullifier` or `rk` for any other spends.
        _ => None,
    };

    if let Some(expected_fvk) = can_verify_nf_rk {
        spend.verify_nullifier(expected_fvk).map_err(|e| {
            ZcashError::InvalidPczt(alloc::format!("invalid Orchard action nullifier: {:?}", e))
        })?;
        spend.verify_rk(expected_fvk).map_err(|e| {
            ZcashError::InvalidPczt(alloc::format!("invalid Orchard action rk: {:?}", e))
        })?;
    }

    Ok(())
}

//check output cmx
fn check_action_output(action: &orchard::pczt::Action) -> Result<(), ZcashError> {
    action
        .output()
        .verify_note_commitment(action.spend())
        .map_err(|e| {
            ZcashError::InvalidPczt(alloc::format!("invalid Orchard action cmx: {:?}", e))
        })?;

    // TODO: Currently the "can decrypt output" check is performed implicitly by
    // `parse_orchard_output`. If desired, that code could be called from here and
    // checked; then in `parse_orchard_output` it would never error if reached.

    Ok(())
}

#[cfg(test)]
mod tests {
    use zcash_vendor::{pczt::Pczt, zcash_protocol::consensus::MAIN_NETWORK};

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

            let hex_str = "50435a5401000000058ace9cb502d5a09cc70c0100bb8aa80185010001227a636173685f636c69656e745f6261636b656e643a70726f706f73616c5f696e666f144951eeff9ccf4eb390ff94a60aa5673d938aa80102deac440c678cd3c6225cd68cba6824f6e015782ade325fbf54ba941d237bc4590000000000a08d061976a9149517c77b7fcc08e66122dccb6ee6713eb7712d2b88ac000001010325d5b38ff44c279744d83c97f139878dc6837a9df74604982f326e87840898422fac20755b7bb7c99d302b782ea36f62a1b0cfe8d7d4d09a58e8ba5da26f457805ac808080088581808008808080800800000000000000761d6f1f36bcc7ce07687877241b6bdcf9e695fe761105db842c3abc6b94b0910000000000c0c393071976a9149517c77b7fcc08e66122dccb6ee6713eb7712d2b88ac000001010325d5b38ff44c279744d83c97f139878dc6837a9df74604982f326e87840898422fac20755b7bb7c99d302b782ea36f62a1b0cfe8d7d4d09a58e8ba5da26f457805ac80808008858180800880808080080000000000000000000000fbc2f4300c01f0b7820d00e3347c8da4ee614674376cbc45359daa54f9b5493e01000000000000000000000000000000000000000000000000000000000000000002ee9c8fa97a77873f3cbb7aedf49910c92157e64b7f56c8cec418a80a0869550f5766df60ba68a809413793f9b3c672f8cb5c1af314860e28c2225bfea1a2492b7c60d3be040a302efa85e86da0e73f0d1861f635463ff334217da9afdc21432501b255f0914ef002c8132512a675b529cfca7af70b2b042a55cc7c2fc887ae09a03fdd18e557d727c5df51d8edb7151222bdb78f310c2fdcfa441c386a75b84e020112e9d5c2ac40e053e5fc9e7658319b1a6e27f2a6a718c7add3588a02b81821d28e459801f5145511ee573c010001cc242cb29c4e1c3efe25ed9f4309db8766c02b4d45526894aa757427f7b0d91b01a9a28f87a2e5ec670db10948c329384cf83ef9717dc91413f963a22a41ece20101c0fb1b819949309e84abf9894efdc74f1260c00cc353d98fd8bbc69c166d861077d72794d754cec4f01e67194e5443fbb8395f0befad08ea836688d2f493f415312a3c53a5ba2675fd7cea2708737faa67d3e3b4dfe5d3fd9554e64c08f5780c01829783df0908c2958f23626891625304936d8bce1b370ce91d6c5756c54abd5d8f6274f50e67accf13eba4849906870a02c1b4e9ba6d3900dd675924a530a42e7326c1ab269c6b63c759e5bfbf15b3f99ecc45c0d4e6d34e59be11f6b7b7140c436c37ea379e3002393137519e53bd17e3595858385b1e8e53b67622f5223eca3112d612251d965cbb9a39fc35771c6b067f2c43eb8c2712f40095865ea294795a1c48cf26145c1c1ed783735faa7a22f67533dd72fee305ca4bd140315f48540680834302ed7139e617d09ff1738ce6e11cab3af038699ecb73acdb104af06cb30f70413c9f6884c2b13eb281713cc06113c2cac7c23a22c150de15cbc8c40ac9f709d3337d185f12cbf4017373439d12cfb4fd3728c969c6eb350f3d12c23abf4fc14c3aa5671ceec76f16e426ba6ef1b8a01bab9a44d1a6c7c77427c3ff9cc3bcb1d239bb29a3ab687cb7c9dc0f026ccc5abf0022c51bbfe05be9f44a6af21136ecfc31f59694c125361d8fb80f10095b49be361c576a5c80d1c0c2d80d7b31ece6fd39dd6dc44532b6c171b3830ad47390462a3dc0a1a85bcac61d7f516bb7cba9eb2015cf9a9866e3aa86f9b4272a54283926fa759d5e3b207da73094beb0dcf8bb20f02ec8138b461e9a0dd4e368a6c3d06d8c10830093b6d4ce985326dcffe5a238773c427f9aee0bb360d60cc26212fc7a4a82bfdc30bd9ddbcbd6986226744a158ea90f7c600dd8bf573e4ab04c3cc2336bb4662168aff17675ddabb5b509cc0034e53ddf1391e0310040ca58b6a74c4d3d51fdf117b6c92bf22e31ef30102a25310737cc580f8a0e294f252052faa13fc1baf102013bf953fd16b976c292cf3819014bdbf11f959d3a34f03e6eff4f2cc70b640424171241954abddf943fef38457f13ffa5b916245d9bfe07934b8688156e429d5e11a4f4aea9378b8c62aa290e1bee108f82234eef19c02b93004633484973de7a2ab0410c7e010c64a4e11479d39670f3c6b01994d20b3b6b56a1a4050eea805b87cbca2ae0ba9d1ef21b2b1f288e1af2c458401379edade2ad3d7491021ff5bc8e000869842953f355630b1773d5bf8587774afea222409bdbf7b057e378e7afd12f8c27e4496ae8d5b11d7e3a362da7037b926a2c82b91e569e27f5f5c68096afae99a3e1316e667b341518e819e02c2707fda419d765d37610e3c1c5d55ca2a47e5fa9d1df7c9282a63a45b87f34baa4d4e2c4197c78adfc55c6b17a26c9981646006297fd513f8418105a1aa40fc86a7c6554854b036a644a9bf3c720f310152eb24da84f9ccd61f411185ac8e5074ae82ece9b6695de8b083a615cbc1f58e0b12101f0698a83f2e43ec9b8dbd5a2b7f90061ac3b68f31c050af2042ecd0701e6c5b108fac2c21cc62ee543bf69b44b77ca2c40c6b9270f50739940e62a6f5a17101ffa605bd62a660a01686a0038ff3b456069fcdc2b7aa743eea3482311dda5b286ee67ed9587e4c83a000000bba118aa5dbe0ac3b9d39ff3c6f55130ca466a1f16e51e60330109c03e13a21e027eaf5bbc5839a2fba75bd99b34edf4f2b3e1ad8c61cde2cc9220c9b9b1c126c404415c4f086e496c6e5dabe375ff15a510ac028465748e34a96c3c6558e8d1cdb5df5847931d345b58e10dc47204e15690fd05e0954cf0bc6a80ef69f7eb3e66aef866c3b2d3294cdddb136a836c6666354dd828bae6143ccc08eec15ad02ea2771d35e40ffad5b3b0e473fe228e8c8693c9fdee1950a570ec3b4363a53b464a008b9d4ed4d4b0153ed0d83122e81a0f9df32505a88f8e2543fb7493f7b776d3df18904aedffb9a0087e337f0390f625341fcc0a594b5a791f974ff373888b1b547e066f3f54904909d9d2071613657b9f691959b3aa2890997dadfe50cde42498237f2625aabbe824c24cd67312ec89f97b5a19c348c5d9d06e05ea11f66161d7b99674144ca670d404280f883af8095e68cdc66d3bdb50d15816ffcb17f0450feb334c7a2cdb2cc1c11fb15e6cbf9ed85f4a4c60e277e990474f2e947f14b2733523caa1794285f647b4e01f78df87314110ec7b9d43407537a9578abbc1916f49b25c576f30dbd0aa2ca20fcfd539340684b7ce68e6e1849425b374e8e79bee150e80b2a1c3af86c8314933a6a9d1011ef6b952e97a289c744abfd68c3b057119280f7eb618b1577d9b5492e7e8aa3bf27e6242ee184a961e8645b630efde7b5bd035c244fbc825b6191a21a8d80b8436acf3878135f10b0f45e30b31c10d0e9f0b68d311a23bfbc59f795211fbbeb2a59ce0ae75dd1d2bacb49a9e3a32e4821249658a4d358b9ba99d9dbdf915948d367a35db07ac05d2b4290e4bcb03506d9d1a8fc6de6e0345f5896b020044613a16003caa3a6d28ad6813b821d9a7d48f96465c2b50aa1e651026892e1f806b8a8c9bbbf17181256f818ceae21b8d4dbfcbfe0f71760a65dbcaec1a86eaee1c66390c5f9bb369755beaf47fa49da9b0ad0528d9d01cd090e45bd8fcb19a9f9fa4d16826cb6d0114b0cac3e14c4e1a4a4fd22c98563b9cbe859c67768a39f67abd5dbe55d3d16ff58e5c45b21d8c573f54b901c0b4980701963cc44bd800f3928fb8f033697ecf95f25295b80366edf0c3b411e92a7a856f00000001207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666f11024951eeff9ccf4eb390ff94a60aa5673d016be75ccb54636978429a73dacae41525aed0e2a08e197893b99264fea2e5de2a6870514937f1ef8eff2206d5bb2497b374f0748a8104743e80c8f8c6fa1d3622c1890d035c500e9c97fa2823b818b39b07e4db325508835e4b09a98bda4c922a143b3cf5ef4ddfc60ef47140872ef580e0d281bffb64cd4fb6e190e82f7565260123be0c9fdd62fc6f815385ae01145935671aaeceb00218d27ea74843b968ad24f3306c6d4955094123d88a5151a6e5210d493086bfe77e21c507bb547c6377150164c59cb53ee13cad93a1378dd17bc36971db5fcf1045bceda83eee160b260f35870e0120ad0651e5259ca7010001e5f91ad5d25a5b3192feac62094bc40d586ec841c0770e4db7e6d10454c5412d018fe5a9218a8c0a006bdea9ae83006bc0ae46c6427fb98ecff557ec9e9b592e6a01792e33cb4ee913d3fa3e390a93d1ba1e9bfd626d79fd2a30eb7ba21422b4a7365008f037b7be17825e9dbc3769a06d6ecef44b5ab6473b7bcaceaff1b9fd0c1f4c5bcaf0e467f666c5388caeef37b4281e24089cbebfe13b83d6dd194187ee0f01deebdde30639b48b124bd387fe9744c7eb493daa5f5d0d445546ef8ac4dd1e05de43a1790e8a78c6cccae80a4ce7ae29b6e0529a3309394a17722b6896c1edb3086194aa0e711c647a5af2edb6c2ac7903a52155aeeead5b723de473382ce40721eb6d9302e20f73d49e3be05a2557bfa2c94ab03e2b28415cfe70059fc6bae5a17dc6e23752ba333da277143ec08857e82a470061512fb811c9fe74659989cf3b442900069b0f731b8cd2c452820fee0c8f4d4a5430cd9118668cb230929df0f51e820e16140196d235da867e6650ea2ed6d88c3c61e03d15e89ec09e636b617e2171133000bd9a569a7feb5cf90a9f58b5e6be24f849d7135bf9087b380bf349c54e5b0c83dad1603c15a03fbe98cd0f5b6f1e838e1a533bb645b7257e0000300d06e706c16ce8acdcf346c37eac806cb44ec956e8379ad5d893f060dc229eb26b05fb0b2caf0be37f0078ddb0b41fa0b51875f03ff256884828d0ed416b75e2c5d2fe3f63f2a32ecbd7b8a82dc16c496d9dd7559020db04097de47362ae2aa25e98ad2e120c3bea2e207d9e2b987bf5633102cc2c04b243467fd659d48f6ab3a0f75b2e3fda2138e3058aaffb6977617ba15f265796e43aeb06f9f1f92aee74a0704d1abde7ed7b656c0d0231429013d7ba6f98bc3ea05fa6fa5162ca49091bca67723174675ec91a9679621aca521a226289c7cfd45af293eb39d67dd767684e4ca8155f81efbcb97b554d8aad779539c0b9390892d99bcb66ead30fc13737c4b0eb2dc557c9de473daf065838dcdef769f45768a4a63f1b3aa36d8b4844182dce2d0286fad0dae42ab0346e8ef2d73cb2bf7262564105edc6e9d03bbc665dd879b33d52ff617a961d1867592017c5a20c8a76ee7b6bf937df9bc71014b9522524261625e9d01ae49449e9fc5777e904655b8e8bf82bf16665ffd21ec2af00299fb30d836fd76dd06e3a1349f8cd6facd1d1a3c3ebf2d561391b929a2ae0aee4d8bd069aa30e3c99562f6050a0cf1801a938a15ca36de0ff33b143565b10d84e48e13f0f50d879e18fb10d3b1936eef73c2c1a8fd2b1908d156236443876d865def12a3fe3cca737f24db9e96e1dafdf4314d56de3d22dedd2888589a9f8b67dea0e3b3ab46dbde8868b3ece15147aad698304ea182d699a89e0b6dfd60ffd75b51423e3cf10506e7fdcb860aee0030b2cdff26795f58a1a7c727d317a7ae0265518203fa2c21a44ce66772d0732145e3d147e8083a545f1134aed6d86a1c8b13abc2a117fc41049109449edbb802b0b2b3d3a3f164329a804d1689f95eb43e786b434e1d3ef523de60f880b40f0ab757d7fbe59193b841e7ace893a2b1576c9780c087e56fc9357da0bcf685b43dbe15816c07aeab5c81857070f3d2df5653da5e90351eb31e5c583d1902a7f9f33780f6172a5b33c80e8cc5d4a4f1882d4359c5c31018cc4c87994bcd8de2a2ba88222ee6bf76c6c006b6c8b8ab2d6be7c854dbb881b000000cb122829a3a96f655544087f6e437db61d519c6c9fad6fd7a5377b6f70ea2c2648a2f5e4cb4e926069dc954ef6e8b1253f836c39d6c0d2438087cc3670d8bf05c40458318155c014a2cc67d54a3c78d886513ec69b894cf3e6d57b42b5f01e85fe1164be973e75d993f062effc72fcf36ad660b23a7c04dae2b953486a3ecc8a64cb0c5302d86107880d104395681a4a4eeca524a1592edcecafdc4b2a75e23bf78ce6869b747266f8b8f32f71ca50dabc9107ab70e8fab2599099fdab3fd0c9372339c98f65963c5b4300e2bf1e9c4a73b0b7ee662ba0dfaa8281d63c9033dc065db336867b3680beda24f10f5f262bec49b8f13f272f7d5af3a8e1cc6c3d04ded77c063a910144ee5dc856d6dc61616bfa75e45ac9ef54fead78e2ca9c2568274e899da8d20623ea08204b03438f2afe823b022c002e3571458ff5c1369f04433ded4817bddba317ab95ac377db122c7ba0647b4cf7a87cc6d467590335675b6c4ab41b29fb7997ad438d54d6db0299c5824122e0abc661da696369dd01ea50e619b5b2b37258031126f6c11b50967541b038344f71ddc63325254ec984dbaaf82c646a11d163d3f02d4697850b14b64cc1f9eaf8f2bdeb8ed89b85bf412c6cfc09291163ad43d5093d67dbca2b78973db1de57781a9bd13e4f3f7909b48906278416eb7ee2623f79742b67662ee5effcbe9afd2df75a5eecd127a31d04ef1d587333a5ea584855c1748c029b83badb4870f19b473441bc970a2a0dadd549531ed8d92bc9ed06f0f9a200ede83ab9bb144a6bd7df956620fb686f2114ca8ecfc76d04171b1853369c4d750ace41914bd4ba7efb3fa9034306ca47dedb79a10df39f832a2b1d00e57c6730b2ad7f2a2aade341d4583314de769f7e11c4090157185745d1b75505281176c6ebbd85a48be625f18f002819f193602513517c81d6c661b20125da3ac7f3d239ceab4341b3ee982e5b75cd153ebb69648e17ae296ddb154c753620de6c0bfdcc2cc453cdcdb0ca7ba99a199017313667df7e72e98ada38bc09fbe2fa6af72921027de6905bbef4601cb5965a35b794e40a0ed55688cef0e010001396e7126e2ecdb4c13f3b8c6f856ac9e45618cba45414953a9de5b6562c641cd0000000001cca601864b2ec1be8921fd7c56e500aed8b9c73b90167f4ba2ce9ed221f3863503c0b4980701ae2935f1dfd8a24aed7c70df7de3a668eb7a49b1319880dde2bbd9031ae5d82f0001368e5e517fa6e3aaee12dc4d2531d0b0868aaadc1e30f7de5b6103d1c4d86520";
            let pczt_hex = hex::decode(hex_str).unwrap();
            let pczt = Pczt::parse(&pczt_hex).unwrap();

            let fingerprint = fingerprint.try_into().unwrap();

            let result = check_pczt(
                &MAIN_NETWORK,
                &fingerprint,
                zip32::AccountId::ZERO,
                &unified_fvk,
                &pczt,
            );

            println!("result: {:?}", result);

            assert_eq!(true, result.is_ok());
        }
    }
    //TODO: add test for happy path
}
