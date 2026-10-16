use crate::{
    errors::{BitcoinError, Result},
    network::Network,
};
use alloc::string::ToString;
use bitcoin::{
    bip32::{ChildNumber, DerivationPath},
    psbt::Psbt,
    NetworkKind,
};

fn invalid() -> BitcoinError {
    BitcoinError::InvalidPsbt("PSBT metadata conflicts with the PSBT Extend coin ID".to_string())
}

pub(crate) fn network_for_coin(coin_id: Option<i128>) -> Result<Network> {
    match coin_id {
        Some(2) => Ok(Network::Litecoin),
        Some(3) => Ok(Network::Dogecoin),
        Some(5) => Ok(Network::Dash),
        Some(145) => Ok(Network::BitcoinCash),
        _ => Err(BitcoinError::InvalidPsbt(
            "missing or unsupported PSBT Extend coin ID".to_string(),
        )),
    }
}

pub(crate) fn validate(psbt: &Psbt, coin_id: Option<i128>) -> Result<Network> {
    let network = network_for_coin(coin_id)?;
    let coin = coin_id.unwrap() as u32;
    if psbt.unknown.keys().any(|key| key.key == b"chain") {
        return Err(invalid());
    }
    let check_path = |path: &DerivationPath, account: bool| -> Result<()> {
        let parts = path.as_ref();
        if parts.len() != if account { 3 } else { 5 } {
            return Err(invalid());
        }
        let purpose_ok = match parts[0] {
            ChildNumber::Hardened { index } => {
                if coin == 2 {
                    matches!(index, 49 | 84)
                } else {
                    index == 44
                }
            }
            _ => false,
        };
        if !purpose_ok
            || parts[1] != (ChildNumber::Hardened { index: coin })
            || !parts[2].is_hardened()
            || parts[3..].iter().any(|part| part.is_hardened())
        {
            return Err(invalid());
        }
        Ok(())
    };
    for (xpub, (_, path)) in &psbt.xpub {
        if xpub.network != NetworkKind::Main {
            return Err(invalid());
        }
        check_path(path, true)?;
    }
    for input in &psbt.inputs {
        for (_, path) in input.bip32_derivation.values() {
            check_path(path, false)?;
        }
        if !input.tap_key_origins.is_empty()
            || input.tap_internal_key.is_some()
            || input.tap_merkle_root.is_some()
            || !input.tap_scripts.is_empty()
            || !input.tap_script_sigs.is_empty()
            || input.tap_key_sig.is_some()
        {
            return Err(invalid());
        }
    }
    for output in &psbt.outputs {
        for (_, path) in output.bip32_derivation.values() {
            check_path(path, false)?;
        }
        if !output.tap_key_origins.is_empty()
            || output.tap_internal_key.is_some()
            || output.tap_tree.is_some()
        {
            return Err(invalid());
        }
    }
    if matches!(coin, 3 | 5) && psbt.unsigned_tx.output.is_empty() {
        return Err(BitcoinError::NoOutputs);
    }
    if coin == 145 {
        super::bch::validate_outputs(psbt)?;
    }
    Ok(network)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{addresses::address::Address, parsed_tx::ParseContext};
    use alloc::{collections::BTreeMap, vec::Vec};
    use bitcoin::{
        bip32::{Fingerprint, Xpub},
        ScriptBuf,
    };
    use core::str::FromStr;
    fn seed() -> Vec<u8> {
        hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap()
    }
    fn fp() -> Fingerprint {
        Fingerprint::from_str("73c5da0a").unwrap()
    }
    fn account(coin: i128) -> (DerivationPath, Xpub) {
        let purpose = if coin == 2 { 84 } else { 44 };
        let path = DerivationPath::from_str(&format!("m/{purpose}'/{coin}'/0'")).unwrap();
        let xpub = keystore::algorithms::secp256k1::get_extended_public_key_by_seed(
            &seed(),
            &path.to_string(),
        )
        .unwrap();
        (path, xpub)
    }
    fn context(coin: i128) -> ParseContext {
        let (path, xpub) = account(coin);
        ParseContext::new(fp(), BTreeMap::from([(path, xpub)]), None, None)
    }
    fn fixture(coin: i128) -> Psbt {
        let data = match coin {
            2 => "70736274ff01007101000000011bfed2d08a2b45c790930953230bdc406eca13e579b5049eb751086b3ed369420000000000ffffffff02005a62020000000016001477f2208e272bef80e98ce956e05d5ed8f596f5a3f05f930300000000160014267f2c10ac39e483c00c982efdd0112bcc3e62830000000000010054010000000130297791cbb43ddbe1b17406f92f79b653a585f061213749fb069644b7f7ffbb00000000020101ffffffff0100e1f5050000000016001496cd3178f3de0f307bcee1aeaa4d5987862dd30a0000000001011f00e1f5050000000016001496cd3178f3de0f307bcee1aeaa4d5987862dd30a01030401000000220602e49c9b9b5d0f127235dc26a0c252814c52fb333d651a946773f59d72c2da99041873c5da0a540000800200008000000080000000000000000000002202029857513f0fe1dc125f219ffef22098e14c653c4bcb0a2aaeff47b3a252569f1a1873c5da0a540000800200008000000080010000000000000000",
            3 => "70736274ff0100770100000001faf994b04953ae7fbd735800e48605de15ded8126b9308556ad98962327ab3430000000000ffffffff02005a6202000000001976a9143bf96544e911ca728f5fabf5a4f3b65deab684c488acc0448403000000001976a91419115174dcc75cce4802aaec82e63b491274c2e288ac000000000001005701000000016f9512863e3f25696cc723f570d3c4077785f1944a895895a70750689453e48b00000000020101ffffffff0100e1f505000000001976a9144a483568665dcdfa68dd58a1f62893448a64333988ac0000000001030401000000220602cc6b0dc33aabcf3a23643e5e2919a80c50fb3dd2129ce409bbc5f0d4643d05e01873c5da0a2c0000800300008000000080000000000000000000002202020745c065a8b7cb03a843a57339cf9164d675a2e5010c6fbd61c09239bbb5f4df1873c5da0a2c0000800300008000000080010000000000000000",
            5 => "70736274ff010077010000000102754c72a9069560e14bf2faa47998f4442149e798cff5f8c953e7d92ab219d20000000000ffffffff02005a6202000000001976a9140a38ec3f3cc4c169c295ddb037c31c63bf2fa39188acf05f9303000000001976a914265a7d4ef63d643548a599c8fa53a0d9ef3fdca488ac0000000000010057010000000105d94a72185cf340264af0461c176c985bf0d55fef0026c94447f723ec03f90600000000020101ffffffff0100e1f505000000001976a9148a4f58c222cd5544c527bc66925652baa70b5e8088ac00000000010304010000002206026fa9a6f213b6ba86447965f6b4821264aaadd7521f049f00db9c43a770ea74051873c5da0a2c000080050000800000008000000000000000000000220202bd7b0ae9a8a0e67f82891d92545433e1b37074ab0bf5867b977f8c70b00f28671873c5da0a2c0000800500008000000080010000000000000000",
            145 => "70736274ff0100770100000001df92cf31d578229fa876e7bf2f5696798f4d18d28e0066010e67feb6f5fb00b90000000000ffffffff02005a6202000000001976a9144f04b6e897c1a3dfe2b74bfc40cbc4d9c64daf3088ac18839303000000001976a914cfdcdfa3e0483ddd3c7b6ab6ee13c5087c26d5f188ac0000000000010057010000000153ef901d37f8bfee6f9453ff7b8dc3e0672ee715498dae97db9dca73261f9a3300000000020101ffffffff0100e1f505000000001976a914086a977c7dad32a56996af2ce90a727238d4899888ac0000000001030441000000220602bbe7dbcdf8b2261530a867df7180b17a90b482f74f2736b8a30d3f756e42e2171873c5da0a2c000080910000800000008000000000000000000000220202f178b9410d11e47e10bef8a8e4bc53e48d9ae15864a024a54f87a802dad5f5141873c5da0a2c0000809100008000000080010000000000000000",
            _ => unreachable!(),
        };
        Psbt::deserialize(&hex::decode(data).unwrap()).unwrap()
    }
    fn rejected_at_all_stages(psbt: &Psbt, coin: i128) {
        assert!(crate::parse_psbt_extend(psbt.serialize(), context(coin), Some(coin)).is_err());
        assert!(crate::check_psbt_extend(psbt.serialize(), context(coin), Some(coin)).is_err());
        assert!(crate::sign_psbt_extend(psbt.serialize(), &seed(), fp(), Some(coin)).is_err());
    }

    #[test]
    fn invalid_coin_ids_are_rejected_at_every_extend_entry() {
        for coin_id in [
            None,
            Some(0),
            Some(1),
            Some(4),
            Some(-1),
            Some(146),
            Some(i128::MIN),
            Some(i128::MAX),
        ] {
            for bytes in [fixture(3).serialize(), Vec::new()] {
                let expected = BitcoinError::InvalidPsbt(
                    "missing or unsupported PSBT Extend coin ID".to_string(),
                );
                assert_eq!(
                    crate::parse_psbt_extend(bytes.clone(), context(3), coin_id).unwrap_err(),
                    expected
                );
                assert_eq!(
                    crate::check_psbt_extend(bytes.clone(), context(3), coin_id).unwrap_err(),
                    expected
                );
                assert_eq!(
                    crate::sign_psbt_extend(bytes, &seed(), fp(), coin_id).unwrap_err(),
                    expected
                );
            }
        }
    }

    #[test]
    fn global_xpub_cannot_override_any_extend_network() {
        for (coin, name) in [
            (2, "Litecoin"),
            (3, "Dogecoin"),
            (5, "Dash"),
            (145, "Bitcoin Cash"),
        ] {
            let mut p = fixture(coin);
            let (path, xpub) = account(coin);
            p.xpub.insert(xpub, (fp(), path));
            let parsed =
                crate::parse_psbt_extend(p.serialize(), context(coin), Some(coin)).unwrap();
            assert_eq!(parsed.overview.network, name);
            let original =
                crate::parse_psbt_extend(fixture(coin).serialize(), context(coin), Some(coin))
                    .unwrap();
            assert_eq!(parsed.overview.from, original.overview.from);
            assert_eq!(parsed.overview.to, original.overview.to);
            crate::check_psbt_extend(p.serialize(), context(coin), Some(coin)).unwrap();
            let signed = crate::sign_psbt_extend(p.serialize(), &seed(), fp(), Some(coin)).unwrap();
            assert!(
                crate::parse_psbt_hex_sign_status(&signed)
                    .unwrap()
                    .is_completed
            );
            if coin == 145 {
                assert!(crate::sign_psbt(signed.clone(), &seed(), fp()).is_err());
            }
            let signed_display =
                crate::parse_psbt_extend(signed, context(coin), Some(coin)).unwrap();
            assert_eq!(signed_display.overview.network, name);
            assert_eq!(signed_display.overview.to, parsed.overview.to);
        }
    }

    #[test]
    fn bch_sign_status_rejects_conflicting_global_xpub_metadata() {
        let (path, xpub) = account(145);
        let mut testnet_xpub = xpub;
        testnet_xpub.network = NetworkKind::Test;
        for (key, origin) in [
            (xpub, DerivationPath::from_str("m/44'/0'/0'").unwrap()),
            (testnet_xpub, path),
            (xpub, DerivationPath::default()),
        ] {
            let mut p = fixture(145);
            p.xpub.insert(key, (fp(), origin));
            let signed = super::super::bch::sign(&p.serialize(), &seed(), fp()).unwrap();
            assert!(crate::parse_psbt_hex_sign_status(&signed).is_err());
        }
    }

    #[test]
    fn conflicting_metadata_is_rejected_before_approval_and_signing() {
        for coin in [2, 3, 5, 145] {
            let original = fixture(coin);
            let (path, mut xpub) = account(coin);
            let mut p = original.clone();
            p.xpub.insert(
                xpub,
                (fp(), DerivationPath::from_str("m/44'/0'/0'").unwrap()),
            );
            rejected_at_all_stages(&p, coin);
            let mut p = original.clone();
            xpub.network = NetworkKind::Test;
            p.xpub.insert(xpub, (fp(), path));
            rejected_at_all_stages(&p, coin);
            let mut p = original.clone();
            p.inputs[0].bip32_derivation.values_mut().next().unwrap().1 =
                DerivationPath::from_str("m/44'/0'/0'/0/0").unwrap();
            rejected_at_all_stages(&p, coin);
            let mut p = original.clone();
            p.outputs[1].bip32_derivation.values_mut().next().unwrap().1 =
                DerivationPath::from_str("m/44'/0'/0'/1/0").unwrap();
            rejected_at_all_stages(&p, coin);
            let mut p = original.clone();
            p.inputs[0].bip32_derivation.values_mut().next().unwrap().1 = DerivationPath::default();
            rejected_at_all_stages(&p, coin);
            for marker in [b"fb".to_vec(), b"tfb".to_vec(), b"unknown".to_vec()] {
                let mut p = original.clone();
                p.unknown.insert(
                    bitcoin::psbt::raw::Key {
                        type_value: 0xfc,
                        key: b"chain".to_vec(),
                    },
                    marker,
                );
                rejected_at_all_stages(&p, coin);
            }
        }
    }

    #[test]
    fn bch_foreign_input_display_keys_must_match_authenticated_prevout() {
        let mut p = fixture(145);
        let mut foreign = p.inputs[0].clone();
        let (foreign_key, (_, foreign_path)) = p.outputs[1]
            .bip32_derivation
            .first_key_value()
            .map(|(key, origin)| (*key, origin.clone()))
            .unwrap();
        foreign.bip32_derivation =
            BTreeMap::from([(foreign_key, (Fingerprint::default(), foreign_path.clone()))]);
        let previous = foreign.non_witness_utxo.as_mut().unwrap();
        previous.output[0].script_pubkey =
            ScriptBuf::new_p2pkh(&bitcoin::PublicKey::new(foreign_key).pubkey_hash());
        let mut txin = p.unsigned_tx.input[0].clone();
        txin.previous_output.txid = previous.compute_txid();
        p.inputs.push(foreign);
        p.unsigned_tx.input.push(txin);

        let expected = Address::from_script(
            &p.inputs[1].non_witness_utxo.as_ref().unwrap().output[0].script_pubkey,
            Network::BitcoinCash,
        )
        .unwrap()
        .to_string();
        let parsed = crate::parse_psbt_extend(p.serialize(), context(145), Some(145)).unwrap();
        assert!(parsed.overview.from.iter().any(|input| input == &expected));
        crate::check_psbt_extend(p.serialize(), context(145), Some(145)).unwrap();
        let signed = crate::sign_psbt_extend(p.serialize(), &seed(), fp(), Some(145)).unwrap();
        let decoded = super::super::bch::deserialize(&signed).unwrap();
        assert_eq!(decoded.inputs[0].partial_sigs.len(), 1);
        assert!(decoded.inputs[1].partial_sigs.is_empty());

        let mut forged = p.clone();
        let fake_key = *p.inputs[0].bip32_derivation.keys().next().unwrap();
        forged.inputs[1].bip32_derivation =
            BTreeMap::from([(fake_key, (Fingerprint::default(), foreign_path.clone()))]);
        let expected_error = BitcoinError::InvalidPsbt(
            "BCH input public key does not match previous output".to_string(),
        );
        assert_eq!(
            crate::parse_psbt_extend(forged.serialize(), context(145), Some(145)).unwrap_err(),
            expected_error
        );
        assert_eq!(
            crate::check_psbt_extend(forged.serialize(), context(145), Some(145)).unwrap_err(),
            expected_error
        );
        assert_eq!(
            crate::sign_psbt_extend(forged.serialize(), &seed(), fp(), Some(145)).unwrap_err(),
            expected_error
        );
        forged.inputs[1]
            .bip32_derivation
            .extend(p.inputs[1].bip32_derivation.clone());
        rejected_at_all_stages(&forged, 145);

        let mut missing = p.clone();
        missing.inputs[1].non_witness_utxo = None;
        rejected_at_all_stages(&missing, 145);
        let mut tampered = p.clone();
        tampered.inputs[1].non_witness_utxo.as_mut().unwrap().output[0].value =
            bitcoin::Amount::from_sat(1);
        rejected_at_all_stages(&tampered, 145);
        let mut out_of_bounds = p.clone();
        out_of_bounds.unsigned_tx.input[1].previous_output.vout = u32::MAX;
        rejected_at_all_stages(&out_of_bounds, 145);
        let mut mismatch = p;
        let mut witness = mismatch.inputs[1].non_witness_utxo.as_ref().unwrap().output[0].clone();
        witness.value = bitcoin::Amount::from_sat(1);
        mismatch.inputs[1].witness_utxo = Some(witness);
        rejected_at_all_stages(&mismatch, 145);
    }

    #[test]
    fn bch_p2sh_recipient_is_exact_cashaddr_and_signable() {
        let script = ScriptBuf::from_bytes(
            hex::decode("a91476a04053bda0a88bda5177b86a15c3b29f55987387").unwrap(),
        );
        let expected = "ppm2qsznhks23z7629mms6s4cwef74vcwvn0h829pq";
        let address = Address::from_script(&script, Network::BitcoinCash)
            .unwrap()
            .to_string();
        assert_eq!(address, expected);
        assert_eq!(Address::from_str(expected).unwrap().script_pubkey(), script);
        let mut p = fixture(145);
        p.unsigned_tx.output[0].script_pubkey = script;
        let display = crate::parse_psbt_extend(p.serialize(), context(145), Some(145)).unwrap();
        assert_eq!(display.detail.to[0].address, expected);
        assert!(display
            .overview
            .to
            .iter()
            .any(|output| output.address == expected));
        crate::check_psbt_extend(p.serialize(), context(145), Some(145)).unwrap();
        let signed = crate::sign_psbt_extend(p.serialize(), &seed(), fp(), Some(145)).unwrap();
        let signed_display = crate::parse_psbt_extend(signed, context(145), Some(145)).unwrap();
        assert_eq!(signed_display.detail.to[0].address, expected);
        let mut different_script = p.unsigned_tx.output[0].script_pubkey.clone().into_bytes();
        different_script[2] ^= 1;
        let different_script = ScriptBuf::from_bytes(different_script);
        let different_address = Address::from_script(&different_script, Network::BitcoinCash)
            .unwrap()
            .to_string();
        assert_ne!(different_address, expected);
        assert_eq!(
            Address::from_str(&different_address)
                .unwrap()
                .script_pubkey(),
            different_script
        );
    }

    #[test]
    fn doge_dash_non_address_outputs_display_scripts_and_sign() {
        let mut witness_script_hash = vec![0x00, 0x20];
        witness_script_hash.extend_from_slice(&[1; 32]);
        let mut taproot = vec![0x51, 0x20];
        taproot.extend_from_slice(&[1; 32]);
        for coin in [3, 5] {
            for script in [
                ScriptBuf::new(),
                ScriptBuf::from_bytes(vec![0x51]),
                ScriptBuf::from_bytes(
                    hex::decode("001476a04053bda0a88bda5177b86a15c3b29f559873").unwrap(),
                ),
                ScriptBuf::from_bytes(witness_script_hash.clone()),
                ScriptBuf::from_bytes(taproot.clone()),
            ] {
                for index in 0..fixture(coin).unsigned_tx.output.len() {
                    let mut p = fixture(coin);
                    p.unsigned_tx.output[index].script_pubkey = script.clone();
                    let display =
                        crate::parse_psbt_extend(p.serialize(), context(coin), Some(coin)).unwrap();
                    let expected = if script.is_empty() {
                        "(empty script)".to_string()
                    } else {
                        script.to_string()
                    };
                    assert_eq!(display.detail.to[index].address, expected);
                    assert!(!display.detail.to[index].is_mine);
                    assert!(display
                        .overview
                        .to
                        .iter()
                        .any(|output| output.address == expected));
                    crate::check_psbt_extend(p.serialize(), context(coin), Some(coin)).unwrap();
                    let signed =
                        crate::sign_psbt_extend(p.serialize(), &seed(), fp(), Some(coin)).unwrap();
                    assert_eq!(
                        Psbt::deserialize(&signed).unwrap().unsigned_tx.output,
                        p.unsigned_tx.output
                    );
                }
            }
            let mut p = fixture(coin);
            p.unsigned_tx.output.clear();
            p.outputs.clear();
            rejected_at_all_stages(&p, coin);
        }
    }

    #[test]
    fn doge_dash_op_return_displays_script_and_preserves_output_when_signing() {
        for coin in [3, 5] {
            for script in [
                ScriptBuf::from_bytes(vec![0x6a]),
                ScriptBuf::from_bytes(hex::decode("6a0568656c6c6f").unwrap()),
                ScriptBuf::from_bytes(hex::decode("6a0300ff80").unwrap()),
            ] {
                for index in 0..fixture(coin).unsigned_tx.output.len() {
                    let mut p = fixture(coin);
                    p.unsigned_tx.output[index].script_pubkey = script.clone();
                    let display =
                        crate::parse_psbt_extend(p.serialize(), context(coin), Some(coin)).unwrap();
                    assert_eq!(display.detail.to[index].address, script.to_string());
                    assert_eq!(
                        display.detail.to[index].value,
                        p.unsigned_tx.output[index].value.to_sat()
                    );
                    assert!(!display.detail.to[index].is_mine);
                    assert!(display
                        .overview
                        .to
                        .iter()
                        .any(|output| output.address == script.to_string()));
                    crate::check_psbt_extend(p.serialize(), context(coin), Some(coin)).unwrap();
                    let signed =
                        crate::sign_psbt_extend(p.serialize(), &seed(), fp(), Some(coin)).unwrap();
                    let signed = Psbt::deserialize(&signed).unwrap();
                    assert_eq!(signed.unsigned_tx.output, p.unsigned_tx.output);
                    assert!(!signed.inputs[0].partial_sigs.is_empty());
                }
            }
        }
    }

    #[test]
    fn doge_dash_supported_outputs_display_and_sign() {
        for coin in [3, 5] {
            for script in [
                fixture(coin).unsigned_tx.output[0].script_pubkey.clone(),
                ScriptBuf::from_bytes(
                    hex::decode("a91476a04053bda0a88bda5177b86a15c3b29f55987387").unwrap(),
                ),
            ] {
                let mut p = fixture(coin);
                p.unsigned_tx.output[0].script_pubkey = script.clone();
                let display =
                    crate::parse_psbt_extend(p.serialize(), context(coin), Some(coin)).unwrap();
                let address = &display.detail.to[0].address;
                assert_eq!(Address::from_str(address).unwrap().script_pubkey(), script);
                assert!(display
                    .overview
                    .to
                    .iter()
                    .any(|output| &output.address == address));
                crate::check_psbt_extend(p.serialize(), context(coin), Some(coin)).unwrap();
                let signed =
                    crate::sign_psbt_extend(p.serialize(), &seed(), fp(), Some(coin)).unwrap();
                let signed_display =
                    crate::parse_psbt_extend(signed, context(coin), Some(coin)).unwrap();
                assert_eq!(signed_display.detail.to[0].address, *address);
            }
        }
    }

    #[test]
    fn bch_unrenderable_outputs_rejected_at_all_stages() {
        for script in [
            ScriptBuf::new(),
            ScriptBuf::from_bytes(vec![0x6a]),
            ScriptBuf::from_bytes(
                hex::decode("001476a04053bda0a88bda5177b86a15c3b29f559873").unwrap(),
            ),
        ] {
            let mut p = fixture(145);
            p.unsigned_tx.output[0].script_pubkey = script;
            rejected_at_all_stages(&p, 145);
        }
    }
}
