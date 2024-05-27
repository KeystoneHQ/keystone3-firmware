use crate::errors::Result;
use crate::structs::TonTransaction;
use crate::{errors::TonError, vendor::cell::BagOfCells};
use alloc::vec::Vec;
use third_party::cryptoxide::ed25519;

pub fn parse_transaction(serial: &[u8]) -> Result<TonTransaction> {
    TonTransaction::parse_hex(serial)
}

pub fn sign_transaction(serial: &[u8], sk: [u8; 32]) -> Result<[u8; 64]> {
    let tx = parse_transaction(serial)?;
    let (keypair, _) = ed25519::keypair(&sk);
    let signature = ed25519::signature(&tx.buffer_to_sign, &keypair);
    Ok(signature)
}

#[cfg(test)]
mod tests {
    extern crate std;
    use base64::{engine::general_purpose::STANDARD, Engine};
    use std::println;
    use third_party::hex;
    use urlencoding;

    use crate::{transaction::parse_transaction, vendor::cell::BagOfCells};

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
        let serial = STANDARD.decode(body).unwrap();
        let tx = parse_transaction(&serial).unwrap();
        println!("{:?}", tx);
        let tx_json = tx.to_json().unwrap();
        println!("{}", tx_json);
    }

    #[test]
    fn test_sign_ton_transaction() {
        //j7SUzAOty6C3woetBmEXobZoCf6vJZGoQVomHJc42oU=
        // tonsign://?network=ton&pk=j7SUzAOty6C3woetBmEXobZoCf6vJZGoQVomHJc42oU%3D&body=te6cckEBAgEARwABHCmpoxdmQcAiAAAADQADAQBoQgArFnMvHAX9tOjTp4%2FRDd3vP2Bn8xG%2BU5MTuKRKUE1NoqAvrwgAAAAAAAAAAAAAAAAAAPa2C0o%3D
        let body = "te6cckEBAgEARwABHCmpoxdmQcAiAAAADQADAQBoQgArFnMvHAX9tOjTp4%2FRDd3vP2Bn8xG%2BU5MTuKRKUE1NoqAvrwgAAAAAAAAAAAAAAAAAAPa2C0o%3D";
        let serial = STANDARD.decode(urlencoding::decode(body).unwrap().into_owned()).unwrap();
        let master_seed = "b4933a592c18291855b30ea5cc8da7cb20da17936df875f018c6027f2103f6ad8ff409400be6e913e43a3bf9dd23274f918e3bd7ca679b06e7fee04bc0d41f95";
        let master_seed = hex::decode(master_seed).unwrap();
        let mut sk: [u8; 32] = [0; 32];
        for i in 0..32 {
            sk[i] = master_seed[i]
        }
        let signature = super::sign_transaction(&serial, sk).unwrap();
        println!("{}", hex::encode(&signature));
        println!("tonkeeper://publish?boc={}", urlencoding::encode(&STANDARD.encode(&signature)));
    }
}
