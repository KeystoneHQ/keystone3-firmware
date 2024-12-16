use alloc::{
    format,
    string::{String, ToString},
    vec,
};
use zcash_vendor::{
    orchard::{
        self,
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
    let parsed_orchard = parse_orchard(seed_fingerprint, ufvk, &pczt.orchard())?;

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
    ufvk: &UnifiedFullViewingKey,
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
        let parsed_to = parse_orchard_output(ufvk, &action)?;

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
    ufvk: &UnifiedFullViewingKey,
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
    let fvk = ufvk.orchard().ok_or(ZcashError::InvalidDataError(
        "orchard is not present in ufvk".to_string(),
    ))?;

    let external_ovk = fvk.to_ovk(zcash_vendor::zip32::Scope::External).clone();
    let internal_ovk = fvk.to_ovk(zcash_vendor::zip32::Scope::Internal).clone();
    let transparent_internal_ovk = ufvk
        .transparent()
        .map(|k| orchard::keys::OutgoingViewingKey::from(k.internal_ovk().as_bytes()));

    let decode_output = |vk: OutgoingViewingKey| {
        if let Ok(Some((note, address, memo))) = decode_output_enc_ciphertext(
            &vk,
            &rho,
            &epk,
            &cmx,
            &cv,
            &enc_ciphertext,
            &out_ciphertext,
        ) {
            let zec_value = format_zec_value(note.value().inner() as f64);
            let ua = unified::Address::try_from_items(vec![Receiver::Orchard(
                address.to_raw_address_bytes(),
            )])
            .unwrap()
            .encode(&NetworkType::Main);
            let memo = decode_memo(memo);
            Some(ParsedTo::new(
                ua,
                zec_value,
                note.value().inner(),
                false,
                true,
                memo,
            ))
        } else {
            None
        }
    };

    let mut keys = vec![external_ovk, internal_ovk];

    if let Some(ovk) = transparent_internal_ovk {
        keys.push(ovk);
    }

    let mut parsed_to = None;

    for key in keys {
        let output = decode_output(key.clone());
        match output {
            Some(output) => {
                parsed_to = Some(output);
                break;
            }
            None => continue,
        }
    }

    Ok(parsed_to.unwrap_or(ParsedTo::new(
        "Unknown Address".to_string(),
        "Unknown Value".to_string(),
        0,
        false,
        false,
        None,
    )))
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
        let hex_str = "50435a5401000000058ace9cb502d5a09cc70c0100c6eea70185010001227a636173685f636c69656e745f6261636b656e643a70726f706f73616c5f696e666f140cd612b51afd462f8b19e3edcaf791579eeea70101cc5a56912078cf9352b33c84818d1c78f23ea6b3f7119c2756fa52ccfdf28ac30000000000c0c393071976a9149517c77b7fcc08e66122dccb6ee6713eb7712d2b88ac000001010325d5b38ff44c279744d83c97f139878dc6837a9df74604982f326e87840898422fac20755b7bb7c99d302b782ea36f62a1b0cfe8d7d4d09a58e8ba5da26f457805ac80808008858180800880808080080000000000000000000000fbc2f4300c01f0b7820d00e3347c8da4ee614674376cbc45359daa54f9b5493e0100000000000000000000000000000000000000000000000000000000000000000250d6b6180802fad46b531a41f8048a995dbdf97777e2bd9edfc2b9aeda6c07a9d8e871bdd1bd33d6156adfc6ba0e2dcf1b708c66ef342a181de1f2e47da4771a0a6dc894a6c882180a27f825febded2c33a12127935079e1989a9d087726823c016bf171d0ccb2dffc3c3b5deca68d85c69d1556894f04d639c21f0e95a8a0ce3385cf3636669f4fcf486cf5ec2b8d373ab2160652104721fa38ac23317d17160801abb7447651a41c23da5886b2472cd66b264f0f3c7b871a2f8a8940112abe039c3d4b9136bda4c9f56be72d010001ac75582207c74bbfa4923d1bf6b9be9e7d0930f15bd6bd3b5c274e01aef2493901320ca25f660cd8863e145fbbf6fd6e4b250ecf49df120f2aabaaa2105df1783701282e3514b060ab4ceba6058c47881907be43cd729177e53a21027e8860d55d334094e241d50c2ffabc846aa6300bc5d67ba26cbb3dc3f6d7affebdb3ff108b3d5660e985bcecfa5b79ec1c228b526bccc38906dd9b9275c30bacce17405ed62d01e2c8dac1035209145fd84981f6411055ab6fb767187d3c2e7d83f685880089cecf027a701f27da26e9626199889f332d235ba402ab59e6510aa8a11b9dd62262d08f9be800256ffbbf22544e0324c409f23cfae76dc04d15df6a8c580d539de3e7e0f13a17d37030072c68113e8a27c8feeb378a58aa846857c76cd108730414515ae58b3d39ec7f007641a577401873759d486071360cc0cfa13b399a7c1fa64516ada52f52d4c19905141f06a99eb61945ad498c1169c6302af0bd1b139fa4ea74f6243269fcf7aa3b01db4386071b151db4d2823787a0916d50b1aa63b2c816c85ebe397b4b9e67ea106e3fc1bfc2af656afcb596f9564cea2f72c9e7099554f629f10f8ede89681458684ddcb7eb2fd577ff3b4b7149c1e56f09189c32f5e93b46bf1fec65ac69cde16fd3d3aa64ac0013f01f654ecd8c60e585aa3fc7a50e39242823fe8abe83fdc86d39d32eae90e886f49eae8a02a9e42f3cca7e152e3a830dfa23ddea3b651ac7bfdac6e1f6994a5935d4ca3c703587d64dd3741abb5e2534493da90cfb207b642029301fc7de44b7f9fe9683144b575031237c6a85a883224d224af772686e90ae5718cabdc772c1a68a4efcc4eb3e866b4c071b90ab4865ae24d516b60cf06f3bab40795768bc59d9a9b13766b0ae3432cce7a1cfccd21ab702354f60e074a1506837d9d1d95df0ec9decd30d046591ec62d29436964508421c4e4df7039bc0b6507d8b35a9ee65aa0c2eebf0972f6ef54d4e50bb50512de2177d5802f37181fa553d46bc12cb173ef9f6729e7201f5012aa48d26b6628cb303d13ebba5563024bfcd5681581c48fb0ee8845a4ec8a1f2658b1260b63e9024025cff1da124bb641f2abe20fb7b8b00d946fd76025afbdb88e94516276ca4a30f2f212ec4c9ade8a5e26d8ec5d877b5c2cd21d814970140efcbd3eae570f9b6031241cb3af089f3822bce5facfb2ee41c159ff0c01325580a79d4755aba69c43a7e114d25b3f4c5acc400106ed5e4171260a43acb4d68784a2832689db227d6159129d5492deb33ffabb3d5c2302dcf0274805709d93b9684fa8e29f25a336b1053faea9b0c23959d8a0b7b663d0a64012f135ba18548540ec382e6bedc654c309e9ef791b8824c3f7a37f106e48be64acda154a7abd4e3d08987ac13a5bab41e8adb5405a6dec45a6abc98624fe92b6cf7ce9129e88e54a60c27ee9af76da21af0ac2989724bbfb368193c49dec64575d3016cf3e4a23017c14c75f6b526070e3b36384df1484c2e7f49d1e6e3b7080a35951df2d1d82acc19fc5c2874519820e26e6ca6975103e2248314b62cdb13e114f02568cff9f9d0dbbe0445c9391b210978416a06b847fb1c0afaebd3709fa3f587b51450641f9ec73ebda35040d01f24a55606420a5b89ca8bff7d990f83376cfffc75e561d6fa845b584852e631220117338761887479770d185e5bdc1076f8f24debbaa903a2097b0eacb18b123a00000000c4c3b827f46d95cac08efed6230bcb9d1934c9510a3849fbb246796f46301928bb373a65866878a40848d2151397078b1b307891a0480f0ea0af7c5d26d8132bc4040f4a736b4e631c3ce945e6ac1eaddfbb52007b29edac1c8a54289d7ade61fcd32cf4ab1baa7a0c759fda36c3caae7685a83c9e6e6feee53c8697a57df335e9239f6265bc2934667122a891468208211904444182140c1947c7b26cb8799445f63bf12608ff15349c3f4b90622f37821ae17784f53eb2287592ca40e1e3f19370a67b5e81a6c36a636681f50b1f6cec77dc3f3f0a2b129c3c79b4b88ccd22da870c1f7907a908036a3238e893a26a0448408c2deac7eb024cadb8098d7946d7b838f4225894603b6134dfec4393d30d7df04c319bf8a22b8ed4174d08eb39716fe994662251599d4ceddc10526540a5226c0d921e333002ade4102156164d897ee560447c63f572979d18daba882dd524ef358606f3b6cebd25939dcfb636e7cbf0a587f26a6922f491f73ed3bd33a6d31a96ee41d6566df473df6d4b915fdf30fac9ed50c3a109a228e8e11203368fb860ee365c6cb1f3d454ede35e4fb91c86d19b74eb47e5833c6ded6541b371d05e8e3a0744f28cf2223a06d119a68f8da2e914c71fa4042e1b2dc42318f18d7d2e4223ea8bffb17f0754e1c2fa3e0fbd8b181891e6338000ee9945b7c18438190146df465cd68d16ebb294b6dd58a4281ce4a466c6623d39b492c5a1a0d1d87f5cb2c07a28954852dcdc6721be4de9d79cd46cb1f626b7fa01e1d88a3a2bf89bd5f555ec877a39268156b2d915cb138d958de54287ceeeaab8d727031273785e4e45c964fe8a022b3829fe315ef5ecfabc35542c6fc1888701cb8d4f44eb235fcbf03abb7a743aa942daedc4bfe2eb2eb59c9d6a265042bb916da786457fae6b491ed5c2c364ebf8b7e58706be1816bf7ecabd056cf7a087cb49c6df364bdb380e3bf1cbd55f367089b953d1a781ac045883d0738ec9dc3d814af06e1c2aabea3cfec3dde9da0114b0cac3e14c4e1a4a4fd22c98563b9cbe859c67768a39f67abd5dbe55d3d16ff58e5c45b21d8c573f54b901a8ce92070184bfac62deb42faa0d8aae4312f6e5bb87d3fbf7f41e3ae973102911b15cdf18000001207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666f12020cd612b51afd462f8b19e3edcaf791570001e5d7d2a541e8a9dbd65430262fcbc3670b8e62bfc371cdbbb03aec8a56202812656652e733b4666631d0ac1863e1914ebfa94027390bf458fd5d4a0300dd9c03eafcb8deb7d8cb6395c581911b9289bc4401f3f76795d107ec8b30254f5527208d81eb51e9a8d9f5e81ed2112c9ae3420bb7057c57f0771b1a8b6af19fca848501181ae892cf374e2592ee04264ed88380162e5af0643f7ab3a74737423c06ff962df9a52b7af937d74f8223f0acf064d7b48799dfdf4973974b645bfc2bf4612f0173753e2dd70394bdd5ecaae04a88ce265a12b17136f31c9d79e33ed256020b26de133790bf8ae697d13d020100014f255bd407e80920d3bf46e4eb44f062bb4dca10053cb4fac2ed5a07b4f085370127f80f55bb75f8e36bea1672ee4e036fa92f93dd78c67d920b567b929295b23c01dca3fd69b33c66202184abb20819ad94b9fe65c07a9c8ec0a6a0cc80f9db0427aa298514d043a8cec61d3680986198df7f44935cfe8ffdb6ecdf782f2efce23c745a7545168a5d5593b4cb6141400706ebbab9e797a9fa9ce8faa0203c328e1401b6f389ff07c9b9554fc680e4553fca4f3c2b62d9a175b36458f54dcbd995d14246efaa5f115681ff4c53a1435fd7815ef46517389b046639b3158500b38deb057c9a4ac111b6b98075f84d4bb4c027ccbb2033728294767f8c3c242354b63064cd886cf20e60fe817b9a66d952bf6eb5a997cc23d79e9b6b93867edb5a9e69f6fac20b9d0c70c68497569ea7b8227df0a5ba6e68874ae2e9df5c2fec6ddea243478c98b60a86a344a09d02367d0ae4b3d49535c094f4245975dbd7542787601c97982564325c4a5bb4332fa79a976e71a1706bb0b0f59559ba55be2bab9386cda943c0d735d9ebcef6998d175ac66c4ab8fe9d2a1c0de31137d91568e13a54ac53d3e62e2c005a3c70c031308cdc233c624366b56e3d8407df2890e6083a30fc8224e25a1aa649784acd4cf525f564b87186eeb1359820b99c4b2bab239a6c79e82cf44c386fee4bf719a8c56a1df1bf028c52e1590a5e76e5bbcc435f7b71ca4726237c14c94ffc8a4a19d41798ee851cccf94b4e2a271dac766f51f10adc8abd343abf2793ef25180bcddda6c3ac63d22183805d97a8dfc7b9432e8a7a00220609b96d0ad8adf4bb6b07616239b530f9e384b15407b8d9d36f671aaddff6da610632df26175224cf0a55a0a543175884c37251c6fa82f9b974ca0fafcf0531b1e67e283eff4ab04652717cbf8c3fb5a205317090e6f687c51d7dc3b5bac8022ccfccc62a6ad2d713f459976ddcc620461bf25445da2e5611253ff575fa96df147506c33c9b5a97fcbf51d54b333430c61720cf720ef57094802dfb6304d8f98ca47b5a2aff93bcab58ee5d5e6c063ceb6faa12c7015d1fbfd084f9de3025ef87c14ee100ef093931e1d4bcc636909a557c472fde2d6568cc1698faeb95afa982b25de03317e88d60edc54a98c1e28f57f1f0bc41b29085d6ef4d44a5da8dfbc20a14881ea6c4e77e6eb9c4031753d201f610c395f070037b74c6ec6af37db29a53484c131fbe21e7acf5fb62009b6bbaccd7a99fc86782b8c0870ac3d421f9e4d16d6b275a38deb8873954197344926db5d131f7a8982e12bda39b082457992398bc063620b189d5479436a31d3bd693ae160a8448455a98e732a1b70325bbc74aca4a389dadff81a9466a90aecf4be59650b6291da62d6862adb94169d874c8a07c0d22137306eadc45a734195d2928291618778d06fd30c41de76cba30b3af42b42d0782638a9d801582bcd1964de6f5ad8aa4e4e770621995fb95932517204cdb5518bb5cd1fc51218c548d7f007b96e66148729eedf33783170de66b24184cef61180c5f04cc4a32a330f46383c1535089894abe84e742e99ee30746218f431683081fa5ad6c05fe91c3415a466c1008423572ab53f3f4bc71dd4ba4b4020b49993a37e4287b30b64069937d12502089dc08aa2936ead0b9700579eeae610906fb2d011fafcbe9f9662612883cfc9482ec528efcfe8599ad804b4d92e2df63ac61ae19000000e43a1fa07ee130fdcbe71433d461500caa3c5353eb8dd328f7ae59cc47efb70b52bf497498f1be83d007fcee95f16c67dbf2158646471f797b2ef14a33ed670cc404fcf32b8638f93c13d65d32b5c2c6ad095ed997c001ef9437b0d25a042e479fd3e61d7ba6f8764d1754ba6855586aeb831210e9997a5a7c2b257b86c5c9d8c347a27e21be3235c35716ebe211ff56f5eac5fa8b6e4e2dab0b4f97918defea11ee692ac74d4077e7c91bcdb054b7f3786d9a569fcb5a70b7e6e2bc11e37583bc7e0a72e64c4501b8815db1285fc2bef2f38d08c63607db80996a468df7ee3429ee80c220798485dcdd681a66c415fca444099390bd0f71c472a6df571d09e75bb9395827b818f12e34a44ad7acfbbb2b99a732acae7cf5918715e6619353d24ca3b4820d282b517d0f55252341aa5ad16b8b76ca599e587366cce9fe6a8c862b30f8cec40c9bd289fdef187cd6421e0a4af9eeba5c35e6bfb18b968926398bf3fb0d33e930729cc4ac892511c94ad95f8df860957f074730b766d716e4d1c63c83de642a0efbf52bfa2d8114a4aa1335d8412c57c3ddd700f857c9a8ebd4d3b88328eae7a51478c73b4bd1a7207b7b3bcfd1540ba1ecb10d65567b987ff43329cd6c4b41cdee30b5f5986e993d4c9120494e506ca17c53f85d2ae77db8ec105902b6ba004fca7cef3cd8ce4458c042581a39e4dea6e68fb259f4d8c4f3261eeeb76a943d790b2e1f1dd163a4f3028b086a4e42a361290aea1f2cc8858928aba8d02c5776075b07a953d848d598445a67351c5a54ccfa36d3c869ec06ec43c53dcd58c570dff6d309133953c72e4893f8ec8ea29b1dd59cad43a477fc779887d2a8871496a4ae8df4469c020eebd281572dbecd98a7c478e525ca372fb13b182da6ec7d924f50d63f042c577e19780ed86da60bccc72edab3a7d4ad2a79484122e11fa0342d70679b3e5cd92edfc3253816433d1d73474a22e6b68100ece18ca0a75c0e9c38e1197ef900fcb2ce21a8a6248e3c9950ae015f88e5c18bbbdc99944c4b28638b0584295e4ca72ac8d87ee2165602bf8dafdc16210bfbbe15797d63f53e01000104a46d0e52c6ec386e384dfb13470bf81d5d4b01abe8fead0821c6c796af6cbf0000000162189c40473eb519fb9b4c4566180a587312b3d9f22722c0f204c90c4873843903a8ce920701ae2935f1dfd8a24aed7c70df7de3a668eb7a49b1319880dde2bbd9031ae5d82f000146f06ee6673b1869f447e861994a879d7ea01599b699ef7ba33fb5979e93ac0b";
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
