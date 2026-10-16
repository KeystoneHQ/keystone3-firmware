use crate::address::public_key_to_address;
use crate::errors::{Result, TronError};
use crate::transaction::wrapped_tron::WrappedTron;
use alloc::borrow::ToOwned;
use alloc::string::String;
use alloc::string::ToString;
use alloc::vec;
use keystore::algorithms::secp256k1;

use hex;
use prost::Message;

pub trait Signer {
    fn sign(&self, seed: &[u8]) -> Result<(String, String)>;
    fn sign_signature_only(&self, seed: &[u8]) -> Result<String>;
}

impl WrappedTron {
    fn validate_seed_signer(&self, seed: &[u8]) -> Result<()> {
        let fingerprint = secp256k1::get_master_fingerprint_by_seed(seed)?;
        if !self.xfp.is_empty()
            && self.xfp.to_uppercase() != hex::encode(fingerprint).to_uppercase()
        {
            return Err(TronError::InvalidParseContext(
                "request fingerprint does not match signing seed".to_string(),
            ));
        }
        let public_key = secp256k1::get_public_key_by_seed(seed, &self.hd_path)?;
        if public_key_to_address(&public_key) != self.from {
            return Err(TronError::NoMyInputs);
        }
        Ok(())
    }

    fn do_sign_digest(&self, seed: &[u8]) -> Result<[u8; 65]> {
        self.validate_seed_signer(seed)?;
        let sig_hash = self.signature_hash()?;

        let message = bitcoin::secp256k1::Message::from_digest_slice(sig_hash.as_slice())
            .map_err(|e| TronError::SignFailure(e.to_string()))?;

        let (rec_id, signature) = secp256k1::sign_message_by_seed(seed, &self.hd_path, &message)?;

        let mut sig_bytes = [0u8; 65];
        sig_bytes[..64].copy_from_slice(&signature);
        sig_bytes[64..].copy_from_slice(&rec_id.to_le_bytes()[..1]);

        Ok(sig_bytes)
    }
}

impl Signer for WrappedTron {
    fn sign(&self, seed: &[u8]) -> Result<(String, String)> {
        let sig_hash = self.signature_hash()?;
        let sig_bytes = self.do_sign_digest(seed)?;

        let mut tx = self.tron_tx.to_owned();
        let count: usize = tx
            .raw_data
            .as_ref()
            .map_or(0, |raw_data| raw_data.contract.len());

        tx.signature = vec![sig_bytes.to_vec(); count];

        let tx_hex = tx.encode_to_vec();
        Ok((hex::encode(tx_hex), hex::encode(sig_hash)))
    }

    fn sign_signature_only(&self, seed: &[u8]) -> Result<String> {
        let sig_bytes = self.do_sign_digest(seed)?;
        Ok(hex::encode(sig_bytes))
    }
}

#[cfg(test)]
mod tests {
    use alloc::string::ToString;
    use hex;

    use crate::test::{prepare_parse_context, prepare_payload};
    use crate::transaction::signer::Signer;
    use crate::transaction::wrapped_tron::WrappedTron;

    fn legacy_payload(token: &str, memo: &str) -> ur_registry::pb::protoc::Payload {
        use ur_registry::pb::protoc::{self, payload, sign_transaction};
        protoc::Payload {
            r#type: payload::Type::SignTx as i32,
            xfp: "73c5da0a".into(),
            content: Some(payload::Content::SignTx(protoc::SignTransaction {
                coin_code: "TRON".into(),
                hd_path: "m/44'/195'/0'/0/0".into(),
                transaction: Some(sign_transaction::Transaction::TronTx(protoc::TronTx {
                    token: token.into(),
                    contract_address: if token == "USDT" {
                        "TR7NHqjeKQxGTCi8q8ZY4pL8otSzgjLj6t".into()
                    } else {
                        "".into()
                    },
                    from: "TUEZSdKsoDHQMeZwihtdoBiN46zxhGWYdH".into(),
                    to: "TSeJkUh4Qv67VNFwY8LaAxERygNdy6NQZK".into(),
                    memo: memo.into(),
                    value: "1".into(),
                    latest_block: Some(protoc::LatestBlock {
                        hash: "02".repeat(32),
                        number: 1,
                        timestamp: 1000,
                    }),
                    ..Default::default()
                })),
                ..Default::default()
            })),
        }
    }

    #[test]
    fn legacy_trx_trc10_trc20_signatures_match_txid_and_owner() {
        use bitcoin::secp256k1::{
            ecdsa::{RecoverableSignature, RecoveryId},
            Message, Secp256k1,
        };
        use cryptoxide::hashing::sha256;
        use prost::Message as ProstMessage;
        let xpub = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        for (token, expected_txid) in [
            (
                "TRX",
                "a76505a6accf647950420f1440ead53266b9b97adcd146719abfd4173d856120",
            ),
            (
                "1002000",
                "85bebc501d6c89b5895728bf54c6bea87d74c7c8f703ad8f2a67beabbffb4ddb",
            ),
            (
                "USDT",
                "040ddfb423a3e80dbab27562bcc41da547ec86c235aa25a94d0e54fd0bb722bf",
            ),
        ] {
            let payload = legacy_payload(token, "");
            crate::check_raw_tx(payload.clone(), prepare_parse_context(xpub)).unwrap();
            let parsed = crate::parse_raw_tx(payload.clone(), prepare_parse_context(xpub)).unwrap();
            let (signed, txid) =
                crate::sign_raw_tx(payload, prepare_parse_context(xpub), &seed).unwrap();
            assert_eq!(txid, expected_txid, "{token}");
            let bytes = hex::decode(signed).unwrap();
            let tx = crate::pb::protocol::Transaction::decode(bytes.as_slice()).unwrap();
            let raw = tx.raw_data.unwrap().encode_to_vec();
            assert_eq!(hex::encode(sha256(&raw)), expected_txid);
            assert_eq!(parsed.detail.raw_data_hash, expected_txid);
            assert_eq!(tx.signature.len(), 1);
            let sig = &tx.signature[0];
            let recovered = Secp256k1::new()
                .recover_ecdsa(
                    &Message::from_digest_slice(&sha256(&raw)).unwrap(),
                    &RecoverableSignature::from_compact(
                        &sig[..64],
                        RecoveryId::from_i32(sig[64] as i32).unwrap(),
                    )
                    .unwrap(),
                )
                .unwrap();
            assert_eq!(
                crate::address::public_key_to_address(&recovered),
                parsed.overview.from
            );
            assert_eq!(parsed.overview.from, "TUEZSdKsoDHQMeZwihtdoBiN46zxhGWYdH");
            assert_eq!(
                hex::encode(sig),
                crate::sign_tx_request(&raw, &"m/44'/195'/0'/0/0".into(), &seed).unwrap()
            );
        }
    }

    #[test]
    fn legacy_large_memo_obeys_total_request_limit() {
        let xpub = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        for token in ["TRX", "1002000", "USDT"] {
            assert!(crate::parse_raw_tx(
                legacy_payload(token, &"a".repeat(2048)),
                prepare_parse_context(xpub)
            )
            .is_ok());
            let payload = legacy_payload(token, &"a".repeat(crate::MAX_SIGN_DATA_LEN));
            assert!(
                crate::parse_raw_tx(payload.clone(), prepare_parse_context(xpub))
                    .unwrap_err()
                    .to_string()
                    .contains("request exceeds")
            );
            assert!(crate::check_raw_tx(payload.clone(), prepare_parse_context(xpub)).is_err());
            assert!(crate::sign_raw_tx(payload, prepare_parse_context(xpub), &[0; 64]).is_err());
        }
    }

    #[test]
    fn test_sign_signature_only_trc20_rejects_wrong_fingerprint() {
        let hex = "1f8b08000000000000031590bf4ac3501c46359452bba871299d4a102a42c8bffbbbf7c6499b1403b6b14d52b42e92e426b5c53636462979029d7d01477707279f40147c0007df41707130856f3870a6f355387ebd9f1a098b1abd34c99230b9acbf70158eaf1099b4db26368427ae5af29c639bdf0e98a652d50fc4500922110121a21efb548c028010142d8814bdbed995106a4a8a0e4d492e26c98defb78ffb3f79a7dcfa5ae505cf21b6359f4447fdc5a1678ce99c9e0dd1558726999b8f269d09ceea82e7b96408dab58bd23c358deccc1fdf38f97cc114ec6746a40e1c41f05cc87b89814edbada9756bda07b3d9893ab2b46eff22746c3c76a6bb2b6a49d129d9b3abfb3e8be3400335f4090d3506818c303042402f0c669851888160504286502c2b408b001d01f5fd40d6286c3c7f3ed46a773fef45486bab5a1ab8a6c7af2d6f395f62ad6c3dfee2c66bef1f257dc3fe50010000";
        let pubkey_str = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let mut tx = WrappedTron::from_payload(payload, &context).unwrap();
        tx.xfp = "73c5da0a".to_string();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        assert!(matches!(
            tx.sign_signature_only(&seed),
            Err(crate::errors::TronError::NoMyInputs)
        ));
    }

    #[test]
    fn test_sign_trx_transfer_rejects_wrong_owner() {
        // https://tronscan.org/#/transaction/0f89c6365796afa96ae05fab57207a4d8c5cc801b92ac2099c7dd9dcd5a91df0
        // txHex: 0a7c0a02ea0522082c84cb547bae782640b8f3c28bfc305a65080112610a2d747970652e676f6f676c65617069732e636f6d2f70726f746f636f6c2e5472616e73666572436f6e747261637412300a1541bf90ce9c87c42a58699b1a2e3f65e88181436d53121541b0ca665130d03c977823834e3fcbe63af2421fa1180112411634795a1461b2775648062bcee3675033158557e497f1187d0312ae8e4589d07a22cf3334f4b5c49133ada012ccf308baefb8062e24d22d62f8da0a40716da301
        // txId: 0f89c6365796afa96ae05fab57207a4d8c5cc801b92ac2099c7dd9dcd5a91df0
        let hex = "1f8b08000000000000030dcebb4ac3501c807153444a17b553e95482501142ce39f99f4b8b83362d74696cd38897eddca205db60ac22d95db49b4fe0e6e8e4e65bf80a6e3e80a083816ffb965fb552df9ce461666c6b9c67cb4c6757cd5fa75aa95739e283419f85eeb7535b4fe2a3a8bea3710a42748cc729621e60a43cc1c17801a2c03166444bd4dc1ef9006d1f7768db47653e6aad5e578f7f6877a3f7e234dd2499f2331adc4c212ff27eb83c97b3dbb8b8be3798c96568c971ecbac99416e32c84d3416f3ebceb47878bc509b91806a3c9a58e23368be77b6bc4c1dda8760046714cb148b154982949b400ad4a8f92960bc23ae5e20a510bda12c9d2006b0bd2d8148ca582045b0f9fcf8dc6cfd753291cbebdefb73ec6ff42b912d514010000";
        let pubkey_str = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let mut tx = WrappedTron::from_payload(payload, &context).unwrap();
        tx.xfp = "73c5da0a".to_string();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        assert!(matches!(
            tx.sign(&seed),
            Err(crate::errors::TronError::NoMyInputs)
        ));
    }

    #[test]
    fn test_sign_trc10_transfer_rejects_wrong_owner() {
        // txHex: 0a8b010a02dd71220897877879243d8e1840d8a6f586fc305a74080212700a32747970652e676f6f676c65617069732e636f6d2f70726f746f636f6c2e5472616e736665724173736574436f6e7472616374123a0a0731303032303030121541bf90ce9c87c42a58699b1a2e3f65e88181436d531a1541b0ca665130d03c977823834e3fcbe63af2421fa120e80712411c161e9365db1c8e361ba5ba1be8c13ca7dd01b68d9410f62597245fba9955d11cc5999622db1ecf455c84d51ab785dd1e560c078d7886cd85b4839e1d94ac2900
        // txId: ce840b7d819b37069268232b4ed4fd49d8ec38141f20301648703cfe4ea0f321
        let hex = "1f8b0800000000000003158f3d4bc350188535961ab26833954e21081521e4bd37f723d7499316ba34b669c48f45927b532dd8066315e9eee4e8a24efe0e41f1e7b83ab92898c2339ce53c9ca36be6c6b00c0b955b83b29817b2b86cbd68ba66ea1c78b7db61a1fda019b5243e88ccad2c932af541390c98ef108eb9937a923852082214521268d66af45d42da2e12b4ed42850bd6dfd7c7fd2f6cd783cf55631d01600068d94932e227d4bb1e91725176c2f9693ab9891757770ab1741ee6f830b6ed644417832224c7dd60dabbed44fbb3d9113eef79fde1858c233689a73b2bb85629613732f628833c0720922a210021c17d5e2130f1949f235f88713e56482a9a519a12ce709a7a385355dda33e929bdf6f8fcde6d3f3726dd030d6822431f5e5f5b32a35ebbdd71f66bd0ffe01545edbc933010000";
        let pubkey_str = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let mut tx = WrappedTron::from_payload(payload, &context).unwrap();
        tx.xfp = "73c5da0a".to_string();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        assert!(matches!(
            tx.sign(&seed),
            Err(crate::errors::TronError::NoMyInputs)
        ));
    }
    #[test]
    fn test_sign_trc20_transfer_rejects_wrong_fingerprint() {
        {
            // txHex: 0ad4010a02e8b32208b0465d7759adf6d640c881858bfc305aae01081f12a9010a31747970652e676f6f676c65617069732e636f6d2f70726f746f636f6c2e54726967676572536d617274436f6e747261637412740a1541bf90ce9c87c42a58699b1a2e3f65e88181436d53121541a614f803b6fd780986a42c78ec9c7f77e6ded13c2244a9059cbb000000000000000000000000b0ca665130d03c977823834e3fcbe63af2421fa100000000000000000000000000000000000000000000000000000000000007c37088f4cd89fc3090018094ebdc0312413da7ea834760bb30c578c9d58aae22f04c172345e3dc5fa20ebe135e6f4e43776fc74151d2f8a35a93c9e644e6f79b2e4facb8d4803f5ad51455da35e69f60b101
            // txId: 762dee17f934b7520a8c83396a823690921355d5160d6848728834fd15dea5c4
            let hex = "1f8b08000000000000031590bf4ac3501c46359452bba871299d4a102a42c8bffbbbf7c6499b1403b6b14d52b42e92e426b5c53636462979029d7d01477707279f40147c0007df41707130856f3870a6f355387ebd9f1a098b1abd34c99230b9acbf70158eaf1099b4db26368427ae5af29c639bdf0e98a652d50fc4500922110121a21efb548c028010142d8814bdbed995106a4a8a0e4d492e26c98defb78ffb3f79a7dcfa5ae505cf21b6359f4447fdc5a1678ce99c9e0dd1558726999b8f269d09ceea82e7b96408dab58bd23c358deccc1fdf38f97cc114ec6746a40e1c41f05cc87b89814edbada9756bda07b3d9893ab2b46eff22746c3c76a6bb2b6a49d129d9b3abfb3e8be3400335f4090d3506818c303042402f0c669851888160504286502c2b408b001d01f5fd40d6286c3c7f3ed46a773fef45486bab5a1ab8a6c7af2d6f395f62ad6c3dfee2c66bef1f257dc3fe50010000";
            let pubkey_str = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let tx = WrappedTron::from_payload(payload, &context).unwrap();
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            assert!(matches!(
                tx.sign(&seed),
                Err(crate::errors::TronError::InvalidParseContext(_))
            ));
        }
    }
}
