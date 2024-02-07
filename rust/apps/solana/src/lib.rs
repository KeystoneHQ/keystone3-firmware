#![no_std]
#![feature(error_in_core)]
#![allow(dead_code)] // add for solana use a lot of external code

extern crate alloc;
extern crate core;

#[cfg(test)]
#[macro_use]
extern crate std;

use crate::read::Read;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use parser::detail::ProgramDetailAnchor;
use serde_json::{json, Value};
use third_party::base58;

mod address;
use crate::parser::structs::ParsedSolanaTx;
use crate::structs::SolanaMessage;
pub use address::get_address;

mod compact;
pub mod errors;
mod instruction;
pub mod message;
pub mod parser;
pub mod read;
mod resolvers;
mod solana_lib;
pub mod structs;
pub mod vertioned_message;

pub fn parse_message(tx_hex: Vec<u8>, from_key: &String) -> errors::Result<SolanaMessage> {
    let raw_message = hex::encode(tx_hex.clone());
    let mut utf8_message =
        String::from_utf8(tx_hex).map_or_else(|_| "".to_string(), |utf8_msg| utf8_msg);
    if app_utils::is_cjk(&utf8_message) {
        utf8_message = "".to_string();
    }
    SolanaMessage::from(raw_message, utf8_message, from_key)
}

pub fn validate_tx(message: &mut Vec<u8>) -> bool {
    message::Message::validate(message)
}

pub fn parse(data: &Vec<u8>) -> errors::Result<ParsedSolanaTx> {
    ParsedSolanaTx::build(data)
}

pub fn sign(message: Vec<u8>, hd_path: &String, seed: &[u8]) -> errors::Result<[u8; 64]> {
    keystore::algorithms::ed25519::slip10_ed25519::sign_message_by_seed(&seed, hd_path, &message)
        .map_err(|e| errors::SolanaError::KeystoreError(format!("sign failed {:?}", e.to_string())))
}

pub fn parse_program_anchor(
    program_account: &String,
    program_data: &String,
    instruction: &String,
    types: &String,
) -> errors::Result<String> {
    let instruction_v: Value = serde_json::from_str(instruction)?;
    let _types_v: Value = serde_json::from_str(types)?;
    let binding = base58::decode(&program_data)?;
    let mut data = binding.as_slice();
    let _method: u8 = borsh::BorshDeserialize::deserialize(&mut data).map_err(|e| {
        errors::SolanaError::ProgramError(format!("borsh deserialize error {:?}", e.to_string()))
    })?;
    let program_method = instruction_v
        .get("name")
        .unwrap_or(&Value::String("Unknown".to_string()))
        .as_str()
        .unwrap_or("Unknown")
        .to_string();
    let detail = ProgramDetailAnchor {
        program_account: program_account.to_string(),
        program_method,
        program_params: Vec::new(),
    };
    Ok(json!(detail).to_string())
}

pub fn get_instruction_index(program_data: &String) -> errors::Result<String> {
    let data_vec = base58::decode(program_data)?;
    if data_vec.len() < 8 {
        return Err(errors::SolanaError::InvalidData(format!(
            "program data length less than 8"
        )));
    }
    let index_data = data_vec[0..8].to_vec();
    Ok(base58::encode(&index_data))
}

#[cfg(test)]
mod tests {
    use super::*;
    use third_party::hex::{FromHex, ToHex};
    use third_party::ur_registry::solana::sol_sign_request::SolSignRequest;

    #[test]
    fn test_get_instruction_index() {
        let program_data = "ASCsAbe1UnERt6XtmeVZZFGNQEaZkXtbcdZExMVwTRGK5TeDis7MorNk".to_string();
        let index = get_instruction_index(&program_data).unwrap();
        assert_eq!("8CLkZYyRkFB", index);
    }

    #[test]
    fn test_parse_program_anchor() {
        let program_account = "CAMMCzo5YL8w4VFF8KVHrK22GGUsp5VTaW7grrKgrWqK".to_string();
        let program_data = "ASCsAbe1UnERt6XtmeVZZFGNQEaZkXtbcdZExMVwTRGK5TeDis7MorNk".to_string();
        let instruction = "{\"name\":\"createAmmConfig\",\"accounts\":[{\"name\":\"owner\",\"isMut\":true,\"isSigner\":true},{\"name\":\"ammConfig\",\"isMut\":true,\"isSigner\":false},{\"name\":\"systemProgram\",\"isMut\":false,\"isSigner\":false}],\"args\":[{\"name\":\"index\",\"type\":\"u16\"},{\"name\":\"tickSpacing\",\"type\":\"u16\"},{\"name\":\"tradeFeeRate\",\"type\":\"u32\"},{\"name\":\"protocolFeeRate\",\"type\":\"u32\"},{\"name\":\"fundFeeRate\",\"type\":\"u32\"}]}".to_string();
        let types = "[{\"name\":\"InitializeRewardParam\",\"type\":{\"kind\":\"struct\",\"fields\":[{\"name\":\"openTime\",\"type\":\"u64\"},{\"name\":\"endTime\",\"type\":\"u64\"},{\"name\":\"emissionsPerSecondX64\",\"type\":\"u128\"}]}},{\"name\":\"Observation\",\"type\":{\"kind\":\"struct\",\"fields\":[{\"name\":\"blockTimestamp\",\"type\":\"u32\"},{\"name\":\"sqrtPriceX64\",\"type\":\"u128\"},{\"name\":\"cumulativeTimePriceX64\",\"type\":\"u128\"},{\"name\":\"padding\",\"type\":\"u128\"}]}},{\"name\":\"PositionRewardInfo\",\"type\":{\"kind\":\"struct\",\"fields\":[{\"name\":\"growthInsideLastX64\",\"type\":\"u128\"},{\"name\":\"rewardAmountOwed\",\"type\":\"u64\"}]}},{\"name\":\"RewardInfo\",\"type\":{\"kind\":\"struct\",\"fields\":[{\"name\":\"rewardState\",\"type\":\"u8\"},{\"name\":\"openTime\",\"type\":\"u64\"},{\"name\":\"endTime\",\"type\":\"u64\"},{\"name\":\"lastUpdateTime\",\"type\":\"u64\"},{\"name\":\"emissionsPerSecondX64\",\"type\":\"u128\"},{\"name\":\"rewardTotalEmissioned\",\"type\":\"u64\"},{\"name\":\"rewardClaimed\",\"type\":\"u64\"},{\"name\":\"tokenMint\",\"type\":\"publicKey\"},{\"name\":\"tokenVault\",\"type\":\"publicKey\"},{\"name\":\"authority\",\"type\":\"publicKey\"},{\"name\":\"rewardGrowthGlobalX64\",\"type\":\"u128\"}]}},{\"name\":\"TickState\",\"type\":{\"kind\":\"struct\",\"fields\":[{\"name\":\"tick\",\"type\":\"i32\"},{\"name\":\"liquidityNet\",\"type\":\"i128\"},{\"name\":\"liquidityGross\",\"type\":\"u128\"},{\"name\":\"feeGrowthOutside0X64\",\"type\":\"u128\"},{\"name\":\"feeGrowthOutside1X64\",\"type\":\"u128\"},{\"name\":\"rewardGrowthsOutsideX64\",\"type\":{\"array\":[\"u128\",3]}},{\"name\":\"padding\",\"type\":{\"array\":[\"u32\",13]}}]}},{\"name\":\"PoolStatusBitIndex\",\"type\":{\"kind\":\"enum\",\"variants\":[{\"name\":\"OpenPositionOrIncreaseLiquidity\"},{\"name\":\"DecreaseLiquidity\"},{\"name\":\"CollectFee\"},{\"name\":\"CollectReward\"},{\"name\":\"Swap\"}]}},{\"name\":\"PoolStatusBitFlag\",\"type\":{\"kind\":\"enum\",\"variants\":[{\"name\":\"Enable\"},{\"name\":\"Disable\"}]}},{\"name\":\"RewardState\",\"type\":{\"kind\":\"enum\",\"variants\":[{\"name\":\"Uninitialized\"},{\"name\":\"Initialized\"},{\"name\":\"Opening\"},{\"name\":\"Ended\"}]}},{\"name\":\"TickArryBitmap\",\"type\":{\"kind\":\"alias\",\"value\":{\"array\":[\"u64\",8]}}}]".to_string();
        let data = base58::decode(program_data.as_str()).unwrap();
        let index_data = data[0..8].to_vec();
        let index = base58::encode(&index_data);
        println!("index: {}", index);
        let p =
            parse_program_anchor(&program_account, &program_data, &instruction, &types).unwrap();
        assert_eq!(p, "{\"program_account\":\"CAMMCzo5YL8w4VFF8KVHrK22GGUsp5VTaW7grrKgrWqK\",\"program_method\":\"createAmmConfig\",\"program_params\":[]}")
    }

    #[test]
    fn test_parse() {
        let tx_hex = "800100080F8FEB8D0AAB0838A7F0BAD992867AB46437EED77DA5EF951126410E76E75A317FDF259B49E85C094A63E50B2897EB5607B862BDF615BF7A0ED15EF7B1E086880B4BE5EB9F5905ED2F96A78F9AC6A9E4B570BCF10CA55BF47C82A9B6C81DB9F21B75805B94F3DED5B05F23A9C3A96CFDCA1417E7A16095052F4173C172EC1AA7AB63D5A1315D0FD6077BBA9F53A5B317F86A6F6B59C709F1616CBE30725728A56EEE9BAC8C894C386EFF1F0E9DDE398B55ED9F41127863F3EBDF16E4759F0AE32982D689B0F3C1B3A2FC01FC1C712DBB8B6474BF0EFD26C7537761350596A5798A0306466FE5211732FFECADBA72C39BE7BC8CE5BBC5F7126B2C439B3A40000000000000000000000000000000000000000000000000000000000000000000000006DDF6E1D765A193D9CBE146CEEB79AC1CB485ED5F5B37913A8CF5857EFF00A9069B8857FEAB8184FB687F634618C035DAC439DC1AEB3B5598A0F000000000018C97258F4E2489F1BB3D1029148E0D830B5A1399DAFF1084048E7BD8DBE9F859CE010E60AFEDB22717BD63192F54145A3F965A33BB82D2C7029EB2CE1E208264A5D5CA9E04CF5DB590B714BA2FE32CB159133FC1C192B72257FD07D39CB0401E054A535A992921064D24E87160DA387C7C35B5DDBC92BB81E41FA8404105448DD6D830A29A9810EBA9920D1F913FB0D98E7B117400A02C26D5C2DEABBE58D42E0707000903A86100000000000007000502C0270900080200017C030000008FEB8D0AAB0838A7F0BAD992867AB46437EED77DA5EF951126410E76E75A317F200000000000000044514C4673586A5367625A726E673355644E377335477348795472346648767870B4B70000000000A50000000000000006DDF6E1D765A193D9CBE146CEEB79AC1CB485ED5F5B37913A8CF5857EFF00A90904010A001301010B060002000C0809000D1100150F010210111209140E0A0C03040506292B04ED0B1AC91E62809698000000000069400F0000000000513B0100010000000000000000000000010903010000010902198F1F4C3A452263D413B2CD17EBCBC1A0E5887364E6261A12A81792EA165A3E000205036F2D364FD63E492CFCB919FF6AA06DA4F35D722DA0DE3AD39857085E77E43C1F04050708090106";
        let tx_vec = Vec::from_hex(tx_hex).unwrap();
        let tx = parse(&tx_vec).unwrap();
        assert_eq!(tx.detail, "[{\"program\":\"ComputeBudget\",\"method\":\"SetComputeUnitPrice\",\"value\":\"25000\"},{\"program\":\"ComputeBudget\",\"method\":\"SetComputeUnitLimit\",\"value\":\"600000\"},{\"program\":\"System\",\"method\":\"CreateAccountWithSeed\",\"funding_account\":\"Agod9YRspk7yaAynUNyqwUDGhn3M2Yw31LThi48t7GMU\",\"new_account\":\"G25BDHTgor2H9ChWfZEDA3p85YDXC9E3uoYi35Uw9F3Y\",\"base_account\":\"\",\"base_pubkey\":\"Agod9YRspk7yaAynUNyqwUDGhn3M2Yw31LThi48t7GMU\",\"seed\":\"DQLFsXjSgbZrng3UdN7s5GsHyTr4fHvx\",\"amount\":\"12039280\",\"space\":\"165\",\"owner\":\"TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA\"},{\"program\":\"Token\",\"method\":\"InitializeAccount\",\"account\":\"G25BDHTgor2H9ChWfZEDA3p85YDXC9E3uoYi35Uw9F3Y\",\"mint\":\"So11111111111111111111111111111111111111112\",\"owner\":\"Agod9YRspk7yaAynUNyqwUDGhn3M2Yw31LThi48t7GMU\",\"sysver_rent\":\"Unknown\"},{\"program\":\"Unknown\",\"program_index\":11,\"account_indexes\":[0,2,0,12,8,9],\"data\":\"\",\"reason\":\"Unable to parse instruction, Program `ATokenGPvbdGVxr1b2hvZbsiqW5xWH25efTNsLJA8knL` is not supported yet\",\"accounts\":\"Agod9YRspk7yaAynUNyqwUDGhn3M2Yw31LThi48t7GMU,67GvNDUJYQsaw1cqaK9EpaEZfRHxvx71CYJVm3yhZ1Ei,Agod9YRspk7yaAynUNyqwUDGhn3M2Yw31LThi48t7GMU,Es9vMFrzaCERmJfrF4H2FYD4KCoNkY11McCe8BenwNYB,11111111111111111111111111111111,TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA\",\"program_account\":\"ATokenGPvbdGVxr1b2hvZbsiqW5xWH25efTNsLJA8knL\"},{\"program\":\"Unknown\",\"program_index\":13,\"account_indexes\":[0,21,15,1,2,16,17,18,9,20,14,10,12,3,4,5,6],\"data\":\"ASCsAbe1UnERt6XtmeVZZFGNQEaZkXtbcdZExMVwTRGK5TeDis7MorNk\",\"reason\":\"Unable to parse instruction, Program `CAMMCzo5YL8w4VFF8KVHrK22GGUsp5VTaW7grrKgrWqK` is not supported yet\",\"accounts\":\"Agod9YRspk7yaAynUNyqwUDGhn3M2Yw31LThi48t7GMU,Unknown,Unknown,G25BDHTgor2H9ChWfZEDA3p85YDXC9E3uoYi35Uw9F3Y,67GvNDUJYQsaw1cqaK9EpaEZfRHxvx71CYJVm3yhZ1Ei,Unknown,Unknown,Unknown,TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA,Unknown,MemoSq4gqABAXKb96qnH8TysNcWxMyWCqXgDLGmfcHr,So11111111111111111111111111111111111111112,Es9vMFrzaCERmJfrF4H2FYD4KCoNkY11McCe8BenwNYB,8ugDp2SoPq4e9A6HP6qaDDrxPU6rbAidyoYBvVr7rXUn,7iiJEtZj2FVeSQjJc1BSALFecA9gKNK7q8jsmhZmQNr9,H4RiEff1caC2UReZDz5GmmzfGhzfR5tJ2xf6g32VyPqJ,9ojjsFjBwn6zMcnxBySPGiPYnhJtEMJ7vnmWdhh1jS33\",\"program_account\":\"CAMMCzo5YL8w4VFF8KVHrK22GGUsp5VTaW7grrKgrWqK\"},{\"program\":\"Token\",\"method\":\"CloseAccount\",\"account\":\"G25BDHTgor2H9ChWfZEDA3p85YDXC9E3uoYi35Uw9F3Y\",\"recipient\":\"Agod9YRspk7yaAynUNyqwUDGhn3M2Yw31LThi48t7GMU\",\"owner\":\"Agod9YRspk7yaAynUNyqwUDGhn3M2Yw31LThi48t7GMU\"}]");
    }

    #[test]
    fn test_solana_sign() {
        let hd_path = "m/44'/501'/0'".to_string();
        let tx_hex =  Vec::from_hex("010002041a93fffb26ce645adeae58f0f414c320bcec30ce12a66bd263a91ec9b3958ff46f345144d352e4190c2dec43e1d3e0296a49bdfc2594eed9d8a5902e22d0af8b00000000000000000000000000000000000000000000000000000000000000000306466fe5211732ffecadba72c39be7bc8ce5bbc5f7126b2c439b3a40000000f70a9d4448ef435c5beab6cbc4211e00ddb4b9ad84886385f8b7ccfb9d9e7ca40303000903d8d600000000000003000502400d0300020200010c020000008096980000000000").unwrap();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let signature = sign(tx_hex, &hd_path, seed.as_slice()).unwrap();
        assert_eq!("9625b26df39be0a392cd2f0db075a238fe7bd98d181b8705bcc6c1c64f652294c54760af911cca245769489c30c12e44cf5e139ca71f1acc834eea4b63017b00", signature.encode_hex::<String>());
    }

    #[test]
    fn test_solana_validate() {
        let mut tx_hex =  Vec::from_hex("010002041a93fffb26ce645adeae58f0f414c320bcec30ce12a66bd263a91ec9b3958ff46f345144d352e4190c2dec43e1d3e0296a49bdfc2594eed9d8a5902e22d0af8b00000000000000000000000000000000000000000000000000000000000000000306466fe5211732ffecadba72c39be7bc8ce5bbc5f7126b2c439b3a40000000f70a9d4448ef435c5beab6cbc4211e00ddb4b9ad84886385f8b7ccfb9d9e7ca40303000903d8d600000000000003000502400d0300020200010c020000008096980000000000").unwrap();
        assert_eq!(true, validate_tx(&mut tx_hex));
    }

    #[test]
    fn test_solana_parse_message() {
        let cbor_hex = "a401d82550316ddac29df74514b9d8f53ebd847a750259021d6d616769636564656e2e696f2077616e747320796f7520746f207369676e20696e207769746820796f757220536f6c616e61206163636f756e743a0a47575a567a6353324d58664671486d50373832517833527a6b6b5158324b666763685a504c703341455a726d0a0a436c69636b205369676e206f7220417070726f7665206f6e6c79206d65616e7320796f7520686176652070726f76656420746869732077616c6c6574206973206f776e656420627920796f752e205468697320726571756573742077696c6c206e6f74207472696767657220616e7920626c6f636b636861696e207472616e73616374696f6e206f7220636f737420616e7920676173206665652e20557365206f66206f7572207765627369746520616e64207365727669636520617265207375626a65637420746f206f7572205465726d73206f6620536572766963653a2068747470733a2f2f6d616769636564656e2e696f2f7465726d732d6f662d736572766963652e70646620616e64205072697661637920506f6c6963793a2068747470733a2f2f6d616769636564656e2e696f2f707269766163792d706f6c6963792e7064660a0a5552493a2068747470733a2f2f6d616769636564656e2e696f0a56657273696f6e3a20310a436861696e2049443a206d61696e6e65740a4e6f6e63653a2076706478336e476662390a4973737565642041743a20323032332d30372d32375430323a31303a34392e3136315a03d90130a20188182cf51901f5f500f500f5021a527447030601";
        let pubkey = "e671e524ef43ccc5ef0006876f9a2fd66681d5abc5871136b343a3e4b073efde".to_string();
        let sol_sign_request = SolSignRequest::try_from(hex::decode(cbor_hex).unwrap()).unwrap();
        let parsed = parse_message(sol_sign_request.get_sign_data(), &pubkey).unwrap();
        assert_eq!("GWZVzcS2MXfFqHmP782Qx3RzkkQX2KfgchZPLp3AEZrm", parsed.from);
    }
}
