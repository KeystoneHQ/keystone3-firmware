use crate::errors::Result;
use crate::structs::{TonProof, TonTransaction};
use crate::utils::sha256;
use crate::vendor::cell::BagOfCells;
use alloc::vec;
use alloc::vec::Vec;
use cryptoxide::ed25519;

pub fn parse_transaction(serial: &[u8]) -> Result<TonTransaction> {
    TonTransaction::parse_hex(serial)
}

pub fn buffer_to_sign(serial: &[u8]) -> Result<Vec<u8>> {
    let boc = BagOfCells::parse(serial)?;
    let root = boc.single_root()?;

    let buffer_to_sign = root.cell_hash();
    Ok(buffer_to_sign.to_vec())
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
    let message = [
        vec![0xff, 0xff],
        "ton-connect".as_bytes().to_vec(),
        sha256(serial),
    ]
    .concat();
    let hash = sha256(&message);
    let (keypair, _) = ed25519::keypair(&sk);
    let signature = ed25519::signature(&hash, &keypair);
    Ok(signature)
}

#[cfg(test)]
mod tests {
    extern crate std;
    use alloc::string::ToString;
    use base64::{engine::general_purpose::STANDARD, Engine};
    use hex;
    use std::println;
    use urlencoding;

    use crate::{transaction::parse_transaction, vendor::cell::BagOfCells};

    use super::sign_proof;

    #[test]
    fn test_parse_transaction() {
        // "tonsign://?pk=j7SUzAOty6C3woetBmEXobZoCf6vJZGoQVomHJc42oU=&body=te6cckEBAwEA7AABHCmpoxdmOZW/AAAABgADAQHTYgAIqFqMWTE1aoxM/MRD/EEluAMqKyKvv/FAn4CTTNIDD6B4KbgAAAAAAAAAAAAAAAAAAA+KfqUACSD7UyTMBDtxsAgA7zuZAqJxsqAciTilI8/iTnGEeq62piAAHtRKd6wOcJwQOThwAwIA1yWThWGAApWA5YrFIkZa+bJ7vYJARri8uevEBP6Td4tUTty6RJsGAh5xAC1IywyQwixSOU8pezOZDC9rv2xCV4CGJzOWH6RX8BTsMAK2ELwgIrsrweR+b2yZuUsWugqtisQzBm6gPg1ubkuzBkk1zw8=";
        let body = "te6cckEBAwEA7AABHCmpoxdmOZW/AAAABgADAQHTYgAIqFqMWTE1aoxM/MRD/EEluAMqKyKvv/FAn4CTTNIDD6B4KbgAAAAAAAAAAAAAAAAAAA+KfqUACSD7UyTMBDtxsAgA7zuZAqJxsqAciTilI8/iTnGEeq62piAAHtRKd6wOcJwQOThwAwIA1yWThWGAApWA5YrFIkZa+bJ7vYJARri8uevEBP6Td4tUTty6RJsGAh5xAC1IywyQwixSOU8pezOZDC9rv2xCV4CGJzOWH6RX8BTsMAK2ELwgIrsrweR+b2yZuUsWugqtisQzBm6gPg1ubkuzBkk1zw8=";
        let result = STANDARD.decode(body).unwrap();
        let boc = BagOfCells::parse(&result).unwrap();

        assert_eq!(boc.roots.len(), 1);
        let root = boc.single_root().unwrap();

        let _ = root.parse_fully(|parser| {
            let _address = parser.load_address().unwrap();
            let remaining = parser.remaining_bits();
            assert_eq!(remaining, 110);
            Ok(())
        });
    }

    #[test]
    fn test_parse_ton_transfer() {
        // tonsign://?network=ton&pk=j7SUzAOty6C3woetBmEXobZoCf6vJZGoQVomHJc42oU%3D&body=te6cckEBAgEARwABHCmpoxdmOz6lAAAACAADAQBoQgArFnMvHAX9tOjTp4%2FRDd3vP2Bn8xG%2BU5MTuKRKUE1NoqHc1lAAAAAAAAAAAAAAAAAAAHBy4G8%3D
        let body = "te6cckEBAgEARwABHCmpoxdmOz6lAAAACAADAQBoQgArFnMvHAX9tOjTp4/RDd3vP2Bn8xG+U5MTuKRKUE1NoqHc1lAAAAAAAAAAAAAAAAAAAHBy4G8=";
        let serial = STANDARD.decode(body).unwrap();
        let tx = parse_transaction(&serial).unwrap();

        assert_eq!(tx.to, "UQBWLOZeOAv7adGnTx-iG7vefsDP5iN8pyYncUiUoJqbRdx9");
        assert_eq!(tx.amount, "1 Ton");
        assert_eq!(tx.action, "Ton Transfer");
        assert!(tx.comment.is_none());
        assert!(tx.data_view.is_none());
        assert!(tx.contract_data.is_none());

        let tx_json = tx.to_json().unwrap();
        let tx_json_str = tx_json.to_string();
        assert!(tx_json_str.contains("\"action\":\"Ton Transfer\""));
        assert!(tx_json_str.contains("\"amount\":\"1 Ton\""));
        assert!(tx_json_str.contains("\"to\":\"UQBWLOZeOAv7adGnTx-iG7vefsDP5iN8pyYncUiUoJqbRdx9\""));
    }

    #[test]
    fn test_sign_ton_transaction() {
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
        println!("{}", hex::encode(signature));
        println!(
            "tonkeeper://publish?boc={}",
            urlencoding::encode(&STANDARD.encode(signature))
        );
    }

    #[test]
    fn test_parse_ston_transfer() {
        let serial = "b5ee9c7241010301009e00011c29a9a3176656eb410000001000030101686200091c1bd942402db834b5977d2a1313119c3a3800c8e10233fa8eaf36c655ecab202faf0800000000000000000000000000010200a80f8a7ea5546de4ef815e87fb3989680800ac59ccbc7017f6d3a34e9e3f443777bcfd819fcc46f94e4c4ee291294135368b002d48cb0c90c22c52394f297b33990c2f6bbf6c425780862733961fa457f014ec02025f1050ae";
        let serial = hex::decode(serial).unwrap();
        let tx = parse_transaction(&serial).unwrap();

        // true destination UQBWLOZeOAv7adGnTx-iG7vefsDP5iN8pyYncUiUoJqbRdx9
        assert_eq!(tx.to, "UQBWLOZeOAv7adGnTx-iG7vefsDP5iN8pyYncUiUoJqbRdx9");
        assert_eq!(tx.amount, "10000000 Unit");
        assert_eq!(tx.action, "Jetton Transfer");
        assert!(tx.comment.is_none());

        assert!(tx.data_view.is_some());
        let data_view = tx.data_view.unwrap();
        assert!(data_view.contains("\"amount\":\"10000000\""));
        assert!(data_view
            .contains("\"destination\":\"UQBWLOZeOAv7adGnTx-iG7vefsDP5iN8pyYncUiUoJqbRdx9\""));
        assert!(data_view.contains("\"forward_ton_amount\":\"1\""));

        // transaction to: EQASODeyhIBbcGlrLvpUJiYjOHRwAZHCBGf1HV5tjKvZVsJb
        assert!(tx.contract_data.is_some());
        let contract_data = tx.contract_data.unwrap();
        assert!(contract_data.contains("Jetton Wallet Address"));
        assert!(contract_data.contains("EQASODeyhIBbcGlrLvpUJiYjOHRwAZHCBGf1HV5tjKvZVsJb"));
    }

    #[test]
    fn test_parse_ton_transfer_with_comment() {
        let serial = "b5ee9c724102050100019700011c29a9a31766611df6000000140003010166420013587ccf19c39b1ca51c29f0253ac98d03b8e5ccfc64c3ac2f21c59c20ee8b65987a1200000000000000000000000000010201fe000000004b657973746f6e652068617264776172652077616c6c6574206f666665727320756e6265617461626c65207365637572697479207769746820332050434920736563757269747920636869707320746f206d616e61676520426974636f696e20616e64206f746865722063727970746f20617373657473206f66660301fe6c696e652e4b657973746f6e65206f666665727320332077616c6c6574732c207768696368206d65616e7320796f752063616e206d616e616765206d756c7469706c65206163636f756e74732073657061726174656c79206f6e206f6e65206465766963652e4b657973746f6e65206f666665727320332077616c6c6574730400942c207768696368206d65616e7320796f752063616e206d616e616765206d756c7469706c65206163636f756e74732073657061726174656c79206f6e206f6e65206465766963652e0a0ac04eabc7";
        let serial = hex::decode(serial).unwrap();
        let tx = parse_transaction(&serial).unwrap();

        assert_eq!(tx.to, "UQAmsPmeM4c2OUo4U-BKdZMaB3HLmfjJh1heQ4s4Qd0Wy7Nc");
        assert_eq!(tx.amount, "0.001 Ton");
        assert_eq!(tx.action, "Ton Transfer");
        assert!(tx.comment.is_some());

        let comment = tx.comment.unwrap();
        assert!(comment.contains("Keystone hardware wallet"));
        assert!(comment.contains("3 PCI security chips"));
        assert!(comment.contains("Bitcoin and other crypto assets offline"));
        assert!(comment.contains("3 wallets"));
        assert!(comment.contains("manage multiple accounts"));
    }

    #[test]
    fn test_sign_ton_proof() {
        let serial = hex::decode("746f6e2d70726f6f662d6974656d2d76322f00000000b5232c324308b148e53ca5ecce6430bdaefdb1095e02189cce587e915fc053b015000000746b6170702e746f6e706f6b65722e6f6e6c696e65142b5866000000003735323061653632393534653666666330303030303030303636353765333639").unwrap();
        let signature = sign_proof(&serial, [0u8; 32]);
        assert_eq!(hex::encode(signature.unwrap()), "aad6c3f5236a56e4aa3b66d68504895987c40e34b67fd9f353ef9037eb814c3eab51fecea8394412fdeacbd5d68aa1706f925dd8c607c8ecea42ad9b39572f00");
    }
}
