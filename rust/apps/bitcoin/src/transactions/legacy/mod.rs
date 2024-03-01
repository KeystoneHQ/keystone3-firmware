mod constants;
pub mod input;
pub mod output;
pub mod parser;
pub mod tx_data;

use crate::errors::{BitcoinError, Result};
pub use crate::transactions::legacy::tx_data::TxData;
use alloc::string::ToString;
use alloc::vec::Vec;
use keystore::algorithms::secp256k1;
use third_party::bitcoin::consensus::serialize;
use third_party::either;
use third_party::secp256k1::Message;

pub fn sign_legacy_tx(tx_data: &mut TxData, seed: &[u8]) -> Result<Vec<u8>> {
    let input_len = tx_data.inputs.len();
    for index in 0..input_len {
        let raw_input = &tx_data.inputs[index].clone();
        let sig_hash = tx_data.signature_hash(index.clone())?;
        let message = match sig_hash {
            either::Left(s) => Message::from_digest_slice(s.as_ref()).map_err(|_e| {
                BitcoinError::SignFailure(format!(
                    "invalid sig hash for input #{}",
                    (index as u8).to_string()
                ))
            })?,
            either::Right(r) => Message::from_digest_slice(r.as_ref()).map_err(|_e| {
                BitcoinError::SignFailure(format!(
                    "invalid sig hash for input #{}",
                    (index as u8).to_string()
                ))
            })?,
        };
        let (_, signature) =
            &secp256k1::sign_message_by_seed(seed, &raw_input.hd_path.to_string(), &message)?;
        tx_data.add_signature(index.clone(), signature)?;
    }
    Ok(serialize(&tx_data.transaction))
}

#[cfg(test)]
mod tests {
    use crate::test::{prepare_parse_context, prepare_payload};
    use crate::{check_raw_tx, sign_raw_tx};
    use app_utils::keystone;
    use core::str::FromStr;
    use third_party::hex;

    #[test]
    fn test_sign_ltc_p2sh_transaction() {
        // tube
        // ypub: ypub6X1mUc1jWSVhJJvVdafzD2SNG88rEsGWwbboBrrmWnMJ4HQwgvKrTkW2L7bQcLs1Pi1enPCXica1fnDryixfCptU1cQCYxVuSMw6woSKr47
        // rawTx: 020000000001011f233765baac332c470de0c00c4c023e375f8c655f9125c1c97768c58c87a55400000000171600142109dc769658b3fed046a63ae69bd7cafe0c3cc6fdffffff011c2500000000000017a914ca41c357b622445d8b6c301201b2eebbb27c6c72870247304402205fd021ae89f96dd541c9fde42a20b410e30e134a98d4a9ee603efcfc289157a402206ad71047a36a95ded190f19329c4027ca05f1180f1183f86123c6340b6d77251012103f66d0987c6d99091236e7362e3b57a6bd37c47176dba3f56397cf8c1791299c600000000
        let hex = "1f8b08000000000000030d8fbb4a0341144049b458b64948155285458808cbcee3de79749228421e6250b19ebd33830bc2c09242eded2d2c53f903f67e44bec12fb0b773e15487d39cac3f1a6cdb45f2617ad3a65da2f434f9e87736d392d03be68ab77e7eb4be5b8c4e882ca83a88d204a012441d4a27052f513994ce5044aea65fbf3fdf7fec34ab0fbdec301e7e1e17fb5e7e8ee0d0684384ca684d96b840cb312a3484516a19980062442c30e6410b92d251ed144a2d248f93ab7cce449738a594359e111885ce686e9cf41249591539b0009e230988e02d700c68adafa5d7dd499476fc7e5d0c3615d859256615eba8c4d92c2f36b7ebfae1de37172b4c293d36bbe5563c07b55dbc3497afabd4ce61b85ffe0373cb71792a010000";
        let pubkey_str = "Mtub2rz9F1pkisRsSZX8sa4Ajon9GhPP6JymLgpuHqbYdU5JKFLBF7Qy8b1tZ3dccj2fefrAxfrPdVkpCxuWn3g72UctH2bvJRkp6iFmp8aLeRZ";
        {
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let signed_tx = sign_raw_tx(payload, context, &seed).unwrap();
            assert_eq!("020000000001011f233765baac332c470de0c00c4c023e375f8c655f9125c1c97768c58c87a554000000001716001477f90df00aabd793f093c1d510c6bd7480777c7cfdffffff011c2500000000000017a914ca41c357b622445d8b6c301201b2eebbb27c6c72870247304402207cadeb7fbf2c550d6fbd8cb6424e546e3c867e743197d0b752976a19928c95af022060adec681af0ca41277b9da1705418e62570c7f3169444e83d2ed0aff03d8b8a01210224ca66698d0c4865a8718a3d35c696f140e4d15c24f4d9415e599db3d75daf3900000000", signed_tx.0);
        }
        {
            // check without change address
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let check = check_raw_tx(payload, context).unwrap();
            assert_eq!((), check);
        }
        {
            // check with change address
            let hex = "1f8b0800000000000003558fbd4a0341144649b458b64948155285458808cbcecfbd7766ac2491086a82011bed66efcc54e24262d0c7b0b448656369ef43f80cfa02f6766e2b7cd5c739c5c9ba83de6a3d6b421c5fad9b87869bbbd15bb77d33a3198317be78e9e67b97d7b3c101b303aaa32a6d042e41d5b1f45ac912c9a3f696134a1abfff7c7dfc8ac3acfeee649fc3feeb7eb1ebe427081eadb1cc48d618762c153a8989d032266d74140a58308b284400a3586bcfb527d44669994667f954a816f144e46c100c96d05b23add7412393a32441440812594182e0406244e742ad83694b9276c3e765d15b54e026959a54a25da58e8ef36271b170b4e1c7ed26f8a58590ead57a7efad4f0cdfcbe996e196efbbbf361e7bf2b2bf107321aa6643d010000";
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let check = check_raw_tx(payload, context).unwrap();
            assert_eq!((), check);
        }
    }

    #[test]
    fn test_check_ltc_tx_to_p2pkh() {
        let hex = "1F8B0800000000000003AD8E4F4B5B4114C549134AFA3606576956E151508490993BFFEED04D8D508A49D3D736A56E67EECC104C9A07468AD9765FE807B0B4043F86EEFD02820B3F823BB72E7CAEDDCABD8BC3E11CCEAFF96273E3F3D15E1962B7382A8F4B2AE79D5F8DCA6D2A30521A26F2BB7A561F4DF636DF78D03C78523D9918EF4929630F017C4F80F352586F199AEEC5EDE9FA8A6F37FD79BD797DD368AD1B9DECD3F0A0FBDDCDE7F138FF5BCBDE696B3469AD90A50441899892479094187A6E82061588280A6E2DFA68ADD32C78E38D714CA66045E7433660C08344421DC1842A405A008BCCD8C7B354E1B8E043E40A391AC65DD436393064AD52327060D03EBD7F9D6FFCE84BBBD587AD3EABBECFF27F155F95615C0B070E9935090D3A51D502598A3C0AAA1439D22013A177A81C4A880C5C48C0B991BE557B26C2B33FAF9E10EE6C67F968BA1A8DD360560E97D389F9820545FD7336B013A0024A94D393D6FFDF2F77DE66F9C7E1C1217C5DEE17E3E2FD62B27B323D1C2DC669B65C7E0B6E77B05A2DF65B97EB76BBF664E6011171304D18020000";
        let extended_pubkey_str = "xpub6BnPH5L8uvcY5gq8GXmN4FXcfbKYq9yijf9SDS8YQEi3q7Ljph3PvSQevxgjP1nUheouxUKKoK7dLTDdxdXnDLujwBXdwdZkpsT1RrhShWY";
        let master_fingerprint =
            third_party::bitcoin::bip32::Fingerprint::from_str("52744703").unwrap();
        let extended_pubkey =
            third_party::bitcoin::bip32::Xpub::from_str(extended_pubkey_str).unwrap();
        let context = keystone::ParseContext::new(master_fingerprint, extended_pubkey);
        let payload = prepare_payload(hex);
        let check = check_raw_tx(payload, context.clone()).unwrap();
        assert_eq!((), check);
    }

    #[test]
    fn test_check_ltc_tx_to_p2wpkh() {
        let hex = "1F8B0800000000000003AD8ECD6A53511485890912EFA4A1A398510842A512B2CFFFD938B12D88B4B546ACD8E9397B9FA3B4E90D26B7C4E4154410C78A529CF820CE1C38B00FE07B74D0DB71A765EFC162B116EB6BDF595F7B39DB9972EA8F67D36A4AD349EF63AB76DB463AAD1DA8C165B368EE1FEEAC3F48CA196448C31888871A521C0697CD3060D41A8C489C4DFFEF97FFE717E2613BFE6EB6FF5DB43AE7AD5EF162EFA8FF264C26A91A7C6B144F2C3A4BD61A0F394B362AE51CBDD494C147E1D84AC344949440F43121060B1C5D742E80CE8CAAF7ACD80629587BF23649C77580AC9290C0E1F521698581232761BCF00E44481673908E108DD12C24C8EED7CBFB83B5D391C68D91DC1841FD23187CAFF9EA0C08AB820C1ED065EF7C50758D092989A4A85614C84A9DC9C7E04DF05A269081B314C2E9D869DC12E1CFCFF76E106E8AE2D1A422F1FEECD45BBD602C177382335C562754ADEC62355BD8B76EC525F82573EEFCF87477F3713178BE77742C5FCD77C707E3A7E5E1D68777C7FBE5413E99CF5F73D8DA5E2ECBDDCE9F5FDD6EE3C6DE1509C82F6221020000";
        let extended_pubkey_str = "xpub6BnPH5L8uvcY5gq8GXmN4FXcfbKYq9yijf9SDS8YQEi3q7Ljph3PvSQevxgjP1nUheouxUKKoK7dLTDdxdXnDLujwBXdwdZkpsT1RrhShWY";
        let master_fingerprint =
            third_party::bitcoin::bip32::Fingerprint::from_str("52744703").unwrap();
        let extended_pubkey =
            third_party::bitcoin::bip32::Xpub::from_str(extended_pubkey_str).unwrap();
        let context = keystone::ParseContext::new(master_fingerprint, extended_pubkey);
        let payload = prepare_payload(hex);
        let check = check_raw_tx(payload, context.clone()).unwrap();
        assert_eq!((), check);
    }

    #[test]
    fn test_sign_dash_p2pkh_transaction() {
        // enroll
        // xpub6DTnbXgbPo6mrRhgim9sg7Jp571onenuioxgfSDJEREH7wudyDQMDSoTdLQiYq3tbvZVkzcPe7nMgL7mbSixQQcShekfhKt3Wdx6dE8MHCk
        // rawTx: 0200000001047d86e714c787a08b7171c2e8dec21285858008189a6a204cf6cb6d12f0f655020000006b483045022100be6eb2819988d2ad9dfdf7688e829026809ca3367c37534d84241b9ef9797dcf022003378e253a71d2db267ee3ea34b1b5771e11cf74810f509fd7221e6d8628737901210252c47cf3f70b4e0b17cecd2f86c0c1d417f0a3397373fac5c0fa4d2f197ee553fdffffff01dc050000000000001976a9147700e233423b13e44f260ecc0547f48972a6429e88ac00000000
        let hex = "1f8b08000000000000030dccbb4a0341148061b2582c4991984aacc2224402cbcee5cccc99ce1b71090889124cec66cecc14625808e6157c033bdfc1de47b0b0b2b4f009acb573e1abfee2cfb3617fb13d6f421ccdb7cd6343cdc3e173d6d6dc4852c131573c65ddbd8bd39b7a78446441fb284a8c4025081f4b27052f95764a3aa4a4b81ebdfe7cbffdb1e3fceea393bfef0f3e27c54ba77ba254d28971a103f9a48104d3ce3a8e0c5b0a151724428c48c270c3d13b86860c8768509bc060901dfcf68a3326bcf03e28c7c0461e9242ebadf42aa5641342044f0e8c08e0a58ea0d1b7171fad262624210fda4dfa5715c0b852e38ab52a3919778bd566ad66cbe5ed6e7e498be5aa9e2948d37aea76d7ebfb4d5363580cbe7aff70e361822b010000";
        let pubkey_str = "xpub6CYEjsU6zPM3sADS2ubu2aZeGxCm3C5KabkCpo4rkNbXGAH9M7rRUJ4E5CKiyUddmRzrSCopPzisTBrXkfCD4o577XKM9mzyZtP1Xdbizyk";
        {
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let signed_tx = sign_raw_tx(payload, context, &seed).unwrap();
            assert_eq!("0200000001047d86e714c787a08b7171c2e8dec21285858008189a6a204cf6cb6d12f0f655020000006b483045022100d9694efa113a13c228bd07acc086db0cb10be6753987892ce893968d9bed342e02204d2a79f620a5371981ddc5acb6df849d580a16c5c361ff059b5f2dac3ee1a141012102b2bbd5a049e1df589b93b5fff9f84e4bca472d4b36e468b87cbe96c023c81d6afdffffff01dc050000000000001976a9147700e233423b13e44f260ecc0547f48972a6429e88ac00000000", signed_tx.0);
        }
        {
            // check
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let check = check_raw_tx(payload, context).unwrap();
            assert_eq!((), check);
        }
        {
            // check with change address
            let hex = "1f8b08000000000000034d8cbb4a03411440c962b12445d654c12a2c426461d979dc79596914492318022a763377660ca22e59a248fe44f003ececfd046b4b0bfd016beddc5238d58173d264d09f3507b50fa393a65ed5585f6f3d27ad4d1547e12db1f963d2dd38dc9f4f07db8806a40bacd401b004e6426939a3a5905670ab310a2a472fdf9fafbf6427bdf8eaa46f9bd97b913f75ba7b4244190965d2a38b129011698da59ae816a10565c87c081a99a28a6a678956a82804a5a5f204b264f8d3cb278439e69c17968009d447a18d33dc8918a3891a0238b4a09807c76500a95d7b71c148248ca3a65edaa27f5c018c2b31ae484bc58bdd6e7e1e26feae599f368be66875b98c66feb09e2def17d3da3bc16fafce20fbe80d3b79ffe65f4b2bf2076c698bec3e010000";
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let check = check_raw_tx(payload, context).unwrap();
            assert_eq!((), check);
        }
    }

    #[test]
    fn test_check_dash_tx_to_p2sh() {
        let hex = "1F8B08000000000000035D8DCD4A02510085199190D924AEC4950C812288F7CEFD87165909A145A342D9ECEE2F9A9ABF65B4EA05DCB496885AF61CF504B517A2F768D6C1591DCEF9BE4C2AB7DB9E1F4D8C2D46F3C972A227A3C24F2A6933246418338082AF949F3EAE774F727B9AC1D04AC5AAD85A57C50CD9AAA25A558D934028600DA1BAB8DDBEBF7DC37226DEA4328FEB74F6355DF0CF5BBDE2A51C8DEC3278F6FC03A56C020F9DC30652868950986B6529E5C23021B50C39D60A6A660161D21922A9050010138A1031A1B25EFE77BBF1824310F2E4853541924100B5E6C0526D8860C99C69212014D0191E1A4EACD19C20C51D94883B2210C552F3CAEEB88671A9464A3590A4062A653F603303FBB3F62C7E38EBC8669D98F96C386DC4A7182E5073B8ECDCC6D997F54E65DF0F7A2B79DD5F0D3A0B17A1C6D5859BDE0CA23BDCE975E328A2E356FB5E653F3E9FBCBC17FCF7FC01552A015579010000";
        let extended_pubkey_str = "xpub6D8VnprVAFFraSRDaj2afaKQJvX1gGMBS5h1iz3Q3RXkEKXK6G2YYmVoU7ke21DghCvpA8AVrpEg1rWw96Cm9dxWjobsFDctPLtQJvk7ohe";
        let master_fingerprint =
            third_party::bitcoin::bip32::Fingerprint::from_str("52744703").unwrap();
        let extended_pubkey =
            third_party::bitcoin::bip32::Xpub::from_str(extended_pubkey_str).unwrap();
        let context = keystone::ParseContext::new(master_fingerprint, extended_pubkey);
        let payload = prepare_payload(hex);
        let check = check_raw_tx(payload, context.clone()).unwrap();
        assert_eq!((), check);
    }

    #[test]
    fn test_sign_btc_p2pkh_transaction() {
        // tube
        // xpub6Ch68rD9nNm8AReQe9VwV6Mi67okew4oiYoNpKAESTvmjoDn5dBUQ9A4oqdxc9VpveH1cxWeH237HLFSmC37gRVwxnL4KvFYxCQqfprRinW
        // rawTx: 0200000002bffdff54dfa9b0e7e4b59484bae19497628b7bfc84098dee8d0ef2166684e010000000006a4730440220401c149a1a0db937afecf8626a736665038e6cf3211c0501a1d26ecf78c86a89022015c7713fdbfcc1408c621d2580e9bb6b34a1198560b365c778736b5b7eefe8f9012103ebeef16ce146b6a471790559667f48d74d3f6f4e34a2c3d087a88b3fc7a8b284fdffffffed192a0e8680142b04c9c2e0bc3341e0286bcd67098ac8f479d937952719290b000000006a47304402204170e47fe4679043ad975bbf15bb6fe557e5bda97d87fe66691dbc91dc6ab8b8022048502ace25f4ce0d9ea7eb8c9e346ea0270dc4848f2e248bd2e0984e72d842f90121021c5807453c5314ab14962548929250954e0f11251a0f380923a1f7f191a1e7f5fdffffff01ac0d0000000000001976a914ebbf2dd7547576c780e7e449936e26e4b63efdfb88ac00000000
        let hex = "1f8b0800000000000003558fbd6ad44114c53149b16cb3218d4b6cc21f21222c7b67e6ceccbd9d6659b7d190183fd046e6deb92382b231a6d03c8b95858d82ade4217c065f407b3b875809f73687c339bf33dad8991c9f2dd6d5f68eced6e76b5dbfdebddcecea28078db54019be6c8ec7078f162fee2f577717cf766eaa3226313f23439da1179b95e0dd2ca61243216dd1a5bd6fbf7f5efe815b23f9b531fa31ddfeb4357cbc36bee3c0803025979a07a36afd81099b6621499e33a33929848c120d2d0b14ae2d626bad4adb5d8d0fc0b33551f155d80c51c5f91cd8384430c7169b53c39842884e7206e0d4c010a8714000cd30fd7e7d98bc9923eecfe1dfcdf10a0fc4b3639f39865c3b4b43252a3d206b4de2c9005d08a206ea95a14f77484009cc97de5caff042ca167b6d1f0059158a65ebbeda7a0eb8ea3c51152c91424352160612c8684a6cc1624ad3cf5bc3e4c17f78e1f6fe7870874f9e9e9e2cde7fb8383d7a7962ab87eff0e2ed3d7df53c1fbac7cbe3f3a56e7fbdf11722ff2208d0010000";
        let pubkey_str = "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYJVVhhawA7d4R5WSWGFNbi8Aw6ZRc1brxMyWMzG3DSSSSoekkudhUd9yLb6qx39T9nMdj";
        {
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let signed_tx = sign_raw_tx(payload, context, &seed).unwrap();
            assert_eq!("0200000002bffdff54dfa9b0e7e4b59484bae19497628b7bfc84098dee8d0ef2166684e010000000006b483045022100f2a9dfd762cb29de410a631707c22049572d32a59f29dabd574c20096ff10e1202204002e792bd2c72339319aa08324282b9b5d85108a7974271bd1a55b92df354e10121029efbcb2db9ee44cb12739e9350e19e5f1ce4563351b770096f0e408f93400c70fdffffffed192a0e8680142b04c9c2e0bc3341e0286bcd67098ac8f479d937952719290b000000006a473044022008f1f05055218422c4cbd7745994fae8e6a1af3a1d211b61cf8680bb23537a7b022019c23a64b5cfc5b742c857478b06813be46ec5aac64c721ecda7d6e1539de4fb01210367e57004b507cc0ae7eb14df09601d1288db4a583f48c9b908b074ec89e3e566fdffffff01ac0d0000000000001976a914ebbf2dd7547576c780e7e449936e26e4b63efdfb88ac00000000", signed_tx.0);
        }
        {
            // check
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let signed_tx = sign_raw_tx(payload, context, &seed).unwrap();
            assert_eq!("0200000002bffdff54dfa9b0e7e4b59484bae19497628b7bfc84098dee8d0ef2166684e010000000006b483045022100f2a9dfd762cb29de410a631707c22049572d32a59f29dabd574c20096ff10e1202204002e792bd2c72339319aa08324282b9b5d85108a7974271bd1a55b92df354e10121029efbcb2db9ee44cb12739e9350e19e5f1ce4563351b770096f0e408f93400c70fdffffffed192a0e8680142b04c9c2e0bc3341e0286bcd67098ac8f479d937952719290b000000006a473044022008f1f05055218422c4cbd7745994fae8e6a1af3a1d211b61cf8680bb23537a7b022019c23a64b5cfc5b742c857478b06813be46ec5aac64c721ecda7d6e1539de4fb01210367e57004b507cc0ae7eb14df09601d1288db4a583f48c9b908b074ec89e3e566fdffffff01ac0d0000000000001976a914ebbf2dd7547576c780e7e449936e26e4b63efdfb88ac00000000", signed_tx.0);
        }
    }

    #[test]
    fn test_sign_btc_p2wpkh_transaction() {
        // tube
        // zpub6rQ4BxDEb1xJE2RJTyaoZT2NEr7FkGCgeEwkbgpBfPUTd6KmtBstbP9G81dPnJZJVAmbg2ZmfSc55FkrcHcKPvmNkLGmXAFqERtiUCn25LH
        // rawTx: 0200000000010264b4e500f14385a4143f081c64bb219599d17926fbbfb1278299fc932f334b680000000000fdfffffff7b0b5b8f2654e27c41cc71aa20538f15e1b16bba6cfa1f9ace2c97b817269bf0100000000fdffffff01708e010000000000160014595a2d41d7093e534bacddc8e3c09e293f7afc8302483045022100eaa6ec59384692e16b20552d0c65fb9e0a642e2bfd3d6a75719a59a3b9f34cf702205dc9612345563000e3ff3616d3872ec3e275a4ae2959564b09767c5911963a1d0121038b5f5a8e58954d78cf546d335c772c58a170a290a09039ab4f7b32058a36c5fe02473044022014d8eb17aca5a76b5dc7f5cb9e290c0dc8bd28fb5b08ac03a7e71883eae0cf200220719346bb534956ee6f73a51822c86d7a4e52879d2af2ea50038e9d2cafeca7100121038b5f5a8e58954d78cf546d335c772c58a170a290a09039ab4f7b32058a36c5fe00000000
        let hex = "1f8b0800000000000003ad8ebf4a9c4114c55193b06ce3c64ab6922590202c3b73e7def9d3258a189b9090252965ee9db92e6165935562bebc481a21e00bd85bf806fa0c965662a99d1f58da0aa7389ce2fc7e9dc595e52ff3cd59a96b9fe7b3c399cca6fd8ba576ed042754b2c983f3a5eeeb8df1e6eea70fe39d6f5bbb5fb7b6bfef8c57de8824f45c61182bca1081eb303bb043f2995c8ea264fddae9cdd5d9bd79d7e1fbc5cee56aefe4c5e078a1fbde4764e740935349290204b6accae0432a36a54460993d5a31d1a9c58c91b02dc65462f4d8dfee6e18e74c21d4624a4130d957d2589c075452871124574746db475213c8b112d7aa49a0249bd5c9ea5d1c2cef8f22be1d99c78ccce07fabc7ea538068034baa904553b6a2d9335b6f99aa5a170d65b05982154108583d297064e296d85be87f7c16c17fd72f9f18ae9bee3a8bfd7598ca5f7f7034f9d94cd39f1ff9f7fea4398a074d48cd7482d366deec65d86b7ab7c7af1e003f9faab8e3010000";
        let pubkey_str = "zpub6rFR7y4Q2AijBEqTUquhVz398htDFrtymD9xYYfG1m4wAcvPhXNfE3EfH1r1ADqtfSdVCToUG868RvUUkgDKf31mGDtKsAYz2oz2AGutZYs";
        {
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let signed_tx = sign_raw_tx(payload, context, &seed).unwrap();
            assert_eq!("0200000000010264b4e500f14385a4143f081c64bb219599d17926fbbfb1278299fc932f334b680000000000fdfffffff7b0b5b8f2654e27c41cc71aa20538f15e1b16bba6cfa1f9ace2c97b817269bf0100000000fdffffff01708e010000000000160014595a2d41d7093e534bacddc8e3c09e293f7afc830248304502210097c472d3daf800adb36f093c2c713948ec4b5c05e756eab42582e4d3d64a2bbb02205813811ef5753bc5e8080e4f22e5569a9623c7ca4399ba1278074acae2f3048f01210330d54fd0dd420a6e5f8d3624f5f3482cae350f79d5f0753bf5beef9c2d91af3c02483045022100d7eb7a2edae6caeedbcd130cfe100ce9643e686e3bfd0230753d088e14ccf15a02201e8533df39f6128fe6ddd89716b66486168b87eba43a1afade814fbedf6ea5b501210330d54fd0dd420a6e5f8d3624f5f3482cae350f79d5f0753bf5beef9c2d91af3c00000000", signed_tx.0);
        }
        {
            // check
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let check = check_raw_tx(payload, context).unwrap();
            assert_eq!(check, ());
        }
        {
            // check with change address
            let hex="1f8b0800000000000003ad8e3f6b935114c689510959123b854ce545500a21f79e7bcefdb36943a91d14c5a063b9e7dc7b108c491b0df912ee2e05c12fe0ee8710fc061d9dc4b1ddfa429742d7c2333c3cc3eff9f5eeed0cdeac67ab52775faf575f56b25a8ccfbbedda0b4ea864939b3fddfea3fdf9ecf8d5f3f9d1bb83e3b70787ef8fe63b8f45127aae3089156582c075921dd809f94c2e4751b27ef7e7bff35f97e6690fbe767bbf47c31ff79bb34eff998fc8ce8126a7925204086c5995c187546c4a89c0327bb462a2538b1923615b8ca9c4e8717cd8df37ce9942a8c5948260b2afa4b1380fa8a40e2348ae8e8cb6445213c8b112d7aa49a0249bd5c9e82236834fd3884fa6e63a53d37c6ff5587d0a106d604915b268ca56347b66eb2d5355eba2a10c364bb0220801ab27058e4cdc3e0e3be3177722f8edef835b867bb3fe1e8b3d8de2f5f3872d94c576b30cf5e3329d6ed505d9c07a1988362772e2eb62f8ffece1a8d30c5ede80d8a9b90290f88bd8f6010000";
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let check = check_raw_tx(payload, context).unwrap();
            assert_eq!(check, ());
        }
    }

    #[test]
    fn test_sign_btc_p2sh_p2wpkh_transaction() {
        // tube
        // rawTx: 02000000000102a80b91b95992983510e3ee1bc603f82684464969b85c3ecf424fe6b86c78576700000000171600141a105d3033973ff6703c955e6f1c51a4e79f82f6fdffffff6a0920b7e80c17930da1b22266b57beb36e1091208043ace6cdbddaed9fbe0430000000017160014b2fc5ef8c5ba5a18148ea094619d790f386f8bfefdffffff01dc0500000000000017a914e2d787a34f79160fa5d7b49ad667febe4bb329538702473044022011e54742454360a5e3834d4087515519ec891cc86625ff3245164a7b26f2d54d022014ea042d9d6d47fbdbccb4b5036255686ef9640a0fa09860d195fa676346d1f201210213d66c6876d8b0bd8c51941451a9530984f8aed64d1871899cd17603c91b9d7202483045022100de48221d2d1ca1ccabc628f24b67bb3935ce2a86f5613067e53401cc650b21ed02206ae860445ec259c8ef718b65b5b2119a439fde02db779a0d2d5b8de0fe064d40012103b659961ff574af3d1a1e74e6fb69578c5d4b91a45cbeaca1a0cf4759ed65d31700000000
        let hex = "1f8b08000000000000035dce3d6b9451100560cc0a2e6b91982aa40a8b10092c3b77e6cedcb99d660b4564d918112c67ee4721ea62582cfc2d96626b61e74fb0b6b4b04c93f4e9f292748169e6c0393ce3addded93b3c5bab683d5d97ab32eeb0ffbbf46433a4e54b81ad8f4fb68323a7eb3d87d5c4a8ee20d67da629945f43633c230633126d3d239c8c1cfcbffbfafe0c9d82fb6c67f1eedfc3d9a7ebb37792a89535229ae4d628f583a351e3ec971d8d488d215a848f0d61a0520ce9a91b3e71cc04df79f4f8e81b2930f0dd75ed8b941ea0e924b522a5612c7ce95b45003f556431618a67af550ab11178c7b3fee4fb73fce633e9cc3edcde18617874ef79aadd55a5d4a6964104103426e81a47972671144470b50338504a5a92704c862373c448b18ba4513364d350c9288a481b92af7e41507b66b12c24e149c13620f0158c52c64dd3b7f7097178e0e27535a2e4f3f3ffb4aafecfdf28bb4d393cd27a4b4d8acec457d59dfaddebedef9f7f01a6587885cc9010000";
        let pubkey_str = "ypub6Ww3ibxVfGzLrAH1PNcjyAWenMTbbAosGNB6VvmSEgytSER9azLDWCxoJwW7Ke7icmizBMXrzBx9979FfaHxHcrArf3zbeJJJUZPf663zsP";
        {
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let signed_tx = sign_raw_tx(payload, context, &seed).unwrap();
            assert_eq!("02000000000102a80b91b95992983510e3ee1bc603f82684464969b85c3ecf424fe6b86c7857670000000017160014f990679acafe25c27615373b40bf22446d24ff44fdffffff6a0920b7e80c17930da1b22266b57beb36e1091208043ace6cdbddaed9fbe0430000000017160014f673fea66cb63170bcd84a646ce66cf0dddc9a32fdffffff01dc0500000000000017a914e2d787a34f79160fa5d7b49ad667febe4bb3295387024830450221009f1ba19d3e76fa505c7c9a23df4a79a106e258021c701cd642379dfd7df226bd0220124ae5bf43671b7d8e5a1e1e750351e92b762ae940824ca0427b84a9a143dfa30121039b3b694b8fc5b5e07fb069c783cac754f5d38c3e08bed1960e31fdb1dda35c2402483045022100a26c15549f9e1bf96ce1f49e08220d3b6d864082f9b8c6a6309837c25e87176702206690e1ef6be4cd0743895a0799e55d45f18f08f20170f62410acc2cfc1c3d95e0121022a421fa4a65a87d1c3e4238155d85f7bd2c5bb87632f331b5722f110586aa19800000000", signed_tx.0);
        }
        {
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let check = check_raw_tx(payload, context).unwrap();
            assert_eq!((), check);
        }
        {
            //check with change address
            let hex="1f8b08000000000000035d8e3d6b94411485892bb8ac4562aa2555580425b0ecfd987be78e9564c514364a9226dddcf9108cf2424c10acfc21d6b696823fc0c2dad2c2d2cade2e2f4995c069ce81e7f04cef6c6fbe3a5b0fb5edbe3c1bce8732bcddf93119d769e42235435e7c9bcc26fb47ebed87a5a4a0de68692d9465206fcbcc844bd12c9cad7441ddfdfaefcff7fff0784a9f26d39f0fb67eed2d3e6fcc9e6a94184d8b5bd3d00395ce4dc6a6298c9f1648bb0117456fad31024bb244923c2504cfb67330db074ece3e126ebd884b83d81d3495685c728912ba54b6c20dcc5bc5a4305ef5ea586b662914e65fee2e36dfad427ab482ebace04a2f8c4cf79a72abb5ba96d23843004382d490b57974175522a78c50136384d2cc230124cd577a443910f61cb24ab658713409c48622d5a447af346abb4565eacce812893a228869ce986cfef7de6d3ddc7b325b7078217a7af8e6f5f1faf8f0991d1c9d5f3c8f259d7cfc309cfa7bbd38895bbfefcf376eb2b8824b3d3de48fdc010000";
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let check = check_raw_tx(payload, context).unwrap();
            assert_eq!((), check);
        }
    }

    #[test]
    fn test_sign_bch_p2pkh_transaction() {
        // enroll
        // rawTx: 0200000001b1cae605eacfec4e2a0f91f13763299a835b857a9f1ce6cba3f8baa435f4db68000000006a47304402207d43a733d0b90a38965c43da9d7208869e6a4635715bb9409c27d07d79bff7aa02204f3ae45fc6fbc374ce4a41fd535c66d9139f5fa8abdeb61ffcb50fe4e28b11bb41210305a8517b0fb8ebeef8c72e41b6c30c0830270c2f0fc29a712e5f0ebfa8ab5403ffffffff01ec790800000000001976a914530523097222a54328b4ab78bdbcab172232065888ac00000000
        let hex = "1f8b08000000000000030d8d3b4e025114400336131a81ca580131c14c3299f7eefb77061a4b7507f77d2e062423cc88ba0a57a05bb07709d616962cc0d858183b2739d5494e4ed61d1e5e6ee7554ca38b6dd554a1ba397ee9b6363322a8880c274fdddec16c7e3e3c09c149ed131436c95048f0a94001bc501a95401b48713d7afddebffdb1d3eceab393bd0ffa1ff9e4b9d33bd3367a9242a1f4481645f049f3e0c8a055ca5be1109c16c210779c11a04c29044ac854d2013d3fdaff8e273306de27137d88643d80e64a30b4da4432dc32cf0d3ae6a5053292c008eddb138b828cd24942026ef2c1ba94725a72a9a6256b2959ce7af9e6966a5804ba5f08803a2d564b79dd967772d72c1f77f543b3d1eb7ae518aafed7cff81f4c55a87f34010000";
        let pubkey_str = "xpub6ByHsPNSQXTWZ7PLESMY2FufyYWtLXagSUpMQq7Un96SiThZH2iJB1X7pwviH1WtKVeDP6K8d6xxFzzoaFzF3s8BKCZx8oEDdDkNnp4owAZ";
        {
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let signed_tx = sign_raw_tx(payload, context, &seed).unwrap();
            assert_eq!("0200000001b1cae605eacfec4e2a0f91f13763299a835b857a9f1ce6cba3f8baa435f4db68000000006b483045022100a3a2abd7a739f8319c753a1e71d164e71eea8841abfdaf4ceca3e3bcb5319073022028f15f4a5b0620eb1da04c3bef1c8fa879d67196919427a2f18cf5b75a8fff94412102bbe7dbcdf8b2261530a867df7180b17a90b482f74f2736b8a30d3f756e42e217fdffffff01ec790800000000001976a914530523097222a54328b4ab78bdbcab172232065888ac00000000", signed_tx.0);
        }
        {
            // check
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let check = check_raw_tx(payload, context).unwrap();
            assert_eq!((), check);
        }
        {
            // check with output(cash address)
            let hex="1f8b0800000000000003658dbb4e0241144003361b1a818a58c1c604b3c966e73db39d819858aa7f70e7ce5c88aeac3c43f8173b7b0b3b3fc1da0fe003d4c6c2d8496f72aa539c9334bbc7d78b711d62ff6a51af6aacab9397e6c12656a20ec0207d6ab68e46e3cbee2962a98c8f22775161ae848f3948c1736d404b70489a9bfef3d7fef5979d25379f8de4add37ecfd2c746ebdcb8e049490dca033990e8a3e1589205a7b577b204511a292df1923312a06244a4084c4783e0796fff3348474c781f6df018c879210cd79281333690e58e796ea1645e39415691b0d2f8c3890549569ba84414dc669dfb42a961c1951e16ec40c1b28b56367f704b0ad3c96d35376e5aedeea0ac70b95de16cb3decc02dbceb7eb09ed76a8db1fdf835e23fd97e17f24a9ccb649010000";
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let check = check_raw_tx(payload, context).unwrap();
            assert_eq!((), check);
        }
    }
}
