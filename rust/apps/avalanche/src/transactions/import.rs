use super::base_tx::BaseTx;
use super::structs::{AvaxFromToInfo, AvaxMethodInfo, AvaxTxInfo, LengthPrefixedVec};
use super::{transferable::TransferableInput, type_id::TypeId};
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;
use ur_registry::extend;

#[derive(Debug)]
pub struct ImportTx {
    base_tx: BaseTx,
    source_chain: [u8; BLOCKCHAIN_ID_LEN],
    transfer_in: LengthPrefixedVec<TransferableInput>,
}

impl AvaxTxInfo for ImportTx {
    fn get_total_input_amount(&self) -> u64 {
        self.transfer_in
            .iter()
            .fold(0, |acc, item| acc + item.get_amount())
    }

    fn get_total_output_amount(&self) -> u64 {
        self.base_tx.get_total_output_amount()
    }

    fn get_outputs_addresses(&self) -> Vec<AvaxFromToInfo> {
        self.base_tx.get_outputs_addresses()
    }

    fn get_network_key(&self) -> String {
        "Import Tx".to_string()
    }

    fn get_method_info(&self) -> Option<AvaxMethodInfo> {
        let method = match self.source_chain {
            X_BLOCKCHAIN_ID => "Sending from X-Chain",
            P_BLOCKCHAIN_ID => "Sending from P-Chain",
            C_BLOCKCHAIN_ID => "Sending from C-Chain",
            _ => "Unknown",
        };

        Some(
            AvaxMethodInfo::from_string(method.to_string())
                .with_method_key("Import Tx".to_string()),
        )
    }
}

impl ImportTx {
    fn get_base_tx(&self) -> &BaseTx {
        &self.base_tx
    }
}

impl TryFrom<Bytes> for ImportTx {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let base_tx = BaseTx::try_from(bytes.clone())?;
        bytes.advance(base_tx.parsed_size());
        let mut source_chain = [0u8; BLOCKCHAIN_ID_LEN];
        bytes.copy_to_slice(&mut source_chain);
        Ok(ImportTx {
            base_tx,
            source_chain,
            transfer_in: LengthPrefixedVec::<TransferableInput>::try_from(bytes)?,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::transactions::type_id::TypeId;
    extern crate std;
    use core::result;
    use std::println;

    #[test]
    fn test_avax_base_import_tx() {
        // x chain import tx from p chain
        {
            let input_bytes = "00000000000300000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000003b8b87c00000000000000000000000010000000132336f8715dd313a426155cccc15ba27c3033dae00000000000000007fc93d85c6d62c5b2ac0b519c87010ea5294012d1e407030d6acd0021cac10d5000000011fbfcaa954ca294e4754e75b4c1232cd5e7539a1286027d395a136e57cc6e917000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000003b9aca000000000100000000000000010000000900000001257ee33547f045cdf50b1811a52f82bd3087e6ca99d16a0461e09d667d0814c01e43d705cedb99728d08341b2550751e79d4c396d9554b99319a2140b5b9677d012e783c9a";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            let result = ImportTx::try_from(bytes).unwrap();
            // assert!(false);
        }

        // x-chain import from c-chain
        {
            let input_bytes = "00000000000300000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000005e69ec0000000000000000000000001000000014effb776db45b22c20b4fd8bed9c315a305ecf8600000000000000007fc93d85c6d62c5b2ac0b519c87010ea5294012d1e407030d6acd0021cac10d50000000178479532682bda3cca3ecd79a19a60ead0b929632fa6652b88ae39f4771b2ace000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000005f5e10000000001000000000000000100000009000000018133ecce7737d6449415f7a532c4314fd005d89fc1d4b1d239fe5275673162f86d06852bb1a0881a5454c7646c586d55f422f695532accc3bd7e4a387c745259011c1b1afd";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            // match ImportTx::try_from(bytes) {
            //     Ok(result) => {
            //         // assert_eq!(result.get_base_tx().get_type_id(), TypeId::XchainImportTx);
            //         assert_eq!(result.source_chain, C_BLOCKCHAIN_ID);
            //         let data = b"\x3d\x9b\xda\xc0\xed\x1dv\x130\xcfh\x0e\xfd\xeb\x1aB\x15\x9e\xb3\x87\xd6\xd2\x95\x0c\x96\xf7\xd2\x8fa\xbb\xe2\xaa";
            //         assert!(false);
            //     }
            //     Err(_) => {
            //         assert!(false);
            //     }
            // }
        }

        // p-chain import form c-chain xZAN6Dr6snqq3LzAhQsCJWpXntGaMVrQRDqDE1ZdCsCgBkwWS
        {
            let input_bytes = "000000000011000000050000000000000000000000000000000000000000000000000000000000000000000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000003b9ab9d900000000000000000000000100000001d45b64545e31a4159cab2e9ebd51a56e60fb418300000000000000007fc93d85c6d62c5b2ac0b519c87010ea5294012d1e407030d6acd0021cac10d5000000011d2fe74495f92313aed48d73aef1b540730870a21f44b1b3fbb833994d8f9a79000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000003b9aca0000000001000000000000000100000009000000017eeec21ac8841dec1f0782889a9fd9ae9509888b2f1b38e912ebfe189be371420b1c1d0a0868647fa82824e07a48271658bcf57f68eb5e1d6c31adcc604263e6002ac028d9";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            let result = ImportTx::try_from(bytes).unwrap();
            assert_eq!(result.get_fee_amount(), 4135);
            assert_eq!(result.get_total_input_amount(), 1_000_000_000);
            result.get_method_info();
            assert!(false);
        }
    }
}
