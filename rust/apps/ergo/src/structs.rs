use app_utils::{impl_public_struct};
use crate::errors::{ErgoError, Result};
use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;
use core::ops::Div;
use ergo_lib::chain::transaction::ergo_transaction::ErgoTransaction;
use ergo_lib::chain::transaction::unsigned::UnsignedTransaction;
use ergo_lib::ergotree_ir::chain::address::{Address, NetworkAddress};
use ergo_lib::ergotree_ir::chain::address::NetworkPrefix::Mainnet;
use ergo_lib::ergotree_ir::chain::ergo_box::BoxTokens;
use ergo_lib::ergotree_ir::serialization::SigmaSerializable;
use ergo_lib::wallet::box_selector::ErgoBoxAssets;
use ergo_lib::wallet::miner_fee::{MINERS_FEE_ADDRESS, MINERS_FEE_MAINNET_ADDRESS_STR};
use hex;

impl_public_struct!(ParsedErgoTx {
    fee: String,
    total_input: String,
    total_output: String,
    from: Vec<ParsedErgoInput>,
    to: Vec<ParsedErgoOutput>
});

impl_public_struct!(ParsedErgoInput {
    box_id: String,
    value: u64,
    amount: String,
    address: String,
    assets: Option<Vec<String>>,
    is_mine: bool
});

impl_public_struct!(ParsedErgoOutput {
    value: u64,
    amount: String,
    address: String,
    assets: Option<Vec<String>>,
    is_change: bool,
    is_fee: bool
});

impl_public_struct!(ParsedErgoAsset {
    token_id: String,
    value: u64
});

impl_public_struct!(ParseContext {
   utxos: Vec<ErgoUnspentBox>,
   addresses: Vec<String>
});

impl_public_struct!(UnparsedErgoAsset {
    token_id: String,
    value: u64
});

impl_public_struct!(ErgoUnspentBox {
   box_id: String,
   address: String,
   value: u64,
   assets: Option<Vec<UnparsedErgoAsset>>
});


impl ParsedErgoTx {
    pub fn from_ergo_tx(tx: UnsignedTransaction, context: ParseContext) -> Result<Self> {
        let parsed_inputs = Self::parse_inputs(&tx, &context)?;
        let parsed_outputs = Self::parse_outputs(&tx, &context)?;

        let fee = {
            let _v = &tx.output_candidates.iter()
                .filter(|x| Address::recreate_from_ergo_tree(&x.ergo_tree)
                    .map(|address| address.eq(&MINERS_FEE_ADDRESS))
                    .unwrap_or(false)
                )
                .map(|fee| fee.value.as_u64())
                .fold(0u64, |acc, x| acc + x);
            normalize_coin(*_v)
        };

        let total_input_amount = {
            let _v = parsed_inputs.iter().fold(0u64, |acc, cur | acc + cur.value);
            normalize_coin(_v)
        };

        let total_output_amount = {
            let _v = parsed_outputs.iter().fold(0u64, |acc, cur| acc + cur.value);
            normalize_coin(_v)
        };


        Ok(Self {
            total_input: total_input_amount,
            total_output: total_output_amount,
            from: parsed_inputs,
            to: parsed_outputs,
            fee,
        })
    }

    pub fn verify(tx: UnsignedTransaction, context: ParseContext) -> Result<()> {
        match Self::parse_inputs(&tx, &context) {
            Ok(_) => {}
            Err(e) => {
                return Err(ErgoError::TransactionParseError(e.to_string()));
            }
        }

        match Self::parse_outputs(&tx, &context) {
            Ok(_) => {}
            Err(e) => {
                return Err(ErgoError::TransactionParseError(e.to_string()));
            }
        }

        tx.validate_stateless()
            .map_err(|e| ErgoError::InvalidTransaction(e.to_string()))
    }

    fn parse_inputs(tx: &UnsignedTransaction, context: &ParseContext) -> Result<Vec<ParsedErgoInput>> {
        let mut parsed_inputs = vec![];
        for input in tx.inputs.iter()  {
            let box_id = input.box_id.to_string();

            let utxo = context.utxos.iter()
                .find(|x| x.box_id == box_id.as_str())
                .ok_or(ErgoError::TransactionParseError(format!("box_id {} not found in utxos", box_id)))?;

            let address = NetworkAddress::try_from(utxo.address.clone())
                .map_err(|e| ErgoError::TransactionParseError(format!("could not parse address in input {}, {}", e.to_string(), utxo.address)))?;
            let address_str = normalize_address(address.to_base58().as_str());

            let assets_formatted = match utxo.clone().assets {
                None => None,
                Some(tokens) => Some(tokens
                    .iter()
                    .map(|v| format!("{} Token Id: {}", v.value, normalize_token_id(&*v.token_id)))
                    .collect())
            };

            parsed_inputs.push(ParsedErgoInput {
                box_id,
                value: utxo.value,
                amount: normalize_coin(utxo.value),
                address: address_str.clone(),
                assets: assets_formatted,
                is_mine: context.addresses.contains(&address.to_base58())
            });
        }
        Ok(parsed_inputs)
    }

    fn parse_outputs(tx: &UnsignedTransaction, context: &ParseContext) -> Result<Vec<ParsedErgoOutput>> {
        let mut parsed_outputs = vec![];
        for output in tx.output_candidates.iter() {

           let address = NetworkAddress::new(Mainnet, &Address::recreate_from_ergo_tree(&output.ergo_tree)
               .map_err(|e| ErgoError::TransactionParseError(e.to_string()))?);
           let address_str = normalize_address(address.to_base58().as_str());

            let value = output.value.as_u64();

            let assets = match &output.tokens {
                None => None,
                Some(tokens) => Some(Self::parse_assets(tokens))
            };

            let assets_formatted = match assets {
                None => None,
                Some(tokens) => Some(tokens
                    .iter()
                    .map(|v| format!("{} Token Id: {}", v.value, v.token_id))
                    .collect())
            };

            parsed_outputs.push(ParsedErgoOutput {
                value: *value,
                amount: normalize_coin(*value),
                address: address_str.clone(),
                assets: assets_formatted,
                is_change: context.addresses.contains(&address.to_base58()),
                is_fee: address.to_base58().eq(&MINERS_FEE_MAINNET_ADDRESS_STR.as_str()),
            });
        }
        Ok(parsed_outputs)
    }

    fn parse_assets(tokens: &BoxTokens) -> Vec<ParsedErgoAsset> {
        let mut parsed_assets = vec![];
        for token in tokens.iter() {
            let token_id = token.token_id.sigma_serialize_bytes()
                .map(|bytes| hex::encode(bytes))
                .map(|token_id| normalize_token_id(token_id.as_str()))
                .map_err(|e| ErgoError::TransactionParseError(e.to_string())).unwrap();

            let value = token.amount.as_u64();

            parsed_assets.push(ParsedErgoAsset {
                token_id,
                value: *value,
            });
        }
        parsed_assets
    }
}

static UNITS_PER_ERGO: f64 = 1_000_000_000f64;

fn normalize_coin(value: u64) -> String {
    format!("{} ERG", (value as f64).div(UNITS_PER_ERGO))
}

fn normalize_token_id(token_id: &str) -> String {
    format!("{}...{}", &token_id[0..8], &token_id[token_id.len() - 8..])
}

fn normalize_address(address: &str) -> String {
    if address.len() > 52 {
        return format!("{}...{}", &address[0..25], &address[address.len() - 24..])
    }
    format!("{}", address)
}

mod tests {
    use alloc::string::ToString;
    use ergo_lib::chain::transaction::reduced::ReducedTransaction;
    use ergo_lib::ergotree_ir::serialization::SigmaSerializable;
    use crate::structs::{normalize_coin, normalize_token_id, ErgoUnspentBox, ParseContext, ParsedErgoAsset, ParsedErgoTx, UnparsedErgoAsset};

    #[test]
    fn text_normalize_coin() {
        let value = 1_000_000_000u64;
        assert_eq!(normalize_coin(value), "1 ERG");
    }

    #[test]
    fn test_normalize_token() {
        let token_id = "fbbaac7337d051c10fc3da0ccb864f4d32d40027551e1c3ea3ce361f39b91e40";
        assert_eq!(normalize_token_id(token_id), "fbbaac73...39b91e40");
    }

    #[test]
    fn test_parse_ergo_tx() {

        let tx_bytes = hex::decode("9402011a9f15bfac9379c882fe0b7ecb2288153ce4f2def4f272214fb80f8e2630f04c00000001fbbaac7337d051c10fc3da0ccb864f4d32d40027551e1c3ea3ce361f39b91e4003c0843d0008cd02dc5b9d9d2081889ef00e6452fb5ad1730df42444ceccb9ea02258256d2fbd262e4f25601006400c0843d1005040004000e36100204a00b08cd0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798ea02d192a39a8cc7a701730073011001020402d19683030193a38cc7b2a57300000193c2b2a57301007473027303830108cdeeac93b1a57304e4f2560000809bee020008cd0388fa54338147371023aacb846c96c57e72cdcd73bc85d20250467e5b79dfa2aae4f25601006400cd0388fa54338147371023aacb846c96c57e72cdcd73bc85d20250467e5b79dfa2aa0000").unwrap();

        let reduced_tx = ReducedTransaction::sigma_parse_bytes(&*tx_bytes).unwrap();

        let parse_context = ParseContext::new(
            vec![
                ErgoUnspentBox {
                    box_id: "1a9f15bfac9379c882fe0b7ecb2288153ce4f2def4f272214fb80f8e2630f04c".to_string(),
                    address: "9hW8c9CcAzSsogR5rJhbmhMtJkjQhULun4pUCpENCstfoW1yHCB".to_string(),
                    value: 8000000,
                    assets: Some(vec![UnparsedErgoAsset::new("fbbaac73...39b91e40".to_string(), 200)]),
                }
            ],vec!["9hW8c9CcAzSsogR5rJhbmhMtJkjQhULun4pUCpENCstfoW1yHCB".to_string()]
        );


        let ergo_tx = ParsedErgoTx::from_ergo_tx(reduced_tx.unsigned_tx, parse_context);
        assert_eq!(ergo_tx.is_ok(), true);
    }
}

