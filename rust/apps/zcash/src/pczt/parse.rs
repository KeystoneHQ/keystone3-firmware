use alloc::{
    format,
    string::{String, ToString},
    vec,
};
#[cfg(feature = "cypherpunk")]
use zcash_note_encryption::{try_output_recovery_with_ovk, try_output_recovery_with_pkd_esk};
use zcash_vendor::{
    pczt::{self, roles::verifier::Verifier, Pczt},
    ripemd::{Digest, Ripemd160},
    sha2::Sha256,
    transparent::{self, address::TransparentAddress},
    zcash_address::{ToAddress, ZcashAddress},
    zcash_protocol::{
        consensus::{self},
        value::ZatBalance,
    },
};

#[cfg(feature = "cypherpunk")]
use zcash_note_encryption::Domain;
#[cfg(feature = "cypherpunk")]
use zcash_vendor::orchard::{
    self, keys::OutgoingViewingKey, note::Note, note_encryption::OrchardDomain, Address,
};
#[cfg(feature = "cypherpunk")]
use zcash_vendor::{
    zcash_address::{
        unified::{self, Encoding, Receiver},
        ConversionError, TryFromAddress,
    },
    zcash_keys::keys::UnifiedFullViewingKey,
};

#[cfg(feature = "cypherpunk")]
use super::structs::ParsedOrchard;
use super::structs::{ParsedFrom, ParsedPczt, ParsedTo, ParsedTransparent};

#[cfg(feature = "cypherpunk")]
use super::ShieldedPool;
use crate::errors::ZcashError;

const ZEC_DIVIDER: u32 = 100_000_000;

#[cfg(feature = "cypherpunk")]
struct NetworkCheckedUnifiedAddress;

#[cfg(feature = "cypherpunk")]
impl TryFromAddress for NetworkCheckedUnifiedAddress {
    type Error = core::convert::Infallible;

    fn try_from_unified(
        net: consensus::NetworkType,
        data: unified::Address,
    ) -> Result<Self, ConversionError<Self::Error>> {
        let _ = (net, data);
        Ok(Self)
    }
}

#[cfg(feature = "cypherpunk")]
fn map_orchard_verifier_error(
    error: pczt::roles::verifier::OrchardError<ZcashError>,
) -> ZcashError {
    match error {
        pczt::roles::verifier::OrchardError::Custom(error) => error,
        error => ZcashError::InvalidDataError(format!("{error:?}")),
    }
}

fn map_transparent_verifier_error(
    error: pczt::roles::verifier::TransparentError<ZcashError>,
) -> ZcashError {
    match error {
        pczt::roles::verifier::TransparentError::Custom(error) => error,
        error => ZcashError::InvalidDataError(format!("{error:?}")),
    }
}

fn format_zec_value(value: f64) -> String {
    let zec_value = format!("{:.8}", value / ZEC_DIVIDER as f64);
    let zec_value = zec_value
        .trim_end_matches('0')
        .trim_end_matches('.')
        .to_string();
    format!("{zec_value} ZEC")
}

/// Attempts to decrypt the output with the given `ovk`, or (if `None`) directly via the
/// PCZT's fields.
///
/// Returns:
/// - `Ok(Some(_))` if the output can be decrypted.
/// - `Ok(None)` if the output cannot be decrypted.
/// - `Err(_)` if `ovk` is `None` and the PCZT is missing fields needed to directly
///   decrypt the output.
#[cfg(feature = "cypherpunk")]
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

        let note = orchard::Note::from_parts(
            recipient,
            value,
            rho,
            rseed,
            (*action.output().note_version()).into(),
        )
        .into_option()
        .ok_or_else(|| ZcashError::InvalidPczt("Orchard action contains invalid note".into()))?;

        let pk_d = OrchardDomain::get_pk_d(&note);
        let esk = OrchardDomain::derive_esk(&note).expect("Orchard notes are post-ZIP 212");

        Ok(try_output_recovery_with_pkd_esk(&domain, pk_d, esk, action))
    }
}

/// Parses a PCZT (Partially Created Zcash Transaction) into a structured format
///
/// This function analyzes the transaction and extracts information about inputs, outputs,
/// values, and fees across supported Zcash pools.
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
#[cfg(feature = "cypherpunk")]
pub fn parse_pczt_cypherpunk<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    ufvk: &UnifiedFullViewingKey,
    pczt: &Pczt,
) -> Result<ParsedPczt, ZcashError> {
    super::validate_supported_pczt(pczt)?;
    let mut parsed_orchard = None;
    #[cfg(zcash_unstable = "nu6.3")]
    let mut parsed_ironwood = None;
    #[cfg(zcash_unstable = "nu6.3")]
    let should_process_ironwood = super::pczt_should_process_ironwood(pczt);
    let mut parsed_transparent = None;

    let verifier = Verifier::new(pczt.clone())
        .with_orchard(|bundle| {
            parsed_orchard = parse_orchard(params, seed_fingerprint, ufvk, bundle, ShieldedPool::Orchard)
                .map_err(pczt::roles::verifier::OrchardError::Custom)?;
            Ok(())
        })
        .map_err(map_orchard_verifier_error)?;
    #[cfg(zcash_unstable = "nu6.3")]
    let verifier = if should_process_ironwood {
        verifier
            .with_ironwood(|bundle| {
                parsed_ironwood = parse_orchard(params, seed_fingerprint, ufvk, bundle, ShieldedPool::Ironwood)
                    .map_err(pczt::roles::verifier::OrchardError::Custom)?;
                Ok(())
            })
            .map_err(map_orchard_verifier_error)?
    } else {
        verifier
    };
    verifier
        .with_transparent(|bundle| {
            parsed_transparent = parse_transparent(params, seed_fingerprint, bundle)
                .map_err(pczt::roles::verifier::TransparentError::Custom)?;
            Ok(())
        })
        .map_err(map_transparent_verifier_error)?;

    let mut total_input_value = 0;
    let mut total_output_value = 0;
    let mut total_change_value = 0;
    //total_input_value = total_output_value + fee_value
    //total_output_value = total_transfer_value + total_change_value

    if let Some(orchard) = &parsed_orchard {
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
    }
    #[cfg(zcash_unstable = "nu6.3")]
    if let Some(ironwood) = &parsed_ironwood {
        total_change_value += ironwood
            .get_to()
            .iter()
            .filter(|v| v.get_is_change())
            .fold(0, |acc, to| acc + to.get_amount());
        total_input_value += ironwood
            .get_from()
            .iter()
            .fold(0, |acc, from| acc + from.get_amount());
        total_output_value += ironwood
            .get_to()
            .iter()
            .fold(0, |acc, to| acc + to.get_amount());
    }

    if let Some(transparent) = &parsed_transparent {
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
    }

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

    #[cfg(zcash_unstable = "nu6.3")]
    let parsed_ironwood = parsed_ironwood;
    #[cfg(not(zcash_unstable = "nu6.3"))]
    let parsed_ironwood = None;

    Ok(ParsedPczt::new(
        parsed_transparent,
        parsed_orchard,
        parsed_ironwood,
        total_transfer_value,
        fee_value,
        has_sapling,
    ))
}
#[cfg(feature = "multi_coins")]
pub fn parse_pczt_multi_coins<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    pczt: &Pczt,
) -> Result<ParsedPczt, ZcashError> {
    super::validate_supported_pczt(pczt)?;
    reject_legacy_parse_unsupported_pczt(pczt)?;

    let mut parsed_transparent = None;

    Verifier::new(pczt.clone())
        .with_transparent(|bundle| {
            parsed_transparent = parse_transparent(params, seed_fingerprint, bundle)
                .map_err(pczt::roles::verifier::TransparentError::Custom)?;
            Ok(())
        })
        .map_err(map_transparent_verifier_error)?;

    let mut total_input_value = 0;
    let mut total_output_value = 0;
    let mut total_change_value = 0;
    //total_input_value = total_output_value + fee_value
    //total_output_value = total_transfer_value + total_change_value

    if let Some(transparent) = &parsed_transparent {
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
    }

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
        None,
        None,
        total_transfer_value,
        fee_value,
        has_sapling,
    ))
}

#[cfg(feature = "multi_coins")]
fn reject_legacy_parse_unsupported_pczt(pczt: &Pczt) -> Result<(), ZcashError> {
    #[cfg(zcash_unstable = "nu6.3")]
    {
        // The legacy multi-coins parser only displays transparent data. Reject any
        // shielded (Sapling/Orchard/Ironwood) PCZT instead of showing an incomplete
        // transaction review. V6 transparent transactions are displayed normally.
        if super::pczt_requires_cypherpunk_support(pczt) {
            return Err(ZcashError::InvalidPczt(
                "Shielded PCZTs require cypherpunk parsing support".to_string(),
            ));
        }
    }
    Ok(())
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
        let parsed_to = parse_transparent_output(params, seed_fingerprint, output)?;
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
    match TransparentAddress::from_script_from_chain(&script) {
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

fn parse_transparent_output<P: consensus::Parameters>(
    _params: &P,
    seed_fingerprint: &[u8; 32],
    output: &transparent::pczt::Output,
) -> Result<ParsedTo, ZcashError> {
    let script = output.script_pubkey().clone();
    match TransparentAddress::from_script_pubkey(&script) {
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
            Ok(ParsedTo::new(
                address,
                zec_value,
                output.value().into_u64(),
                false,
                false,
                None,
            ))
        }
        _ => Err(ZcashError::InvalidPczt(
            "transparent output script pubkey mismatch".to_string(),
        )),
    }
}

#[cfg(feature = "cypherpunk")]
fn parse_orchard<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    ufvk: &UnifiedFullViewingKey,
    orchard: &orchard::pczt::Bundle,
    pool: ShieldedPool,
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
        let parsed_to = parse_orchard_output(params, ufvk, action, pool)?;
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

#[cfg(feature = "cypherpunk")]
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

#[cfg(feature = "cypherpunk")]
fn is_wallet_orchard_address(
    ufvk: &UnifiedFullViewingKey,
    address: &Address,
) -> Result<bool, ZcashError> {
    let fvk = ufvk.orchard().ok_or(ZcashError::InvalidDataError(
        "orchard is not present in ufvk".to_string(),
    ))?;
    let external_ivk = fvk.to_ivk(zcash_vendor::zip32::Scope::External);
    let internal_ivk = fvk.to_ivk(zcash_vendor::zip32::Scope::Internal);

    Ok(external_ivk.diversifier_index(address).is_some()
        || internal_ivk.diversifier_index(address).is_some())
}

#[cfg(feature = "cypherpunk")]
fn is_internal_orchard_address(
    ufvk: &UnifiedFullViewingKey,
    address: &Address,
) -> Result<bool, ZcashError> {
    let fvk = ufvk.orchard().ok_or(ZcashError::InvalidDataError(
        "orchard is not present in ufvk".to_string(),
    ))?;
    let internal_ivk = fvk.to_ivk(zcash_vendor::zip32::Scope::Internal);

    Ok(internal_ivk.diversifier_index(address).is_some())
}

#[cfg(feature = "cypherpunk")]
pub(crate) fn validate_orchard_user_address<P: consensus::Parameters>(
    params: &P,
    user_address: &str,
    address: &Address,
) -> Result<(), ZcashError> {
    let za = ZcashAddress::try_from_encoded(user_address)
        .map_err(|e| ZcashError::InvalidPczt(format!("user address is invalid: {e:?}")))?;
    za.clone()
        .convert_if_network::<NetworkCheckedUnifiedAddress>(params.network_type())
        .map_err(|e| ZcashError::InvalidPczt(format!("user address network mismatch: {e:?}")))?;
    let receiver = Receiver::Orchard(address.to_raw_address_bytes());
    if !za.matches_receiver(&receiver) {
        return Err(ZcashError::InvalidPczt(
            "user address is not match with address in decoded note".to_string(),
        ));
    }
    Ok(())
}

#[cfg(feature = "cypherpunk")]
fn parse_orchard_output<P: consensus::Parameters>(
    params: &P,
    ufvk: &UnifiedFullViewingKey,
    action: &orchard::pczt::Action,
    pool: ShieldedPool,
) -> Result<ParsedTo, ZcashError> {
    let pool_label = pool.label();
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

    let decode_output = |vk: Option<OutgoingViewingKey>, is_internal_ovk: bool| {
        match decode_output_enc_ciphertext(action, vk.as_ref())? {
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
                    validate_orchard_user_address(params, user_address, &address)?;
                }

                let belongs_to_wallet = is_wallet_orchard_address(ufvk, &address)?;
                let is_internal = is_internal_orchard_address(ufvk, &address)?;
                if is_internal_ovk && !belongs_to_wallet {
                    return Err(ZcashError::InvalidPczt(alloc::format!(
                        "{pool_label} output was recoverable with an internal OVK but does not belong to this wallet"
                    )));
                }
                let is_dummy = match vk {
                    Some(_) => false,
                    None => matches!((action.output().user_address(), value), (None, 0)),
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
                    alloc::format!("enc_ciphertext field for {pool_label} action is undecryptable"),
                )),
                // We couldn't directly decrypt a zero-valued note. This is okay because
                // it is checked elsewhere that the direct details in the PCZT are valid,
                // and thus the output definitely has zero value and thus can't adversely
                // affect balance. The decryption failure means that the `enc_ciphertext`
                // contains no in-band data (as is the case for e.g. dummy outputs).
                (None, _) => Ok(None),
            },
        }
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
                (Some(addr), _) => {
                    if let Some(recipient) = output.recipient() {
                        validate_orchard_user_address(params, addr, recipient)?;
                    }
                    Ok((addr.clone(), false))
                }
                (None, 0) => Ok(("Dummy output".into(), true)),
                (None, _) => Err(ZcashError::InvalidPczt(alloc::format!(
                    "missing user address for {pool_label} output"
                ))),
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

#[cfg(all(test, feature = "multi_coins", not(feature = "cypherpunk")))]
mod legacy_tests {
    use super::*;
    use zcash_vendor::{
        pczt::roles::creator::Creator,
        zcash_protocol::consensus::{BranchId, MainNetwork, NetworkConstants},
    };

    #[cfg(zcash_unstable = "nu6.3")]
    #[test]
    fn legacy_parse_allows_v6_transparent_pczt() {
        // A v6 transparent-only PCZT parses normally; it is no longer rejected for
        // being v6.
        let pczt = Creator::new_v6(
            BranchId::Nu6_3.into(),
            10,
            MainNetwork.coin_type(),
            [0; 32],
            [0; 32],
            [1; 32],
        )
        .build();

        let result = parse_pczt_multi_coins(&MainNetwork, &[7u8; 32], &pczt);

        assert!(result.is_ok(), "v6 transparent parse should succeed: {result:?}");
    }
}

#[cfg(feature = "cypherpunk")]
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

#[cfg(feature = "cypherpunk")]
#[cfg(test)]
mod tests {
    use super::*;
    use alloc::collections::BTreeMap;
    use zcash_vendor::{
        transparent::pczt,
        zcash_address::ZcashAddress,
        zcash_protocol::consensus::{Parameters, MAIN_NETWORK},
    };

    extern crate std;

    fn p2sh_output_with_matching_seed_fingerprint(
        seed_fingerprint: [u8; 32],
    ) -> transparent::pczt::Output {
        let hash = [0x11; 20];
        let script_pubkey = {
            let mut script = vec![0xa9, 0x14];
            script.extend_from_slice(&hash);
            script.push(0x87);
            script
        };
        let user_address =
            ZcashAddress::from_transparent_p2sh(MAIN_NETWORK.network_type(), hash).encode();
        let mut bip32_derivation = BTreeMap::new();
        bip32_derivation.insert(
            [0x02; 33],
            pczt::Bip32Derivation::parse(seed_fingerprint, vec![0]).unwrap(),
        );

        pczt::Output::parse(
            42_000,
            script_pubkey,
            Some(vec![0x51]),
            bip32_derivation,
            Some(user_address),
            BTreeMap::new(),
        )
        .unwrap()
    }

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
    fn test_validate_orchard_user_address_rejects_invalid_address() {
        let sk = orchard::keys::SpendingKey::from_bytes([2; 32]).unwrap();
        let fvk = orchard::keys::FullViewingKey::from(&sk);
        let address = fvk.address_at(0u32, orchard::keys::Scope::External);

        let result = validate_orchard_user_address(&MAIN_NETWORK, "not-a-zcash-address", &address);

        assert!(matches!(
            result,
            Err(ZcashError::InvalidPczt(msg)) if msg.contains("user address is invalid")
        ));
    }

    #[test]
    fn test_parse_p2sh_output_is_never_marked_as_change() {
        let seed_fingerprint = [0x22; 32];
        let output = p2sh_output_with_matching_seed_fingerprint(seed_fingerprint);

        let parsed = parse_transparent_output(&MAIN_NETWORK, &seed_fingerprint, &output).unwrap();

        assert!(!parsed.get_is_change());
    }

    fn p2pkh_output_to_external_recipient(value: u64) -> transparent::pczt::Output {
        let hash = [0x11; 20];
        let script_pubkey = {
            // OP_DUP OP_HASH160 <20-byte push> OP_EQUALVERIFY OP_CHECKSIG
            let mut script = vec![0x76, 0xa9, 0x14];
            script.extend_from_slice(&hash);
            script.push(0x88);
            script.push(0xac);
            script
        };
        let user_address =
            ZcashAddress::from_transparent_p2pkh(MAIN_NETWORK.network_type(), hash).encode();
        // No bip32_derivation: this output goes to an address we don't control.
        pczt::Output::parse(
            value,
            script_pubkey,
            None,
            BTreeMap::new(),
            Some(user_address),
            BTreeMap::new(),
        )
        .unwrap()
    }

    #[test]
    fn test_parse_p2pkh_output_decodes_recipient_address_and_value() {
        let seed_fingerprint = [0x22; 32];
        let output = p2pkh_output_to_external_recipient(100_000);

        let parsed = parse_transparent_output(&MAIN_NETWORK, &seed_fingerprint, &output).unwrap();

        assert!(parsed.get_address().starts_with("t1"));
        assert_eq!(parsed.get_value(), "0.001 ZEC");
        // A p2pkh output to an address we don't control is a recipient, not change.
        assert!(!parsed.get_is_change());
    }

    #[test]
    fn test_decode_memo_with_hex_content() {
        {
            let mut memo = [0u8; 512];
            memo[0] = 0xF6;
            memo[1] = 0x01;
            let result = decode_memo(memo);
            assert!(result.is_some());
            let hex_str = result.unwrap();
            assert!(hex_str.starts_with("f6"));
        }
        {
            let mut memo = [0u8; 512];
            memo[0] = 0xF5;
            let result = decode_memo(memo);
            assert!(result.is_some());
        }
    }

    #[test]
    fn test_format_zec_value_with_different_amounts() {
        let test_cases = vec![
            (0, "0 ZEC"),
            (1, "0.00000001 ZEC"),
            (100_000_000, "1 ZEC"),
            (150_000_000, "1.5 ZEC"),
            (10_000, "0.0001 ZEC"),
            (123_456_789, "1.23456789 ZEC"),
            (1, "0.00000001 ZEC"),
            (10, "0.0000001 ZEC"),
        ];

        for (input, expected) in test_cases {
            assert_eq!(format_zec_value(input as f64), expected);
        }
    }

    #[test]
    fn test_format_zec_value_rounding() {
        // Test trailing zero removal
        let value = 100_000_000; // 1.00000000 ZEC
        let result = format_zec_value(value as f64);
        assert_eq!(result, "1 ZEC");

        let value = 150_000_000; // 1.50000000 ZEC
        let result = format_zec_value(value as f64);
        assert_eq!(result, "1.5 ZEC");
    }

    #[test]
    fn test_format_zec_value_edge_cases() {
        // Test very large values
        let value = u64::MAX as f64;
        let result = format_zec_value(value);
        assert!(result.ends_with(" ZEC"));

        // Test max realistic ZEC value (21 million ZEC = 2.1e15 zatoshis)
        let max_zec = 2_100_000_000_000_000_u64;
        let result = format_zec_value(max_zec as f64);
        assert_eq!(result, "21000000 ZEC");
    }

    #[test]
    fn test_decode_memo_empty() {
        let memo = [0u8; 512];
        let result = decode_memo(memo);
        assert!(result.is_some());
        let decoded = result.unwrap();
        assert!(decoded.is_empty());
    }

    #[test]
    fn test_decode_memo_full_text() {
        let memo = [b'A'; 512];
        let result = decode_memo(memo);
        assert!(result.is_some());
        let decoded = result.unwrap();
        assert_eq!(decoded.len(), 512);
    }

    #[test]
    fn test_decode_memo_with_padding() {
        let mut memo = [0u8; 512];
        let text = b"Hello World";
        memo[..text.len()].copy_from_slice(text);
        let result = decode_memo(memo);
        assert!(result.is_some());
        assert_eq!(result.unwrap(), "Hello World");
    }

    #[test]
    fn test_decode_memo_utf8_with_multibyte() {
        let mut memo = [0u8; 512];
        let text = "测试中文".as_bytes();
        memo[..text.len()].copy_from_slice(text);
        let result = decode_memo(memo);
        assert!(result.is_some());
        assert_eq!(result.unwrap(), "测试中文");
    }

    #[test]
    fn test_decode_memo_boundary_cases() {
        // Test with first byte <= 0xF4 (UTF-8 decoding path) - simple ASCII
        let mut memo = [0u8; 512];
        memo[0] = b'A';
        memo[1] = b'B';
        memo[2] = b'C';
        let result = decode_memo(memo);
        assert!(result.is_some());
        assert_eq!(result.unwrap(), "ABC");

        // Test with 0xF5 (should use hex encoding)
        let mut memo = [0u8; 512];
        memo[0] = 0xF5;
        let result = decode_memo(memo);
        assert!(result.is_some());
        let hex_str = result.unwrap();
        assert!(hex_str.starts_with("f5"));
    }

    #[test]
    fn test_decode_memo_f6_marker_with_zeros() {
        // Test 0xF6 marker with all zeros after it (should return None)
        let mut memo = [0u8; 512];
        memo[0] = 0xF6;
        let result = decode_memo(memo);
        assert_eq!(result, None);
    }

    #[test]
    fn test_decode_memo_f6_marker_with_data() {
        // Test 0xF6 marker with non-zero data after it
        let mut memo = [0u8; 512];
        memo[0] = 0xF6;
        memo[1] = 0xFF;
        memo[2] = 0xAB;
        let result = decode_memo(memo);
        assert!(result.is_some());
        let hex_str = result.unwrap();
        assert!(hex_str.starts_with("f6ff"));
    }

    #[test]
    fn test_decode_memo_special_characters() {
        let mut memo = [0u8; 512];
        let text = b"Test!@#$%^&*()_+-=[]{}|;:',.<>?/`~";
        memo[..text.len()].copy_from_slice(text);
        let result = decode_memo(memo);
        assert!(result.is_some());
        assert_eq!(result.unwrap(), "Test!@#$%^&*()_+-=[]{}|;:',.<>?/`~");
    }

    #[test]
    fn test_format_zec_value_precision() {
        // Test that we maintain proper precision
        let value = 12345678; // 0.12345678 ZEC
        let result = format_zec_value(value as f64);
        assert_eq!(result, "0.12345678 ZEC");

        // Test single zatoshi
        let value = 1;
        let result = format_zec_value(value as f64);
        assert_eq!(result, "0.00000001 ZEC");
    }

    #[test]
    fn test_format_zec_value_no_fractional() {
        // Test whole number amounts
        let test_cases = vec![
            (100_000_000u64, "1 ZEC"),
            (200_000_000u64, "2 ZEC"),
            (1_000_000_000u64, "10 ZEC"),
            (10_000_000_000u64, "100 ZEC"),
        ];

        for (input, expected) in test_cases {
            assert_eq!(format_zec_value(input as f64), expected);
        }
    }

    #[test]
    fn test_decode_memo_newlines_and_tabs() {
        let mut memo = [0u8; 512];
        let text = b"Line1\nLine2\tTabbed";
        memo[..text.len()].copy_from_slice(text);
        let result = decode_memo(memo);
        assert!(result.is_some());
        assert_eq!(result.unwrap(), "Line1\nLine2\tTabbed");
    }

    #[test]
    fn test_decode_memo_all_printable_ascii() {
        let mut memo = [0u8; 512];
        // Test all printable ASCII characters (32-126)
        for i in 32..=126 {
            memo[i - 32] = i as u8;
        }
        let result = decode_memo(memo);
        assert!(result.is_some());
        let decoded = result.unwrap();
        // Should decode all printable ASCII
        assert!(decoded.len() <= 512);
    }

    #[test]
    fn test_format_zec_value_fractional_cents() {
        // Test values that should show as fractional cents
        let value = 1000; // 0.00001 ZEC
        let result = format_zec_value(value as f64);
        assert_eq!(result, "0.00001 ZEC");

        let value = 500; // 0.000005 ZEC
        let result = format_zec_value(value as f64);
        assert_eq!(result, "0.000005 ZEC");
    }

    #[test]
    fn test_decode_memo_mixed_content() {
        let mut memo = [0u8; 512];
        let text = b"Test123!@# ZEC Payment";
        memo[..text.len()].copy_from_slice(text);
        let result = decode_memo(memo);
        assert!(result.is_some());
        assert_eq!(result.unwrap(), "Test123!@# ZEC Payment");
    }

    #[test]
    fn test_format_zec_value_typical_transaction_amounts() {
        // Test common transaction amounts
        let test_cases = vec![
            (10_000_000u64, "0.1 ZEC"),     // 0.1 ZEC
            (50_000_000u64, "0.5 ZEC"),     // 0.5 ZEC
            (100_000_000u64, "1 ZEC"),      // 1 ZEC
            (250_000_000u64, "2.5 ZEC"),    // 2.5 ZEC
            (1_000_000_000u64, "10 ZEC"),   // 10 ZEC
            (10_000_000_000u64, "100 ZEC"), // 100 ZEC
        ];

        for (input, expected) in test_cases {
            assert_eq!(format_zec_value(input as f64), expected);
        }
    }
}
