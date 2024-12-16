use alloc::{
    format,
    string::{String, ToString},
    vec,
};
use zcash_note_encryption::{
    try_output_recovery_with_ovk, try_output_recovery_with_pkd_esk, Domain,
};
use zcash_vendor::{
    orchard::{
        self,
        keys::OutgoingViewingKey,
        note::{Note, Nullifier, Rho},
        note_encryption::OrchardDomain,
        value::ValueCommitment,
        Address,
    },
    pczt::{self, Pczt},
    ripemd::{Digest, Ripemd160},
    sha2::Sha256,
    transparent::address::{Script, TransparentAddress},
    zcash_address::{
        unified::{self, Encoding, Receiver},
        ToAddress, ZcashAddress,
    },
    zcash_keys::keys::UnifiedFullViewingKey,
    zcash_protocol::{
        consensus::{MainNetwork, NetworkType},
        memo::Memo,
    },
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

/// Attempts to decrypt the output with the given `ovk`, or (if `None`) directly via the
/// PCZT's fields.
///
/// Returns:
/// - `Ok(Some(_))` if the output can be decrypted.
/// - `Ok(None)` if the output cannot be decrypted.
/// - `Err(_)` if `ovk` is `None` and the PCZT is missing fields needed to directly
///   decrypt the output.
pub fn decode_output_enc_ciphertext(
    action: &orchard::pczt::Action,
    ovk: Option<&OutgoingViewingKey>,
) -> Result<Option<(Note, Address, [u8; 512])>, ZcashError> {
    let domain = OrchardDomain::for_pczt_action(action);

    if let Some(ovk) = ovk {
        Ok(try_output_recovery_with_ovk(
            &domain,
            &ovk,
            action,
            action.cv_net(),
            &action.output().encrypted_note().out_ciphertext,
        ))
    } else {
        // If we reached here, none of our OVKs matched; recover directly as the fallback.

        let recipient = action.output().recipient().ok_or_else(|| {
            ZcashError::InvalidPczt("Missing recipient field for Orchard action".into())
        })?;
        let value = action.output().value().ok_or_else(|| {
            ZcashError::InvalidPczt("Missing value field for Orchard action".into())
        })?;
        let rho = orchard::note::Rho::from_bytes(&action.spend().nullifier().to_bytes())
            .into_option()
            .ok_or_else(|| {
                ZcashError::InvalidPczt("Missing rho field for Orchard action".into())
            })?;
        let rseed = action.output().rseed().ok_or_else(|| {
            ZcashError::InvalidPczt("Missing rseed field for Orchard action".into())
        })?;

        let note = orchard::Note::from_parts(recipient, value, rho, rseed)
            .into_option()
            .ok_or_else(|| {
                ZcashError::InvalidPczt("Orchard action contains invalid note".into())
            })?;

        let pk_d = OrchardDomain::get_pk_d(&note);
        let esk = OrchardDomain::derive_esk(&note).expect("Orchard notes are post-ZIP 212");

        Ok(try_output_recovery_with_pkd_esk(&domain, pk_d, esk, action))
    }
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
                Some(ta),
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
                false,
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
    let orchard = orchard
        .clone()
        .into_parsed()
        .map_err(|e| ZcashError::InvalidPczt(alloc::format!("invalid Orchard bundle: {:?}", e)))?;
    let mut parsed_orchard = ParsedOrchard::new(vec![], vec![]);
    orchard.actions().iter().try_for_each(|action| {
        let spend = action.spend().clone();

        if let Some(value) = spend.value() {
            //only adds non-dummy spend
            if value.inner() != 0 {
                let parsed_from = parse_orchard_spend(seed_fingerprint, &spend)?;
                parsed_orchard.add_from(parsed_from);
            }
        }
        let parsed_to = parse_orchard_output(ufvk, &action)?;
        if !parsed_to.get_is_dummy() {
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
    spend: &orchard::pczt::Spend,
) -> Result<ParsedFrom, ZcashError> {
    let value = spend
        .value()
        .ok_or(ZcashError::InvalidPczt("value is not present".to_string()))?
        .inner();
    let zec_value = format_zec_value(value as f64);

    let zip32_derivation = spend.zip32_derivation().clone();

    let is_mine = match zip32_derivation {
        Some(zip32_derivation) => seed_fingerprint == zip32_derivation.seed_fingerprint(),
        None => false,
    };

    Ok(ParsedFrom::new(None, zec_value, value, is_mine))
}

fn parse_orchard_output(
    ufvk: &UnifiedFullViewingKey,
    action: &orchard::pczt::Action,
) -> Result<ParsedTo, ZcashError> {
    let output = action.output();
    let fvk = ufvk.orchard().ok_or(ZcashError::InvalidDataError(
        "orchard is not present in ufvk".to_string(),
    ))?;

    let external_ovk = fvk.to_ovk(zcash_vendor::zip32::Scope::External).clone();
    let internal_ovk = fvk.to_ovk(zcash_vendor::zip32::Scope::Internal).clone();
    let transparent_internal_ovk = ufvk
        .transparent()
        .map(|k| orchard::keys::OutgoingViewingKey::from(k.internal_ovk().as_bytes()));

    let decode_output =
        |vk: Option<OutgoingViewingKey>, is_internal: bool| match decode_output_enc_ciphertext(
            action,
            vk.as_ref(),
        )? {
            Some((note, address, memo)) => {
                let zec_value = format_zec_value(note.value().inner() as f64);
                let ua = unified::Address::try_from_items(vec![Receiver::Orchard(
                    address.to_raw_address_bytes(),
                )])
                .unwrap()
                .encode(&NetworkType::Main);
                let memo = decode_memo(memo);
                Ok(Some(ParsedTo::new(
                    ua,
                    zec_value,
                    note.value().inner(),
                    is_internal,
                    true,
                    false,
                    memo,
                )))
            }
            // We couldn't decrypt.
            None => match (vk, output.value()) {
                // We couldn't decrypt with this OVK; try the next key.
                (Some(_), _) => Ok(None),
                // This will only occur on the last iteration, because `keys` below uses
                // `vk.is_none()` as a fallback. We require that non-trivial outputs are
                // visible to the Keystone device.
                (None, Some(value)) if value.inner() != 0 => Err(ZcashError::InvalidPczt(
                    "enc_ciphertext field for Orchard action is undecryptable".into(),
                )),
                // We couldn't directly decrypt a zero-valued note. This is okay because
                // it is checked elsewhere that the direct details in the PCZT are valid,
                // and thus the output definitely has zero value and thus can't adversely
                // affect balance. The decryption failure means that the `enc_ciphertext`
                // contains no in-band data (as is the case for e.g. dummy outputs).
                (None, _) => Ok(None),
            },
        };

    let mut keys = vec![(Some(external_ovk), false), (Some(internal_ovk), true)];

    if let Some(ovk) = transparent_internal_ovk {
        keys.push((Some(ovk), true));
    }

    // Require that we can view all non-zero-valued outputs by falling back on direct
    // decryption.
    keys.push((None, false));

    let mut parsed_to = None;

    for key in keys {
        // TODO: Should this be a soft error ("catch" the decryption failure error here
        // and store it in `ParsedTo` to inform the user that an output of their
        // transaction is unreadable, but still give them the option to sign), or a hard
        // error (throw via `?` and cause the entire transaction to be unsignable)? The
        // answer will likely depend on what checks are performed on the PCZT prior to
        // here. For now we've chosen the latter (hard error).
        let output = decode_output(key.0, key.1)?;
        match output {
            Some(output) => {
                parsed_to = Some(output);
                break;
            }
            None => continue,
        }
    }

    // undecodable output
    // we should verify the cv_net in checking phrase, the transaction checking should failed if the net value is not correct
    // so the value should be trustable

    let value = output
        .value()
        .clone()
        .ok_or(ZcashError::InvalidPczt("value is not present".to_string()))?
        .inner();

    // TODO: undecoded output can be non-dummy if it has memo,
    // we should decode the enc_ciphertext with output's rseed and recipient
    Ok(parsed_to.unwrap_or(ParsedTo::new(
        "Unknown Address".to_string(),
        format_zec_value(value as f64),
        value,
        false,
        false,
        value == 0,
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
        let hex_str = "50435a5401000000058ace9cb502d5a09cc70c0100e480a80185010001227a636173685f636c69656e745f6261636b656e643a70726f706f73616c5f696e666f140cd612b51afd462f8b19e3edcaf79157bc80a80101286baaaa462b23e7e650e297f4f2899f790744ab4887fce79d44482bdfd455410000000000c0c393071976a9149517c77b7fcc08e66122dccb6ee6713eb7712d2b88ac000001010325d5b38ff44c279744d83c97f139878dc6837a9df74604982f326e87840898422fac20755b7bb7c99d302b782ea36f62a1b0cfe8d7d4d09a58e8ba5da26f457805ac80808008858180800880808080080000000000000000000000fbc2f4300c01f0b7820d00e3347c8da4ee614674376cbc45359daa54f9b5493e01000000000000000000000000000000000000000000000000000000000000000002f1d773637e4e0ea9d1f7de2a8d12e8828a66459de9e3533df8145ffaaa5ce73800a9eca84366240683b76e92c991f7374fa3d8cf0c2171ee422e8e4a385b9716902e45e2a8a7ced1acc8bdb3dbe39cf485faaeacd5ba8027d3cd050575adf9bb01689584ed9c46006686190462bbfca41f810803ee1cc8e71a19e624e2145cb1aeab4665c923c32d4f99f25e0202f20e014478555ac756cdec7982a344fcbca5330133eea265d63854fb9adabaf8c011b762a0718cba2f7d751a529b200f998c670cc6653ff4ce92f723d38cb101000104a0e1de8f23e3bd683c2e740fe337e3e1d9e1bdc9ef2932b30c92d1a3d37700014abf5abcbc3595dd0dac8348d577ee963869d9d03d37835d570b429d785b09800150e2cc6470c95b7f97db236576ae68e1122894fd913adeb1f5d23f8dd4cef30c991d5c50a00a89102832ca29155a5c892689056cbabc7bf38fa7fc81b14fbb258acf774e4a10922e14e5da5440fb621d341b284d03696594af8edbafbc9df12801cdc1fcfc02539d9c95a5b9497dfa074e697f2626b77eec5d1f1a1753201066084e8d8744226008d1079119ac14f0a03f7ee3093ae65c3bc98a7ebea1e21fb6dc9055df0a2a0ba3fee547acbbe7b46ab7410ee27aec1cfd5e7ff6d0b4ae8bdb229cb97f45021af193f674e8726cb2351008f9f405fdc49f12fdff10649207423dbe7099cc082490833931e55eb433df8aba64f9159d349b53fd6355214ad0bfe0d73722170ee68715b6693cf66cb1dc48532ab686870553ebc64898a6773a0497a5e5532d336b5aedc04377ea469bc1013c57ca3cd96644c9bcbdbd0ccd3104cede84d73738ab2e135e2d70e7c48c8fc981f04041b5acf8a351ad5c835295c696c57713bb2a7d13049e900262c12cc9ef23237831764b1f261b6190430f9e9c4ac5de1cd60fd8690842ca99c8bfd0f4566c5d156a15b68f079aa777c7bc7c29e8ce94a4b53cbb6a4f1f9095d4559401f79991c5d5cc82e9336137d9e95f33ac73e69e0d810dd7ef622ddf8c0bdf2e3ff285a053b60334bec13f88196ab52f47faa74f14da28e48a0526b47d32843184483ba283b66da41d59a503b39fd4d8ac16c01b98e437f1ea1a36d36b6d6abad2b5290935d0b105c7988090869a7fa745049a53fb2f150588fb3abc96bed63db89af26bf3074f31e8806f31a5814a54201d1cb88abc128344732ec3ea192214bd0846bcb93db5af58b167a68353a70fdd0d4c0f78680e067fd7f742d84d202f23279e8a035e0584ceffde059844bd3a3488a23ae8b92ed617c992852fd7ad100358ccb6fd04afbd7a87b10ad66f5061d7899d9ca70c3f83c2c5ddf12e400e53c6e17743677f92efaf2683dbd63476089fa4c7d56cd00bf6d7aba3f0aa8d9d84ec1cdb314102f0ad45a780de8313ac4aad688b12972103a44d7668ba02f21317f319062bbeb758ada01d553f1565ea057413672871a834488062c58d65d9121bac7a4009b3e135efa8551cdb720b61a123e937111f1937aa1418756934aab6119a8263e4b3a9f3cad1ca7b5153e890b0ab8205e18caf21788d11eb738b30a37dba8c993c1176fbfa3ddce2dfbd20bed9facf4ec5408a2dae7c3297927c7f4cdb76fae99a6e5cfb9fbdfe103a941846f3768e705546c407b85d707824dc5439707bb9fd946cbcae9374dc21f9422032f3bee0e57788c33f816fd293cb0cc8862aa209462294d21e6d05c9189c31657c489d7d25206d27123c230de825dd63c89d6ba774e5297527d552f75c69a26305dfa6fbbac3b2822d944fdaa224779fdfb1777585cc7854b876b27de0c89cac611505b3fea3cc480e07d83647f9fa3bdba233d00779f6603d6cc0cb5de3d34057ec9e78941c3c100a508bfab8b8da9a308cea597631620ce93ba90b5e715cb4c8381c77af36ac4d13d799cd95c37f20ced60c7f9fa6f6b5e7c662444c84bf7add70551c0c1b3693270119986e1d40e8190b98563e642880228d81e44e15e73b56b182b4652141acac08000000ccbe7de39eb36a14e9ef6a8bf805ad9133b07898d86f2d6d618530c1cf23b132dcf14852fe0344ed8c300dd10a7b8908381fad55b038adca573812298f464990c40416c99750e5b3e8b6bafbcc91a7a21afedd51ca8dcc7fd7c23f119bf9cc51d71a34846b12b14b6248a53f6216a6ca92bf9703ed3032238d79a07804415f05c04454c3df2a333f4963f9617f6c6e881fed6f0149c0bebb9e0e5c0809e746798575400c52a5dcd7b9aa5ae8ddbc4a2f920e99130c130a47f7fad55e0033b161ab149f8a780cc0edd26f451cfe6263d1bc95e0e3860bdea0fbdd572b9530f07280926f6fd8fa07c8313b6f812bcd9307fdab02045bf639857ce3d67435bc0283624e3bde49462bb72069c462fec39098a67ecb6a9205a7d6edf367e84485d2c40cfaa7c0446fae597e136866d891f7580dbfa947c2f0b42c41e7b46a03f44459e40d869022250e9e6cea2f8af487efaeabe225c7edfa3ddd5554aa71edba60150a6601c8c9abeb96ae43373a7baf88ae8a64be69fcc9ad6d1a8bf895f0b6fc5eb66138e0917668a4d5c0c26c0a1b90238753e1092f9eea40378153a72e22f08e0bbdb498c6467088885e889da006836898153a1e686819430192b90b5f9a823e79fa4bd06baa22ac443e0605b21bb48e11291b212401ad33bd0f706c62e971fbd277bc53b06ebcfa4acb647270187a01f93b1cd605b7b420082e1b6f771998c4a8559c0c0e7b1a4e4214d63de28f7d25b6fc51d5a0ba40c758811c8ef0ac20ecda526d6b17069816877ba8b0f799202b1525d8cd0006b09e76b4f5a558547cbd8fb77ac469186857f9d7ef063840a030158721079e4695d5640d703d5f0642cc937df3cc060b9c59c9d5583e8489e151c46444b7d987cd0be5a8b4b45758472ade1cede2dcb450641bf6b12f3fa1dba5a03238f658834ebc862cf4bd6bb30db70a8e27431b1148998ef923942c575ae4e35c4925034fd3ece15ca01cc46deac64ba408f0b5643ec8799d712ec97cea9cf2a25256d5ac460114b0cac3e14c4e1a4a4fd22c98563b9cbe859c67768a39f67abd5dbe55d3d16ff58e5c45b21d8c573f54b901a8ce9207015e7264d194256668f624bd58d8ed8534c276bb3e00a2372731ecf297160494db000001207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666f12020cd612b51afd462f8b19e3edcaf791570001052705358d7aebca05cce3c7425a1a4648ebfddb4201abc6ebeb49aa5435a737b30727b116fe9a20afa5f2a5cfb67b062f06eced823f0fa65e281bdf0913c2a3f5f1263629eb2d87ee55c3469080ec33b0a9911446c0b8f922a8571478144b2a37c58d85cde4b38ed820207d2b57ee63ce74fa1ca9236c5c3a69029b743897a6012b069e529909e9970874961808ffb8f6f2f0d1798f317825a043d65428db9ab9bc87149a7aa91fa3521fe601156f884de6d830c1671e6b9dd5f08f4fd03bda1d0172b777b1d2ef1ccb30e255e663ec68957e333dc577448867bd44419e266879a535161dc0296fd55d9c2e95010001aadd62227373935cf81f0c9282252f2f739794da190a2c8c84637ffda917511001528f0a01acd61c99a963e39cbf14b7c27aa7b421fc72af591496bc8c260fc5f8010209a54b62463ca977c73d4cbf7a011b11bec95fd1790555f30b0edcc86ec13e8d9ee0ae96c64cd68f70e84fc8ec08d3a4df1f8e9201b03138d6df89cf25572823fb23db29cd163d5c952e1e48c1d108ef36c2fdab9e45646dbdda4b133d341a01e09694bc0c1ca20570234cd24f079f9f6bb0fe6859954f9a0883ad38e95e144e66d4d2f3098a38c3081a1178bb39e9d2b08af34f85068a3263a6b31e9acc3292cc9180e027ad7002b89e5bba3be970d4991fb8fdc74c5a8b3945ba5de35429f07a20bd7b1ab21022f8d64fb567f203eb603a7d3a86867c9a46251e7b046e9629a1614d6c0a43e9c40888e7c78f64215507db3b90b0d078b7d35f22c9c43d5372bdda2f790ee72d54021a8f5756a2259b7e6b213a0dbdd37bd417dec3ac40665ab8a57bc6203a88f11cfb0106adb61879e9be807e79db7365d42a8a7805dad54236447ded3725fc8da5bae4ee981640353d559d324e218b9104c4aec342e7a9ae6108100b2556aad0f6678f2d61ea20648e21bf14d7fbf7f28f0bb999383cb23bb24f89da3e6347754dcf3957232d67959b11e4d5fb8fe2d3eb15243e4bc5ea09a9abaf1e2483ef9a1c7303d3d1c098aef1ea586cf935297c6aaafd25995123adfea8bf542cd79781665f22a204e798adbd3b0473e2b83ff90860efaccb9b893248db815c23f4548b17a75b4e8ccb0225f5d0d7439d8c4579435b944833c2205241741e6a1e91f12f838d4b889b9a39af44317645e756399c21f37d9a6cdfd19d749d76c33761bb1f22ce30eb124e9100c11db89a15e5578771e63f901bbe071ef01c5e0918fc0ae91ee343b3e47619406f54709ee0ec2cdb42ddaad36df36152dd6e88d50ff5afb58815bbef551c814ae9fdad025690add07574ded80907edc50bededed00d3448bfbf7182017ad9a914f83baa8634edd48dd0b7fb236b894274a09163d1a8df7590e60981bd1a8a9adf778838e06f271bdc6d29be74624b219b26c09023c9af9323a5c7bf125d32416621fa8da49aed1f7f06416728a465219571b65b50839a8b308457b98f9a674a76615930ba69f6c69b77185013a1121a7c461da670165e4871fd8f0e03857d554d37f45eadc3ce441f27491abef6db3ff0f90d8260fd9e4fc1364114a4d22fa369d33306982c9402fd9a73a94d5739700a0ad86e317bcfe6831f02aac02d8a9f082ea0876d7193fa6d2b49aadc37a6b3ba5b597e03a8ec6cac78050c7cbabfe6ffac18b546be713e30a28549a04738058d9509930125c40f16b67e763f6f9f53caa941e7f0cb176fdefea02de80699b222eb9f03404094d70f6f83d717b049e3317620cd90ee3aa43f4511318a3662eea318b693c1937e756381e6c2fe66aab3392793b6617edb14bb182fcec08c1bc6a331ee64d05a17d6e134da0217ab57747f72c92e206c3ce08284a9ceeacc457d73ea9b7ef3c2f285bb709a51747dd68bf4bbeed25e3a7bad4031f76dc5d569d07d1a846fd1c39f2bcf4aedf40da445881929eb00fb0d9bd59e630f69a605ecbe971fcc2600da109acb1e4e8a1f74d7a34c98abb0d055f9dd18cf2bfb2c9ea857f56875aeb3001636452237e0c37489bd4151c32f5277c021909499bd5f0d378903c6e914b3025000000b94466593877e8035b6db3b9f445647c88c49d5234ef74fa1ca266e2fd33f12bd61b430fcdfa965fb293a9b39397e7563d8ce69f04fc4275ef32794a305899a8c4045fbe2af5e9caca7ea8b2250b25472a5e30b4a2310cb09a0e4afecb7b8a157f51243c720358faa35d6aff8fa80c7a967d0076d6d4c14f2f53f01703c2a71d3a0d90da921480d7268bd78d324ec5bd99c5cf735820ce2332bf10621a67225976d707c58304705d018090eb05e01965d5783ac4176047ff1615e066d036adf7b9891bcbff7a1fee78811da0dcb5d649c00ccfea85b7eef512668c64fb20f5bb04c8ec5c2b5d950b0a3ff8310eb8e926f56cacb3e4edcf323ed976cb6f3a4a9b75a90b88351c6f42e1f00a61dcdb490148f37d9e81e31a2f9630593a30ad55b4c85ec2325f58ddae24c581d3fe0b1d5d166ec3d63dba77eb21a3d3246afc25c0c04013b6a2e7bd219adc716cd415ce3885df8f279dd5e470249b22c1b0f793ff649d423be6654aef055bbfc84d1f2a1ef62c82612f8e62018115e95e29cc2b5fb52a6995b13ee96373234ace55f35443932bcc0eff5dbc9bcc58dc3859ec1fb980c3a5ea12f298d870781cfda22e1ea14f47d0719e225bc085bf1b09a6221bfa33208fc25f7818facdab11b1d4898a6550eab8ed7c78016d21e2c7fdfe1604639fa13523c4bd74cc23d0a3fccdb63c38ea7a5f0f20a2458ed7059af108d99dd00575a0ad8a0341ededb7a1ff0988a4c135be579344295adee905c0bd279267822b9192703c7442bbc2d1b406c25013e06b9bce2dede25f48954ba6c4cd6e6b7290690d63fd02c8a1844d886435973c4520aad023cf29ebe766cdece277b19cbc2b533a66f6fcd8ec8933f7c37721fa9cbd9b9c34cba5ff658c1552fea9524390f84a26bad251504357025c3bb66ec0d84c1ed1a56ddcfc37fba5d2eade50a4e23c3ed242971ae589c16f5500f3d9b73098b9333f6eef3944e8a607bb3ebc496a83f36d2451d598c984bf8e668e475cb7ee17d38b45b2440128342b239c8926225f4c1e2c915e50c6089c79256aab528d83e4b47a30060341878665c8965a473074f1050100011598a078e9f33f65f07e435b9a597c9b9613e6981d949f07a42c6bbdd14ede1f00000001269db5bb2b896bc1edbfa2b594fa55bfcecd9bde9ca3d8f8b77c4108404e692703a8ce920701ae2935f1dfd8a24aed7c70df7de3a668eb7a49b1319880dde2bbd9031ae5d82f00012ac4baf09718100016e3f173dbbb29e316b999badfa483bfa3688bb29483101f";
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
