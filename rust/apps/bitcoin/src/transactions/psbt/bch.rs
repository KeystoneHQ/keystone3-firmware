use crate::errors::{BitcoinError, Result};
use alloc::{string::ToString, vec::Vec};
use bitcoin::{
    bip32::{ChildNumber, Fingerprint},
    consensus::{deserialize_partial, encode::VarInt, serialize, Encodable},
    hashes::{sha256d, Hash},
    psbt::Psbt,
    secp256k1::{Message, Secp256k1},
    PublicKey, ScriptBuf, Transaction, TxOut,
};

const ALL_FORKID: u32 = 0x41;
struct SigningKey(bitcoin::secp256k1::SecretKey);
impl Drop for SigningKey {
    fn drop(&mut self) {
        self.0.non_secure_erase();
    }
}

pub(crate) fn validate_outputs(psbt: &Psbt) -> Result<()> {
    if psbt.unsigned_tx.output.is_empty() {
        return Err(BitcoinError::NoOutputs);
    }
    for output in &psbt.unsigned_tx.output {
        if !output.script_pubkey.is_p2pkh() && !output.script_pubkey.is_p2sh() {
            return Err(invalid(
                "unsupported BCH output: cannot display a recipient address",
            ));
        }
    }
    Ok(())
}

fn invalid(message: &str) -> BitcoinError {
    BitcoinError::InvalidPsbt(message.to_string())
}

fn rewrite_signatures(bytes: &mut [u8], from: u8, to: u8) -> Result<()> {
    if !bytes.starts_with(b"psbt\xff") {
        return Err(invalid("invalid PSBT magic"));
    }
    let mut pos = 5;
    let mut map = 0;
    let mut input_count = None;
    while pos < bytes.len() {
        let key = field(bytes, &mut pos)?;
        if key.is_empty() {
            map += 1;
            continue;
        }
        let value = field(bytes, &mut pos)?;
        if map == 0 && bytes[key.clone()] == [0] {
            let tx: Transaction = bitcoin::consensus::deserialize(&bytes[value.clone()])
                .map_err(|_| invalid("invalid unsigned transaction"))?;
            input_count = Some(tx.input.len());
        }
        if map > 0
            && map <= input_count.ok_or_else(|| invalid("missing unsigned transaction"))?
            && bytes[key.start] == 2
        {
            let last = value
                .end
                .checked_sub(1)
                .filter(|_| !value.is_empty())
                .ok_or_else(|| invalid("empty BCH signature"))?;
            if bytes[last] != from {
                return Err(invalid("BCH requires SIGHASH_ALL | FORKID signatures"));
            }
            bytes[last] = to;
        }
    }
    Ok(())
}

fn field(bytes: &[u8], pos: &mut usize) -> Result<core::ops::Range<usize>> {
    let (size, consumed) = deserialize_partial::<VarInt>(&bytes[*pos..])
        .map_err(|_| invalid("invalid PSBT field length"))?;
    *pos += consumed;
    let size = usize::try_from(size.0).map_err(|_| invalid("PSBT field too large"))?;
    let end = pos
        .checked_add(size)
        .filter(|end| *end <= bytes.len())
        .ok_or_else(|| invalid("truncated PSBT field"))?;
    let range = *pos..end;
    *pos = end;
    Ok(range)
}

pub(crate) fn deserialize(bytes: &[u8]) -> Result<Psbt> {
    let mut normalized = bytes.to_vec();
    rewrite_signatures(&mut normalized, 0x41, 0x01)?;
    let psbt = Psbt::deserialize(&normalized).map_err(|e| invalid(&e.to_string()))?;
    if psbt.inputs.is_empty() {
        return Err(BitcoinError::NoInputs);
    }
    validate_outputs(&psbt)?;
    for (index, input) in psbt.inputs.iter().enumerate() {
        let output = prevout(&psbt, index)?;
        for key in input.bip32_derivation.keys() {
            if ScriptBuf::new_p2pkh(&PublicKey::new(*key).pubkey_hash()) != output.script_pubkey {
                return Err(invalid(
                    "BCH input public key does not match previous output",
                ));
            }
        }
        if input.sighash_type.is_some_and(|t| t.to_u32() != ALL_FORKID) {
            return Err(invalid("BCH supports only SIGHASH_ALL | FORKID"));
        }
        for (_, path) in input.bip32_derivation.values() {
            let components = path.as_ref();
            if components.len() != 5
                || components[0] != (ChildNumber::Hardened { index: 44 })
                || components[1] != (ChildNumber::Hardened { index: 145 })
            {
                return Err(invalid("unsupported BCH derivation path"));
            }
        }
    }
    Ok(psbt)
}

fn prevout(psbt: &Psbt, index: usize) -> Result<&TxOut> {
    let input = &psbt.inputs[index];
    let outpoint = psbt.unsigned_tx.input[index].previous_output;
    let previous = input
        .non_witness_utxo
        .as_ref()
        .ok_or_else(|| invalid("BCH signing requires non_witness_utxo"))?;
    if previous.compute_txid() != outpoint.txid {
        return Err(invalid("BCH previous transaction ID mismatch"));
    }
    let output = previous
        .output
        .get(outpoint.vout as usize)
        .ok_or(BitcoinError::InvalidInput)?;
    if input
        .witness_utxo
        .as_ref()
        .is_some_and(|witness| witness != output)
    {
        return Err(invalid("BCH UTXO data mismatch"));
    }
    if !output.script_pubkey.is_p2pkh() {
        return Err(invalid("BCH supports only P2PKH inputs"));
    }
    Ok(output)
}

fn signature_hash(tx: &Transaction, index: usize, output: &TxOut) -> Result<sha256d::Hash> {
    let mut prevouts = Vec::new();
    let mut sequences = Vec::new();
    let mut outputs = Vec::new();
    for input in &tx.input {
        input.previous_output.consensus_encode(&mut prevouts)?;
        input.sequence.consensus_encode(&mut sequences)?;
    }
    for output in &tx.output {
        output.consensus_encode(&mut outputs)?;
    }
    let mut data = serialize(&tx.version);
    data.extend_from_slice(sha256d::Hash::hash(&prevouts).as_byte_array());
    data.extend_from_slice(sha256d::Hash::hash(&sequences).as_byte_array());
    tx.input[index]
        .previous_output
        .consensus_encode(&mut data)?;
    output.script_pubkey.consensus_encode(&mut data)?;
    output.value.consensus_encode(&mut data)?;
    tx.input[index].sequence.consensus_encode(&mut data)?;
    data.extend_from_slice(sha256d::Hash::hash(&outputs).as_byte_array());
    tx.lock_time.consensus_encode(&mut data)?;
    ALL_FORKID.consensus_encode(&mut data)?;
    Ok(sha256d::Hash::hash(&data))
}

pub(crate) fn sign(bytes: &[u8], seed: &[u8], fingerprint: Fingerprint) -> Result<Vec<u8>> {
    let mut psbt = deserialize(bytes)?;
    let secp = Secp256k1::new();
    let mut owned = false;
    for index in 0..psbt.inputs.len() {
        let input = &psbt.inputs[index];
        if input.final_script_sig.is_some()
            || input.final_script_witness.is_some()
            || input.redeem_script.is_some()
            || input.witness_script.is_some()
        {
            return Err(invalid("unsupported BCH input script fields"));
        }
        let output = prevout(&psbt, index)?;
        let script = output.script_pubkey.clone();
        let message =
            Message::from_digest(signature_hash(&psbt.unsigned_tx, index, output)?.to_byte_array());
        for (key, sig) in &input.partial_sigs {
            if ScriptBuf::new_p2pkh(&key.pubkey_hash()) != script {
                return Err(invalid("BCH signature public key mismatch"));
            }
            secp.verify_ecdsa(&message, &sig.signature, &key.inner)
                .map_err(|_| invalid("invalid existing BCH signature"))?;
        }
        let derivations = input.bip32_derivation.clone();
        for (key, (fp, path)) in derivations {
            if fp != fingerprint {
                continue;
            }
            let secret = SigningKey(
                keystore::algorithms::secp256k1::get_private_key_by_seed(seed, &path.to_string())
                    .map_err(|e| BitcoinError::GetKeyError(e.to_string()))?,
            );
            let public = PublicKey::new(secret.0.public_key(&secp));
            if public.inner != key || ScriptBuf::new_p2pkh(&public.pubkey_hash()) != script {
                return Err(invalid("BCH derived public key does not match input"));
            }
            let signature = secp.sign_ecdsa(&message, &secret.0);
            psbt.inputs[index]
                .partial_sigs
                .insert(public, bitcoin::ecdsa::Signature::sighash_all(signature));
            owned = true;
        }
        psbt.inputs[index].sighash_type =
            Some(bitcoin::psbt::PsbtSighashType::from_u32(ALL_FORKID));
    }
    if !owned {
        return Err(BitcoinError::NoMyInputs);
    }
    let mut result = psbt.serialize();
    rewrite_signatures(&mut result, 0x01, 0x41)?;
    Ok(result)
}

#[cfg(test)]
mod tests {
    use super::*;
    use core::str::FromStr;
    const BCH: &str = "70736274ff0100770100000001df92cf31d578229fa876e7bf2f5696798f4d18d28e0066010e67feb6f5fb00b90000000000ffffffff02005a6202000000001976a9144f04b6e897c1a3dfe2b74bfc40cbc4d9c64daf3088ac18839303000000001976a914cfdcdfa3e0483ddd3c7b6ab6ee13c5087c26d5f188ac0000000000010057010000000153ef901d37f8bfee6f9453ff7b8dc3e0672ee715498dae97db9dca73261f9a3300000000020101ffffffff0100e1f505000000001976a914086a977c7dad32a56996af2ce90a727238d4899888ac0000000001030441000000220602bbe7dbcdf8b2261530a867df7180b17a90b482f74f2736b8a30d3f756e42e2171873c5da0a2c000080910000800000008000000000000000000000220202f178b9410d11e47e10bef8a8e4bc53e48d9ae15864a024a54f87a802dad5f5141873c5da0a2c0000809100008000000080010000000000000000";
    fn seed() -> Vec<u8> {
        hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap()
    }
    fn fp() -> Fingerprint {
        Fingerprint::from_str("73c5da0a").unwrap()
    }
    fn fixture() -> Psbt {
        Psbt::deserialize(&hex::decode(BCH).unwrap()).unwrap()
    }
    fn sign_psbt(psbt: &Psbt) -> Result<Vec<u8>> {
        crate::sign_psbt_extend(psbt.serialize(), &seed(), fp(), Some(145))
    }

    #[test]
    #[allow(deprecated)]
    fn bch_matches_legacy_digest_signature_and_final_transaction() {
        use crate::transactions::legacy::{input::TxIn, sign_legacy_tx, TxData};
        for variant in 0..3 {
            let mut psbt = fixture();
            if variant > 0 {
                psbt.unsigned_tx.version = bitcoin::transaction::Version::TWO;
                psbt.unsigned_tx.lock_time = bitcoin::absolute::LockTime::from_consensus(800_000);
                psbt.unsigned_tx.input[0].sequence = bitcoin::Sequence(0xfffffffe);
                psbt.unsigned_tx.output[0].value = bitcoin::Amount::from_sat(12_345_678);
            }
            if variant == 2 {
                let mut input = psbt.inputs[0].clone();
                let previous = input.non_witness_utxo.as_mut().unwrap();
                previous.output[0].value = bitcoin::Amount::from_sat(50_000_000);
                let mut txin = psbt.unsigned_tx.input[0].clone();
                txin.previous_output.txid = previous.compute_txid();
                txin.sequence = bitcoin::Sequence(0xfffffffd);
                psbt.inputs.push(input);
                psbt.unsigned_tx.input.push(txin);
            }
            let mut legacy = TxData {
                inputs: psbt
                    .inputs
                    .iter()
                    .enumerate()
                    .map(|(index, input)| {
                        let (key, (_, path)) = input.bip32_derivation.first_key_value().unwrap();
                        let outpoint = psbt.unsigned_tx.input[index].previous_output;
                        TxIn {
                            previous_output: outpoint.txid.to_string(),
                            vout: outpoint.vout,
                            value: prevout(&psbt, index).unwrap().value.to_sat(),
                            pubkey: key.to_string(),
                            hd_path: path.to_string(),
                        }
                    })
                    .collect(),
                outputs: vec![],
                script_type: "P2PKH".to_string(),
                network: "BCH".to_string(),
                extended_pubkey: alloc::string::String::new(),
                xfp: fp().to_string(),
                transaction: psbt.unsigned_tx.clone(),
            };
            for index in 0..psbt.inputs.len() {
                let old_hash = match legacy.signature_hash(index).unwrap() {
                    either::Right(hash) => hash.to_byte_array(),
                    _ => panic!("BCH must use ForkID"),
                };
                assert_eq!(
                    old_hash,
                    signature_hash(&psbt.unsigned_tx, index, prevout(&psbt, index).unwrap())
                        .unwrap()
                        .to_byte_array()
                );
            }
            let signed = deserialize(&sign_psbt(&psbt).unwrap()).unwrap();
            let mut finalized = signed.unsigned_tx.clone();
            for (index, input) in signed.inputs.iter().enumerate() {
                let (key, signature) = input.partial_sigs.first_key_value().unwrap();
                let mut bytes = signature.signature.serialize_der().to_vec();
                bytes.push(0x41);
                finalized.input[index].script_sig = bitcoin::script::Builder::new()
                    .push_slice(bitcoin::script::PushBytesBuf::try_from(bytes).unwrap())
                    .push_key(key)
                    .into_script();
            }
            let legacy_bytes = sign_legacy_tx(&mut legacy, &seed()).unwrap();
            assert_eq!(serialize(&finalized), legacy_bytes, "variant {variant}");
            assert_eq!(finalized.compute_txid(), legacy.transaction.compute_txid());
        }
    }

    #[test]
    fn bch_psbt_does_not_hide_mismatched_change_and_rejects_no_outputs() {
        let mut psbt = fixture();
        let path = bitcoin::bip32::DerivationPath::from_str("m/44'/145'/0'").unwrap();
        let xpub = keystore::algorithms::secp256k1::get_extended_public_key_by_seed(
            &seed(),
            &path.to_string(),
        )
        .unwrap();
        let context = || {
            crate::parsed_tx::ParseContext::new(
                fp(),
                alloc::collections::BTreeMap::from([(path.clone(), xpub)]),
                None,
                None,
            )
        };
        crate::check_psbt(psbt.serialize(), context()).unwrap();
        psbt.unsigned_tx.output[1].script_pubkey = psbt.unsigned_tx.output[0].script_pubkey.clone();
        let display = crate::parse_psbt(psbt.serialize(), context()).unwrap();
        assert!(!display.detail.to[1].is_mine);
        assert_eq!(display.overview.total_output_sat, "99999000 sats");
        psbt.outputs[1]
            .bip32_derivation
            .values_mut()
            .next()
            .unwrap()
            .1 = bitcoin::bip32::DerivationPath::from_str("m/44'/145'/0'/1/1").unwrap();
        let display = crate::parse_psbt(psbt.serialize(), context()).unwrap();
        assert!(!display.detail.to[1].is_mine);
        assert_eq!(display.overview.total_output_sat, "99999000 sats");
        psbt.unsigned_tx.output.clear();
        psbt.outputs.clear();
        assert!(matches!(sign_psbt(&psbt), Err(BitcoinError::NoOutputs)));
    }

    #[test]
    fn bch_forkid_signature_and_wire_roundtrip() {
        let psbt = fixture();
        let expected =
            hex::decode("f292551b43d5fcc05ac26707d02303ff280513a489daf31f476b7ee1c326134a")
                .unwrap();
        assert_eq!(
            signature_hash(&psbt.unsigned_tx, 0, prevout(&psbt, 0).unwrap())
                .unwrap()
                .as_byte_array(),
            expected.as_slice()
        );
        let signed = sign_psbt(&psbt).unwrap();
        let parsed = deserialize(&signed).unwrap();
        assert_eq!(parsed.inputs[0].sighash_type.unwrap().to_u32(), 0x41);
        assert_eq!(parsed.inputs[0].partial_sigs.len(), 1);
        let (key, signature) = parsed.inputs[0].partial_sigs.first_key_value().unwrap();
        Secp256k1::verification_only()
            .verify_ecdsa(
                &Message::from_digest_slice(&expected).unwrap(),
                &signature.signature,
                &key.inner,
            )
            .unwrap();
        let mut wire = signature.signature.serialize_der().to_vec();
        wire.push(0x41);
        assert!(signed.windows(wire.len()).any(|value| value == wire));
        assert_eq!(parsed.unsigned_tx, psbt.unsigned_tx);
        assert!(
            crate::parse_psbt_hex_sign_status(&signed)
                .unwrap()
                .is_completed
        );
        let account_path = bitcoin::bip32::DerivationPath::from_str("m/44'/145'/0'").unwrap();
        let xpub = keystore::algorithms::secp256k1::get_extended_public_key_by_seed(
            &seed(),
            &account_path.to_string(),
        )
        .unwrap();
        let context = crate::parsed_tx::ParseContext::new(
            fp(),
            alloc::collections::BTreeMap::from([(account_path, xpub)]),
            None,
            None,
        );
        crate::check_psbt(
            psbt.serialize(),
            crate::parsed_tx::ParseContext::new(
                fp(),
                context.extended_public_keys.clone(),
                None,
                None,
            ),
        )
        .unwrap();
        let display = crate::parse_psbt(signed.clone(), context).unwrap();
        assert_eq!(
            display.overview.from,
            vec!["qqyx49mu0kkn9ftfj6hje6g2wfer34yfnq5tahq3q6".to_string()]
        );
        assert!(!display.overview.need_sign);
        let request =
            ur_registry::crypto_psbt_extend::CryptoPSBTExtend::new(signed.clone(), Some(145))
                .unwrap();
        let cbor: Vec<u8> = request.try_into().unwrap();
        let decoded = ur_registry::crypto_psbt_extend::CryptoPSBTExtend::try_from(cbor).unwrap();
        assert_eq!(decoded.get_coin_id(), Some(145));
        assert_eq!(decoded.get_psbt(), signed);
        assert_eq!(
            crate::sign_psbt_extend(signed.clone(), &seed(), fp(), Some(145)).unwrap(),
            signed
        );
        assert!(crate::sign_psbt(signed, &seed(), fp()).is_err());
    }

    #[test]
    fn bch_multiple_inputs_preserve_external_signatures() {
        let mut p = fixture();
        let second_txin = p.unsigned_tx.input[0].clone();
        let second_input = p.inputs[0].clone();
        p.unsigned_tx.input.push(second_txin);
        p.inputs.push(second_input);
        let previous = p.inputs[1].non_witness_utxo.as_mut().unwrap();
        previous.lock_time = bitcoin::absolute::LockTime::from_consensus(1);
        p.unsigned_tx.input[1].previous_output.txid = previous.compute_txid();
        let signed = sign_psbt(&p).unwrap();
        let mut parsed = deserialize(&signed).unwrap();
        for index in 0..2 {
            let (key, signature) = parsed.inputs[index].partial_sigs.first_key_value().unwrap();
            let msg = Message::from_digest(
                signature_hash(&parsed.unsigned_tx, index, prevout(&parsed, index).unwrap())
                    .unwrap()
                    .to_byte_array(),
            );
            Secp256k1::verification_only()
                .verify_ecdsa(&msg, &signature.signature, &key.inner)
                .unwrap();
        }
        parsed.inputs[1]
            .bip32_derivation
            .values_mut()
            .next()
            .unwrap()
            .0 = Fingerprint::default();
        let mut external = parsed.serialize();
        rewrite_signatures(&mut external, 1, 0x41).unwrap();
        let resigned = crate::sign_psbt_extend(external, &seed(), fp(), Some(145)).unwrap();
        assert_eq!(
            deserialize(&resigned).unwrap().inputs[1].partial_sigs,
            parsed.inputs[1].partial_sigs
        );
    }

    #[test]
    fn bch_defaults_to_all_forkid_but_rejects_other_sighashes() {
        let mut psbt = fixture();
        psbt.inputs[0].sighash_type = None;
        let result = deserialize(&sign_psbt(&psbt).unwrap()).unwrap();
        assert_eq!(result.inputs[0].sighash_type.unwrap().to_u32(), 0x41);
        for value in [0, 1, 2, 3, 0x42, 0x43, 0xc1] {
            psbt.inputs[0].sighash_type = Some(bitcoin::psbt::PsbtSighashType::from_u32(value));
            assert!(sign_psbt(&psbt).is_err(), "{value}");
        }
    }

    #[test]
    fn bch_rejects_wrong_utxo_key_path_and_script() {
        let original = fixture();
        let mut p = original.clone();
        p.inputs[0].non_witness_utxo.as_mut().unwrap().output[0].value =
            bitcoin::Amount::from_sat(1);
        assert!(sign_psbt(&p).is_err());
        let mut p = original.clone();
        p.inputs[0].witness_utxo = Some(TxOut {
            value: bitcoin::Amount::from_sat(1),
            ..prevout(&original, 0).unwrap().clone()
        });
        assert!(sign_psbt(&p).is_err());
        p.inputs[0].non_witness_utxo = None;
        assert!(sign_psbt(&p).is_err());
        let mut p = original.clone();
        p.inputs[0].bip32_derivation.values_mut().next().unwrap().1 =
            bitcoin::bip32::DerivationPath::from_str("m/44'/145'/0'/0/1").unwrap();
        assert!(sign_psbt(&p).is_err());
        p.inputs[0].bip32_derivation.values_mut().next().unwrap().1 =
            bitcoin::bip32::DerivationPath::from_str("m/44'/5'/0'/0/0").unwrap();
        assert!(sign_psbt(&p).is_err());
        assert!(crate::sign_psbt_extend(
            original.serialize(),
            &seed(),
            Fingerprint::default(),
            Some(145)
        )
        .is_err());
        let mut p = original.clone();
        let prev = p.inputs[0].non_witness_utxo.as_mut().unwrap();
        prev.output[0].script_pubkey = ScriptBuf::new();
        p.unsigned_tx.input[0].previous_output.txid = prev.compute_txid();
        assert!(sign_psbt(&p).is_err());
    }

    #[test]
    fn bch_preserves_unknown_fields_and_rejects_truncated_psbt() {
        let mut p = fixture();
        let key = bitcoin::psbt::raw::Key {
            type_value: 0xee,
            key: vec![1, 2],
        };
        p.inputs[0].unknown.insert(key.clone(), vec![2, 0x41, 0x01]);
        p.outputs[0].unknown.insert(key.clone(), vec![2, 0x41]);
        let signed = sign_psbt(&p).unwrap();
        let parsed = deserialize(&signed).unwrap();
        assert_eq!(parsed.inputs[0].unknown, p.inputs[0].unknown);
        assert_eq!(parsed.outputs[0].unknown, p.outputs[0].unknown);
        for end in 0..signed.len() {
            assert!(deserialize(&signed[..end]).is_err(), "{end}");
        }
        let mut changed = parsed;
        changed.unsigned_tx.output[0].value = bitcoin::Amount::from_sat(1);
        let mut changed_bytes = changed.serialize();
        rewrite_signatures(&mut changed_bytes, 1, 0x41).unwrap();
        assert!(crate::sign_psbt_extend(changed_bytes, &seed(), fp(), Some(145)).is_err());
    }

    #[test]
    fn other_psbt_extend_chains_keep_standard_signatures() {
        for (coin, data) in [
            (2, "70736274ff01007101000000011bfed2d08a2b45c790930953230bdc406eca13e579b5049eb751086b3ed369420000000000ffffffff02005a62020000000016001477f2208e272bef80e98ce956e05d5ed8f596f5a3f05f930300000000160014267f2c10ac39e483c00c982efdd0112bcc3e62830000000000010054010000000130297791cbb43ddbe1b17406f92f79b653a585f061213749fb069644b7f7ffbb00000000020101ffffffff0100e1f5050000000016001496cd3178f3de0f307bcee1aeaa4d5987862dd30a0000000001011f00e1f5050000000016001496cd3178f3de0f307bcee1aeaa4d5987862dd30a01030401000000220602e49c9b9b5d0f127235dc26a0c252814c52fb333d651a946773f59d72c2da99041873c5da0a540000800200008000000080000000000000000000002202029857513f0fe1dc125f219ffef22098e14c653c4bcb0a2aaeff47b3a252569f1a1873c5da0a540000800200008000000080010000000000000000"),
            (3, "70736274ff0100770100000001faf994b04953ae7fbd735800e48605de15ded8126b9308556ad98962327ab3430000000000ffffffff02005a6202000000001976a9143bf96544e911ca728f5fabf5a4f3b65deab684c488acc0448403000000001976a91419115174dcc75cce4802aaec82e63b491274c2e288ac000000000001005701000000016f9512863e3f25696cc723f570d3c4077785f1944a895895a70750689453e48b00000000020101ffffffff0100e1f505000000001976a9144a483568665dcdfa68dd58a1f62893448a64333988ac0000000001030401000000220602cc6b0dc33aabcf3a23643e5e2919a80c50fb3dd2129ce409bbc5f0d4643d05e01873c5da0a2c0000800300008000000080000000000000000000002202020745c065a8b7cb03a843a57339cf9164d675a2e5010c6fbd61c09239bbb5f4df1873c5da0a2c0000800300008000000080010000000000000000"),
            (5, "70736274ff010077010000000102754c72a9069560e14bf2faa47998f4442149e798cff5f8c953e7d92ab219d20000000000ffffffff02005a6202000000001976a9140a38ec3f3cc4c169c295ddb037c31c63bf2fa39188acf05f9303000000001976a914265a7d4ef63d643548a599c8fa53a0d9ef3fdca488ac0000000000010057010000000105d94a72185cf340264af0461c176c985bf0d55fef0026c94447f723ec03f90600000000020101ffffffff0100e1f505000000001976a9148a4f58c222cd5544c527bc66925652baa70b5e8088ac00000000010304010000002206026fa9a6f213b6ba86447965f6b4821264aaadd7521f049f00db9c43a770ea74051873c5da0a2c000080050000800000008000000000000000000000220202bd7b0ae9a8a0e67f82891d92545433e1b37074ab0bf5867b977f8c70b00f28671873c5da0a2c0000800500008000000080010000000000000000"),
        ] {
            let bytes = hex::decode(data).unwrap();
            let result = crate::sign_psbt_extend(bytes.clone(), &seed(), fp(), Some(coin)).unwrap();
            assert_eq!(result, crate::sign_psbt(bytes, &seed(), fp()).unwrap());
            let parsed = Psbt::deserialize(&result).unwrap();
            let (key, signature) = parsed.inputs[0].partial_sigs.first_key_value().unwrap();
            let mut cache = bitcoin::sighash::SighashCache::new(&parsed.unsigned_tx);
            let (message, _) = parsed.sighash_ecdsa(0, &mut cache).unwrap();
            Secp256k1::verification_only().verify_ecdsa(&message, &signature.signature, &key.inner).unwrap();
            assert_eq!(signature.to_vec().last(), Some(&1));
            assert!(crate::sign_psbt_extend(hex::decode(data).unwrap(), &seed(), fp(), Some(145)).is_err());
        }
    }
}
