use crate::errors::Result;
use crate::structs::{TonProof, TonTransaction};
use crate::utils::sha256;
use crate::vendor::cell::BagOfCells;
use alloc::{format, vec};
use alloc::vec::Vec;
use third_party::cryptoxide::ed25519;
use third_party::hex;

pub fn parse_transaction(serial: &[u8]) -> Result<TonTransaction> {
    TonTransaction::parse_hex(serial)
}

pub fn buffer_to_sign(serial: &[u8]) -> Result<Vec<u8>> {
    let boc = BagOfCells::parse(serial)?;
    let root = boc.single_root()?;
    let buffer_to_sign = root.cell_hash()?;
    Ok(buffer_to_sign)
}

pub fn sign_transaction(serial: &[u8], sk: [u8; 32]) -> Result<[u8; 64]> {
    let tx = buffer_to_sign(serial)?;
    let (keypair, _) = ed25519::keypair(&sk);
    let signature = ed25519::signature(&tx, &keypair);
    Ok(signature)
}

pub fn parse_proof(serial: &[u8]) -> Result<TonProof> {
    TonProof::parse_hex(serial)
}

pub fn sign_proof(serial: &[u8], sk: [u8; 32]) -> Result<[u8; 64]> {
    let message = [vec![0xff, 0xff], "ton-connect".as_bytes().to_vec(), sha256(serial)].concat();
    let hash = sha256(&message);
    let (keypair, _) = ed25519::keypair(&sk);
    let signature = ed25519::signature(&hash, &keypair);
    Ok(signature)
}

#[cfg(test)]
mod tests {
    extern crate std;
    use base64::{engine::general_purpose::STANDARD, Engine};
    use num_traits::sign;
    use std::println;
    use third_party::hex;
    use urlencoding;

    use crate::{transaction::parse_transaction, vendor::cell::BagOfCells};

    use super::sign_proof;

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
        let serial = STANDARD
            .decode(urlencoding::decode(body).unwrap().into_owned())
            .unwrap();
        let master_seed = "b4933a592c18291855b30ea5cc8da7cb20da17936df875f018c6027f2103f6ad8ff409400be6e913e43a3bf9dd23274f918e3bd7ca679b06e7fee04bc0d41f95";
        let master_seed = hex::decode(master_seed).unwrap();
        let mut sk: [u8; 32] = [0; 32];
        for i in 0..32 {
            sk[i] = master_seed[i]
        }
        let signature = super::sign_transaction(&serial, sk).unwrap();
        println!("{}", hex::encode(&signature));
        println!(
            "tonkeeper://publish?boc={}",
            urlencoding::encode(&STANDARD.encode(&signature))
        );
    }

    #[test]
    fn test_parse_ston_transfer() {
        let serial = "b5ee9c7241010301009e00011c29a9a3176656eb410000001000030101686200091c1bd942402db834b5977d2a1313119c3a3800c8e10233fa8eaf36c655ecab202faf0800000000000000000000000000010200a80f8a7ea5546de4ef815e87fb3989680800ac59ccbc7017f6d3a34e9e3f443777bcfd819fcc46f94e4c4ee291294135368b002d48cb0c90c22c52394f297b33990c2f6bbf6c425780862733961fa457f014ec02025f1050ae";
        let serial = hex::decode(serial).unwrap();
        let tx = parse_transaction(&serial).unwrap();
        //true destination UQBWLOZeOAv7adGnTx-iG7vefsDP5iN8pyYncUiUoJqbRdx9
        //transaction to: EQASODeyhIBbcGlrLvpUJiYjOHRwAZHCBGf1HV5tjKvZVsJb
        //contract destination: EQBWLOZeOAv7adGnTx+iG7vefsDP5iN8pyYncUiUoJqbRYG4
        println!("{:?}", tx);
    }

    // #[test]
    // fn test_ston_provide_liqudity() {
    //     let serial = hex::decode("b5ee9c724102060100016500021e29a9a31766612b0c00000015000303030101d3620008a85a8c5931356a8c4cfcc443fc4125b8032a2b22afbff1409f80934cd2030fa07c8acff8000000000000000000000000000f8a7ea500169076c4a3033231210ff800ef3b9902a271b2a01c8938a523cfe24e71847aaeb6a620001ed44a77ac0e709c103dfd240302004ffcf9e58f8010f72448354d4afbe624e28c182138a9bfb313435d4065ec20fa58cbadd10f39c4034901686200091c1bd942402db834b5977d2a1313119c3a3800c8e10233fa8eaf36c655ecab208f0d1800000000000000000000000000010401ae0f8a7ea5001828589d5aad523079df3800ef3b9902a271b2a01c8938a523cfe24e71847aaeb6a620001ed44a77ac0e709d002d48cb0c90c22c52394f297b33990c2f6bbf6c425780862733961fa457f014ec081c9c380105004ffcf9e58f80022a16a3164c4d5aa3133f3110ff10496e00ca8ac8abeffc5027e024d33480c3e403498878abc8").unwrap();
    //     let tx = parse_transaction(&serial).unwrap();
    //     //true destination UQBWLOZeOAv7adGnTx-iG7vefsDP5iN8pyYncUiUoJqbRdx9
    //     //transaction to: EQASODeyhIBbcGlrLvpUJiYjOHRwAZHCBGf1HV5tjKvZVsJb
    //     //contract destination: EQBWLOZeOAv7adGnTx+iG7vefsDP5iN8pyYncUiUoJqbRYG4
    //     println!("{:?}", tx);
    // }

    #[test]
    fn test_parse_ton_transfer_with_comment() {
        let serial = "b5ee9c724102050100019700011c29a9a31766611df6000000140003010166420013587ccf19c39b1ca51c29f0253ac98d03b8e5ccfc64c3ac2f21c59c20ee8b65987a1200000000000000000000000000010201fe000000004b657973746f6e652068617264776172652077616c6c6574206f666665727320756e6265617461626c65207365637572697479207769746820332050434920736563757269747920636869707320746f206d616e61676520426974636f696e20616e64206f746865722063727970746f20617373657473206f66660301fe6c696e652e4b657973746f6e65206f666665727320332077616c6c6574732c207768696368206d65616e7320796f752063616e206d616e616765206d756c7469706c65206163636f756e74732073657061726174656c79206f6e206f6e65206465766963652e4b657973746f6e65206f666665727320332077616c6c6574730400942c207768696368206d65616e7320796f752063616e206d616e616765206d756c7469706c65206163636f756e74732073657061726174656c79206f6e206f6e65206465766963652e0a0ac04eabc7";
        let serial = hex::decode(serial).unwrap();
        let tx = parse_transaction(&serial).unwrap();
        println!("{:?}", tx);
    }

    #[test]
    fn test_sign_ton_proof() {
        let serial = hex::decode("746f6e2d70726f6f662d6974656d2d76322f00000000b5232c324308b148e53ca5ecce6430bdaefdb1095e02189cce587e915fc053b015000000746b6170702e746f6e706f6b65722e6f6e6c696e65142b5866000000003735323061653632393534653666666330303030303030303636353765333639").unwrap();
        //b4933a592c18291855b30ea5cc8da7cb20da17936df875f018c6027f2103f6ad
        let signature = sign_proof(&serial, [0u8; 32]);
        println!("{}", hex::encode(signature.unwrap()));

        // ffff746f6e2d636f6e6e6563745adfd8ce7eeb56a65c82002216e042f69abe0dfb40b13b1096b5a817e8f9e8d7
        // e87aadb24661f2b3b517a4a3b24cda5aef17dc381bd99cc971811c4c096385b3
        // f7dfec305cd324692fcb73ce55700724a86b22e3f74d3f06b6712da2f5cfd1b7cfd4a0b1944311951b4c4829dee97dd2fbef989afbcc5756408daa95e6ad1d02

        // ffff746f6e2d636f6e6e6563745adfd8ce7eeb56a65c82002216e042f69abe0dfb40b13b1096b5a817e8f9e8d7
        // e87aadb24661f2b3b517a4a3b24cda5aef17dc381bd99cc971811c4c096385b3
        // f7dfec305cd324692fcb73ce55700724a86b22e3f74d3f06b6712da2f5cfd1b7cfd4a0b1944311951b4c4829dee97dd2fbef989afbcc5756408daa95e6ad1d02
    }
}
