use super::*;
struct SeedSigner {
    seed: [u8; 64],
}

impl PcztSigner for SeedSigner {
    type Error = ZcashError;
    fn sign_transparent(
        &self,
        hash: &[u8],
        key_path: BTreeMap<Vec<u8>, Zip32Derivation>,
    ) -> Result<BTreeMap<Vec<u8>, ZcashSignature>, Self::Error> {
        let message = Message::from_digest_slice(hash).unwrap();
        let fingerprint = calculate_seed_fingerprint(&self.seed)
            .map_err(|e| ZcashError::SigningError(e.to_string()))?;

        let mut result = BTreeMap::new();
        key_path.iter().try_for_each(|(pubkey, path)| {
            let path_fingerprint = path.seed_fingerprint.clone();
            if fingerprint == path_fingerprint {
                let my_pubkey = get_public_key_by_seed(&self.seed, &path.to_string())
                    .map_err(|e| ZcashError::SigningError(e.to_string()))?;
                if my_pubkey.serialize().to_vec().eq(pubkey) {
                    let signature = sign_message_by_seed(&self.seed, &path.to_string(), &message)
                        .map(|(rec_id, signature)| signature)
                        .map_err(|e| ZcashError::SigningError(e.to_string()))?;
                    result.insert(pubkey.clone(), signature);
                }
            }
            Ok(())
        })?;

        Ok(result)
    }

    fn sign_sapling(
        &self,
        hash: &[u8],
        alpha: [u8; 32],
        path: Zip32Derivation,
    ) -> Result<Option<ZcashSignature>, Self::Error> {
        // we don't support sapling yet
        Err(ZcashError::SigningError(
            "sapling not supported".to_string(),
        ))
    }

    fn sign_orchard(
        &self,
        hash: &[u8],
        alpha: [u8; 32],
        path: Zip32Derivation,
    ) -> Result<Option<ZcashSignature>, Self::Error> {
        let fingerprint = calculate_seed_fingerprint(&self.seed)
            .map_err(|e| ZcashError::SigningError(e.to_string()))?;

        let path_fingerprint = path.seed_fingerprint.clone();
        if fingerprint == path_fingerprint {
            sign_message_orchard(&self.seed, alpha, hash, &path.to_string())
                .map(|signature| Some(signature))
                .map_err(|e| ZcashError::SigningError(e.to_string()))
        } else {
            Ok(None)
        }
    }
}

#[cfg(test)]
mod tests {
    use alloc::{collections::btree_map::BTreeMap, vec};
    use zcash_vendor::pczt::{
        common::{Global, Zip32Derivation},
        orchard::{self, Action},
        sapling, transparent, Pczt, Version, V5_TX_VERSION, V5_VERSION_GROUP_ID,
    };

    use super::*;

    extern crate std;

    const HARDENED_MASK: u32 = 0x8000_0000;

    use std::println;

    #[test]
    fn test_pczt_sign() {
        let seed = hex::decode("00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000").unwrap();

        let fingerprint =
            hex::decode("a833c2361e2d72d8fef1ec19071a6433b5f3c0b8aafb82ce2930b2349ad985c5")
                .unwrap();

        let signer = SeedSigner {
            seed: seed.try_into().unwrap(),
        };

        let pczt = Pczt {
            version: Version::V0,
            transparent: transparent::Bundle {
                inputs: vec![],
                outputs: vec![],
            },
            sapling: sapling::Bundle {
                anchor: [0; 32],
                spends: vec![],
                outputs: vec![],
                value_balance: 0,
                bsk: None,
            },
            orchard: orchard::Bundle {
                anchor: hex::decode("a6c1ad5befd98da596ebe78491d76f76402f3400bf921f73a3b176bd70ab5000").unwrap().try_into().unwrap(),
                actions: vec![
                    Action {
                        cv: hex::decode("4ac2480c13624d2b8aabf82ee808b4e4965d6c26efd9cfc9070f69e1a9a69609").unwrap().try_into().unwrap(),
                        spend: orchard::Spend {
                            value: None,
                            witness: None,
                            alpha: Some(hex::decode("105dd4f80b149ee6a8d5f11b2d0f7d0caa7cece6d0dce3ce494ce14977def354").unwrap().try_into().unwrap()),
                            fvk: None,
                            proprietary: BTreeMap::new(),
                            recipient: None,
                            rho: None,
                            rseed: None,
                            nullifier: hex::decode("ef870733c09572b274782e32e28809c201a90c1e179ad78e88eb1477c7bd9631").unwrap().try_into().unwrap(),
                            rk: hex::decode("7fe9364e043a92f893100dc09fc70f1a4faad022687767f8c3495a83a57e6726").unwrap().try_into().unwrap(),
                            spend_auth_sig: None,
                            zip32_derivation: Some(Zip32Derivation {
                                seed_fingerprint: fingerprint.clone().try_into().unwrap(),
                                derivation_path: vec![HARDENED_MASK + 44, HARDENED_MASK + 133, HARDENED_MASK + 0],
                            }),
                        },
                        output: orchard::Output {
                            cmx: hex::decode("dbc7cc05319a4c70a6792ec195e99f1f3028194338953ee28f2f9426e06e1039").unwrap().try_into().unwrap(),
                            ephemeral_key: hex::decode("c7a4a801f5a0cf4380263eed1acc4952ecc61805a6bc4c17ce0fe783aa8f582e").unwrap().try_into().unwrap(),
                            enc_ciphertext: hex::decode("212c8799c5cc99400f3c1685820fd7d6b86a155f43b35d803aab357ef65f95430c922192e3a5c9d0e23d34a39d4257d2361c6f7ffbe386e4573ace688854f1c45ff08644d1ec4de6ad877104f9172cffbbcaf97993eac6a54b426c492697dcfe15461a661a0dd770696f1e6a59b3d280034b38f96cecfb8bb8c3ee642640887e021cc06406c1dc94a1d0c1e71bdb864bd97e1ca6beb25bcdb5bb756ca209da24aea0cfe45f65e159ad395e78133bd56c227da05778df4368fbb5247bb33cddafc7fefd67a8cb26d7b8841896f3e7ca57e218273335851e980ee470a7995e7ff179eb4a566ca8a7aca67cee124b8d8fd32072804d288f9db115edabb90b2cb3121dcb6069f8cb0809e1d53e1b71182f6a903436fe6685706d0e089e2cef276b27e7cd0a32230e1da7f5ded3edd136dc263f4913b1fd519eefd7f4a23dbfa8c530807e2c352b1b2e4d69cce2ffc506e85bba1dbf0daf212bc5ec204964aa26329071ae19cfc2614b7e6f2b5f0b831b1dafaa91e1b2e0e46f7d6b7e6870209cf10fc13908b88d079802f3e2fa2a62a3a88dd7fba600655948ff716ee6e7ee76f2deeb32a2cac8726168dcedad7584c9b42a4938b617b605f3e7acde18f8f5b5495ecd5ccbb7c9d86888b8f6a236cb79eced16eb41dbe884382d78dd26768d7110843aa3e3804c0757768458d4556f69e8887d1cbbd3f3ab0c9eb0b66319052a6089a94fb769ecf80930ae87cf04d282eb4fcaa24e5959c3b535ff99ba2ecb0f71931035f37f9875c944bdbd741adb95d5fed3ddbb78585c21f58c3d74a6cc18418e8537e1b8").unwrap().try_into().unwrap(),
                            out_ciphertext: hex::decode("6295187eb1d8dc74a065d46ae2bc235a47e5914b4320419e1312157ca16f153269e44278ad6f999a3899dfa6d004ce685cd7759a33112b26e5359dc7fe7ec3d81429854b4bbf767857120d14019353e5").unwrap().try_into().unwrap(),
                            ock: None,
                            proprietary: BTreeMap::new(),
                            recipient: None,
                            rseed: None,
                            shared_secret: None,
                            value: None,
                            zip32_derivation: Some(Zip32Derivation {
                                seed_fingerprint: fingerprint.clone().try_into().unwrap(),
                                derivation_path: vec![HARDENED_MASK + 44, HARDENED_MASK + 133, HARDENED_MASK + 0],
                            }),
                        },
                        rcv: None,
                    },
                    Action {
                        cv: hex::decode("6c78ee94ced314a28898218fb3d9594ff97b96d7d92c71f9e1866731eddd3ca8").unwrap().try_into().unwrap(),
                        spend: orchard::Spend {
                            value: None,
                            witness: None,
                            alpha: Some(hex::decode("15716c9c0c9af80201b63a1b0a329fd9579824cfae4cd9f086848d729dd37cff").unwrap().try_into().unwrap()),
                            fvk: None,
                            proprietary: BTreeMap::new(),
                            recipient: None,
                            rho: None,
                            rseed: None,
                            nullifier: hex::decode("0e65a80237a3d3e1dcede4fe7632eec67254e0e1af721cd20fa8b9800263f508").unwrap().try_into().unwrap(),
                            rk: hex::decode("afa2899a1fc1f5d16639e162979b29bedbf84aeb0987a2d8143d10134a47f722").unwrap().try_into().unwrap(),
                            spend_auth_sig: None,
                            zip32_derivation: Some(Zip32Derivation {
                                seed_fingerprint: fingerprint.clone().try_into().unwrap(),
                                derivation_path: vec![HARDENED_MASK + 44, HARDENED_MASK + 133, HARDENED_MASK + 0],
                            }),
                        },
                        output: orchard::Output {
                            cmx: hex::decode("c7e391d7deb77891735e12be5f63e8821a79636a774578706bf495bef678072b").unwrap().try_into().unwrap(),
                            ephemeral_key: hex::decode("b306bb760dc8b8018db27ee58072969d1665b98095b41034615d4ff62410800f").unwrap().try_into().unwrap(),
                            enc_ciphertext: hex::decode("572b855e2c2b872eb6d9e375831a34b486833a015d0d42876c8b8b47409aa67d238903a931539466b12645d0e7f18ad6637fc81152b145585511245a48b9e4a20069dcf3d10aef699388f6a1855567c5312d66a94724db45c10ae0bc4a6af7fa508b4184859a1bfc38dbed7258b39406a64af9a401ab9d921f74fd8fb2f44893458d9a0fd67c773a8d65ecffe2f0868755b8e359ab3b7bf6ebce2553745b96d31bdcea662188691f9fc12fd652b8528e6339924c66f12e39e4b1b3041fde91cef49b7c9f4c0201e22f712ecc599219acdc4d5c77b795ebe3c80a701e22780274c2f88298fa40af2bcba9b78b258e80bebf5bf962c82e020e7444aafa3c857f8fefe5c2c79627873e334af336defc71c772c472840228cd6a7a870ff16efc2204d232b1f4da4b17ca5c9dee367c7aecd0f1deb9ec65f6c03f26ec95c6e9f03f5da0419260be47703ac2a56467883a272858625cb64bd3c0e388a15197665493377984c78aee751bab65971ea0b511879b6339856d724780250843a34af9462c765ac5200b22b6a35341c73ef4da9fc82087f3fa9dfb6ce6434a1e60b6c15beedfa3a8ca2feeeb249fb73154d541c4ced12936fe5ab6b0ac989eee5d045a36659d31d7352f77db6c32b8a827a456ce93bcd8f69e9b3b17ba2f44016107a886392af6c413e54d3707008573d2b393693616dae726e2e1ae52e437a0a5bab14ffe5ea26df3be381770fa9fce263a0adfbf4bf5182826c573da06d83011e85dbb16866099de5dc79465f46b29552565eede84f36ae0b443ea05e46a97362be8796bb635549108").unwrap().try_into().unwrap(),
                            out_ciphertext: hex::decode("f60df073061724815f4ae663a99a6781fc5ca797390541172c5cf8b4fece3d45a07d97636853bdaec1758fa8ba339b935462ff4bc23ced395990a6551fcee705d092bcd33a0a68c41f2cd15d59128060").unwrap().try_into().unwrap(),
                            ock: None,
                            proprietary: BTreeMap::new(),
                            recipient: None,
                            rseed: None,
                            shared_secret: None,
                            value: None,
                            zip32_derivation: Some(Zip32Derivation {
                                seed_fingerprint: fingerprint.clone().try_into().unwrap(),
                                derivation_path: vec![HARDENED_MASK + 44, HARDENED_MASK + 133, HARDENED_MASK + 0],
                            }),
                        },
                        rcv: None,
                    }
                ],
                flags: 3,
                value_balance: 10000,
                zkproof: None,
                bsk: None,
            },
            global: Global {
                tx_version: V5_TX_VERSION,
                version_group_id: V5_VERSION_GROUP_ID,
                consensus_branch_id: 0xc2d6_d0b4,
                lock_time: 0,
                expiry_height: 2705733,
                proprietary: BTreeMap::new(),
            },
        };

        let signed = pczt.sign(&signer).unwrap();

        assert_eq!("274d411da4e2cdeab282ac5b61b6b2acb0d6edfe9b9fc6282c200ed621a581a1234b44710fedee313667cc315d896ec69bb0e233b9897bf3fea2820f84757419", hex::encode(signed.orchard.actions[0].spend.spend_auth_sig.unwrap()));
        assert_eq!("cc49f7b09c5d2bb2ed55390da728e7a37639d461b040183a8ac87020d8236f1db920b3b162c41bfeba278c170702716e81db09320e4ce69fda4095c53091052f", hex::encode(signed.orchard.actions[1].spend.spend_auth_sig.unwrap()));
    }
}
