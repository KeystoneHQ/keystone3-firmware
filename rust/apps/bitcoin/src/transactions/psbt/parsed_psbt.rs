use crate::errors::{BitcoinError, Result};
use crate::network::Network;
use crate::transactions::parsed_tx::{ParseContext, ParsedInput, ParsedOutput, ParsedTx, TxParser};
use crate::transactions::psbt::wrapped_psbt::WrappedPsbt;
use alloc::vec::Vec;
use core::ops::Index;
use third_party::bitcoin::bip32::ChildNumber;

impl TxParser for WrappedPsbt {
    fn parse(&self, context: Option<&ParseContext>) -> Result<ParsedTx> {
        let network = self.determine_network()?;
        let context = context.ok_or(BitcoinError::InvalidParseContext(format!("empty context")))?;
        let inputs = self
            .psbt
            .inputs
            .iter()
            .enumerate()
            .map(|(i, input)| self.parse_input(input, i, context, &network))
            .collect::<Result<Vec<ParsedInput>>>()?;
        let outputs = self
            .psbt
            .outputs
            .iter()
            .enumerate()
            .map(|(i, output)| self.parse_output(output, i, context, &network))
            .collect::<Result<Vec<ParsedOutput>>>()?;
        self.normalize(inputs, outputs, &network)
    }
    fn determine_network(&self) -> Result<Network> {
        if let Some((xpub, _)) = self.psbt.xpub.first_key_value() {
            return if xpub.network == third_party::bitcoin::network::Network::Bitcoin {
                Ok(Network::Bitcoin)
            } else {
                Ok(Network::BitcoinTestnet)
            };
        }
        let possible_my_input = self
            .psbt
            .inputs
            .iter()
            .find(|v| {
                v.bip32_derivation.first_key_value().is_some()
                    || v.tap_key_origins.first_key_value().is_some()
            })
            .ok_or(BitcoinError::InvalidInput)?;
        let path =
            if let Some((_, (_, path))) = possible_my_input.bip32_derivation.first_key_value() {
                path
            } else if let Some((_, (_, (_, path)))) =
                possible_my_input.tap_key_origins.first_key_value()
            {
                path
            } else {
                return Err(BitcoinError::InvalidInput);
            };
        let coin_type = path.index(1);
        match coin_type {
            ChildNumber::Hardened { index } => match index {
                0 => Ok(Network::Bitcoin),
                1 => Ok(Network::BitcoinTestnet),
                _ => Err(BitcoinError::InvalidTransaction(format!(
                    "unknown network {}",
                    index
                ))),
            },
            _ => Err(BitcoinError::InvalidTransaction(format!(
                "unsupported derivation path {}",
                path
            ))),
        }
    }
}

#[cfg(test)]
mod tests {
    extern crate std;

    use alloc::vec::Vec;

    use alloc::string::ToString;
    use core::str::FromStr;
    use std::collections::BTreeMap;
    use third_party::bitcoin::bip32::{DerivationPath, Fingerprint, Xpub};

    use super::*;
    use crate::parsed_tx::TxParser;
    use crate::TxChecker;
    use third_party::bitcoin::psbt::Psbt;
    use third_party::bitcoin_hashes::hex::FromHex;
    use third_party::either::Left;

    #[test]
    fn test_parse_psbt() {
        let psbt_hex = "70736274ff01005202000000016d41e6873468f85aff76d7709a93b47180ea0784edaac748228d2c474396ca550000000000fdffffff01a00f0000000000001600146623828c1f87be7841a9b1cc360d38ae0a8b6ed0000000000001011f6817000000000000160014d0c4a3ef09e997b6e99e397e518fe3e41a118ca1220602e7ab2537b5d49e970309aae06e9e49f36ce1c9febbd44ec8e0d1cca0b4f9c3191873c5da0a54000080010000800000008000000000000000000000";
        let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
        let wpsbt = WrappedPsbt { psbt };
        let master_fingerprint = Fingerprint::from_str("73c5da0a").unwrap();
        let extended_pubkey = Xpub::from_str("xpub6Bm9M1SxZdzL3TxdNV8897FgtTLBgehR1wVNnMyJ5VLRK5n3tFqXxrCVnVQj4zooN4eFSkf6Sma84reWc5ZCXMxPbLXQs3BcaBdTd4YQa3B").unwrap();
        let path = DerivationPath::from_str("m/84'/1'/0'").unwrap();
        let mut keys = BTreeMap::new();
        keys.insert(path, extended_pubkey);

        let result = wpsbt
            .parse(Some(&ParseContext {
                master_fingerprint,
                extended_public_keys: keys,
                verify_code: None,
                multisig_wallet_config: None,
            }))
            .unwrap();
        assert_eq!("0.00005992 tBTC", result.detail.total_input_amount);
        assert_eq!("0.00004 tBTC", result.detail.total_output_amount);
        assert_eq!("0.00001992 tBTC", result.detail.fee_amount);

        assert_eq!("4000 sats", result.overview.total_output_sat);
        assert_eq!("1992 sats", result.overview.fee_sat);

        assert_eq!("Bitcoin Testnet", result.overview.network);

        let first_input = result.detail.from.get(0).unwrap();
        assert_eq!(
            "tb1q6rz28mcfaxtmd6v789l9rrlrusdprr9pqcpvkl",
            first_input.address.clone().unwrap()
        );
        assert_eq!(true, first_input.path.is_some());
        assert_eq!(5992, first_input.value);

        let first_output = result.detail.to.get(0).unwrap();
        assert_eq!(
            "tb1qvc3c9rqls7l8ssdfk8xrvrfc4c9gkmks87chvk",
            first_output.address
        );
        assert_eq!(false, first_output.path.is_some());
        assert_eq!(4000, first_output.value);
    }

    #[test]
    fn test_parse_multisig_psbt() {
        let psbt_hex = "70736274ff0100890200000001ed88352da4dca05f68c31cb05cf0940e2095b2fd602817d83ce0b2a6b0cd1eb80100000000fdffffff022d2a0000000000002200209dd6ca583cc39e08119cc73366d27e36bf9bba5f189ea697f1ebd8e04abd0690d00700000000000022002058f6dcd978c83e0bf741858f4c8e0d1212201074f02beb9b4ac4b2b75555718f01c80c004f010488b21e04b9124e6180000002cd32b21fd3718c3c08da48da5f2b44b00088dadf3179b8d7e0d23c63ad116d0e02fea2dd02ac2399962b4c0918aa8c21c3db988e9721ea425ecf00cc2705435b681452744703300000800000008000000080020000804f010488b21e045428576b8000000242ce3e3e0b5cd02d32c4b87816f1ae67e6b93c643750eb4fad4a264db872df9d02faa46048f4034e16d6cc1eaf687b3e21323d2359266bdd966e3eb5ca12b3eee51450659928300000800000008000000080020000804f010488b21e0481b9c1088000000253eaf13fdc5a252d0f66c061680e3302cddbfd00d2cd17ef7f2733edc6d66a5902b4cc2d00911da74b936b00f49b97c830b174d2b941b1894b1078b4ecec5d6e4f14515e384a300000800000008000000080020000800001012b78370000000000002200207bb3f49d46913a6e845e9b72ec167e60ab68a985c03fdf075f752175ef9e974101030401000000010569522102efafaa647275be895cfe637cb1e2b1eacde91196faf5c3e8a20b5a54b467253821031f47c601b25a6ebbbb6c87042d3888569270788095a16dd568970ca5cbd9befc2103d3eaaf547b30efabaeb20764d4747daec425b08c91205c3bb876b1a52175100e53ae220602efafaa647275be895cfe637cb1e2b1eacde91196faf5c3e8a20b5a54b46725381c52744703300000800000008000000080020000800000000000000000220603d3eaaf547b30efabaeb20764d4747daec425b08c91205c3bb876b1a52175100e1c506599283000008000000080000000800200008000000000000000002206031f47c601b25a6ebbbb6c87042d3888569270788095a16dd568970ca5cbd9befc1c515e384a3000008000000080000000800200008000000000000000000001016952210248dc844665aa6aafe1116bd9a754b98b9e888a33d36aececd30bbce267b3b2df2102bf4ce0d4ad56477221d9ac15f0bd1dd7fdc9d3ecee22749a9f5c19bc6baf29292103f87381ec21cbadfc089c2f0df9a0128f22b159af572bf61c1b76b6219f140e8153ae22020248dc844665aa6aafe1116bd9a754b98b9e888a33d36aececd30bbce267b3b2df1c52744703300000800000008000000080020000800100000000000000220203f87381ec21cbadfc089c2f0df9a0128f22b159af572bf61c1b76b6219f140e811c50659928300000800000008000000080020000800100000000000000220202bf4ce0d4ad56477221d9ac15f0bd1dd7fdc9d3ecee22749a9f5c19bc6baf29291c515e384a300000800000008000000080020000800100000000000000000101695221025489d7af6a4bb8b62cb7ad7f1c06368530e5ac31fe62f210f926fc2e027bfe7c210371eb571331a919b1b7a3a06348c6dc299c1e953ec39daee2912bc97f5b9fc4b22103f1bb9f033b28a98dbb82fe3a56c5b8c0317afb706d13d15591a3f12c7f65e4b753ae22020371eb571331a919b1b7a3a06348c6dc299c1e953ec39daee2912bc97f5b9fc4b21c527447033000008000000080000000800200008000000000010000002202025489d7af6a4bb8b62cb7ad7f1c06368530e5ac31fe62f210f926fc2e027bfe7c1c50659928300000800000008000000080020000800000000001000000220203f1bb9f033b28a98dbb82fe3a56c5b8c0317afb706d13d15591a3f12c7f65e4b71c515e384a30000080000000800000008002000080000000000100000000";
        let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
        let wpsbt = WrappedPsbt { psbt };
        let master_fingerprint = Fingerprint::from_str("515E384A").unwrap();
        let extended_pubkey = Xpub::from_str("xpub6EVD16NVrvhxWJ9iKDZVfhRYPhoLBrKyPtmNAgDz1vRHvCSyU3beHwsXmt4vaLf5aJux6CMoDqaFXXGLZGeoco75v2yiSxJyTKJFSMWBoTR").unwrap();
        let path = DerivationPath::from_str("m/48'/0'/0'/2'").unwrap();
        let mut keys = BTreeMap::new();
        keys.insert(path, extended_pubkey);

        let result = wpsbt
            .parse(Some(&ParseContext {
                master_fingerprint,
                extended_public_keys: keys,
                verify_code: None,
                multisig_wallet_config: None,
            }))
            .unwrap();

        assert_eq!("0.000142 BTC", result.detail.total_input_amount);
        assert_eq!("0.00012797 BTC", result.detail.total_output_amount);
        assert_eq!("Unsigned", result.detail.sign_status.unwrap());
        assert_eq!((0, 2), result.detail.from[0].sign_status);
    }

    #[test]
    fn test_parse_psbt_taproot_tx() {
        let psbt_hex = "70736274ff01005e02000000013aee4d6b51da574900e56d173041115bd1e1d01d4697a845784cf716a10c98060000000000ffffffff0100190000000000002251202258f2d4637b2ca3fd27614868b33dee1a242b42582d5474f51730005fa99ce8000000000001012bbc1900000000000022512022f3956cc27a6a9b0e0003a0afc113b04f31b95d5cad222a65476e8440371bd1010304000000002116b68df382cad577d8304d5a8e640c3cb42d77c10016ab754caa4d6e68b6cb296d190073c5da0a5600008001000080000000800000000002000000011720b68df382cad577d8304d5a8e640c3cb42d77c10016ab754caa4d6e68b6cb296d011820c913dc9a8009a074e7bbc493b9d8b7e741ba137f725f99d44fbce99300b2bb0a0000";
        let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
        let wpsbt = WrappedPsbt { psbt };
        let master_fingerprint = Fingerprint::from_str("73c5da0a").unwrap();
        let extended_pubkey = Xpub::from_str("tpubDDfvzhdVV4unsoKt5aE6dcsNsfeWbTgmLZPi8LQDYU2xixrYemMfWJ3BaVneH3u7DBQePdTwhpybaKRU95pi6PMUtLPBJLVQRpzEnjfjZzX").unwrap();
        let path = DerivationPath::from_str("m/86'/1'/0'").unwrap();
        let mut keys = BTreeMap::new();
        keys.insert(path, extended_pubkey);

        let result = wpsbt
            .parse(Some(&ParseContext {
                master_fingerprint,
                extended_public_keys: keys,
                verify_code: None,
                multisig_wallet_config: None,
            }))
            .unwrap();

        assert_eq!("0.00006588 tBTC", result.detail.total_input_amount);
        assert_eq!("0.000064 tBTC", result.detail.total_output_amount);
        assert_eq!("0.00000188 tBTC", result.detail.fee_amount);

        assert_eq!("6400 sats", result.overview.total_output_sat);
        assert_eq!("188 sats", result.overview.fee_sat);

        assert_eq!("Bitcoin Testnet", result.overview.network);

        let first_input = result.detail.from.get(0).unwrap();
        assert_eq!(
            "tb1pytee2mxz0f4fkrsqqws2lsgnkp8nrw2atjkjy2n9gahggsphr0gszaxxmv",
            first_input.address.clone().unwrap()
        );
        assert_eq!(true, first_input.path.is_some());
        assert_eq!(6588, first_input.value);

        let first_output = result.detail.to.get(0).unwrap();
        assert_eq!(
            "tb1pyfv094rr0vk28lf8v9yx3veaacdzg26ztqk4ga84zucqqhafnn5q9my9rz",
            first_output.address
        );
        assert_eq!(false, first_output.path.is_some());
        assert_eq!(6400, first_output.value);
    }

    #[test]
    fn test_parse_multi_sig_psbt() {
        let psbt_hex= "70736274ff01005e0200000001d8d89245a905abe9e2ab7bb834ebbc50a75947c82f96eeec7b38e0b399a62c490000000000fdffffff0158070000000000002200202b9710701f5c944606bb6bab82d2ef969677d8b9d04174f59e2a631812ae739bf76c27004f01043587cf0473f7e9418000000147f2d1b4bef083e346eb1949bcd8e2b59f95d8391a9eb4e1ea9005df926585480365fd7b1eca553df2c4e17bc5b88384ceda3d0d98fa3145cff5e61e471671a0b214c45358fa300000800100008000000080010000804f01043587cf04bac1483980000001a73adbe2878487634dcbfc3f7ebde8b1fc994f1ec06860cf01c3fe2ea791ddb602e62a2a9973ee6b3a7af47c229a5bde70bca59bd04bbb297f5693d7aa256b976d1473c5da0a3000008001000080000000800100008000010120110800000000000017a914980ec372495334ee232575505208c0b2e142dbb5872202032ed737f53936afb128247fc71a0b0b5be4d9348e9a48bfda9ef31efe3e45fa2e47304402203109d97095c61395881d6f75093943b16a91e1a4fff73bf193fcfe6e7689a35c02203bd187fed5bba45ee2c322911b8abb07f1d091520f5598259047d0dee058a75e01010304010000000104220020ffac81e598dd9856d08bd6c55b712fd23ea8522bd075fcf48ed467ced2ee015601054752210267ea4562439356307e786faf40503730d8d95a203a0e345cb355a5dfa03fce0321032ed737f53936afb128247fc71a0b0b5be4d9348e9a48bfda9ef31efe3e45fa2e52ae2206032ed737f53936afb128247fc71a0b0b5be4d9348e9a48bfda9ef31efe3e45fa2e1cc45358fa30000080010000800000008001000080000000000000000022060267ea4562439356307e786faf40503730d8d95a203a0e345cb355a5dfa03fce031c73c5da0a3000008001000080000000800100008000000000000000000000";
        let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
        let wpsbt = WrappedPsbt { psbt };
        let master_fingerprint = Fingerprint::from_str("73c5da0a").unwrap();
        let extended_pubkey = Xpub::from_str("tpubDFH9dgzveyD8yHQb8VrpG8FYAuwcLMHMje2CCcbBo1FpaGzYVtJeYYxcYgRqSTta5utUFts8nPPHs9C2bqoxrey5jia6Dwf9mpwrPq7YvcJ").unwrap();
        let path = DerivationPath::from_str("m/48'/1'/0'/1'").unwrap();
        let mut keys = BTreeMap::new();
        keys.insert(path, extended_pubkey);

        let result = wpsbt
            .parse(Some(&ParseContext {
                master_fingerprint,
                extended_public_keys: keys,
                verify_code: None,
                multisig_wallet_config: None,
            }))
            .unwrap();

        println!("result is {:?}", result);

        assert_eq!("0.00002065 tBTC", result.detail.total_input_amount);
        assert_eq!("0.0000188 tBTC", result.detail.total_output_amount);
        assert_eq!("0.00000185 tBTC", result.detail.fee_amount);

        assert_eq!("1880 sats", result.overview.total_output_sat);
        assert_eq!("185 sats", result.overview.fee_sat);

        assert_eq!("Bitcoin Testnet", result.overview.network);
        assert_eq!("1/2 Signed", result.overview.sign_status.unwrap());

        let first_input = result.detail.from.get(0).unwrap();
        assert_eq!(
            "2N77EPE2yfeTLR3CwNUCBQ7LZEUGW6N9B6y",
            first_input.address.clone().unwrap()
        );
        assert_eq!(true, first_input.path.is_some());
        assert_eq!(2065, first_input.value);

        let first_output = result.detail.to.get(0).unwrap();
        assert_eq!(
            "tb1q9wt3quqltj2yvp4mdw4c95h0j6t80k9e6pqhfav79f33sy4wwwdsakdtne",
            first_output.address
        );
        assert_eq!(false, first_output.path.is_some());
        assert_eq!(1880, first_output.value);
    }
}
