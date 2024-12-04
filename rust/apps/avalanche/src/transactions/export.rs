use super::base_tx::BaseTx;
use super::structs::LengthPrefixedVec;
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use crate::transactions::transferable::TransferableOutput;
use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug)]
pub struct ExportTx {
    base_tx: BaseTx,
    dest_chain: [u8; BLOCKCHAIN_ID_LEN],
    transfer_out: LengthPrefixedVec<TransferableOutput>,
}

impl ExportTx {
    fn get_base_tx(&self) -> &BaseTx {
        &self.base_tx
    }

    fn get_dest_chain(&self) -> [u8; BLOCKCHAIN_ID_LEN] {
        self.dest_chain
    }

    fn get_transfer_out(&self) -> &LengthPrefixedVec<TransferableOutput> {
        &self.transfer_out
    }
}

impl TryFrom<Bytes> for ExportTx {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let base_tx = BaseTx::try_from(bytes.clone())?;
        bytes.advance(base_tx.parsed_size());
        let mut dest_chain = [0u8; BLOCKCHAIN_ID_LEN];
        bytes.copy_to_slice(&mut dest_chain);

        Ok(ExportTx {
            base_tx,
            dest_chain,
            transfer_out: LengthPrefixedVec::<TransferableOutput>::try_from(bytes.clone())?,
        })
    }
}

mod tests {
    use super::*;
    use crate::transactions::type_id::TypeId;
    extern crate std;
    use std::println;

    #[test]
    fn test_avax_base_export_tx() {
        // p chain export to x
        {
            let input_bytes = "000000000012000000050000000000000000000000000000000000000000000000000000000000000000000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000002faef3a1000000000000000000000001000000010969ea62e2bb30e66d82e82fe267edf6871ea5f7000000010e291a4cf01e2da30570a17c916d9af0bb839fbd6eb1b6ce3cf0955d54b972f8000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000003b9aca00000000010000000000000000ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000000bebc200000000000000000000000001000000010969ea62e2bb30e66d82e82fe267edf6871ea5f7000000010000000900000001ab9f7138c2446557f8671280ea08c30c8e6e3fc5c84656f7f2a197e77660385c352e165a987d650c2915afa0cf4ea15c1134f7c50681e476ffa71cd540feffab006dac4774";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            match ExportTx::try_from(bytes) {
                Ok(result) => {
                    assert_eq!(result.get_base_tx().get_type_id(), TypeId::PchainExportTx);
                    assert_eq!(result.get_dest_chain(), X_BLOCKCHAIN_ID);
                }
                Err(e) => match e {
                    AvaxError::InvalidHex(msg) => {
                        assert!(false);
                    }
                    _ => {}
                },
            }
        }
    }
}
