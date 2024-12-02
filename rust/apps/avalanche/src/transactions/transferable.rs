// extern crate alloc;
use alloc::vec::Vec;
use core::{fmt, str::FromStr};
use super::asset_id::AssetId;
use super::outputs::secp256k1_transfer;

pub const TX_ID_LEN: usize = 32;
pub type TxId = [u8; TX_ID_LEN];

pub trait OutputTrait {
    fn display(&self);
    fn get_addresses(&self) -> Vec<[u8; 20]>;
    fn get_addresses_len(&self) -> u32;
}

pub struct Output<T>
where
    T: OutputTrait + FromStr,
{
    pub output_type: T,
}

impl<T> Output<T>
where
    T: OutputTrait + FromStr,
{
    pub fn new(output_type: T) -> Self {
        Output { output_type }
    }

    pub fn display(&self) {
        self.output_type.display();
    }
}

// impl<T> fmt::Display for Output<T>
// where
//     T: OutputTrait + fmt::Display,
// {
//     fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
//         write!(f, "{}", self.output_type)
//     }
// }

pub trait InputTrait {
    fn display(&self);
    fn get_addresses(&self) -> Vec<[u8; 20]>;
}

pub struct TransferableInput<T>
where
    T: InputTrait,
{
    pub tx_id: TxId,
    pub utxo_index: u32,
    pub asset_id: AssetId,
    pub input: T,
}

impl<T> TransferableInput<T>
where
    T: InputTrait,
{
    pub fn new(tx_id: TxId, utxo_index: u32, asset_id: AssetId, input: T) -> Self {
        TransferableInput {
            tx_id,
            utxo_index,
            asset_id,
            input,
        }
    }

    pub fn display(&self) {
        self.input.display();
    }
}

impl<T> fmt::Display for TransferableInput<T>
where
    T: InputTrait + fmt::Display,
{
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "TxID: {:?}\nUTXO Index: {}\nAssetID: {:?}\nInput: {}",
            self.tx_id, self.utxo_index, self.asset_id, self.input
        )
    }
}

pub struct SECP256K1Input {
    pub amount: u64,
    pub addresses: Vec<[u8; 20]>,
}

impl InputTrait for SECP256K1Input {
    fn display(&self) {
    }

    fn get_addresses(&self) -> Vec<[u8; 20]> {
        self.addresses.clone()
    }
}

impl fmt::Display for SECP256K1Input {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "SECP256K1 Input: amount = {}, addresses = {:?}",
            self.amount, self.addresses
        )
    }
}