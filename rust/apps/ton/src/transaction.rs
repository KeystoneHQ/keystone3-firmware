use alloc::vec::Vec;
use crate::structs::TonTransaction;
use crate::{errors::TonError, vendor::cell::BagOfCells};
use crate::errors::Result;

// pub fn parse_transaction(serial: &[u8])-> Result<TonTransaction> {
//     // let boc = BagOfCells::parse(serial)?.single_root()?.parse_fully(|parser| {
        
//     // })
// }

#[cfg(test)]
mod tests {
    extern crate std;
    use std::println;
    use base64::{engine::general_purpose::STANDARD, Engine};
    use third_party::hex;

    use crate::vendor::cell::BagOfCells;

    #[test]
    fn test_parse_transaction() {
        // "tonsign://?pk=j7SUzAOty6C3woetBmEXobZoCf6vJZGoQVomHJc42oU=&body=te6cckEBAwEA7AABHCmpoxdmOZW/AAAABgADAQHTYgAIqFqMWTE1aoxM/MRD/EEluAMqKyKvv/FAn4CTTNIDD6B4KbgAAAAAAAAAAAAAAAAAAA+KfqUACSD7UyTMBDtxsAgA7zuZAqJxsqAciTilI8/iTnGEeq62piAAHtRKd6wOcJwQOThwAwIA1yWThWGAApWA5YrFIkZa+bJ7vYJARri8uevEBP6Td4tUTty6RJsGAh5xAC1IywyQwixSOU8pezOZDC9rv2xCV4CGJzOWH6RX8BTsMAK2ELwgIrsrweR+b2yZuUsWugqtisQzBm6gPg1ubkuzBkk1zw8=";
        let body = "te6cckEBAwEA7AABHCmpoxdmOZW/AAAABgADAQHTYgAIqFqMWTE1aoxM/MRD/EEluAMqKyKvv/FAn4CTTNIDD6B4KbgAAAAAAAAAAAAAAAAAAA+KfqUACSD7UyTMBDtxsAgA7zuZAqJxsqAciTilI8/iTnGEeq62piAAHtRKd6wOcJwQOThwAwIA1yWThWGAApWA5YrFIkZa+bJ7vYJARri8uevEBP6Td4tUTty6RJsGAh5xAC1IywyQwixSOU8pezOZDC9rv2xCV4CGJzOWH6RX8BTsMAK2ELwgIrsrweR+b2yZuUsWugqtisQzBm6gPg1ubkuzBkk1zw8=";
        let result = STANDARD.decode(body).unwrap();
        let result = BagOfCells::parse(&result).unwrap();
        println!("{:?}", result);
        result.single_root().unwrap().parse_fully(|parser| {
            let address = parser.load_address().unwrap();
            println!("{}", parser.remaining_bits());
            println!("{}", address);
            Ok(())
        });
        // let result = super::parse_transaction(&serial);
        // assert!(result.is_err());
    }

    #[test]
    fn test_parse_ton_transfer() {
        // tonsign://?network=ton&pk=j7SUzAOty6C3woetBmEXobZoCf6vJZGoQVomHJc42oU%3D&body=te6cckEBAgEARwABHCmpoxdmOz6lAAAACAADAQBoQgArFnMvHAX9tOjTp4%2FRDd3vP2Bn8xG%2BU5MTuKRKUE1NoqHc1lAAAAAAAAAAAAAAAAAAAHBy4G8%3D
        let body = "te6cckEBAgEARwABHCmpoxdmOz6lAAAACAADAQBoQgArFnMvHAX9tOjTp4/RDd3vP2Bn8xG+U5MTuKRKUE1NoqHc1lAAAAAAAAAAAAAAAAAAAHBy4G8=";
        let result = STANDARD.decode(body).unwrap();
        let result = BagOfCells::parse(&result).unwrap();
        println!("{:?}", result);
        let root = result.single_root().unwrap();
        root.parse_fully(|parser| {
            let wallet_id = parser.load_u32(32).unwrap();
            let timeout = parser.load_u32(32).unwrap();
            let seq_no = parser.load_u32(32).unwrap();
            let order = parser.load_u8(8).unwrap();
            let send_mode = parser.load_u8(8).unwrap();
            println!("{}", parser.remaining_bits());
            Ok(())
        });
        root.reference(0).unwrap().parse_fully(|parser| {
            let header = parser.load_bits(6).unwrap();
            let address = parser.load_address().unwrap();
            let coins = parser.load_coins().unwrap();
            println!("{}", address);
            println!("{}", coins);
            println!("{}", parser.remaining_bits());
            Ok(())
        });
    }
}