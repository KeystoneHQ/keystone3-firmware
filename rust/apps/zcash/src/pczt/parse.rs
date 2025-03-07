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
    zcash_protocol::{
        consensus::{self},
        value::ZatBalance,
    },
};

use crate::errors::ZcashError;

use super::structs::{ParsedFrom, ParsedOrchard, ParsedPczt, ParsedTo, ParsedTransparent};

const ZEC_DIVIDER: u32 = 100_000_000;

fn format_zec_value(value: f64) -> String {
    let zec_value = format!("{:.8}", value / ZEC_DIVIDER as f64);
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
            ovk,
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

/// Parses a PCZT (Partially Created Zcash Transaction) into a structured format
///
/// This function analyzes the transaction and extracts information about inputs, outputs,
/// values, and fees across different Zcash pools (transparent and Orchard).
///
/// # Parameters
/// * `params` - Network consensus parameters
/// * `seed_fingerprint` - Fingerprint of the seed used to identify owned addresses
/// * `ufvk` - Unified Full Viewing Key used to decrypt transaction details
/// * `pczt` - The Partially Created Zcash Transaction to parse
///
/// # Returns
/// * `Result<ParsedPczt, ZcashError>` - A structured representation of the transaction or an error
///
/// # Details
/// The function:
/// 1. Parses Orchard and transparent components of the transaction
/// 2. Calculates total input, output, and change values
/// 3. Handles Sapling pool interactions (though full Sapling decoding is not supported)
/// 4. Computes transfer values and fees
/// 5. Returns a structured representation of the transaction
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

    parsed_orchard.clone().map(|orchard| {
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
    });

    parsed_transparent.clone().map(|transparent| {
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
    });

    //treat all sapling output as output value since we don't support sapling decoding yet
    //sapling value_sum can be trusted

    let value_balance = (*pczt.sapling().value_sum())
        .try_into()
        .ok()
        .and_then(|v| ZatBalance::from_i64(v).ok())
        .ok_or(ZcashError::InvalidPczt(
            "sapling value_sum is invalid".to_string(),
        ))?;
    let sapling_value_sum: i64 = value_balance.into();
    if sapling_value_sum < 0 {
        //value transfered to sapling pool
        total_output_value = total_output_value.saturating_add(sapling_value_sum.unsigned_abs())
    } else {
        //value transfered from sapling pool
        //this should not happen with Zashi.
        total_input_value = total_input_value.saturating_add(sapling_value_sum as u64)
    };

    let total_transfer_value = format_zec_value((total_output_value - total_change_value) as f64);
    let fee_value = format_zec_value((total_input_value - total_output_value) as f64);

    let has_sapling = !pczt.sapling().spends().is_empty() || !pczt.sapling().outputs().is_empty();

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
        let parsed_from = parse_transparent_input(params, seed_fingerprint, input)?;
        parsed_transparent.add_from(parsed_from);
        Ok::<_, ZcashError>(())
    })?;
    transparent.outputs().iter().try_for_each(|output| {
        let parsed_to = parse_transparent_output(seed_fingerprint, output)?;
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
    //P2SH address is not supported by Zashi yet, we only consider P2PKH address at the moment.
    match script.address() {
        Some(TransparentAddress::PublicKeyHash(hash)) => {
            //find the pubkey in the derivation path
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
        _ => Err(ZcashError::InvalidPczt(
            "transparent input script pubkey mismatch".to_string(),
        )),
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
        Some(TransparentAddress::ScriptHash(_hash)) => {
            let address = output.user_address().clone().ok_or_else(|| {
                ZcashError::InvalidPczt("missing user address for transparent output".into())
            })?;
            let zec_value = format_zec_value(output.value().into_u64() as f64);
            // we only consider the simple p2sh script at the moment. multisig is not considered;
            let is_change = output
                .bip32_derivation()
                .first_key_value()
                .map(|(_, derivation)| seed_fingerprint == derivation.seed_fingerprint())
                .unwrap_or(false);
            Ok(ParsedTo::new(
                address,
                zec_value,
                output.value().into_u64(),
                is_change,
                false,
                None,
            ))
        }
        _ => Err(ZcashError::InvalidPczt(
            "transparent output script pubkey mismatch".to_string(),
        )),
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
                let parsed_from = parse_orchard_spend(seed_fingerprint, spend)?;
                parsed_orchard.add_from(parsed_from);
            }
        }
        let parsed_to = parse_orchard_output(params, ufvk, action)?;
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
    use super::*;
    use zcash_vendor::zcash_protocol::consensus::MAIN_NETWORK;

    extern crate std;

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
        assert!(!result.get_has_sapling());
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
        assert!(!transparent.get_to()[0].get_is_change());
        assert!(!transparent.get_to()[0].get_is_dummy());
        assert_eq!(transparent.get_to()[0].get_amount(), 100_000);
        let orchard = result.get_orchard();
        assert!(orchard.is_some());
        let orchard = orchard.unwrap();
        assert_eq!(orchard.get_from().len(), 1);
        assert_eq!(orchard.get_from()[0].get_address(), None);
        assert_eq!(orchard.get_from()[0].get_value(), "0.14985 ZEC");
        assert!(orchard.get_from()[0].get_is_mine());
        assert_eq!(orchard.get_from()[0].get_amount(), 14985000);
        assert_eq!(orchard.get_to().len(), 1);
        assert_eq!(orchard.get_to()[0].get_address(), "<internal-address>");
        assert_eq!(orchard.get_to()[0].get_value(), "0.1487 ZEC");
        assert_eq!(orchard.get_to()[0].get_memo(), None);
        assert!(orchard.get_to()[0].get_is_change());
        assert!(!orchard.get_to()[0].get_is_dummy());
        assert_eq!(orchard.get_to()[0].get_amount(), 14870000);
    }

    #[test]
    fn test_decode_pczt_to_sapling() {
        let hex_str = "50435a5401000000058ace9cb502d5a09cc70c0100ec8ba80185010001227a636173685f636c69656e745f6261636b656e643a70726f706f73616c5f696e666f144951eeff9ccf4eb390ff94a60aa5673dc48ba80100000002ac338dcff9cf137ba61ae13c992b53109861de0e794f41ba2399af570c768ae76f5c33dcb56261068f5413a002d03086a41d2dd40d6bc41ad2b8eb1831c3cc65ea183dd3a9cf918cc71b00d97a83263c9a42d277c154450976b9fc4404ff8322c404b2f3c896db40d43126376480272a057830c30855b16f7a2e8a84160a92d10c893f6bad7cf2c4f8f13e01a109c8b62b0f01f416b29e01d08e4053b596c6a2d9ca56b51c92a644a25449848f8c82945ed7ccf67045a25c5180b2f13a10b7f0137d543cabd753aa3f09a41f1b0318bbe608a50158d74dfe702f418e1ca2c57809de09dc51a255d7a83a45ce3dcaec15adbfdd5d221e1a3e43fd740c05ae24f41ebd7df84732602276708710969a1c4589fbafda4a2e8be96657a27c4dd33da64f583b755ab9f8b9cc3ac4898d175160cae430f697ddd2c04ff7103e45897d8caf52a106c807e9e639b7307776713266e448a826a5027d12c7b1b96a5b5057b7a66c10b2eaa366524db0221e025c984f72b5652bbb3e887e47bee059f8791184806ffd3ddeb167d41d16b6d748781d7cf7c8474ec406cc6925f9fc1f2a25f327147831b0432d930ab177678cbb5cd9ba646812c42a39891331858ed1877ae6fddf67183cbf3331c1ee0da60689d7019f714033742d5e036fcc6fdd298023a5450f59d57755f3d7684bb929f6a146bba970f3787aaf302ef8270fdd4670d80148d3240c1c00ac6e6a36eade8a6cad9576678ddd9f316653a541030e6131577caab1cf1837473de30552b18ca47cca65a79a6e077a1df3f8241ea3dc169df1f1f5fdb671be38a9a963e9d6197a28f30d94c99f2d2712a1fa090d29a2c091f11e58378331c3460beb25d76bdb6a050efc03474fc223f1f90a491096935ff8579890b4151dc824e214ee45506ffaee400b6ba7865704d5a8ad8bd9d6545e9eead4b0be8a0994d47750d8b5a130806ae84dbcf552763954c3313000df63a0b9b0186f4756dd86c02ac54da43648e502f99e0704a66b8ff3037b6ce3c531a81fbbf83cf526e5130666000cdff65015dfdeb5fa7a6c80e4d4477c00016eda1757b0b4c2f1851ef53408cdd09b6965ad895fd9d4b1fd0617a0b0ac88659182f086a85963f8cc976001a08d060184f4595f43b2229d250ccf8941e7972b36ae3f3c7b2492c5a452a6d60da5fb97012b5c6bb24cc8d10f1d8f093ea3a78279d2e6914a1cba7eceff5a0b97c8b7bc040000014e7a7331646d6470773461736b6e70307270673737353671336e77736e64356b74747666746c766166763061716374367076397633706a6572716873733635396a636c63656a746b7175716674723401207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666f01007e67b65e0e9bca4567ccb50d8978da4eac6f2632fae722539d8bac8b3705cae2e35265414af4c9aec1f314decff9de81c795d1efbf62f9fbc4835062cdf48f5b1821183721a30bdc256c8817c1bab47765ac92104f8575134b9dbfd8f626150dc404c79c1e4914b8e1583f30dddd622fd852075e31b3435f7674ee199a3e439340ed647d6feaaa3d61b56cf4a9871c1d2dd16b156e9d623245ce794d30f06fbb118858cf2d80e924f4953c47c8938969f5d1b97371a4089972166a338efd78bb650b7868f0f9c011be13403e4995a0e454f8b379b684553bb34b2dff48d5475c4a7a97a13c3100ad75a1296b066cd6a2db0a6053c7ad6f81a717f2f51dae0b96695e2936fc7c979e524eeab8ebcbb6638f3d68c480d3fba0fecd1bf0b2efd5254102c8d97db89d67eaf49e5be32549c3626c3a1874c9f695f080bafd54ea4d80fec72bdec44c7648e27b2e1087cf1ee81c38ec01757157a1fe9ec8ee81901187c5bcabe3a3d1b7c2a05a63372aab06a21552fd7bc7431ae601ab307ed3451d111cb86a261291fefed4978558bbf62099d3e766b611cde5f207d95bc8ca9d50b460b14c8cb2dcde6b4a926537c6255e028a9423d98b76581cc9e99a5b0df20a43678b6ce4927e433c25cf00a3a4f97b8237cec8a18948791f4bb0b4362908ce8bcefd94eecc1e1c4319c47be8f234196a974bfbbbd76b8ddae7026526f27d5e5699e3aa534a042daaf9ecab6303d167a4cb9a3bd5703f3e135e6c45fba5fbe4f4889707206e5086b47af7e7dba7ac10b0b32db1a9caaccb2526500b9480ac9332bd4a4d496bc7eed630edf6a01de960fbabe766192de0d07b29b9ad9c029bf872914ab142a795991b7dffe8322de6b51cff4b4362070e1b2a753a1815702ea5035812e10ade2e3a0d310a8a25aac537251a2fa2a7d40c8168407ed6c1b3212eb0cdc47f0377dc50d795019499a348931c9d84a8dab8e720b2c8d42182809c234322f63d064eef4f80ce75f845b1347796b36c0cbaa99c0015b4dc30ae777e4b294ec862c07bace978564f4d7aa79ebc0268385205c4cc0f00016340cd133ad2995f980c9ebb6a14d1b8891aecadd897fb89ea4842b36d3bbf71d4594704ec94bc7673ed62010001772d3a1d8c04cf4051ee4e079ee662ea6e4ca8368247df928887b11a59c8d61701a1e5f40d9da687a1e0254d62cd004d56b512de67e4135610268c47a1e556fb0300000000bf9a0c47417e4bfba4d866f5b2762b342b430857c17a29138f4f59739390f61670590401ebea9616759f3d1f855b712c237898d67841c44e006d922783c8e02c3ca6c505029a5e3c582173eebd763c0c32746a46011381f223701ace23bcc5617b7405268550afd777a0138ca528fb44720600f6b81fb0e23b2ba4e1a21882e889c8ebf82f31166f087d042ae49cea162e499e9d0883b610108fb7f1397b3b0b4ee351290a0123f48d843320d31777010f37759faf689b1cc4882a5ebcce2e7bd3ce71805a33b87989c47ff7ab5707fc86452e2a49e778cace05026ed1acd2f1eca9b78a1f3501dc81e7da6e61186045a442a269bca8960b10df99233b1fca2262c5a6cd010fbcade4772250c8c99a620d1b0100013c72f350a5fb02a34d7aa9ba6580db86c5fb296c86adea6033cb95649a46d33a018cfa3b58f3d598ac9f509eaf9351c44a83211a79a04f17e5f154eb86addab01a01f08e7fbb25b69a456e293e9720fd9703b60c479f5e9468cc991fd7b4ac43700592ff2d75cec6cb7e9425226a8bb144e54e86c445c345fdc1fe1e3a0269d14c29e1f3b49fc0b4c5c95046eb46bdd6a6e8f42a48577733a3f10956a0091161db00018391cab603411efcfdf27b04a7db524e81dde69ddda3f5e6af2702d58ae82b3cb173c114059cf95e884d22eeac429f976f158ed027839eb5fbc8885b73c8f64ab89c618f13e9955cfd994cb1bb75bef1249df49815e0f13e351e64cded9e2bbfb11ef1bd261d5a7ba637c5eb8dd035e3e5c2292311f8c7a6fa92a846211becee977d8d232ee62ceb5ebeba78f05da856cd0be335770a87cb91b83b58843a3e311b4df3af1aca4d4be4a1bbdb2480d4efff6bc98af1f95b4d4c6a58fbd3742fa61db2a49933138b3786e9b5775ebdde58de68739c4ffd62d4e86759106f891eea8e2100792c45cb3b68ad07332100bb291ca1201c79ce0e04a76cf4814d62209004e70e500c34a70c9158d6015deb4475ebd593058a6bf82c31e13bd02e93ca51042e08492ce67e2a7df01eac78e7fb9eaca726dac17a27a993844da3f7b722350d4f62523d964ffdd390220ba1e99e5615405e7e97b6733734ade4ef4441c190be855a913c04ce0c2d537ee5b505764821e0bc667038bc938473b58b53eb37903fa4a32719df1fbf380d1b6d525da31982fd3cd03a0eef388a65b0a1649e4adf5c92144e0e65d4c3192f67d3b9909240f335d19d13a57c579f04e31c60d6b0ba362753be38fc0c5d40b6af321d5d6a6c1abbcec982af39d42394c34b61b808ed0bc36f9003ba5d9a122d53cc242c14bcedd30d6b5b303d24a859ef2d559378a916ac5e5d24fa17787d7c171c881514738abed0b757b67f297a1f167b1a3f43a552f4e37e1678caf66057dad1f384b918d8ad36d72ea1633aaa0248000370105e9766d7b300c4c303e72d0e83c3d52059df29ccdc62e6974ff4c6a3ee0a7dd03d7f8d48cb3e9620e83fa02a6ecbf95a3af9765baab0176aae8d507384abd291aba006e7de3c802e78d3d8db3dc718cea36159aca411d17aaf101bc55cbb7a681833e05bfb0f23d584f0f8db1b333445ea47be715be1fccff9e9337f9e740fd737b22a2afe2935134e67f989748cd5470a92ae12e7ece501c68900717b3a3aba6c513e2c4514d854d63fad26d3770485aeb5407a83fd249744a5995501a95123b33e4e80442c1db0b050714dc06491579ff7a8286a6c6d2c27edf8f6aa70667e2cc9af63f208535fb5ae610315797071dfced79f4a4ba943dc8ae9b4008d21de1c46abe2ba296d133d1387a3225425050c920165d85913e754c7430b07b5b6df842010ddd1306fc74f14de29640d8e41741d7791aa128085dd4421424e24a407c62976718d0c5846790cc3948dba055d0258a53b0d6617b72f0aa112f6f654f7a2f219f4481263f21a7d4132facab430b03b1f575494db945cca62d38cfdd00e86b2f20526171f6533b3f13400946c9902ee348a41b925afad8a9f695845277e394ad8ee3f26205b7a25825e3ceefd8d629e87a1af0ce6c45fcccac6ee32d2b64dd75a3c2c0601819cf447911878cb2b2d16cbca1bb7192652db8a549c1612838387954895572b000000b03dbb44ce10a905f792050b1aa8e285ef9ab1bc860db688afe9aa76554022165d4b0fec38ad2427118ecfb2d80b43f2e2f1d0bf58eea3b95d4735522d6f3228c4048a920344ceea4a719fdb41a052e537b76cc1303776255389fa92c2d481120dad733dfba897dff10a066697d41aef022577446ccea0634004af98ba8e7e4d832c69aa7596f706c1b3d0f1e7887836892ca862cbc45b891d302a2301b98ed4f8d7e148869d30104ed8f1467cd9b2217331d7e3874a25688edf9421172f7bb0cd56af6e890b633b849e6a7df2017dcd116707bf194c77529eccfc4e4de8cf81d2b94f54fb3ffe6dd87447abb0c926dc1fb6ce2e2fd684a1fc1b4dcf1c1245d93f7f79ed738c0a8d80ea6194039a388926f555b088d166f8f005c8eea2d120230951d0f0528c981babecc3be4f52f9cf932df91ee41335de3941db27e5a7a117ed2d16c69f2be3dbdc4df453894efc86d161a364b0f8443e829e57aabfa3eb3ea85ba58c924a186d326c3987eb535ad4176223621e6d339352fbff0dabc8aa18486433dd0b99fa5236f4e6bf353335ec1de5cfb8860fdc8ab0ec02ebf4689f5d51e9d7cb497ffd5bfcb93a42e97c12a518d79d5da9d11aed018e908be3c1ec5d1cb12067a33cf4dc710ad6186e577ec11dc8df077056bc7473909cfdd87b60e20bc010893d893f5a82e3941da1e39b731d79fda52b4022e79884a255d96312eef5bbe5fefa0999e63f543996c176a4ae9773c2306b349afc6148594052bb15dc5c006691442813d3369e52aeda9c305236959dc096aa544aa364b8a24d08d8a895ad7e7361ef7eb1c36516f4a13e1b7740b44f805e5ac744180d39129c8e8665799b2195ad2d96c55145c4af04af8bd76ed54e5452e5ca253fccaddb3829b26c1055c6d517f950741035b384e264bd677b5265079d0610de5e54a1d92e5d48fd705f38ac14c7e87c6281899b2f81ea4c8e4f62bf91e50e769f3cc7e2faa0c16aa666c9a942759ddbb94c4d07fc1b6859b15c3d11f9e8b7015fd4f8520a3d8fa00ccdeede53f443672815ae36466b733a006887ab60540ccc59fd50d4a9f1a759a63f8b010001e3996dce9674d98958bfd1e46cf28a54df3c0de71122cbb93288648bf75bb7320000000001cd4bffb62423806a435a31a774d639275e7f3be1ac7703091d745b8ce8a8292ee3a722e3194f6fe7bc2df7c88d94b5b98442b0368eeea9bb298ae35bfe05e186c2147137cbc46f2902e7cb51161120829bf48d59f274e6808ffb1a260de3b008d31ba22f275af783e39f511ecdc10d621e04a40cec76437cf97bec8728a0dc19000114b0cac3e14c4e1a4a4fd22c98563b9cbe859c67768a39f67abd5dbe55d3d16ff58e5c45b21d8c573f54b901c0b49807014bffc5036be15238fea26adbfca534330c943fad580fdc3f2b487ee5510aa91801846a55accbc6d7cf8a388ed5ca3ce07dd77ff88000259da019ba8daa61b15347017d6d7535f6e3eade13042662d05eefa91d1f4757fcd0b4b844554e77366f4b36a7068e15c2f06aaa11f6bcfd2e9e36231530069f7fb99a2771050689f24f332b427fdd05d1d5f3219230a1406f0955e3ddc0ef5ce2c7f160d797f3e54ef5782001a1e0ac172341eb105c1c4cd0b214c393110edf135827ffa908522c0adbe047f04121402edab64ddf965b98a2325fb204da47aa4ba418120c2077e0a0f083cc53a27273257f9ac2783b4b6a06b835e6110de28bb42059261263682dd2224accfbc1339e3c137904b7789ec0393f7e3fc510c4c3edf0120f6a176aeb721ef944c237368b2d5d3fab8c4f5841174817748561262841a0f7b5347e83f0ab07a54798ac548000f1c70354010ed9de251594dda8495ed7abd55babc4d6b56493562f0e37d3293c8a882758980efc55e338e8a004ad39dd72289d00423e8a4459eb74401cd84e0e4e14563df191a2a65b4b37113b5230680555051b22d74a8e1f1d706f90f3133bb3bbe4f993d18a0f4eb7f4174b1d8555ce3396855d04676f1ce4f06dda07371f4ef5bde9c6f0d76aeb9e27e93fba28c679dfcb991cbcb8395a2b57924cbd170ea3c02568acebf5ca1ec30d6a7d7cd217a47d6a1b8311bf9462a5f939c6b743073ef9b30bae6122da1605bad6ec5d49b41d4d40caa96c1cf6302b66c5d2d10d396e0183683f64ec039e3f3ecfd4753b55e83d3d70f635dc360a342d69256bfa349d2e26bdef115ced7bd36305a54386996133c4e65759f3731637a40eba67da103f98adbe364f148b0cc2042cafc6be1166fae39090ab4b354bfb6217b964453b63f8dbd10df936f1734973e0b3bd25f4ed440566c923085903f696bc6347ec0f6f3f63aab58e63b6449583df5658a91972a20291c6311b5b3e5240aff8d7d00212278dfeae9949f887b70ae81e084f8897a5054627acef3efd01c8b29793d522ca2ced953b7fb95e3ba986333da9e69cd355223c929731094b6c2174c7638d2e60040850b766b126a2b4843fcdfdffa5d5cab3f53bc860a3bef68958b5f066177097b04c2aa045a0deffcaca41c5ac92e694466578f5909e72bb78d33310f705cc2dcaa338b312112db04b435a706d63244dd435238f0aa1e9e1598d354708102dcc4273c8a0ed2337ecf7879380a07e7d427c7f9d82e538002bd1442978402cdaf63debf5b40df902dae98dadc029f281474d190cddecef1b10653248a234151f91982912012669f74d0cfa1030ff37b152324e5b8346b3335a0aaeb63a0a2de2bca6a8d987d668defba89dc082196a922634ed88e065c669e526bb8815ee1be8ae2ad91d463bab75ee941d33cc5817b613c63cda943a4c07f600591b088a25d53fdee371cef596766823f4a518a583b1158243afe89700f0da76da46d0060f15d2444cefe7914c9a61e829c730eceb216288fee825f6b3b6298f6f6b6bd62e4c57a617a0aa10ea7a83aa6b6b0ed685b6a3d9e5b8fd14f56cdc18021b12253f3fd4915c19bd831a7920be55d969b2ac23359e2559da77de2373f06ca014ba2787d063cd07ee4944222b7762840eb94c688bec743fa8bdf7715c8fe29f104c2a01fb1dfcae4233ee8eece0af6dfae15925a261b8814a22830570fcbcad85384c1b012fac20755b7bb7c99d302b782ea36f62a1b0cfe8d7d4d09a58e8ba5da26f457803a0808080088581808008808080800800004220db463466091bc1a7dde8bcc53ea875bcabe9cdef45bdda6ea4dce9830c398ad3a6cf27673cf9bc86ce3a502bea62fcf197599542a312aad33cdbf0976d06c40437e76543ef96f903074d8e97c0b551c7fb94249d358dd04d8337e00f0742e9e57eb3008d3e814188a9d2444c4e6094d3fc779af7a3a11b297b4af3064b2a3195daaf3d1e8d682f12824f799e6c46db73b5b7676ee9ceeae026cdf610bfd7a7b96cab6e27f1642cce2e5360b1fd0ee4b1b4a87ceb5d8a0b9e6e433ab94b16920d6b5c792614084879e7a0b7c47bc9ad0e637359a9651661e6ba963b5c311977f0276f5c2f791217b7ea0691bfd20271cf9492f2dcc2dd0951aa015d8c21062549d8adcba1b59ff9d9ef29932694a22e5d26542963efa5724f730e93f700b2a22910d49fe03386695997b86bc1fa44bf6532b8360e3eed30445aada9f1d8e9a0fa51c7d1717a60f7c70721f4fbf4335cb1182f22a1792d7d4c5e9ff2f449f9373d0acd3e413777da3489a43ef944765817a37c16b11967abb16dc13027c312c02b49d78bc30fed0efc54af79d0b9dc2dac1b062190890346f738dcbc562e8dc20e57dedfd0d8752e9651bee550c34900381528061d0d0f69348ff0c9a368bcefbfd307d59b01a9d91a533b8d309de686dc60fa6e31872ad25348f653a00619d035ae000a6fdb92d78067739058e272d60900cc0653440e2c907157df8b1681e254c94225b2c6b8fba7bb24cdfba0ebafe1697babc8f544e8d7312764fce4a485149f89c92112f63a2cf537f3c207fb50078fa20d0d00b8c093357f32ef5544ec7a8da37ee868b5c58a427913f11eea557e5830eb6a2882ec791dd5bb0aa4046ba716b6e03588d60bc39b7ad560c4642be0a8201665a78396735072c8ddb35781224cf9be6050fab075f24061973bdd2f2587503aef1a8ececd60cd58be6ee7807b7744c4a798326365d3cb561ac3b909984d1bb9fa5175f706dc145006612fb42f7e51ab7ddb02393d68a0f93c30c007d8a1082fcb480114b0cac3e14c4e1a4a4fd22c98563b9cbe859c67768a39f67abd5dbe55d3d16ff58e5c45b21d8c573f54b901808b910701f35323d2510d77264cce5f5fac679fccdcbd877939a2ab3006910a92a45295fc00000001207a636173685f636c69656e745f6261636b656e643a6f75747075745f696e666f11024951eeff9ccf4eb390ff94a60aa5673d01bdcb2a53666e628f2fd162b9c7db0fcdc2b668f1db89b62cf99455a34514742903c0a9070006f01fcf2202c1550a6bd0770a536233035d23e02b38ecb0bf94d02525708e08000189172a0a6aa69b6d9582ff56401903d22036a4d28801ba351609b12f2ebd9d17";
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
        assert!(result.get_has_sapling());
        assert_eq!(result.get_total_transfer_value(), "0.001 ZEC");
        assert_eq!(result.get_fee_value(), "0.0002 ZEC");
    }
}
