pub mod detail;
pub mod overview;
pub mod structs;

use crate::errors::{Result, SolanaError};
use crate::message::Message;
use crate::parser::detail::{
    CommonDetail, ProgramDetail, ProgramDetailGeneralUnknown, SolanaDetail,
};
use crate::parser::overview::{
    ProgramOverviewGeneral, ProgramOverviewTransfer, ProgramOverviewUnknown, ProgramOverviewVote,
    SolanaOverview,
};
use crate::parser::structs::{ParsedSolanaTx, SolanaTxDisplayType};
use crate::read::Read;
use alloc::borrow::ToOwned;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use serde_json::json;

impl ParsedSolanaTx {
    pub fn build(data: &Vec<u8>) -> Result<Self> {
        let message = Message::read(data.clone().to_vec().as_mut())?;
        let raw_details = message.to_program_details()?;
        let display_type = Self::detect_display_type(&raw_details);
        let parsed_overview = Self::build_overview(&display_type, &raw_details)?;
        let parsed_detail = Self::build_detail(&display_type, &raw_details, &message)?;
        Ok(Self {
            display_type,
            overview: parsed_overview,
            detail: parsed_detail,
            network: "Solana Mainnet".to_string(),
        })
    }

    fn detect_display_type(details: &[SolanaDetail]) -> SolanaTxDisplayType {
        let transfer: Vec<&SolanaDetail> = details
            .iter()
            .filter(|d| Self::is_system_transfer_detail(&d.common))
            .collect::<Vec<&SolanaDetail>>();
        if transfer.len() == 1 {
            return SolanaTxDisplayType::Transfer;
        }
        let vote: Vec<&SolanaDetail> = details
            .iter()
            .filter(|d| Self::is_vote_detail(&d.common))
            .collect::<Vec<&SolanaDetail>>();
        if vote.len() == 1 {
            return SolanaTxDisplayType::Vote;
        }
        let unknown: Vec<&SolanaDetail> = details
            .iter()
            .filter(|d| Self::is_unknown_detail(&d.common))
            .collect::<Vec<&SolanaDetail>>();
        if unknown.len() == details.len() {
            return SolanaTxDisplayType::Unknown;
        }
        SolanaTxDisplayType::General
    }
    fn is_system_transfer_detail(common: &CommonDetail) -> bool {
        common.program.eq("System") && common.method.eq("Transfer")
    }

    fn is_vote_detail(common: &CommonDetail) -> bool {
        common.program.eq("Vote") && common.method.eq("Vote")
    }

    fn is_unknown_detail(common: &CommonDetail) -> bool {
        common.program.eq("Unknown") && common.method.eq("")
    }
    fn build_genera_detail(details: &[SolanaDetail]) -> Result<String> {
        let parsed_detail = details
            .iter()
            .map(|d| {
                if Self::is_unknown_detail(&d.common) {
                    if let ProgramDetail::Unknown(v) = &d.kind {
                        return SolanaDetail {
                            common: d.common.clone(),
                            kind: ProgramDetail::GeneralUnknown(
                                ProgramDetailGeneralUnknown::from_unknown_detail(&v),
                            ),
                        };
                    }
                }
                d.clone()
            })
            .collect::<Vec<SolanaDetail>>();
        Ok(serde_json::to_string(&parsed_detail)?)
    }

    fn build_unknown_detail(details: &[SolanaDetail], message: &Message) -> Result<String> {
        let value = json!({
                    "header": {
                        "num_required_signatures": message.header.num_required_signatures,
                        "num_readonly_signed_accounts": message.header.num_readonly_signed_accounts,
                        "num_readonly_unsigned_accounts": message.header.num_readonly_unsigned_accounts,
                    },
                    "accounts": message.accounts.iter().map(|account| {third_party::base58::encode(&account.value)}).collect::<Vec<String>>(),
                    "block_hash": third_party::base58::encode(&message.block_hash.value),
                    "instructions": details,
                }).to_string();
        Ok(value)
    }

    fn build_detail(
        display_type: &SolanaTxDisplayType,
        details: &[SolanaDetail],
        message: &Message,
    ) -> Result<String> {
        match display_type {
            SolanaTxDisplayType::Unknown => Self::build_unknown_detail(details, message),
            SolanaTxDisplayType::General => Self::build_genera_detail(details),
            SolanaTxDisplayType::Transfer | SolanaTxDisplayType::Vote => {
                Ok(serde_json::to_string(&details)?)
            }
        }
    }

    fn build_transfer_overview(details: &[SolanaDetail]) -> Result<SolanaOverview> {
        let overview: Option<SolanaOverview> = details
            .iter()
            .find(|d| Self::is_system_transfer_detail(&d.common))
            .and_then(|detail| {
                if let ProgramDetail::SystemTransfer(v) = &detail.kind {
                    Some(SolanaOverview::Transfer(ProgramOverviewTransfer {
                        value: v.value.to_string(),
                        main_action: "SOL Transfer".to_string(),
                        from: v.from.to_string(),
                        to: v.to.to_string(),
                    }))
                } else {
                    None
                }
            });
        overview.ok_or(SolanaError::ParseTxError(
            "parse system transfer failed, empty transfer program".to_string(),
        ))
    }
    fn build_vote_overview(details: &[SolanaDetail]) -> Result<SolanaOverview> {
        let overview: Option<SolanaOverview> = details
            .iter()
            .find(|d| Self::is_vote_detail(&d.common))
            .and_then(|detail| {
                if let ProgramDetail::VoteVote(v) = &detail.kind {
                    Some(SolanaOverview::Vote(ProgramOverviewVote {
                        votes_on: v.slots.to_owned(),
                        main_action: "Vote".to_string(),
                        vote_account: v.vote_account.to_string(),
                    }))
                } else {
                    None
                }
            });
        overview.ok_or(SolanaError::ParseTxError(
            "parse vote failed, empty vote program".to_string(),
        ))
    }

    fn build_general_overview(details: &[SolanaDetail]) -> Result<SolanaOverview> {
        let mut overview = Vec::new();
        details.iter().for_each(|d| {
            if d.common.program != SolanaTxDisplayType::Unknown.to_string() {
                overview.push(ProgramOverviewGeneral {
                    program: d.common.program.to_string(),
                    method: d.common.method.to_string(),
                })
            }
        });
        Ok(SolanaOverview::General(overview))
    }
    fn build_overview(
        display_type: &SolanaTxDisplayType,
        details: &[SolanaDetail],
    ) -> Result<SolanaOverview> {
        match display_type {
            SolanaTxDisplayType::Transfer => Self::build_transfer_overview(details),
            SolanaTxDisplayType::Vote => Self::build_vote_overview(details),
            SolanaTxDisplayType::General => Self::build_general_overview(details),
            SolanaTxDisplayType::Unknown => {
                // all instructions can not be parsed.
                Ok(SolanaOverview::Unknown(ProgramOverviewUnknown::default()))
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::{json, Value};
    use third_party::hex::FromHex;

    #[test]
    fn test_parse_transaction_1() {
        // System.Transfer
        let data = "01000103876c762c4c83532f82966935ba1810659a96237028a2af6688dadecb0155ae071c7d0930a08193e702b0f24ebba96f179e9c186ef1208f98652ee775001744490000000000000000000000000000000000000000000000000000000000000000a7516fe1d3af3457fdc54e60856c0c3c87f4e5be3d10ffbc7a5cce8bf96792a101020200010c020000008813000000000000";
        let transaction = Vec::from_hex(data).unwrap();
        let parsed = ParsedSolanaTx::build(&transaction).unwrap();
        match parsed.overview {
            SolanaOverview::Transfer(overview) => {
                assert_eq!("0.000005 SOL".to_string(), overview.value);
                assert_eq!(
                    "A7dxsCbMy5ktZwQUgsQhVxsoJpx6wPAZYEcccQVjWnkE".to_string(),
                    overview.from
                );
                assert_eq!(
                    "2vCzt15qsXSCsf5k6t6QF9DiQSpE7kPTg3PdvFZtm2Tr".to_string(),
                    overview.to
                );
                assert_eq!("SOL Transfer".to_string(), overview.main_action);
            }
            _ => println!("program overview parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail = json!([
            {
                "program": "System",
                "method": "Transfer",
                "value": "0.000005 SOL",
                "from": "A7dxsCbMy5ktZwQUgsQhVxsoJpx6wPAZYEcccQVjWnkE",
                "to": "2vCzt15qsXSCsf5k6t6QF9DiQSpE7kPTg3PdvFZtm2Tr"
            }
        ]);
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_transaction_1_1() {
        // System.Transfer + Others
        // https://solscan.io/tx/3BH8wvsa13wS7pkByo4bfzpgvRZYfDijH3Jj9Rg8DzzX2nvk1jRo1zVpTh8PSdf9xFwmEJUFUSuU1L4pXHeSxBiD
        let data = "0301070a2877bcb8e7acfa950840ace015aee0f7c34f8da98cb47a0379a0848d1d146732295c04a089afe0f89e2eb1068d40b6dabc909d1c8fbd42a33d38388a9028952a0bfed77df018f67f348da938e5e96b9f66d05a542123fa3f9bebf7332df339cf000000000000000000000000000000000000000000000000000000000000000006a1d8179137542a983437bdfe2a7ab2557f535c8a78722b68a49dc00000000006a1d817a502050b680791e6ce6db88e1e5b7150f61fc6790a4eb4d10000000006a7d51718c774c928566398691d5eb68b5eb8a39b4b6d5c73555b210000000006a7d517192c5c51218cc94c3d4af17f58daee089ba1fd44e3dbd98a0000000006a7d517193584d0feed9bb3431d13206be544281b57b8566cc5375ff40000006dcf56df0c05417d5642726a2159a63bec115672bf8c57a694cf097e91a21c6f5cd38793d98807ac0a75e5553f78d37654a08385ecb7280ce9c7c905843164d305030200010c02000000af613239da0000000301010c08000000c800000000000000030101240100000006a1d8179137542a983437bdfe2a7ab2557f535c8a78722b68a49dc0000000000402010774000000000bfed77df018f67f348da938e5e96b9f66d05a542123fa3f9bebf7332df339cfc384e275e260429599cc2093f115fa55564dee44af077765b86417bd6967b37d00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000004060109060805020402000000";
        let transaction = Vec::from_hex(data).unwrap();
        let parsed = ParsedSolanaTx::build(&transaction).unwrap();
        match parsed.overview {
            SolanaOverview::Transfer(overview) => {
                assert_eq!("937.262473647 SOL".to_string(), overview.value);
                assert_eq!(
                    "3iyCqaBWcvHfEpN6NsdzjYpnAxVvioXTpjnPBLUwhva5".to_string(),
                    overview.from
                );
                assert_eq!(
                    "3nT6cqXNXyt3XieMKkyeGdPFcC1oXg5LHGvx3jJg6Fyj".to_string(),
                    overview.to
                );
                assert_eq!("SOL Transfer".to_string(), overview.main_action);
            }
            _ => println!("program overview parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail = json!([
            {
                "program": "System",
                "method": "Transfer",
                "value": "937.262473647 SOL",
                "from": "3iyCqaBWcvHfEpN6NsdzjYpnAxVvioXTpjnPBLUwhva5",
                "to": "3nT6cqXNXyt3XieMKkyeGdPFcC1oXg5LHGvx3jJg6Fyj"
            },
            {
                "program": "System",
                "method": "Allocate",
                "new_account": "3nT6cqXNXyt3XieMKkyeGdPFcC1oXg5LHGvx3jJg6Fyj",
                "space": "200"
            },
            {
                "program": "System",
                "method": "Assign",
                "account": "3nT6cqXNXyt3XieMKkyeGdPFcC1oXg5LHGvx3jJg6Fyj",
                "new_owner": "Stake11111111111111111111111111111111111111"
            },
            {
                "program": "Stake",
                "method": "Initialize",
                "stake_account": "3nT6cqXNXyt3XieMKkyeGdPFcC1oXg5LHGvx3jJg6Fyj",
                "sysvar_rent": "SysvarRent111111111111111111111111111111111",
                "staker": "opsM31pXd4cLvC3HoiUQhkTcU261GPtHKnKxRaGuto8",
                "withdrawer": "EAE2Sioy7PwkdGpGxaoSwiySzA5a7KFijKH7j6SF41rk",
                "timestamp": 0,
                "epoch": 0,
                "custodian": "11111111111111111111111111111111"
            },
            {
                "program": "Stake",
                "method": "DelegateStake",
                "stake_account": "3nT6cqXNXyt3XieMKkyeGdPFcC1oXg5LHGvx3jJg6Fyj",
                "vote_account": "8Pep3GmYiijRALqrMKpez92cxvF4YPTzoZg83uXh14pW",
                "sysvar_clock": "SysvarC1ock11111111111111111111111111111111",
                "sysvar_stake_history": "SysvarStakeHistory1111111111111111111111111",
                "config_account": "StakeConfig11111111111111111111111111111111",
                "stake_authority_pubkey": "opsM31pXd4cLvC3HoiUQhkTcU261GPtHKnKxRaGuto8"
            }
        ]);
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_transaction_2() {
        // Vote.Vote
        // {
        //   "details": {
        //     "hash": "9yoSeo4kEjumeyYFfohaCUYyjsu134xhXbrAxumCmysC",
        //     "slots": "142209632",
        //     "sysvar_clock": "SysvarC1ock11111111111111111111111111111111",
        //     "sysvar_slot_hashes": "SysvarS1otHashes111111111111111111111111111",
        //     "timestamp": "1658224090",
        //     "vote_account": "DHdYsTEsd1wGrmthR1ognfRXPkWBmxAWAv2pKdAix3HY",
        //     "vote_authority_pubkey": "D8izqaR979Fc2amDoGHmYqEugjckEi1RQL1Y1JKyHUwX"
        //   },
        //   "method_name": "Vote",
        //   "overview": {
        //     "hash": "9yoSeo4kEjumeyYFfohaCUYyjsu134xhXbrAxumCmysC",
        //     "slots": "142209632",
        //     "timestamp": "1658224090",
        //     "vote_account": "DHdYsTEsd1wGrmthR1ognfRXPkWBmxAWAv2pKdAix3HY"
        //   },
        //   "program_name": "Vote"
        // }
        let data = "01000305b446cb8fd7c225bf416df87c286710d75711af95222e41216da2177289cbbfa6b68edcd94d93de68614892bd165a94a6647aa040d87b9a042b41a009bdb469cf06a7d51718c774c928566398691d5eb68b5eb8a39b4b6d5c73555b210000000006a7d517192f0aafc6f265e3fb77cc7ada82c529d0be3b136e2d0055200000000761481d357474bb7c4d7624ebd3bdb3d8355e73d11043fc0da35380000000009254bd5e695fabf43f0ead6da730e88cf39bec6991c2c4374bcade97d0a73be7010404010302003d02000000010000000000000060f2790800000000856a887d33af1cd1723388576a7be8fa6d9c9c80c548495a24bf680c908812cf01da7dd66200000000";
        let transaction = Vec::from_hex(data).unwrap();
        let parsed = ParsedSolanaTx::build(&transaction).unwrap();
        match parsed.overview {
            SolanaOverview::Vote(overview) => {
                assert_eq!("Vote".to_string(), overview.main_action);
                assert_eq!(vec!["142209632"], overview.votes_on);
                assert_eq!(
                    "DHdYsTEsd1wGrmthR1ognfRXPkWBmxAWAv2pKdAix3HY".to_string(),
                    overview.vote_account
                );
            }
            _ => println!("program overview parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail = json!([
            {
                "program": "Vote",
                "method": "Vote",
                "vote_account": "DHdYsTEsd1wGrmthR1ognfRXPkWBmxAWAv2pKdAix3HY",
                "sysvar_slot_hashes": "SysvarS1otHashes111111111111111111111111111",
                "sysvar_clock": "SysvarC1ock11111111111111111111111111111111",
                "vote_authority_pubkey": "D8izqaR979Fc2amDoGHmYqEugjckEi1RQL1Y1JKyHUwX",
                "slots": [
                    "142209632"
                ],
                "hash": "9yoSeo4kEjumeyYFfohaCUYyjsu134xhXbrAxumCmysC",
                "timestamp": "1658224090"
            }
        ]);
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_transaction_3() {
        //System.CreateAccount + Token.InitializeMint + AToken.CreateAssociatedAccount + Custom Program
        // https://solscan.io/tx/34YhTdSXdcXF5DQ29rhLrvt7GtCYGHYJMtchpHotfsRx3TGdDm8scoNKhGY77s6r9hxQPoXQ7f2d1k1nA8aKdmKk
        let data = "0200050a06852df21778a462ea79aae81500eae98a935dcca05f8b899ca8b41021a79980acc933a10d87058ad3131361cd345fe95eb7598ad52d972ee559f1ea3f8deb452bb2df65fdf1ad0514f549457e4338bb71e6885354aa5ed87969ef14f5fc736772295dfa0330919867f6f90f2e334d1a56a2203ec3d4086151aab0171ca13c74b626da01ca1cb62be1bbbf9927dd0de251964d351736fd36100bb0e06f728b4100000000000000000000000000000000000000000000000000000000000000008c97258f4e2489f1bb3d1029148e0d830b5a1399daff1084048e7bd8dbe9f8590b7065b1e3d17c45389d527f6b04c3cd58b86c731aa0fdb549b6d1bc03f8294606a7d517192c5c51218cc94c3d4af17f58daee089ba1fd44e3dbd98a0000000006ddf6e1d765a193d9cbe146ceeb79ac1cb485ed5f5b37913a8cf5857eff00a92865a919afcfd4d57cf8f69e11990c98a55e4cc4389ba43c7d32184ca652adb406050200013400000000604d160000000000520000000000000006ddf6e1d765a193d9cbe146ceeb79ac1cb485ed5f5b37913a8cf5857eff00a90902010843000006852df21778a462ea79aae81500eae98a935dcca05f8b899ca8b41021a799800106852df21778a462ea79aae81500eae98a935dcca05f8b899ca8b41021a799800707030100000005087b0012000000536e65616b65722023313830333539303435000000003200000068747470733a2f2f6170692e737465706e2e636f6d2f72756e2f6e66746a736f6e2f3130332f3130363036313531353732319001010100000006852df21778a462ea79aae81500eae98a935dcca05f8b899ca8b41021a799800164010607000400010509080009030104000907010000000000000007090201000000030905080a0a010000000000000000";
        let transaction = Vec::from_hex(data).unwrap();
        let parsed = ParsedSolanaTx::build(&transaction).unwrap();
        match parsed.overview {
            SolanaOverview::General(overview) => {
                let overview_1 = overview.get(0).unwrap();
                let overview_2 = overview.get(1).unwrap();
                let overview_3 = overview.get(2).unwrap();
                assert_eq!("System", overview_1.program);
                assert_eq!("CreateAccount", overview_1.method);
                assert_eq!("Token", overview_2.program);
                assert_eq!("InitializeMint", overview_2.method);
                assert_eq!("Token", overview_3.program);
                assert_eq!("MintTo", overview_3.method);
            }
            _ => println!("program overview parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail = json!([
            {
                "program": "System",
                "method": "CreateAccount",
                "funding_account": "STEPNq2UGeGSzCyGVr2nMQAzf8xuejwqebd84wcksCK",
                "new_account": "CdV4w55UDTvcza5d6V2Y6m7TF9Xmq9MHPUBYMe9WtptL",
                "amount": "1461600",
                "space": "82",
                "owner": "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA"
            },
            {
                "program": "Token",
                "method": "InitializeMint",
                "mint": "CdV4w55UDTvcza5d6V2Y6m7TF9Xmq9MHPUBYMe9WtptL",
                "sysver_rent": "SysvarRent111111111111111111111111111111111",
                "mint_authority_pubkey": "STEPNq2UGeGSzCyGVr2nMQAzf8xuejwqebd84wcksCK",
                "freeze_authority_pubkey": "STEPNq2UGeGSzCyGVr2nMQAzf8xuejwqebd84wcksCK",
                "decimals": 0
            },
            {
                "program": "Unknown",
                "reason": "Unable to parse instruction, Program `metaqbxxUerdq28cj1RbAWkYQm3ybzjb6a8bt518x1s` is not supported yet"
            },
            {
                "program": "Unknown",
                "reason": "Unable to parse instruction, Program `ATokenGPvbdGVxr1b2hvZbsiqW5xWH25efTNsLJA8knL` is not supported yet"
            },
            {
                "program": "Token",
                "method": "MintTo",
                "mint": "CdV4w55UDTvcza5d6V2Y6m7TF9Xmq9MHPUBYMe9WtptL",
                "mint_to_account": "DG3Za1KX8Tj1TeZJy2U9nDa8qX3tyZCBYNehNy4fFsnQ",
                "mint_authority_pubkey": "STEPNq2UGeGSzCyGVr2nMQAzf8xuejwqebd84wcksCK",
                "amount": "1"
            },
            {
                "program": "Unknown",
                "reason": "Unable to parse instruction, Program `metaqbxxUerdq28cj1RbAWkYQm3ybzjb6a8bt518x1s` is not supported yet"
            }
        ]);
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_transaction_4() {
        // System.CreateAccount + Token.InitializeAccount + Token.TokenTransfer + Token.CloseAccount
        // https://solscan.io/tx/5mSjiAapKzn7TEDWH3pkmNUYXSvmVyAAyF3zTEbmX9AosdfiC1dEbsQnNgDhUDBpNoYmSnPS99HPaBsKsakGR1hf
        let data = "02000407e9940f6435ae992ddbb4ac739ada475fde93bd54c6a9f36a8b60b37fe23ec3fdd8ffff8ad461ca3138f356758b148f2dffa7d055a79356b52727298026189ae82e6df8bd210e5f167971908e8746aa6790aa3bc74ee48a4bbf23236f9effaa65069b8857feab8184fb687f634618c035dac439dc1aeb3b5598a0f0000000000106a7d517192c5c51218cc94c3d4af17f58daee089ba1fd44e3dbd98a00000000000000000000000000000000000000000000000000000000000000000000000006ddf6e1d765a193d9cbe146ceeb79ac1cb485ed5f5b37913a8cf5857eff00a9bff514b7cba346fe333553de5579d50a7da74cf950dd7bdd27327ce9a17c876f04050200013400000000f01d1f0000000000a50000000000000006ddf6e1d765a193d9cbe146ceeb79ac1cb485ed5f5b37913a8cf5857eff00a9060401030004010106030201000903242d84be0000000006030100000109";
        let transaction = Vec::from_hex(data).unwrap();
        let parsed = ParsedSolanaTx::build(&transaction).unwrap();
        match parsed.overview {
            SolanaOverview::General(mut overview) => {
                let overview_1 = overview.pop().unwrap();
                let overview_2 = overview.pop().unwrap();
                let overview_3 = overview.pop().unwrap();
                let overview_4 = overview.pop().unwrap();
                assert_eq!("Token", overview_1.program);
                assert_eq!("CloseAccount", overview_1.method);
                assert_eq!("Token", overview_2.program);
                assert_eq!("Transfer", overview_2.method);
                assert_eq!("Token", overview_3.program);
                assert_eq!("InitializeAccount", overview_3.method);
                assert_eq!("System", overview_4.program);
                assert_eq!("CreateAccount", overview_4.method);
            }
            _ => println!("program overview parse error!"),
        };
        let expected_detail = json!(
            [{
                "program": "System",
                "method": "CreateAccount",
                "funding_account": "GinwSnwbsjkWXkancBr5E6EPrQtKdwnE5vPdriv1tK3i",
                "new_account": "Fc5UC9wa32FVzeFB2ijduV4R5nnGQu4dXH8ZrRUCSHMh",
                "amount": "2039280",
                "space": "165",
                "owner": "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA"
            },
            {
                "program": "Token",
                "method": "InitializeAccount",
                "account": "Fc5UC9wa32FVzeFB2ijduV4R5nnGQu4dXH8ZrRUCSHMh",
                "mint": "So11111111111111111111111111111111111111112",
                "owner": "GinwSnwbsjkWXkancBr5E6EPrQtKdwnE5vPdriv1tK3i",
                "sysver_rent": "SysvarRent111111111111111111111111111111111"
            },
            {
                "program": "Token",
                "method": "Transfer",
                "source_account": "48F1neXh5bGgKr8G6CM6tFZkaC51UgtVb5pqGLC27Doi",
                "recipient": "Fc5UC9wa32FVzeFB2ijduV4R5nnGQu4dXH8ZrRUCSHMh",
                "owner": "GinwSnwbsjkWXkancBr5E6EPrQtKdwnE5vPdriv1tK3i",
                "amount": "3196333348"
            },
            {
                "program": "Token",
                "method": "CloseAccount",
                "account": "Fc5UC9wa32FVzeFB2ijduV4R5nnGQu4dXH8ZrRUCSHMh",
                "recipient": "GinwSnwbsjkWXkancBr5E6EPrQtKdwnE5vPdriv1tK3i",
                "owner": "GinwSnwbsjkWXkancBr5E6EPrQtKdwnE5vPdriv1tK3i"
            }
        ]);
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_transaction_5() {
        // Memo + AToken.CreateAssociatedAccount + Token.SetAuthority
        // https://solscan.io/tx/55gHV4rWvLbyz7V5rhn3NeMPMKKiuhHJbLRSwLqzCc2482jeowmc93UJuCD7h3GpB1E3pVdDETZQu3CBFZSnAXJS
        let data = "0201060908a13fb5c9e7bc18aef6d4ec2e5bca9fb0b8c329c32bdf2baae9125aa3191cd36eeb5c79927943eef87a2828925665d2b3612a070fe5eee74680d8ac0b779ca136a3ae0cda1d97779bcd08c24409fe1c76f84f218aeed3296d8efe2dade261a606a7d517192c5c51218cc94c3d4af17f58daee089ba1fd44e3dbd98a0000000006ddf6e1d765a193d9cbe146ceeb79ac1cb485ed5f5b37913a8cf5857eff00a90b3338a0ab2cc841d5b014bc6a3cf756291874b319c9517d9bbfa9e4e9661ef90000000000000000000000000000000000000000000000000000000000000000054a5350f85dc882d614a55672788a296ddf1eababd0a60678884932f4eef6a08c97258f4e2489f1bb3d1029148e0d830b5a1399daff1084048e7bd8dbe9f859704b00127cf4d5d2ca44446993ee3bab439ce957bde518d1767b108b87a4a7d00307002c416141414141414141414141414141414141414141414141414141414141414141414141414141414141413d08070002010506040300040202012306030108a13fb5c9e7bc18aef6d4ec2e5bca9fb0b8c329c32bdf2baae9125aa3191cd3";
        let transaction = Vec::from_hex(data).unwrap();
        let parsed = ParsedSolanaTx::build(&transaction).unwrap();
        match parsed.overview {
            SolanaOverview::General(mut overview) => {
                let overview_1 = overview.pop().unwrap();
                assert_eq!("Token", overview_1.program);
                assert_eq!("SetAuthority", overview_1.method);
            }
            _ => println!("program overview parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail = json!([
            {
                "program": "Unknown",
                "reason": "Unable to parse instruction, Program `Memo1UhkJRfHyvLMcVucJwxXeuD728EqVDDwQDxFMNo` is not supported yet"
            },
            {
                "program": "Unknown",
                "reason": "Unable to parse instruction, Program `ATokenGPvbdGVxr1b2hvZbsiqW5xWH25efTNsLJA8knL` is not supported yet"
            },
            {
                "program": "Token",
                "method": "SetAuthority",
                "account": "4gHmx6Puk1J9YntAUvnyrXP68SmjCvQocuArQCt5o4p5",
                "old_authority_pubkey": "8Tz15moyu4eL48o4Pq5XLyxX5XkkKEsNcgx27ycaPLaU",
                "authority_type": "close account",
                "new_authority_pubkey": "agsWhfJ5PPGjmzMieWY8BR5o1XRVszUBQ5uFz4CtDiJ"
            }
        ]);
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_transaction_6() {
        // Token.Approve
        // https://solscan.io/tx/zf2KZX8S9BavoYxuxYrX47BmDY9YEdeMSzeCxakDoMGBCr7qYHjHAuEj5Qt2k6hV12XJKBGNAfsqLPPyPjthy5B
        let data = "0301070faa30697d8ea2d14ce506c401ad5f1bd33476ebcb8a8b5cea89fa0aafb7c04f3925070f12913aa23553bfaf08f0a6f293aadb24dd66711db239c9b0ccca751b05dcc3a6c16cb67f59d67085e174cd7469f3e00b99a63d6c8d95337096f9e8437d8c17a1e64eba64bb7238d33b21461db8824508c879f91199d9eff9309ff63952baf04e4356057aaea057a6d744e1a0dbb99091448d6c2807114f0a016c9f2c39df8b1e991b87277d51b2ee23b63496ff1a54ab30e61eb4c4572e52fb99af9421e636e5095d76cede0e72ff2a024a07652d423fb7f3978a4663a278d13e1c1ba10deb821d34b39060c73598d3dd86ecf853df3b2f38b02991ad2ccfa51306e01600000000000000000000000000000000000000000000000000000000000000003f5877e18f96dea58c638a21d2be860ba96f0e21d1d84c6a94dba44e2be81f0e494500f4fdcbc9ad22814e250c0d6763266f6ca9169e12662f477601991e1a36be49a1eeb81bf889c158fd8b7496ff9141d4aa433eae3948d0d8488f78951b78069b8857feab8184fb687f634618c035dac439dc1aeb3b5598a0f0000000000106a7d517192c5c51218cc94c3d4af17f58daee089ba1fd44e3dbd98a0000000006ddf6e1d765a193d9cbe146ceeb79ac1cb485ed5f5b37913a8cf5857eff00a954c495b382bac905bb70f97bc252b2ee3b796b2836a3287ed5787c70eb2484120608020001340000000030266d0500000000a50000000000000006ddf6e1d765a193d9cbe146ceeb79ac1cb485ed5f5b37913a8cf5857eff00a90e04010c000d01010e03010200090440084e05000000000b0a090a020106030504070e110140084e05000000005d4d3700000000000e02010001050e030100000109";
        let transaction = Vec::from_hex(data).unwrap();
        let parsed = ParsedSolanaTx::build(&transaction).unwrap();
        match parsed.overview {
            SolanaOverview::General(overview) => {
                let overview_1 = overview.get(0).unwrap();
                assert_eq!("System", overview_1.program);
                assert_eq!("CreateAccount", overview_1.method);
                let overview_2 = overview.get(1).unwrap();
                assert_eq!("Token", overview_2.program);
                assert_eq!("InitializeAccount", overview_2.method);
                let overview_3 = overview.get(2).unwrap();
                assert_eq!("Token", overview_3.program);
                assert_eq!("Approve", overview_3.method);
                let overview_4 = overview.get(3).unwrap();
                assert_eq!("Token", overview_4.program);
                assert_eq!("Revoke", overview_4.method);
                let overview_5 = overview.get(4).unwrap();
                assert_eq!("Token", overview_5.program);
                assert_eq!("CloseAccount", overview_5.method);
            }
            _ => println!("program overview parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail = json!([
          {
            "program": "System",
            "method": "CreateAccount",
            "funding_account": "CTM8DpiZXt1R85fxY4NM85P5t3QqR4Wjm1V85Z8EjVCC",
            "new_account": "3VYL1TrNMJFQaLy3SC9jAe9gNgymPgQgcZQNUzcc6M3i",
            "amount": "91039280",
            "space": "165",
            "owner": "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA"
          },
          {
            "program": "Token",
            "method": "InitializeAccount",
            "account": "3VYL1TrNMJFQaLy3SC9jAe9gNgymPgQgcZQNUzcc6M3i",
            "mint": "So11111111111111111111111111111111111111112",
            "owner": "CTM8DpiZXt1R85fxY4NM85P5t3QqR4Wjm1V85Z8EjVCC",
            "sysver_rent": "SysvarRent111111111111111111111111111111111"
          },
          {
            "program": "Token",
            "method": "Approve",
            "source_account": "3VYL1TrNMJFQaLy3SC9jAe9gNgymPgQgcZQNUzcc6M3i",
            "delegate_account": "FrmjFyCUdgrZfps1cJ7B3BTLnj9jjEJ34MwZncJBCxaG",
            "owner": "CTM8DpiZXt1R85fxY4NM85P5t3QqR4Wjm1V85Z8EjVCC",
            "amount": "89000000"
          },
          {
            "program": "Unknown",
            "reason": "Unable to parse instruction, Program `Dooar9JkhdZ7J3LHN3A7YCuoGRUggXhQaG4kijfLGU2j` is not supported yet"
          },
          {
            "program": "Token",
            "method": "Revoke",
            "source_account": "3VYL1TrNMJFQaLy3SC9jAe9gNgymPgQgcZQNUzcc6M3i",
            "owner": "CTM8DpiZXt1R85fxY4NM85P5t3QqR4Wjm1V85Z8EjVCC"
          },
          {
            "program": "Token",
            "method": "CloseAccount",
            "account": "3VYL1TrNMJFQaLy3SC9jAe9gNgymPgQgcZQNUzcc6M3i",
            "recipient": "CTM8DpiZXt1R85fxY4NM85P5t3QqR4Wjm1V85Z8EjVCC",
            "owner": "CTM8DpiZXt1R85fxY4NM85P5t3QqR4Wjm1V85Z8EjVCC"
          }
        ]);
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_transaction_7() {
        // System.CreateAccountWithSeed + Stake: Initialize + Stake: Delegate
        // https://solscan.io/tx/UxNDLmLJb1nR9sx3Q4xnELJZuneM4W9WBfb2pwBYDjWfCHxpWqCGgjpUsrwqMAFzfkCCNDj4AUpzvguSQ8tDHEk
        let data = "010007096aefb992fa0cd54aea185bf65a7da92aad6bd46da5a67c7675a04e6540d86f7a3d2ce2421048aa748a6cc22b5696032f902cfc0b3dd6bce0d379f76c383bceda0000000000000000000000000000000000000000000000000000000000000000e23a2b23b625e7513991be370a2c20d5c5e276491d36777ef2e5b1227ffe732906a1d8179137542a983437bdfe2a7ab2557f535c8a78722b68a49dc00000000006a1d817a502050b680791e6ce6db88e1e5b7150f61fc6790a4eb4d10000000006a7d51718c774c928566398691d5eb68b5eb8a39b4b6d5c73555b210000000006a7d517192c5c51218cc94c3d4af17f58daee089ba1fd44e3dbd98a0000000006a7d517193584d0feed9bb3431d13206be544281b57b8566cc5375ff4000000aada712c5d14f4e64d913b330ff3e519bc7f2aac580997f0c549620601866915030202000174030000006aefb992fa0cd54aea185bf65a7da92aad6bd46da5a67c7675a04e6540d86f7a18000000000000007374616b653a302e3231363239323431373439393638393500de2a9200000000c80000000000000006a1d8179137542a983437bdfe2a7ab2557f535c8a78722b68a49dc0000000000402010774000000006aefb992fa0cd54aea185bf65a7da92aad6bd46da5a67c7675a04e6540d86f7a6aefb992fa0cd54aea185bf65a7da92aad6bd46da5a67c7675a04e6540d86f7a00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000004060103060805000402000000";
        let transaction = Vec::from_hex(data).unwrap();
        let parsed = ParsedSolanaTx::build(&transaction).unwrap();
        match parsed.overview {
            SolanaOverview::General(overview) => {
                let overview_1 = overview.get(0).unwrap();
                assert_eq!("System", overview_1.program);
                assert_eq!("CreateAccountWithSeed", overview_1.method);
                let overview_2 = overview.get(1).unwrap();
                assert_eq!("Stake", overview_2.program);
                assert_eq!("Initialize", overview_2.method);
                let overview_3 = overview.get(2).unwrap();
                assert_eq!("Stake", overview_3.program);
                assert_eq!("DelegateStake", overview_3.method);
            }
            _ => println!("program overview parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail = json!([
            {
                "program": "System",
                "method": "CreateAccountWithSeed",
                "funding_account": "8CSELK4udyP5M3XU3nNRc9N8zfqL3Z6zFybiz3Bm9beH",
                "new_account": "57oZmoSzqF5at3ioAV7h849mUa6FKdu98ig3FwmjS4Nd",
                "base_account": "",
                "base_pubkey": "8CSELK4udyP5M3XU3nNRc9N8zfqL3Z6zFybiz3Bm9beH",
                "seed": "stake:0.2162924174996895",
                "amount": "2452282880",
                "space": "200",
                "owner": "Stake11111111111111111111111111111111111111"
            },
            {
                "program": "Stake",
                "method": "Initialize",
                "stake_account": "57oZmoSzqF5at3ioAV7h849mUa6FKdu98ig3FwmjS4Nd",
                "sysvar_rent": "SysvarRent111111111111111111111111111111111",
                "staker": "8CSELK4udyP5M3XU3nNRc9N8zfqL3Z6zFybiz3Bm9beH",
                "withdrawer": "8CSELK4udyP5M3XU3nNRc9N8zfqL3Z6zFybiz3Bm9beH",
                "timestamp": 0,
                "epoch": 0,
                "custodian": "11111111111111111111111111111111"
            },
            {
                "program": "Stake",
                "method": "DelegateStake",
                "stake_account": "57oZmoSzqF5at3ioAV7h849mUa6FKdu98ig3FwmjS4Nd",
                "vote_account": "GE6atKoWiQ2pt3zL7N13pjNHjdLVys8LinG8qeJLcAiL",
                "sysvar_clock": "SysvarC1ock11111111111111111111111111111111",
                "sysvar_stake_history": "SysvarStakeHistory1111111111111111111111111",
                "config_account": "StakeConfig11111111111111111111111111111111",
                "stake_authority_pubkey": "8CSELK4udyP5M3XU3nNRc9N8zfqL3Z6zFybiz3Bm9beH"
            }
        ]);
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_transaction_8() {
        // Stake: Withdraw
        // https://solscan.io/tx/4UDXRHMzzfFFnayfbR5pmRvdn3YqPwvYb7s5TkoL92osGGagENpfYXLFnK5guZ7187Dd3eNjDFDMkBd7jixpDfnG
        let data = "01000305575949043cea1e1713d06b6b2eba6bb22d303884908e683fcaaa7b0ba6209be859d521dc428449106dabb34dadd3b44cc7795f58be0d4a81aaeaada967b21bd206a1d8179137542a983437bdfe2a7ab2557f535c8a78722b68a49dc00000000006a7d51718c774c928566398691d5eb68b5eb8a39b4b6d5c73555b210000000006a7d517193584d0feed9bb3431d13206be544281b57b8566cc5375ff400000067415e51677a2d98e9f86da5c65fe4c72bdee5cb2755af7a27d13dda710aa26001020501000304000c04000000ed77410300000000";
        let transaction = Vec::from_hex(data).unwrap();
        let parsed = ParsedSolanaTx::build(&transaction).unwrap();
        match parsed.overview {
            SolanaOverview::General(overview) => {
                let overview_1 = overview.get(0).unwrap();
                assert_eq!("Stake", overview_1.program);
                assert_eq!("Withdraw", overview_1.method);
            }
            _ => println!("program overview parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail = json!([
            {
                "program": "Stake",
                "method": "Withdraw",
                "stake_account": "73fnG8GC1ZanZSVv86re4kQsbrsiLP7xPCvXizCs9XW9",
                "recipient": "6sySB1243EqqtMsExjNwmbouFVksZAF6w6bGe99V2CgX",
                "sysvar_clock": "SysvarC1ock11111111111111111111111111111111",
                "sysvar_stake_history": "SysvarStakeHistory1111111111111111111111111",
                "withdraw_authority_pubkey": "6sySB1243EqqtMsExjNwmbouFVksZAF6w6bGe99V2CgX",
                "stake_authority_pubkey": "",
                "amount": "54622189"
            }
        ]);
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_transaction_9() {
        // Stake: Deactivate
        // https://solscan.io/tx/23nCgTp9zNo7e56bcFiyYgM4t4A9HGGX4z3JNPMzBxGbGExAVSMBfpkE3digYRcYbKUQfwnq3rGtEND7fD5HiT2x
        let data = "010002044dd6a13d7b9ca64c690638eb9679f4a264a5a93022212ec608b24964dbc5701aff979426efda42a314f5b5477ea3264fddfb5ee1b9f939bff1e90cbea09cde3306a1d8179137542a983437bdfe2a7ab2557f535c8a78722b68a49dc00000000006a7d51718c774c928566398691d5eb68b5eb8a39b4b6d5c73555b2100000000a4eb4d5967097a0ec98783003eba3fde67341ba6d6d55f7d6987494a952466520102030103000405000000";
        let transaction = Vec::from_hex(data).unwrap();
        let parsed = ParsedSolanaTx::build(&transaction).unwrap();
        match parsed.overview {
            SolanaOverview::General(overview) => {
                let overview_1 = overview.get(0).unwrap();
                assert_eq!("Stake", overview_1.program);
                assert_eq!("Deactivate", overview_1.method);
            }
            _ => println!("program overview parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail = json!([
            {
                "program": "Stake",
                "method": "Deactivate",
                "delegated_stake_account": "JCj29zzZjPjZnDKVP7EyR6gordbAAYacKGkG811NepcJ",
                "sysvar_clock": "SysvarC1ock11111111111111111111111111111111",
                "stake_authority_pubkey": "6ErDKgZ7M1jdHp9fMWQ7mB3vmxBAWKH7YJ9FGk1qaYBK"
            }
        ]);
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_transaction_10() {
        // https://solscan.io/tx/3KCJ2aWgKc6cyEagFdk74WfM9eDw7VumB7rPw96fQkD1CmjG3w29gTazDEvNsc2bNbkQaZAvL2att11Siy8qF89k
        let data = "0301080fae0e9965d80b3bb521ed714366a4d461fd58d7b7c97caa15564ba34c3ec5c04d940d487f489c470872533e2d8b55a5ec1ae1fd130cefae0f1bd1527a9b6955c1ab9daad5867d8a4dba28bb9b9bc4146bc81a83e877c01d693d9860e2863df6f5aaf29edc6d0d3544fccda1232277d6032783264c5cfc335600c85f30754adaa9f604b96c15a6018a88598d0c5a310fe2b6333aa48ba916e502be02578ca50384cc51e45da7f68a2906979e692c1e8bc87e51deca9ddfe7e673895a01ef80facf5f4019373457f129bf4cae6a4255518b885bf718157dd4357233dc79268c4cbf069b8857feab8184fb687f634618c035dac439dc1aeb3b5598a0f0000000000106a7d517192c5c51218cc94c3d4af17f58daee089ba1fd44e3dbd98a0000000006a7d51718c774c928566398691d5eb68b5eb8a39b4b6d5c73555b210000000023166cdfc331b06925f390147d4270172c25a5b218580326b09081a9f3bbe90c051e8a28c6a067b32fbb33323ed92334b6adbdc4639b871c8a2e44f47058ef8506ddf6e1d765a193d9cbe146ceeb79ac1cb485ed5f5b37913a8cf5857eff00a900000000000000000000000000000000000000000000000000000000000000000508c2ceb1b5d05c874980ac52cf659740e7e9b9356aaf2a0362673263526c15e83b5d0c7735cf4f76914b1488bc665d32dce3140950851428922cc65fbb565b070c03030200090424eb0700000000000d0200013400000000f01d1f0000000000a50000000000000006ddf6e1d765a193d9cbe146ceeb79ac1cb485ed5f5b37913a8cf5857eff00a90c040107000801010e02090401080e0a03010405060a0b02090c090424eb0700000000000c02030001050c03010000010901";
        let transaction = Vec::from_hex(data).unwrap();
        let parsed = ParsedSolanaTx::build(&transaction).unwrap();
        match parsed.overview {
            SolanaOverview::General(overview) => {
                let overview_1 = overview.get(0).unwrap();
                assert_eq!("Token", overview_1.program);
                assert_eq!("Approve", overview_1.method);
                let overview_2 = overview.get(1).unwrap();
                assert_eq!("System", overview_2.program);
                assert_eq!("CreateAccount", overview_2.method);
                let overview_3 = overview.get(2).unwrap();
                assert_eq!("Token", overview_3.program);
                assert_eq!("InitializeAccount", overview_3.method);
                let overview_4 = overview.get(3).unwrap();
                assert_eq!("TokenLending", overview_4.program);
                assert_eq!("DepositReserveLiquidity", overview_4.method);
                let overview_5 = overview.get(4).unwrap();
                assert_eq!("Token", overview_5.program);
                assert_eq!("Revoke", overview_5.method);
                let overview_6 = overview.get(5).unwrap();
                assert_eq!("Token", overview_6.program);
                assert_eq!("CloseAccount", overview_6.method);
            }
            _ => println!("program overview parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail = json!([
            {
                "program": "Token",
                "method": "Approve",
                "source_account": "CWJtEyYYHy3ydjHn5Beh48mHiW9BBHSYjcGDJkB8awNx",
                "delegate_account": "CYvAAqCR6LjctqdWvPe1CfBW9p5uSc85Da45gENrVSr8",
                "owner": "CiSrMrPbsnr2pXFHEKXSvHqw1r29qbpRnK1qV9n7zYCC",
                "amount": "518948"
            },
            {
                "program": "System",
                "method": "CreateAccount",
                "funding_account": "CiSrMrPbsnr2pXFHEKXSvHqw1r29qbpRnK1qV9n7zYCC",
                "new_account": "Axw63e2KwrSmqWsZcNUQNXHH4cSfv2xEJBZG7Ua5Rrit",
                "amount": "2039280",
                "space": "165",
                "owner": "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA"
            },
            {
                "program": "Token",
                "method": "InitializeAccount",
                "account": "Axw63e2KwrSmqWsZcNUQNXHH4cSfv2xEJBZG7Ua5Rrit",
                "mint": "So11111111111111111111111111111111111111112",
                "owner": "CiSrMrPbsnr2pXFHEKXSvHqw1r29qbpRnK1qV9n7zYCC",
                "sysver_rent": "SysvarRent111111111111111111111111111111111"
            },
            {
                "program": "Unknown",
                "reason": "Unable to parse instruction, Error occurred when parsing program instruction, reason: `Custom program error: 0x0`"
            },
            {
                "program": "TokenLending",
                "method": "DepositReserveLiquidity",
                "source_liquidity_account": "CWJtEyYYHy3ydjHn5Beh48mHiW9BBHSYjcGDJkB8awNx",
                "destination_collateral_account": "Axw63e2KwrSmqWsZcNUQNXHH4cSfv2xEJBZG7Ua5Rrit",
                "reserve_account": "HZMUNJQDwT8rdEiY2r15UR6h8yYg7QkxiekjyJGFFwnB",
                "reserve_liquidity_supply_account": "EkabaFX962r7gbdjQ6i2kfbrjFA6XppgKZ4APeUhA7gS",
                "reserve_collateral_mint": "7QpRNyLenfoUA8SrpDTaaurtx4JxAJ2j4zdkNUMsTa6A",
                "lending_market_account": "3My6wgR1fHmDFqBvv1hys7PigtH1megLncRCh2PkBMTR",
                "lending_market_authority_pubkey": "Lz3nGpTr7SfSf7eJqcoQEkXK2fSK3dfCoSdQSKxbXxQ",
                "user_transfer_authority_pubkey": "CYvAAqCR6LjctqdWvPe1CfBW9p5uSc85Da45gENrVSr8",
                "sysvar_clock": "SysvarC1ock11111111111111111111111111111111",
                "token_program_id": "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
                "liquidity_amount": "518948"
            },
            {
                "program": "Token",
                "method": "Revoke",
                "source_account": "CWJtEyYYHy3ydjHn5Beh48mHiW9BBHSYjcGDJkB8awNx",
                "owner": "CiSrMrPbsnr2pXFHEKXSvHqw1r29qbpRnK1qV9n7zYCC"
            },
            {
                "program": "Token",
                "method": "CloseAccount",
                "account": "Axw63e2KwrSmqWsZcNUQNXHH4cSfv2xEJBZG7Ua5Rrit",
                "recipient": "CiSrMrPbsnr2pXFHEKXSvHqw1r29qbpRnK1qV9n7zYCC",
                "owner": "CiSrMrPbsnr2pXFHEKXSvHqw1r29qbpRnK1qV9n7zYCC"
            }
        ]);
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_transaction_11() {
        // unknown https://solscan.io/tx/2qbYriWvjGbNJg32tGh8io6NC4teMdhSAtu1jmskEKdGDAh9kAt2MUcZUo7ac7JCKjUa9wjqcWnDhx7T5FZ7azzC
        let data = "0100060e1a93fffb26ce645adeae58f0f414c320bcec30ce12a66bd263a91ec9b3958ff45d2a5ee5685c17e07cede5bef98300d4170ebbe2d99f064c4bb05ee97b35de7d75119b3175807586e3f4a7e5cd0f890e96a753b10fccc7681e9473a0083270f1bfa153f6e9dc8b683a60c22c944131adc8abfb93befad36efcc572c783e27055f7f1afd0d58a5a0dcfa04274a0c6aaa6e8c38380af1cd2b1c3f881cc9e24ac330b62ba074f722c9d4114f2d8f70a00c66002337b9bf90c873657a6d201db4c80067d7c615ffb9455750b6b889211bcf4c76f687cbb6ba93adc37ecaefbfb41b207154414aa83b02488ade068d5effab202658060008643843ceacd8dc5e1b89f0000000000000000000000000000000000000000000000000000000000000000222829e89767b2043c86d1b51f31364e5adaeb861fd62e7a7f46be4dbbc55ca4cfa6452d2d0f594a1ddd1bead98a7a52fd63925ae3266435dde44a5ab3fdb8f10545e365bef271ad75350367565da40da336dc1c879bb1548a7afcc55aa9391e0b616d4895472c6a02e9ecd3e87951f18401d675dedcbe33e8673e00221522d306ddf6e1d765a193d9cbe146ceeb79ac1cb485ed5f5b37913a8cf5857eff00a91b421195ecaa4f6b65834080c6ac091cd06449b7cb33a5cbe42a86b27d36a469010c0d020507010a03000409080d0b0610f223c68952e1f2b68096980000000000";
        let transaction = Vec::from_hex(data).unwrap();
        let parsed = ParsedSolanaTx::build(&transaction).unwrap();
        match parsed.overview {
            SolanaOverview::Unknown(overview) => {
                assert_eq!("This transaction can not be decoded", overview.description);
            }
            _ => println!("program overview parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail = json!({
            "accounts": [
                "2nkVYBJQg5UavBjvBT4jcahJamnFyr8wzPhzabDaRnBD",
                "7GgPYjS5Dza89wV6FpZ23kUJRG5vbQ1GM25ezspYFSoE",
                "8szGkuLTAux9XMgZ2vtY39jVSowEcpBfFfD8hXSEqdGC",
                "Du3Ysj1wKbxPKkuPPnvzQLQh8oMSVifs3jGZjJWXFmHN",
                "HgsT81nSFHBB1hzRP9KEsPE5EEVCPwyCsUao2FFuL72i",
                "mSoLzYCxHdYgdzU16g5QSh3i5K3z3KZK7ytfqcJm7So",
                "SLRjonR3SeJX74EqK1LgqK227D8kK2SEwXc3MKegyvh",
                "UefNb6z6yvArqe4cJHTXCqStRsKmWhGxnZzuHbikP5Q",
                "11111111111111111111111111111111",
                "3JLPCS1qM2zRw3Dp6V4hZnYHd4toMNPkNesXdX9tg6KM",
                "EyaSjUtSgo9aRD1f8LWXwdvkpDTmXAW54yoSHZRF14WL",
                "MarBmsSgKXdrN1egZf5sqe1TMai9K1rChYNDJgjq7aD",
                "mRefx8ypXNxE59NhoBqwqb3vTvjgf8MYECp4kgJWiDY",
                "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA"
            ],
            "block_hash": "2qQSUu3YPsSYKTVN71jgGpmqZFwTzFjjE8WYsGvSm8hE",
            "header": {
                "num_readonly_signed_accounts": 0,
                "num_readonly_unsigned_accounts": 6,
                "num_required_signatures": 1
            },
            "instructions": [
                {
                    "account_indexes": [
                        2,
                        5,
                        7,
                        1,
                        10,
                        3,
                        0,
                        4,
                        9,
                        8,
                        13,
                        11,
                        6
                    ],
                    "accounts": "8szGkuLTAux9XMgZ2vtY39jVSowEcpBfFfD8hXSEqdGC,mSoLzYCxHdYgdzU16g5QSh3i5K3z3KZK7ytfqcJm7So,UefNb6z6yvArqe4cJHTXCqStRsKmWhGxnZzuHbikP5Q,7GgPYjS5Dza89wV6FpZ23kUJRG5vbQ1GM25ezspYFSoE,EyaSjUtSgo9aRD1f8LWXwdvkpDTmXAW54yoSHZRF14WL,Du3Ysj1wKbxPKkuPPnvzQLQh8oMSVifs3jGZjJWXFmHN,2nkVYBJQg5UavBjvBT4jcahJamnFyr8wzPhzabDaRnBD,HgsT81nSFHBB1hzRP9KEsPE5EEVCPwyCsUao2FFuL72i,3JLPCS1qM2zRw3Dp6V4hZnYHd4toMNPkNesXdX9tg6KM,11111111111111111111111111111111,TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA,MarBmsSgKXdrN1egZf5sqe1TMai9K1rChYNDJgjq7aD,SLRjonR3SeJX74EqK1LgqK227D8kK2SEwXc3MKegyvh",
                    "data": "WuE7HjnsyebB3jriTd85Ef",
                    "program": "Unknown",
                    "program_account": "mRefx8ypXNxE59NhoBqwqb3vTvjgf8MYECp4kgJWiDY",
                    "program_index": 12,
                    "reason": "Unable to parse instruction, Program `mRefx8ypXNxE59NhoBqwqb3vTvjgf8MYECp4kgJWiDY` is not supported yet"
                }
            ]
        });
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_transaction_12() {
        // Stake: Merge https://solscan.io/tx/5tNL5ujYCFEgbJzxdN5dtHE4nAVnVaVh2fqGhjvNb2mEA7WCcZfUhX5yK7q5StUpKxZyMcY7TM9nkU1LzLNwCpz4
        let data = "01000306507e6e0eedc8bad07d8e1907faf5cd44364b75503f4b2f9a2efd196fabcdbc0d8a35bc824e0b6a9970622a2d84513cb3d0d8d9274d026c09b189a3c991b04e679afd0fdcc13de092bc32eaef523c79183e3c923924634e64b9808b67ce00316f06a7d51718c774c928566398691d5eb68b5eb8a39b4b6d5c73555b210000000006a7d517193584d0feed9bb3431d13206be544281b57b8566cc5375ff400000006a1d8179137542a983437bdfe2a7ab2557f535c8a78722b68a49dc000000000453461b52d93efbb8f738fe6df0fb3d12c5a014432f41011d426e3b01eb4f66b01050501020304000407000000";
        let transaction = Vec::from_hex(data).unwrap();
        let parsed = ParsedSolanaTx::build(&transaction).unwrap();
        match parsed.overview {
            SolanaOverview::General(overview) => {
                let overview_1 = overview.get(0).unwrap();
                assert_eq!("Stake", overview_1.program);
                assert_eq!("Merge", overview_1.method);
            }
            _ => println!("program overview parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail = json!([{"program":"Stake","method":"Merge","destination_stake_account":"AJWneUm7QuQENJXkn7rDMp1Bvfab3R5vY7LcGuMmTtQv","source_stake_account":"BS1bVhRD2iJrGbiHV61J3SRg8TsGjR2a9TSBeoJBY6Ce","sysvar_clock":"SysvarC1ock11111111111111111111111111111111","sysvar_stake_history":"SysvarStakeHistory1111111111111111111111111","stake_authority_pubkey":"6RDS19fkTJ3BtTx78NswVm2VSPRmwM9tEa6kGuadAPKS"}]);
        assert_eq!(expected_detail, parsed_detail);
    }
}
