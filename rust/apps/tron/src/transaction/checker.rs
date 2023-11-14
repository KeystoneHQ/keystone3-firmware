use crate::errors::Result;
use crate::transaction::wrapped_tron::WrappedTron;
use app_utils::keystone;

pub trait TxChecker {
    fn check(&self, context: &keystone::ParseContext) -> Result<()>;
}

impl TxChecker for WrappedTron {
    fn check(&self, context: &keystone::ParseContext) -> Result<()> {
        self.check_input(context)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::errors::TronError;
    use crate::test::{prepare_parse_context, prepare_payload};
    use alloc::string::ToString;

    #[test]
    fn test_check_trx_transfer() {
        // https://tronscan.org/#/transaction/0f89c6365796afa96ae05fab57207a4d8c5cc801b92ac2099c7dd9dcd5a91df0
        // txHex: 0a7c0a02ea0522082c84cb547bae782640b8f3c28bfc305a65080112610a2d747970652e676f6f676c65617069732e636f6d2f70726f746f636f6c2e5472616e73666572436f6e747261637412300a1541bf90ce9c87c42a58699b1a2e3f65e88181436d53121541b0ca665130d03c977823834e3fcbe63af2421fa1180112411634795a1461b2775648062bcee3675033158557e497f1187d0312ae8e4589d07a22cf3334f4b5c49133ada012ccf308baefb8062e24d22d62f8da0a40716da301
        // txId: 0f89c6365796afa96ae05fab57207a4d8c5cc801b92ac2099c7dd9dcd5a91df0
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let context = prepare_parse_context(pubkey_str);
        {
            // invalid xfp
            let hex = "1f8b08000000000000030dcebb4ac3501c807153444a17b553e95482501142ce39f99f4b8b83362d74696cd38897eddca205db60ac22d95db49b4fe0e6e8e4e65bf80a6e3e80a083816ffb965fb552df9ce461666c6b9c67cb4c6757cd5fa75aa95739e283419f85eeb7535b4fe2a3a8bea3710a42748cc729621e60a43cc1c17801a2c03166444bd4dc1ef9006d1f7768db47653e6aad5e578f7f6877a3f7e234dd2499f2331adc4c212ff27eb83c97b3dbb8b8be3798c96568c971ecbac99416e32c84d3416f3ebceb47878bc509b91806a3c9a58e23368be77b6bc4c1dda8760046714cb148b154982949b400ad4a8f92960bc23ae5e20a510bda12c9d2006b0bd2d8148ca582045b0f9fcf8dc6cfd753291cbebdefb73ec6ff42b912d514010000";
            let payload = prepare_payload(hex);
            let tx = WrappedTron::from_payload(payload, &context).unwrap();
            let check = tx.check(&context);
            let error = Err(TronError::InvalidParseContext(
                "invalid xfp, expected 73c5da0a, got 707EED6C".to_string(),
            ));
            assert_eq!(error, check);
        }
        {
            // check From address
            let hex = "1f8b08000000000000038d8fbd4ac35018408938842e6aa792a904a12e21dffdf9bedc2b0eda2a64696c6341d79b9b1b156c2f1471f029fa06be813f8b822fe12b383b886b71318f209ce97096136e74b7a6cb91af5d7fb2f4b7defa9b681db436cc84c5da8089bf82cee6ac3c2dbabbd66a4995e38972d22692572e3182b304c9a030ca36c828da19a7520e52a67190424b0afda7efcff735ec85c387208a6767783ff1237971329ce777c7c5d16271ce2f73319e5ed9b2a0eb721effa3e101db2f3a870d2964ba76bab1504b7002d130dd408502b425210832656466b8d375c51423460a2419ce1c6f27b65f56ab5eefe7e3f1ed17f2e7d7833f1930e17d0f010000";
            let payload = prepare_payload(hex);
            let tx = WrappedTron::from_payload(payload, &context).unwrap();
            let check = tx.check(&context);
            let error = Err(TronError::NoMyInputs);
            assert_eq!(error, check);
        }
        {
            let hex = "1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000";
            let payload = prepare_payload(hex);
            let tx = WrappedTron::from_payload(payload, &context).unwrap();
            let check = tx.check(&context);
            assert_eq!(Ok(()), check);
        }
    }
}
