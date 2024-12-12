use alloc::{
    format,
    string::{String, ToString},
    vec,
};
use zcash_vendor::{
    orchard::{
        keys::{EphemeralKeyBytes, FullViewingKey, OutgoingViewingKey},
        note::{Memo, Note, Nullifier, Rho},
        note_encryption::OrchardDomain,
        note_ext::{try_output_recovery_with_ovk, ENC_CIPHERTEXT_SIZE, OUT_CIPHERTEXT_SIZE},
        value::ValueCommitment,
        Address,
    },
    pczt::{self, Pczt},
    ripemd::{self, Digest, Ripemd160},
    sha2::Sha256,
    zcash_address::{
        unified::{self, Encoding, Receiver},
        ToAddress, ZcashAddress,
    },
    zcash_keys::keys::UnifiedFullViewingKey,
    zcash_primitives::legacy::{Script, TransparentAddress},
    zcash_protocol::consensus::NetworkType,
};

use crate::errors::ZcashError;

use super::structs::{ParsedFrom, ParsedOrchard, ParsedPczt, ParsedTo, ParsedTransparent};

const ZEC_DIVIDER: u32 = 1_000_000_00;

fn format_zec_value(value: f64) -> String {
    let mut zec_value = format!("{:.8}", value as f64 / ZEC_DIVIDER as f64);
    let zec_value = zec_value
        .trim_end_matches('0')
        .trim_end_matches('.')
        .to_string();
    format!("{} ZEC", zec_value)
}

pub fn decode_output_enc_ciphertext(
    ovk: &OutgoingViewingKey,
    rho: &Rho,
    epk: &[u8; 32],
    cmx: &[u8; 32],
    cv: &[u8; 32],
    enc_ciphertext: &[u8; ENC_CIPHERTEXT_SIZE],
    out_ciphertext: &[u8; OUT_CIPHERTEXT_SIZE],
) -> Result<Option<(Note, Address, Memo)>, ZcashError> {
    let domain = OrchardDomain::new(rho.clone());

    let ephemeral_key = EphemeralKeyBytes::from(epk.clone());

    let cv = ValueCommitment::from_bytes(cv)
        .into_option()
        .ok_or(ZcashError::InvalidPczt("cv is not valid".to_string()))?;

    let result = try_output_recovery_with_ovk(
        &domain,
        &ovk,
        &ephemeral_key,
        cmx,
        &cv,
        enc_ciphertext,
        out_ciphertext,
    );

    Ok(result)
}

pub fn parse_pczt(
    seed_fingerprint: &[u8; 32],
    ufvk: &UnifiedFullViewingKey,
    pczt: &Pczt,
) -> Result<ParsedPczt, ZcashError> {
    let parsed_orchard = parse_orchard(
        seed_fingerprint,
        ufvk.orchard().ok_or(ZcashError::InvalidDataError(
            "orchard is not present in ufvk".to_string(),
        ))?,
        &pczt.orchard(),
    )?;

    let parsed_transparent = parse_transparent(seed_fingerprint, &pczt.transparent())?;

    let mut my_output_value = 0;

    parsed_orchard.clone().and_then(|orchard| {
        my_output_value = orchard
            .get_to()
            .iter()
            .filter(|v| v.get_visible() && !v.get_is_change())
            .fold(0, |acc, to| acc + to.get_amount());
        Some(())
    });

    parsed_transparent.clone().and_then(|transparent| {
        my_output_value += transparent
            .get_to()
            .iter()
            .filter(|v| v.get_visible() && !v.get_is_change())
            .fold(0, |acc, to| acc + to.get_amount());
        Some(())
    });

    let total_transfer_value = format_zec_value(my_output_value as f64);

    Ok(ParsedPczt::new(
        parsed_transparent,
        parsed_orchard,
        total_transfer_value,
    ))
}

fn parse_transparent(
    seed_fingerprint: &[u8; 32],
    transparent: &pczt::transparent::Bundle,
) -> Result<Option<ParsedTransparent>, ZcashError> {
    let mut parsed_transparent = ParsedTransparent::new(vec![], vec![]);
    transparent.inputs().iter().try_for_each(|input| {
        let parsed_from = parse_transparent_input(seed_fingerprint, &input)?;
        parsed_transparent.add_from(parsed_from);
        Ok(())
    })?;
    transparent.outputs().iter().try_for_each(|output| {
        let parsed_to = parse_transparent_output(seed_fingerprint, &output)?;
        parsed_transparent.add_to(parsed_to);
        Ok(())
    })?;
    if parsed_transparent.get_from().is_empty() && parsed_transparent.get_to().is_empty() {
        Ok(None)
    } else {
        Ok(Some(parsed_transparent))
    }
}

fn parse_transparent_input(
    seed_fingerprint: &[u8; 32],
    input: &pczt::transparent::Input,
) -> Result<ParsedFrom, ZcashError> {
    let script = Script(input.script_pubkey().clone());
    match script.address() {
        Some(TransparentAddress::PublicKeyHash(hash)) => {
            let pubkey = input
                .bip32_derivation()
                .keys()
                .find(|pubkey| hash[..] == Ripemd160::digest(Sha256::digest(pubkey))[..]);
            let ta = ZcashAddress::from_transparent_p2pkh(NetworkType::Main, hash).encode();

            let zec_value = format_zec_value(input.value().clone() as f64);

            let is_mine = match pubkey {
                Some(pubkey) => match input.bip32_derivation().get(pubkey) {
                    //pubkey validation is checked on transaction checking part
                    Some(bip32_derivation) => {
                        seed_fingerprint == &bip32_derivation.seed_fingerprint
                    }
                    None => false,
                },
                None => false,
            };
            Ok(ParsedFrom::new(
                ta,
                zec_value,
                input.value().clone(),
                is_mine,
            ))
        }
        _ => {
            return Err(ZcashError::InvalidPczt(
                "transparent input script pubkey mismatch".to_string(),
            ));
        }
    }
}

fn parse_transparent_output(
    seed_fingerprint: &[u8; 32],
    output: &pczt::transparent::Output,
) -> Result<ParsedTo, ZcashError> {
    let script = Script(output.script_pubkey().clone());
    match script.address() {
        Some(TransparentAddress::PublicKeyHash(hash)) => {
            let pubkey = output
                .bip32_derivation()
                .keys()
                .find(|pubkey| hash[..] == Ripemd160::digest(Sha256::digest(pubkey))[..]);
            let ta = ZcashAddress::from_transparent_p2pkh(NetworkType::Main, hash).encode();

            let zec_value = format_zec_value(output.value().clone() as f64);
            let is_change = match pubkey {
                Some(pubkey) => match output.bip32_derivation().get(pubkey) {
                    Some(bip32_derivation) => {
                        seed_fingerprint == &bip32_derivation.seed_fingerprint
                    }
                    None => false,
                },
                None => false,
            };
            Ok(ParsedTo::new(
                ta,
                zec_value,
                output.value().clone(),
                is_change,
                true,
                None,
            ))
        }
        _ => {
            return Err(ZcashError::InvalidPczt(
                "transparent output script pubkey mismatch".to_string(),
            ));
        }
    }
}

fn parse_orchard(
    seed_fingerprint: &[u8; 32],
    fvk: &FullViewingKey,
    orchard: &pczt::orchard::Bundle,
) -> Result<Option<ParsedOrchard>, ZcashError> {
    let mut parsed_orchard = ParsedOrchard::new(vec![], vec![]);
    orchard.actions().iter().try_for_each(|action| {
        let spend = action.spend().clone();

        if let Some(value) = spend.value() {
            //dummy spend maybe
            if *value != 0 {
                let parsed_from = parse_orchard_spend(seed_fingerprint, &spend)?;
                parsed_orchard.add_from(parsed_from);
            }
        }
        let parsed_to = parse_orchard_output(fvk, &action)?;
        if parsed_to.get_amount() != 0 {
            parsed_orchard.add_to(parsed_to);
        }

        Ok(())
    })?;

    if parsed_orchard.get_from().is_empty() && parsed_orchard.get_to().is_empty() {
        Ok(None)
    } else {
        Ok(Some(parsed_orchard))
    }
}

fn parse_orchard_spend(
    seed_fingerprint: &[u8; 32],
    spend: &pczt::orchard::Spend,
) -> Result<ParsedFrom, ZcashError> {
    let recipient = spend.recipient().clone().ok_or(ZcashError::InvalidPczt(
        "recipient is not present".to_string(),
    ))?;
    let value = spend
        .value()
        .clone()
        .ok_or(ZcashError::InvalidPczt("value is not present".to_string()))?;
    let zec_value = format_zec_value(value as f64);

    let ua = unified::Address(vec![Receiver::Orchard(recipient)]).encode(&NetworkType::Main);

    let zip32_derivation = spend.zip32_derivation().clone();

    let is_mine = match zip32_derivation {
        Some(zip32_derivation) => seed_fingerprint == &zip32_derivation.seed_fingerprint,
        None => false,
    };

    Ok(ParsedFrom::new(ua, zec_value, value, is_mine))
}

fn parse_orchard_output(
    fvk: &FullViewingKey,
    action: &pczt::orchard::Action,
) -> Result<ParsedTo, ZcashError> {
    let output = action.output().clone();
    let epk = output.ephemeral_key().clone();
    let cmx = output.cmx().clone();
    let nf_old = action.spend().nullifier().clone();
    let rho = Rho::from_nf_old(Nullifier::from_bytes(&nf_old).into_option().ok_or(
        ZcashError::InvalidPczt("nullifier is not valid".to_string()),
    )?);
    let cv = action.cv_net().clone();
    let enc_ciphertext = output.enc_ciphertext().clone().try_into().unwrap();
    let out_ciphertext = output.out_ciphertext().clone().try_into().unwrap();
    let external_ovk = fvk.to_ovk(zcash_vendor::zip32::Scope::External).clone();
    let internal_ovk = fvk.to_ovk(zcash_vendor::zip32::Scope::Internal).clone();
    //it is external output
    if let Some((note, address, memo)) = decode_output_enc_ciphertext(
        &external_ovk,
        &rho,
        &epk,
        &cmx,
        &cv,
        &enc_ciphertext,
        &out_ciphertext,
    )? {
        let zec_value = format_zec_value(note.value().inner() as f64);
        let ua = unified::Address(vec![Receiver::Orchard(address.to_raw_address_bytes())])
            .encode(&NetworkType::Main);
        //empty memo
        let mut bytes = [0u8; 512];
        bytes[0] = 0xF6;
        let mut memo_str = None;
        if memo == bytes {
            memo_str = Some("".to_string());
        } else {
            memo_str = Some(String::from_utf8(memo.to_vec()).unwrap_or(hex::encode(memo)));
        }
        Ok(ParsedTo::new(
            ua,
            zec_value,
            note.value().inner(),
            false,
            true,
            memo_str,
        ))
    } else if let Some((note, address, memo)) = decode_output_enc_ciphertext(
        &internal_ovk,
        &rho,
        &epk,
        &cmx,
        &cv,
        &enc_ciphertext,
        &out_ciphertext,
    )? {
        let zec_value = format_zec_value(note.value().inner() as f64);
        let ua = unified::Address(vec![Receiver::Orchard(address.to_raw_address_bytes())])
            .encode(&NetworkType::Main);
        //empty memo
        let mut bytes = [0u8; 512];
        bytes[0] = 0xF6;
        let mut memo_str = None;
        if memo == bytes {
            memo_str = Some("".to_string());
        } else {
            memo_str = Some(String::from_utf8(memo.to_vec()).unwrap_or(hex::encode(memo)));
        }
        Ok(ParsedTo::new(
            ua,
            zec_value,
            note.value().inner(),
            true,
            true,
            memo_str,
        ))
    } else {
        Ok(ParsedTo::new(
            "Unknown Address".to_string(),
            "Unknown Value".to_string(),
            0,
            false,
            false,
            None,
        ))
    }
}

#[cfg(test)]
mod tests {
    use zcash_vendor::zcash_protocol::consensus::MAIN_NETWORK;

    use super::*;
    extern crate std;
    use std::println;
    #[test]
    fn test_parse() {
        let fingerprint =
            hex::decode("1b290d0b4449383e1f899dab47bfa689794f15673c4bba95eed10e14ca5aa953")
                .unwrap();
        let hex_str = "50435a5401000000058ace9cb502d5a09cc70c010092dda70185010001227a636173685f636c69656e745f6261636b656e643a70726f706f73616c5f696e666f140cd612b51afd462f8b19e3edcaf79157eadca7010000000000fbc2f4300c01f0b7820d00e3347c8da4ee614674376cbc45359daa54f9b5493e010000000000000000000000000000000000000000000000000000000000000000027a36861ad6a58872c2905a35028f34dff096d5ce4249acd1ab0bfffdfe3b5eb5b6dc07e492c92d702a85fac123a1847a6f651e00f8905d0e437ed33fa9def4187d066726af90a706e876083793efbc6a3e0fbcec36c8b5bd3ebb6422de550901000114b0cac3e14c4e1a4a4fd22c98563b9cbe859c67768a39f67abd5dbe55d3d16ff58e5c45b21d8c573f54b901e0f5f201012e37833cbe331fe961f4adccead166294519a354f2ee72022fbffafd660424000126639c70fd66ea727c38e7bbcbc185f69eab410d41c2cf1ffa799deeb0cae9ce017d6d7535f6e3eade13042662d05eefa91d1f4757fcd0b4b844554e77366f4b36a7068e15c2f06aaa11f6bcfd2e9e36231530069f7fb99a2771050689f24f332b427fdd05d1d5f3219230a1406f0955e3ddc0ef5ce2c7f160d797f3e54ef5782001d9a3ab170d54720a916ed11694a363087edd33e80c4fba5b99ab4e268844c626e095c6147f90aa00e564894d523d591803c3f70e5f3cf0b78df11c30c692eb53b61d570fc4e15c65718ddb408e62add513bf87284e72c4142fbe75e92e18d344fb61b11769c751e83f12141f7ba68eb0c19844adfaacccf642609a1f2773cce422f7c31f04e8bd109baefd7739b9c48a570c5e4d613d9f2b5a0621b61a4e7eda788c1b1bf44905b00a67e2474ffdf203cc9bd32e593a741ed1dadf5e0a2c3cf02a9d82393100c0297533ab75c22b113db4693d8cf61b21570c1f21edf89477a2e7bf740cb1e779f3088da9819a590c5474384ed04d724606a76ddf80036e2ac52a8fa7057aa0667da7c4ef890f2eab3c5892e39be29b020793a5d3f0f6f3942981c26a38007ab14443f4b375536142b59f222f5337bf4c671bd6f57aa3fd5e050726dc3c53fd99c2e7156a1092c95bce18d9e2a5cc568a96556430c1785958b26fb22b12eec609dbfb205dd0c6675c576ea301bf0aa397c4cd383f6ace064434e405b5093c1d81175c618596a01ecadad95d75fee2eb4390fef14a084ac781b79e487f0f1dbb6c1523d9a86316044757219311a23d5ad6e71d34b8f6a2832494d2b98d3d7597692c9b1a0b7d4bc8e93db1c90c8331e9709ac9973ca3fbf38b2aa955382b0c104faa2b1148b1b627090aa939e321b82edcb0ce70aa4eee298997556b4f358449e7d9f0bcb971dcd97024ace98fe01e7c6d96d6fb235fcbcfef949d71f12912278dfeae9949f887b70ae81e084f8897a5054627acef3efd01c8b29793d522ca2ced953b7fb95e3ba986333da9e69cd355223c929731094b6c2174c7638d2e60040850b766b126a2b4843fcdfdffa5d5cab3f53bc860a3bef68958b5f066177097b04c2aa045a0deffcaca41c5ac92e694466578f5909e72bb78d33310f705cc2dcaa338b312112db04b435a706d63244dd435238f0aa1e9e1598d354708102dcc4273c8a0ed2337ecf7879380a07e7d427c7f9d82e538002bd1442978402cdaf63debf5b40df902dae98dadc029f281474d190cddecef1b10653248a234151f91982912012669f74d0cfa1030ff37b152324e5b8346b3335a0aaeb63a0a2de2bca6a8d987d668defba89dc082196a922634ed88e065c669e526bb8815ee1be8ae2ad91d463bab75ee941d33cc5817b613c63cda943a4c07f600591b088a25d53fdee371cef596766823f4a518a583b1158243afe89700f0da76da46d0060f15d2444cefe7914c9a61e829c730eceb216288fee825f6b3b6298f6f6b6bd62e4c57a617a0aa10ea7a83aa6b6b0ed685b6a3d9e5b8fd14f56cdc18021b12253f3fd4915c19bd831a7920be55d969b2ac23359e2559da77de2373f06ca014ba2787d063cd07ee4944222b7762840eb94c688bec743fa8bdf7715c8fe29f104c2a01f8694190e10a867f0da343c7a83e9e60f5cadc0717ff8c85995c4e0e83282118012fac20755b7bb7c99d302b782ea36f62a1b0cfe8d7d4d09a58e8ba5da26f457803a080808008858180800880808080080000696012fdb6b2495af31ed47860aa8fc997122eaf26a13dcd808dcb8d0a54b40471f2e41a9f8348cd1fd3be79a5ffa71323a0d33ccaab119e18b216992914319cc404940b9229535858fdf66aa59af9f5b6364bb4a748348b57bb1da739068003b16e132fdb03bbc52524a6a65a5722f9b551091fc6b1d2659a99f82d389ef0da3e1a979b581e554ee04705ab87960f087e04fa96060116e2256e83a422b6df29124404cb3d40c3e0afb61721119381ddeb8c5b78b5bda89343a5b3b089d8e96c9a483896df4478d03344ec6018788c955eecaf64717b2373043bc2b9c49d1b372af6549ec298c2fa8a913cc5e5528e40943ed76a0542a9fb49735fd5342b4becb40354ff9cf41b275eaada8a964e2b4d89414e6e5890cc4da6b44001c7981be6ace45327ae853bc4dd05a228bd21f81596864650ff6b41b8a8a5d32844d92aead66100ac8febc76e20ca14b1bc9773b63e8845a87df9f87248aab299e7f2c8cc9d7b589f88c7cf37b33e7b5db0957cd348fe108f4aa41c986a293ff264cf01db6ed6835f2bf4c5e2699b793f6a5f3224fd263b4938c28ed7458f799dfc4e54be208951f95688e12a1befc08bdf82c0fa9c36840dbfcbcb7c022d623234da44d910d2c6ac2b686904e642acccd1eda42bddd28331d8730a3740967b66d98f3824227eeecb1860c80687ed0ca0b3a48c58ede68e8d013ad133c2562ef7d113261294b76b12a4619933db7c6cbdce7ad685e641299c2df68737250fc2c964d462affbc827b00689b45e1501786fdcd50ecda725cbdb48a247df0ba5b6bbd7bc531c6c3ece0196780c1ad311dad1dacb6a1517ffb5500cf2ecb8b91edb7708b1752509efd5c4255b6a936a9a03fe37f4a7b9ec46cd9da5e11edc0c372328909cac9abf23b0e1064b50692bdbe5a8071e40609196cf13ee2fb08993a9ae8ddade7f7327562a7b47a280ea2e6948d16246b7f616b2b14951116b0b0f42a39ba3230573b6d624461ca130dfcf799e4aba8172129a85a01b5e7d890114b0cac3e14c4e1a4a4fd22c98563b9cbe859c67768a39f67abd5dbe55d3d16ff58e5c45b21d8c573f54b901c0d9f1010177409c3ae24bf4db43dda6ba04f72a99d11505526d84802873ad59b3b8d287a7000001207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666f12020cd612b51afd462f8b19e3edcaf791570001137c5cef4a07f2bc3762e7e962bf82740a4d26d97753bd1b0db5c4280b4fd630da4201211bb5f31298b9a033ce84ef217ac75bc835c957ac96029289120885a61c4a7fedab4ed8012c3f7925723a18967a7ef096be28366c3de89951508f4712ca523393c31c84a40ec67bc69baef166158bed91d1d6bf0678e74ba1e7f2978301e80861184531333aa5a10a444f41fa833fbbb67015de5eae8dd531394d49de3498b8b352d27e199bc8a6fa0631f019c4739d244ec17590dd70acca4e54ec4a2b012263f4a7aec0cf522215c76d681ff5002ad5d217b4c2c434186d3977e7679b101b21aa9c9c9c1555c3dc0e0100014b2f50d4e164e9254f3190c9c37e109b9e9bcf0ee57f2e8ab4a2e1abbf28f02601b1628a1d867e1bb6b9cfafedf4898ced8fa2ab963176fc34d08447028900030401c47481ed30dad03f670fee1dcd6e8cf4378679036593d41794c0395bcab9a837e387339fa6069cd9d12fe7b32c7c5746219ab6f0c73008cd221cd2a4b930f3171e41774a55f8089935dd78048d0146d7cfedfb96e65c953f2b8302c08df3f11a01bceeacdd093d60a239d9014c05d6e49abb83111bbc965b83a89fb713332bb86b1d6b412a350d8a54bac24e2733b5192a1665973f38e014210e61ec54bf0befd250569c1a2e8a9c6e0f703163fcfba8818f731b368cb214e5d3f83ffd4ef426693720426c1db709c60f853f30e7fb18c8bf3948c19037d35cd013bf8643b056a4d347c20a1ef14352ddd78ab14ef920a3c7bf17cf5cbdff0b301f0a32966d3c1b1d06e3bf22233c826eefa5d89b12bd36baadd239f7a4fddd819a16d81de45dcdf23d14a02d23dc9c38b6c4ba67cd91ffc9cc93db37dec85b80811ced339b03ec5f4fa04808764add0418e6ae411175c9ee7ca73a26f091e2b5ee94a2a1264139c86110dc057ce2080f7e128f0ae5ccf502571cd520159c86a0d7b328ac7955e6bf2a744c12b4f46981709872795e33255c038b3711f178b6500feeaab0391084602edb7c0cbd366b69b96dcde967fa6a0838bfc4e5384c454090975d2bee96ab05681ec62be6cb9925d608485ed3626b7b56752a3acdac11324dde367fcd7b3c016ab65c1e026d41d8f1a62eb36b4f1fc1176e93dd4068d53d052d82373841bb855c6b1b0f9c4705998f0962eb0ab377e246cbcf9d5a3a4198db223f75efd25a6b9f3f5a37e76fa549fd1a9c6ecf4db44708094966c9f5c896a9a9d7ca9bd18b70a9fc532ab6f41a6f8378bcbdf1b0f0c0278b2906aa87dadd21c5c3505e62ab9e6387ff02ecd2b2a230e1e49e4658752f372e4c9df26a44a58fcd49febeb7a69814baa9128fb5d984da723eb50223cd0611e414e9eea939dd0a1c07cffb2685f2dcf9bc0b6f8e98796a93c0161b2a02feba7c74f8c21167249fa87b38547b4c8521f2cb23f515ebeabf04c680fc22cb3ad6d1e962c6ce19c2be523d3e3aee5905d68efa0a24ea8eac8faab9f9439a49a1c92441fa93cb4356d7f2cc2160c611021a9a0f38875faef114d7c8b91e4d889ca1e8fb30e085162b8438a169eb092a73bae6630794c201205989d30ca191cb606d5e8af3a45d42ba8242759497ae550f72ccd13097295ac14003c6fbcd83d8003e5df4d9666cd61c1cdfee604af4132b424ced1e75c769a432774c3194e6346502ba2aa7af3459f26d4126ef334d6aa72181410336e33061dcf280c0534633a7dcb5fbbc68588e4a446dba01fe040d9f9ea5a70ebab435a82cbb055d9a077c0450db603cfd370217544b817ca16d3b23e19f8b395308a3263a1341bdbc7040a46e823a9a20a7fc0b8ce92bc3a934d6d6eaa29433b42ee98877e7ad61d2595956edac7a1abbb479956df26b73499c4fe172e61d25a5625fc4905e77400046f06e5ef55c890864e27c4a3acd6e4684d4304c74580cb0fee1b5b1660271272604c590a34cbcd99ad8e9335ea3b9b441e1cd2744043f34334c6cb005c9a5dd3fb77890fb5e6121e1ab9b58dc22726a8e9580823ec1230158e1cdc2c9c3c490c7348fb711603e7bed09b9ca176507c52abfb1d634fcb21c012fac20755b7bb7c99d302b782ea36f62a1b0cfe8d7d4d09a58e8ba5da26f457803a080808008858180800880808080080000d35e9d8df2f008031ffb303c359fda181afdcd5ab8f41f4270c2f6284ae90632823ebbf0b00c1e073c1b37d0a3728a485b2baa37b4e00f76725de68f8231c582c404ccb974183a96870bd8ffddc60344f3631bd363e71b1741bcd705496e1f387dadd35b5a7e6d7816761ff1a003bfbec4ed5e40af51da0a3bde8a1a90c5cef175852893190f2c1b70a6db940c45c22a2a73c6112cc6a826efae664d42966a02e23784b967a6d751c6f786dda11715e4f1b710f3839d8aeea697f4369e850757618c0ee3727c2adc331aace77ff9c3a0ee63a6d9dda01c3ca81dda3f5d948e440edf1d838987fc398d5f7d7162b7bf328c4f0bb1268f67e2048df8cb25f675dceba91d087640950c24cdd89e716a17c915a1b42a50f1dc8709590cd9fa17417382a2b5edb02adee0e89d7bfe16e3260f6631197689e1e79f4c168a8b877a1f4594eeeb5eb2b7870cc3d7a59dd345ef5cb057ca7c43641e8ddd079d6e65d65a995ea29cc637c8e6b490149e2fc3679b65b26d5ab044f74d30fbe031fef077f42ff4391c266fd9efa22eaa7a8ffd9ab8a577ba228b3c29cd516677538a43c8ff09a2858d696df9386299bb187ab3c7cf9eefbadbbc56d9bee7615a96383614a773e6a4d70b840de129b418ad1bb4029bafa24afbdecb51fb101e00fd277464a9fee6039787c23107d86c0fb1e3a20ec832ab71c7fd26919f4c70aaeedc9f52d9bcba18a62eb298fa2d50286f1d1bfb78e8fa369d03ea693a050c65ac6e002840c90b2dde1ed91e5f7147397d4dc4b56134112c6b34f6a7869d7ac4b9fbfbae98616d42727a8329c76687ea035238693fd839821793bd7ba467e928f9085a7987de60ba7633a86296f6fd4af4d212280dc3d6db872058b83af52ed42aec39e47ef00331d4ac2cc750626365e12eca667ddc78b1c583e8bf11e1f313bdc26592005532a94e22ee261ca220910d523097b5030d1f36e4e30cde18f1bd75655706100349e2e7c91e9da062d88e7bcb475f526da589dcdc3242ef0199a66f4d2212d15f6b9ddf1d738b41226e3954130fee9e1654409d3eb52a82e0e18f62bb13bab4f37db92901904e01f623ee8ddd67ca82e30c2c06cd9fb08ae1f79e111efd808f0ca7eec26060604a000001207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666fd80100d50175316677787573387a63687136343772346a61717833327537736a32777479706c7376756e336a6563613272633477353575387572736e337132773438377634756d6b686738776c727465726e686e3064776761707471683537327566306b357078736d717a75726633336671677868353375616a6b7472683377733735677766786c65677537686e676839726c6c743635636e7568393675766d65336366346b663366357938737936396a39356e6c7370726176396d3977336d7a6c336b757663306d6874716d3074746c71716a6d78673535750113c85eea64b911e9493618159b24d8d1f202ac3799c4f77aa1e78767dd27bc2a03904e00a5352047e2bbef44cefe58d8b14b140dbd8ea88bf69c0c1d19fe7dec2219841300012544bbd98ed5bc19a4ef6af5014b1424fd4fd2101118b596ae9c4c90e876921b";
        let pczt_hex = hex::decode(hex_str).unwrap();
        let pczt = Pczt::parse(&pczt_hex).unwrap();

        let fingerprint = fingerprint.try_into().unwrap();
        let ufvk = "uview1s2e0495jzhdarezq4h4xsunfk4jrq7gzg22tjjmkzpd28wgse4ejm6k7yfg8weanaghmwsvc69clwxz9f9z2hwaz4gegmna0plqrf05zkeue0nevnxzm557rwdkjzl4pl4hp4q9ywyszyjca8jl54730aymaprt8t0kxj8ays4fs682kf7prj9p24dnlcgqtnd2vnskkm7u8cwz8n0ce7yrwx967cyp6dhkc2wqprt84q0jmwzwnufyxe3j0758a9zgk9ssrrnywzkwfhu6ap6cgx3jkxs3un53n75s3";
        let unified_fvk = UnifiedFullViewingKey::decode(&MAIN_NETWORK, ufvk).unwrap();

        let result = parse_pczt(&fingerprint, &unified_fvk, &pczt);
        println!("{:?}", result);
    }

    #[test]
    fn test_format_zec_value() {
        let value = 10000;
        let zec_value = format_zec_value(value as f64);
        println!("zec_value: {}", zec_value);
    }
}
