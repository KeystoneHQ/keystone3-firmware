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
        self, keys::OutgoingViewingKey, note::Note, note_encryption::OrchardDomain, Address,
    },
    pczt::{self, roles::verifier::Verifier, Pczt},
    ripemd::{Digest, Ripemd160},
    sha2::Sha256,
    transparent::{self, address::TransparentAddress},
    zcash_address::{
        unified::{self, Encoding, Receiver},
        ToAddress, ZcashAddress,
    },
    zcash_keys::keys::UnifiedFullViewingKey,
    zcash_protocol::consensus::{self},
};

use crate::errors::ZcashError;

use super::structs::{ParsedFrom, ParsedOrchard, ParsedPczt, ParsedTo, ParsedTransparent};

const ZEC_DIVIDER: u32 = 1_000_000_00;

fn format_zec_value(value: f64) -> String {
    let zec_value = format!("{:.8}", value as f64 / ZEC_DIVIDER as f64);
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

pub fn parse_pczt<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    ufvk: &UnifiedFullViewingKey,
    pczt: &Pczt,
) -> Result<ParsedPczt, ZcashError> {
    let mut parsed_orchard = None;
    let mut parsed_transparent = None;

    Verifier::new(pczt.clone())
        .with_orchard(|bundle| {
            parsed_orchard = parse_orchard(params, seed_fingerprint, ufvk, bundle)
                .map_err(pczt::roles::verifier::OrchardError::Custom)?;
            Ok(())
        })
        .map_err(|e| ZcashError::InvalidDataError(alloc::format!("{:?}", e)))?
        .with_transparent(|bundle| {
            parsed_transparent = parse_transparent(params, seed_fingerprint, bundle)
                .map_err(pczt::roles::verifier::TransparentError::Custom)?;
            Ok(())
        })
        .map_err(|e| ZcashError::InvalidDataError(alloc::format!("{:?}", e)))?;

    let mut total_input_value = 0;
    let mut total_output_value = 0;
    let mut total_change_value = 0;
    //total_input_value = total_output_value + fee_value
    //total_output_value = total_transfer_value + total_change_value

    parsed_orchard.clone().and_then(|orchard| {
        total_change_value += orchard
            .get_to()
            .iter()
            .filter(|v| v.get_is_change())
            .fold(0, |acc, to| acc + to.get_amount());
        total_input_value += orchard
            .get_from()
            .iter()
            .fold(0, |acc, from| acc + from.get_amount());
        total_output_value += orchard
            .get_to()
            .iter()
            .fold(0, |acc, to| acc + to.get_amount());
        Some(())
    });

    parsed_transparent.clone().and_then(|transparent| {
        total_change_value += transparent
            .get_to()
            .iter()
            .filter(|v| v.get_is_change())
            .fold(0, |acc, to| acc + to.get_amount());
        total_input_value += transparent
            .get_from()
            .iter()
            .fold(0, |acc, from| acc + from.get_amount());
        total_output_value += transparent
            .get_to()
            .iter()
            .fold(0, |acc, to| acc + to.get_amount());
        Some(())
    });

    let total_transfer_value = format_zec_value((total_output_value - total_change_value) as f64);
    let fee_value = format_zec_value((total_input_value - total_output_value) as f64);

    let has_sapling = pczt.sapling().spends().len() > 0 || pczt.sapling().outputs().len() > 0;

    Ok(ParsedPczt::new(
        parsed_transparent,
        parsed_orchard,
        total_transfer_value,
        fee_value,
        has_sapling,
    ))
}

fn parse_transparent<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    transparent: &transparent::pczt::Bundle,
) -> Result<Option<ParsedTransparent>, ZcashError> {
    let mut parsed_transparent = ParsedTransparent::new(vec![], vec![]);
    transparent.inputs().iter().try_for_each(|input| {
        let parsed_from = parse_transparent_input(params, seed_fingerprint, &input)?;
        parsed_transparent.add_from(parsed_from);
        Ok::<_, ZcashError>(())
    })?;
    transparent.outputs().iter().try_for_each(|output| {
        let parsed_to = parse_transparent_output(seed_fingerprint, &output)?;
        parsed_transparent.add_to(parsed_to);
        Ok::<_, ZcashError>(())
    })?;
    if parsed_transparent.get_from().is_empty() && parsed_transparent.get_to().is_empty() {
        Ok(None)
    } else {
        Ok(Some(parsed_transparent))
    }
}

fn parse_transparent_input<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    input: &transparent::pczt::Input,
) -> Result<ParsedFrom, ZcashError> {
    let script = input.script_pubkey().clone();
    match script.address() {
        Some(TransparentAddress::PublicKeyHash(hash)) => {
            let pubkey = input
                .bip32_derivation()
                .keys()
                .find(|pubkey| hash[..] == Ripemd160::digest(Sha256::digest(pubkey))[..]);
            let ta = ZcashAddress::from_transparent_p2pkh(params.network_type(), hash).encode();

            let zec_value = format_zec_value(input.value().into_u64() as f64);

            let is_mine = match pubkey {
                Some(pubkey) => match input.bip32_derivation().get(pubkey) {
                    //pubkey validation is checked on transaction checking part
                    Some(bip32_derivation) => {
                        seed_fingerprint == bip32_derivation.seed_fingerprint()
                    }
                    None => false,
                },
                None => false,
            };
            Ok(ParsedFrom::new(
                Some(ta),
                zec_value,
                input.value().into_u64(),
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
    output: &transparent::pczt::Output,
) -> Result<ParsedTo, ZcashError> {
    let script = output.script_pubkey().clone();
    match script.address() {
        Some(TransparentAddress::PublicKeyHash(hash)) => {
            let pubkey = output
                .bip32_derivation()
                .keys()
                .find(|pubkey| hash[..] == Ripemd160::digest(Sha256::digest(pubkey))[..]);

            //this is checked in crate::pczt::check
            let address = output.user_address().clone().ok_or_else(|| {
                ZcashError::InvalidPczt("missing user address for transparent output".into())
            })?;

            let zec_value = format_zec_value(output.value().into_u64() as f64);
            let is_change = match pubkey {
                Some(pubkey) => match output.bip32_derivation().get(pubkey) {
                    Some(bip32_derivation) => {
                        seed_fingerprint == bip32_derivation.seed_fingerprint()
                    }
                    None => false,
                },
                None => false,
            };
            Ok(ParsedTo::new(
                address,
                zec_value,
                output.value().into_u64(),
                is_change,
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

fn parse_orchard<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    ufvk: &UnifiedFullViewingKey,
    orchard: &orchard::pczt::Bundle,
) -> Result<Option<ParsedOrchard>, ZcashError> {
    let mut parsed_orchard = ParsedOrchard::new(vec![], vec![]);
    orchard.actions().iter().try_for_each(|action| {
        let spend = action.spend();

        if let Some(value) = spend.value() {
            //only adds non-dummy spend
            if value.inner() != 0 {
                let parsed_from = parse_orchard_spend(seed_fingerprint, &spend)?;
                parsed_orchard.add_from(parsed_from);
            }
        }
        let parsed_to = parse_orchard_output(params, ufvk, &action)?;
        if !parsed_to.get_is_dummy() {
            parsed_orchard.add_to(parsed_to);
        }

        Ok::<_, ZcashError>(())
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

    let zip32_derivation = spend.zip32_derivation();

    let is_mine = match zip32_derivation {
        Some(zip32_derivation) => seed_fingerprint == zip32_derivation.seed_fingerprint(),
        None => false,
    };

    Ok(ParsedFrom::new(None, zec_value, value, is_mine))
}

fn parse_orchard_output<P: consensus::Parameters>(
    params: &P,
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

    // we should verify the cv_net in checking phrase, the transaction checking should failed if the net value is not correct
    // so the value should be trustable
    let value = output
        .value()
        .ok_or(ZcashError::InvalidPczt("value is not present".to_string()))?
        .inner();

    let decode_output =
        |vk: Option<OutgoingViewingKey>, is_internal: bool| match decode_output_enc_ciphertext(
            action,
            vk.as_ref(),
        )? {
            Some((note, address, memo)) => {
                let zec_value = format_zec_value(note.value().inner() as f64);
                let memo = decode_memo(memo);

                // Check output recipient with decoded address here to save CPU
                // if the address is not match, return error
                let recipient = action.output().recipient().ok_or(ZcashError::InvalidPczt(
                    "recipient is not present".to_string(),
                ))?;
                if recipient != address {
                    return Err(ZcashError::InvalidPczt(
                        "recipient is not match with address in decoded note".to_string(),
                    ));
                }
                let ua = unified::Address::try_from_items(vec![Receiver::Orchard(
                    address.to_raw_address_bytes(),
                )])
                .unwrap()
                .encode(&params.network_type());
                let user_address = action.output().user_address();
                if let Some(user_address) = user_address {
                    let za = ZcashAddress::try_from_encoded(user_address).unwrap();
                    let receiver = Receiver::Orchard(address.to_raw_address_bytes());
                    if !za.matches_receiver(&receiver) {
                        return Err(ZcashError::InvalidPczt(
                            "user address is not match with address in decoded note".to_string(),
                        ));
                    }
                }

                let is_dummy = match vk {
                    Some(_) => false,
                    None => match (action.output().user_address(), value) {
                        (None, 0) => true,
                        _ => false,
                    },
                };

                Ok(Some(ParsedTo::new(
                    if is_internal {
                        "<internal-address>".to_string()
                    } else {
                        user_address.clone().unwrap_or(ua)
                    },
                    zec_value,
                    note.value().inner(),
                    is_internal,
                    is_dummy,
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

    match parsed_to {
        None => {
            let (address, is_dummy) = match (output.user_address(), value) {
                (Some(addr), _) => Ok((addr.clone(), false)),
                (None, 0) => Ok(("Dummy output".into(), true)),
                (None, _) => Err(ZcashError::InvalidPczt(
                    "missing user address for Orchard output".into(),
                )),
            }?;
            Ok(ParsedTo::new(
                address,
                format_zec_value(value as f64),
                value,
                false,
                is_dummy,
                None,
            ))
        }
        Some(x) => Ok(x),
    }
}

fn decode_memo(memo_bytes: [u8; 512]) -> Option<String> {
    let first = memo_bytes[0];

    //decode as utf8.
    if first <= 0xF4 {
        let mut temp_memo = memo_bytes.to_vec();
        temp_memo.reverse();
        let mut result = vec![];
        let mut found = false;
        for i in temp_memo.iter() {
            if *i != 0 && !found {
                found = true;
            }
            if found {
                result.push(*i);
            }
        }
        result.reverse();

        return Some(String::from_utf8(result).unwrap());
    }

    if first == 0xF6 {
        let temp_memo = memo_bytes.to_vec();
        let result = temp_memo[1..].iter().find(|&&v| v != 0);
        match result {
            Some(_v) => return Some(hex::encode(memo_bytes)),
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

    #[test]
    fn test_format_zec_value() {
        let value = 10000;
        let zec_value = format_zec_value(value as f64);
        assert_eq!(zec_value, "0.0001 ZEC");
    }

    #[test]
    fn test_decode_memo() {
        {
            let mut memo = [0u8; 512];
            memo[0] = 0xF6;
            let result = decode_memo(memo);
            assert_eq!(result, None);
        }
        {
            let memo = hex::decode("74657374206b657973746f6e65206d656d6f0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000").unwrap().try_into().unwrap();
            let result = decode_memo(memo);
            assert!(result.is_some());
            assert_eq!(result.unwrap(), "test keystone memo");
        }
    }

    #[test]
    fn test_decode_pczt_to_p2pkh_transparent_output() {
        let hex_str = "50435a5401000000058ace9cb502d5a09cc70c0100d989a80185010001227a636173685f636c69656e745f6261636b656e643a70726f706f73616c5f696e666f144951eeff9ccf4eb390ff94a60aa5673db189a8010001a08d061976a9149517c77b7fcc08e66122dccb6ee6713eb7712d2b88ac00000123743158547742385031783459697042744c6850575331334a50445135524d6b4d41364d01207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666f0100000000fbc2f4300c01f0b7820d00e3347c8da4ee614674376cbc45359daa54f9b5493e010000000000000000000000000000000000000000000000000000000000000000023ad18a78e48f81fe95b3569486ee1db9eed90a319fac6faea1eed4e35b936717236c4a092e80c35e67e0b51b7a41de4013eaffed855b138934b9dcf28ad51f1ea51f60210a8d7f4f6ffb848bbafd4cdfd09df400e53c595861cc0dd8afc32dae010fba942a1bc32f2cc78408be742cd13846bbd50f3869c09d59bffc4e758949b56f422def9eb9d491721b9fc198edf183e9c32920eb30c476f236009a16355d1401b169abb973b87b0fd009693c349a5fdccaa2f1a266ea99f4e7f1fa13d2a8ec96023d73d0573b6fcdbe1b95010001986da568298f3de75fc0bf29de32aec6f2dada30c4e20286eb23667048d21d29017dd8c0825f1f2bc7aac48163e06f44024a657495e7c75fd1d2192336eef159640181679891b0cc1eaf42df83756d69c6833d552e0faa6d3c6af1405f2fcd4fbe0c360dff7f03b6da416da80b533cfb442be30185c9a7f4cba15f7fc836ccbd5d3e88daccb4be13ee8b9f850e542106ecb8107538ea23f4e21882e0c3fa46c0a228018daff38304e9759e20f4bb6f424a1040b554548feaf87e285a0a2ac9de930047e4bdc7042e08965e83fee3c20b18e3a7119457853e42b704f3df3449da1c365804abe0dd16ea51dbe4502e425ef8bb0103a9c314b041a9bf531facdc6ccab8404459261003b77426a46dff084bc2ff8f9a3da8c250bdd0ba8419d086b88fb25285d4fe8d047041cd2be273e8ccfabe154a818f491ea3fb9b9d854f4935480aa253fe35882ddfcc90b2b71eaa44f5299a34a946993fef33bc55c87a5f3427ac646f42eabe190c6856f4c8c1d1f505262b85353de115b061e2a1df24e69ec911e8f1c78b5c0716b8f01d16c560e3b81a07b3030c3beb15613b23e925c9456a9bfee1a25ef51f1ce9bf853c1264f6dd6fa2ea7087a8a78051626518afc8e88a1ed226f0cec1159a4a5ec4131041b3d8cc58ba10c7f676f52805be117a5689fa1a474e3f5bce38349f10e63ba7721a019ced0b63be870cef6f9ef2d228d8a58e695e871f7c4b0e94445d5255708c50298b13c55044f8f5995a258826291eda6f1671681e8f101aad68a17330a3faf4017f87b315aa53bb57fb785953dd9a480c85dd1182fcb235df28c7d3f3d8b79fe72f3b42d63c7ae563f0a4df22de593a488ee514dbda39291a69a0fd09a17ca6786cba733fdadb0f785147676378e7fbdd2a00a45b14460f415578abf268306554e68261022eb4c1e60df1566ac1853d01b2685924329e17963014ba9cd39f6c1c7f7e169574bab623fc15066223196ce0b1ef2251e5dc1f56707910d825c1451bf98115afd2dff4800fcfefc74c237b7bfec5cc2bf20e158f09cbd482b2f17cb0dcef82660ff0f697459e9cb1fdc77a10cffa9b91d88c3791b4a47e11781e0a7e1215315900a40bad41570d70ed6e7598d48eb8956ef90afda546e823446a3cb65a80f1407ea065d196f07524561be836cb5de0b9d5ba3665d2abf29c30bcef800dbeee9e293c59e4c5cee22984e205ebd67147ed3bf8234e654e32053e16bfe2cdeb70f83e1288793284056aac734eb8593feae99c48036b3d852f228febddd3c402bc5ae92d1e1adc313a53eae14213d67afb688efe0c2cdbff9ff826ad0b6cb2d8907949fbacc71fa539a9c60e6c3e2b79ba68b4b73b5b7d6f6e8f08ab80eb53099df60f6bd0c06d9b1e701c7471b9e9453bc78fa00ecaa28a1dbc9b56d2b0750eb5d038d58257a94fc57072f676555b9120536834309db16e168d8dc3f2b8680435853f216b576e39e9ea212f3fb2804c85065d801cf77c2586a76c404023554755ee41cffc358a30d67a26f758979f7f52233a680f0fa64ecdce560dfb206f232ec18450806a8fd8eccf23ea0cbd2dbe3374ccce23449d58aef4512452a329c037f96294f7a97a99fd7721e7bf07e32940b96dbb29e91a6bf267dfecf3e7bccaa657d405d2e7fd76ff8d51203b532a54868089c81d010b3b1b123a1303ec7643dbd09c1f396bc6915f6c4a02cdeba9becd724bb1f42e000000e0ebfc0224e6821685772ac6261127fa8dc097c4ce073ae9e468dbf8edb67a0131399b289425cf7741a1e779f1a089700276df5b193e1822bbf4416c89d4bc94c404b2ca1419e8f7cd98ed4f692efa5f01cc21558245469cfb558da73c13b42074ac966cc41733ffdfcade01c68062a0e0b4a74a3ab15123d62a6e270ea03c2ce88d337034e9537d7f871e8b1363b14096fd4ceb6ba46b2f7308ee1e0824eadd073dbdad58da08aa58b87fa7328710759597fe5a70516299c4a4302888b974da65c259cf8b3b4a1b7207e4d30f4b0f97e48702a25e17d51fa7ea2889d5926d9c66a151d8c713f267f95e0e730d89dbf2140cddcdc2d3508fa7902b1c360244ec407723929891f3990577a7478d4e0e3e374e4fc59e2ee704fca7a51196170e517eb74c339b016eeb3ff49ce10c13b217bbea0f4c234e4fe2766a788cd23000fe8920f973227ef987104d4d458f7754ed14fa2c798d7081d9311e0109c27ad89f362b24818cd803a600b2f21cf36e321c89ac785268e566e850d4c328eefce8062bbad81a854e08fa9a48f110f00072523cb6a670c52891af6e893ac010d4cf540a7ef598437803488d203de92966eb8ca9594d36ccb023d229ac8c391f4b9d50a569890052f27d9271f122c10f78b4c94c256f1e67a9e9c20779d34a0898daedb972ccee455ebb0e48b39d405f74f9074b943c5b5a38612e94bc8c82ec6176b90156289a99f816f64a5cd8cdfd8a9d1de4158e144869e3c535d1928f14d76d6a806b8730b92ada3353ededeb8c2151e876c566121ea2a6cfc93e3da2b1780b827ed7e5420c716b52b7811e650befc972685d153ac68ec9bf7d8b0a09659c5b36bd8226796770ebc05a6b9057a2c82feaae40a281edec65993cd792c980a845d2fbff8189d288500875f768205053e84b399b1a6141f380b7cfc235f26d3ce316cf2afae0404be86ea803a2733c9d32478c822dfa905a4e360c5fef6e05419088fe8921392c4366f976ecfd33332c0273e6ad6770a7a993010bf78b0565a3aa4fbb531de5fbd44a6caa636b2c03f0a38250bf599389154c4a55e4599a13b719551486bf010001587617b07bb6120ac6886c2a4641b8a0b9180ebd3375cf55e1f4066464ad43450000000001e902c227e9c4f4206e1f917c596fe9f4e6d2dca81063093d7065a70ed79674052df49c10eb57f2dda99dee1c1c01e9ed7efd1c9f6f971bf62906a97830b7ad2beec2cfcb30451b6c157fd144041e0429fa1aa2f9f7d0e84e74014205f0a9bd08171a83f92b3493fe29fa0802eefab10ecef3904aa569d82f6e95fdf2dc9cdd28000114b0cac3e14c4e1a4a4fd22c98563b9cbe859c67768a39f67abd5dbe55d3d16ff58e5c45b21d8c573f54b901a8ce920701de6d3d52208c36d6a557c3ed5843cf85a03b029ec491b8ad2541db678ab25a3d0125056d422f2e1393ba7e5295422e9ff38ce8276124ae0fec4bb06e147be83a8f017d6d7535f6e3eade13042662d05eefa91d1f4757fcd0b4b844554e77366f4b36a7068e15c2f06aaa11f6bcfd2e9e36231530069f7fb99a2771050689f24f332b427fdd05d1d5f3219230a1406f0955e3ddc0ef5ce2c7f160d797f3e54ef5782001cfdcac17ee2155c2c8b84c2ddc976891132206e97f7296033a7ef7eaa12ef214c4abf71d1439d50cefd21bedf293df7e11e187c64e13dd9a7905a1e2a26d2647cc1004394bba40a982173a00559da43ccad644d32eb4c4f785264fe3ebd9b12a1d7a6c31aab9e988f717752dbb6d8f555dcf360e25f71a7b1d95ab6b54761c3774882f16455a42d4ee6692b641bd0ccf14d81b56f0503932377bf5fae7d5a9c103c10e2ccd48e8778e0b217e8255746c998108508a6e81e1f1c1992ccd65ee9bfb196e0f7257c5dd5392cc0947e0b77bd30e18165c8a3541e62949e40c88c90fdf827133cbd17ab082f151d390180ca92cf0d8c4bd08e01b2f4f50b18bd589171d96723d00a3716420df0a98200f20a5136f36e955b51946a471eb029951e42268c00c34c53b660d2c636bf39e6f67e8933002c7b64fb410cd5e07cdac9e47d9d21c150a6fac33462496e7df033277558b54ac238f073776cb533551ced42caeba694f31fd8d40ccb9fb2e104aa0357a122141052c9a741c4c2c736b6dd0363ddbf27e2922ae2800cb93abe63b70c172de70362d9830e53800398884a7a64ff68ed99e0b9d2e26bdef115ced7bd36305a54386996133c4e65759f3731637a40eba67da103f98adbe364f148b0cc2042cafc6be1166fae39090ab4b354bfb6217b964453b63f8dbd10df936f1734973e0b3bd25f4ed440566c923085903f696bc6347ec0f6f3f63aab58e63b6449583df5658a91972a20291c6311b5b3e5240aff8d7d00212278dfeae9949f887b70ae81e084f8897a5054627acef3efd01c8b29793d522ca2ced953b7fb95e3ba986333da9e69cd355223c929731094b6c2174c7638d2e60040850b766b126a2b4843fcdfdffa5d5cab3f53bc860a3bef68958b5f066177097b04c2aa045a0deffcaca41c5ac92e694466578f5909e72bb78d33310f705cc2dcaa338b312112db04b435a706d63244dd435238f0aa1e9e1598d354708102dcc4273c8a0ed2337ecf7879380a07e7d427c7f9d82e538002bd1442978402cdaf63debf5b40df902dae98dadc029f281474d190cddecef1b10653248a234151f91982912012669f74d0cfa1030ff37b152324e5b8346b3335a0aaeb63a0a2de2bca6a8d987d668defba89dc082196a922634ed88e065c669e526bb8815ee1be8ae2ad91d463bab75ee941d33cc5817b613c63cda943a4c07f600591b088a25d53fdee371cef596766823f4a518a583b1158243afe89700f0da76da46d0060f15d2444cefe7914c9a61e829c730eceb216288fee825f6b3b6298f6f6b6bd62e4c57a617a0aa10ea7a83aa6b6b0ed685b6a3d9e5b8fd14f56cdc18021b12253f3fd4915c19bd831a7920be55d969b2ac23359e2559da77de2373f06ca014ba2787d063cd07ee4944222b7762840eb94c688bec743fa8bdf7715c8fe29f104c2a014c18207b76f3808351694eae9a99f8d7786e4c3e6b0c3452a518b0375deb0829012fac20755b7bb7c99d302b782ea36f62a1b0cfe8d7d4d09a58e8ba5da26f457803a080808008858180800880808080080000c86d0beb146429ab2ddc5e2b67b68cd0fa540c8a2c1637cde3220874577fd72337afa5c4823cffe1c5c57ba90eb737f081827bbf51437a2c420afa809bb04f3cc4046b05a8223b1b1114958bc0e10ecb6ae0b383ebd22f686f57d2f905acca999ae1e85f85acc5cb5b517b4233d3db94dc05259c76e8a04ae5d84f4331348388387edd327e40ae6b542f5b92cfa0a55f01ba9ba3f0035d64311f55042c1b86a8178f3ce47592cc1cdc3d4dfcbe66b267906a2c38313651863037d5fb3aeb4fcb85cb06e489536fe35784e5a1c0bc9a8083fd43ca2aeb18881caa02e9bde0a29ebb0ed1687299d97ce49bb6545050756fda15ee31c9cd947bf9019d90db96e89e3ee3e63717c34b485530590387b8bd2f57adc2c5b2fea35209ea22b4e2cb5e2d65e1f56cd1f16e5954bfb8425826cd87b75e57262d710bd1d5c9bd3b4a2c99a89926cc32c59e16ceb64698e1bcd82ae21d02ee4cb67e814861cd22810a0adaff558df41125e37179d16adc7cd4e1296bd31f44290e8c218664074158e724aee81a5ee5fb7f16852263b6902521c90dc4380b54aaf700a1ca6bd93a22ec1fd062f14b32f6d2d6ff51e151bfda4ccd569bfb966d294be0ee61dae648877e25b0841a27d5c224d4fd949926d4dfde6d28b7d14e16ae60d2112a79da714bb454a9f6a034a191c659fcd0c20a35d85f18b8700a29c5cb9c386f2afb10e8fafa892c3a1c5fbfee08cd58610339b7222f5945e775cfbe87089f48081b38775541cadeebbd5b51ee981b9558a0d4e01a0fba29d0b50fa9b843db2dbcc25071352041a199d7a85d5bd956d7f61db4a95cc26b1709fa48c0eba34676ee7f855b70ea4f8657f6f00180b43be23c6edd3259a84b873d560f60f5a7d7fd54b0330f835398c4ef2bb3a61d2fae5088b03c542ac58f663ad15cb471e39f6f06d2a47cd696bda59923f64718e81a5438f1711d43e284b9c566e596dc77f1e0809f96d40f76804c265ab9654c1ff8c18a1e8410164d09ae5bc1dd982eceb57c0114b0cac3e14c4e1a4a4fd22c98563b9cbe859c67768a39f67abd5dbe55d3d16ff58e5c45b21d8c573f54b901f0cb8b0701cdf9bded0827a82dc56ad98807f9c96ca814b2651a6b82d22a5c10d5fc80cdbd00000001207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666f11024951eeff9ccf4eb390ff94a60aa5673d015249a562d4ea0bc08e26f627c4a418d274e930230cab2139e2766d2654f0ed3903b882070039fb66568096852da4cb54410485be43a51a0269351ed32433ed7ddd1b12d43b00013b4c678abdaf00e1fc4587a41d1402c75bbc0dcc1c0e2b7652dc14352b87623f";
        let pczt_hex = hex::decode(hex_str).unwrap();
        let pczt = Pczt::parse(&pczt_hex).unwrap();
        let fingerprint =
            hex::decode("2fac20755b7bb7c99d302b782ea36f62a1b0cfe8d7d4d09a58e8ba5da26f4578")
                .unwrap();
        let ufvk = "uview10zf3gnxd08cne6g7ryh6lln79duzsayg0qxktvyc3l6uutfk0agmyclm5g82h5z0lqv4c2gzp0eu0qc0nxzurxhj4ympwn3gj5c3dc9g7ca4eh3q09fw9kka7qplzq0wnauekf45w9vs4g22khtq57sc8k6j6s70kz0rtqlyat6zsjkcqfrlm9quje8vzszs8y9mjvduf7j2vx329hk2v956g6svnhqswxfp3n760mw233w7ffgsja2szdhy5954hsfldalf28wvav0tctxwkmkgrk43tq2p7sqchzc6";
        let fingerprint = fingerprint.try_into().unwrap();
        let unified_fvk = UnifiedFullViewingKey::decode(&MAIN_NETWORK, ufvk).unwrap();

        let result = parse_pczt(&MAIN_NETWORK, &fingerprint, &unified_fvk, &pczt);
        assert!(result.is_ok());
        let result = result.unwrap();
        assert_eq!(result.get_has_sapling(), false);
        assert_eq!(result.get_total_transfer_value(), "0.001 ZEC");
        assert_eq!(result.get_fee_value(), "0.00015 ZEC");
        let transparent = result.get_transparent();
        assert!(transparent.is_some());
        let transparent = transparent.unwrap();
        assert_eq!(transparent.get_from().len(), 0);
        assert_eq!(transparent.get_to().len(), 1);
        assert_eq!(
            transparent.get_to()[0].get_address(),
            "t1XTwB8P1x4YipBtLhPWS13JPDQ5RMkMA6M"
        );
        assert_eq!(transparent.get_to()[0].get_value(), "0.001 ZEC");
        assert_eq!(transparent.get_to()[0].get_is_change(), false);
        assert_eq!(transparent.get_to()[0].get_is_dummy(), false);
        assert_eq!(transparent.get_to()[0].get_amount(), 100_000);
        let orchard = result.get_orchard();
        assert!(orchard.is_some());
        let orchard = orchard.unwrap();
        assert_eq!(orchard.get_from().len(), 1);
        assert_eq!(orchard.get_from()[0].get_address(), None);
        assert_eq!(orchard.get_from()[0].get_value(), "0.14985 ZEC");
        assert_eq!(orchard.get_from()[0].get_is_mine(), true);
        assert_eq!(orchard.get_from()[0].get_amount(), 14985000);
        assert_eq!(orchard.get_to().len(), 1);
        assert_eq!(orchard.get_to()[0].get_address(), "<internal-address>");
        assert_eq!(orchard.get_to()[0].get_value(), "0.1487 ZEC");
        assert_eq!(orchard.get_to()[0].get_memo(), None);
        assert_eq!(orchard.get_to()[0].get_is_change(), true);
        assert_eq!(orchard.get_to()[0].get_is_dummy(), false);
        assert_eq!(orchard.get_to()[0].get_amount(), 14870000);
    }
}
