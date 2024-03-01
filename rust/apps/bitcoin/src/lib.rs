#![no_std]
#![feature(error_in_core)]

#[allow(unused_imports)] // stupid compiler
#[macro_use]
extern crate alloc;
extern crate core;

#[cfg(test)]
#[macro_use]
extern crate std;

pub mod addresses;
pub mod errors;
mod macros;
pub mod network;
mod transactions;
pub use addresses::get_address;
pub use transactions::legacy::sign_legacy_tx;
pub use transactions::parsed_tx;
pub use transactions::psbt::parsed_psbt;

use alloc::vec::Vec;

use third_party::bitcoin::psbt::Psbt;

use crate::errors::{BitcoinError, Result};
use crate::parsed_tx::{ParseContext, ParsedTx, TxParser};
use crate::transactions::legacy::TxData;
use crate::transactions::psbt::wrapped_psbt::WrappedPsbt;
use crate::transactions::tx_checker::TxChecker;
use alloc::string::{String, ToString};
use app_utils::keystone;
use third_party::bitcoin::bip32::Fingerprint;
use third_party::either::{Left, Right};
use third_party::hex;
use third_party::ur_registry::pb::protoc;

pub fn sign_psbt(psbt_hex: Vec<u8>, seed: &[u8], mfp: Fingerprint) -> Result<Vec<u8>> {
    let psbt = deserialize_psbt(psbt_hex)?;
    let mut wpsbt = WrappedPsbt { psbt };
    let result = wpsbt.sign(seed, mfp)?;
    Ok(result.serialize())
}

pub fn parse_psbt(psbt_hex: Vec<u8>, context: ParseContext) -> Result<ParsedTx> {
    let psbt = deserialize_psbt(psbt_hex)?;
    let wpsbt = WrappedPsbt { psbt };
    wpsbt.parse(Some(&context))
}

pub fn check_psbt(psbt_hex: Vec<u8>, context: ParseContext) -> Result<()> {
    let psbt = deserialize_psbt(psbt_hex)?;
    let wpsbt = WrappedPsbt { psbt };
    wpsbt.check(Left(&context))
}

pub fn parse_raw_tx(raw_tx: protoc::Payload, context: keystone::ParseContext) -> Result<ParsedTx> {
    let tx_data = TxData::from_payload(raw_tx, &context)?;
    tx_data.parse(None)
}

pub fn sign_raw_tx(
    raw_tx: protoc::Payload,
    context: keystone::ParseContext,
    seed: &[u8],
) -> Result<(String, String)> {
    let tx_data = &mut TxData::from_payload(raw_tx, &context)?;
    let signed_tx = sign_legacy_tx(tx_data, seed)?;
    Ok((
        hex::encode(signed_tx),
        tx_data.transaction.txid().to_string(),
    ))
}

pub fn check_raw_tx(raw_tx: protoc::Payload, context: keystone::ParseContext) -> Result<()> {
    let tx_data = TxData::from_payload(raw_tx, &context)?;
    tx_data.check(Right(&context))
}

fn deserialize_psbt(psbt_hex: Vec<u8>) -> Result<Psbt> {
    Psbt::deserialize(&psbt_hex).map_err(|e| BitcoinError::InvalidPsbt(format!("{}", e)))
}

#[cfg(test)]
mod test {
    use crate::addresses::xyzpub::{convert_version, Version};
    use crate::alloc::string::ToString;
    use crate::parse_raw_tx;
    use crate::transactions::parsed_tx::{
        DetailTx, OverviewTx, ParsedInput, ParsedOutput, ParsedTx,
    };
    use alloc::vec::Vec;
    use app_utils::keystone;
    use core::str::FromStr;
    use third_party::hex::FromHex;
    use third_party::ur_registry::pb::protobuf_parser::{parse_protobuf, unzip};
    use third_party::ur_registry::pb::protoc::{Base, Payload};

    macro_rules! build_overview_tx {
        ($total_output_amount:expr, $fee_amount:expr, $total_output_sat:expr, $fee_sat:expr,$from:expr, $to: expr, $network: expr, $fee_larger_than_amount:expr) => {
            OverviewTx {
                total_output_amount: $total_output_amount.to_string(),
                fee_amount: $fee_amount.to_string(),
                total_output_sat: $total_output_sat.to_string(),
                fee_sat: $fee_sat.to_string(),
                from: $from.iter().map(|i| i.to_string()).collect(),
                to: $to.iter().map(|i| i.to_string()).collect(),
                network: $network.to_string(),
                fee_larger_than_amount: $fee_larger_than_amount,
            }
        };
    }

    macro_rules! build_parsed_input {
        ($address:expr, $amount:expr, $value:expr, $path:expr) => {
            ParsedInput {
                address: Some($address.to_string()),
                amount: $amount.to_string(),
                value: $value,
                path: Some($path.to_string()),
            }
        };
    }

    macro_rules! build_parsed_output {
        ($address:expr, $amount:expr, $value:expr, $path:expr) => {
            ParsedOutput {
                address: $address.to_string(),
                amount: $amount.to_string(),
                value: $value,
                path: Some($path.to_string()),
            }
        };
    }

    macro_rules! build_detailed_tx {
        ($from:expr, $to:expr, $total_input_amount:expr, $total_output_amount:expr, $fee_amount:expr, $total_input_sat:expr, $total_output_sat:expr, $fee_sat:expr, $network:expr) => {
            DetailTx {
                from: $from
                    .iter()
                    .map(|i| build_parsed_input!(i.0, i.1, i.2, i.3))
                    .collect(),
                to: $to
                    .iter()
                    .map(|i| build_parsed_output!(i.0, i.1, i.2, i.3))
                    .collect(),
                total_input_amount: $total_input_amount.to_string(),
                total_output_amount: $total_output_amount.to_string(),
                fee_amount: $fee_amount.to_string(),
                total_input_sat: $total_input_sat.to_string(),
                total_output_sat: $total_output_sat.to_string(),
                fee_sat: $fee_sat.to_string(),
                network: $network.to_string(),
            }
        };
    }

    pub fn prepare_parse_context(pubkey_str: &str) -> keystone::ParseContext {
        let master_fingerprint =
            third_party::bitcoin::bip32::Fingerprint::from_str("73c5da0a").unwrap();
        let extended_pubkey_str = convert_version(pubkey_str, &Version::Xpub).unwrap();
        let extended_pubkey =
            third_party::bitcoin::bip32::Xpub::from_str(extended_pubkey_str.as_str()).unwrap();
        keystone::ParseContext::new(master_fingerprint, extended_pubkey)
    }

    pub fn prepare_payload(hex: &str) -> Payload {
        let bytes = Vec::from_hex(hex).unwrap();
        let unzip_data = unzip(bytes.clone()).unwrap();
        let base: Base = parse_protobuf(unzip_data).unwrap();
        base.data.unwrap()
    }

    #[test]
    fn test_parse_legacy_ltc_tx() {
        // tube
        let hex = "1f8b0800000000000003558dbb4a03411846b36be192266baa902a2c8212583233ffdc162ccc0d63349268306837333b2b1875558c0979061fc0c242ec051b0b0b5b0b3bc156b0147d005bd30a1f070e1cf83c379feb9dd7d3d896bae7e9456ad2a3e2a7ebb9794f20d16c36783d7873b3739bfd7a7e9131ce12442124dcaa902a2dc32851101a3086608b2dc4b498293d7e3dddfda2654f5fbbdeeb82ff5e2e66825b27bbaa58a48d564a598cf54c4052a096c4334a42c1320b11610c63c60d5560a5b442c70669a264c239f84e713d5b43444422a20a4b6c1281ad8a88c51a04274c01235672c18d4418255881e1d6628230301dc78831008349e1e5fedb0b72c7151a2d55c85205cd5641e5301b74d6b8185407fbfcb0795c8dc4e660d4dc6ef787b59a386d75d2dde4e0d0ff7cb8720a9920535e99e583eaeede683c9d801e9eb5b6366abd8bbdc664e7723a1df346efa43d4efd9b9f8ff98213e43affcf4acfdd3f9997819c79010000";
        let pubkey_str = "ypub6X1mUc1jWSVhJJvVdafzD2SNG88rEsGWwbboBrrmWnMJ4HQwgvKrTkW2L7bQcLs1Pi1enPCXica1fnDryixfCptU1cQCYxVuSMw6woSKr47";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let parsed_tx = parse_raw_tx(payload, context).unwrap();
        let overview = build_overview_tx!(
            "0.023 LTC",
            "0.0000225 LTC",
            "2300000 sats",
            "2250 sats",
            vec!("MWAVYuwzx3bkqHNKBQtYDxSvzzw6DQnJwo"),
            vec![
                "MG67WAWZ6jEmA97LWuERJTkBB7pHMoVfgj",
                "MWAVYuwzx3bkqHNKBQtYDxSvzzw6DQnJwo"
            ],
            "Litecoin",
            false
        );
        let input1 = (
            "MWAVYuwzx3bkqHNKBQtYDxSvzzw6DQnJwo",
            "0.1851975 LTC",
            18519750,
            "m/49'/2'/0'/0/0",
        );
        let output1 = (
            "MG67WAWZ6jEmA97LWuERJTkBB7pHMoVfgj",
            "0.023 LTC",
            2300000,
            "",
        );
        let output2 = (
            "MWAVYuwzx3bkqHNKBQtYDxSvzzw6DQnJwo",
            "0.162175 LTC",
            16217500,
            "M/49'/2'/0'/0/0",
        );
        let detail = build_detailed_tx!(
            vec![input1],
            vec![output1, output2],
            "0.1851975 LTC",
            "0.185175 LTC",
            "0.0000225 LTC",
            "18519750 sats",
            "18517500 sats",
            "2250 sats",
            "Litecoin"
        );
        let expected_parsed_tx = ParsedTx { overview, detail };
        assert_eq!(expected_parsed_tx, parsed_tx);
    }

    #[test]
    fn test_parse_legacy_dash_tx() {
        // enroll
        let hex = "1f8b0800000000000003558ebb4a03411885374b90254d62aa902a2c82120c99cb3f333b88a046a2a00951e305bb9d997f428c1a8c177c005b2b6b41f4017c03b1166cad6c2db5b5755be154878ff39d282c17b726adb1c35a6f32be18dbf171f5338cc2722498a22da268fc1e16f2abcb3bebe519271d738a63434a691ac00d3412f4ac21a8f20a080526b11ad49e7fbe9f7ec95c747817466fd3a5c77c3588ef73852582d46a0e4625d459271855a92659c15383e0c1b1941aae09300644133498708fa9949ea714ac76a5a0f27ab318af10ae8594d9887188096aa9c00be9498a969bec200a4c2d330eac523e018188d258cb5c22c033ef9daa174f9a00b34d31db24599aa43e5f880f8cde5cf67effc85ef75b5db1d639122373383ceb8a813a3d1d0d2657a587dba94a1007f5858cb6c3e1c664677de0daa3cb4dddeef7393ddbf6fd5e7777effa00ce7bb6f4f1252bb9b8d8f9afaabdf4fe00936c1ab575010000";
        let pubkey_str = "xpub6DTnbXgbPo6mrRhgim9sg7Jp571onenuioxgfSDJEREH7wudyDQMDSoTdLQiYq3tbvZVkzcPe7nMgL7mbSixQQcShekfhKt3Wdx6dE8MHCk";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let parsed_tx = parse_raw_tx(payload, context).unwrap();
        let overview = build_overview_tx!(
            "0.001 DASH",
            "0.0000225 DASH",
            "100000 sats",
            "2250 sats",
            vec!["XciiKrSHgdFkuL9FTT31qRfTPNUVxX4sPc"],
            vec![
                "Xb9LAffWjcxTCN5GMj5kbZiqN5g7nnkgrv",
                "XciiKrSHgdFkuL9FTT31qRfTPNUVxX4sPc"
            ],
            "Dash",
            false
        );
        let input1 = (
            "XciiKrSHgdFkuL9FTT31qRfTPNUVxX4sPc",
            "0.01 DASH",
            1000000,
            "m/44'/5'/0'/0/0",
        );
        let output1 = (
            "Xb9LAffWjcxTCN5GMj5kbZiqN5g7nnkgrv",
            "0.001 DASH",
            100000,
            "",
        );
        let output2 = (
            "XciiKrSHgdFkuL9FTT31qRfTPNUVxX4sPc",
            "0.0089775 DASH",
            897750,
            "M/44'/5'/0'/0/0",
        );
        let detail = build_detailed_tx!(
            vec![input1],
            vec![output1, output2],
            "0.01 DASH",
            "0.0099775 DASH",
            "0.0000225 DASH",
            "1000000 sats",
            "997750 sats",
            "2250 sats",
            "Dash"
        );
        let expected_parsed_tx = ParsedTx { overview, detail };
        assert_eq!(expected_parsed_tx, parsed_tx);
    }

    #[test]
    fn test_parse_legacy_bch_tx() {
        // enroll
        let hex = "1f8b08000000000000035dcd3d6a1b411880e1f5dac5a2262b554295bc041c1684e69b9d6f7e0a83b10898808564bbb1bbf9e6c748b6b2f1227b135dc226a44e20e00be40039402e901ba44a972229e3daf0564ff36669efc5bc99d43e0c674dbdae5d7d3df89566692f43ae60c214143fd3cef6e1e4a8f7523aeb81a31f191fdc4800d088c8f311c90a41094168e220197efb77fff897bdca4e3ea7d98f6efe756790145fb63a074c2127af31107ae496ac06c763a594a95820c94823c420230962ce2033249cd7c0827268c8409ef47fffd92d0e19774032621006942327b9f60644e011009f140c2213962a8a16751056fb4a2a6da4335e33905197ddd55888bd3108dc1bb3a7c650569df2e6dd5ab8462c6ffd6d7b5dc9b6c2a68deddd46da15dfa8d57b77a9ad16575cbfcd1fa6fda448cafd4e01fae2c3eb1b2b16f5e9e26cb39ee2e6e8645acf270d5e9caf8fa767cb37f9c74fbbfdada27bfcfc39fc3efb0f41e405467f010000";
        let pubkey_str = "xpub6CjD9XYc1hEKcAMsSasAA87Mw8bSUr6WQKrJ1ErLofJPP9sxeZ3sh1dH2S5ywQTRNrXsfXzT686jJNdX2m9KhvMDh4eQM9AdSkkQLLMbDG6";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let parsed_tx = parse_raw_tx(payload, context).unwrap();
        let overview = build_overview_tx!(
            "0.0001 BCH",
            "0.0000225 BCH",
            "10000 sats",
            "2250 sats",
            vec!["qpfs2gcfwg322segkj4h30du4vtjyvsxtq6msk90a5"],
            vec![
                "qpfs2gcfwg322segkj4h30du4vtjyvsxtq6msk90a5",
                "qpt4cr4juduwl36w35rwfwvz6am2z7mxcg8a84k28n",
            ],
            "Bitcoin Cash",
            false
        );
        let input1 = (
            "qpfs2gcfwg322segkj4h30du4vtjyvsxtq6msk90a5",
            "0.005555 BCH",
            555500,
            "m/44'/145'/0'/0/1",
        );
        let output1 = (
            "qpt4cr4juduwl36w35rwfwvz6am2z7mxcg8a84k28n",
            "0.0001 BCH",
            10000,
            "",
        );
        let output2 = (
            "qpfs2gcfwg322segkj4h30du4vtjyvsxtq6msk90a5",
            "0.0054325 BCH",
            543250,
            "M/44'/145'/0'/0/1",
        );
        let detail = build_detailed_tx!(
            vec![input1],
            vec![output1, output2],
            "0.005555 BCH",
            "0.0055325 BCH",
            "0.0000225 BCH",
            "555500 sats",
            "553250 sats",
            "2250 sats",
            "Bitcoin Cash"
        );
        let expected_parsed_tx = ParsedTx { overview, detail };
        assert_eq!(expected_parsed_tx, parsed_tx);
    }

    #[test]
    fn test_parse_legacy_btc_native_segwit_tx() {
        // tube
        let hex = "1f8b0800000000000003658d3b4b42611c87f5d07070e9e4244e22412188effdb29517caa130921ae5ffbee73d8a17f498e6e553b4340611f8151a823e41d027696b288820ad31f80dcff27b1edf4b6f9f8d2bc3d0e51ae3e1646887fdec97e77b695f2259ab554525ffe6a576cacd4aebf4b059bfa8b5ce6b4797f5667a5771ee8cc3bc4811124566382d2ab05004ee2270cc1029c36c22f7f8f9bdfa40fb3e5979fe6b2678d8cadf275307c2027296600c587050a112064ba4003bc3a5c004bbc882309a6b4d210a29a20c21b329291552428264f6385546c49ad051a5b1a2c2ad554e6bc04681d09668ceb4e31b00bb4ea9b5433106045b4c8120c443a121f37e97ce072725c5f64ae877b8444981a50ac6e2b877153398ebee288e07d3c12ceab439eb4d661da20591cb482a42e56c14dcbc2433897ca250fd7bcdf56241898d05a1f162acd4a2ddebb6afbbedfe14f59642f0100d74a483dba72093fcd7a6b9e7c60f67e05bf594010000";
        let pubkey_str = "zpub6rQ4BxDEb1xJE2RJTyaoZT2NEr7FkGCgeEwkbgpBfPUTd6KmtBstbP9G81dPnJZJVAmbg2ZmfSc55FkrcHcKPvmNkLGmXAFqERtiUCn25LH";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let parsed_tx = parse_raw_tx(payload, context).unwrap();
        let overview = build_overview_tx!(
            "0.00026 BTC",
            "0.0000315 BTC",
            "26000 sats",
            "3150 sats",
            vec!["bc1qaukjm4glwmt8ghx5fmp92rgw3xa40xdmp2t8lr"],
            vec![
                "bc1qksq4ax9jpqqmumwfhg54ktwh29627zf78237wp",
                "bc1qx9yy32cq623qyr88ygkjgvjglu0kz665d0m9f9"
            ],
            "Bitcoin Mainnet",
            false
        );
        let input1 = (
            "bc1qaukjm4glwmt8ghx5fmp92rgw3xa40xdmp2t8lr",
            "0.00298739 BTC",
            298739,
            "M/84'/0'/0'/1/32",
        );
        let output1 = (
            "bc1qksq4ax9jpqqmumwfhg54ktwh29627zf78237wp",
            "0.00026 BTC",
            26000,
            "",
        );
        let output2 = (
            "bc1qx9yy32cq623qyr88ygkjgvjglu0kz665d0m9f9",
            "0.00269589 BTC",
            269589,
            "M/84'/0'/0'/1/33",
        );
        let detail = build_detailed_tx!(
            vec![input1],
            vec![output1, output2],
            "0.00298739 BTC",
            "0.00295589 BTC",
            "0.0000315 BTC",
            "298739 sats",
            "295589 sats",
            "3150 sats",
            "Bitcoin Mainnet"
        );
        let expected_parsed_tx = ParsedTx { overview, detail };
        assert_eq!(expected_parsed_tx, parsed_tx);
    }

    #[test]
    fn test_parse_legacy_btc_segwit_tx() {
        // tube
        let hex = "1f8b08000000000000ff5d8fb16bd4411085f14ef0388b8454e1aa700811e1b8999dd9d99d4e730645f4b818112c7776670b0939120e0b2bff103bc156b0f46fb1b0b4107b3b7fc44e78cd3c988fef4d46077b67d7ab6df3a3cdf576b7addb8bd9d7f1d04e12a4d3d3c7b29a7f1a4fc727af5607f76a5516f3b0c8ce75c1c17c5128e0224a895472ed11e5e8cbef1fdffec0fd49f8359a84fd0fa3f9c75bd38792624a59aa6517ee1c6a278fc325ca033273909e81aaa0b93b2150d4ac21aaa92258c9b327d31308484da44a4ed2b281b55c232a23472c1a093473cfc59b70c39c30abd6864906aea2694be1f0f3edf9de8b25ebf112fe6509377a4c0edd9a0ecfad99d4ea54802163007524714b665124040b05a1296182ead952005029377a641255057b8f894ba786053db14b37d19806d9c6a6587818eea5162c503ba7a883716c03f1f0e79dfff5f0c1f1744eebf5f9d5a3f7f4bcbc5dbf133f3fdb5d064aabdda63c6dcfda9bcdeb97fbdfeffe05f955ecdcc8010000";
        let pubkey_str = "ypub6WuuYTJLGMKvLmzxPSU28HTcUFeeaMHrycuJr4Q6JZe7suJWuZLEYgzHYUF9MJ4Y17q4AcVKgJaLa2UwvWPRartJVqcuhMCK5TFGCfZXhFD";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let parsed_tx = parse_raw_tx(payload, context).unwrap();
        let overview = build_overview_tx!(
            "0.000015 BTC",
            "0.0000005 BTC",
            "1500 sats",
            "50 sats",
            vec![
                "36kTQjs54H29LRhL9WiBSFbNtUyNMNCpEv",
                "3G8X84wtGDtiENAYagDSBcofp7NkjqHJFS"
            ],
            vec!["3NNSqAz3LajNv6eSQtn237CtPaHdJdYPVR"],
            "Bitcoin Mainnet",
            false
        );
        let input1 = (
            "36kTQjs54H29LRhL9WiBSFbNtUyNMNCpEv",
            "0.0000055 BTC",
            550,
            "M/49'/0'/0'/0/0",
        );
        let input2 = (
            "3G8X84wtGDtiENAYagDSBcofp7NkjqHJFS",
            "0.00001 BTC",
            1000,
            "M/49'/0'/0'/0/1",
        );
        let output1 = (
            "3NNSqAz3LajNv6eSQtn237CtPaHdJdYPVR",
            "0.000015 BTC",
            1500,
            "",
        );
        let detail = build_detailed_tx!(
            vec![input1, input2],
            vec![output1],
            "0.0000155 BTC",
            "0.000015 BTC",
            "0.0000005 BTC",
            "1550 sats",
            "1500 sats",
            "50 sats",
            "Bitcoin Mainnet"
        );
        let expected_parsed_tx = ParsedTx { overview, detail };
        assert_eq!(expected_parsed_tx, parsed_tx);
    }

    #[test]
    fn test_parse_legacy_btc_legacy_tx() {
        // tube
        let hex = "1f8b08000000000000035d90cf6a935110c5632a18be4d633686ac62109442c8ccbdf3dd99bbd3c6d08d420537aee4fe99b1584b9a5a89b8f4415cb8f115dcf4010a7d83be806fe046c14fc58d3067318bc3f99d33e88f769f9d2dd755a78767ebf37559bf995ced0cfaa30103af568fc37276b1d334fbcf972f9fac0e1e2d5f8cee792cb91648f3e20acc095b99c79cfc5ccc2b39c31a939ff4a6d7971fbf7c870703f7b33fb8ba3bfc7c73f6e946f310414128040ce640a56a27884256384b0e2e7224c59c8422e5564939438ad55a32b39a6dd89b1c34fbe035ab1a86a2482187448c1ca16d63086c2495a97a0b46ea29b9e22b082791ecbb9824d9098dbfde99ed3e5d10dd5fc0df5bd01fc0ae4a85d6392c0a05d8aa7122c939f36fc45c09b85b8624f9a089c9aab49ab2380da5ade8e41f60a6d0fd41318ac51858adad013b5b04344dde77b50aa690108d0b1278f444d179d36e19d3f18fdbff03c29e6ff672c1cdf1db0da5f7f1f5e96673f2ee646b47af5a3a3edf1eb9181c7f3016e7797b3afc766bdc9bf5a61787bf001ef38242e5010000";
        let pubkey_str = "xpub6Ch68rD9nNm8AReQe9VwV6Mi67okew4oiYoNpKAESTvmjoDn5dBUQ9A4oqdxc9VpveH1cxWeH237HLFSmC37gRVwxnL4KvFYxCQqfprRinW";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let parsed_tx = parse_raw_tx(payload, context).unwrap();
        let overview = build_overview_tx!(
            "0.00001 BTC",
            "0.000043 BTC",
            "1000 sats",
            "4300 sats",
            vec![
                "1NVWpSCxyzpPgSeGRs4zqFciZ7N1UEQtEc",
                "1PCUEmiFARh3FSLJXgzDGDeKtKqg8eMMPm",
            ],
            vec!["bc1qksq4ax9jpqqmumwfhg54ktwh29627zf78237wp"],
            "Bitcoin Mainnet",
            true
        );
        let input1 = (
            "1PCUEmiFARh3FSLJXgzDGDeKtKqg8eMMPm",
            "0.00003 BTC",
            3000,
            "M/44'/0'/0'/0/4",
        );
        let input2 = (
            "1NVWpSCxyzpPgSeGRs4zqFciZ7N1UEQtEc",
            "0.000023 BTC",
            2300,
            "M/44'/0'/0'/0/0",
        );
        let output1 = (
            "bc1qksq4ax9jpqqmumwfhg54ktwh29627zf78237wp",
            "0.00001 BTC",
            1000,
            "",
        );
        let detail = build_detailed_tx!(
            vec![input1, input2],
            vec![output1],
            "0.000053 BTC",
            "0.00001 BTC",
            "0.000043 BTC",
            "5300 sats",
            "1000 sats",
            "4300 sats",
            "Bitcoin Mainnet"
        );
        let expected_parsed_tx = ParsedTx { overview, detail };
        assert_eq!(expected_parsed_tx, parsed_tx);
    }
}
