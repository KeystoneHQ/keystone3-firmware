use crate::errors::{CardanoError, R};
use crate::structs::{ParseContext, ParsedCardanoTx, ParsedCardanoSignData, SignDataResult};
use alloc::collections::BTreeMap;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use cardano_serialization_lib;
use cardano_serialization_lib::crypto::{Ed25519Signature, PublicKey, Vkey, Vkeywitness};
use third_party::cryptoxide::hashing::blake2b_256;
use third_party::hex;

pub fn parse_tx(tx: Vec<u8>, context: ParseContext) -> R<ParsedCardanoTx> {
    let cardano_tx =
        cardano_serialization_lib::protocol_types::fixed_tx::FixedTransaction::from_bytes(tx)?;
    ParsedCardanoTx::from_cardano_tx(cardano_tx, context)
}

pub fn parse_sign_data(sign_data: Vec<u8>, derviation_path: String) -> R<ParsedCardanoSignData> {
    ParsedCardanoSignData::build(sign_data, derviation_path)
}

pub fn check_tx(tx: Vec<u8>, context: ParseContext) -> R<()> {
    let cardano_tx =
        cardano_serialization_lib::protocol_types::fixed_tx::FixedTransaction::from_bytes(tx)?;
    ParsedCardanoTx::verify(cardano_tx, context)
}

pub fn sign_data(
    path: &String,
    payload: &str,
    entropy: &[u8],
    passphrase: &[u8],
) -> R<SignDataResult> {
    let icarus_master_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_icarus_master_key_by_entropy(
            entropy, passphrase,
        )
        .map_err(|e| CardanoError::SigningFailed(e.to_string()))?;
    let bip32_signing_key =
        keystore::algorithms::ed25519::bip32_ed25519::derive_extended_privkey_by_xprv(
            &icarus_master_key,
            path,
        )
        .unwrap();
    let signed_data = bip32_signing_key.sign::<Vec<u8>>(&hex::decode(payload).unwrap());
    let pub_key = bip32_signing_key.public().public_key().to_vec();
    Ok(SignDataResult::new(pub_key, signed_data.to_bytes().to_vec()))
}

pub fn sign_tx(
    tx: Vec<u8>,
    context: ParseContext,
    entropy: &[u8],
    passphrase: &[u8],
) -> R<Vec<u8>> {
    let cardano_tx =
        cardano_serialization_lib::protocol_types::fixed_tx::FixedTransaction::from_bytes(tx)?;
    let hash = blake2b_256(cardano_tx.raw_body().as_ref());
    let mut witness_set = cardano_serialization_lib::TransactionWitnessSet::new();
    let mut vkeys = cardano_serialization_lib::crypto::Vkeywitnesses::new();
    let icarus_master_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_icarus_master_key_by_entropy(
            entropy, passphrase,
        )
        .map_err(|e| CardanoError::SigningFailed(e.to_string()))?;

    let mut signatures = BTreeMap::new();

    for utxo in context.get_utxos() {
        if !utxo
            .get_master_fingerprint()
            .eq(&context.get_master_fingerprint())
        {
            continue;
        }
        match keystore::algorithms::ed25519::bip32_ed25519::derive_extended_pubkey_by_xprv(
            &icarus_master_key,
            &utxo.get_path().to_string(),
        )
        .map(|v| v.public_key())
        .map_err(|e| CardanoError::SigningFailed(e.to_string()))
        {
            Ok(pubkey) => {
                if signatures.contains_key(&pubkey) {
                    continue;
                }
                match keystore::algorithms::ed25519::bip32_ed25519::sign_message_by_xprv(
                    &icarus_master_key,
                    &hash,
                    &utxo.get_path().to_string(),
                )
                .map_err(|e| CardanoError::SigningFailed(e.to_string()))
                {
                    Ok(signature) => {
                        signatures.insert(pubkey, signature);
                    }
                    Err(e) => return Err(e),
                }
            }
            Err(e) => return Err(e),
        }
    }

    for signer in context.get_cert_keys() {
        if !signer
            .get_master_fingerprint()
            .eq(&context.get_master_fingerprint())
        {
            continue;
        }
        match keystore::algorithms::ed25519::bip32_ed25519::derive_extended_pubkey_by_xprv(
            &icarus_master_key,
            &signer.get_path().to_string(),
        )
        .map(|v| v.public_key())
        .map_err(|e| CardanoError::SigningFailed(e.to_string()))
        {
            Ok(pubkey) => {
                if signatures.contains_key(&pubkey) {
                    continue;
                }
                match keystore::algorithms::ed25519::bip32_ed25519::sign_message_by_xprv(
                    &icarus_master_key,
                    &hash,
                    &signer.get_path().to_string(),
                )
                .map_err(|e| CardanoError::SigningFailed(e.to_string()))
                {
                    Ok(signature) => {
                        signatures.insert(pubkey, signature);
                    }
                    Err(e) => return Err(e),
                }
            }
            Err(e) => return Err(e),
        }
    }
    for (pubkey, signature) in signatures {
        let v = Vkeywitness::new(
            &Vkey::new(
                &PublicKey::from_bytes(&pubkey)
                    .map_err(|e| CardanoError::SigningFailed(e.to_string()))?,
            ),
            &Ed25519Signature::from_bytes(signature.to_vec())
                .map_err(|e| CardanoError::SigningFailed(e.to_string()))?,
        );
        vkeys.add(&v);
    }
    witness_set.set_vkeys(&vkeys);

    Ok(witness_set.to_bytes())
}

#[cfg(test)]
mod test {
    use super::*;
    use cardano_serialization_lib::Transaction;

    extern crate std;

    use crate::transaction::parse_tx;
    use std::println;
    use third_party::{cryptoxide::hashing::blake2b_256, hex};
    use third_party::ur_registry::cardano::cardano_sign_data_signature::CardanoSignDataSignature;

    #[test]
    fn spike_transaction() {
        let tx_hex = "84a80083825820878132f34ab1ae44afa85b6a1f8c1305f02ca4f1030d52f02032cbf5d0d1347f00825820cf74b8e2cdb7fa9273d26142dfe71813f1ff399faa159ced431cf1e132836a3503825820cf74b8e2cdb7fa9273d26142dfe71813f1ff399faa159ced431cf1e132836a35040182a300581d71edfff663d37fc5f9753bc4222e0da2bfe08aa48db0837d2c329adeb301821a004ddd98a2581c22f6999d4effc0ade05f6e1a70b702c65d6b3cdf0e301e4a8267f585a158201df45c7e73e680c256ce0eb777f1299aca718d44aea8ae17c61feae58341b2f801581cdda5fdb1002f7389b33e036b6afee82a8189becb6cba852e8b79b4fba1480014df1047454e531b00000001c37f3dac028200582057e5829401ce8998a6defdda9033dc5a7ba75e67f7d34bce27970cbaf208dd51a20058390125e59a44f460455dba1187f2e13c69dae31bc979e7a408a96d4de5a91ef024cbafb575d639d0b9f67bef0e56937045e0c19f43f75701fa7601821a004ea300a1581cdda5fdb1002f7389b33e036b6afee82a8189becb6cba852e8b79b4fba1480014df1047454e531a005cecc9021a0004a8b80b58208b89de52350671a9d946810a2ff18efd6251349bcb0be48636dc66b4d6df7e650d81825820cf74b8e2cdb7fa9273d26142dfe71813f1ff399faa159ced431cf1e132836a350010a20058390125e59a44f460455dba1187f2e13c69dae31bc979e7a408a96d4de5a91ef024cbafb575d639d0b9f67bef0e56937045e0c19f43f75701fa76011a00454e2c111a0006fd141282825820062f97b0e64130bc18b4a227299a62d6d59a4ea852a4c90db3de2204a2cd19ea0282582018722887e72afbb28dd4f24b401ea95af7c178148a3488d03adf582e0a0879ff00a20482d8799f581cd7a2b4740596a8095e94896e96500047bb34c836eb81247ed70c45b1d8799fd8799f581cd7a2b4740596a8095e94896e96500047bb34c836eb81247ed70c45b1ffd87a80ffd8799f581cdda5fdb1002f7389b33e036b6afee82a8189becb6cba852e8b79b4fb480014df1047454e53ff1b00000001c256acc21b00000001c2256181d8799f4040ffd8799f1b07bd198288051c751b1900000000000000ff58201df45c7e73e680c256ce0eb777f1299aca718d44aea8ae17c61feae58341b2f8d87a80d87a80011a000f42401a000f4240d8799f1a001e84801a0159dc2b190bb8ff1a000f4240ffd8799f581cd7a2b4740596a8095e94896e96500047bb34c836eb81247ed70c45b1d8799fd8799f581cd7a2b4740596a8095e94896e96500047bb34c836eb81247ed70c45b1ffd87a80ffd8799f581cdda5fdb1002f7389b33e036b6afee82a8189becb6cba852e8b79b4fb480014df1047454e53ff1b00000001c256acc21b00000001c256acc2d8799f4040ffd8799f1b07bd198288051c751b1900000000000000ff58201df45c7e73e680c256ce0eb777f1299aca718d44aea8ae17c61feae58341b2f8d87a80d87a80001a000f42401a000f4240d8799f1a000f42401a0159dc2b00ff00ff0581840000d87a9f1a00314b41ff821a000e27be1a15eab7dcf5f6";
        // let tx = Transaction::from_hex(tx_hex).unwrap();
        // println!("{}", tx.to_json().unwrap());

        let tx_hex = hex::decode(tx_hex).unwrap();
        let cardano_tx = cardano_serialization_lib::Transaction::from_bytes(tx_hex).unwrap();
        println!("{}", cardano_tx.to_json().unwrap());
    }

    #[test]
    fn spike_fixed_transaction() {
        //tx: 84a80083825820878132f34ab1ae44afa85b6a1f8c1305f02ca4f1030d52f02032cbf5d0d1347f00825820cf74b8e2cdb7fa9273d26142dfe71813f1ff399faa159ced431cf1e132836a3503825820cf74b8e2cdb7fa9273d26142dfe71813f1ff399faa159ced431cf1e132836a35040182a300581d71edfff663d37fc5f9753bc4222e0da2bfe08aa48db0837d2c329adeb301821a004ddd98a2581c22f6999d4effc0ade05f6e1a70b702c65d6b3cdf0e301e4a8267f585a158201df45c7e73e680c256ce0eb777f1299aca718d44aea8ae17c61feae58341b2f801581cdda5fdb1002f7389b33e036b6afee82a8189becb6cba852e8b79b4fba1480014df1047454e531b00000001c37f3dac028200582057e5829401ce8998a6defdda9033dc5a7ba75e67f7d34bce27970cbaf208dd51a20058390125e59a44f460455dba1187f2e13c69dae31bc979e7a408a96d4de5a91ef024cbafb575d639d0b9f67bef0e56937045e0c19f43f75701fa7601821a004ea300a1581cdda5fdb1002f7389b33e036b6afee82a8189becb6cba852e8b79b4fba1480014df1047454e531a005cecc9021a0004a8b80b58208b89de52350671a9d946810a2ff18efd6251349bcb0be48636dc66b4d6df7e650d81825820cf74b8e2cdb7fa9273d26142dfe71813f1ff399faa159ced431cf1e132836a350010a20058390125e59a44f460455dba1187f2e13c69dae31bc979e7a408a96d4de5a91ef024cbafb575d639d0b9f67bef0e56937045e0c19f43f75701fa76011a00454e2c111a0006fd141282825820062f97b0e64130bc18b4a227299a62d6d59a4ea852a4c90db3de2204a2cd19ea0282582018722887e72afbb28dd4f24b401ea95af7c178148a3488d03adf582e0a0879ff00a20482d8799f581cd7a2b4740596a8095e94896e96500047bb34c836eb81247ed70c45b1d8799fd8799f581cd7a2b4740596a8095e94896e96500047bb34c836eb81247ed70c45b1ffd87a80ffd8799f581cdda5fdb1002f7389b33e036b6afee82a8189becb6cba852e8b79b4fb480014df1047454e53ff1b00000001c256acc21b00000001c2256181d8799f4040ffd8799f1b07bd198288051c751b1900000000000000ff58201df45c7e73e680c256ce0eb777f1299aca718d44aea8ae17c61feae58341b2f8d87a80d87a80011a000f42401a000f4240d8799f1a001e84801a0159dc2b190bb8ff1a000f4240ffd8799f581cd7a2b4740596a8095e94896e96500047bb34c836eb81247ed70c45b1d8799fd8799f581cd7a2b4740596a8095e94896e96500047bb34c836eb81247ed70c45b1ffd87a80ffd8799f581cdda5fdb1002f7389b33e036b6afee82a8189becb6cba852e8b79b4fb480014df1047454e53ff1b00000001c256acc21b00000001c256acc2d8799f4040ffd8799f1b07bd198288051c751b1900000000000000ff58201df45c7e73e680c256ce0eb777f1299aca718d44aea8ae17c61feae58341b2f8d87a80d87a80001a000f42401a000f4240d8799f1a000f42401a0159dc2b00ff00ff0581840000d87a9f1a00314b41ff821a000e27be1a15eab7dcf5f6
        //tx body: a80083825820878132f34ab1ae44afa85b6a1f8c1305f02ca4f1030d52f02032cbf5d0d1347f00825820cf74b8e2cdb7fa9273d26142dfe71813f1ff399faa159ced431cf1e132836a3503825820cf74b8e2cdb7fa9273d26142dfe71813f1ff399faa159ced431cf1e132836a35040182a300581d71edfff663d37fc5f9753bc4222e0da2bfe08aa48db0837d2c329adeb301821a004ddd98a2581c22f6999d4effc0ade05f6e1a70b702c65d6b3cdf0e301e4a8267f585a158201df45c7e73e680c256ce0eb777f1299aca718d44aea8ae17c61feae58341b2f801581cdda5fdb1002f7389b33e036b6afee82a8189becb6cba852e8b79b4fba1480014df1047454e531b00000001c37f3dac028200582057e5829401ce8998a6defdda9033dc5a7ba75e67f7d34bce27970cbaf208dd51a20058390125e59a44f460455dba1187f2e13c69dae31bc979e7a408a96d4de5a91ef024cbafb575d639d0b9f67bef0e56937045e0c19f43f75701fa7601821a004ea300a1581cdda5fdb1002f7389b33e036b6afee82a8189becb6cba852e8b79b4fba1480014df1047454e531a005cecc9021a0004a8b80b58208b89de52350671a9d946810a2ff18efd6251349bcb0be48636dc66b4d6df7e650d81825820cf74b8e2cdb7fa9273d26142dfe71813f1ff399faa159ced431cf1e132836a350010a20058390125e59a44f460455dba1187f2e13c69dae31bc979e7a408a96d4de5a91ef024cbafb575d639d0b9f67bef0e56937045e0c19f43f75701fa76011a00454e2c111a0006fd141282825820062f97b0e64130bc18b4a227299a62d6d59a4ea852a4c90db3de2204a2cd19ea0282582018722887e72afbb28dd4f24b401ea95af7c178148a3488d03adf582e0a0879ff00
        //hash: 6dadb65461bf4e1e9a983f16ba033ca0b490fc3d845db2ec6bf6789a14e7e9c3
        //signature: a100828258201e7a836e4d144ea0a3c2f53878dbbb3e4583476d2a970fe5b693bc1d7974faf8584020ef5f3eedfaec7af8c1dafc575fc7af38cd1a2d7215ae2fb7f3f81415caea18f01cc125974133dda35e89bdd2c3b662f33509437564a88f106084f7693a0f0b8258207508644588abb8da3996ed16c70162c730af99b8e43c730d4219ba61b78b463a5840a4a620028627eb384790530010a71349a22dc2c533edafba3fd0dcd60c7b593bd3ba999ec041e429b70ae19feb2f6660cecc3b5715b0e4e4118878878969f80f
        let tx_bytes = hex::decode("84a80083825820878132f34ab1ae44afa85b6a1f8c1305f02ca4f1030d52f02032cbf5d0d1347f00825820cf74b8e2cdb7fa9273d26142dfe71813f1ff399faa159ced431cf1e132836a3503825820cf74b8e2cdb7fa9273d26142dfe71813f1ff399faa159ced431cf1e132836a35040182a300581d71edfff663d37fc5f9753bc4222e0da2bfe08aa48db0837d2c329adeb301821a004ddd98a2581c22f6999d4effc0ade05f6e1a70b702c65d6b3cdf0e301e4a8267f585a158201df45c7e73e680c256ce0eb777f1299aca718d44aea8ae17c61feae58341b2f801581cdda5fdb1002f7389b33e036b6afee82a8189becb6cba852e8b79b4fba1480014df1047454e531b00000001c37f3dac028200582057e5829401ce8998a6defdda9033dc5a7ba75e67f7d34bce27970cbaf208dd51a20058390125e59a44f460455dba1187f2e13c69dae31bc979e7a408a96d4de5a91ef024cbafb575d639d0b9f67bef0e56937045e0c19f43f75701fa7601821a004ea300a1581cdda5fdb1002f7389b33e036b6afee82a8189becb6cba852e8b79b4fba1480014df1047454e531a005cecc9021a0004a8b80b58208b89de52350671a9d946810a2ff18efd6251349bcb0be48636dc66b4d6df7e650d81825820cf74b8e2cdb7fa9273d26142dfe71813f1ff399faa159ced431cf1e132836a350010a20058390125e59a44f460455dba1187f2e13c69dae31bc979e7a408a96d4de5a91ef024cbafb575d639d0b9f67bef0e56937045e0c19f43f75701fa76011a00454e2c111a0006fd141282825820062f97b0e64130bc18b4a227299a62d6d59a4ea852a4c90db3de2204a2cd19ea0282582018722887e72afbb28dd4f24b401ea95af7c178148a3488d03adf582e0a0879ff00a20482d8799f581cd7a2b4740596a8095e94896e96500047bb34c836eb81247ed70c45b1d8799fd8799f581cd7a2b4740596a8095e94896e96500047bb34c836eb81247ed70c45b1ffd87a80ffd8799f581cdda5fdb1002f7389b33e036b6afee82a8189becb6cba852e8b79b4fb480014df1047454e53ff1b00000001c256acc21b00000001c2256181d8799f4040ffd8799f1b07bd198288051c751b1900000000000000ff58201df45c7e73e680c256ce0eb777f1299aca718d44aea8ae17c61feae58341b2f8d87a80d87a80011a000f42401a000f4240d8799f1a001e84801a0159dc2b190bb8ff1a000f4240ffd8799f581cd7a2b4740596a8095e94896e96500047bb34c836eb81247ed70c45b1d8799fd8799f581cd7a2b4740596a8095e94896e96500047bb34c836eb81247ed70c45b1ffd87a80ffd8799f581cdda5fdb1002f7389b33e036b6afee82a8189becb6cba852e8b79b4fb480014df1047454e53ff1b00000001c256acc21b00000001c256acc2d8799f4040ffd8799f1b07bd198288051c751b1900000000000000ff58201df45c7e73e680c256ce0eb777f1299aca718d44aea8ae17c61feae58341b2f8d87a80d87a80001a000f42401a000f4240d8799f1a000f42401a0159dc2b00ff00ff0581840000d87a9f1a00314b41ff821a000e27be1a15eab7dcf5f6").unwrap();
        let tx = cardano_serialization_lib::protocol_types::fixed_tx::FixedTransaction::from_bytes(
            tx_bytes,
        )
        .unwrap();
        let body = tx.raw_body();
        let hash = blake2b_256(&body);
    }

    #[test]
    fn test_sign_data() {
        let entropy = hex::decode("7a4362fd9792e60d97ee258f43fd21af").unwrap();
        let passphrase = b"";
        let path = "m/1852'/1815'/0'/0/0".to_string();
        let payload = "846a5369676e6174757265315882a301270458390069fa1bd9338574702283d8fb71f8cce1831c3ea4854563f5e4043aea33a4f1f468454744b2ff3644b2ab79d48e76a3187f902fe8a1bcfaad676164647265737358390069fa1bd9338574702283d8fb71f8cce1831c3ea4854563f5e4043aea33a4f1f468454744b2ff3644b2ab79d48e76a3187f902fe8a1bcfaad4043abc123";
        let sign_data_result = sign_data(&path, payload, &entropy, passphrase).unwrap();
        assert_eq!(hex::encode(sign_data_result.get_signature()), "451d320df8d5a944c469932943332e02ed6721fe9e1f93dde08bb45e48e48ed7f6d0463ff8c2f65ab626bdefcf1b0825bde2ef64b5ccd271554bf98e03d6ea07");
        // 2ae9d64b6a954febcc848afaa6ca1e9c49559e23fe68d085631ea2a020b695ff
        // 2ae9d64b6a954febcc848afaa6ca1e9c49559e23fe68d085631ea2a020b695ffed535d78ef7d225ba596dbbf3c2aea38b6807f793d8edd9671a4c2de5cdb5ba8
        assert_eq!(hex::encode(sign_data_result.get_pub_key()), "2ae9d64b6a954febcc848afaa6ca1e9c49559e23fe68d085631ea2a020b695ff");
    }
}
