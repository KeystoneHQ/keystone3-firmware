use super::base_tx::BaseTx;
use super::structs::LengthPrefixedVec;
use super::transferable::TransferableInput;
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug)]
pub struct ImportTx {
    base_tx: BaseTx,
    source_chain: [u8; BLOCKCHAIN_ID_LEN],
    transfer_in: LengthPrefixedVec<TransferableInput>,
}

impl ImportTx {
    fn get_base_tx(&self) -> &BaseTx {
        &self.base_tx
    }

    fn get_source_chain(&self) -> [u8; BLOCKCHAIN_ID_LEN] {
        self.source_chain
    }

    fn get_transfer_in(&self) -> &LengthPrefixedVec<TransferableInput> {
        &self.transfer_in
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

mod tests {
    use super::*;
    use crate::transactions::type_id::TypeId;
    extern crate std;
    use std::println;

    #[test]
    fn test_avax_base_import_tx() {
        // x chain import tx from p chain
        {
            let input_bytes = "00000000000300000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000000bdc7fc0000000000000000000000001000000010969ea62e2bb30e66d82e82fe267edf6871ea5f70000000000000000000000000000000000000000000000000000000000000000000000000000000000000001dcf4ca85474e87a743ec8feb54836d2b403b36c7c738c3e2498fdd346dac4774000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000000bebc20000000001000000000000000100000009000000013494447c558e20adc5a9985a7acf9ed5b7ff12011406af213db033bb2b2271504c8224787b352e19db896d3350321487c08dba13c973009aaf682183b0d5f99f00f10c7fe4";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            match ImportTx::try_from(bytes) {
                Ok(result) => {
                    println!("{:?}", result);
                    assert_eq!(result.get_base_tx().get_type_id(), TypeId::XchainImportTx);
                    assert_eq!(result.get_source_chain(), P_BLOCKCHAIN_ID);
                }
                Err(_) => {
                    assert!(false);
                },
            }
        }

        // x-chain import from c-chain
        {
            let input_bytes = "00000000000300000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000005e69ec0000000000000000000000001000000014effb776db45b22c20b4fd8bed9c315a305ecf8600000000000000007fc93d85c6d62c5b2ac0b519c87010ea5294012d1e407030d6acd0021cac10d50000000178479532682bda3cca3ecd79a19a60ead0b929632fa6652b88ae39f4771b2ace000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000005f5e10000000001000000000000000100000009000000018133ecce7737d6449415f7a532c4314fd005d89fc1d4b1d239fe5275673162f86d06852bb1a0881a5454c7646c586d55f422f695532accc3bd7e4a387c745259011c1b1afd";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            match ImportTx::try_from(bytes) {
                Ok(result) => {
                    println!("{:?}", result);
                    assert_eq!(result.get_base_tx().get_type_id(), TypeId::XchainImportTx);
                    assert_eq!(result.get_source_chain(), C_BLOCKCHAIN_ID);
                    let data = b"\x3d\x9b\xda\xc0\xed\x1dv\x130\xcfh\x0e\xfd\xeb\x1aB\x15\x9e\xb3\x87\xd6\xd2\x95\x0c\x96\xf7\xd2\x8fa\xbb\xe2\xaa";
                    assert!(false);
                }
                Err(_) => {
                    assert!(false);
                },
            }
        }
    }
}
