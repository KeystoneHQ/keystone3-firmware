use super::{extend, wrapped_psbt::WrappedPsbt};
use crate::{
    errors::{BitcoinError, Result},
    network::Network,
    parsed_tx::{ParseContext, ParsedTx},
    transactions::tx_checker::TxChecker,
    PsbtSignStatus,
};
use alloc::{collections::BTreeMap, string::ToString, vec::Vec};
use bitcoin::{
    bip32::Fingerprint,
    consensus::{deserialize, deserialize_partial, encode::VarInt, serialize},
    hashes::{sha256d, Hash},
    psbt::Psbt,
    PublicKey, ScriptBuf, Transaction, TxOut, Txid,
};
use either::Left;

type Map = BTreeMap<Vec<u8>, Vec<u8>>;
fn invalid(message: &str) -> BitcoinError {
    BitcoinError::InvalidPsbt(message.to_string())
}

fn field<'a>(bytes: &mut &'a [u8]) -> Result<&'a [u8]> {
    let (length, used): (VarInt, usize) =
        deserialize_partial(bytes).map_err(|_| invalid("invalid DASH PSBT length"))?;
    *bytes = &bytes[used..];
    let length = usize::try_from(length.0).map_err(|_| invalid("DASH PSBT field too large"))?;
    if length > bytes.len() {
        return Err(invalid("truncated DASH PSBT field"));
    }
    let (value, rest) = bytes.split_at(length);
    *bytes = rest;
    Ok(value)
}
fn read_map(bytes: &mut &[u8]) -> Result<Map> {
    let mut map = Map::new();
    loop {
        let key = field(bytes)?;
        if key.is_empty() {
            return Ok(map);
        }
        let value = field(bytes)?;
        if map.insert(key.to_vec(), value.to_vec()).is_some() {
            return Err(invalid("duplicate DASH PSBT key"));
        }
    }
}
fn encode_maps(maps: &[Map]) -> Vec<u8> {
    let mut bytes = b"psbt\xff".to_vec();
    for map in maps {
        for (key, value) in map {
            bytes.extend(serialize(&VarInt(key.len() as u64)));
            bytes.extend_from_slice(key);
            bytes.extend(serialize(&VarInt(value.len() as u64)));
            bytes.extend_from_slice(value);
        }
        bytes.push(0);
    }
    bytes
}

fn previous_transaction(raw: &[u8]) -> Result<(Transaction, Txid)> {
    fn take<T: bitcoin::consensus::Decodable>(bytes: &mut &[u8]) -> Result<T> {
        let (value, used) =
            deserialize_partial(bytes).map_err(|_| invalid("invalid DASH previous transaction"))?;
        *bytes = &bytes[used..];
        Ok(value)
    }
    let mut suffix = raw;
    let version: i32 = take(&mut suffix)?;
    let inputs: Vec<bitcoin::TxIn> = take(&mut suffix)?;
    let outputs: Vec<TxOut> = take(&mut suffix)?;
    let lock_time: u32 = take(&mut suffix)?;
    let tx = Transaction {
        version: bitcoin::transaction::Version(version),
        lock_time: bitcoin::absolute::LockTime::from_consensus(lock_time),
        input: inputs,
        output: outputs,
    };
    let version = version as u32;
    if (version & 0xffff) >= 3 && version >> 16 != 0 {
        let payload = field(&mut suffix)?;
        if payload.len() > 10_000 {
            return Err(invalid("DASH extra payload too large"));
        }
    } else if version >> 16 != 0 {
        return Err(invalid("invalid DASH special transaction version"));
    }
    if !suffix.is_empty() {
        return Err(invalid("unexpected trailing DASH transaction data"));
    }
    Ok((tx, Txid::from_raw_hash(sha256d::Hash::hash(raw))))
}

struct DashPsbt {
    maps: Vec<Map>,
    wrapper: WrappedPsbt,
}
impl DashPsbt {
    fn decode(bytes: &[u8]) -> Result<Self> {
        let mut rest = bytes
            .strip_prefix(b"psbt\xff")
            .ok_or_else(|| invalid("invalid DASH PSBT magic"))?;
        let global = read_map(&mut rest)?;
        let raw_tx = global
            .get(&vec![0])
            .ok_or_else(|| invalid("missing DASH unsigned transaction"))?;
        let tx: Transaction =
            deserialize(raw_tx).map_err(|_| invalid("invalid DASH unsigned transaction"))?;
        if (tx.version.0 as u32) >> 16 != 0 {
            return Err(invalid("signing DASH special transactions is unsupported"));
        }
        if tx.input.is_empty() {
            return Err(BitcoinError::NoInputs);
        }
        if tx.output.is_empty() {
            return Err(BitcoinError::NoOutputs);
        }
        let mut maps = vec![global];
        for _ in 0..tx.input.len() + tx.output.len() {
            maps.push(read_map(&mut rest)?);
        }
        if !rest.is_empty() {
            return Err(invalid("trailing DASH PSBT data"));
        }
        let mut view_maps = maps.clone();
        for (index, input) in tx.input.iter().enumerate() {
            let map = &mut view_maps[index + 1];
            let raw = map
                .remove(&vec![0])
                .ok_or_else(|| invalid("DASH signing requires non_witness_utxo"))?;
            let (previous, txid) = previous_transaction(&raw)?;
            if txid != input.previous_output.txid {
                return Err(invalid("DASH previous transaction ID mismatch"));
            }
            let prevout = previous
                .output
                .get(input.previous_output.vout as usize)
                .ok_or_else(|| invalid("DASH previous output index out of bounds"))?;
            if !prevout.script_pubkey.is_p2pkh() {
                return Err(invalid("DASH supports only P2PKH inputs"));
            }
            if let Some(raw_witness) = map.get(&vec![1]) {
                let witness: TxOut =
                    deserialize(raw_witness).map_err(|_| invalid("invalid DASH witness UTXO"))?;
                if witness != *prevout {
                    return Err(invalid("DASH witness and non-witness UTXO mismatch"));
                }
            }
            map.insert(vec![1], serialize(prevout));
        }
        let psbt =
            Psbt::deserialize(&encode_maps(&view_maps)).map_err(|e| invalid(&e.to_string()))?;
        extend::validate(&psbt, Some(5))?;
        for input in &psbt.inputs {
            if input.redeem_script.is_some()
                || input.witness_script.is_some()
                || input.final_script_sig.is_some()
                || input.final_script_witness.is_some()
            {
                return Err(invalid("unsupported DASH input script fields"));
            }
            input
                .ecdsa_hash_ty()
                .map_err(|_| invalid("unsupported DASH sighash type"))?;
            let script = &input
                .witness_utxo
                .as_ref()
                .ok_or_else(|| invalid("missing verified DASH output"))?
                .script_pubkey;
            for key in input.bip32_derivation.keys() {
                if ScriptBuf::new_p2pkh(&PublicKey::new(*key).pubkey_hash()) != *script {
                    return Err(invalid("DASH public key does not match previous output"));
                }
            }
        }
        Ok(Self {
            maps,
            wrapper: WrappedPsbt { psbt },
        })
    }
}

pub(crate) fn parse(bytes: &[u8], context: &ParseContext) -> Result<ParsedTx> {
    let decoded = DashPsbt::decode(bytes)?;
    let mut parsed = decoded
        .wrapper
        .parse_in_network(Some(context), Network::Dash, false)?;
    parsed.overview.has_witness_only_inputs = false;
    Ok(parsed)
}
pub(crate) fn check(bytes: &[u8], context: &ParseContext) -> Result<()> {
    DashPsbt::decode(bytes)?.wrapper.check(Left(context))
}
pub(crate) fn sign_status(bytes: &[u8]) -> Result<PsbtSignStatus> {
    let decoded = DashPsbt::decode(bytes)?;
    if decoded
        .wrapper
        .psbt
        .inputs
        .iter()
        .all(|input| input.bip32_derivation.is_empty())
        && decoded.wrapper.psbt.xpub.is_empty()
    {
        return Err(invalid("missing DASH key origins"));
    }
    Ok(PsbtSignStatus {
        sign_status: decoded.wrapper.get_overall_sign_status(),
        is_completed: decoded.wrapper.is_sign_completed(),
    })
}
pub(crate) fn sign(bytes: &[u8], seed: &[u8], fingerprint: Fingerprint) -> Result<Vec<u8>> {
    let mut decoded = DashPsbt::decode(bytes)?;
    let mut owned = false;
    for input in &decoded.wrapper.psbt.inputs {
        for (key, (fp, path)) in &input.bip32_derivation {
            if *fp != fingerprint {
                continue;
            }
            let public =
                keystore::algorithms::secp256k1::get_public_key_by_seed(seed, &path.to_string())
                    .map_err(|e| BitcoinError::GetKeyError(e.to_string()))?;
            if public != *key {
                return Err(invalid("DASH derived public key mismatch"));
            }
            owned = true;
        }
    }
    if !owned {
        return Err(BitcoinError::NoMyInputs);
    }
    let signed = decoded.wrapper.sign(seed, fingerprint)?;
    for (index, input) in signed.inputs.iter().enumerate() {
        for (key, signature) in &input.partial_sigs {
            let mut raw_key = vec![2];
            raw_key.extend(key.to_bytes());
            decoded.maps[index + 1].insert(raw_key, signature.to_vec());
        }
    }
    Ok(encode_maps(&decoded.maps))
}

#[cfg(test)]
mod tests {
    use super::*;
    use bitcoin::{
        bip32::DerivationPath,
        secp256k1::{Message, Secp256k1},
    };
    use core::str::FromStr;
    fn seed() -> Vec<u8> {
        hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap()
    }
    fn fp() -> Fingerprint {
        Fingerprint::from_str("73c5da0a").unwrap()
    }
    fn context() -> ParseContext {
        let path = DerivationPath::from_str("m/44'/5'/0'").unwrap();
        let xpub = keystore::algorithms::secp256k1::get_extended_public_key_by_seed(
            &seed(),
            &path.to_string(),
        )
        .unwrap();
        ParseContext::new(fp(), BTreeMap::from([(path, xpub)]), None, None)
    }
    fn fixture() -> Psbt {
        Psbt::deserialize(&hex::decode("70736274ff010077010000000102754c72a9069560e14bf2faa47998f4442149e798cff5f8c953e7d92ab219d20000000000ffffffff02005a6202000000001976a9140a38ec3f3cc4c169c295ddb037c31c63bf2fa39188acf05f9303000000001976a914265a7d4ef63d643548a599c8fa53a0d9ef3fdca488ac0000000000010057010000000105d94a72185cf340264af0461c176c985bf0d55fef0026c94447f723ec03f90600000000020101ffffffff0100e1f505000000001976a9148a4f58c222cd5544c527bc66925652baa70b5e8088ac00000000010304010000002206026fa9a6f213b6ba86447965f6b4821264aaadd7521f049f00db9c43a770ea74051873c5da0a2c000080050000800000008000000000000000000000220202bd7b0ae9a8a0e67f82891d92545433e1b37074ab0bf5867b977f8c70b00f28671873c5da0a2c0000800500008000000080010000000000000000").unwrap()).unwrap()
    }
    fn maps(bytes: &[u8]) -> Vec<Map> {
        let mut rest = &bytes[5..];
        let mut maps = Vec::new();
        while !rest.is_empty() {
            maps.push(read_map(&mut rest).unwrap());
        }
        maps
    }
    fn cbtx() -> Vec<u8> {
        hex::decode("03000500010000000000000000000000000000000000000000000000000000000000000000ffffffff2703da5c1e04d04580650800007588782d01000fe4b883e5bda9e7a59ee4bb99e9b1bc04f09f909f88f581b80253640f03000000001976a9147c086eada12bdb10a265c16c08a7ae87366bd48188acf82c2e09000000001976a91453dc8bffae84a3851ab9fe296417da85d5e7185888aca0f04c20af0300da5c1e0054e04067fa61cef61f1e91b0d52c21b28cdc69cc9d0383f0bb88ba0129b973e2f67e47d5580e70c7e77af8b1a6670669c7424b896ab3d44d349a4988e9a8328d00b22fef542f5c630e4f2a67ad56d30cf2a3dac55848abcd2bac30bf780318792bcdf32f7729443846c9fdc57050a131cd12c0e54a16f6265e900f459e3b9dd43421a0bc3f61efd49ce752e440e5131a8d2758a280ad883b8eb902c49ee5b878b30000000000000000").unwrap()
    }
    fn special() -> Vec<Map> {
        let mut p = fixture();
        let mut previous = p.inputs[0].non_witness_utxo.clone().unwrap();
        previous.version = bitcoin::transaction::Version(0x00050003);
        previous.input[0].previous_output = bitcoin::OutPoint::null();
        let mut raw = serialize(&previous);
        let mut payload = vec![2, 0];
        payload.extend_from_slice(&1989850u32.to_le_bytes());
        payload.extend_from_slice(&[0x42; 64]);
        raw.extend(serialize(&VarInt(payload.len() as u64)));
        raw.extend(payload);
        p.unsigned_tx.input[0].previous_output.txid =
            Txid::from_raw_hash(sha256d::Hash::hash(&raw));
        let mut result = maps(&p.serialize());
        result[1].insert(vec![0], raw);
        result[0].insert(vec![0xaa, 1], vec![0x99, 0x33]);
        result[1].insert(vec![0xaa, 2], vec![0x22, 0x44]);
        result
    }
    fn reject(maps: &[Map]) {
        let bytes = encode_maps(maps);
        assert!(crate::parse_psbt_extend(bytes.clone(), context(), Some(5)).is_err());
        assert!(crate::check_psbt_extend(bytes.clone(), context(), Some(5)).is_err());
        assert!(crate::sign_psbt_extend(bytes, &seed(), fp(), Some(5)).is_err());
    }
    fn verify_signed(signed: &[u8]) {
        let decoded = DashPsbt::decode(signed).unwrap();
        let p = &decoded.wrapper.psbt;
        let mut tx = p.unsigned_tx.clone();
        tx.input[0].script_sig = p.inputs[0]
            .witness_utxo
            .as_ref()
            .unwrap()
            .script_pubkey
            .clone();
        let mut preimage = serialize(&tx);
        preimage.extend_from_slice(&1u32.to_le_bytes());
        let message = Message::from_digest(sha256d::Hash::hash(&preimage).to_byte_array());
        for (key, sig) in &p.inputs[0].partial_sigs {
            assert_eq!(sig.sighash_type, bitcoin::sighash::EcdsaSighashType::All);
            Secp256k1::verification_only()
                .verify_ecdsa(&message, &sig.signature, &key.inner)
                .unwrap();
        }
        assert_eq!(p.inputs[0].partial_sigs.len(), 1);
    }
    #[test]
    fn dash_special_prevout_display_sign_and_preserve_wire_data() {
        let original = special();
        let bytes = encode_maps(&original);
        let display = crate::parse_psbt_extend(bytes.clone(), context(), Some(5)).unwrap();
        assert_eq!(display.overview.network, "Dash");
        assert!(!display.overview.has_witness_only_inputs);
        assert_eq!(
            display.overview.from,
            crate::parse_psbt_extend(fixture().serialize(), context(), Some(5))
                .unwrap()
                .overview
                .from
        );
        crate::check_psbt_extend(bytes.clone(), context(), Some(5)).unwrap();
        let signed = crate::sign_psbt_extend(bytes.clone(), &seed(), fp(), Some(5)).unwrap();
        verify_signed(&signed);
        assert!(
            crate::parse_psbt_hex_sign_status(&signed)
                .unwrap()
                .is_completed
        );
        assert_eq!(
            crate::parse_psbt_extend(signed.clone(), context(), Some(5))
                .unwrap()
                .overview
                .to,
            display.overview.to
        );
        let mut returned = maps(&signed);
        returned[1].retain(|key, _| key[0] != 2);
        assert_eq!(returned, original);
        assert!(!returned[1].contains_key(&vec![1]));
        assert_eq!(
            crate::sign_psbt_extend(signed.clone(), &seed(), fp(), Some(5)).unwrap(),
            signed
        );
        assert!(crate::sign_psbt(bytes.clone(), &seed(), fp()).is_err());
        assert!(crate::parse_psbt(bytes.clone(), context()).is_err());
        assert!(crate::check_psbt(bytes, context()).is_err());
    }
    #[test]
    fn dash_real_cbtx_txid_and_mixed_input_spend() {
        let raw = cbtx();
        let (previous, txid) = previous_transaction(&raw).unwrap();
        assert_eq!(
            txid.to_string(),
            "c550cae3f65d396a2f517a7220c7851cc3dc065e20141dadf9f19676348730f2"
        );
        assert_eq!(previous.output[1].value.to_sat(), 154021112);
        assert_ne!(previous.compute_txid(), txid);
        let mut p = fixture();
        let mut external = p.unsigned_tx.input[0].clone();
        external.previous_output = bitcoin::OutPoint::new(txid, 1);
        p.unsigned_tx.input.push(external);
        p.inputs.push(bitcoin::psbt::Input::default());
        let mut original = maps(&p.serialize());
        original[2].insert(vec![0], raw.clone());
        let bytes = encode_maps(&original);
        crate::parse_psbt_extend(bytes.clone(), context(), Some(5)).unwrap();
        crate::check_psbt_extend(bytes.clone(), context(), Some(5)).unwrap();
        let signed = crate::sign_psbt_extend(bytes, &seed(), fp(), Some(5)).unwrap();
        verify_signed(&signed);
        assert_eq!(maps(&signed)[2][&vec![0]], raw);
        assert!(crate::parse_psbt_hex_sign_status(&signed).is_ok());
    }
    #[test]
    fn dash_zero_input_special_prevout_and_global_xpub() {
        let p = fixture();
        let prevout = p.inputs[0].non_witness_utxo.as_ref().unwrap().output[0].clone();
        let mut raw = serialize(&0x00090003i32);
        raw.extend(serialize(&Vec::<bitcoin::TxIn>::new()));
        raw.extend(serialize(&vec![prevout]));
        raw.extend(serialize(&0u32));
        raw.extend(serialize(&VarInt(4)));
        raw.extend([1, 2, 3, 4]);
        let (previous, txid) = previous_transaction(&raw).unwrap();
        assert!(previous.input.is_empty());
        let mut p = p;
        p.unsigned_tx.input[0].previous_output.txid = txid;
        let path = DerivationPath::from_str("m/44'/5'/0'").unwrap();
        let xpub = keystore::algorithms::secp256k1::get_extended_public_key_by_seed(
            &seed(),
            &path.to_string(),
        )
        .unwrap();
        p.xpub.insert(xpub, (fp(), path));
        let mut wire = maps(&p.serialize());
        wire[1].insert(vec![0], raw.clone());
        let bytes = encode_maps(&wire);
        crate::check_psbt_extend(bytes.clone(), context(), Some(5)).unwrap();
        let signed = crate::sign_psbt_extend(bytes, &seed(), fp(), Some(5)).unwrap();
        verify_signed(&signed);
        assert_eq!(maps(&signed)[1][&vec![0]], raw);
        assert_eq!(
            crate::parse_psbt_extend(signed.clone(), context(), Some(5))
                .unwrap()
                .overview
                .network,
            "Dash"
        );
        assert!(
            crate::parse_psbt_hex_sign_status(&signed)
                .unwrap()
                .is_completed
        );
    }

    #[test]
    fn dash_rejects_tampered_and_missing_previous_transactions() {
        let original = special();
        let mut p = original.clone();
        *p[1].get_mut(&vec![0]).unwrap().last_mut().unwrap() ^= 1;
        reject(&p);
        let mut p = original.clone();
        p[1].remove(&vec![0]);
        reject(&p);
        let mut p = original.clone();
        let (_, used): (Transaction, usize) = deserialize_partial(&p[1][&vec![0]]).unwrap();
        p[1].get_mut(&vec![0]).unwrap().truncate(used);
        reject(&p);
        for change_txid in [false, true] {
            let mut p = original.clone();
            let mut tx: Transaction = deserialize(&p[0][&vec![0]]).unwrap();
            if change_txid {
                tx.input[0].previous_output.txid = Txid::all_zeros();
            } else {
                tx.input[0].previous_output.vout = u32::MAX;
            }
            p[0].insert(vec![0], serialize(&tx));
            reject(&p);
        }
        let prevout = fixture().inputs[0]
            .non_witness_utxo
            .as_ref()
            .unwrap()
            .output[0]
            .clone();
        let mut p = original.clone();
        p[1].insert(vec![1], serialize(&prevout));
        crate::check_psbt_extend(encode_maps(&p), context(), Some(5)).unwrap();
        let mut wrong_value = prevout;
        wrong_value.value = bitcoin::Amount::from_sat(1);
        p[1].insert(vec![1], serialize(&wrong_value));
        reject(&p);
        let mut p = original;
        let mut tx: Transaction = deserialize(&p[0][&vec![0]]).unwrap();
        tx.version = bitcoin::transaction::Version(0x00050003);
        p[0].insert(vec![0], serialize(&tx));
        reject(&p);
    }
    #[test]
    fn dash_rejects_truncation_duplicates_and_payload_length_errors() {
        let original = special();
        let bytes = encode_maps(&original);
        for end in 0..bytes.len() {
            assert!(DashPsbt::decode(&bytes[..end]).is_err(), "prefix {end}");
        }
        let mut trailing = bytes.clone();
        trailing.push(0);
        assert!(DashPsbt::decode(&trailing).is_err());
        let mut duplicate = b"psbt\xff".to_vec();
        for _ in 0..2 {
            duplicate.extend([1, 0]);
            let value = &original[0][&vec![0]];
            duplicate.extend(serialize(&VarInt(value.len() as u64)));
            duplicate.extend(value);
        }
        duplicate.push(0);
        assert!(DashPsbt::decode(&duplicate).is_err());
        let raw = &original[1][&vec![0]];
        let (_, used): (Transaction, usize) = deserialize_partial(raw).unwrap();
        for suffix in [
            vec![],
            vec![71],
            vec![0xfd, 70, 0],
            [serialize(&VarInt(10001)), vec![0; 10001]].concat(),
        ] {
            let mut broken = raw[..used].to_vec();
            broken.extend(suffix);
            assert!(previous_transaction(&broken).is_err());
        }
    }
}
