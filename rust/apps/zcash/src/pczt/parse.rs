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
    ripemd::{Digest, Ripemd160},
    sha2::Sha256,
    zcash_address::{
        unified::{self, Encoding, Receiver},
        ToAddress, ZcashAddress,
    },
    zcash_keys::keys::UnifiedFullViewingKey,
    zcash_primitives::legacy::{Script, TransparentAddress},
    zcash_protocol::consensus::{MainNetwork, NetworkType},
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

        //none dummy output: not my output or the amount it not 0
        if !parsed_to.get_visible() || parsed_to.get_amount() != 0 {
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
        let memo = decode_memo(memo);
        Ok(ParsedTo::new(
            ua,
            zec_value,
            note.value().inner(),
            false,
            true,
            memo,
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
        let memo = decode_memo(memo);
        Ok(ParsedTo::new(
            ua,
            zec_value,
            note.value().inner(),
            true,
            true,
            memo,
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

fn decode_memo(memo_bytes: [u8; 512]) -> Option<String> {
    let first = memo_bytes[0];

    //decode as utf8.
    if first <= 0xF4 {
        let mut temp_memo = memo_bytes.to_vec();
        temp_memo.reverse();
        let mut result = vec![];
        temp_memo.iter().for_each(|v| {
            if *v != 0 {
                result.push(*v);
            }
        });
        result.reverse();

        return Some(String::from_utf8(result).unwrap());
    }

    if first == 0xF6 {
        let temp_memo = memo_bytes.to_vec();
        let result = temp_memo[1..].iter().find(|&&v| v != 0);
        match result {
            Some(v) => return Some(hex::encode(memo_bytes)),
            None => {
                return None;
            }
        }
    }

    Some(hex::encode(memo_bytes))
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
            hex::decode("2fac20755b7bb7c99d302b782ea36f62a1b0cfe8d7d4d09a58e8ba5da26f4578")
                .unwrap();
        let hex_str = "50435a5401000000058ace9cb502d5a09cc70c0100cedfa70185010001227a636173685f636c69656e745f6261636b656e643a70726f706f73616c5f696e666f148c3b8fdb49a840dbade76e60189071b1a6dfa701016020bd3f06f68e126fc9f3569e546f7663f07a1845e8ffcecd1f433f3e9aae480000000000c0843d1976a914ef2c6cc589f3ad61071f6ea56736831faf06e16c88ac0000010102b96236741e1bba262a0a052335f4b131f2d6fa241592f0e48551dd2fa5b722d81b290d0b4449383e1f899dab47bfa689794f15673c4bba95eed10e14ca5aa95305ac80808008858180800880808080080000000000000000000000fbc2f4300c01f0b7820d00e3347c8da4ee614674376cbc45359daa54f9b5493e010000000000000000000000000000000000000000000000000000000000000000028ff5d24a8f0d819e9d53cb91bb91beaf434060c3b83b9a904b1c8e4da4cb14a176dc9dadb2923007bd366cb07ec98d0cd04fc3d8b70d8acc506246100952a60683ccf672426de1bd0be6cb48186871921968ae0ec2de0f58b472ef30b6df8a95016a43618be77fa1da1316d16bb6cff3665a7689b88745df2fe12d80fac9fb529484cb7e290144052c160cd57750382ef12dd7fdd6f8956820e7d47637c815cf1901cb8716d31e08cf3a6d20ee24084da77776ef26b63840e628c5928bfe983051b1cc8c52bac2df712bf2d3a10100013e986e152d9b29d8fad41e7a89d8ae4c2e070c74d6cfaf6a3f7af3ee5fded01301da2a2f078e181b8db108e2553f5d53b190fed0ee2119876dc34a2aff62025f8a0144be60cd84eda0fd2330cb909d53dda02d844a073e025b14a7178e3d3e8a2e1be05b292659e016d3bc6ec6485e557b132125a46f03d091dbc9c332a2c5e012356bff82f727a593b40ac5774de9491d20c5974807c3d77e49c5187cbb5372a13701e6fce0cd0a2e52f9b42ba99342d343a374912d1556d12d06550b17c2e1dad411e69f534e0897b85342187acf47020d618541f3e097ab495062bfdf1947df47824a3deca30a7241547fc680ecfa9a5fd724dfa2cfb7f85b6045a99c47cc06b5605c0aa91d1523d3d2d4b5949833680ef75f7dc1912f6747984969ed71a70e48eca3b465902ef5a1c7ab0a5a3d7a129fcefbcb8210b956397de77a080405e553ceef220e090b664b23245be6386659c85d5546601b64634cbac69410c48c9d7662ff2a11bb2a1f25bf3122b227522d95aa7a928ce6273406c8342d0fde300d9fc4eb579b971a4a3f6b437d81f1ce1d2cc15a340725a4fcfb80dc645eecbbea93d44164ab363814887c06ca6451135d69e111a4d19612fa7672943fb776f0518face60e9fff0ae5363a828221280f1091a9f4b3362ebc1cdba976063c726d950eac15f5ac911fe8c2c3d9551772ded16283b030994f4feea4b46b00604b6bdd31e9e481fc261e8485af34af0913a0efdd6b42f6ecab78783ddde443c3c91ccdfaccfe7467bf2aff45afeec799184d883952d7d6a48e03687dabb949ff4345a8b2da77ebcf2316c3d3b8cc7797537a9612ca7b625462e50b3fee6074a1dc0d739cdf368661f329a16148cb7e332d09b56207e8c7fe578bad6faa76fe01c48a7760a99c2750262bcfa08ce34649c477bf2bfd8df6432a9ee9689b87c7c8766f044e33ca4ef6ba13491d12e4077639ee1286689cc4d487e6703100b9255965c837ed9d938f530024627574b7f616d854eab87f1f61ec3404db8b34c08890b44f544a688e6c7c5319cda336c41836b25b23955c6c972b89f0e3e9d01003676472d3a75deff5f25c19c009b66d0a4e1ec2ab113509bcce96217a0936250d1ca3b0709e749fed14783f8a2dda0884160b2d070ee600d0fc5d7f0fd9d79736c3375b912b8629a1def526a868ef7fa91af243f341e3a7341f8bd40f580c9abfe3511d656e87e0eb743e17eb6d68edab1a241512098b5b09b994f47aacb52cdeb1b809f702df4b8dba423b979fcf5cc95502e5a31f601393c8488f1269cd9449f2a63a5dc7691f157ce93d09fee18871727e00d4fabacfd64b0e80c8d432337b91aa5e0c4949ca754776324c7a82db1a6e7f03bb00a501dae6bd8e32794aef23129e2589f810040d3c3c1a26bb24ea95ad5f6feca941c2fb796859640819fd2fb11fe577c20fba3923021991b3280864064e9ac19dbb9ec5383dd163062dc4188f3a67954bc7c94d76c1328acbe5752441101310f7da04e20dcea377dd75ee16a1961dacb08a1cafae8f1b186eee30b4cc0f13fab9e2bdfd0fc41ef0e2280c8237ef4d96aa4a2966963a05f89d398bb714041ef5c7dca814a1e634446398df736fda20e99cf8a66ecd111befe159f09d321d58a079ba1a852a0a1a91966763139ff01edbdf84057f5b412f0111c58cac4a724c3a53aca7d3c14cede7558dcb1dea044ffaf778a6f1da11b622000000164a563a5626b4cd633c17f83fe1f5c47de79e1d205d207907a4b84548c3e935c62a6229fc7f1f0c9dc51c47e822abab229254128df34045c7119bd7d48f859ec40428e36c2b6d574201b6b98bbdd55b552da3605240a7e11836874bab0af7f39da51a98719eaaf79706035281884c56278aa9e709fa2efb113a6de6297a88695c63eaf9fc313b24a8e7780aa51ada64e21031e78aaa06dd1a857a03d50dad7119c93119b8d1acc15c3c4eb2dac6f8e76870fbe08983d1ce563fe8fc5603b77cff6835c8b1122e4c7aebe1d711cb294a3a54e2ee3780917778bdf13ef8526841cf28cd50537ff2e074a8fc1dd3f0d8a1a44e27f207ab8e9a61e657d137faeb46be1245e01e1ee1d14a48e26c406ee4569a3b16009f04d708090ccce53dfd3c1300680eb806a2258292367ea41ed6cb85212d3db13930f977288df36a9d07f980f6c816a67f168c9cf2cefb6341ad0518b6c023b07157869223bad02861069dd970147a50966c96d9d425273f2278fb96a08638becc73fdd51aaefd5e98d04ba4ad06a802f20714b253ef36507f0160c5be975d5526dde0b1d78e2efe25f62338eada5ecb3a3c663794e2d1ef462d40c43e17c9a70bf92c82904a6c20a726b2c932211161033cd11935bc367d45e937b7dcd6927b706ff4d0de2cdbf0668de3ff9b0d83799c9b2ceb4c98f42dda80902a31f7b23a2cfacf6cbcfc976fc9a9b15cf0b467a8e14f6fee8fc4a64f7a3d8cda51b4abebb4524136738ebb9ef3c1d59e4806adae667ada2cccd1e4f2ca540903450409e78e016e81c9f64d2becca76fa0646cff4c6cb73fd4a6d4fe186b6a49aaef01538668fbee949434cc48438f89f3f5c40d4094f4cfa79b4a15d305838b3ce15ac61dc4bf0edf4006385956e2f61be2ae9365ee250330dd298bc1810948478bc8c69def1d11f09afe4bf36f4dd353224d06642228fa82699b07dcf012b56ab3c2247aca3844b71bc8b12e58cd73a58b564bd64b6190e22346a64d099d0e5be6e63aec8db7d01a6e4300a17750f9a93b123a762950633fe0e507065c3443b147b14642b21823b57f468fc7d7579fe8bf68401a88f3c0102c2b3b9bae3a53626924868465d384e9ad6b89de0413f01429c08aeb3636703000001207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666f12028c3b8fdb49a840dbade76e60189071b10001200f953e0b4acf079cd0c3d8c9872d65f93c17741975fb372c74dc040795160c585829af38db84d84d7dda5505f31ac176ebc3bf47702e567d889005db27c2b6b01521174f3a79bd1cc0088c9e804ed3aaed7e54f4bb570b30624a0c0649d421417d21ea4bb0a6a9a88e7acbadc786b137dd914bb1dd4034f512548c56c5e39e01f100300c39677fd59390b75d74030537cbc4e9eaf92dcbee087654fd82bb308649d1f4608a53f1ee2ff291918815e5c4896e21419bdb10a487de2933b844f80e01949814bc4b94b60c7aaaf8149aeb1807012b27064a5228001baf8af4b6350f5257a78a061ef21db4da6c0c010001396bf5e21df27bd8aeef15a91a9f08ec40c043e6659a75f89fa9fe47e2abf33e013983c7e2185375c177b68a8b9326fc7a9038d76efd5e6e6880b2b61a6107135901bbc7c188fb6d32d3db0eb71c44a8edfdfaed2254b4384c2398fd863fd1091738173477a336b11b48c4c03a8b13cf7958a4d6bf131f1da1ad380d6830531c1406f535b7c0dbb3a4eb8a4a355d77337c16b93255c5a27dfabe72b238b11cf1a12901aea8b0f003d985e638bf4737423ad642a6a03579080aa7cf3c7853546159455b5c7f00101613d7dddcdca20c334f4d03864d5a25e6755a6b5c8d95b1fe1a4a11802696e31cd509c88e4aedadddcb95e2e31ee60ee8dcb9b05cfa8d468c35ed3e2857b64f31913da2ede97e0fbd2cd428d5de01c685c3c93589f57c25f699cb0077dca46d21a2cca6c1fb05af0d37da3c4c30f3bd66653fb445f60c0d0c172e0473b370263d27078e41cd2976579bb723bb4da32c6bec91aefc7db72f8b7aca2bf46aa3161b89b3627e807e388744d0c9d7fc2316a3edf074cc5c345674a201fcb15c79192886cd39fdbd2ca7181597a4d3d470a6e54421d5ea9702077652d436a8a5d5a20f4e5bddc4eba94a3449ff8d592de8a8054007da20fda54429c768858fb01a470fb2b6596334efbe035b8c1c775064f8b4caf9df828fb0c79cdd2545b4153cab3a2ab01e87d7ed729fec81d29a948023d2958061e50bd04bd460100519e4c0f91d7f034948365710ef5c3dda0d615b23f9470e559e1ddf323231030c6eb1c8bd0c672377060b2ae23509938f75a5fe76ff622c664a94e44d7e50aa9b12b15f31354d57449e75873b17cf298201bd4b8853f6c5551560992b2fe8b5708c44f48d097d10d15270dc151e3d070cf23e8a6e202c27212424ad8e82212dd57c2a3270124b951afd7510bb14b2d9c396e145982f14a2f265fcd15c52b9358c9cb6c6c606110da0facdb97ddffa7b0cb69b1637f98fae8cf5ffc792c033fcb41b2177b412a98a044c722f6ff888d19910508b5f3c3fb0e052488e681b270c5d246d2e5a0d4a448a1a9d6076ff27625b975c413a3df3a519da37d642d4577c421de7030b3616d3e34a4c5dc6f188ceffbefd8fafeffc1a02e1a9204d1b3253671f9b58f6330700fc8b2451ea29eca76860d6daa3fb95f873219436d9b76bb3ff341d1cfb3465edf4f8f7881ccd1e18ced943ab43b13bec88cdfb15b6f4429ce46599727d31bcdbc9d633b3ff7f15a011f2e9838983da763e87b53d35ede0f8e15866eef2062061465e69c49b63bc4ba2973efa98f6729af6694a4e65b09618dddc58783736f0694d7e96e7059824125d3b84c1448c5997b8eda75d0264c357ef1f0b64683f6caf0b2768f77b1a3f077e24c4a6dbbddcd72006b6b29989bb27d7ccc9d8053a411324b1ee597baed3218a1f82a0076a67372e26cbc53042b5803a2ccaee592f003a943396674ccac05a32280684b44ae0948fd3144bd45a92794da2b081ab0bce77c88f45c66ac603fbf54c3784e17f7ef7ba033dd8c4c8fd6cecc1abf48710f10937b1096aaa4912f2f98f7354c3727b619fcfb4b715f97729a1b181ed4022f8fe3db593e489f0973c45078c04523d5a9053969ab0366b97c9308b9f84bc1c48c5ea27c222d14a10534eabaa51bbfc8eb61d7ec66995f28e4a7f005c69c00001b09ef9a1ad301bc5b07a4d1923a669f83f2e0f94d07820c680223045cff758320000006be1092bf6e6f49fa897634e56b4f30416ee54bd69a2adfd9d133ccbe9497417eeac12cefc4ef57f0ab6a42e3ba4c476f52ae5429a32e795836fcb862a429c87c40447c4dec673c79151f028ada56d03c9aedbf511cda01c4b2e9a0f6dbbe8156be16bd866be7052397657257b54b2c0e16e6d6ef5bee72a0918e2cb65d893530c56033a996d3af76c74a71c430c682b2f6dee8952e76b3adaf71aa5c4a4e8caa7f09f518ff905649dd1213d00c073a1df1b8cd91192e78046eb375c6e940b301dee00619e100c9b00b01d64b2ee7796f7a6b6552467fefa87a995ea86ce808615a790c9ded5873e3ddc62a116641aa8ecd248674cf2a5c320d0c30f75287e9fb30ad45d22b9a67fce8579f587497afc18502f67da592356a733ea2ef6f8877dec729d524023995d0f200d25cbfcbdca2a592a7a880415f299ef918bbc000a913c105d2fe3aa78764abc505118eec06fea7153c1865f149a925cc0109663122d2b98f58562b35e734edc2cf697acb6fc85cf7de1185d0c2e7f3194dab3e36b7870bb019cdb23aa47fc32182a745884be8aa2af5dfe4521bad7e32ff5f91d26d4eacd4b3e33a91891c276b2d43ebbec04b76ff714466eaf705133c37b008a9378fd4f05fa60ef56bac26262a29351ee2eba725dc4f3be1b025d01328a3b5e97927a2d81d962c0666a55cb5d2070b91b42c06f69b493d62a54c3801ffd1fcb719f72c0444f4c21482324e820fc62440d008d7573f4d21efbf76a3d82be94469e2cea85b125748ab1027fa887538e24f90aa624afcbb43377d6b3a5d02170f67cbb7c34976e554d84986e068e3cacca648acf5de68602d3bb1dc09ecfe05a694f95d018269bac6fd5a6ce0e50d53cf0ce8be0828860f6d05338b2ca4e6526c1c0551eff7c22a23050a33ccc68daea11cf61edaee9bae774df06527caa2af43d72b84c195dd1cf37a5ba062d70485c451062a013ea658120259555efbcf6d37aaeabe70a9ce6711470ca3bfc80468c18a80ae70180db7f4db00190a6224e4ec8679786827ce2e5df576fef56920a19659ee68c75d61d04221c3f9f1265c04988f1fae48a15010001051462ea45eb3247c9fa619df8b7fc8cb96bf7ed8ad766eefe7bd84a87580a4200000001c6e1b53445acb210a2a6afe07aca639582e3092cae16bae1ea7255780191ea3903a88f3c01ae2935f1dfd8a24aed7c70df7de3a668eb7a49b1319880dde2bbd9031ae5d82f0001e5f04a732f0b3b8c60cedeaf48b94ad87b2021a0c78bb51917e7317d08260106";
        let pczt_hex = hex::decode(hex_str).unwrap();
        let pczt = Pczt::parse(&pczt_hex).unwrap();

        let fingerprint = fingerprint.try_into().unwrap();
        let ufvk = "uview10zf3gnxd08cne6g7ryh6lln79duzsayg0qxktvyc3l6uutfk0agmyclm5g82h5z0lqv4c2gzp0eu0qc0nxzurxhj4ympwn3gj5c3dc9g7ca4eh3q09fw9kka7qplzq0wnauekf45w9vs4g22khtq57sc8k6j6s70kz0rtqlyat6zsjkcqfrlm9quje8vzszs8y9mjvduf7j2vx329hk2v956g6svnhqswxfp3n760mw233w7ffgsja2szdhy5954hsfldalf28wvav0tctxwkmkgrk43tq2p7sqchzc6";
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

    #[test]
    fn test_decode_memo() {
        {
            let mut memo = [0u8; 512];
            memo[0] = 0xF6;
            let result = decode_memo(memo);
            println!("result: {:?}", result);
        }
        {
            let memo = hex::decode("74657374206b657973746f6e65206d656d6f0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000").unwrap().try_into().unwrap();
            let result = decode_memo(memo);
            println!("result: {:?}", result);
        }
    }

    #[test]
    fn test_decode_pczt_2() {
        let hex_str = "50435a5401000000058ace9cb502d5a09cc70c0100eae5a70185010001227a636173685f636c69656e745f6261636b656e643a70726f706f73616c5f696e666f148c3b8fdb49a840dbade76e60189071b1c2e5a701019a03f6b13fb77ed0555fe488da3699921bac7e03d21b461169e0e6cf8fba9d330000000000e091431976a914ef2c6cc589f3ad61071f6ea56736831faf06e16c88ac0000010102b96236741e1bba262a0a052335f4b131f2d6fa241592f0e48551dd2fa5b722d81b290d0b4449383e1f899dab47bfa689794f15673c4bba95eed10e14ca5aa95305ac80808008858180800880808080080000000000000000000000fbc2f4300c01f0b7820d00e3347c8da4ee614674376cbc45359daa54f9b5493e0100000000000000000000000000000000000000000000000000000000000000000258d9709079df9e036e6a34c66c2308ec2a7ae8ae062b966285268c29ec88eeb11b0063aeef9143d3e5c3d1a80b7b59c6732c750dc96539e794e36bc779b86a29c168b1777845e05cf141d25191532e9170105b396a8f989d90601c2615252f8c01ac9ee8f91404fad8a5d58396537511bb88263f922bd8d850929621339c04c594f0c280bf665161d2b97b118f711688d49e2c447d5a00df9c6d3890823061351001e0c3163c6b21a98c0bbe65c57696965cda66fac4198cb3c6dae64432f23b1764657d6d2da0a2cbfaf89c930100018df94357b3886c5b95a00c2c95defb13af982b9c1919979a947be6512009731b019871da2ff5c38b8d7fa97712fbe30b519cd5da9693e05b4522540659f1ce69ef01008dd48ae6bf31b1d26f9c72c17dbaa0bea35af1effd7ad55bf85b69a578e91832ce0310a818a46f3f97ac73230b43a52e52db1e7209a9202c222501e5887d346b94101e390e327350124bce2fdd7dc11fef667c38ec0c1be6fa2b3d64b84112018ce0ed880a8963057601ed691f863510ecf483c583d6ee2cc34f88bfff3680af4585f572319007aced6fb162a7651ff3f70c9493cbc704814174846a7478fdbb6577656e3b5806b22a3d28fdd9d41997e199ecbbe8318e408afa4f5a5afcedd3372155bf3d1760cd6a42bc78426842572737fc1baffcf43ab16a66af31c44c3ed957011a19d97c43ba6fc433709cd762bb70ac9fa71e9300c29c84ae8bbb718f420f8fe919589f9c84f4e0343bb76c3d539025873839627a9e16c27bdd5b15bb411e8c8b2e1a0a587a704e471780d547ce3785afdb86a747a26ab18093220f88571d043637cd2d26f79ce358a0a95444e3f9668bf94f51edd9ed6e6814f9fea61bbe5e443746a2fa632fb33a6a6f6209bceb200db848d97621407169d55a1f7bd7284f26316c345900cb15410cd068ae0c11063d8024c17431e09b3ce2666357865904752f8300a2ec40d930b054fca369b802fd3dd9cf6a7db8bfa52ff455e4ce6cf20a2abb0bebf9fdb3aeeeeae45d0b7b41b68491b0f7c75a11f74af7600191d093823ba2e8939b3dc05ce7973d923c39541bade04ae8fdbd1708fa60652cc43d34f52483b02edb9d3c5f4d3af6080987fa8f1f08f54f243b29c92496576035c96f1a23fc9ba38e3f19194a2b854e310f49289340d27c404367ee3184f76d7b74f9600a363f1de5a375728a1199569c78852b1e1e3dea47d7d58d2d5a2fb7940d055709823ca9edbef7e9ff18e6afa5a074bef578312a7366cd6f185644f86b2619190d5fe5a9c38988892bb8df8b27ce3a8c52737856eceff5bc9205522388715eac0d29b6f25771a38f2fd6c35292df760d8b674faa519407ae3d2c3ec3b8e10654336b56884bcec86f65358f52e715946abc6147aad7875834d2ff3e35740d7ce62acd084aea23065cb74c9db4a3440b8e51b6956cf0360e43e8b73a760d31216d3f4954f4ae50d5b51fb9a9144ed95599012af253f82229d1d27b3bbaa7c448b916517f9377084985101f79dedee7933ab783a271f31e9439e4ac827854065d3032b93f115b1c88ee2591283edfa11b6284104e7307ff036c0a5417c7f57dd5682683fc6ab0b92ac30989cadc01fa74dffe632c22e108b5f7f23906eb9e62b9bf046be8613ce57f14cf2d488724e8fd8c804e0b7b7b46537f18d061fb6b986a241b4f7975673dfeadc48c357771fec7377b1e760c5a1e430ff28a3f86440e97d52dfd6c1a35e45ee8b3ba36abd928c6a082ce014b1aa8674503e499008535992926fb111c2f5a7ad6b25cf2e8d89700606eaf97616a95ea1ba16282ff374447f41fc4fd745ecb7513fda1e43bff4206f1cff72418314c92a806eb179389c7994834c9b29a16b0b59577358fffd72281adaef823e7cfdf4e86597b0cccba6495f13e78bcfc39bec4bd79ce9cd4797b40ec43e4616c8b535528241e55eda9ccd32722013b85bc9cc5ef0efd694f3ba9312ef14f5058062bed5efea32c01005085da023e0000001a223282647ce65868668f2fec970a918cb2a09031ef3b396724b7fa868ee01dfe866f4ba1dbe4ca87c97ad544d0221ee155b3529684806e44db6fd118c7b0b7c4042b6b9aa254a8cb9cdf4f346a6576b45a724829e93eb56ded7c6b201ab285cd4d03fbff8717d58865b3918737f2dd15ed621b9fd1e11367e285e95565eb3eefb01aa6b22e0dc722da4230b5dd1f08ff5bdc5b8b9187121117925e1ddf33eebc1327bfc8945133b5a3fa7f692137665e109d9513dddd08369cf0cd8b4af2b829c89de5d5794a628e4544460751fcc3e2c996cf3dd82bf7c9b50062c28ee8143328bb9a452626fb4cdbc1aa714f4cab7e217b73df02ecc5d1326d5516eaa0d688c60f415eff55a9aa5ed80e4b7feb138dcd62edb3fe9ed945cfb910ce08ebf0f6278d5da4e020c18b1d1e6dbadf255e192d0589bf7f7ec005420454b4a5c635aa7d32e0f46393d53b9a288f437f986914b0510a7ecd3c5617da11db57d34b891ab40f791c373c2a083ad5ed8b3d494f6c2fe7b5ea5b5bb4c6f0c7296386130a2584bf4ce7aedb1d41625a30fac3ace085355cdf0071fc542e2f73e9cf054521d8578b2c8a3a3806b182fdb6053a8a523cd4da561783d725e6244ea7aa99d4acd8d03ea5e9dcf2fcdbbea1ead681bb5b6d2fec606b392b3021f93b60b1722c7b9ffddce1a529187a1cd0b339be1d01de9535fdfb584d79523491310b648ad9c1188807f8313f6d49d3eb5a48cc4adbc6f31453ad1d7f45e08f7bacc8b2621d4602de45340d31b4b70923f5dd03e26ef6dc3fd1e5bbbd03a9f5e34cdb06650cfe21b4c3abc20db26c87805f8f877bfdcbe16b2a03ec72ace7a03163024c88058bbd0cd927c254e769071c6a5f2b0b2519532a811be033ef972e5c98780ba450e2fab3decc170250a5adeffd2c5f3f65a20d084e7347e8ad436409decfcc139602f2c977ff2f644eebecd2f456f0ae106514da38dcb7be70110e4765e4dbc957095ac60c6bd33db5ae9025500a357008e4310e0c87a40fde0118fb9315a97e2d4b6997f5b5dc702cd62033cb91f8a163a140aaef1eafe78640d52e841d99a0bf42a0872a010001e127c18fed42f37fd3fb23cd195ce275526ef50302b966b8a433db30abc6e51c00000001e48493f14723aa75ec32c8c13bb29454ab3c618d6575d55a7ab6bfc3dcfc273db0047fdf10e31858f636e034465958172a08936adc9be1e2fa28e8df8f0ae08dc53848c4f52c0773327dd97a2ce283cdde231abb2c58aab3097f733ce4fb8f340ca67db74731f466eba77d7d2ac537fb1fd3ac798f529f727e924bc31a51821701f937a200856fa5fa542e2f88d468be18cf8305af26f9a48d0e2305ac778a98be8a71eb9373fb8b9020f79aad548ccd60fdc2ff00fc460aeba30160dfec81480c01eef2be08fbd3e1e8a64068dfe795f2bc465b73ccb60b0e390dcc3a5f9fcab1d68675386ca795bfd6d6d28c0100011f3458af949a563b37b5af4de816156a2946644a6bdfffc53b1dda56dc34352f016d578975fd6af35c1e4e76dafb44d588eb59fcfab93a727f50e281ba95e27a4c01b777182305401beaf4719a50aeced58ebf6588c5436be4e98cfb247ca9b4670fa8ab53104256a644d5d5db5f24daa3ce579f1605ca57e8a69956229d9c73b40fc8097d4c5d2c5a810ec0baf6ec0ff48fac85a44310230759af10d1dda3b5ad3901b6c6af91049db394ebcb43a6470d3a228a49a57cb200a027432fb93ac148cad35d14afbd000de73bf27e12e6914acd86dfc958960de72e4000c956a326065783be51fa1c15aa45414f1d97146f8862dccedab3101a755616360bbab987bb73f4385c906c13ec01c3b9ba117c8489e1516202a2d779e9544718fbd304cce37b8f0e8057ce3d31a6c63cb79646cf1af3b93bee2324bed20b142b830e63f4e409fb9b8bc55a1d700d8f8fcf7b10dbabb671bb74e93e765aa56ca4fd1b5ccf7a54432490219b181415169765204828293278a32c84fb4d917b3cfe1d0c31cc5af44b0c05b8890e3893c6d3f8209ed42aba86b0343bd95fdfc69db115ad3bfeac2a396c8e9d250206ceec434a48eefbca667d873bb5e3633754d876f206495d4309428afcc5fd0cd73f862fcd0b7c0ba1a057c4d67ed76a7f51bc19d8ed246b05dec4b3b5127c22d0497f822906f26ba1f8f0255d82f0850b5cebbe7fbe13fbd482bcaf5791b104d907bee83b878bbd7dd3b17dc4e01e86feb687866570054b7843a8fdfdc3a6316f9738fd7702d2cab555b5432e2b2f04ca2126cc233ebbf59985651c0bc6b00eca3160e4e1b2a969671c90e446d31919ec78782dd7f10a1464a473b4bf4a8030c9a686adc4f7c4f220486e2f66f382b8b3cd24ef57f2abf82e1225369ff45b33221a006f551b3f4a42c18d4c2d1de58986dd495acb708768f361173021d20d068efdfd4c2ad2f987c1f25deb9352aaa2f01bb29e3cf59d8e032e898cdff68e0ed3dd7d2f940cbdeeccebce860829eba0b5c6497563c09b87acf070ceedca900cc3b929ac75697918446b25fe968c3b83269b9f4a10cd50756a77c98ccee4f90d8245539d0f277b1db49559a428988bbb8a95eb3ae4d167c6030db1063f7ca612276d1782c3259421986323caaca7448138bf887a4a31d9ddab2787f8c8a89139840b0ebecfa0cae07654a8219b4ea9a78030f30760e6b1429790b3024b7e243db83e3b1849523eee31660d2c49dd1e9d579717b5115c88edd726308d7ef94c3863531e1f2aadf9b0fc60d57e5c916dea8568addcb13df2b0e9a0b93598e517209b9b80e250b0a8945de8b7a1a5a68bc552b3e3016ab7ae7432b1995e22db86247371b558986b05b6238dc011be5876db1ebb09031e235c8ce53073633018dc37e7d34c5277150bf1e3d7c1287a8c3d3a94a58d84514071b1ec250a971f5e3b0437fd07a3b492f78d13aceaa0049e7354979b14c2226fc58f633e1e4312607d3137ba5d5c603482ac4ec04730ebd2a09fae15a142c5e425a5edb263dd76097b1ea345408b665ce1cc1c2b1ef7e27f2f005fc1e8c17268a29e50698e4776c9bd29917c8575af618181289a8f74e51d7fcae9b9c424a9c7536e923a656ef1db18084d540a419fc2bdad282e759f1919bc9f173d40fb5451f8c15d84a29e2ae3a42c017cee774d3e533b5269b35b274a24acc91e1e787778c2759bbc11fdaa5dc99a3d000000395dd461f8cdd44264e834175a5b1a0e5d23edb93c6e400dfc520b27a1b12f1434a95976168b7e3e3e76409a44ab98fb2e754b9fdfb88a7eb8a1be4a0c14fb83c4042e8400d46d582f83ebf1cc30622efe09702c3a58b00e60466a3808ba5e9990d78851d654168296ecedbc8b7b37df78b0aa9b2c08b9decbae868c082b002aef73ac73640a842e095baa7d58e2745e665c37d33d82b8e765603023458cccf4fa523d71941144be0cd9626caf057a2c83036f35f9b013028371fd9139fb4a5c7f156e35e1dad67b93b3aa2762d1e55687c6d7b38464628ce2d0a932d254d0c6d00bb4e859cc0b38f70946e51725daeff7d86fe3d058382f9c514891cb9470355f16bc5c9b4ba94ee071b6bc7a46c850c2b1120aa18d9527493716940ca9baff82bad3fe0d81a3e0bad63e5dccfb57b9e20a461fcf5a0cfbb4ff9575ac0d3c23124d239198a3105d5f897def65f584e1b1e7cb96703a242a1cf51839a60ffe738bd11f7930bbbef4fb5a1480db00812067063a8545dfd78d5c1943d45d6665458411d350b24cbc85f4e9a3b140a344ae0e14787838547b3ecde886ef470c4d12561e1ca8e6e9a24cb7353dc3055eba608528b012d04828f5e38a25ab478842f2b081bd4236de5f4a7d1332968589a72092193cae569ffb56a0e5f4c3bedf60caf4665f3dfc67bed7db9f8976b9ba75a818bf28eb00a1b220fdcadb4ddda8dc39b40ceb9ad2df37c52981433ab159da901d5eb3239d159cff393e931a2c82bd572da4ebfddfe6f9a7ba56e9be62da08d0fca327d40f8d2791c9dc3594ac941889180cb9cf7058aaa8d4f4f5bec3135920117773719cad754d326bce0cf1346351d0c426fdce833b16fb8b900f0bab684ff69b95fc0ea3995b46ec59f657a37b17a3613b60aa7c5069c40c79a74a6bdeb9d7bc3f0a51d49214a37e88a69fe44edbeac917e8e2805f43c6b07f9694bec6fc60ecd9d0f02b368642f8d325782e9f7a7e0b8ccb8c065d021419a59f6f30fbe614cdbdc6bf046e01a6e4300a17750f9a93b123a762950633fe0e507065c3443b147b14642b21823b57f468fc7d7579fe8bf68401c89c4201655e49b76964cf785f1dd759e877a2c178e0fe44c4a5b91688a88b0fae3d2a63000001207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666f12028c3b8fdb49a840dbade76e60189071b10001b70676bb468a16b2afaefc8e0833a0bea250e7516b2bb00c56238ced8f937e3103c89c4201ae2935f1dfd8a24aed7c70df7de3a668eb7a49b1319880dde2bbd9031ae5d82f00019a8b09ad6dc2799bbe383047484ceef04d8d48dfd0a08567d0d94bb16c90a62e";
        let pczt_hex = hex::decode(hex_str).unwrap();
        let pczt = Pczt::parse(&pczt_hex).unwrap();
        let fingerprint =
            hex::decode("2fac20755b7bb7c99d302b782ea36f62a1b0cfe8d7d4d09a58e8ba5da26f4578")
                .unwrap();
        let ufvk = "uview10zf3gnxd08cne6g7ryh6lln79duzsayg0qxktvyc3l6uutfk0agmyclm5g82h5z0lqv4c2gzp0eu0qc0nxzurxhj4ympwn3gj5c3dc9g7ca4eh3q09fw9kka7qplzq0wnauekf45w9vs4g22khtq57sc8k6j6s70kz0rtqlyat6zsjkcqfrlm9quje8vzszs8y9mjvduf7j2vx329hk2v956g6svnhqswxfp3n760mw233w7ffgsja2szdhy5954hsfldalf28wvav0tctxwkmkgrk43tq2p7sqchzc6";

        let fingerprint = fingerprint.try_into().unwrap();
        let unified_fvk = UnifiedFullViewingKey::decode(&MAIN_NETWORK, ufvk).unwrap();

        let result = parse_pczt(&fingerprint, &unified_fvk, &pczt);
        println!("{:?}", result);
    }
}
