use super::base_tx::BaseTx;
use super::tx_header::{Header, BLOCKCHAIN_ID_LEN};
use super::type_id::TypeId;
use super::structs::LengthPrefixedVec;
use crate::errors::{AvaxError, Result};
use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;


#[derive(Debug)]
pub struct ImportTx {
    base_tx: BaseTx,
    source_chain: [u8; BLOCKCHAIN_ID_LEN],
    // transfer_in: LengthPrefixedVec<Bytes>,
}

impl TryFrom<Bytes> for ImportTx {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let base_tx = BaseTx::try_from(bytes.clone())?;
        bytes.advance(base_tx.parsed_size());
        let mut source_chain = [0u8; BLOCKCHAIN_ID_LEN];
        bytes.copy_to_slice(&mut source_chain);
        
        let transfer_in_len = bytes.get_u32();
        println!("transfer_in_len = {:?}", transfer_in_len);

        println!("bytes = {:?}", hex::encode(bytes));

        Ok(ImportTx {
            base_tx,
            source_chain,
        })
    }
}

mod tests {
    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_avax_base_import_tx() {
        // x chain import tx from p chain
        {
            let input_bytes = "00000000000300000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000000bdc7fc0000000000000000000000001000000010969ea62e2bb30e66d82e82fe267edf6871ea5f70000000000000000000000000000000000000000000000000000000000000000000000000000000000000001dcf4ca85474e87a743ec8feb54836d2b403b36c7c738c3e2498fdd346dac4774000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000000bebc20000000001000000000000000100000009000000013494447c558e20adc5a9985a7acf9ed5b7ff12011406af213db033bb2b2271504c8224787b352e19db896d3350321487c08dba13c973009aaf682183b0d5f99f00f10c7fe4";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            let result = ImportTx::try_from(bytes);
            println!("{:?}", result);
            assert!(false);
            // match ImportTx::try_from(bytes) {
            //     Ok(result) => {
            //         println!("{:?}", result);
            //         assert!(false);
            //         // assert_eq!(result.get_type_id(), TypeId::BaseTx);
            //         // assert_eq!(result.get_network_id(), 5);
            //         // assert_eq!(result.get_blockchain_id(), [0; BLOCKCHAIN_ID_LEN]);
            //         // assert_eq!(result.get_inputs_len(), 1);
            //         // assert_eq!(result.get_outputs_len(), 2);
            //         // assert_eq!(result.get_memo(), "\0\0\0\0");
            //     }
            //     Err(e) => match e {
            //         AvaxError::InvalidHex(msg) => {
            //             println!("failed....");
            //             assert!(false);
            //         }
            //         _ => {}
            //     },
            // }
        }
    }
}
