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
    ripemd::{self, Digest},
    sha2::Sha256,
    zcash_address::{
        unified::{self, Encoding, Receiver},
        ToAddress, ZcashAddress,
    },
    zcash_keys::{keys::UnifiedFullViewingKey},
    zcash_protocol::consensus::NetworkType,
};

use crate::errors::ZcashError;

use super::structs::{ParsedFrom, ParsedOrchard, ParsedPczt, ParsedTo, ParsedTransparent};

const ZEC_DIVIDER: u32 = 1_000_000_00;

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
        &pczt.orchard,
    )?;

    let parsed_transparent = parse_transparent(seed_fingerprint, &pczt.transparent)?;

    Ok(ParsedPczt::new(parsed_transparent, parsed_orchard))
}

fn parse_transparent(
    seed_fingerprint: &[u8; 32],
    transparent: &pczt::transparent::Bundle,
) -> Result<Option<ParsedTransparent>, ZcashError> {
    let mut parsed_transparent = ParsedTransparent::new(vec![], vec![]);
    transparent.inputs.iter().try_for_each(|input| {
        let parsed_from = parse_transparent_input(seed_fingerprint, &input)?;
        parsed_transparent.add_from(parsed_from);
        Ok(())
    })?;
    transparent.outputs.iter().try_for_each(|output| {
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
    let ta = if let Some(redeem_script) = input.redeem_script.as_ref() {
        let script_hash = *ripemd::Ripemd160::digest(Sha256::digest(redeem_script)).as_ref();
        ZcashAddress::from_transparent_p2sh(NetworkType::Main, script_hash).encode()
    } else {
        let pubkey_hash =
            *ripemd::Ripemd160::digest(Sha256::digest(input.script_pubkey.clone())).as_ref();
        ZcashAddress::from_transparent_p2pkh(NetworkType::Main, pubkey_hash).encode()
    };

    let zec_value = format!("{:.8}", input.value as f64 / ZEC_DIVIDER as f64);

    let is_mine = match input.bip32_derivation.get(&input.script_pubkey) {
        //pubkey validation is checked on transaction checking part
        Some(bip32_derivation) => seed_fingerprint == &bip32_derivation.seed_fingerprint,
        None => false,
    };

    Ok(ParsedFrom::new(ta, zec_value, is_mine))
}

fn parse_transparent_output(
    seed_fingerprint: &[u8; 32],
    output: &pczt::transparent::Output,
) -> Result<ParsedTo, ZcashError> {
    let ta = if let Some(redeem_script) = output.redeem_script.as_ref() {
        let script_hash = *ripemd::Ripemd160::digest(Sha256::digest(redeem_script)).as_ref();
        ZcashAddress::from_transparent_p2sh(NetworkType::Main, script_hash).encode()
    } else {
        let pubkey_hash =
            *ripemd::Ripemd160::digest(Sha256::digest(output.script_pubkey.clone())).as_ref();
        ZcashAddress::from_transparent_p2pkh(NetworkType::Main, pubkey_hash).encode()
    };
    let zec_value = format!("{:.8}", output.value as f64 / ZEC_DIVIDER as f64);
    let is_mine = match output.bip32_derivation.get(&output.script_pubkey) {
        Some(bip32_derivation) => seed_fingerprint == &bip32_derivation.seed_fingerprint,
        None => false,
    };
    Ok(ParsedTo::new(ta, zec_value, is_mine, None))
}

fn parse_orchard(
    seed_fingerprint: &[u8; 32],
    fvk: &FullViewingKey,
    orchard: &pczt::orchard::Bundle,
) -> Result<Option<ParsedOrchard>, ZcashError> {
    let mut parsed_orchard = ParsedOrchard::new(vec![], vec![]);
    orchard.actions.iter().try_for_each(|action| {
        let spend = action.spend.clone();

        let parsed_from = parse_orchard_spend(seed_fingerprint, &spend)?;
        let parsed_to = parse_orchard_output(fvk, &action)?;

        parsed_orchard.add_from(parsed_from);
        parsed_orchard.add_to(parsed_to);

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
    let recipient = spend.recipient.clone().ok_or(ZcashError::InvalidPczt(
        "recipient is not present".to_string(),
    ))?;
    let value = spend
        .value
        .clone()
        .ok_or(ZcashError::InvalidPczt("value is not present".to_string()))?;
    let zec_value: String = format!("{:.8}", value as f64 / ZEC_DIVIDER as f64);
    let zip32_derivation = spend
        .zip32_derivation
        .clone()
        .ok_or(ZcashError::InvalidPczt(
            "zip32 derivation is not present".to_string(),
        ))?;

    let ua = unified::Address(vec![Receiver::Orchard(recipient)]).encode(&NetworkType::Main);

    let fingerprint = zip32_derivation.seed_fingerprint;
    Ok(ParsedFrom::new(
        ua,
        zec_value,
        seed_fingerprint == &fingerprint,
    ))
}

fn parse_orchard_output(
    fvk: &FullViewingKey,
    action: &pczt::orchard::Action,
) -> Result<ParsedTo, ZcashError> {
    let output = action.output.clone();
    let epk = output.ephemeral_key.clone();
    let cmx = output.cmx.clone();
    let nf_old = action.spend.nullifier.clone();
    let rho = Rho::from_nf_old(Nullifier::from_bytes(&nf_old).into_option().ok_or(
        ZcashError::InvalidPczt("nullifier is not valid".to_string()),
    )?);
    let cv = action.cv.clone();
    let enc_ciphertext = output.enc_ciphertext.clone().try_into().unwrap();
    let out_ciphertext = output.out_ciphertext.clone().try_into().unwrap();
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
        let zec_value: String = format!("{:.8}", note.value().inner() as f64 / ZEC_DIVIDER as f64);
        let ua = unified::Address(vec![Receiver::Orchard(address.to_raw_address_bytes())])
            .encode(&NetworkType::Main);
        let memo_str = String::from_utf8(memo.to_vec())
            .map_err(|_| ZcashError::InvalidPczt("Invalid UTF-8 sequence in memo".to_string()))?;
        Ok(ParsedTo::new(ua, zec_value, false, Some(memo_str)))
    } else if let Some((note, address, memo)) = decode_output_enc_ciphertext(
        &internal_ovk,
        &rho,
        &epk,
        &cmx,
        &cv,
        &enc_ciphertext,
        &out_ciphertext,
    )? {
        let zec_value: String = format!("{:.8}", note.value().inner() as f64 / ZEC_DIVIDER as f64);
        let ua = unified::Address(vec![Receiver::Orchard(address.to_raw_address_bytes())])
            .encode(&NetworkType::Main);
        let memo_str = String::from_utf8(memo.to_vec())
            .map_err(|_| ZcashError::InvalidPczt("Invalid UTF-8 sequence in memo".to_string()))?;
        Ok(ParsedTo::new(ua, zec_value, true, Some(memo_str)))
    } else {
        Ok(ParsedTo::new(
            "Unknown Address".to_string(),
            "Unknown Value".to_string(),
            false,
            None,
        ))
    }
}
