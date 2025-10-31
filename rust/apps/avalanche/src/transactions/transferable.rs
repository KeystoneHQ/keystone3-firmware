use super::asset_id::AssetId;
use super::inputs::secp256k1_transfer_input::SECP256K1TransferInput;
use super::outputs::secp256k1_transfer_output::SECP256K1TransferOutput;
use super::structs::ParsedSizeAble;
use super::type_id::TypeId;
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;
pub const TX_ID_LEN: usize = 32;
pub type TxId = [u8; TX_ID_LEN];

#[derive(Debug, Clone)]
enum OutputType {
    SECP256K1(SECP256K1TransferOutput),
    //todo other output types
}

impl OutputTrait for OutputType {
    fn get_addresses_len(&self) -> usize {
        match self {
            OutputType::SECP256K1(output) => output.get_addresses_len(),
        }
    }

    fn get_addresses(&self) -> Vec<String> {
        match self {
            OutputType::SECP256K1(output) => output.get_addresses(),
        }
    }

    fn get_transfer_output_len(&self) -> usize {
        match self {
            OutputType::SECP256K1(output) => output.get_transfer_output_len(),
        }
    }

    fn get_amount(&self) -> u64 {
        match self {
            OutputType::SECP256K1(output) => output.get_amount(),
        }
    }
}

impl TryFrom<Bytes> for OutputType {
    type Error = AvaxError;

    fn try_from(bytes: Bytes) -> Result<Self> {
        let mut type_bytes = bytes.clone();
        let type_id = type_bytes.get_u32();
        match TypeId::try_from(type_id)? {
            TypeId::Secp256k1TransferOutput => {
                SECP256K1TransferOutput::try_from(bytes).map(OutputType::SECP256K1)
            }
            _ => {
                Err(AvaxError::InvalidHex(
                    "Unsupported output type found in input bytes.".to_string(),
                ))
            }
        }
    }
}

pub trait OutputTrait {
    fn get_addresses(&self) -> Vec<String>;
    fn get_addresses_len(&self) -> usize;
    fn get_transfer_output_len(&self) -> usize;
    fn get_amount(&self) -> u64;
}

#[derive(Debug, Clone)]
pub struct TransferableOutput {
    asset_id: AssetId,
    pub output: OutputType,
}

impl TransferableOutput {
    pub fn asset_id(&self) -> AssetId {
        self.asset_id.clone()
    }

    pub fn get_amount(&self) -> u64 {
        self.output.get_amount()
    }

    pub fn get_addresses(&self) -> Vec<String> {
        self.output.get_addresses()
    }
}

impl ParsedSizeAble for TransferableOutput {
    fn parsed_size(&self) -> usize {
        self.output.get_transfer_output_len() + ASSET_ID_LEN
    }
}

impl TryFrom<Bytes> for TransferableOutput {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let asset_id = AssetId::try_from(bytes.split_to(ASSET_ID_LEN))?;
        Ok(TransferableOutput {
            asset_id,
            output: OutputType::try_from(bytes)?,
        })
    }
}

pub trait InputTrait {
    fn get_transfer_input_len(&self) -> usize;
    fn get_amount(&self) -> u64;
}

#[derive(Debug, Clone)]
pub struct TransferableInput {
    pub tx_id: TxId,
    pub utxo_index: u32,
    pub asset_id: AssetId,
    pub input: InputType,
}

impl TransferableInput {
    pub fn get_amount(&self) -> u64 {
        self.input.get_amount()
    }
}

impl TryFrom<Bytes> for TransferableInput {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let tx_id: [u8; TX_ID_LEN] = bytes.split_to(TX_ID_LEN)[..]
            .try_into()
            .map_err(|_| AvaxError::InvalidHex("error data to tx_id".to_string()))?;
        let utxo_index = bytes.get_u32();
        let asset_id = AssetId::try_from(bytes.split_to(ASSET_ID_LEN))?;
        Ok(TransferableInput {
            tx_id,
            utxo_index,
            asset_id,
            input: InputType::try_from(bytes)?,
        })
    }
}

impl ParsedSizeAble for TransferableInput {
    fn parsed_size(&self) -> usize {
        self.input.get_transfer_input_len() + TX_ID_LEN + ASSET_ID_LEN + 4
    }
}

#[derive(Debug, Clone)]
enum InputType {
    SECP256K1(SECP256K1TransferInput),
    //todo other input types
}

impl TryFrom<Bytes> for InputType {
    type Error = AvaxError;

    fn try_from(bytes: Bytes) -> Result<Self> {
        let mut type_bytes = bytes.clone();
        let type_id = type_bytes.get_u32();
        match TypeId::try_from(type_id)? {
            TypeId::Secp256k1TransferInput => Ok(InputType::SECP256K1(
                SECP256K1TransferInput::try_from(bytes)?,
            )),
            _ => {
                Err(AvaxError::InvalidHex(
                    "Unsupported input type found in input bytes.".to_string(),
                ))
            }
        }
    }
}

impl InputTrait for InputType {
    fn get_transfer_input_len(&self) -> usize {
        match self {
            InputType::SECP256K1(input) => input.get_transfer_input_len(),
        }
    }

    fn get_amount(&self) -> u64 {
        match self {
            InputType::SECP256K1(input) => input.get_amount(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_transferable_output() {
        let input_bytes = "000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000005f5e100000000000000000000000001000000018771921301d5bffff592dae86695a615bdb4a4413d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000017c771d2000000000000000000000001000000010969ea62e2bb30e66d82e82fe267edf6871ea5f7";
        let binary_data = hex::decode(input_bytes).expect("Failed to decode hex string");
        let mut bytes = Bytes::from(binary_data);
        let output_len = bytes.get_u32();
        assert_eq!(output_len, 2);
        let result = TransferableOutput::try_from(bytes.clone()).unwrap();
        assert_eq!(result.output.get_amount(), 100000000);
        assert_eq!(result.output.get_addresses_len(), 1);
        assert_eq!(
            result.output.get_addresses(),
            vec!["avax1saceyycp6klllavjmt5xd9dxzk7mffzp6fzwtu".to_string()]
        );
        assert_eq!(result.output.get_transfer_output_len(), 48);
    }

    #[test]
    fn test_transferable_intput() {
        // secp256k1 transfer intput
        {
            let input_bytes = "0000000157d5e23e2e1f460b618bba1b55913ff3ceb315f0d1acc41fe6408edc4de9facd000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000001dbd670d0000000100000000";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            let input_len = bytes.get_u32();
            assert_eq!(input_len, 1);
            let result = TransferableInput::try_from(bytes.clone()).unwrap();
            assert_eq!(
                result.tx_id,
                [
                    87, 213, 226, 62, 46, 31, 70, 11, 97, 139, 186, 27, 85, 145, 63, 243, 206, 179,
                    21, 240, 209, 172, 196, 31, 230, 64, 142, 220, 77, 233, 250, 205
                ]
            );
            assert_eq!(result.utxo_index, 0);
            assert_eq!(result.input.get_amount(), 498951949);
            assert_eq!(result.input.get_transfer_input_len(), 20);
        }

        // x-chain import transferin
        {
            let input_bytes = "00000001dcf4ca85474e87a743ec8feb54836d2b403b36c7c738c3e2498fdd346dac4774000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000000bebc20000000001000000000000000100000009000000013494447c558e20adc5a9985a7acf9ed5b7ff12011406af213db033bb2b2271504c8224787b352e19db896d3350321487c08dba13c973009aaf682183b0d5f99f00f10c7fe4";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            let input_len = bytes.get_u32();
            assert_eq!(input_len, 1);
            let result = TransferableInput::try_from(bytes.clone()).unwrap();
            assert_eq!(
                result.tx_id,
                [
                    220, 244, 202, 133, 71, 78, 135, 167, 67, 236, 143, 235, 84, 131, 109, 43, 64,
                    59, 54, 199, 199, 56, 195, 226, 73, 143, 221, 52, 109, 172, 71, 116
                ]
            );
            assert_eq!(result.utxo_index, 1);
            assert_eq!(result.input.get_amount(), 200000000);
            assert_eq!(result.input.get_transfer_input_len(), 20);
        }
    }
}
