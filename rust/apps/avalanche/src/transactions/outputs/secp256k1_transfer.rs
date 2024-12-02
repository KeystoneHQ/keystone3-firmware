use alloc::vec::Vec;
use core::fmt;

use crate::transactions::transferable::OutputTrait;

#[derive(Debug, Clone)]
pub struct SECP256K1TransferOutput {
    pub type_id: u32,
    pub amount: u64,                 
    pub locktime: u64,
    pub threshold: u32,
    pub addresses_len: u32,      
    pub addresses: Vec<[u8; 20]>,
}

impl OutputTrait for SECP256K1TransferOutput {
    fn display(&self) {}

    fn get_addresses(&self) -> Vec<[u8; 20]> {
        self.addresses.clone()
    }

    fn get_addresses_len(&self) -> u32 {
        self.addresses_len
    }
}

impl SECP256K1TransferOutput {
    pub fn new(type_id: u32, amount: u64, locktime: u64, threshold: u32, addresses: Vec<[u8; 20]>) -> Self {
        let addresses_len = addresses.len() as u32;
        SECP256K1TransferOutput {
            type_id,
            amount,
            locktime,
            threshold,
            addresses_len,
            addresses,
        }
    }

    pub fn display(&self) {
        #[cfg(feature = "std")]
        {
            extern crate std;
            use std::println;
            println!("SECP256K1TransferOutput:");
            println!("  Type ID: {}", self.type_id);
            println!("  Amount: {}", self.amount);
            println!("  Locktime: {}", self.locktime);
            println!("  Threshold: {}", self.threshold);
            println!("  Addresses Length: {}", self.addresses_len);
            println!("  Addresses: ");
            for (index, address) in self.addresses.iter().enumerate() {
                println!("    Address {}: {:?}", index, address);
            }
        }
    }
}
