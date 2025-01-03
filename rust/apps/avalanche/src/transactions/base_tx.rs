use super::structs::{
    AvaxFromToInfo, AvaxMethodInfo, AvaxTxInfo, LengthPrefixedVec, ParsedSizeAble,
};
use super::transferable::{TransferableInput, TransferableOutput};
use super::tx_header::Header;
use super::type_id::TypeId;
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use bitcoin::secp256k1::{ecdsa::RecoverableSignature, Message, PublicKey, Secp256k1};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;
use cryptoxide::hashing::sha256;

#[derive(Debug, Clone)]
pub struct BaseTx {
    codec_id: u16,
    pub type_id: TypeId,
    pub tx_header: Header,
    outputs: LengthPrefixedVec<TransferableOutput>,
    inputs: LengthPrefixedVec<TransferableInput>,
    pub memo_len: u32,
    pub memo: Vec<u8>,
    tx_size: usize,
}

impl BaseTx {
    pub fn get_blockchain_id(&self) -> [u8; BLOCKCHAIN_ID_LEN] {
        self.tx_header.get_blockchain_id()
    }

    pub fn get_inputs_len(&self) -> u32 {
        self.inputs.get_len() as u32
    }
    pub fn get_outputs_len(&self) -> u32 {
        self.outputs.get_len() as u32
    }

    pub fn parsed_size(&self) -> usize {
        self.tx_size
    }
}

pub fn avax_base_sign(seed: &[u8], path: String, unsigned_data: Vec<u8>) -> Result<[u8; 65]> {
    let mut bytes: [u8; 65] = [0; 65];

    let sig = Secp256k1::new()
        .sign_ecdsa_recoverable(
            &Message::from_slice(&sha256(unsigned_data.as_slice())).expect("Invalid hash length"),
            &keystore::algorithms::secp256k1::get_private_key_by_seed(&seed, &path.to_string())
                .map_err(|_| AvaxError::InvalidHex(format!("get private key error")))?,
        )
        .serialize_compact();

    bytes[..64].copy_from_slice(&sig.1);
    bytes[64] = sig.0.to_i32() as u8;
    Ok(bytes)
}

impl AvaxTxInfo for BaseTx {
    fn get_total_output_amount(&self) -> u64 {
        self.outputs
            .iter()
            .fold(0, |acc, item| acc + item.get_amount())
    }

    fn get_total_input_amount(&self) -> u64 {
        self.inputs
            .iter()
            .fold(0, |acc, item| acc + item.get_amount())
    }

    fn get_network(&self) -> Option<String> {
        match self.get_blockchain_id() {
            X_BLOCKCHAIN_ID => Some("Avalanche X-Chain".to_string()),
            C_BLOCKCHAIN_ID => Some("Avalanche C-Chain".to_string()),
            P_BLOCKCHAIN_ID => Some("Avalanche P-Chain".to_string()),
            _ => None,
        }
    }

    fn get_method_info(&self) -> Option<AvaxMethodInfo> {
        match self.type_id {
            TypeId::BaseTx => Some(AvaxMethodInfo::from_string("Send".to_string())),
            _ => Some(AvaxMethodInfo::from_string("Unknown".to_string())),
        }
    }

    fn get_outputs_addresses(&self) -> Vec<AvaxFromToInfo> {
        self.outputs
            .iter()
            .map(|output| {
                AvaxFromToInfo::from(
                    format!("{} AVAX", output.get_amount() as f64 / NAVAX_TO_AVAX_RATIO),
                    output.get_addresses(),
                )
            })
            .collect()
    }
}

impl TryFrom<Bytes> for BaseTx {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let initial_len = bytes.len();
        let codec_id = bytes.get_u16();
        let type_id = TypeId::try_from(bytes.get_u32())?;
        let tx_header = Header::try_from(bytes.clone())?;
        bytes.advance(tx_header.parsed_size());

        let outputs = LengthPrefixedVec::<TransferableOutput>::try_from(bytes.clone())?;
        bytes.advance(outputs.parsed_size());

        let inputs = LengthPrefixedVec::<TransferableInput>::try_from(bytes.clone())?;
        bytes.advance(inputs.parsed_size());

        let memo_len = bytes.get_u32();
        let memo = bytes.split_to(memo_len as usize).to_vec();
        let tx_size = initial_len - bytes.len();

        Ok(BaseTx {
            codec_id,
            type_id,
            tx_header,
            outputs,
            inputs,
            memo_len,
            memo,
            tx_size,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_avax_base_transaction() {
        {
            // x-chain fuji test case
            let input_bytes = "00000000000000000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000000bbdfb400000000000000000000000010000000169bc9b5b6cbbbd490abbd79a37ad6cd643be87ab3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000002faf08000000000000000000000000010000000132336f8715dd313a426155cccc15ba27c3033dae0000000163c5b29498bf6a9f1e2a5d20f8eeddaf92096c0ce1c9c2cf6b93fd9a0d12f725000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000003b7c4580000000010000000000000000";
            let input_bytes = "00000000000000000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000000bbdfb400000000000000000000000010000000169bc9b5b6cbbbd490abbd79a37ad6cd643be87ab3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000002faf0800000000000000000000000001000000016498cb45e255f5937b816a59c34a7559a2d437b10000000163c5b29498bf6a9f1e2a5d20f8eeddaf92096c0ce1c9c2cf6b93fd9a0d12f725000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000003b7c4580000000010000000000000000";

            // x-chain mainnet test case
            // let input_bytes = "00000000000000000001ed5f38341e436e5d46e2bb00b45d62ae97d1b050c64bc634ae10626739e35c4b0000000221e67317cbc4be2aeb00677ad6462778a8f52274b9d605df2591b23027a87dff000000070000000218711a00000000000000000000000001000000017c949a8013befa47e992078764ff735b18a26b5b21e67317cbc4be2aeb00677ad6462778a8f52274b9d605df2591b23027a87dff0000000700000003cf87a80c00000000000000000000000100000001d5ae9a7d5b31660f08c0aefc1547fb195fbfc85d000000021ddbc2d7d67f14df1e36111bbeef2adae97067c4ceb9db94b73e8883a5a6dd640000000121e67317cbc4be2aeb00677ad6462778a8f52274b9d605df2591b23027a87dff000000050000000395e95a000000000100000000885eea33e82eff5130de90152c0ebb98f5cfdc7c7529596fe2473a35654aac830000000021e67317cbc4be2aeb00677ad6462778a8f52274b9d605df2591b23027a87dff0000000500000002522030ec00000001000000000000000400000000000000020000000900000001a6810c96af6f4e4281031b795f78c37f3395b6d35806179d37b40603d547e2f262969f5363e168c064712607679b01ed13a76daab84addc94a3745b0549a53e5000000000900000001cefe480034588db7b5e0993410b6dbdd2e37e3ec94e75b450dd4c56c32f3b4c61cd9dab507232eb1211a846165336a7d7d975b39612df8d88174e1a92c27535f004a454d1e";
            // let mut bytes =
            //     Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            // let result = BaseTx::try_from(bytes).unwrap();

            // assert_eq!(result.get_blockchain_id(), X_BLOCKCHAIN_ID);
            // assert_eq!(
            //     "fuji1dx7fkkmvhw75jz4m67dr0ttv6epmapat8vwcu4",
            //     result
            //         .get_outputs_addresses()
            //         .get(0)
            //         .unwrap()
            //         .address
            //         .get(0)
            //         .unwrap()
            // );
            // assert_eq!(result.get_inputs_len(), 1);
            // assert_eq!(result.get_outputs_len(), 2);
        }

        // x chain base tx
        {
            let input_bytes = "00000000000000000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000001312d00000000000000000000000001000000018771921301d5bffff592dae86695a615bdb4a4413d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000004b571c0000000000000000000000001000000010969ea62e2bb30e66d82e82fe267edf6871ea5f7000000019eae34633c2103aaee5253bb3ca3046c2ab4718a109ffcdb77b51d0427be6bb7000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000005f5e100000000010000000000000000";
            let input_bytes = "00000000000000000001ed5f38341e436e5d46e2bb00b45d62ae97d1b050c64bc634ae10626739e35c4b0000000121e67317cbc4be2aeb00677ad6462778a8f52274b9d605df2591b23027a87dff00000007000000000089544000000000000000000000000100000001512e7191685398f00663e12197a3d8f6012d9ea300000001db720ad6707915cc4751fb7e5491a3af74e127a1d81817abe9438590c0833fe10000000021e67317cbc4be2aeb00677ad6462778a8f52274b9d605df2591b23027a87dff000000050000000000989680000000010000000000000000";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            match BaseTx::try_from(bytes) {
                Ok(result) => {
                    assert_eq!(
                        result.get_network().unwrap(),
                        "Avalanche X-Chain".to_string()
                    );
                    assert_eq!(
                        result.get_blockchain_id(),
                        [
                            171, 104, 235, 30, 225, 66, 160, 92, 254, 118, 140, 54, 225, 31, 11,
                            89, 109, 181, 163, 198, 199, 122, 171, 230, 101, 218, 217, 230, 56,
                            202, 148, 247
                        ]
                    );
                    assert_eq!(result.get_outputs_len(), 2);
                }
                Err(e) => match e {
                    AvaxError::InvalidHex(msg) => {
                        assert_eq!(
                            msg, "Unsupported output type found in input bytes.",
                            "Unexpected error message"
                        );
                    }
                    _ => {}
                },
            }
        }
    }

    #[test]
    fn test_sign_base_tx() {
        let seed = hex::decode("b75a396d4965e5352b6c2c83e4a59ad3d243fbd58133ea9fe0631e5c1576808cb7c1a578099f35278ba00fccd2709a2ef73d7e31380898a63a15b5b3f4532010").unwrap();

        // c chain import 2NqkXZiNn9KkcM8AzRApymjbp556LRWhTeF6icL5BovqdRL39j
        assert_eq!(hex::encode(avax_base_sign(&seed, String::from("m/44'/60'/0'/0/5"),
        hex::decode("000000000000000000057fc93d85c6d62c5b2ac0b519c87010ea5294012d1e407030d6acd0021cac10d5000000000000000000000000000000000000000000000000000000000000000000000001281937d79ec913734e3705754b5a930b2b1899ab47dce7ecb002fca77e16dffb000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000005f5e100000000010000000000000001a9b548da818607e83cbcf6802370691948cbd4160000000005f589443d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa").unwrap()).unwrap()), 
        "8e8d5d48956752364d1f5deac5dc32e9034cbc0c94d077934fff59dcb2f468ba7448aa4f3a13ae7907254770c4914c078ee90c5ab856eff8c679daeeee92dc7500".to_string());

        // c chain export 2JrHpYEKKwQuFxDg51uNk8pCAzEbYp3UUiAZeEsFrjEEjZVT1A
        assert_eq!(hex::encode(avax_base_sign(&seed, String::from("m/44'/60'/0'/0/5"),
        hex::decode("000000000001000000057fc93d85c6d62c5b2ac0b519c87010ea5294012d1e407030d6acd0021cac10d5000000000000000000000000000000000000000000000000000000000000000000000001a9b548da818607e83cbcf6802370691948cbd416000000005371d2663d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000000000001000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000053717aaa0000000000000000000000010000000169bc9b5b6cbbbd490abbd79a37ad6cd643be87ab").unwrap()).unwrap()), 
        "0a5dd882621638bad2c76f5c88ccf23d08a17d00b12eafe6e0c0de4a4268edbb0c774e850cc77823edf6afcf51975b8f37e99dc394e5f929d77be2c239a1720500".to_string());

        assert_eq!(hex::encode(avax_base_sign(&seed, String::from("m/44'/9000'/0'/0/0"),
        hex::decode("000000000022000000050000000000000000000000000000000000000000000000000000000000000000000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000700000000000f42400000000000000000000000010000000132336f8715dd313a426155cccc15ba27c3033dae3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000004d58ade90000000000000000000000010000000132336f8715dd313a426155cccc15ba27c3033dae00000001410b47f7c7aa13f88122be58735c5e985edc65d86fb0baf0b016359c22253d75000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000004d680464000000010000000000000000").unwrap()).unwrap()), 
        "a3b62b557e703e59c2e63f0124c8d12b13f50ba695b234ff2d39def50df2f7ad255d161ce088d318f4dad1a8df5321387461e56af72621d4074ff6459cbd09bc00".to_string());

        // p-chain send
        assert_eq!(hex::encode(avax_base_sign(&seed, String::from("m/44'/9000'/0'/0/5"),
            hex::decode("a3b62b557e703e59c2e63f0124c8d12b13f50ba695b234ff2d39def50df2f7ad255d161ce088d318f4dad1a8df5321387461e56af72621d4074ff6459cbd09bc00").unwrap()).unwrap()),
        "02baf08b64b33b8bf017cc15a4ddd1c48b9e4231b900b74b2026b4fae9a512885bc8dc2110d5aad04424b91caa0b7263c4aab41ae74f4c783f8e505182b026b300".to_string());

        // p-chain import
        assert_eq!(hex::encode(avax_base_sign(&seed, String::from("m/44'/9000'/0'/0/5"),
            hex::decode("000000000011000000050000000000000000000000000000000000000000000000000000000000000000000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000053716a830000000000000000000000010000000169bc9b5b6cbbbd490abbd79a37ad6cd643be87ab00000000000000007fc93d85c6d62c5b2ac0b519c87010ea5294012d1e407030d6acd0021cac10d500000001ac39ef990eb8b9dec7df7a8c0accd6203fa22ff354442aa5626b54ef2d58f42d000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000053717aaa0000000100000000").unwrap()).unwrap()), 
        "0d4d8d482ca9e206fd60ffb0000e52fd56693dd7c5a688da4e5fa620b3fcd1155210ce8ef5efeceae3d53657de0cb8ac17a5e35fecb1d6732c48ed30c4b1031501".to_string());

        // p-chain export
        assert_eq!(hex::encode(avax_base_sign(&seed, String::from("m/44'/9000'/0'/0/5"),
        hex::decode("000000000012000000050000000000000000000000000000000000000000000000000000000000000000000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000017d68c240000000000000000000000010000000169bc9b5b6cbbbd490abbd79a37ad6cd643be87ab00000001b9376e9a05dac917513e5385e64bf2dfb1cee8a29848a80ada96ebe6d6a04050000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000053716a83000000010000000000000000ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000003b9aca000000000000000000000000010000000169bc9b5b6cbbbd490abbd79a37ad6cd643be87ab").unwrap()).unwrap()), 
        "9efee613e2cdbca7c70d197913a402f2ff93e070e7edf5ca60d5e441579403e33687bf6c2632fc40dc6fdacd07a4d6383fc6eec35f6173251a3afd26176ad9d500".to_string());

        // x-chain import
        assert_eq!(hex::encode(avax_base_sign(&seed, String::from("m/44'/9000'/0'/0/5"),
            hex::decode("00000000000300000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000003b8b87c00000000000000000000000010000000169bc9b5b6cbbbd490abbd79a37ad6cd643be87ab000000000000000000000000000000000000000000000000000000000000000000000000000000000000000143286d1ed254c32dbdd03fdd0935a1324788f5de8f87e8f2d809206472575dba000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000003b9aca000000000100000000").unwrap()).unwrap()), 
            "5a5bc0444e1a18bc730694618a0d10ba320796d66b2c6bb9aab0cdda77fc31502ed22ed50454935f8914c2e036f9e38280d08dc36b83c59f2b9ba31e9c16914101".to_string());
    }
}
