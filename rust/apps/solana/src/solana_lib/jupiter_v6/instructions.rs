use alloc::string::String;
use alloc::vec::Vec;

use borsh::{from_slice, BorshDeserialize};
use serde_derive::Serialize;

use crate::solana_lib::solana_program::errors::ProgramError;
use crate::solana_lib::traits::Dispatch;

use super::errors::JupiterError;

#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub struct SharedAccountsRouteArgs {
    /// The unique identifier for the route
    pub id: u8,
    /// The route plan to use for the route
    pub route_plan: Vec<RoutePlanStep>,
    /// The input amount for the route
    pub in_amount: u64,
    /// The expected output amount after accounting for slippage and platform fee
    pub quoted_out_amount: u64,
    /// The maximum allowed slippage in basis points (bps)
    pub slippage_bps: u16,
    /// The platform fee in basis points (bps)
    pub platform_fee_bps: u8,
}

#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub struct RoutePlanStep {
    pub swap: Swap,
    pub percent: u8,
    pub input_index: u8,
    pub output_index: u8,
}

#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub enum Swap {
    #[default]
    Saber,
    SaberAddDecimalsDeposit,
    SaberAddDecimalsWithdraw,
    TokenSwap,
    Sencha,
    Step,
    Cropper,
    Raydium,
    Crema {
        a_to_b: bool,
    },
    Lifinity,
    Mercurial,
    Cykura,
    Serum {
        side: Side,
    },
    MarinadeDeposit,
    MarinadeUnstake,
    Aldrin {
        side: Side,
    },
    AldrinV2 {
        side: Side,
    },
    Whirlpool {
        a_to_b: bool,
    },
    Invariant {
        x_to_y: bool,
    },
    Meteora,
    GooseFX,
    DeltaFi {
        stable: bool,
    },
    Balansol,
    MarcoPolo {
        x_to_y: bool,
    },
    Dradex {
        side: Side,
    },
    LifinityV2,
    RaydiumClmm,
    Openbook {
        side: Side,
    },
    Phoenix {
        side: Side,
    },
    Symmetry {
        from_token_id: u64,
        to_token_id: u64,
    },
    TokenSwapV2,
    HeliumTreasuryManagementRedeemV0,
    StakeDexStakeWrappedSol,
    StakeDexSwapViaStake {
        bridge_stake_seed: u32,
    },
    GooseFXV2,
    Perps,
    PerpsAddLiquidity,
    PerpsRemoveLiquidity,
    MeteoraDlmm,
    OpenBookV2 {
        side: Side,
    },
    RaydiumClmmV2,
    StakeDexPrefundWithdrawStakeAndDepositStake {
        bridge_stake_seed: u32,
    },
    Clone {
        pool_index: u8,
        quantity_is_input: bool,
        quantity_is_collateral: bool,
    },
    SanctumS {
        src_lst_value_calc_accs: u8,
        dst_lst_value_calc_accs: u8,
        src_lst_index: u32,
        dst_lst_index: u32,
    },
    SanctumSAddLiquidity {
        lst_value_calc_accs: u8,
        lst_index: u32,
    },
    SanctumSRemoveLiquidity {
        lst_value_calc_accs: u8,
        lst_index: u32,
    },
    RaydiumCP,
    WhirlpoolSwapV2 {
        a_to_b: bool,
        remaining_accounts_info: Option<RemainingAccountsInfo>,
    },
    OneIntro,
}

#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub struct RemainingAccountsInfo {
    pub slices: Vec<RemainingAccountsSlice>,
}

#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub struct RemainingAccountsSlice {
    pub accounts_type: AccountsType,
    pub length: u8,
}

#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub enum AccountsType {
    #[default]
    TransferHookA,
    TransferHookB,
}

#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub enum Side {
    #[default]
    Bid,
    Ask,
}

#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub struct Field {
    pub name: String,
    pub r#type: String,
}

#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub struct RouteArgs {
    pub route_plan: Vec<RoutePlanStep>,
    pub in_amount: u64,
    pub quoted_out_amount: u64,
    pub slippage_bps: u16,
    pub platform_fee_bps: u8,
}

#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub struct ExactOutRouteArgs {
    pub route_plan: Vec<RoutePlanStep>,
    pub out_amount: u64,
    pub quoted_in_amount: u64,
    pub slippage_bps: u16,
    pub platform_fee_bps: u8,
}
#[derive(BorshDeserialize, Serialize, Debug, Default, Clone)]
#[borsh(crate = "borsh")]
pub struct SharedAccountsExactOutRouteArgs {
    pub id: u8,
    pub route_plan: Vec<RoutePlanStep>,
    pub out_amount: u64,
    pub quoted_in_amount: u64,
    pub slippage_bps: u16,
    pub platform_fee_bps: u8,
}

#[derive(Debug)]
pub enum JupiterInstructions {
    /// Route by using program owned token accounts and open orders accounts.
    SharedAccountsRoute(SharedAccountsRouteArgs),
    /// route_plan Topologically sorted trade DAG
    Route(RouteArgs),
    /// exactOutRoute
    ExactOutRoute(ExactOutRouteArgs),
    /// Route by using program owned token accounts and open orders accounts.
    SharedAccountsExactOutRoute(SharedAccountsExactOutRouteArgs),
}

impl Dispatch for JupiterInstructions {
    fn dispatch(instrucion_data: &[u8]) -> Result<Self, ProgramError> {
        let data = instrucion_data;
        let ix_type = &data[..8];
        let ix_data = &data[8..];
        let ix_type_hex = hex::encode(ix_type);
        match ix_type_hex.as_str() {
            "c1209b3341d69c81" => {
                //  sharedAccountsRoute methodId = sighash(SIGHASH_GLOBAL_NAMESPACE, "shared_accounts_route")
                Ok(JupiterInstructions::SharedAccountsRoute(
                    from_slice::<SharedAccountsRouteArgs>(ix_data).unwrap(),
                ))
            }
            "e517cb977ae3ad2a" => {
                // route methodId = sighash(SIGHASH_GLOBAL_NAMESPACE, "route")
                Ok(JupiterInstructions::Route(
                    from_slice::<RouteArgs>(ix_data).unwrap(),
                ))
            }
            "d033ef977b2bed5c" => {
                // exactOutRoute methodId = sighash(SIGHASH_GLOBAL_NAMESPACE, "exact_out_route")
                Ok(JupiterInstructions::ExactOutRoute(
                    from_slice::<ExactOutRouteArgs>(ix_data).unwrap(),
                ))
            }
            "b0d169a89a7d453e" => {
                // sharedAccountsExactOutRoute methodId = sighash(SIGHASH_GLOBAL_NAMESPACE, "shared_accounts_exact_out_route")
                Ok(JupiterInstructions::SharedAccountsExactOutRoute(
                    from_slice::<SharedAccountsExactOutRouteArgs>(ix_data).unwrap(),
                ))
            }
            _ => Err(JupiterError::UnknownJupiterInstruction.into()),
        }
    }
}

#[cfg(test)]
mod tests {
    use std::prelude::rust_2024::ToString;

    use serde_json::json;
    use third_party::bitcoin;

    use crate::message::Message;
    use crate::read::Read;
    use crate::solana_lib::utils::{sighash, SIGHASH_GLOBAL_NAMESPACE};

    use super::*;

    #[test]
    fn test_generate_shared_accounts_route_sighash() {
        let method_name = "shared_accounts_route";
        let method_id = sighash(SIGHASH_GLOBAL_NAMESPACE, method_name);
        let method_id_hex = hex::encode(method_id);
        assert_eq!(method_id_hex, "c1209b3341d69c81");

        let route_method_name = "route";
        let route_method_id = sighash(SIGHASH_GLOBAL_NAMESPACE, route_method_name);
        let route_method_id_hex = hex::encode(route_method_id);
        assert_eq!(route_method_id_hex, "e517cb977ae3ad2a");
    }

    #[test]
    fn test_route_args() {
        //https://solscan.io/tx/3BaTUxM2LBCD69w3RgDoZ8cvREGiwZDRnZNVopy7ctmh6PVG9u6cdB8F4vwCndPcGBtb7RDkZf46NwaeKcmHypG8
        let instruction_data =
            "e517cb977ae3ad2a01000000110164000140420f00000000000c480100000000000e0000";
        let instruction_bytes = hex::decode(instruction_data).unwrap();
        let parse_instruction_res = JupiterInstructions::dispatch(&instruction_bytes);
        if let Ok(JupiterInstructions::Route(args)) = parse_instruction_res {
            assert_eq!(args.route_plan.len(), 1);
            assert_eq!(args.in_amount, 1000000);
            assert_eq!(args.quoted_out_amount, 83980);
            assert_eq!(args.slippage_bps, 14);
            assert_eq!(args.platform_fee_bps, 0);
        }
    }

    #[test]
    fn test_shared_accounts_route_args() {
        // https://solscan.io/tx/4HasMe6JzqCjZjQyUUps2n63cHBuQUVAPNowLpa6Sb95DUUpSKBjLSGZfGkyUZ9NBuJUwqY5PEoYL9BihqterXgg
        let instruction_data = "c1209b3341d69c810f03000000266400012664010226640203c2ce010000000000d1410d0000000000320000";
        let instruction_bytes = hex::decode(instruction_data).unwrap();
        let parse_instruction_res = JupiterInstructions::dispatch(&instruction_bytes);
        if let Ok(JupiterInstructions::SharedAccountsRoute(args)) = parse_instruction_res {
            assert_eq!(args.id, 15);
            assert_eq!(args.route_plan.len(), 3);
            assert_eq!(args.in_amount, 118466);
            assert_eq!(args.quoted_out_amount, 868817);
            assert_eq!(args.slippage_bps, 50);
            assert_eq!(args.platform_fee_bps, 0);
        } else {
            panic!("Failed to parse shared accounts route args");
        }
    }

    #[test]
    fn test_jupiter_sol_to_wif_raw_tx_parse() {
        // https://solscan.io/tx/3BaTUxM2LBCD69w3RgDoZ8cvREGiwZDRnZNVopy7ctmh6PVG9u6cdB8F4vwCndPcGBtb7RDkZf46NwaeKcmHypG8
        //  curl https://api.mainnet-beta.solana.com -X POST -H "Content-Type: application/json" -d '
        // {
        //   "jsonrpc": "2.0",
        //   "id": 1,
        //   "method": "getTransaction",
        //   "params": [
        //     "3BaTUxM2LBCD69w3RgDoZ8cvREGiwZDRnZNVopy7ctmh6PVG9u6cdB8F4vwCndPcGBtb7RDkZf46NwaeKcmHypG8",
        //     {
        //       "encoding": "base58",
        //       "maxSupportedTransactionVersion": 0
        //     }
        //   ]
        // }'
        // Bhkm2aMNao5ZpC1h84aDChUDb66V7FcsA5D6HRFP6Vzk1sTBWv4PSpnHVLCcLsR2hzwLd3U8UquZYCkvQQ39rGCedZqWvLJam3nT52xGqvHQAodmVynpHGrQ3nR71CXECgjXrvBcGVPXga3mty6SzcPqvLYPy7HSkQrFb3ntGkKNXDSzwC2chuJg5e8vRitkmgE69EX5dBD1y18CwwugUTotVMvLGUBoTgHxjMDTSeSeagL9PfWAnHBdfDPq3L9JwGD9YERKcn9AcY1LT9xkuu1tpxJrQ2Xy4CoYzcvKKLtk9MDCJqQGC5XXpfaXg9WrMinH6XRdhbuAfvAwNpyGYG5YmV16TrZDDNZmGwpwzCsqzuqbsMuyWozUkCRpzYPGizEnWUQUpt4yHYiufJnRSs8YxkCNUdgrDFnmuMZm9tcWm5XjexXiT4KL4HuBwhBdDWfTYbJpZM3bP18brTHtpa89S3sEMD4pEaecgPytyM7uopoyE7bNuxBM3hLLEiENJatGUUH6S57wqM98eyHFjQn5uimftNBPJFaHoLzfpUzo7oRwxati7x1z6uxXCXJ2tZsHnGHuURjE8bpT3J9v8LKBsGVcBUg5WSKfnKUEm3jTQ9hP6eTJt5R1G3qbqLFxJXFoC5P3kwQsDA3Jb1FZgDExbJv2vwQVMy9qgrAH4J1MPYrURKonipr1xZkajtCZxpaEJ24uhGr1h6De8Ge9Er26xgiZwAbiEadNnpPFVKvehBjDQW2NR3dSKXZgigAqo3z4WAsPo8r7Z3CVsYVqweqanisumQgGGS7chDxzwRFGhsdbxuanvPqNd2ZzsfknsijccYEYPu5MAqwzMwsokWYTE1dMo8N6bxGs7K2bkVdeHpexfbZqPkH9acbvH19Kf1yTKD9wghe4SqhqNhPSg3U19xTF2MECBs3wisWDyR6m2VxCnVXo5couQipH2NobzfrUZVaHprY5xg36xUxLfsRiYwLBUg35dDNh6Sr5LigUFp1RdmUHcHpubFuqNy6ea45MtFmTUpCXWTf1sDjffkun1ygJCLvTQBSv46D9A7ZzYsVTHav2BMTHAyrRMDydz7i4bXymAe1uZUUB1PuMDnS6LLdiyF7Ng5iG3ND2kaHyLb
        let raw_tx_base58 = "Bhkm2aMNao5ZpC1h84aDChUDb66V7FcsA5D6HRFP6Vzk1sTBWv4PSpnHVLCcLsR2hzwLd3U8UquZYCkvQQ39rGCedZqWvLJam3nT52xGqvHQAodmVynpHGrQ3nR71CXECgjXrvBcGVPXga3mty6SzcPqvLYPy7HSkQrFb3ntGkKNXDSzwC2chuJg5e8vRitkmgE69EX5dBD1y18CwwugUTotVMvLGUBoTgHxjMDTSeSeagL9PfWAnHBdfDPq3L9JwGD9YERKcn9AcY1LT9xkuu1tpxJrQ2Xy4CoYzcvKKLtk9MDCJqQGC5XXpfaXg9WrMinH6XRdhbuAfvAwNpyGYG5YmV16TrZDDNZmGwpwzCsqzuqbsMuyWozUkCRpzYPGizEnWUQUpt4yHYiufJnRSs8YxkCNUdgrDFnmuMZm9tcWm5XjexXiT4KL4HuBwhBdDWfTYbJpZM3bP18brTHtpa89S3sEMD4pEaecgPytyM7uopoyE7bNuxBM3hLLEiENJatGUUH6S57wqM98eyHFjQn5uimftNBPJFaHoLzfpUzo7oRwxati7x1z6uxXCXJ2tZsHnGHuURjE8bpT3J9v8LKBsGVcBUg5WSKfnKUEm3jTQ9hP6eTJt5R1G3qbqLFxJXFoC5P3kwQsDA3Jb1FZgDExbJv2vwQVMy9qgrAH4J1MPYrURKonipr1xZkajtCZxpaEJ24uhGr1h6De8Ge9Er26xgiZwAbiEadNnpPFVKvehBjDQW2NR3dSKXZgigAqo3z4WAsPo8r7Z3CVsYVqweqanisumQgGGS7chDxzwRFGhsdbxuanvPqNd2ZzsfknsijccYEYPu5MAqwzMwsokWYTE1dMo8N6bxGs7K2bkVdeHpexfbZqPkH9acbvH19Kf1yTKD9wghe4SqhqNhPSg3U19xTF2MECBs3wisWDyR6m2VxCnVXo5couQipH2NobzfrUZVaHprY5xg36xUxLfsRiYwLBUg35dDNh6Sr5LigUFp1RdmUHcHpubFuqNy6ea45MtFmTUpCXWTf1sDjffkun1ygJCLvTQBSv46D9A7ZzYsVTHav2BMTHAyrRMDydz7i4bXymAe1uZUUB1PuMDnS6LLdiyF7Ng5iG3ND2kaHyLb";
        let sol_to_wif_tx_bytes = bitcoin::base58::decode(&raw_tx_base58).unwrap();
        let sig_len = sol_to_wif_tx_bytes[0];
        assert_eq!(sig_len, 1);
        // eat sig num
        let sol_to_wif_tx_bytes = sol_to_wif_tx_bytes[1..].to_vec();
        assert_eq!(
            "8001000a132b8d8b3addd92759f55b840c2852f5bd50aee3552fe987ee0d4fe24b9043df8e42614f4f96dc9be3d7a9406063bd264dc64a647d011a3cd9c22595e22d2d482c7b74cddf885191465eeed9471ce82f67f7e89a9b31b8f3f0f43d1b233ed5d970b3ac999f734c896b8d2ff41464d44b7b2c03bf0f3a625242b65ad9f43ddd68c65a83c2016ee4f94bbdcdaf7ca664f8dba36e60bbbc0bb3252d75af7c0385f29907596e2bcda82c35c9493b009dc009b26826b24ffeaa4bbccabd42c42f847b641034b6e0f2274be43505923581e23620014a9a58a4194882ee0939d5435b813a5ae300f561442184c502cffc47bd0b7ed99bda7f0186e66aa09ccf6a392890eefc9471dc72392be08cd16232181ca89ac6f865ea26d6c6224f5672a780bfa9950306466fe5211732ffecadba72c39be7bc8ce5bbc5f7126b2c439b3a400000008c97258f4e2489f1bb3d1029148e0d830b5a1399daff1084048e7bd8dbe9f859069b8857feab8184fb687f634618c035dac439dc1aeb3b5598a0f00000000001000000000000000000000000000000000000000000000000000000000000000006ddf6e1d765a193d9cbe146ceeb79ac1cb485ed5f5b37913a8cf5857eff00a90479d55bf231c06eee74c56ece681507fdb1b2dea3f48e5102b1cda256bc138fc5f9fb32f49111ab20c33f2598fc836c113e291881ac21ee29169394011244e4b43ffa27f5d7f64a74c09b1f295879de4b09ab36dfc9dd514b321aa7b38ce5e80e03685f8e909053e458121c66f5a76aedc7706aa11c82f8aa952a8f2b7879a90aa8ab583cfcdb430ccaa52cee9ee4cd8ae8d5ccd9015018a6a52eb9ffdb7ac6cc02a4ff7756d58b1b826e8266ab5458b121b752f6648a3eb17b3102062abcbf0709000502f2df01000900090308ac0000000000000a060001000b0c0d01010c0200010c0200000040420f00000000000d010101110e150d0001020e0f0e100e110d0003010402050607081224e517cb977ae3ad2a01000000110164000140420f00000000000c480100000000000e00000d03010000010900",
            hex::encode(&sol_to_wif_tx_bytes[64..])
        );
        // eat sig
        let mut message = sol_to_wif_tx_bytes[64..].to_vec();
        let message = Message::read(&mut message).unwrap();
        // message detail
        let msg_detail = message.to_program_details().unwrap();
        let expect_msg_detail = json!({
          "program": "JupiterV6",
          "method": "Route",
          "accounts": [
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "3w1iMvjKGxpbGaaSekNUsZBcVKERg2BCsUZMGrjcTMsj",
            "5U7yDrr8hx4wuSsgsmXi3zte1AF7kZ6xWzcJqfS1dau1",
            "9JvT3avL4jxnnSQyqbUs2GsRgxBuYR8YUwwUFiJzwjp3",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "EKpQGSJtjMFqKZ9KQanSqYXRcF8fBopzLHYxdM65zcjm",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "D8cy77BBepLMngZx6ZukaTff5hCt1HrWyKk3Hnd9oitf",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "whirLbMiicVdio4qvUfM5KAg6Ct8VwpYzGff3uctyCc",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "3w1iMvjKGxpbGaaSekNUsZBcVKERg2BCsUZMGrjcTMsj",
            "D6NdKrKNQPmRZCCnG1GqXtF7MMoHB7qR6GU5TkG59Qz1",
            "5U7yDrr8hx4wuSsgsmXi3zte1AF7kZ6xWzcJqfS1dau1",
            "76LDmQCyrfqnQ6AUoX3C2HHY67WnG3ED9RQH8UkU5rrk",
            "9JvT3avL4jxnnSQyqbUs2GsRgxBuYR8YUwwUFiJzwjp3",
            "VgwuBBGNCVyT3fnEvRPFhMcYsDN9BH66F6nMQoX9azw",
            "26G9MV4TcmpLBCiJ6utya3NzS2zkFqrKFWNacPiPhhZT",
            "77nTTBuYQgxZG3o9P2s6FjgCcmbuy4F7fQ9a7RaX7wq7",
            "Hzy2TagxhPQEykb8Ck8kdX9BXq9HVcy1mRHevZn6VWdA",
            "icFSZ1T9xkL2RWBSG9Q1XMJbwr8UuHpCwK9ww1qbUYd"
          ],
          "args": {
            "route_plan": [
              {
                "swap": {
                  "Whirlpool": {
                    "a_to_b": true
                  }
                },
                "percent": 100,
                "input_index": 0,
                "output_index": 1
              }
            ],
            "in_amount": 1000000,
            "quoted_out_amount": 83980,
            "slippage_bps": 14,
            "platform_fee_bps": 0
          }
        });

        assert_eq!(
            expect_msg_detail,
            serde_json::to_value(msg_detail[5].clone()).unwrap()
        );
    }

    #[test]
    fn test_jupiter_usdc_to_sol_raw_tx_parse_1() {
        // https://solscan.io/tx/4HasMe6JzqCjZjQyUUps2n63cHBuQUVAPNowLpa6Sb95DUUpSKBjLSGZfGkyUZ9NBuJUwqY5PEoYL9BihqterXgg
        // https://solana.com/docs/rpc/http/gettransaction
        //  curl https://api.mainnet-beta.solana.com -X POST -H "Content-Type: application/json" -d '
        // {
        //   "jsonrpc": "2.0",
        //   "id": 1,
        //   "method": "getTransaction",
        //   "params": [
        //     "4HasMe6JzqCjZjQyUUps2n63cHBuQUVAPNowLpa6Sb95DUUpSKBjLSGZfGkyUZ9NBuJUwqY5PEoYL9BihqterXgg",
        //     {
        //       "encoding": "base58",
        //       "maxSupportedTransactionVersion": 0
        //     }
        //   ]
        // }'
        // Signatures: An array of signatures included on the transaction. 64bytes * sig num
        // Message: List of instructions to be processed atomically.
        // a2bwxpsMyjM4RUu6Ze6mhvPn3uGUoM762dExWj1BgqMLnqxWtRwus22kVhbi1gCj1HRrrPS1ArGAcZdteq91ZKAXF4y51j7PqPqSTp2zGuzQe9tAckPALy9WB65jTdnpE5augrAUmYLa3xQE68KSdAreANoHbqmSekenafb3jXgx5ao8dUVojuFoiawBCZmDDFudZNH9pS7z2kFyoAR3X8Dt2Q4n1r3zqo1PVSXLFBp2HqpnjztGZyaodJU9jqq9AgUDBQBPQHqxuYf8FqsAAimY2Q4bHEYnuuBNjEdaJQGQ8EgBTjpFNn7EPMwwHkJwo9iqXfJZhLHVqiVLbY74nt9kjpZHG6fFE6yG2eB2yhG3XXE4rRWthwaQqfKX5en2jgqGNMGTpwGCiaA4dZPnVwLa9YZy9kco4EsEVGgwMVRhnfRVtqxgeB4APa61D5Ya9cpsZ55GSVrm2bj1L9QixbRVbGYXB2EZGKXZrK2zvKkFwUhSsEyxNgCjpCUrz57H5vUtsESFLD23A8jMAuUYCTdFPpR16m73MSDa8iWTmVfL6vPnqbndbGeecxTjGQ9o3VFcdeowuxUYdc4r8gCrfSp9WbJyU6oKzBoJnzkLZWHfKESZduredkgRxfXHfKgyst8WbEa6Dqu5Rppv7F493RsiM6DBnD51gLaDxufyPGL2VMHQoJtjVRP6Rcy7VwVa1DrvzUCttUQZshfkZiBXqTFmutb7LWDnRycVkpzJHE9P2wg23Q7JbtYaQFGimDgoo97Ezjq46wo9vp4hCsunbib3PQ34rCwcHvsxmucz6Ky4VCqnDWf98nQe6zryqPEK76NP8ZMXnKopFpoQr23wNcCsRvjTiZAbrzn81s9nZJsTEzd4G2X2K7cSy4qghjGbdicnHroknQ7MV9SSqKNr9PcW2kZCq18bfvqkkcoBNqy1NKeVwqESa4xQrstTh77VTaBp6bKrbLG7QQRByWMaB2ZZLGii4xShNvcMHcgQfp2MpdzgprR5c2vmz9fwKWm3ptFebGk8PNQHMJ9WEpz8KgyMQvbu3pSbimVcgmuXPug3nELjQXXXN9Jcwq212YMxMUtVUXzgvAVbx3jjpici5FyDqPYMTiiLvjxDd4jNjNNP3DXbzQsJ2hTkVFV2A7UUvshLYuftGW2v6cJV8gfrHxWhrt6bd8HoHbqFDw4kMfHiAohe5b38ncd67jpujTfjzSsxRNcrF2aUdtdpTgkPYCfTjx4BJjFHtR1p98QpPJNDs9Mtr4Bq28cbtdejaDUgN7PN4F6enFcokzQBNhvhK8HAu1gQJSdyT244KFB57LTVnXiXxtFwbsDDhrug4TAo9XeMjr5SwaknxLyrhVoyvs2tY445iVPDBTJJoSbSs4xeXw2awxyPhkToUAnHDDdjhx7LnMhC45k7QREDQhXAn5j8Do3AXjvou1SgTDokCVYVRBmYRwyp31mXiJ1QU2Vu4UTqfMHZP5
        let solana_transaction_base58 = "a2bwxpsMyjM4RUu6Ze6mhvPn3uGUoM762dExWj1BgqMLnqxWtRwus22kVhbi1gCj1HRrrPS1ArGAcZdteq91ZKAXF4y51j7PqPqSTp2zGuzQe9tAckPALy9WB65jTdnpE5augrAUmYLa3xQE68KSdAreANoHbqmSekenafb3jXgx5ao8dUVojuFoiawBCZmDDFudZNH9pS7z2kFyoAR3X8Dt2Q4n1r3zqo1PVSXLFBp2HqpnjztGZyaodJU9jqq9AgUDBQBPQHqxuYf8FqsAAimY2Q4bHEYnuuBNjEdaJQGQ8EgBTjpFNn7EPMwwHkJwo9iqXfJZhLHVqiVLbY74nt9kjpZHG6fFE6yG2eB2yhG3XXE4rRWthwaQqfKX5en2jgqGNMGTpwGCiaA4dZPnVwLa9YZy9kco4EsEVGgwMVRhnfRVtqxgeB4APa61D5Ya9cpsZ55GSVrm2bj1L9QixbRVbGYXB2EZGKXZrK2zvKkFwUhSsEyxNgCjpCUrz57H5vUtsESFLD23A8jMAuUYCTdFPpR16m73MSDa8iWTmVfL6vPnqbndbGeecxTjGQ9o3VFcdeowuxUYdc4r8gCrfSp9WbJyU6oKzBoJnzkLZWHfKESZduredkgRxfXHfKgyst8WbEa6Dqu5Rppv7F493RsiM6DBnD51gLaDxufyPGL2VMHQoJtjVRP6Rcy7VwVa1DrvzUCttUQZshfkZiBXqTFmutb7LWDnRycVkpzJHE9P2wg23Q7JbtYaQFGimDgoo97Ezjq46wo9vp4hCsunbib3PQ34rCwcHvsxmucz6Ky4VCqnDWf98nQe6zryqPEK76NP8ZMXnKopFpoQr23wNcCsRvjTiZAbrzn81s9nZJsTEzd4G2X2K7cSy4qghjGbdicnHroknQ7MV9SSqKNr9PcW2kZCq18bfvqkkcoBNqy1NKeVwqESa4xQrstTh77VTaBp6bKrbLG7QQRByWMaB2ZZLGii4xShNvcMHcgQfp2MpdzgprR5c2vmz9fwKWm3ptFebGk8PNQHMJ9WEpz8KgyMQvbu3pSbimVcgmuXPug3nELjQXXXN9Jcwq212YMxMUtVUXzgvAVbx3jjpici5FyDqPYMTiiLvjxDd4jNjNNP3DXbzQsJ2hTkVFV2A7UUvshLYuftGW2v6cJV8gfrHxWhrt6bd8HoHbqFDw4kMfHiAohe5b38ncd67jpujTfjzSsxRNcrF2aUdtdpTgkPYCfTjx4BJjFHtR1p98QpPJNDs9Mtr4Bq28cbtdejaDUgN7PN4F6enFcokzQBNhvhK8HAu1gQJSdyT244KFB57LTVnXiXxtFwbsDDhrug4TAo9XeMjr5SwaknxLyrhVoyvs2tY445iVPDBTJJoSbSs4xeXw2awxyPhkToUAnHDDdjhx7LnMhC45k7QREDQhXAn5j8Do3AXjvou1SgTDokCVYVRBmYRwyp31mXiJ1QU2Vu4UTqfMHZP5";
        let solana_transaction_bytes = bitcoin::base58::decode(solana_transaction_base58).unwrap();
        // get signatures length
        let sig_num = solana_transaction_bytes[0];
        assert_eq!(1, sig_num);
        // get signaturs arrary by length
        let mut sigs = Vec::new();
        // eat sig_num
        let solana_transaction_bytes = &solana_transaction_bytes[1..];
        // get signatures
        for _ in 0..sig_num {
            let sig = &solana_transaction_bytes[..64];
            sigs.push(sig);
        }
        // message
        let mut message = solana_transaction_bytes[64..].to_vec();
        // message
        let message = Message::read(&mut message).unwrap();
        // message detail
        let msg_detail = message.to_program_details().unwrap();

        let expect_msg_detail = json!({
          "program": "JupiterV6",
          "method": "SharedAccountsRoute",
          "accounts": [
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "7iWnBRRhBCiNXXPhqiGzvvBkKrvFSWqqmxRyu9VyYBxE",
            "3w1iMvjKGxpbGaaSekNUsZBcVKERg2BCsUZMGrjcTMsj",
            "GrfQTEskA8ZP2eNorbRogpw5DFGNEHBiZGHE2EiGHDqm",
            "EpdaePzdqRkMtdZJquVPUWgyoJ5YEEpYALki6dv9VBrt",
            "2oL6my4QDDCfpgJZX1bZV1NgbmuNptKdgcE8wJm6efgk",
            "5U7yDrr8hx4wuSsgsmXi3zte1AF7kZ6xWzcJqfS1dau1",
            "FucMYjkfiJo4G9fbhQuc4cwpXnyhQRLRNwJzBmxRHryr#11",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#35",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "D8cy77BBepLMngZx6ZukaTff5hCt1HrWyKk3Hnd9oitf",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#120",
            "FucMYjkfiJo4G9fbhQuc4cwpXnyhQRLRNwJzBmxRHryr#4",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#120",
            "FucMYjkfiJo4G9fbhQuc4cwpXnyhQRLRNwJzBmxRHryr#9",
            "FucMYjkfiJo4G9fbhQuc4cwpXnyhQRLRNwJzBmxRHryr#2",
            "EpdaePzdqRkMtdZJquVPUWgyoJ5YEEpYALki6dv9VBrt",
            "2vJaV1w69B9MbwpEumDddzGDZBModm9aWfLdEztsiyrL",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#183",
            "FucMYjkfiJo4G9fbhQuc4cwpXnyhQRLRNwJzBmxRHryr#11",
            "FucMYjkfiJo4G9fbhQuc4cwpXnyhQRLRNwJzBmxRHryr#15",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#120",
            "7iWnBRRhBCiNXXPhqiGzvvBkKrvFSWqqmxRyu9VyYBxE",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#112",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#120",
            "G6h6hc4vyRDBytCgMDJT3kbdKLKkQ8ScSzwhstV2giry",
            "CjZ4NmS4fpCtsxwTHWnZ1MJiCFAGfACp2cCtY4c2Dwdj",
            "4tpFC9xZNGqoXrGHQQyCS4xQyqouyyccyixEPwhGKYg3",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#120",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#188",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#120",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#186",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#189",
            "2vJaV1w69B9MbwpEumDddzGDZBModm9aWfLdEztsiyrL",
            "6uYNWPP9jTBojn8h8s7s7w5JNLegiSns9r98LF2VAukw",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#184",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#183",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#190",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#120",
            "7iWnBRRhBCiNXXPhqiGzvvBkKrvFSWqqmxRyu9VyYBxE",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#112",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#120",
            "4RyamTNKtAA8NjzMWQzYwmCtwBgR7X8xf4RXJsFQFskf",
            "CYN7wqM2AXmWR23GY2xtgAtGhhpWKuavTcdjTFFt2FkK",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#187",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#120",
            "5U9kcysnaz9Ja8aQZgtQdwd5qgSyqz8WBXZ9RPPMibjW#9",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#120",
            "5U9kcysnaz9Ja8aQZgtQdwd5qgSyqz8WBXZ9RPPMibjW#11",
            "5U9kcysnaz9Ja8aQZgtQdwd5qgSyqz8WBXZ9RPPMibjW#20",
            "6uYNWPP9jTBojn8h8s7s7w5JNLegiSns9r98LF2VAukw",
            "2oL6my4QDDCfpgJZX1bZV1NgbmuNptKdgcE8wJm6efgk",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#184",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#35",
            "5U9kcysnaz9Ja8aQZgtQdwd5qgSyqz8WBXZ9RPPMibjW#13",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#120",
            "7iWnBRRhBCiNXXPhqiGzvvBkKrvFSWqqmxRyu9VyYBxE",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#112",
            "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq#120",
            "CJ8B6Kd626N1xkdkdAQP4mrH1eD98CUTQKvyjWGWrmNH",
            "HhPsDsW4sqy1Lfmana4cvUMvBtydQPx2YcVEkLVnquCF",
            "44noMuSTbgGNACjH4HBnXUy89oJnBQsV2FfPsRutN1Gg",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4"
          ],
          "args": {
            "id": 15,
            "route_plan": [
              {
                "swap": "MeteoraDlmm",
                "percent": 100,
                "input_index": 0,
                "output_index": 1
              },
              {
                "swap": "MeteoraDlmm",
                "percent": 100,
                "input_index": 1,
                "output_index": 2
              },
              {
                "swap": "MeteoraDlmm",
                "percent": 100,
                "input_index": 2,
                "output_index": 3
              }
            ],
            "in_amount": 118466,
            "quoted_out_amount": 868817,
            "slippage_bps": 50,
            "platform_fee_bps": 0
          }
        });

        assert_eq!(
            expect_msg_detail,
            serde_json::to_value(&msg_detail[3]).unwrap()
        );
        // Address Lookup Table Account(s)
        let alt = message.address_table_lookups.unwrap();
        let alt_base58 = alt
            .iter()
            .map(|a| bitcoin::base58::encode(a.account_key.value.as_slice()))
            .collect::<Vec<String>>();
        assert_eq!(
            alt_base58,
            vec![
                "qKd3QpjMTSr7wC63LLt7A5u2S7N6FKdf3KsdXk5rbrq".to_string(),
                "FucMYjkfiJo4G9fbhQuc4cwpXnyhQRLRNwJzBmxRHryr".to_string(),
                "5U9kcysnaz9Ja8aQZgtQdwd5qgSyqz8WBXZ9RPPMibjW".to_string(),
            ]
        );
        // parse instructions
        let instruction = &message.instructions[3];
        assert_eq!(
            hex::encode(&instruction.data),
            "c1209b3341d69c810f03000000266400012664010226640203c2ce010000000000d1410d0000000000320000"
        );
        let jupiter_instruction =
            JupiterInstructions::dispatch(instruction.data.as_slice()).unwrap();
        println!("{:?}", jupiter_instruction);
    }

    #[test]
    fn test_jupiter_usdc_to_sol_raw_tx_parse_3() {
        //https://solscan.io/tx/4ApbuHM1rBPbBPbNN5Qp1wARRyK8rK58no73eZNrMNh7sv2HzAnZRsYBfDyrWwV2N9V996JbJWa4VvoHc4KtbLFp
        //  curl https://api.mainnet-beta.solana.com -X POST -H "Content-Type: application/json" -d '
        // {
        //   "jsonrpc": "2.0",
        //   "id": 1,
        //   "method": "getTransaction",
        //   "params": [
        //     "4ApbuHM1rBPbBPbNN5Qp1wARRyK8rK58no73eZNrMNh7sv2HzAnZRsYBfDyrWwV2N9V996JbJWa4VvoHc4KtbLFp",
        //     {
        //       "encoding": "base58",
        //       "maxSupportedTransactionVersion": 0
        //     }
        //   ]
        // }'
        let solana_transaction_base58 = "VofQgHuHCBz3GSJ8ZQnApbNkhmDNw7FQXbv3bipiKueqbdFmLkv3X3GNjDpzMSVrnHDJxikhAaVMyGte3VR38vLPyqG2HN9ZmPFYzmdyfN9GYyGLuN3XSRvRo7qH8UawGEodeSmrCRr2KtQBxFeiK6vG3T9JJPU3R89LXVZhht3diDh5tLN66KtVkJFD58hTxqxZwJhddtCTgm4eznMGmwG9tZ8GoTJVTt45NMSkDzHy8nA9N49BkDNENyrjw22eHva4aSWwasgme43GH8hUNySQqhRWgNmzijoRdQP8wkg6t64dFFe667Koz61w3Cq5L1pxa88z1qRvTui23nemu6DztPrPthVdGP9v5WcUdnUnNUuuSA9nyJZzEPBPJccmecyK2fK24bXDh9uz9rnQkwMaMh74YvtrRP6hRgrtVyvA2ViYBaANrkQRmAq3kr2oAhb2zG5xdESviDycwUTHJZ7dpbSQpPmD82z33XArymEkqjB5U6DHqAZsZhiceKdb2J7NqhjXvridDBh4SDqecfX3WRFTFiAXhnYBT823aTj2F22hTqZ6qJ45zGDrfoBM17YN1unTk8w3wVCq2wtXb3tuiVvpddGGL93tqfrbKNEZTpi5m1doMVYCJmNSPUmj8vnU9bBb5hp2XSkrJ1XYCpaHBcD3nRbKqcZ4Z4sNoc5eDp24JrA77UKv4MRmoLMXGYbKqa9n3UPv1XXf9NhR5iS4aETZ4Yjn8i5yLujdUNmk8xz96LpYDF5hi6xCuQKsvX2rjBrgUP2N7o8KFvK5Aywb2qCozAfZ6Jbo1zBojeNNM3WWQQfD2BBfpSEVRo37Gf2cfcTot9efTx2m5LeXCbvGSkCaCCwM5VwTF2T2SuvZGdPcAeqBoVgdNWUAsaDj7dPVRrLsgPAYhiJG7V6NjA8duczwFUXM83hTvJ4vRZhrqdc1485dMzoCSaS836r3pDkd5em3pzVPKTMFYbjCavYs8cQ7cmFLXEKp6dHvessPrFHNzTn1JQ7tQmnFyM8Nezv6xP6eNbd1VLGhF9uKjjVVpeN11xCsxpDbcHk9mk6gQ8c4P4uB5YxbnhAWdGjVqCYLcKQZPLrrRpFk25JKXn9P1kWEec8SEttw9rKVTo26hJcPyoW5fHEge9p4uDGZcTo2BixQYRP3PpWW5nguFtY7pJK13bYqkdrcGtFvwrwNSkDrbgJ6cxTHrfFtPknuZC7Eu5BXdGfejoKyoWGGiyF5YZPutL9haUri73ivMVYUASyFndXLe8Ah5oPATUutopuneDusWxhTQuCLXwP9XhQMiwCVhp5suYDdtGuMF6RY7cGJnWMXE3iWvvHmfrQJMPJGBU54DToTr5aGhR3obQ9g7r4BQ46VxuMypm12DTdTLje2idPhYhz4bDEr971iTCS5wFNSQVfXh1bX4mrnmogqK8eWJcw3PKRsTNjb67ECAaRyZUiNPYvgZPNTdebwFr6eP3WFJL7AGgn9FkM8gGyLYkLqcRNo3Z11yzQd48Mp6bDuUZM";
        let solana_transaction_bytes = bitcoin::base58::decode(solana_transaction_base58).unwrap();
        // get signatures length
        let sig_num = solana_transaction_bytes[0];
        assert_eq!(1, sig_num);
        // eat sig_num
        let solana_transaction_bytes = &solana_transaction_bytes[1..];

        // message
        let mut message = solana_transaction_bytes[64..].to_vec();
        // message
        let message = Message::read(&mut message).unwrap();
        // message detail
        let msg_detail = message.to_program_details().unwrap();
        let expect_msg_detail = json!({
          "program": "JupiterV6",
          "method": "SharedAccountsRoute",
          "accounts": [
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "GP8StUXNYSZjPikyRsvkTbvRV1GBxMErb59cpeCJnDf1",
            "3w1iMvjKGxpbGaaSekNUsZBcVKERg2BCsUZMGrjcTMsj",
            "GrfQTEskA8ZP2eNorbRogpw5DFGNEHBiZGHE2EiGHDqm",
            "HoBCz6z9AG92GGozMWEkBPE9UhQWGZ5cXhYcjoGJvwP2",
            "GyY4VgEpJQhiKZRAJJmoM4hv5Q2xC4pvX68MGrGidxyG",
            "5U7yDrr8hx4wuSsgsmXi3zte1AF7kZ6xWzcJqfS1dau1",
            "EPjFWdd5AufqSSqeM2qN1xzybapC8G4wEGGkZwyTDt1v",
            "So11111111111111111111111111111111111111112",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "D8cy77BBepLMngZx6ZukaTff5hCt1HrWyKk3Hnd9oitf",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "PhoeNiXZ8ByJGLkxNfZRnkUfjvmuYqLR89jjFHGqdXY",
            "7aDTsspkQNGKmrexAN7FLx9oxU3iPczSSvHNggyuqYkR",
            "AbJCZ9TAJiby5AY3cHcXS2gUdENC6mtsm6m7XpC2ZMvE",
            "GP8StUXNYSZjPikyRsvkTbvRV1GBxMErb59cpeCJnDf1",
            "BkB26Mm5Xm1ENXJWCRttrJdSzstcCEyz46SuJQ4ynNs",
            "HoBCz6z9AG92GGozMWEkBPE9UhQWGZ5cXhYcjoGJvwP2",
            "5FhnYa75QKfMkPjBCrM7iucf2wMBNzHE2chyyTUfJEqj",
            "FYgSJ1AxY8Ax2Pczj5mQKuH36Y2eKjrvko7rdyoyXtpx",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "CAMMCzo5YL8w4VFF8KVHrK22GGUsp5VTaW7grrKgrWqK",
            "GP8StUXNYSZjPikyRsvkTbvRV1GBxMErb59cpeCJnDf1",
            "E64NGkDLLCdQ2yFNPcavaKptrEgmiQaNykUuLC1Qgwyp",
            "AH12x44P1D4TEdHvBfreRWgeqygopz4QHmor2Yzfk3Yj",
            "BkB26Mm5Xm1ENXJWCRttrJdSzstcCEyz46SuJQ4ynNs",
            "GyY4VgEpJQhiKZRAJJmoM4hv5Q2xC4pvX68MGrGidxyG",
            "39jfKQTvWgGN4q8eegKVXupbfma1PthGU9FurRxTArZN",
            "Es4qbeeF4YCfFdgAQb57ZD98Eeb86dfDYuBtBxBNcosq",
            "DU4wZSUtB16hnYrHXyjdj9GtEGoCywSMdK2ZeLCapJwe",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "3E2UTxUKytyS27wmY6ZjvS4SyGvKLEJV7ZSVeduVxFzX",
            "6xMH1EsR64437Z3x3mKtpkfNBU5wXFV4hoZPKcwR2QYj",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4"
          ],
          "args": {
            "id": 14,
            "route_plan": [
              {
                "swap": {
                  "Phoenix": {
                    "side": "Bid"
                  }
                },
                "percent": 100,
                "input_index": 0,
                "output_index": 1
              },
              {
                "swap": "RaydiumClmm",
                "percent": 100,
                "input_index": 1,
                "output_index": 2
              }
            ],
            "in_amount": 55963,
            "quoted_out_amount": 426684,
            "slippage_bps": 2000,
            "platform_fee_bps": 0
          }
        });

        assert_eq!(
            expect_msg_detail,
            serde_json::to_value(&msg_detail[3]).unwrap()
        );
    }

    #[test]
    fn test_jupiter_usdc_to_sol_raw_tx_parse_4() {
        // https://solscan.io/tx/4CsSqnRV74cP7p9SnU27H72Kv6QGkSPE7fCbpvUdofwNKL7GDjdGFPU6VgtmRuZ68DHgYiGccRAwNCoUj9Ns6UPy
        //  curl https://api.mainnet-beta.solana.com -X POST -H "Content-Type: application/json" -d '
        // {
        //   "jsonrpc": "2.0",
        //   "id": 1,
        //   "method": "getTransaction",
        //   "params": [
        //     "4CsSqnRV74cP7p9SnU27H72Kv6QGkSPE7fCbpvUdofwNKL7GDjdGFPU6VgtmRuZ68DHgYiGccRAwNCoUj9Ns6UPy",
        //     {
        //       "encoding": "base58",
        //       "maxSupportedTransactionVersion": 0
        //     }
        //   ]
        // }'
        // 56hfnf23iBSXDk3w5fh3ARzfEsPt8A168ypCmtk8K5AcGpEUiuVnBYQSCrxNfnX2m8cKAiHW2x4SvoPp4MCfqqwQYEh6kKrcLAqCFJycaLhURevWXsGTiDE82qUWXNJZcFceKaGuN9799t61LS1X4L8LkPaRgwFTykHYh2QeCaR8cp6JzFyrPfibH2Jma9iyWPhmAnQkA6nw2JAKdWnBeUqfjWATHkZL98Eui9c8jedGFq4ZirhdT6dyzxoL8AP6Yys6fPMKWNuRKsYfNDKpR6u9wQ5xzjHBiTk8ughDceCzsvQP8DAk48AcBSmSsAwwwxcZ4qbSFXQxXncNATBxcpboaS8hPmrPmAuGLuUQ9ga6YBCNG8Wu1QPm6EdmWXUxJCgcVAbnT77oYwyQLE6VAUpBwEtYgipPeS7GYoSwJ4vgUKsWscAHMS1RWRA4HEKi8JCQPhhQAAhT9mgCvKzY8hQyE4Cp1HH2Q53eya22wwtqeauqgZqDcNxZPkn2Tu3CH4EgqvrukdHNhprKGCZpVjWAJhrpmt65UFN45LByeShMXebJNUSXzv5mjC9wJTpcUFYLi4MZZ8AyXgxoEvotemKLdTnEJX3uaqxxbscXYiMKZu1q8URKn2ZYVv9bsHt2JRedGLZCGCMAiGZjTX9CUFPxECpUMmqBzPr8qxd9xHB6oyR2YQtWKe7TJ5JKfdrrpNvToLMu88JS8GRE9CECpeAk1FMwbBjzr3tozYfMU8BHd5Uaj3MeDgR22fyJgCMaR3Po5bmrjdfkyRftUJK5NNAzKXhWQayrcRdgAPavAw9GVkum4gJAf2WBSQMnsCPgmDUMyLjhJsHcSYgb7zhHrJxmHkSSvQFJkkyhUTVSMxYpNq5BPrxhfHYF9cJtw86dMFed5MaU75mddidqV5c6dD9NFRNtKSQv6fhh4TvnAQHiYn4KdCM7YUqofRkVWUMASxRDBNvAo2bthU8L7HHRrPJA9o5KmHS8zHYYRhp5yhKtNY4aDS9JRwYPutbmDuAfyH12Kh11Sf62pHeUXB9dLAbM3DFL6L11fZu4Ev451dc8cT3EAq8N9mfuDcuzqCKnfXZMUM96LYk71ASt7V3BPAChwyEnzW7Mk46uMFmeM7fqWXmDk
        let solana_transaction_base58 = "56hfnf23iBSXDk3w5fh3ARzfEsPt8A168ypCmtk8K5AcGpEUiuVnBYQSCrxNfnX2m8cKAiHW2x4SvoPp4MCfqqwQYEh6kKrcLAqCFJycaLhURevWXsGTiDE82qUWXNJZcFceKaGuN9799t61LS1X4L8LkPaRgwFTykHYh2QeCaR8cp6JzFyrPfibH2Jma9iyWPhmAnQkA6nw2JAKdWnBeUqfjWATHkZL98Eui9c8jedGFq4ZirhdT6dyzxoL8AP6Yys6fPMKWNuRKsYfNDKpR6u9wQ5xzjHBiTk8ughDceCzsvQP8DAk48AcBSmSsAwwwxcZ4qbSFXQxXncNATBxcpboaS8hPmrPmAuGLuUQ9ga6YBCNG8Wu1QPm6EdmWXUxJCgcVAbnT77oYwyQLE6VAUpBwEtYgipPeS7GYoSwJ4vgUKsWscAHMS1RWRA4HEKi8JCQPhhQAAhT9mgCvKzY8hQyE4Cp1HH2Q53eya22wwtqeauqgZqDcNxZPkn2Tu3CH4EgqvrukdHNhprKGCZpVjWAJhrpmt65UFN45LByeShMXebJNUSXzv5mjC9wJTpcUFYLi4MZZ8AyXgxoEvotemKLdTnEJX3uaqxxbscXYiMKZu1q8URKn2ZYVv9bsHt2JRedGLZCGCMAiGZjTX9CUFPxECpUMmqBzPr8qxd9xHB6oyR2YQtWKe7TJ5JKfdrrpNvToLMu88JS8GRE9CECpeAk1FMwbBjzr3tozYfMU8BHd5Uaj3MeDgR22fyJgCMaR3Po5bmrjdfkyRftUJK5NNAzKXhWQayrcRdgAPavAw9GVkum4gJAf2WBSQMnsCPgmDUMyLjhJsHcSYgb7zhHrJxmHkSSvQFJkkyhUTVSMxYpNq5BPrxhfHYF9cJtw86dMFed5MaU75mddidqV5c6dD9NFRNtKSQv6fhh4TvnAQHiYn4KdCM7YUqofRkVWUMASxRDBNvAo2bthU8L7HHRrPJA9o5KmHS8zHYYRhp5yhKtNY4aDS9JRwYPutbmDuAfyH12Kh11Sf62pHeUXB9dLAbM3DFL6L11fZu4Ev451dc8cT3EAq8N9mfuDcuzqCKnfXZMUM96LYk71ASt7V3BPAChwyEnzW7Mk46uMFmeM7fqWXmDk";
        let solana_transaction_bytes = bitcoin::base58::decode(solana_transaction_base58).unwrap();
        // get signatures length
        let sig_num = solana_transaction_bytes[0];
        assert_eq!(1, sig_num);
        // eat sig_num
        let solana_transaction_bytes = &solana_transaction_bytes[1..];

        // message
        let mut message = solana_transaction_bytes[64..].to_vec();
        // message
        let message = Message::read(&mut message).unwrap();
        // print account
        let message_normal_accounts = message.accounts.clone();
        let message_alt = message.address_table_lookups.clone().unwrap();
        let mut all_accounts: Vec<String> = vec![];
        for account in message_normal_accounts.clone() {
            all_accounts.push(bitcoin::base58::encode(account.value.as_slice()));
        }
        for account in message_alt {
            all_accounts.push(bitcoin::base58::encode(
                account.account_key.value.as_slice(),
            ));
        }
        // message detail
        let msg_detail = message.to_program_details().unwrap();
        let expect_msg_detail = json!({
          "program": "JupiterV6",
          "method": "SharedAccountsRoute",
          "accounts": [
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "69yhtoJR4JYPPABZcSNkzuqbaFbwHsCkja1sP1Q2aVT5",
            "3w1iMvjKGxpbGaaSekNUsZBcVKERg2BCsUZMGrjcTMsj",
            "GrfQTEskA8ZP2eNorbRogpw5DFGNEHBiZGHE2EiGHDqm",
            "E6LAwCLSHLkDCoMXZPtnDtpcvCYWcs3ZZLHLreiFwjUi",
            "qqdJ4z1yu4sTbAitwXZsGNDoGZFgL2HfVKSVwAXWCfq",
            "5U7yDrr8hx4wuSsgsmXi3zte1AF7kZ6xWzcJqfS1dau1",
            "DBzy7bCEPMqkkR4kjaHPN5nEYUQjfWn1r1aFzHf8rzor#19",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#107",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "D8cy77BBepLMngZx6ZukaTff5hCt1HrWyKk3Hnd9oitf",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#106",
            "DBzy7bCEPMqkkR4kjaHPN5nEYUQjfWn1r1aFzHf8rzor#45",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#106",
            "DBzy7bCEPMqkkR4kjaHPN5nEYUQjfWn1r1aFzHf8rzor#50",
            "DBzy7bCEPMqkkR4kjaHPN5nEYUQjfWn1r1aFzHf8rzor#21",
            "E6LAwCLSHLkDCoMXZPtnDtpcvCYWcs3ZZLHLreiFwjUi",
            "3b74Vnu1KTVXv2Zn5HY9sGsc1GczLKteDjmwHuFNeBLG",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#112",
            "DBzy7bCEPMqkkR4kjaHPN5nEYUQjfWn1r1aFzHf8rzor#19",
            "DBzy7bCEPMqkkR4kjaHPN5nEYUQjfWn1r1aFzHf8rzor#44",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#106",
            "69yhtoJR4JYPPABZcSNkzuqbaFbwHsCkja1sP1Q2aVT5",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#110",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#106",
            "DBzy7bCEPMqkkR4kjaHPN5nEYUQjfWn1r1aFzHf8rzor#20",
            "DBzy7bCEPMqkkR4kjaHPN5nEYUQjfWn1r1aFzHf8rzor#49",
            "DBzy7bCEPMqkkR4kjaHPN5nEYUQjfWn1r1aFzHf8rzor#47",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#106",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#109",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#106",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#105",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#111",
            "3b74Vnu1KTVXv2Zn5HY9sGsc1GczLKteDjmwHuFNeBLG",
            "qqdJ4z1yu4sTbAitwXZsGNDoGZFgL2HfVKSVwAXWCfq",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#112",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#107",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#108",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#106",
            "69yhtoJR4JYPPABZcSNkzuqbaFbwHsCkja1sP1Q2aVT5",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#110",
            "DFE3RxMMs7wiKjWMCzj4Nm47dDsoqpHFS44XhKeFUkPK#106",
            "7heB4bc7sT2wMC1ag2Jd2QUs1sU6FqBDhA74huTQyCK2",
            "5oviiScsHu3mGNvdYQGh6qnhaf8nTmHH1LPuzGUr2NqZ",
            "DTWWiEC8sFVrAnLj861wnEwuBkX4onpZNc2VdL1vTDgs",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4"
          ],
          "args": {
            "id": 10,
            "route_plan": [
              {
                "swap": "MeteoraDlmm",
                "percent": 100,
                "input_index": 0,
                "output_index": 1
              },
              {
                "swap": "MeteoraDlmm",
                "percent": 100,
                "input_index": 1,
                "output_index": 2
              }
            ],
            "in_amount": 55962,
            "quoted_out_amount": 425253,
            "slippage_bps": 500,
            "platform_fee_bps": 0
          }
        });

        assert_eq!(
            expect_msg_detail,
            serde_json::to_value(&msg_detail[3]).unwrap()
        );
    }

    #[test]
    fn test_jupiter_usdc_to_sol_raw_tx_parse_5() {
        // https://solscan.io/tx/3Sv55Vrz2GF1MNbE7RbKRx59S4E83VHeMFEzZYCp1nbSBu6xc5CSBR5bvsLdpoWjCsEzSWZFKsquvm4Kr7nSaSxx
        //  curl https://api.mainnet-beta.solana.com -X POST -H "Content-Type: application/json" -d '
        // {
        //   "jsonrpc": "2.0",
        //   "id": 1,
        //   "method": "getTransaction",
        //   "params": [
        //     "3Sv55Vrz2GF1MNbE7RbKRx59S4E83VHeMFEzZYCp1nbSBu6xc5CSBR5bvsLdpoWjCsEzSWZFKsquvm4Kr7nSaSxx",
        //     {
        //       "encoding": "base58",
        //       "maxSupportedTransactionVersion": 0
        //     }
        //   ]
        // }'

        // 36erfEswkMaFebifxd26DfW6KEjSF3Q68SWYxx6zqC6NWpxmXZ56Q1jY3ph8wn9wm6FdZC72QTQ2pTr1SrTguAZCn63D6cQ8JMtZSwh39qVrFPnFt9oFqRmnhF5mJQo61ogKfindU111iQ8hv7eAU29CHG3Eg6ZXcuypWjk5NEsauqXZFfqFJqRZBNSrqyvQQiBkDEACA9LY5Bjw1L7Trr4crzD4jsjPyaszVFXQTrZqxLFi8yw2PPHoSkQ6oGCahB3WzfJSk1ChSrL1UDaaxnEwkxaNT1GmaDpCVwzqauBQrDJrSeDrBaX3hEvo1QeL9DaBnwAskqBFQA9h8AYHMLBYZJMKKrTh25ZWJP1AhZf7wikneM31FAGHqC68XLdwrEefAmUaSvJxffrryvv7FCDTypB8osWPATmb2rwaWAzfsRy2LBVSyVgJzjhrqnBoaJxc2CPZpt1wfEe9u73NsDyEEVb8Bfe792R6riZqeoBNtrTkhfja2qvVZELAier93spUmiVwqRmq1juzHDfgj7scgTE9GuLqb96TKPSVcWttZwM8rDWPX8S7s2fzid2dnU7zHo9AyMdRR8qGeZ12VXdMuW1DRD4HxgUT5pD1fXJiPQK7wAmsENGLUuAKSZ1m7r4gFW1VLQydqwbXKycPFTAwPQFMLv6B7Jn4KgH7c7ZZwuvi35TsEEgTSTJM5mrSHt2rqkdhiveTxFxWiS8CXSHxKyvhZBKVYzxGuiQmdtEgxZ5DbfKaLfMDmHcA33hgPvDHVXU1D8byPwneP4S6DmpKiRdqrhy4dH6zdReCAR61UwKspF154wjZm2b2nQcoNQA1J4CWHH1HrqR8szk53zRPqTw83nTV9zJgHH99hn67yKYqX14kKsS5c3DCscauAAEN9Wg3vXtsPL8CiuvfjWqt4hZPPWnjCcCvdVEQ4Yicz8UWAJUmvXvo4tMNHEYZsvoYPQndDJtozqo6dSv1JQju3iJghwfPTSXF1bGYB19wwcybkxqtHZsurDs89bph8ctbu74JtgwgzdUKucWZbwRBPWC2EHg4SdAEcKYwcMQa7SGZmSLc95zc3hQBTDtxpX4BwWjLypqvrNs

        let solana_transaction_base58 = "36erfEswkMaFebifxd26DfW6KEjSF3Q68SWYxx6zqC6NWpxmXZ56Q1jY3ph8wn9wm6FdZC72QTQ2pTr1SrTguAZCn63D6cQ8JMtZSwh39qVrFPnFt9oFqRmnhF5mJQo61ogKfindU111iQ8hv7eAU29CHG3Eg6ZXcuypWjk5NEsauqXZFfqFJqRZBNSrqyvQQiBkDEACA9LY5Bjw1L7Trr4crzD4jsjPyaszVFXQTrZqxLFi8yw2PPHoSkQ6oGCahB3WzfJSk1ChSrL1UDaaxnEwkxaNT1GmaDpCVwzqauBQrDJrSeDrBaX3hEvo1QeL9DaBnwAskqBFQA9h8AYHMLBYZJMKKrTh25ZWJP1AhZf7wikneM31FAGHqC68XLdwrEefAmUaSvJxffrryvv7FCDTypB8osWPATmb2rwaWAzfsRy2LBVSyVgJzjhrqnBoaJxc2CPZpt1wfEe9u73NsDyEEVb8Bfe792R6riZqeoBNtrTkhfja2qvVZELAier93spUmiVwqRmq1juzHDfgj7scgTE9GuLqb96TKPSVcWttZwM8rDWPX8S7s2fzid2dnU7zHo9AyMdRR8qGeZ12VXdMuW1DRD4HxgUT5pD1fXJiPQK7wAmsENGLUuAKSZ1m7r4gFW1VLQydqwbXKycPFTAwPQFMLv6B7Jn4KgH7c7ZZwuvi35TsEEgTSTJM5mrSHt2rqkdhiveTxFxWiS8CXSHxKyvhZBKVYzxGuiQmdtEgxZ5DbfKaLfMDmHcA33hgPvDHVXU1D8byPwneP4S6DmpKiRdqrhy4dH6zdReCAR61UwKspF154wjZm2b2nQcoNQA1J4CWHH1HrqR8szk53zRPqTw83nTV9zJgHH99hn67yKYqX14kKsS5c3DCscauAAEN9Wg3vXtsPL8CiuvfjWqt4hZPPWnjCcCvdVEQ4Yicz8UWAJUmvXvo4tMNHEYZsvoYPQndDJtozqo6dSv1JQju3iJghwfPTSXF1bGYB19wwcybkxqtHZsurDs89bph8ctbu74JtgwgzdUKucWZbwRBPWC2EHg4SdAEcKYwcMQa7SGZmSLc95zc3hQBTDtxpX4BwWjLypqvrNs";
        let solana_transaction_bytes = bitcoin::base58::decode(solana_transaction_base58).unwrap();
        // get signatures length
        let sig_num = solana_transaction_bytes[0];
        assert_eq!(1, sig_num);
        // eat sig_num
        let solana_transaction_bytes = &solana_transaction_bytes[1..];

        // message
        let mut message = solana_transaction_bytes[64..].to_vec();
        // message
        let message = Message::read(&mut message).unwrap();
        // print account
        let message_normal_accounts = message.accounts.clone();
        let message_alt = message.address_table_lookups.clone().unwrap();
        let mut all_accounts: Vec<String> = vec![];
        for account in message_normal_accounts {
            all_accounts.push(bitcoin::base58::encode(account.value.as_slice()));
        }
        for account in message_alt.clone() {
            all_accounts.push(bitcoin::base58::encode(
                account.account_key.value.as_slice(),
            ));
        }
        let msg_detail = message.to_program_details().unwrap();
        let expect_msg_detail = json!({
          "program": "JupiterV6",
          "method": "SharedAccountsRoute",
          "accounts": [
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "HU23r7UoZbqTUuh3vA7emAGztFtqwTeVips789vqxxBw",
            "3w1iMvjKGxpbGaaSekNUsZBcVKERg2BCsUZMGrjcTMsj",
            "GrfQTEskA8ZP2eNorbRogpw5DFGNEHBiZGHE2EiGHDqm",
            "DY6pE7aiDafuk35REZF9p9av3vbV2VQrvdZ4YyB1pZ4C",
            "EaUghZfmuhgtZEEwaZwX5EBroz4WyH8VM5NSXi4tam5A",
            "9i6MNePQf2vMqUkVhk3P943Xad9WtUzNkL3oHk7FrXpJ",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#77",
            "Qq72ct3Kd3zmxZEA7XmL4mRduGgELjGNchAyUa3NQDJ#15",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "D8cy77BBepLMngZx6ZukaTff5hCt1HrWyKk3Hnd9oitf",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#69",
            "2nP8TYJXPZCrMUeST6GNtZ7pgP1Lzi2tc9PnuEqbt4Zs#59",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#69",
            "2nP8TYJXPZCrMUeST6GNtZ7pgP1Lzi2tc9PnuEqbt4Zs#53",
            "2nP8TYJXPZCrMUeST6GNtZ7pgP1Lzi2tc9PnuEqbt4Zs#56",
            "DY6pE7aiDafuk35REZF9p9av3vbV2VQrvdZ4YyB1pZ4C",
            "68nvzx72UsVA9ruAUcHkLp1yRFGovDoQ4gvhwKsxWTES",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#15",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#77",
            "2nP8TYJXPZCrMUeST6GNtZ7pgP1Lzi2tc9PnuEqbt4Zs#61",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#69",
            "HU23r7UoZbqTUuh3vA7emAGztFtqwTeVips789vqxxBw",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#66",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#69",
            "2nP8TYJXPZCrMUeST6GNtZ7pgP1Lzi2tc9PnuEqbt4Zs#58",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#8",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#4",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#16",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#13",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#15",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#6",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#7",
            "H1CX96kfY7vmC8YNP1owiUuSZmTKZ4MBg2dZ935Ukn97",
            "68nvzx72UsVA9ruAUcHkLp1yRFGovDoQ4gvhwKsxWTES",
            "HU23r7UoZbqTUuh3vA7emAGztFtqwTeVips789vqxxBw",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#12",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#9",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#11",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#14",
            "11111111111111111111111111111111",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#10",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#5",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#69",
            "Qq72ct3Kd3zmxZEA7XmL4mRduGgELjGNchAyUa3NQDJ#12",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#69",
            "Qq72ct3Kd3zmxZEA7XmL4mRduGgELjGNchAyUa3NQDJ#14",
            "Qq72ct3Kd3zmxZEA7XmL4mRduGgELjGNchAyUa3NQDJ#13",
            "H1CX96kfY7vmC8YNP1owiUuSZmTKZ4MBg2dZ935Ukn97",
            "EaUghZfmuhgtZEEwaZwX5EBroz4WyH8VM5NSXi4tam5A",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#13",
            "Qq72ct3Kd3zmxZEA7XmL4mRduGgELjGNchAyUa3NQDJ#15",
            "Qq72ct3Kd3zmxZEA7XmL4mRduGgELjGNchAyUa3NQDJ#21",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#69",
            "HU23r7UoZbqTUuh3vA7emAGztFtqwTeVips789vqxxBw",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#66",
            "FPfYfCiGuVsJPwnuTewo1i5PW3aHrXcqCtwroAV7kwWD#69",
            "Qq72ct3Kd3zmxZEA7XmL4mRduGgELjGNchAyUa3NQDJ#20",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4"
          ],
          "args": {
            "id": 11,
            "route_plan": [
              {
                "swap": "MeteoraDlmm",
                "percent": 100,
                "input_index": 0,
                "output_index": 1
              },
              {
                "swap": {
                  "MarcoPolo": {
                    "x_to_y": false
                  }
                },
                "percent": 100,
                "input_index": 1,
                "output_index": 2
              },
              {
                "swap": "MeteoraDlmm",
                "percent": 100,
                "input_index": 2,
                "output_index": 3
              }
            ],
            "in_amount": 200,
            "quoted_out_amount": 1523,
            "slippage_bps": 500,
            "platform_fee_bps": 0
          }
        });

        assert_eq!(
            expect_msg_detail,
            serde_json::to_value(&msg_detail[3]).unwrap()
        );
    }

    #[test]
    fn test_jupiter_usdc_to_sol_raw_tx_parse_6_by_raydium() {
        // https://solscan.io/tx/5gJqzAWhpvMpiv8TxGqXXvjZ4N8PfeWu2Rnt8ynA1BD47Jb58RJih5Bjr2GxLRMBJ3y1FuxXDXhsgamsk3uQ7BDB
        //  curl https://api.mainnet-beta.solana.com -X POST -H "Content-Type: application/json" -d '
        // {
        //   "jsonrpc": "2.0",
        //   "id": 1,
        //   "method": "getTransaction",
        //   "params": [
        //     "5gJqzAWhpvMpiv8TxGqXXvjZ4N8PfeWu2Rnt8ynA1BD47Jb58RJih5Bjr2GxLRMBJ3y1FuxXDXhsgamsk3uQ7BDB",
        //     {
        //       "encoding": "base58",
        //       "maxSupportedTransactionVersion": 0
        //     }
        //   ]
        // }'

        // g4eYaEKCK6ofnqG4tTCSWTihXedPgRL63C99mWeSr9TsNGLVvgxS9E2BR8gFbk2c4kfHPS1E6FuKyXSLZDor217p2wDBeYvJqk1GSTwqDRhRnLdxmBZyGbj5dWFUQKvmbMLWXSYa85iWLa7YgukEQxPy4QcwzmDuXTDbLqGVHwsB3n82sdPbuznf64iL1hS6U5HGwn1ZeFz7KY91UxNFbTrbjynUzhd45WXarj6G4hPDEV5ev5cigt3f9dtSnwVTfasMnp8oHQCk1rsC2QHk4ruEFCX8SRsJqNkE85St6Tv2fe9eGu6PJCfbarojVaANaSpFjBtNLd4xLRufJMDaMjMzrmBSFKTXTkMcthoTMvy4rmCMC95p3eceuisEjYRMRS44mw5DPceBWww1UxwhumAz6CWedRyiJMwMEESJgSWz281aLKt5684XdEQGtryJkbfgfNo5WhCagYzhWDZsq7hUpB1e56k6bws7juLe2pwA1j4ZRZUuHYiZ6F6JXgMC34NXzyWK9Gf1qsnYnHByz7UuQtawEnrF6DND5MPi4KoqTNRwCusmGPr48iDkerUiExQVn72cKFaB4mDsk1si4awD17WkZTMstv5r3jrKoUAqEmJMi9xtXrsZkwa9M1Kqhr1uxhhRrZ71uoZRdd6xBqQLs6Jb3mtKeEDVcX4Gi8BkyrWLxA2LtnseCk68SPduwR3MAPmUHg1PWhSijDP5q2WDTq1ShBBbbD4rzUtM5ezrLfmb7tGvuxbzrocfs2NjN99BbcXD1xNM9YKSFb5TAJz4Cy12skN8j2v3RNcXwkAmqNCn8cx7YKV7hMxxA6wXYQQSQrQS7GgCxrAwYnHQQwbud7PUEQZ2xSQ15PP8WHHExWCFGCBm7bbRJ42nnxsmGEeWmq9W4zGU3kgB4p1LexgKpVqXZ92VPxgNpYJoNhb7PUcz4Ngm8e3Asdyx18zTg7HSSWpnvnZMNV3hcYgDXf978oUD9EExgiqxGEcUvcyVJFiRJDKMmceztZCjXuwBuL4AAfbYaugAVpo3eQLJ4NaLeffcPgwjkb6cVMPy14qTGKm5LsE3HJ6AXT52YRLsMhHimMXk7UggY6tzdmynxNDJu9wu

        let solana_transaction_base58 = "g4eYaEKCK6ofnqG4tTCSWTihXedPgRL63C99mWeSr9TsNGLVvgxS9E2BR8gFbk2c4kfHPS1E6FuKyXSLZDor217p2wDBeYvJqk1GSTwqDRhRnLdxmBZyGbj5dWFUQKvmbMLWXSYa85iWLa7YgukEQxPy4QcwzmDuXTDbLqGVHwsB3n82sdPbuznf64iL1hS6U5HGwn1ZeFz7KY91UxNFbTrbjynUzhd45WXarj6G4hPDEV5ev5cigt3f9dtSnwVTfasMnp8oHQCk1rsC2QHk4ruEFCX8SRsJqNkE85St6Tv2fe9eGu6PJCfbarojVaANaSpFjBtNLd4xLRufJMDaMjMzrmBSFKTXTkMcthoTMvy4rmCMC95p3eceuisEjYRMRS44mw5DPceBWww1UxwhumAz6CWedRyiJMwMEESJgSWz281aLKt5684XdEQGtryJkbfgfNo5WhCagYzhWDZsq7hUpB1e56k6bws7juLe2pwA1j4ZRZUuHYiZ6F6JXgMC34NXzyWK9Gf1qsnYnHByz7UuQtawEnrF6DND5MPi4KoqTNRwCusmGPr48iDkerUiExQVn72cKFaB4mDsk1si4awD17WkZTMstv5r3jrKoUAqEmJMi9xtXrsZkwa9M1Kqhr1uxhhRrZ71uoZRdd6xBqQLs6Jb3mtKeEDVcX4Gi8BkyrWLxA2LtnseCk68SPduwR3MAPmUHg1PWhSijDP5q2WDTq1ShBBbbD4rzUtM5ezrLfmb7tGvuxbzrocfs2NjN99BbcXD1xNM9YKSFb5TAJz4Cy12skN8j2v3RNcXwkAmqNCn8cx7YKV7hMxxA6wXYQQSQrQS7GgCxrAwYnHQQwbud7PUEQZ2xSQ15PP8WHHExWCFGCBm7bbRJ42nnxsmGEeWmq9W4zGU3kgB4p1LexgKpVqXZ92VPxgNpYJoNhb7PUcz4Ngm8e3Asdyx18zTg7HSSWpnvnZMNV3hcYgDXf978oUD9EExgiqxGEcUvcyVJFiRJDKMmceztZCjXuwBuL4AAfbYaugAVpo3eQLJ4NaLeffcPgwjkb6cVMPy14qTGKm5LsE3HJ6AXT52YRLsMhHimMXk7UggY6tzdmynxNDJu9wu";
        let solana_transaction_bytes = bitcoin::base58::decode(solana_transaction_base58).unwrap();
        // get signatures length
        let sig_num = solana_transaction_bytes[0];
        assert_eq!(1, sig_num);
        // eat sig_num
        let solana_transaction_bytes = &solana_transaction_bytes[1..];

        // message
        let mut message = solana_transaction_bytes[64..].to_vec();
        // message
        let message = Message::read(&mut message).unwrap();
        // print account
        let message_normal_accounts = message.accounts.clone();
        let message_alt = message.address_table_lookups.clone().unwrap();
        let mut all_accounts: Vec<String> = vec![];
        for account in message_normal_accounts {
            all_accounts.push(bitcoin::base58::encode(account.value.as_slice()));
        }
        for account in message_alt.clone() {
            all_accounts.push(bitcoin::base58::encode(
                account.account_key.value.as_slice(),
            ));
        }
        let msg_detail = message.to_program_details().unwrap();

        let expect_msg_detail = json!({
          "program": "JupiterV6",
          "method": "Route",
          "accounts": [
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "3w1iMvjKGxpbGaaSekNUsZBcVKERg2BCsUZMGrjcTMsj",
            "GrfQTEskA8ZP2eNorbRogpw5DFGNEHBiZGHE2EiGHDqm",
            "9i6MNePQf2vMqUkVhk3P943Xad9WtUzNkL3oHk7FrXpJ",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "So11111111111111111111111111111111111111112",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "D8cy77BBepLMngZx6ZukaTff5hCt1HrWyKk3Hnd9oitf",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "CAMMCzo5YL8w4VFF8KVHrK22GGUsp5VTaW7grrKgrWqK",
            "3w1iMvjKGxpbGaaSekNUsZBcVKERg2BCsUZMGrjcTMsj",
            "4BLNHtVe942GSs4teSZqGX24xwKNkqU7bGgNn3iUiUpw",
            "EXHyQxMSttcvLPwjENnXCPZ8GmLjJYHtNBnAkcFeFKMn",
            "GrfQTEskA8ZP2eNorbRogpw5DFGNEHBiZGHE2EiGHDqm",
            "9i6MNePQf2vMqUkVhk3P943Xad9WtUzNkL3oHk7FrXpJ",
            "9PeQs7co3NtYnkV2CuWCSC6MXxwrMgHBX1E2qNEUj7MY",
            "G5uMMdPTeaafVVEnp3SLNLARarJXjHd5JaKuG3ojMPig",
            "BidNmgznWp3ERbuemvdPANYmF2ePMHzGvbqjhMpRZYrn",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "8ZwGS7KJJUjpBbYJ1SZdEh7Cgs7ZfWzhvYpaRkr1koD4",
            "BD13Hf2ZThRL3v6wUDcVwhmsK5Eg5qJPR3CRtwkp5o6e",
            "CFKm9oMiyxQXbE1QtTgzpHCBgqpBp4xaZXujhzeHYBG9",
            "4GsyWi1rVeV9RGWjgPZrFUV3xocEp3URzJBchWFzi6Ct",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4"
          ],
          "args": {
            "route_plan": [
              {
                "swap": "RaydiumClmm",
                "percent": 100,
                "input_index": 0,
                "output_index": 1
              }
            ],
            "in_amount": 660232,
            "quoted_out_amount": 5032384,
            "slippage_bps": 500,
            "platform_fee_bps": 0
          }
        });
        assert_eq!(
            expect_msg_detail,
            serde_json::to_value(&msg_detail[3]).unwrap()
        );
    }

    #[test]
    fn test_jupiter_usdc_to_sol_raw_tx_parse_7_by_exactout() {
        // https://solscan.io/tx/5yBGmFNYx8kLnVtbZx8wqegmwbUkE8YruYBAci6Atv7kjohDt99EdLJRsA3atZfdtaK72duZeyZ9VCPJzrvPRMAr

        //  curl https://api.mainnet-beta.solana.com -X POST -H "Content-Type: application/json" -d '
        // {
        //   "jsonrpc": "2.0",
        //   "id": 1,
        //   "method": "getTransaction",
        //   "params": [
        //     "5yBGmFNYx8kLnVtbZx8wqegmwbUkE8YruYBAci6Atv7kjohDt99EdLJRsA3atZfdtaK72duZeyZ9VCPJzrvPRMAr",
        //     {
        //       "encoding": "base58",
        //       "maxSupportedTransactionVersion": 0
        //     }
        //   ]
        // }'

        // 88RG9uof754r6RuMM9RvNZgdqJNWEAW8aciaq2aqEMgTneawKuCDnu8dFbW5kkYUPHTnnNLLMh4AbUrrLXDpKLiQZNfUiy9rsYpiWFcAF6sVzLRYKLXNQqQ1qs2HYtN6XXoq3km7x6JHnXcjdg7ZzQnYcaB2nv6Apc6teRV6Yjr4LdtMAVTH6vQJUGoxTSP6cXDgZNF5tv2hFkxr211htBCiDACUWQxHvooXepTg2x5WTuyUGyzaWqym7jVMvSWZXmsvG2qjm2PLocoK4WdZbsahJ4Cj644nXYRS48tfPHEFBfvDhP4sqchEccxg4yGZ2c9cQRVo5cTJCjcvp5Ahn5WBpQN3jvW2tJ8myJsicyghGyoL3Z7Ex4Uaz5SUUNduNKGWgHb7zhvzXoJHYwzG7NyBdDpffEv1rC6AawG69KJeMsv65dGe74dsxpKR27fQ2qgwivo3UNXW6Se8bdis79rP1fkhYy3FkCM4LYbNsifeao359ajRB9bAnJGECzQMnVbEwJzGq9rhQpGquYKw5sSHgDrhdaR3sUYsCYK88Th2ABLCA8BzssNmQGAaWKiVauCGZTTPzCjQYFvKyTzJXQRvVGeFR5DapLuk4hknrRwXNKuj9enSfsTWzzSnK5VjkAGTP7ntcu33uLuEMtkbxrgZiVjjDCL8wReVRunmZskMDWv5Ts64sKAs5H5vLa99tFFmgYuJd3L4ZBJQfmzdbma25MrUG964xvEAv6L9ThFUBS9zgjLiyPGD3gYkMjdvdhDHiHYHENgWDZhqUnwehPoskqcWxmwUme6WBSu9czSTGp5acxrQDgndVU8sNjGYb6aUXhsd7q2TCD92dtEEVyxpxMbTCSHtmRJAbS9pUhzbytETZbXxP4G996hP8e5S5MNstYAcMcjUEhBBMNtb1y1u1TZNqaGXPXyQBWe3ueK5KKyRZP8TVbu2mJv73HamB5GFgPmG5iyXvfFcp26yL7YqaK5vvhD6dk7dYaMvjJtQWBqKzbAUmqpxvNqvhqHqG9vPeJAtovig4TRqgWpi3zu4qHtyqDWuXibxfrUjjPjoazYWFrasA4apFC2qNi81FEPcYEoLLBNChJwgnYgrFLMTuWDNJe4PVGSqX5EtyrHMk7tswYZPV1iZVV5jmFzzdSquyHSV5UvPf9AFdUNUrecbuDg1RnKpfbPCemAw9oMRH

        let solana_transaction_base58 = "88RG9uof754r6RuMM9RvNZgdqJNWEAW8aciaq2aqEMgTneawKuCDnu8dFbW5kkYUPHTnnNLLMh4AbUrrLXDpKLiQZNfUiy9rsYpiWFcAF6sVzLRYKLXNQqQ1qs2HYtN6XXoq3km7x6JHnXcjdg7ZzQnYcaB2nv6Apc6teRV6Yjr4LdtMAVTH6vQJUGoxTSP6cXDgZNF5tv2hFkxr211htBCiDACUWQxHvooXepTg2x5WTuyUGyzaWqym7jVMvSWZXmsvG2qjm2PLocoK4WdZbsahJ4Cj644nXYRS48tfPHEFBfvDhP4sqchEccxg4yGZ2c9cQRVo5cTJCjcvp5Ahn5WBpQN3jvW2tJ8myJsicyghGyoL3Z7Ex4Uaz5SUUNduNKGWgHb7zhvzXoJHYwzG7NyBdDpffEv1rC6AawG69KJeMsv65dGe74dsxpKR27fQ2qgwivo3UNXW6Se8bdis79rP1fkhYy3FkCM4LYbNsifeao359ajRB9bAnJGECzQMnVbEwJzGq9rhQpGquYKw5sSHgDrhdaR3sUYsCYK88Th2ABLCA8BzssNmQGAaWKiVauCGZTTPzCjQYFvKyTzJXQRvVGeFR5DapLuk4hknrRwXNKuj9enSfsTWzzSnK5VjkAGTP7ntcu33uLuEMtkbxrgZiVjjDCL8wReVRunmZskMDWv5Ts64sKAs5H5vLa99tFFmgYuJd3L4ZBJQfmzdbma25MrUG964xvEAv6L9ThFUBS9zgjLiyPGD3gYkMjdvdhDHiHYHENgWDZhqUnwehPoskqcWxmwUme6WBSu9czSTGp5acxrQDgndVU8sNjGYb6aUXhsd7q2TCD92dtEEVyxpxMbTCSHtmRJAbS9pUhzbytETZbXxP4G996hP8e5S5MNstYAcMcjUEhBBMNtb1y1u1TZNqaGXPXyQBWe3ueK5KKyRZP8TVbu2mJv73HamB5GFgPmG5iyXvfFcp26yL7YqaK5vvhD6dk7dYaMvjJtQWBqKzbAUmqpxvNqvhqHqG9vPeJAtovig4TRqgWpi3zu4qHtyqDWuXibxfrUjjPjoazYWFrasA4apFC2qNi81FEPcYEoLLBNChJwgnYgrFLMTuWDNJe4PVGSqX5EtyrHMk7tswYZPV1iZVV5jmFzzdSquyHSV5UvPf9AFdUNUrecbuDg1RnKpfbPCemAw9oMRH";
        let solana_transaction_bytes = bitcoin::base58::decode(solana_transaction_base58).unwrap();
        // get signatures length
        let sig_num = solana_transaction_bytes[0];
        assert_eq!(1, sig_num);
        // eat sig_num
        let solana_transaction_bytes = &solana_transaction_bytes[1..];

        // message
        let mut message = solana_transaction_bytes[64..].to_vec();
        // message
        let message = Message::read(&mut message).unwrap();
        // print account
        let message_normal_accounts = message.accounts.clone();
        let message_alt = message.address_table_lookups.clone().unwrap();
        let mut all_accounts: Vec<String> = vec![];
        for account in message_normal_accounts {
            all_accounts.push(bitcoin::base58::encode(account.value.as_slice()));
        }
        for account in message_alt.clone() {
            all_accounts.push(bitcoin::base58::encode(
                account.account_key.value.as_slice(),
            ));
        }
        let msg_detail = message.to_program_details().unwrap();
        let expect_msg_detail = json!({
          "program": "JupiterV6",
          "method": "ExactOutRoute",
          "accounts": [
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "3w1iMvjKGxpbGaaSekNUsZBcVKERg2BCsUZMGrjcTMsj",
            "GrfQTEskA8ZP2eNorbRogpw5DFGNEHBiZGHE2EiGHDqm",
            "5U7yDrr8hx4wuSsgsmXi3zte1AF7kZ6xWzcJqfS1dau1",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "EPjFWdd5AufqSSqeM2qN1xzybapC8G4wEGGkZwyTDt1v",
            "So11111111111111111111111111111111111111112",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "D8cy77BBepLMngZx6ZukaTff5hCt1HrWyKk3Hnd9oitf",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "CAMMCzo5YL8w4VFF8KVHrK22GGUsp5VTaW7grrKgrWqK",
            "3w1iMvjKGxpbGaaSekNUsZBcVKERg2BCsUZMGrjcTMsj",
            "9iFER3bpjf1PTTCQCfTRu17EJgvsxo9pVyA9QWwEuX4x",
            "8sLbNZoA1cfnvMJLPfp98ZLAnFSYCFApfJKMbiXNLwxj",
            "GrfQTEskA8ZP2eNorbRogpw5DFGNEHBiZGHE2EiGHDqm",
            "5U7yDrr8hx4wuSsgsmXi3zte1AF7kZ6xWzcJqfS1dau1",
            "6mK4Pxs6GhwnessH7CvPivqDYauiHZmAdbEFDpXFk9zt",
            "6P4tvbzRY6Bh3MiWDHuLqyHywovsRwRpfskPvyeSoHsz",
            "3MsJXVvievxAbsMsaT6TS4i6oMitD9jazucuq3X234tC",
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
            "3oGasDzyKWb7JWF5RPbdRUtwMhjmYLWGqeNKZZsgycMr",
            "DoPuiZfJu7sypqwR4eiU7C5TMcmmiFoU4HaF5SoD8mRy",
            "8ywGkaNUa8KZBsYjWpVnr7Gcy7PvppTLHYETC9nq53ZW",
            "9ugCm9KeJ4CePY7tr5MEByUX1tehTnD8H6sahJoNjYNf",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4"
          ],
          "args": {
            "route_plan": [
              {
                "swap": "RaydiumClmm",
                "percent": 100,
                "input_index": 0,
                "output_index": 1
              }
            ],
            "out_amount": 100000,
            "quoted_in_amount": 13511,
            "slippage_bps": 2000,
            "platform_fee_bps": 0
          }
        });

        assert_eq!(
            expect_msg_detail,
            serde_json::to_value(&msg_detail[3]).unwrap()
        );
    }

    // https://solscan.io/tx/2mCi5xkVaPxgphtu7z6R1qUt7dmXkFUCHBrepCjSGbT8LjqYnoq9hB7NFtjLvSZ5WGu7cMtsYNbFVpRxEnTb3dRN
    #[test]
    fn test_jupiter_mnde_to_wif_raw_tx_parse_8_by_shared_accounts_exactout_route() {
        // https://solscan.io/tx/2mCi5xkVaPxgphtu7z6R1qUt7dmXkFUCHBrepCjSGbT8LjqYnoq9hB7NFtjLvSZ5WGu7cMtsYNbFVpRxEnTb3dRN
        //  curl https://api.mainnet-beta.solana.com -X POST -H "Content-Type: application/json" -d '
        // {
        //   "jsonrpc": "2.0",
        //   "id": 1,
        //   "method": "getTransaction",
        //   "params": [
        //     "2mCi5xkVaPxgphtu7z6R1qUt7dmXkFUCHBrepCjSGbT8LjqYnoq9hB7NFtjLvSZ5WGu7cMtsYNbFVpRxEnTb3dRN",
        //     {
        //       "encoding": "base58",
        //       "maxSupportedTransactionVersion": 0
        //     }
        //   ]
        // }'

        // GLVQD9v1twbyRFWRgnc7o6fe4oUD8WZc8gfJGQZj6MDBHHaDaGeVkdZXRPNouB7B5JMLaGtUXHxr4GiR4G5Wk9nEYSKgjt3nw6i18ASFuywDmESHecFvyNeXypKUnjMGXKy7FmstzAXnMfjPG7wN97fgLHhNbgfhJ5CpfTv2kFKDXpeFA4d13nALTRi6JDjV6YhsiHP95Bdtee3otwpi7ZucTAV2DMdtZu22uwq2V9sp4n3BjEtDf85GHxXsQFp7oW5ar7zazeesPDPyRQYeUsSopdNv4E484cRwpZQbKnAm49aD1UtJd6VcxFBe2bJWQnTh1H8MeGrqruVvEU9ubg4yJFhrYJiT9HEtFPwaCXQHVb57D76KQWYakgddDfSjasR99PEapn68cWBqzyFNCScvx1HZnkLwDcoUweoF3dhN8sS1PXZsfUeRtWoRBMeB9ZgpTbcNk9njwy96ReangLybDntMK57YKy8VD8s25iWSFpi9g1axdH2cRDmYjTsJB65TwkuZaPMUCXQmTxeCQJvQUGYq7k7XrEeACsSvcQDWb7HeER89QpUxDaMwmQi6BM6WnwDD2yXaj4FExvbT9yqFLSWSi4Ksi6qCpHhNXTsmwnQtHHEEoxfhJePAwTSundSKL7j7gvFbgJ3QkLVCNNNiGMMjprEjsn2DuUh6sGSjBpW17R19gKPD8ir4kTHDi4PpNnUshMmHntdGPxdkD4YNwncUEHozR1QikpcAEGufohbMWdHjbFX1GpJwVt8LFTDAmuasM9nuUg6Htrs2asEaSQ1Dm123Ko44nRAEtuhyDiZ2oif5NLroiTXdBiFFA8pxhv1qbTh42KUQBaCNnxAvQrygNEPBwm4MxMcRN3Kaqz8N4os56eHjqmAS4HR7s9oWprp6q29875x14VSjEH6cR6j4SyYNdy7biYfhcFy7bT21WyFCV1Thn44NCA1oASvqmcTxCYmHnN9MVuus1R24LcFznnBo5pzSWGTGrn58GPxtavyptT2cYHJphFAG5UcFbkyJczs7sJiMTh57ZFP9z7qTfiJmYwY4Q43X5KVhSY83DLsBL9AAy6XbxMQqU15HFMw51xqGAP7TbAzpg18VKSjFeQ4Zv9QPzX7sJxLmSY7evHqqK1MynoTXCg2XVzZJ1MbKEPyecgdoJmUWRu7BmUcgpEf5mactrGGCbb3xJNQbdfkEiWsYh3sk4tZ4izhVnxGM4XGq5jMPE9kvcRKAhhh3wEwv72n4sx2UaEahCbG5r

        let solana_transaction_base58 = "GLVQD9v1twbyRFWRgnc7o6fe4oUD8WZc8gfJGQZj6MDBHHaDaGeVkdZXRPNouB7B5JMLaGtUXHxr4GiR4G5Wk9nEYSKgjt3nw6i18ASFuywDmESHecFvyNeXypKUnjMGXKy7FmstzAXnMfjPG7wN97fgLHhNbgfhJ5CpfTv2kFKDXpeFA4d13nALTRi6JDjV6YhsiHP95Bdtee3otwpi7ZucTAV2DMdtZu22uwq2V9sp4n3BjEtDf85GHxXsQFp7oW5ar7zazeesPDPyRQYeUsSopdNv4E484cRwpZQbKnAm49aD1UtJd6VcxFBe2bJWQnTh1H8MeGrqruVvEU9ubg4yJFhrYJiT9HEtFPwaCXQHVb57D76KQWYakgddDfSjasR99PEapn68cWBqzyFNCScvx1HZnkLwDcoUweoF3dhN8sS1PXZsfUeRtWoRBMeB9ZgpTbcNk9njwy96ReangLybDntMK57YKy8VD8s25iWSFpi9g1axdH2cRDmYjTsJB65TwkuZaPMUCXQmTxeCQJvQUGYq7k7XrEeACsSvcQDWb7HeER89QpUxDaMwmQi6BM6WnwDD2yXaj4FExvbT9yqFLSWSi4Ksi6qCpHhNXTsmwnQtHHEEoxfhJePAwTSundSKL7j7gvFbgJ3QkLVCNNNiGMMjprEjsn2DuUh6sGSjBpW17R19gKPD8ir4kTHDi4PpNnUshMmHntdGPxdkD4YNwncUEHozR1QikpcAEGufohbMWdHjbFX1GpJwVt8LFTDAmuasM9nuUg6Htrs2asEaSQ1Dm123Ko44nRAEtuhyDiZ2oif5NLroiTXdBiFFA8pxhv1qbTh42KUQBaCNnxAvQrygNEPBwm4MxMcRN3Kaqz8N4os56eHjqmAS4HR7s9oWprp6q29875x14VSjEH6cR6j4SyYNdy7biYfhcFy7bT21WyFCV1Thn44NCA1oASvqmcTxCYmHnN9MVuus1R24LcFznnBo5pzSWGTGrn58GPxtavyptT2cYHJphFAG5UcFbkyJczs7sJiMTh57ZFP9z7qTfiJmYwY4Q43X5KVhSY83DLsBL9AAy6XbxMQqU15HFMw51xqGAP7TbAzpg18VKSjFeQ4Zv9QPzX7sJxLmSY7evHqqK1MynoTXCg2XVzZJ1MbKEPyecgdoJmUWRu7BmUcgpEf5mactrGGCbb3xJNQbdfkEiWsYh3sk4tZ4izhVnxGM4XGq5jMPE9kvcRKAhhh3wEwv72n4sx2UaEahCbG5r";
        let solana_transaction_bytes = bitcoin::base58::decode(solana_transaction_base58).unwrap();
        // get signatures length
        let sig_num = solana_transaction_bytes[0];
        assert_eq!(1, sig_num);
        // eat sig_num
        let solana_transaction_bytes = &solana_transaction_bytes[1..];

        // message
        let mut message = solana_transaction_bytes[64..].to_vec();
        // message
        let message = Message::read(&mut message).unwrap();
        // print account
        let message_normal_accounts = message.accounts.clone();
        let message_alt = message.address_table_lookups.clone().unwrap();
        let mut all_accounts: Vec<String> = vec![];
        for account in message_normal_accounts {
            all_accounts.push(bitcoin::base58::encode(account.value.as_slice()));
        }
        for account in message_alt.clone() {
            all_accounts.push(bitcoin::base58::encode(
                account.account_key.value.as_slice(),
            ));
        }
        let msg_detail = message.to_program_details().unwrap();

        let expect_msg_detail = json!({
          "program": "JupiterV6",
          "method": "SharedAccountsExactOutRoute",
          "accounts": [
            "3t6q9eS6Hx1xbaA6MF7T5ujuuK2Tc7RUQ14QUxEHgrur#5",
            "69yhtoJR4JYPPABZcSNkzuqbaFbwHsCkja1sP1Q2aVT5",
            "3w1iMvjKGxpbGaaSekNUsZBcVKERg2BCsUZMGrjcTMsj",
            "DNF1LVkvg4ZiefjwMBiHdX5RTt4BZhLzN6H9h7rm719b",
            "AyBXy1mCZa2dV5TKhSQsnrNCXdDRu8Z9MtPCD1EXbdE7",
            "9JyoFvDbKRmVGY11dhoBAVyBX2uDgxbNcaxWNoyaa4v1",
            "9JvT3avL4jxnnSQyqbUs2GsRgxBuYR8YUwwUFiJzwjp3",
            "MNDEFzGvMt87ueuHvVU9VcTqsAP5b3fTGPsHuuPA5ey",
            "EKpQGSJtjMFqKZ9KQanSqYXRcF8fBopzLHYxdM65zcjm",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "D8cy77BBepLMngZx6ZukaTff5hCt1HrWyKk3Hnd9oitf",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
            "3t6q9eS6Hx1xbaA6MF7T5ujuuK2Tc7RUQ14QUxEHgrur#1",
            "3t6q9eS6Hx1xbaA6MF7T5ujuuK2Tc7RUQ14QUxEHgrur#5",
            "4sWDaxMsX2ZMKmk9m97u8fZtcg37rVePE1UFicCkd69h#120",
            "3t6q9eS6Hx1xbaA6MF7T5ujuuK2Tc7RUQ14QUxEHgrur#2",
            "4sWDaxMsX2ZMKmk9m97u8fZtcg37rVePE1UFicCkd69h#120",
            "4sWDaxMsX2ZMKmk9m97u8fZtcg37rVePE1UFicCkd69h#121",
            "4sWDaxMsX2ZMKmk9m97u8fZtcg37rVePE1UFicCkd69h#119",
            "4sWDaxMsX2ZMKmk9m97u8fZtcg37rVePE1UFicCkd69h#120",
            "4sWDaxMsX2ZMKmk9m97u8fZtcg37rVePE1UFicCkd69h#120",
            "4sWDaxMsX2ZMKmk9m97u8fZtcg37rVePE1UFicCkd69h#120",
            "4sWDaxMsX2ZMKmk9m97u8fZtcg37rVePE1UFicCkd69h#120",
            "4sWDaxMsX2ZMKmk9m97u8fZtcg37rVePE1UFicCkd69h#120",
            "4sWDaxMsX2ZMKmk9m97u8fZtcg37rVePE1UFicCkd69h#120",
            "4sWDaxMsX2ZMKmk9m97u8fZtcg37rVePE1UFicCkd69h#120",
            "4sWDaxMsX2ZMKmk9m97u8fZtcg37rVePE1UFicCkd69h#120",
            "AyBXy1mCZa2dV5TKhSQsnrNCXdDRu8Z9MtPCD1EXbdE7",
            "GuTi2PNxaCBivpLeiLzS8tSqBMHtpTqLe63RNuim5XW7",
            "69yhtoJR4JYPPABZcSNkzuqbaFbwHsCkja1sP1Q2aVT5",
            "4sWDaxMsX2ZMKmk9m97u8fZtcg37rVePE1UFicCkd69h#201",
            "3t6q9eS6Hx1xbaA6MF7T5ujuuK2Tc7RUQ14QUxEHgrur#5",
            "69yhtoJR4JYPPABZcSNkzuqbaFbwHsCkja1sP1Q2aVT5",
            "822jCwCEU2SKw34fappH7NzYNckjaBrTc3SuFj1A9gB4#200",
            "qqdJ4z1yu4sTbAitwXZsGNDoGZFgL2HfVKSVwAXWCfq",
            "822jCwCEU2SKw34fappH7NzYNckjaBrTc3SuFj1A9gB4#246",
            "GuTi2PNxaCBivpLeiLzS8tSqBMHtpTqLe63RNuim5XW7",
            "822jCwCEU2SKw34fappH7NzYNckjaBrTc3SuFj1A9gB4#242",
            "7b97zheCD7PkhC7ZsyVAd7JkpH7681aYRFCiDrCMdGkC",
            "5oNQQZwRS4aCm8vtWL1PzY6mibsjfr8CfUbCF1s9g1v",
            "7kTgVEdmQi7YXt4MjV4tCxUx21VFoYzN6ZqeEWnVwNBD",
            "822jCwCEU2SKw34fappH7NzYNckjaBrTc3SuFj1A9gB4#245",
            "3t6q9eS6Hx1xbaA6MF7T5ujuuK2Tc7RUQ14QUxEHgrur#143",
            "69yhtoJR4JYPPABZcSNkzuqbaFbwHsCkja1sP1Q2aVT5",
            "3t6q9eS6Hx1xbaA6MF7T5ujuuK2Tc7RUQ14QUxEHgrur#138",
            "3t6q9eS6Hx1xbaA6MF7T5ujuuK2Tc7RUQ14QUxEHgrur#142",
            "qqdJ4z1yu4sTbAitwXZsGNDoGZFgL2HfVKSVwAXWCfq",
            "9JyoFvDbKRmVGY11dhoBAVyBX2uDgxbNcaxWNoyaa4v1",
            "3t6q9eS6Hx1xbaA6MF7T5ujuuK2Tc7RUQ14QUxEHgrur#144",
            "3t6q9eS6Hx1xbaA6MF7T5ujuuK2Tc7RUQ14QUxEHgrur#140",
            "3t6q9eS6Hx1xbaA6MF7T5ujuuK2Tc7RUQ14QUxEHgrur#141",
            "3t6q9eS6Hx1xbaA6MF7T5ujuuK2Tc7RUQ14QUxEHgrur#5",
            "CyetpqmSj9qD2TZeW81cnpQgN65DAdihRprXXGZKex5R",
            "3t6q9eS6Hx1xbaA6MF7T5ujuuK2Tc7RUQ14QUxEHgrur#147",
            "C3FkLYeCGFo7SKhVu7UdcK74SYD5h6J2Zzm63nvrWgf2",
            "3t6q9eS6Hx1xbaA6MF7T5ujuuK2Tc7RUQ14QUxEHgrur#225",
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4"
          ],
          "args": {
            "id": 10,
            "route_plan": [
              {
                "swap": "Raydium",
                "percent": 100,
                "input_index": 0,
                "output_index": 1
              },
              {
                "swap": {
                  "Whirlpool": {
                    "a_to_b": false
                  }
                },
                "percent": 100,
                "input_index": 1,
                "output_index": 2
              },
              {
                "swap": "RaydiumClmm",
                "percent": 100,
                "input_index": 2,
                "output_index": 3
              }
            ],
            "out_amount": 10000,
            "quoted_in_amount": 155488677,
            "slippage_bps": 2000,
            "platform_fee_bps": 0
          }
        });

        assert_eq!(
            expect_msg_detail,
            serde_json::to_value(&msg_detail[2]).unwrap()
        );
    }

    #[test]
    fn test_notion_v0_tx_parse() {
        //
        let solana_message_hex = "800100080ed027455eccf0385fc38f217c5bfbb95f711a16fbe782af7b33a97d1f17ebcd79281764ffe753b4a7a87393561a7178e5d7048eabf5302f5b0ac9ff8c2cd40560f06ebde13476f2455bc64dd1d49730033c0bac118dcbf72c38da62354df37a861c98c04915df1902f5c239c753a4bb55932b2e60b713c81bc81be9badfed8e7968e9def8e27815d652d0072a832a901e7960d89173ba8cb1c9450507eb7129f6d82fac025f4fec4efb956ef13dc4145063534dcddc006382f016f49a64eb71818c97258f4e2489f1bb3d1029148e0d830b5a1399daff1084048e7bd8dbe9f8591b8ffe6224eb2dcd8c0558731a23be68b7f64292a77e9b76889a44264468fff3000000000000000000000000000000000000000000000000000000000000000006ddf6e1d765a193d9cbe146ceeb79ac1cb485ed5f5b37913a8cf5857eff00a90306466fe5211732ffecadba72c39be7bc8ce5bbc5f7126b2c439b3a40000000069b8857feab8184fb687f634618c035dac439dc1aeb3b5598a0f00000000001ca4d39964c9cb5f9790d0a12969f60fd9724936284ea4a12daded42ddfa69c5d0479d52dedbf6bc5ecd09d84534a34aea5975043b36fd02b24650bb58443595c973c265a4390cf341062b67c0c2df7225807028c17a4006a4cfea9e5cfebe53f0706060001071e080901010a000502c05c15000a000903d8d600000000000006060002000b0809010106060003000c080901010d280900041b091c1d00050e0f0210111b091f20000212130314152122160003041718191a092324250126e517cb977ae3ad2a0003000000020302030219a086010000000000054000000000000032004b0903020000010903fe234c2bed04b83d2bc2e13d5320d2c0e31393816adb2d0bca769afbeb0bea1004c6c8c4c30304c5c2ac6488e2949297db40612a5f2340f0bc6a1c476982accc1a20b032e4d99c04fa048e8b898a031d8f8dd64fd4ba4db1bd148b9135fd8024bc94a40d9849e774d871cb2d366ae6a0265205b7b5bfbeb60564bca2bbba";
        let mut solana_message_bytes = hex::decode(solana_message_hex).unwrap();
        // message
        let message = Message::read(&mut solana_message_bytes).unwrap();
        // print account
        let message_normal_accounts = message.accounts.clone();
        let message_alt = message.address_table_lookups.clone().unwrap();
        let mut all_accounts: Vec<String> = vec![];
        for account in message_normal_accounts {
            all_accounts.push(bitcoin::base58::encode(account.value.as_slice()));
        }
        for account in message_alt.clone() {
            all_accounts.push(bitcoin::base58::encode(
                account.account_key.value.as_slice(),
            ));
        }
        let msg_detail = message.to_program_details().unwrap();
        println!(
            "msg_detail: {}",
            serde_json::to_string_pretty(&msg_detail).unwrap()
        );
        let expect_msg_detail = json!({
              "data": "ataeiVXfFLiWHPpsXYoZ7QseUEuuGiPUu6NuuVorXh2oq9xQ8ogN",
              "accounts": [
                "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
                "F1YXtcxMQVLkM43tWFQ9QDko73B5su6Bh1x9EuoEerjz",
                "84YEm7injLQaBQ2W4gsjzdTwMjDFAAU2zMRzJfgsLzBw",
                "J73motbRk4WuL41XQ3dJrdd8aFyM2GffwnpNPaegvPyq#4",
                "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
                "J73motbRk4WuL41XQ3dJrdd8aFyM2GffwnpNPaegvPyq#197", // 3THMPkPmcHJ54JtHRyazhs7UN7HbV5KiNJVLJs6hSPSC
                "J73motbRk4WuL41XQ3dJrdd8aFyM2GffwnpNPaegvPyq#194", // EUc3MtHPLL956pTDfM5q25jp5Fk9zW7omEzh1uyDY7s6
                "F1YXtcxMQVLkM43tWFQ9QDko73B5su6Bh1x9EuoEerjz",
                "FYuDzXNvMvwmWsDjCRPSG7tFxcdStwbdsxYSv2S1BesA",
                "J73motbRk4WuL41XQ3dJrdd8aFyM2GffwnpNPaegvPyq#198",
                "J73motbRk4WuL41XQ3dJrdd8aFyM2GffwnpNPaegvPyq#200",
                "HBYnhq68N6yadGcCgCYTGkBwUbrXAH6NpNEqeRaZCxJR",
                "J73motbRk4WuL41XQ3dJrdd8aFyM2GffwnpNPaegvPyq#196",
                "J73motbRk4WuL41XQ3dJrdd8aFyM2GffwnpNPaegvPyq#195",
                "J73motbRk4WuL41XQ3dJrdd8aFyM2GffwnpNPaegvPyq#4",
                "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
                "Cbx3At3XTekfYgwEBMZjBnY3FkBmMEPfMj91yPbGw4eD#143",
                "Cbx3At3XTekfYgwEBMZjBnY3FkBmMEPfMj91yPbGw4eD#141",
                "F1YXtcxMQVLkM43tWFQ9QDko73B5su6Bh1x9EuoEerjz",
                "HBYnhq68N6yadGcCgCYTGkBwUbrXAH6NpNEqeRaZCxJR",
                "Cbx3At3XTekfYgwEBMZjBnY3FkBmMEPfMj91yPbGw4eD#142",
                "Cbx3At3XTekfYgwEBMZjBnY3FkBmMEPfMj91yPbGw4eD#139",
                "2vdWYceq9uLQpDbhXmQPC7Nvejuw4TfvYvBJwCtYiz3A",
                "Cbx3At3XTekfYgwEBMZjBnY3FkBmMEPfMj91yPbGw4eD#137",
                "Cbx3At3XTekfYgwEBMZjBnY3FkBmMEPfMj91yPbGw4eD#138",
                "FRarKBkvFwK7NCcK6e3wrg9Xrc2Tzswz4wWWjGEDhXn9#100",
                "FRarKBkvFwK7NCcK6e3wrg9Xrc2Tzswz4wWWjGEDhXn9#188",
                "FRarKBkvFwK7NCcK6e3wrg9Xrc2Tzswz4wWWjGEDhXn9#183",
                "F1YXtcxMQVLkM43tWFQ9QDko73B5su6Bh1x9EuoEerjz",
                "2vdWYceq9uLQpDbhXmQPC7Nvejuw4TfvYvBJwCtYiz3A",
                "84YEm7injLQaBQ2W4gsjzdTwMjDFAAU2zMRzJfgsLzBw",
                "FRarKBkvFwK7NCcK6e3wrg9Xrc2Tzswz4wWWjGEDhXn9#181",
                "FRarKBkvFwK7NCcK6e3wrg9Xrc2Tzswz4wWWjGEDhXn9#191",
                "FRarKBkvFwK7NCcK6e3wrg9Xrc2Tzswz4wWWjGEDhXn9#190",
                "FRarKBkvFwK7NCcK6e3wrg9Xrc2Tzswz4wWWjGEDhXn9#182",
                "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
                "FRarKBkvFwK7NCcK6e3wrg9Xrc2Tzswz4wWWjGEDhXn9#162",
                "FRarKBkvFwK7NCcK6e3wrg9Xrc2Tzswz4wWWjGEDhXn9#187",
                "FRarKBkvFwK7NCcK6e3wrg9Xrc2Tzswz4wWWjGEDhXn9#186",
                "3hVztnGsAWjBs8uKAA4eZ1PSXjun74AAp11BdmeTMgXy"
              ],
              "program_account": "JUP4Fb2cqiRUcaTHdrPC8h2gNsA2ETXiPDD33WcGuJB",
              "program":"Unknown"
        });
        assert_eq!(
            expect_msg_detail,
            serde_json::to_value(&msg_detail[5]).unwrap()
        );
    }
}
