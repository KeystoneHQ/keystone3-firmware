use alloc::string::{String, ToString};
use alloc::vec::Vec;

use serde_json::{from_slice, from_str, Value};

use crate::errors::Result;
use crate::proto_wrapper::fee::format_fee_from_value;
use crate::proto_wrapper::sign_doc::SignDoc;
use crate::transaction::detail::{CommonDetail, CosmosTxDetail, DetailMessage, DetailUnknown};
use crate::transaction::overview::{CommonOverview, CosmosTxOverview, MsgOverview};
use crate::transaction::structs::{CosmosTxDisplayType, DataType, ParsedCosmosTx};
use crate::transaction::utils::get_network_by_chain_id;

use self::detail::MsgDetail;

pub mod detail;
pub mod overview;
pub mod structs;
mod utils;

impl ParsedCosmosTx {
    pub fn build(data: &Vec<u8>, data_type: DataType) -> Result<Self> {
        match data_type {
            DataType::Amino => Self::build_from_amino(data),
            DataType::Direct => Self::build_from_direct(data),
        }
    }

    fn detect_display_type(kind: &Vec<MsgOverview>) -> CosmosTxDisplayType {
        if kind.is_empty() {
            return CosmosTxDisplayType::Unknown;
        }
        if kind.len() > 1 {
            return CosmosTxDisplayType::Multiple;
        }
        match kind[0] {
            MsgOverview::Send(_) => CosmosTxDisplayType::Send,
            MsgOverview::Delegate(_) => CosmosTxDisplayType::Delegate,
            MsgOverview::Redelegate(_) => CosmosTxDisplayType::Redelegate,
            MsgOverview::Undelegate(_) => CosmosTxDisplayType::Undelegate,
            MsgOverview::WithdrawReward(_) => CosmosTxDisplayType::WithdrawReward,
            MsgOverview::Transfer(_) => CosmosTxDisplayType::Transfer,
            MsgOverview::Vote(_) => CosmosTxDisplayType::Vote,
            MsgOverview::Message(_) => CosmosTxDisplayType::Message,
            // _ => CosmosTxDisplayType::Unknown,
        }
    }
    fn build_overview_from_amino(data: Value) -> Result<CosmosTxOverview> {
        let chain_id = data["chain_id"].as_str().unwrap_or(&"");
        let kind = CosmosTxOverview::from_value(&data["msgs"])?;
        let common = CommonOverview {
            network: get_network_by_chain_id(chain_id)?,
        };
        Ok(CosmosTxOverview {
            display_type: Self::detect_display_type(&kind),
            common,
            kind,
        })
    }

    fn build_detail_from_amino(data: Value) -> Result<String> {
        let chain_id = data["chain_id"].as_str().unwrap_or(&"");
        let common = CommonDetail {
            network: get_network_by_chain_id(chain_id)?,
            chain_id: chain_id.to_string(),
            fee: format_fee_from_value(data["fee"].clone()).ok(),
        };
        let kind = CosmosTxDetail::from_value(&data["msgs"])?;
        if kind.is_empty() {
            return Ok(serde_json::to_string::<DetailUnknown>(
                &common.to_unknown(),
            )?);
        }
        if let MsgDetail::Message(msg) = &kind[0] {
            return Ok(serde_json::to_string::<DetailMessage>(msg)?);
        }
        let detail = serde_json::to_string::<CosmosTxDetail>(&CosmosTxDetail { common, kind })?;
        Ok(detail)
    }

    fn build_from_amino(data: &Vec<u8>) -> Result<Self> {
        let v: Value = from_slice(data.as_slice())?;
        let overview = Self::build_overview_from_amino(v.clone())?;
        Ok(Self {
            overview,
            detail: Self::build_detail_from_amino(v.clone())?,
        })
    }

    fn build_from_direct(data: &Vec<u8>) -> Result<Self> {
        let sign_doc = SignDoc::parse(data)?;
        let doc_str = serde_json::to_string(&sign_doc)?;
        let v: Value = from_str(doc_str.as_str())?;
        let overview = Self::build_overview_from_amino(v.clone())?;
        Ok(Self {
            overview,
            detail: Self::build_detail_from_amino(v.clone())?,
        })
    }
}

#[cfg(test)]
mod tests {
    use serde_json::{json, Value};
    use third_party::hex;

    use crate::transaction::structs::DataType;

    use super::*;

    #[test]
    fn test_parse_cosmos_send_amino_json() {
        //{"account_number": String("1674671"), "chain_id": String("cosmoshub-4"), "fee": Object {"amount": Array [Object {"amount": String("2583"), "denom": String("uatom")}], "gas": String("103301")}, "memo": String(""), "msgs": Array [Object {"type": String("cosmos-sdk/MsgSend"), "value": Object {"amount": Array [Object {"amount": String("12000"), "denom": String("uatom")}], "from_address": String("cosmos17u02f80vkafne9la4wypdx3kxxxxwm6f2qtcj2"), "to_address": String("cosmos1kwml7yt4em4en7guy6het2q3308u73dff983s3")}}], "sequence": String("2")}
        let raw_tx = "7B226163636F756E745F6E756D626572223A2231363734363731222C22636861696E5F6964223A22636F736D6F736875622D34222C22666565223A7B22616D6F756E74223A5B7B22616D6F756E74223A2232353833222C2264656E6F6D223A227561746F6D227D5D2C22676173223A22313033333031227D2C226D656D6F223A22222C226D736773223A5B7B2274797065223A22636F736D6F732D73646B2F4D736753656E64222C2276616C7565223A7B22616D6F756E74223A5B7B22616D6F756E74223A223132303030222C2264656E6F6D223A227561746F6D227D5D2C2266726F6D5F61646472657373223A22636F736D6F733137753032663830766B61666E65396C61347779706478336B78787878776D3666327174636A32222C22746F5F61646472657373223A22636F736D6F73316B776D6C37797434656D34656E37677579366865743271333330387537336466663938337333227D7D5D2C2273657175656E6365223A2232227D";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Amino).unwrap();
        let overview = result.overview;
        assert_eq!("Cosmos Hub", overview.common.network);
        match overview.kind[0].clone() {
            MsgOverview::Send(overview) => {
                assert_eq!("0.012 ATOM", overview.value);
                assert_eq!(
                    "cosmos17u02f80vkafne9la4wypdx3kxxxxwm6f2qtcj2",
                    overview.from
                );
                assert_eq!("cosmos1kwml7yt4em4en7guy6het2q3308u73dff983s3", overview.to);
            }
            _ => {
                panic!("program overview parse error!")
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Cosmos Hub",
                "Chain ID": "cosmoshub-4",
                "Max Fee": "266.826483 ATOM",
                "Fee": "0.002583 ATOM",
                "Gas Limit": "103301"
            },
            "kind": [
                {
                    "Method": "Send",
                    "Value": "0.012 ATOM",
                    "From": "cosmos17u02f80vkafne9la4wypdx3kxxxxwm6f2qtcj2",
                    "To": "cosmos1kwml7yt4em4en7guy6het2q3308u73dff983s3"
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_send_direct() {
        let raw_tx = hex::decode("0A9D010A9A010A1C2F636F736D6F732E62616E6B2E763162657461312E4D736753656E64127A0A2C65766D6F7331747173647A37383573716A6E6C67676565306C77786A77666B36646C33366165327566396572122C65766D6F73316C637A71773934766674616D357A387777676C6E373337346D713230636A78706739387039661A1C0A07617465766D6F7312113130303030303030303030303030303030127C0A570A4F0A282F65746865726D696E742E63727970746F2E76312E657468736563703235366B312E5075624B657912230A21039F4E693730F116E7AB01DAC46B94AD4FCABC3CA7D91A6B121CC26782A8F2B8B212040A02080112210A1B0A07617465766D6F7312103236323530303030303030303030303010A8B4061A0C65766D6F735F393030302D3420DEC08D01").unwrap();
        let result = ParsedCosmosTx::build(&raw_tx, DataType::Direct).unwrap();
        let overview = result.overview;
        assert_eq!("Evmos Testnet", overview.common.network);
        assert_eq!(CosmosTxDisplayType::Send, overview.display_type);
        match overview.kind[0].clone() {
            MsgOverview::Send(overview) => {
                assert_eq!("10000000000000000 atevmos", overview.value);
                assert_eq!(
                    "evmos1tqsdz785sqjnlggee0lwxjwfk6dl36ae2uf9er",
                    overview.from
                );
                assert_eq!("evmos1lczqw94vftam5z8wwgln7374mq20cjxpg98p9f", overview.to);
            }
            _ => {
                panic!("program overview parse error!")
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Evmos Testnet",
                "Chain ID": "evmos_9000-4",
                "Fee": "2625000000000000 atevmos",
                "Max Fee": "275625000000000000000 atevmos",
                "Gas Limit": "105000",
            },
            "kind": [
                {
                    "Method": "Send",
                    "Value": "10000000000000000 atevmos",
                    "From": "evmos1tqsdz785sqjnlggee0lwxjwfk6dl36ae2uf9er",
                    "To": "evmos1lczqw94vftam5z8wwgln7374mq20cjxpg98p9f"
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_delegate_direct() {
        //  Object {"account_number": Number(2318430), "Chain ID": String("evmos_9000-4"), "Fee": Object {"amount": Array [Object {"amount": String("8750000000000000"), "denom": String("atevmos")}], "gas": Number(350000), "granter": String(""), "payer": String("")}, "memo": String(""), "msgs": Array [Object {"type": String("/cosmos.staking.v1beta1.MsgDelegate"), "Value": Object {"amount": Object {"amount": String("10000000000000000"), "denom": String("atevmos")}, "delegator_address": String("evmos1tqsdz785sqjnlggee0lwxjwfk6dl36ae2uf9er"), "validator_address": String("evmosvaloper10t6kyy4jncvnevmgq6q2ntcy90gse3yxa7x2p4")}}]}
        let raw_tx = "0AAC010AA9010A232F636F736D6F732E7374616B696E672E763162657461312E4D736744656C65676174651281010A2C65766D6F7331747173647A37383573716A6E6C67676565306C77786A77666B36646C33366165327566396572123365766D6F7376616C6F706572313074366B7979346A6E63766E65766D67713671326E74637939306773653379786137783270341A1C0A07617465766D6F7312113130303030303030303030303030303030127C0A570A4F0A282F65746865726D696E742E63727970746F2E76312E657468736563703235366B312E5075624B657912230A21039F4E693730F116E7AB01DAC46B94AD4FCABC3CA7D91A6B121CC26782A8F2B8B212040A02080112210A1B0A07617465766D6F7312103837353030303030303030303030303010B0AE151A0C65766D6F735F393030302D3420DEC08D01";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Direct)
                .unwrap();
        let overview = result.overview;
        assert_eq!("Evmos Testnet", overview.common.network);
        assert_eq!(CosmosTxDisplayType::Delegate, overview.display_type);
        match overview.kind[0].clone() {
            MsgOverview::Delegate(overview) => {
                assert_eq!("10000000000000000 atevmos", overview.value);
                assert_eq!(
                    "evmos1tqsdz785sqjnlggee0lwxjwfk6dl36ae2uf9er",
                    overview.from
                );
                assert_eq!(
                    "evmosvaloper10t6kyy4jncvnevmgq6q2ntcy90gse3yxa7x2p4",
                    overview.to
                );
            }
            _ => {
                panic!("program overview parse error!")
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Evmos Testnet",
                "Chain ID": "evmos_9000-4",
                "Fee": "8750000000000000 atevmos",
                "Gas Limit": "350000",
                "Max Fee": "3062500000000000000000 atevmos",
            },
            "kind": [
                {
                    "Method": "Delegate",
                    "Value": "10000000000000000 atevmos",
                    "From": "evmos1tqsdz785sqjnlggee0lwxjwfk6dl36ae2uf9er",
                    "To": "evmosvaloper10t6kyy4jncvnevmgq6q2ntcy90gse3yxa7x2p4"
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_delegate_amino() {
        let raw_tx = "7b226163636f756e745f6e756d626572223a2237363431222c22636861696e5f6964223a226f736d6f2d746573742d35222c22666565223a7b22616d6f756e74223a5b7b22616d6f756e74223a2234363235222c2264656e6f6d223a22756f736d6f227d5d2c22676173223a22313834393931227d2c226d656d6f223a22222c226d736773223a5b7b2274797065223a22636f736d6f732d73646b2f4d736744656c6567617465222c2276616c7565223a7b22616d6f756e74223a7b22616d6f756e74223a2232303030303030222c2264656e6f6d223a22756f736d6f227d2c2264656c656761746f725f61646472657373223a226f736d6f3137753032663830766b61666e65396c61347779706478336b78787878776d36667a6d63677963222c2276616c696461746f725f61646472657373223a226f736d6f76616c6f7065723168683067357866323365357a656b673435636d65726339376873346e32303034647932743236227d7d5d2c2273657175656e6365223a2230227d";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Amino).unwrap();
        let overview = result.overview;
        assert_eq!("Cosmos Hub", overview.common.network);
        assert_eq!(CosmosTxDisplayType::Delegate, overview.display_type);
        match overview.kind[0].clone() {
            MsgOverview::Delegate(overview) => {
                assert_eq!("2000000 uosmo", overview.value);
                assert_eq!("osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc", overview.from);
                assert_eq!(
                    "osmovaloper1hh0g5xf23e5zekg45cmerc97hs4n2004dy2t26",
                    overview.to
                );
            }
            _ => {
                panic!("program overview parse error!")
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Cosmos Hub",
                "Chain ID": "osmo-test-5",
                "Fee": "4625 uosmo",
                "Gas Limit": "184991",
                "Max Fee": "855583375 uosmo",
            },
            "kind": [
                {
                    "Method": "Delegate",
                    "Value": "2000000 uosmo",
                    "From": "osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc",
                    "To": "osmovaloper1hh0g5xf23e5zekg45cmerc97hs4n2004dy2t26"
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_undelegate_direct() {
        let raw_tx = "0AAE010AAB010A252F636F736D6F732E7374616B696E672E763162657461312E4D7367556E64656C65676174651281010A2C65766D6F7331747173647A37383573716A6E6C67676565306C77786A77666B36646C33366165327566396572123365766D6F7376616C6F706572316668666B6B6C6D76373634723375366B716B65666C6D64647236726C7A68663472396C6165361A1C0A07617465766D6F7312113334303030303030303030303030303034127C0A570A4F0A282F65746865726D696E742E63727970746F2E76312E657468736563703235366B312E5075624B657912230A21039F4E693730F116E7AB01DAC46B94AD4FCABC3CA7D91A6B121CC26782A8F2B8B212040A02080112210A1B0A07617465766D6F7312103735303030303030303030303030303010E0A7121A0C65766D6F735F393030302D3420DEC08D01";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Direct)
                .unwrap();
        let overview = result.overview;
        assert_eq!("Evmos Testnet", overview.common.network);
        assert_eq!(CosmosTxDisplayType::Undelegate, overview.display_type);
        match overview.kind[0].clone() {
            MsgOverview::Undelegate(overview) => {
                assert_eq!("34000000000000004 atevmos", overview.value);
                assert_eq!("evmos1tqsdz785sqjnlggee0lwxjwfk6dl36ae2uf9er", overview.to);
                assert_eq!(
                    "evmosvaloper1fhfkklmv764r3u6kqkeflmddr6rlzhf4r9lae6",
                    overview.validator
                );
            }
            _ => {
                panic!("program overview parse error!")
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Evmos Testnet",
                "Chain ID": "evmos_9000-4",
                "Fee": "7500000000000000 atevmos",
                "Max Fee": "2250000000000000000000 atevmos",
                "Gas Limit": "300000"
            },
            "kind": [
                {
                    "Method": "Undelegate",
                    "Value": "34000000000000004 atevmos",
                    "To": "evmos1tqsdz785sqjnlggee0lwxjwfk6dl36ae2uf9er",
                    "Validator": "evmosvaloper1fhfkklmv764r3u6kqkeflmddr6rlzhf4r9lae6"
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_undelegate_amino() {
        let raw_tx = "7B226163636F756E745F6E756D626572223A2237363431222C22636861696E5F6964223A226F736D6F2D746573742D35222C22666565223A7B22616D6F756E74223A5B7B22616D6F756E74223A2239353132222C2264656E6F6D223A22756F736D6F227D5D2C22676173223A22323337373838227D2C226D656D6F223A22222C226D736773223A5B7B2274797065223A22636F736D6F732D73646B2F4D7367556E64656C6567617465222C2276616C7565223A7B22616D6F756E74223A7B22616D6F756E74223A2232303030303030222C2264656E6F6D223A22756F736D6F227D2C2264656C656761746F725F61646472657373223A226F736D6F3137753032663830766B61666E65396C61347779706478336B78787878776D36667A6D63677963222C2276616C696461746F725F61646472657373223A226F736D6F76616C6F7065723168683067357866323365357A656B673435636D65726339376873346E32303034647932743236227D7D5D2C2273657175656E6365223A2231227D";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Amino).unwrap();
        let overview = result.overview;
        assert_eq!("Cosmos Hub", overview.common.network);
        assert_eq!(CosmosTxDisplayType::Undelegate, overview.display_type);
        match overview.kind[0].clone() {
            MsgOverview::Undelegate(overview) => {
                assert_eq!("2000000 uosmo", overview.value);
                assert_eq!("osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc", overview.to);
                assert_eq!(
                    "osmovaloper1hh0g5xf23e5zekg45cmerc97hs4n2004dy2t26",
                    overview.validator
                );
            }
            _ => {
                panic!("program overview parse error!")
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Cosmos Hub",
                "Chain ID": "osmo-test-5",
                "Fee": "9512 uosmo",
                "Max Fee": "2261839456 uosmo",
                "Gas Limit": "237788"
            },
            "kind": [
                {
                    "Method": "Undelegate",
                    "Value": "2000000 uosmo",
                    "To": "osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc",
                    "Validator": "osmovaloper1hh0g5xf23e5zekg45cmerc97hs4n2004dy2t26"
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_redelegate_direct() {
        let raw_tx = "0AE8010AE5010A2A2F636F736D6F732E7374616B696E672E763162657461312E4D7367426567696E526564656C656761746512B6010A2C65766D6F7331747173647A37383573716A6E6C67676565306C77786A77666B36646C33366165327566396572123365766D6F7376616C6F7065723135387777617334763666676375327833706C67373073367530666D306C6C653233376B6C74721A3365766D6F7376616C6F7065723173393564776E773776756B73363838746E7A75756530686C366773637A6D7A6A6C663735397A221C0A07617465766D6F7312113235303030303030303030303030303030127D0A570A4F0A282F65746865726D696E742E63727970746F2E76312E657468736563703235366B312E5075624B657912230A21039F4E693730F116E7AB01DAC46B94AD4FCABC3CA7D91A6B121CC26782A8F2B8B212040A02080112220A1C0A07617465766D6F731211333735303030303030303030303030303010E0C65B1A0C65766D6F735F393030302D3420DEC08D01";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Direct)
                .unwrap();
        let overview = result.overview;
        assert_eq!("Evmos Testnet", overview.common.network);
        assert_eq!(CosmosTxDisplayType::Redelegate, overview.display_type);
        match overview.kind[0].clone() {
            MsgOverview::Redelegate(overview) => {
                assert_eq!("25000000000000000 atevmos", overview.value);
                assert_eq!("evmos1tqsdz785sqjnlggee0lwxjwfk6dl36ae2uf9er", overview.to);
                assert_eq!(
                    "evmosvaloper1s95dwnw7vuks688tnzuue0hl6gsczmzjlf759z",
                    overview.new_validator
                );
            }
            _ => {
                panic!("program overview parse error!")
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Evmos Testnet",
                "Chain ID": "evmos_9000-4",
                "Fee": "37500000000000000 atevmos",
                "Max Fee": "56250000000000000000000 atevmos",
                "Gas Limit": "1500000"
            },
            "kind": [
                {
                    "Method": "Re-delegate",
                    "Value": "25000000000000000 atevmos",
                    "To": "evmos1tqsdz785sqjnlggee0lwxjwfk6dl36ae2uf9er",
                    "Old Validator": "evmosvaloper158wwas4v6fgcu2x3plg70s6u0fm0lle237kltr",
                    "New Validator": "evmosvaloper1s95dwnw7vuks688tnzuue0hl6gsczmzjlf759z"
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_redelegate_amino() {
        let raw_tx = "7B226163636F756E745F6E756D626572223A2237363431222C22636861696E5F6964223A226F736D6F2D746573742D35222C22666565223A7B22616D6F756E74223A5B7B22616D6F756E74223A2238313634222C2264656E6F6D223A22756F736D6F227D5D2C22676173223A22333236353539227D2C226D656D6F223A22222C226D736773223A5B7B2274797065223A22636F736D6F732D73646B2F4D7367426567696E526564656C6567617465222C2276616C7565223A7B22616D6F756E74223A7B22616D6F756E74223A2232303030303030222C2264656E6F6D223A22756F736D6F227D2C2264656C656761746F725F61646472657373223A226F736D6F3137753032663830766B61666E65396C61347779706478336B78787878776D36667A6D63677963222C2276616C696461746F725F6473745F61646472657373223A226F736D6F76616C6F7065723176617130746E657130766D6E6B6B34386A7872716C616165666478386B6C3274783036656739222C2276616C696461746F725F7372635F61646472657373223A226F736D6F76616C6F7065723168683067357866323365357A656B673435636D65726339376873346E32303034647932743236227D7D5D2C2273657175656E6365223A2231227D";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Amino).unwrap();
        let overview = result.overview;
        assert_eq!("Cosmos Hub", overview.common.network);
        assert_eq!(CosmosTxDisplayType::Redelegate, overview.display_type);
        match overview.kind[0].clone() {
            MsgOverview::Redelegate(overview) => {
                assert_eq!("2000000 uosmo", overview.value);
                assert_eq!("osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc", overview.to);
                assert_eq!(
                    "osmovaloper1vaq0tneq0vmnkk48jxrqlaaefdx8kl2tx06eg9",
                    overview.new_validator
                );
            }
            _ => {
                panic!("program overview parse error!")
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Cosmos Hub",
                "Chain ID": "osmo-test-5",
                "Fee": "8164 uosmo",
                "Gas Limit": "326559",
                "Max Fee": "2666027676 uosmo"
            },
            "kind": [
                {
                    "Method": "Re-delegate",
                    "Value": "2000000 uosmo",
                    "To": "osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc",
                    "Old Validator": "osmovaloper1hh0g5xf23e5zekg45cmerc97hs4n2004dy2t26",
                    "New Validator": "osmovaloper1vaq0tneq0vmnkk48jxrqlaaefdx8kl2tx06eg9"
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_withdraw_reward_direct() {
        let raw_tx = "0AA1010A9E010A372F636F736D6F732E646973747269627574696F6E2E763162657461312E4D7367576974686472617744656C656761746F7252657761726412630A2C65766D6F7331747173647A37383573716A6E6C67676565306C77786A77666B36646C33366165327566396572123365766D6F7376616C6F7065723135387777617334763666676375327833706C67373073367530666D306C6C653233376B6C7472127C0A570A4F0A282F65746865726D696E742E63727970746F2E76312E657468736563703235366B312E5075624B657912230A21039F4E693730F116E7AB01DAC46B94AD4FCABC3CA7D91A6B121CC26782A8F2B8B212040A02080112210A1B0A07617465766D6F7312103837353030303030303030303030303010B0AE151A0C65766D6F735F393030302D3420DEC08D01";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Direct)
                .unwrap();
        let overview = result.overview;
        assert_eq!("Evmos Testnet", overview.common.network);
        assert_eq!(CosmosTxDisplayType::WithdrawReward, overview.display_type);
        match overview.kind[0].clone() {
            MsgOverview::WithdrawReward(overview) => {
                assert_eq!("evmos1tqsdz785sqjnlggee0lwxjwfk6dl36ae2uf9er", overview.to);
                assert_eq!(
                    "evmosvaloper158wwas4v6fgcu2x3plg70s6u0fm0lle237kltr",
                    overview.validator
                );
            }
            _ => {
                panic!("program overview parse error!")
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Evmos Testnet",
                "Chain ID": "evmos_9000-4",
                "Fee": "8750000000000000 atevmos",
                "Max Fee": "3062500000000000000000 atevmos",
                "Gas Limit": "350000"
            },
            "kind": [
                {
                    "Method": "Withdraw Reward",
                    "To": "evmos1tqsdz785sqjnlggee0lwxjwfk6dl36ae2uf9er",
                    "Validator": "evmosvaloper158wwas4v6fgcu2x3plg70s6u0fm0lle237kltr"
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_withdraw_reward_amino() {
        let raw_tx = "7b226163636f756e745f6e756d626572223a2230222c22636861696e5f6964223a2265766d6f735f393030302d34222c22666565223a7b22616d6f756e74223a5b7b22616d6f756e74223a22313030222c2264656e6f6d223a2275636f736d227d5d2c22676173223a22323530227d2c226d656d6f223a22536f6d65206d656d6f222c226d736773223a5b7b2274797065223a22636f736d6f732d73646b2f4d7367576974686472617744656c65676174696f6e526577617264222c2276616c7565223a7b2264656c656761746f725f61646472657373223a22636f736d6f7331706b707472653766646b6c366766727a6c65736a6a766878686c63337234676d6d6b38727336222c2276616c696461746f725f61646472657373223a22636f736d6f733130647972393839396736743070656c6577346e7666346a3563336a6367763072373371676135227d7d5d2c2273657175656e6365223a2230227d";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Amino).unwrap();
        let overview = result.overview;
        assert_eq!("Evmos Testnet", overview.common.network);
        assert_eq!(CosmosTxDisplayType::WithdrawReward, overview.display_type);
        match overview.kind[0].clone() {
            MsgOverview::WithdrawReward(overview) => {
                assert_eq!("cosmos1pkptre7fdkl6gfrzlesjjvhxhlc3r4gmmk8rs6", overview.to);
                assert_eq!(
                    "cosmos10dyr9899g6t0pelew4nvf4j5c3jcgv0r73qga5",
                    overview.validator
                );
            }
            _ => {
                panic!("program overview parse error!")
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Evmos Testnet",
                "Chain ID": "evmos_9000-4",
                "Fee": "100 ucosm",
                "Max Fee": "25000 ucosm",
                "Gas Limit": "250"
            },
            "kind": [
                {
                    "Method": "Withdraw Reward",
                    "To": "cosmos1pkptre7fdkl6gfrzlesjjvhxhlc3r4gmmk8rs6",
                    "Validator": "cosmos10dyr9899g6t0pelew4nvf4j5c3jcgv0r73qga5"
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_multiple_amino() {
        let raw_tx = "7b226163636f756e745f6e756d626572223a2230222c22636861696e5f6964223a2265766d6f735f393030302d34222c22666565223a7b22616d6f756e74223a5b7b22616d6f756e74223a22313030222c2264656e6f6d223a2275636f736d227d5d2c22676173223a22323530227d2c226d656d6f223a22536f6d65206d656d6f222c226d736773223a5b7b2274797065223a222f636f736d6f732e646973747269627574696f6e2e763162657461312e4d7367576974686472617744656c656761746f72526577617264222c2276616c7565223a7b2264656c656761746f725f61646472657373223a22636f736d6f73316d336d6539683566326a7478787a79377263726c36656b3938753737386c7376786368726366222c2276616c696461746f725f61646472657373223a22636f736d6f7376616c6f706572317074797a65776e6e73326b6e33376577746d76367070737668646e6d65617076746663397935227d7d2c7b2274797065223a222f636f736d6f732e646973747269627574696f6e2e763162657461312e4d7367576974686472617744656c656761746f72526577617264222c2276616c7565223a7b2264656c656761746f725f61646472657373223a22636f736d6f73316d336d6539683566326a7478787a79377263726c36656b3938753737386c7376786368726366222c2276616c696461746f725f61646472657373223a22636f736d6f7376616c6f706572317074797a65776e6e73326b6e33376577746d76367070737668646e6d65617076746663397935227d7d5d2c2273657175656e6365223a2230227d";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Amino).unwrap();
        let overview = result.overview;
        assert_eq!("Evmos Testnet", overview.common.network);
        assert_eq!(CosmosTxDisplayType::Multiple, overview.display_type);
        for each in overview.kind.clone() {
            match each {
                MsgOverview::WithdrawReward(overview) => {
                    assert_eq!("cosmos1m3me9h5f2jtxxzy7rcrl6ek98u778lsvxchrcf", overview.to);
                    assert_eq!(
                        "cosmosvaloper1ptyzewnns2kn37ewtmv6ppsvhdnmeapvtfc9y5",
                        overview.validator
                    );
                }
                _ => {
                    panic!("program overview parse error!")
                }
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Evmos Testnet",
                "Chain ID": "evmos_9000-4",
                "Fee": "100 ucosm",
                "Max Fee": "25000 ucosm",
                "Gas Limit": "250"
            },
            "kind": [
                {
                    "Method": "Withdraw Reward",
                    "To": "cosmos1m3me9h5f2jtxxzy7rcrl6ek98u778lsvxchrcf",
                    "Validator": "cosmosvaloper1ptyzewnns2kn37ewtmv6ppsvhdnmeapvtfc9y5"
                },
                {
                    "Method": "Withdraw Reward",
                    "To": "cosmos1m3me9h5f2jtxxzy7rcrl6ek98u778lsvxchrcf",
                    "Validator": "cosmosvaloper1ptyzewnns2kn37ewtmv6ppsvhdnmeapvtfc9y5"
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_amino_transfer() {
        let raw_tx = "7B226163636F756E745F6E756D626572223A22383435393638222C22636861696E5F6964223A226F736D6F7369732D31222C22666565223A7B22616D6F756E74223A5B7B22616D6F756E74223A2235333333222C2264656E6F6D223A22756F736D6F227D5D2C22676173223A22323133333035227D2C226D656D6F223A22222C226D736773223A5B7B2274797065223A22636F736D6F732D73646B2F4D73675472616E73666572222C2276616C7565223A7B227265636569766572223A22636F736D6F733130647972393839396736743070656C6577346E7666346A3563336A6367763072373371676135222C2273656E646572223A226F736D6F3137753032663830766B61666E65396C61347779706478336B78787878776D36667A6D63677963222C22736F757263655F6368616E6E656C223A226368616E6E656C2D30222C22736F757263655F706F7274223A227472616E73666572222C2274696D656F75745F686569676874223A7B227265766973696F6E5F686569676874223A223136323134333538222C227265766973696F6E5F6E756D626572223A2234227D2C22746F6B656E223A7B22616D6F756E74223A22333030303030222C2264656E6F6D223A22756F736D6F227D7D7D5D2C2273657175656E6365223A2232227D";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Amino).unwrap();
        let overview = result.overview;
        assert_eq!("Osmosis", overview.common.network);
        assert_eq!(CosmosTxDisplayType::Transfer, overview.display_type);
        match overview.kind[0].clone() {
            MsgOverview::Transfer(overview) => {
                assert_eq!("osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc", overview.from);
                assert_eq!("cosmos10dyr9899g6t0pelew4nvf4j5c3jcgv0r73qga5", overview.to);
                assert_eq!("300000 uosmo", overview.value);
            }
            _ => {
                panic!("program overview parse error!")
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Osmosis",
                "Chain ID": "osmosis-1",
                "Fee": "5333 uosmo",
                "Max Fee": "1137555565 uosmo",
                "Gas Limit": "213305"
            },
            "kind": [
                {
                    "Method": "IBC Transfer",
                    "To": "cosmos10dyr9899g6t0pelew4nvf4j5c3jcgv0r73qga5",
                    "From": "osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc",
                    "Value": "300000 uosmo",
                    "Source Channel": "channel-0"
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_amino_transfer_empty_revision_number() {
        let raw_tx = "7B226163636F756E745F6E756D626572223A22383435393638222C22636861696E5F6964223A226F736D6F7369732D31222C22666565223A7B22616D6F756E74223A5B7B22616D6F756E74223A22373738222C2264656E6F6D223A22756F736D6F227D5D2C22676173223A22323439343038227D2C226D656D6F223A22222C226D736773223A5B7B2274797065223A22636F736D6F732D73646B2F4D73675472616E73666572222C2276616C7565223A7B227265636569766572223A2263656C65737469613137753032663830766B61666E65396C61347779706478336B78787878776D36666D3236676738222C2273656E646572223A226F736D6F3137753032663830766B61666E65396C61347779706478336B78787878776D36667A6D63677963222C22736F757263655F6368616E6E656C223A226368616E6E656C2D36393934222C22736F757263655F706F7274223A227472616E73666572222C2274696D656F75745F686569676874223A7B227265766973696F6E5F686569676874223A22333039333237227D2C22746F6B656E223A7B22616D6F756E74223A223130303030222C2264656E6F6D223A226962632F44373945374438334142333939424646463933343333453534464141343830433139313234384643353536393234413241383335314145323633384233383737227D7D7D5D2C2273657175656E6365223A223339227D";
        let result = ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Amino);
        assert!(result.is_ok());
    }

    #[test]
    fn test_parse_cosmos_direct_transfer() {
        let raw_tx = "0ac8010ac5010a292f6962632e6170706c69636174696f6e732e7472616e736665722e76312e4d73675472616e736665721297010a087472616e73666572120b6368616e6e656c2d3139321a100a057561746f6d120738303336323936222d636f736d6f7331666a39666a6a776470376b763768776a723465726763337468717779397474726a73636578722a2a73696631666a39666a6a776470376b763768776a723465726763337468717779397474726864683066673207080110caca970638d5b4b5cffd85d4b917127d0a510a460a1f2f636f736d6f732e63727970746f2e736563703235366b312e5075624b657912230a210280abfddd08b1ccb49d599356cf92bc6e70b30a6383660f83b51265692a7ccafc12040a02080118d23d12280a0d0a05756f736d6f12043230303010b9601a086665655061796572220a6665654772616e7465721a096f736d6f7369732d3120b42c";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Direct)
                .unwrap();
        let overview = result.overview;
        assert_eq!("Osmosis", overview.common.network);
        assert_eq!(CosmosTxDisplayType::Transfer, overview.display_type);
        match overview.kind[0].clone() {
            MsgOverview::Transfer(overview) => {
                assert_eq!(
                    "cosmos1fj9fjjwdp7kv7hwjr4ergc3thqwy9ttrjscexr",
                    overview.from
                );
                assert_eq!("sif1fj9fjjwdp7kv7hwjr4ergc3thqwy9ttrhdh0fg", overview.to);
                assert_eq!("8.036296 ATOM", overview.value);
            }
            _ => {
                panic!("program overview parse error!")
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Osmosis",
                "Chain ID": "osmosis-1",
                "Fee": "2000 uosmo",
                "Max Fee": "24690000 uosmo",
                "Gas Limit": "12345"
            },
            "kind": [
                {
                    "Method": "IBC Transfer",
                    "To": "sif1fj9fjjwdp7kv7hwjr4ergc3thqwy9ttrhdh0fg",
                    "From": "cosmos1fj9fjjwdp7kv7hwjr4ergc3thqwy9ttrjscexr",
                    "Value": "8.036296 ATOM",
                    "Source Channel": "channel-192"
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_amino_vote() {
        let raw_tx = "7B226163636F756E745F6E756D626572223A22383435393638222C22636861696E5F6964223A226F736D6F7369732D31222C22666565223A7B22616D6F756E74223A5B7B22616D6F756E74223A2231393436222C2264656E6F6D223A22756F736D6F227D5D2C22676173223A223737383134227D2C226D656D6F223A22222C226D736773223A5B7B2274797065223A22636F736D6F732D73646B2F4D7367566F7465222C2276616C7565223A7B226F7074696F6E223A342C2270726F706F73616C5F6964223A22353635222C22766F746572223A226F736D6F3137753032663830766B61666E65396C61347779706478336B78787878776D36667A6D63677963227D7D5D2C2273657175656E6365223A2232227D";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Amino).unwrap();
        let overview = result.overview;
        assert_eq!("Osmosis", overview.common.network);
        assert_eq!(CosmosTxDisplayType::Vote, overview.display_type);
        match overview.kind[0].clone() {
            MsgOverview::Vote(overview) => {
                assert_eq!("NO_WITH_VETO", overview.voted);
                assert_eq!("#565", overview.proposal);
                assert_eq!(
                    "osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc",
                    overview.voter
                );
            }
            _ => {
                panic!("program overview parse error!")
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Osmosis",
                "Chain ID": "osmosis-1",
                "Fee": "1946 uosmo",
                "Max Fee": "151426044 uosmo",
                "Gas Limit": "77814"
            },
            "kind": [
                {
                    "Method": "Vote",
                    "Voter": "osmo17u02f80vkafne9la4wypdx3kxxxxwm6fzmcgyc",
                    "Proposal": "#565",
                    "Voted": "NO_WITH_VETO",
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_vote_direct() {
        let raw_tx = "0a330a310a1b2f636f736d6f732e676f762e763162657461312e4d7367566f74651212085f120c6465736d6f7331766f7465721804127d0a510a460a1f2f636f736d6f732e63727970746f2e736563703235366b312e5075624b657912230a210280abfddd08b1ccb49d599356cf92bc6e70b30a6383660f83b51265692a7ccafc12040a02080118d23d12280a0d0a05756f736d6f12043230303010b9601a086665655061796572220a6665654772616e7465721a096f736d6f7369732d3120b42c";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Direct)
                .unwrap();
        let overview = result.overview;
        assert_eq!("Osmosis", overview.common.network);
        assert_eq!(CosmosTxDisplayType::Vote, overview.display_type);
        match overview.kind[0].clone() {
            MsgOverview::Vote(overview) => {
                assert_eq!("NO_WITH_VETO", overview.voted);
                assert_eq!("#95", overview.proposal);
                assert_eq!("desmos1voter", overview.voter);
            }
            _ => {
                panic!("program overview parse error!")
            }
        }
        let expected_detail = json!({
            "common": {
                "Network": "Osmosis",
                "Chain ID": "osmosis-1",
                "Fee": "2000 uosmo",
                "Max Fee": "24690000 uosmo",
                "Gas Limit": "12345"
            },
            "kind": [
                {
                    "Method": "Vote",
                    "Voter": "desmos1voter",
                    "Proposal": "#95",
                    "Voted": "NO_WITH_VETO",
                }
            ]
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_unknown_amino() {
        let raw_tx = "7b226163636f756e745f6e756d626572223a2230222c22636861696e5f6964223a2265766d6f735f393030302d34222c22666565223a7b22616d6f756e74223a5b7b22616d6f756e74223a22313030222c2264656e6f6d223a2275636f736d227d5d2c22676173223a22323530227d2c226d656d6f223a22536f6d65206d656d6f222c226d736773223a5b7b2274797065223a222f6962632e636f72652e6368616e6e656c2e76312e4d736741636b6e6f776c656467656d656e74222c2276616c7565223a7b2261636b6e6f776c656467656d656e74223a2265794a795a584e31624851694f694a4255543039496e303d222c227061636b6574223a7b2264617461223a2265794a32595778705a47463062334a666458426b5958526c6379493657337369634856695832746c6553493665794a6c5a4449314e544535496a6f694e6c70545957566154564a6b596c565253305a4a5356526d616d4e545630524b4e32784a536b3556576e6c6a626a5530656c645057557468597a306966537769634739335a5849694f6949314e5445794e6a676966563073496e5a6862484e6c644639316347526864475666615751694f6949784e7a51304d54453549697769633278686332686659574e726379493657313139222c2264657374696e6174696f6e5f6368616e6e656c223a226368616e6e656c2d30222c2264657374696e6174696f6e5f706f7274223a22636f6e73756d6572222c2273657175656e6365223a22333236313533222c22736f757263655f6368616e6e656c223a226368616e6e656c2d353638222c22736f757263655f706f7274223a2270726f7669646572222c2274696d656f75745f686569676874223a7b227265766973696f6e5f686569676874223a333030302c227265766973696f6e5f6e756d626572223a327d2c2274696d656f75745f74696d657374616d70223a313638393735313434363837332c22746f6b656e223a7b22616d6f756e74223a2231303030303030303030222c2264656e6f6d223a22756461726963227d7d2c2270726f6f665f61636b6564223a22436f6b4a436f594a436a64685932747a4c334276636e527a4c324e76626e4e31625756794c324e6f595735755a57787a4c324e6f595735755a5777744d43397a5a5846315a57356a5a584d764d7a49324d54557a456941493931562b3152676d2f686a5952524b2f4a4f7831414237627279456a70486666637143703832514b66426f4f43414559415341424b67594141723645785145694c41674245696743424c364578514567392f325949543133455932427744314a77473438343759755838636f5771514d4a6a6f6e317a6b78632b6b67496977494152496f4241612b684d55424948475168683467304135652b756158697653444f32694e7674523065494f5a684e64447a796b49556f4f7849434973434145534b41594b766f544641534275644747334d332b44714670676246466b416e30397a4371425459624d342f4267635065617732617a345341694c67674245676349474c364578514567476945676165645a39466f6864303273763745484a583863744c32765171635843314d50734868692b5153546d454d694c6767424567634b4b4c364578514567476945674744727971616c4569595756782b6161655056466b615576515253614535654d392b744c70756657696e59694c6767424567634d507236457851456747694567734e71542b5030684862327241435578612f5373764b6d744a755246574a6f58475a4b61614a52444c4d41694c7767424567674f6741472b684d554249426f6849454d4e785a664c395a2b627251444779426f70784c4971334f694d744449533161335652574c68384b3439496930494152497045504142766f544641534479736a4b435579786742716149734b6d6839743554623232457a4339483267754d2f45574249706170716941694c7767424567675339674b2b684d554249426f68494e2b657151684c495a5338724c344239554d323378746234416e4a5a7447436d565a5754693538447659534969384941524949464d7748766f544641534161495343697a52612b6178707a6b4e496355776348526650543943306255452b6e4968536a5559594e4947536438534974434145534b5261304437364578514567624a6d4832416d767362534e466a3470674b454f7133367849354d45326c5a3065346d7762516a79555963674969304941524970474e6761766f5446415344574c4474506c414537342f4f65613737756633483538396c6743394f38674855592b526d504278474f5a6941694c51674245696b6136444b2b684d5542494f5661506864786f6434686c5055536858646b2f314c554358485371773744756c2f58564541572b2b396849434974434145534b527a475462364578514567744771342f3866694e4d66324a446f4e4261574e642b722f3936714c4778693664724f707356635549325167496930494152497048714a34766f54464153424b386a79586c4f57443066503067313332442b366f6175754d452f2f3076532f7934365a2b784b64666e6941694c67674245696f672b704943766f544641534466547939354459667573376e422f544f4b6c3569666b4f7170506758506f7a647a475769634562396b4c6941694c67674245696f6972727344766f54464153444b793747417a546357764b45454e614747416245656a3936316c4359444d52333549684a512b586f42384341694d41674245676b6b6a704148766f544641534161495344696b79616a7a586f77762b6a58562b444c78626f7a4a45446c74415377626377764c4d31535a6b71694c434975434145534b69626f3467322b684d55424942434b6131445a4e5a41586e473373465a6a574c756f4957764b6d6e5659686b6e72683236736556685a3049434975434145534b696a6b6b52532b684d554249473164706a4474674f355933527462316d727848316e59455667736d506f6b55514f4f326f46562f574e4249434975434145534b69726373534b2b684d5542494e5138416f6954744c685472674e554876396367444f5a314e75514d57754c4a4e496c64466767447048354943497743414553435379753230362b684d554249426f68494d327747596f53666541624b3730716337355455636d79593858534e4263426c6e55547a3751377565526b4376774243766b4243674e70596d4d5349425541423946386945372b54702f657469637434617a6746553346305043485239646f5563636756306b4847676b494152674249414571415141694a5167424569454254683163566a734e73502f63756d2f4a6663444873547462584a773045316677694364575545665778726b694a77674245674542476943715a51514736673132343533555053366d7152342f3271484a435077687038706f35655973794246574f53496c43414553495146694c4561376a2b7745775655353679365036346a6a4b74644e457179747135664746573249653478624f69496c43414553495145313155517535375054624b346b5765476b4c555031695375647876475a72327863306e656c374247306279496e4341455341514561494a4c69457077636e66724a57413879794b583361316c624e3648464266597849356f5a75444f51684d456f222c2270726f6f665f686569676874223a7b227265766973696f6e5f686569676874223a2231363134313132222c227265766973696f6e5f6e756d626572223a2231227d2c227369676e6572223a22636f736d6f73316e6e61376b356c79776e393963643633656c6366716d36703863356334716375673461656635227d7d5d2c2273657175656e6365223a2230227d";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Amino).unwrap();
        let overview = result.overview;
        assert_eq!("Evmos Testnet", overview.common.network);
        assert_eq!(CosmosTxDisplayType::Unknown, overview.display_type);
        assert_eq!(0, overview.kind.len());
        let expected_detail = json!({
            "Network": "Evmos Testnet",
            "Chain ID": "evmos_9000-4",
            "Fee": "100 ucosm",
            "Gas Limit": "250",
            "Max Fee": "25000 ucosm",
            "Message": "Unknown Data"
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_cosmos_unknown_amino_1() {
        let raw_tx = "7B226163636F756E745F6E756D626572223A302C22636861696E5F6964223A2270756C7361722D32222C22666565223A5B5D2C226D656D6F223A22437265617465204B65706C722053656372657420656E6372797074696F6E206B65792E204F6E6C7920617070726F7665207265717565737473206279204B65706C722E222C226D736773223A5B5D2C2273657175656E6365223A307D";
        let result =
            ParsedCosmosTx::build(&hex::decode(raw_tx).unwrap().to_vec(), DataType::Amino).unwrap();
        let overview = result.overview;
        assert_eq!("Cosmos Hub", overview.common.network);
        assert_eq!(CosmosTxDisplayType::Unknown, overview.display_type);
        assert_eq!(0, overview.kind.len());
        let expected_detail = json!({
            "Network": "Cosmos Hub",
            "Chain ID": "pulsar-2",
            "Message": "Unknown Data"
        });
        let parsed_detail: Value = from_str(result.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }
}
