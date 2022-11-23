use crate::errors::Result;
use crate::transaction::wrapped_tron::{WrappedTron, NETWORK};
use alloc::string::{String, ToString};

#[derive(Debug, Eq, PartialEq)]
pub struct ParsedTx {
    pub overview: OverviewTx,
    pub detail: DetailTx,
}

#[derive(Debug, Clone, Eq, PartialEq)]
pub struct OverviewTx {
    pub value: String,
    pub method: String,
    pub from: String,
    pub to: String,
    pub network: String,
}

#[derive(Debug, Clone, Eq, PartialEq)]
pub struct DetailTx {
    pub value: String,
    pub method: String,
    pub from: String,
    pub to: String,
    pub network: String,
    pub token: String,
    pub contract_address: String,
}

pub trait TxParser {
    fn parse(&self) -> Result<ParsedTx>;
}

impl TxParser for WrappedTron {
    fn parse(&self) -> Result<ParsedTx> {
        let overview = OverviewTx {
            value: self.format_amount()?,
            method: self.format_method()?,
            from: self.from.to_string(),
            to: self.to.to_string(),
            network: NETWORK.to_string(),
        };
        let detail = DetailTx {
            value: self.format_amount()?,
            method: self.format_method()?,
            from: self.from.to_string(),
            to: self.to.to_string(),
            network: NETWORK.to_string(),
            contract_address: self.contract_address.to_string(),
            token: self.token.to_string(),
        };
        Ok(ParsedTx { overview, detail })
    }
}

#[cfg(test)]
mod tests {
    use crate::test::{prepare_parse_context, prepare_payload};
    use crate::transaction::wrapped_tron::WrappedTron;
    use crate::TxParser;
    use alloc::string::ToString;

    #[test]
    fn test_parse_trx_transfer() {
        let hex = "1f8b08000000000000030dcebb4ac3501c807153444a17b553e95482501142ce39f99f4b8b83362d74696cd38897eddca205db60ac22d95db49b4fe0e6e8e4e65bf80a6e3e80a083816ffb965fb552df9ce461666c6b9c67cb4c6757cd5fa75aa95739e283419f85eeb7535b4fe2a3a8bea3710a42748cc729621e60a43cc1c17801a2c03166444bd4dc1ef9006d1f7768db47653e6aad5e578f7f6877a3f7e234dd2499f2331adc4c212ff27eb83c97b3dbb8b8be3798c96568c971ecbac99416e32c84d3416f3ebceb47878bc509b91806a3c9a58e23368be77b6bc4c1dda8760046714cb148b154982949b400ad4a8f92960bc23ae5e20a510bda12c9d2006b0bd2d8148ca582045b0f9fcf8dc6cfd753291cbebdefb73ec6ff42b912d514010000";
        let pubkey_str = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let tx = WrappedTron::from_payload(payload, &context).unwrap();
        let parsed_tx = tx.parse().unwrap();
        assert_eq!(
            "TTS7Y53sS4rzrDCtZaiuRzqxd16atCe2UR".to_string(),
            parsed_tx.overview.from
        );
        assert_eq!(
            "TTS7Y53sS4rzrDCtZaiuRzqxd16atCe2UR".to_string(),
            parsed_tx.detail.from
        );
        assert_eq!(
            "TS5zPoC4XEBmHvDNAnnW2gH3MQhcRN6iRm".to_string(),
            parsed_tx.overview.to
        );
        assert_eq!(
            "TS5zPoC4XEBmHvDNAnnW2gH3MQhcRN6iRm".to_string(),
            parsed_tx.detail.to
        );
        assert_eq!("TRON".to_string(), parsed_tx.overview.network);
        assert_eq!("TRON".to_string(), parsed_tx.detail.network);
        assert_eq!("0.000001 TRX".to_string(), parsed_tx.overview.value);
        assert_eq!("0.000001 TRX".to_string(), parsed_tx.detail.value);
        assert_eq!("TRX Transfer".to_string(), parsed_tx.detail.method);
        assert_eq!("TRX Transfer".to_string(), parsed_tx.overview.method);
        assert_eq!(true, parsed_tx.detail.contract_address.is_empty());
        assert_eq!(true, parsed_tx.detail.token.is_empty());
    }

    #[test]
    fn test_parse_trc10_transfer() {
        let hex = "1f8b0800000000000003158f3d4bc350188535961ab26833954e21081521e4bd37f723d7499316ba34b669c48f45927b532dd8066315e9eee4e8a24efe0e41f1e7b83ab92898c2339ce53c9ca36be6c6b00c0b955b83b29817b2b86cbd68ba66ea1c78b7db61a1fda019b5243e88ccad2c932af541390c98ef108eb9937a923852082214521268d66af45d42da2e12b4ed42850bd6dfd7c7fd2f6cd783cf55631d01600068d94932e227d4bb1e91725176c2f9693ab9891757770ab1741ee6f830b6ed644417832224c7dd60dabbed44fbb3d9113eef79fde1858c233689a73b2bb85629613732f628833c0720922a210021c17d5e2130f1949f235f88713e56482a9a519a12ce709a7a385355dda33e929bdf6f8fcde6d3f3726dd030d6822431f5e5f5b32a35ebbdd71f66bd0ffe01545edbc933010000";
        let pubkey_str = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let tx = WrappedTron::from_payload(payload, &context).unwrap();
        let parsed_tx = tx.parse().unwrap();
        assert_eq!(
            "TTS7Y53sS4rzrDCtZaiuRzqxd16atCe2UR".to_string(),
            parsed_tx.overview.from
        );
        assert_eq!(
            "TTS7Y53sS4rzrDCtZaiuRzqxd16atCe2UR".to_string(),
            parsed_tx.detail.from
        );
        assert_eq!(
            "TS5zPoC4XEBmHvDNAnnW2gH3MQhcRN6iRm".to_string(),
            parsed_tx.overview.to
        );
        assert_eq!(
            "TS5zPoC4XEBmHvDNAnnW2gH3MQhcRN6iRm".to_string(),
            parsed_tx.detail.to
        );
        assert_eq!("TRON".to_string(), parsed_tx.overview.network);
        assert_eq!("TRON".to_string(), parsed_tx.detail.network);
        assert_eq!("0.001 BTT".to_string(), parsed_tx.overview.value);
        assert_eq!("0.001 BTT".to_string(), parsed_tx.detail.value);
        assert_eq!("TRC-10 Transfer".to_string(), parsed_tx.detail.method);
        assert_eq!("TRC-10 Transfer".to_string(), parsed_tx.overview.method);
        assert_eq!(true, parsed_tx.detail.contract_address.is_empty());
        assert_eq!("1002000".to_string(), parsed_tx.detail.token);
    }

    #[test]
    fn test_parse_trc20_transfer() {
        {
            let hex = "1f8b08000000000000031590bf4ac3501c46359452bba871299d4a102a42c8bffbbbf7c6499b1403b6b14d52b42e92e426b5c53636462979029d7d01477707279f40147c0007df41707130856f3870a6f355387ebd9f1a098b1abd34c99230b9acbf70158eaf1099b4db26368427ae5af29c639bdf0e98a652d50fc4500922110121a21efb548c028010142d8814bdbed995106a4a8a0e4d492e26c98defb78ffb3f79a7dcfa5ae505cf21b6359f4447fdc5a1678ce99c9e0dd1558726999b8f269d09ceea82e7b96408dab58bd23c358deccc1fdf38f97cc114ec6746a40e1c41f05cc87b89814edbada9756bda07b3d9893ab2b46eff22746c3c76a6bb2b6a49d129d9b3abfb3e8be3400335f4090d3506818c303042402f0c669851888160504286502c2b408b001d01f5fd40d6286c3c7f3ed46a773fef45486bab5a1ab8a6c7af2d6f395f62ad6c3dfee2c66bef1f257dc3fe50010000";
            let pubkey_str = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let tx = WrappedTron::from_payload(payload, &context).unwrap();
            let parsed_tx = tx.parse().unwrap();
            assert_eq!(
                "TTS7Y53sS4rzrDCtZaiuRzqxd16atCe2UR".to_string(),
                parsed_tx.overview.from
            );
            assert_eq!(
                "TTS7Y53sS4rzrDCtZaiuRzqxd16atCe2UR".to_string(),
                parsed_tx.detail.from
            );
            assert_eq!(
                "TS5zPoC4XEBmHvDNAnnW2gH3MQhcRN6iRm".to_string(),
                parsed_tx.overview.to
            );
            assert_eq!(
                "TS5zPoC4XEBmHvDNAnnW2gH3MQhcRN6iRm".to_string(),
                parsed_tx.detail.to
            );
            assert_eq!("TRON".to_string(), parsed_tx.overview.network);
            assert_eq!("TRON".to_string(), parsed_tx.detail.network);
            assert_eq!("0.001987 USDT".to_string(), parsed_tx.overview.value);
            assert_eq!("0.001987 USDT".to_string(), parsed_tx.detail.value);
            assert_eq!("TRC-20 Transfer".to_string(), parsed_tx.detail.method);
            assert_eq!("TRC-20 Transfer".to_string(), parsed_tx.overview.method);
            assert_eq!(
                "TR7NHqjeKQxGTCi8q8ZY4pL8otSzgjLj6t".to_string(),
                parsed_tx.detail.contract_address
            );
            assert_eq!(true, parsed_tx.detail.token.is_empty());
        }
        {
            let hex = "1f8b08000000000000036590bb4ec2500086158d212c4ae3409848638231697aaeeda993012f040a166c8074316d4f1b01a1da969b2383a3cfe0ec03b0bac81b38b8b8bab8b9b85a66bf7cc33f7ef9d32961b7199603ee158c30880337b8cdaf52e99490a648254405585ca632db66ebb2211c4080d6320901db9708f76dc9810c4b34c157180714a07cb62e135294a1468b32489461e1ebf169b580873ba5ef4d4134cbe7ba4ef98c5a555e1b75c65ed49df560db98f6f5f6b06359732d2f9aa7837034ad3f443066fde0a27d17e90a19d6303c0b5bb8177771248a666512c57a496db6260ebfbf31c60d7b6e065a87f6355c1f20ab3239da40fb10fce7b89139819e9ff47ac4610e5320f21dc401c19801e062c23d9ff89ae203e051c55535d725d4b63972806d3305a8d4657b9f3fcb5ceee3e5fd6d014bd9cc56f5ca14d2eb97ae9395132acfbf4ae1d5f803c61a369f5e010000";
            let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
            let payload = prepare_payload(hex);
            let context = prepare_parse_context(pubkey_str);
            let tx = WrappedTron::from_payload(payload, &context).unwrap();
            let parsed_tx = tx.parse().unwrap();
            assert_eq!(
                "TDkrnwMzs1t8joGVpsL64mK31ErR3itX3s".to_string(),
                parsed_tx.detail.from
            );
            assert_eq!(
                "THvstLB7QRvbdqhPuNayTo9W5j93Mk2ZHv".to_string(),
                parsed_tx.detail.to
            );
            assert_eq!("10 JST".to_string(), parsed_tx.detail.value);
            assert_eq!(
                "TCFLL5dx5ZJdKnWuesXxi1VPwjLVmWZZy9".to_string(),
                parsed_tx.detail.contract_address
            );
        }
    }
}
