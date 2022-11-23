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
        let first_input = self.psbt.inputs.get(0).ok_or(BitcoinError::InvalidInput)?;
        let (_, (_, path)) = first_input
            .bip32_derivation
            .first_key_value()
            .ok_or(BitcoinError::InvalidInput)?;
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
    use third_party::bitcoin::bip32::{DerivationPath, ExtendedPubKey, Fingerprint};

    use super::*;
    use crate::parsed_tx::TxParser;
    use third_party::bitcoin::hashes::hex::FromHex;
    use third_party::bitcoin::psbt::Psbt;

    #[test]
    fn test_parse_psbt() {
        let psbt_hex = "70736274ff01005202000000016d41e6873468f85aff76d7709a93b47180ea0784edaac748228d2c474396ca550000000000fdffffff01a00f0000000000001600146623828c1f87be7841a9b1cc360d38ae0a8b6ed0000000000001011f6817000000000000160014d0c4a3ef09e997b6e99e397e518fe3e41a118ca1220602e7ab2537b5d49e970309aae06e9e49f36ce1c9febbd44ec8e0d1cca0b4f9c3191873c5da0a54000080010000800000008000000000000000000000";
        let psbt = Psbt::deserialize(&Vec::from_hex(psbt_hex).unwrap()).unwrap();
        let wpsbt = WrappedPsbt { psbt };
        let master_fingerprint = Fingerprint::from_str("73c5da0a").unwrap();
        let extended_pubkey = ExtendedPubKey::from_str("xpub6Bm9M1SxZdzL3TxdNV8897FgtTLBgehR1wVNnMyJ5VLRK5n3tFqXxrCVnVQj4zooN4eFSkf6Sma84reWc5ZCXMxPbLXQs3BcaBdTd4YQa3B").unwrap();
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
}
