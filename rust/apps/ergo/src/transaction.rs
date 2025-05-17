use crate::errors::{ErgoError, Result};
use crate::structs::{ParseContext, ParsedErgoTx};
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::str::FromStr;
use ergo_lib::chain::transaction::reduced::ReducedTransaction;
use ergo_lib::chain::transaction::Transaction;
use ergo_lib::ergotree_ir::serialization::SigmaSerializable;
use ergo_lib::wallet::derivation_path::DerivationPath;
use ergo_lib::wallet::ext_secret_key::ExtSecretKey;
use ergo_lib::wallet::mnemonic::MnemonicSeed;
use ergo_lib::wallet::Wallet;

pub fn parse_tx(tx: Vec<u8>, context: ParseContext) -> Result<ParsedErgoTx> {
    let reduced_ergo_tx = ReducedTransaction::keystone_sigma_parse_bytes(tx.clone().as_mut_slice())
        .map_err(|e| ErgoError::TransactionParseError(e.to_string()))?;
    ParsedErgoTx::from_ergo_tx(reduced_ergo_tx.unsigned_tx, context)
}

pub fn sign_tx(
    tx: Vec<u8>,
    seed: &[u8],
    entropy: &[u8],
    derivation_paths: &Vec<String>,
) -> Result<Vec<u8>> {
    let reduced_ergo_tx = ReducedTransaction::keystone_sigma_parse_bytes(tx.clone().as_mut_slice())
        .map_err(|e| ErgoError::TransactionParseError(e.to_string()))?;

    let _seed: [u8; 64] = seed
        .try_into()
        .map_err(|_| ErgoError::SigningFailed("invalid seed".to_string()))?;
    let master_key = ExtSecretKey::derive_master(MnemonicSeed::from(_seed))
        .map_err(|e| ErgoError::SigningFailed(e.to_string()))?;

    let secrets = derivation_paths
        .iter()
        .map(|path| DerivationPath::from_str(path.as_str()))
        .flatten()
        .map(|derivation_path| master_key.derive(derivation_path))
        .flatten()
        .map(|ext_secret_key| ext_secret_key.secret_key())
        .collect();

    let wallet = Wallet::from_secrets(secrets);

    let signed_tx: Transaction = wallet
        .sign_reduced_transaction_deterministic(reduced_ergo_tx, entropy)
        .map_err(|e| ErgoError::SigningFailed(e.to_string()))?;

    signed_tx
        .keystone_sigma_serialize_bytes()
        .map_err(|e| ErgoError::SigningFailed(e.to_string()))
}

pub fn check_tx(tx: Vec<u8>, context: ParseContext) -> Result<()> {
    let reduced_ergo_tx = ReducedTransaction::keystone_sigma_parse_bytes(tx.clone().as_mut_slice())
        .map_err(|e| ErgoError::TransactionParseError(e.to_string()))?;

    ParsedErgoTx::verify(reduced_ergo_tx.unsigned_tx, context)
}

mod tests {
    use crate::structs::{ErgoUnspentBox, ParseContext, UnparsedErgoAsset};
    use crate::transaction::{check_tx, parse_tx, sign_tx};
    use alloc::string::ToString;

    #[test]
    fn test_parse_tx() {
        let tx_bytes = hex::decode("9702011a9f15bfac9379c882fe0b7ecb2288153ce4f2def4f272214fb80f8e2630f04c00000001fbbaac7337d051c10fc3da0ccb864f4d32d40027551e1c3ea3ce361f39b91e4003c0843d240008cd02dc5b9d9d2081889ef00e6452fb5ad1730df42444ceccb9ea02258256d2fbd262e4f25601006400c0843d691005040004000e36100204a00b08cd0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798ea02d192a39a8cc7a701730073011001020402d19683030193a38cc7b2a57300000193c2b2a57301007473027303830108cdeeac93b1a57304e4f2560000809bee02240008cd0388fa54338147371023aacb846c96c57e72cdcd73bc85d20250467e5b79dfa2aae4f25601006400cd0388fa54338147371023aacb846c96c57e72cdcd73bc85d20250467e5b79dfa2aa0000").unwrap();

        let parse_context = ParseContext::new(
            vec![ErgoUnspentBox::new(
                "1a9f15bfac9379c882fe0b7ecb2288153ce4f2def4f272214fb80f8e2630f04c".to_string(),
                "9hW8c9CcAzSsogR5rJhbmhMtJkjQhULun4pUCpENCstfoW1yHCB".to_string(),
                8000000,
                Some(vec![UnparsedErgoAsset::new(
                    "fbbaac73...39b91e40".to_string(),
                    200,
                )]),
            )],
            vec!["9hW8c9CcAzSsogR5rJhbmhMtJkjQhULun4pUCpENCstfoW1yHCB".to_string()],
        );

        let ergo_tx = parse_tx(tx_bytes, parse_context);
        assert_eq!(ergo_tx.is_ok(), true);
    }

    #[test]
    fn test_check_tx() {
        let tx_bytes = hex::decode("9702011a9f15bfac9379c882fe0b7ecb2288153ce4f2def4f272214fb80f8e2630f04c00000001fbbaac7337d051c10fc3da0ccb864f4d32d40027551e1c3ea3ce361f39b91e4003c0843d240008cd02dc5b9d9d2081889ef00e6452fb5ad1730df42444ceccb9ea02258256d2fbd262e4f25601006400c0843d691005040004000e36100204a00b08cd0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798ea02d192a39a8cc7a701730073011001020402d19683030193a38cc7b2a57300000193c2b2a57301007473027303830108cdeeac93b1a57304e4f2560000809bee02240008cd0388fa54338147371023aacb846c96c57e72cdcd73bc85d20250467e5b79dfa2aae4f25601006400cd0388fa54338147371023aacb846c96c57e72cdcd73bc85d20250467e5b79dfa2aa0000").unwrap();
        let parse_context = ParseContext::new(
            vec![ErgoUnspentBox::new(
                "1a9f15bfac9379c882fe0b7ecb2288153ce4f2def4f272214fb80f8e2630f04c".to_string(),
                "9hW8c9CcAzSsogR5rJhbmhMtJkjQhULun4pUCpENCstfoW1yHCB".to_string(),
                8000000,
                Some(vec![UnparsedErgoAsset::new(
                    "fbbaac73...39b91e40".to_string(),
                    200,
                )]),
            )],
            vec!["9hW8c9CcAzSsogR5rJhbmhMtJkjQhULun4pUCpENCstfoW1yHCB".to_string()],
        );

        let check_result = check_tx(tx_bytes.clone(), parse_context);
        assert_eq!(check_result.is_ok(), true);
    }

    #[test]
    fn test_sign_tx() {
        let tx_bytes = hex::decode("9702011a9f15bfac9379c882fe0b7ecb2288153ce4f2def4f272214fb80f8e2630f04c00000001fbbaac7337d051c10fc3da0ccb864f4d32d40027551e1c3ea3ce361f39b91e4003c0843d240008cd02dc5b9d9d2081889ef00e6452fb5ad1730df42444ceccb9ea02258256d2fbd262e4f25601006400c0843d691005040004000e36100204a00b08cd0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798ea02d192a39a8cc7a701730073011001020402d19683030193a38cc7b2a57300000193c2b2a57301007473027303830108cdeeac93b1a57304e4f2560000809bee02240008cd0388fa54338147371023aacb846c96c57e72cdcd73bc85d20250467e5b79dfa2aae4f25601006400cd0388fa54338147371023aacb846c96c57e72cdcd73bc85d20250467e5b79dfa2aa0000").unwrap();
        let seed = [
            89, 218, 62, 125, 241, 91, 168, 234, 228, 236, 37, 226, 207, 147, 146, 54, 18, 249,
            149, 130, 132, 49, 185, 116, 81, 79, 48, 111, 93, 172, 122, 203, 51, 108, 240, 107, 8,
            140, 24, 249, 33, 255, 46, 241, 130, 90, 235, 135, 162, 15, 206, 189, 128, 18, 229,
            217, 204, 248, 77, 255, 114, 24, 146, 35,
        ];
        let entropy = [
            145, 73, 62, 160, 63, 92, 17, 62, 48, 237, 66, 83, 168, 148, 78, 43, 21, 221, 55, 42,
            11, 6, 87, 63, 0, 26, 34, 99, 146, 201, 217, 77,
        ];
        let derivation_paths = vec!["m/44'/429'/0'/0/6".to_string()];

        let sign_result = sign_tx(tx_bytes, &seed, &entropy, &derivation_paths);
        assert_eq!(sign_result.is_ok(), true);
    }
}
