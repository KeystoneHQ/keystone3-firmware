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

    use core::str::FromStr;
    use std::collections::BTreeMap;
    use third_party::bitcoin::bip32::{DerivationPath, Fingerprint, Xpub};

    use super::*;
    use crate::parsed_tx::TxParser;
    use third_party::bitcoin::psbt::Psbt;
    use third_party::bitcoin_hashes::hex::FromHex;

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
}
