use alloc::{string::String, vec::Vec};
use app_utils::impl_public_struct;

impl_public_struct!(ParsedPczt {
    transparent: Option<ParsedTransparent>,
    orchard: Option<ParsedOrchard>,
    total_transfer_value: String,
    fee_value: String,
    has_sapling: bool
});

impl_public_struct!(ParsedTransparent {
    from: Vec<ParsedFrom>,
    to: Vec<ParsedTo>
});

impl ParsedTransparent {
    pub fn add_from(&mut self, from: ParsedFrom) {
        self.from.push(from);
    }

    pub fn add_to(&mut self, to: ParsedTo) {
        self.to.push(to);
    }
}

impl_public_struct!(ParsedFrom {
    address: Option<String>,
    value: String,
    amount: u64,
    is_mine: bool
});

impl_public_struct!(ParsedTo {
    address: String,
    value: String,
    amount: u64,
    is_change: bool,
    is_dummy: bool,
    memo: Option<String>
});

impl_public_struct!(ParsedOrchard {
    from: Vec<ParsedFrom>,
    to: Vec<ParsedTo>
});

impl ParsedOrchard {
    pub fn add_from(&mut self, from: ParsedFrom) {
        self.from.push(from);
    }

    pub fn add_to(&mut self, to: ParsedTo) {
        self.to.push(to);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::{string::ToString, vec};
    extern crate std;

    #[test]
    fn test_parsed_pczt_creation() {
        let pczt = ParsedPczt::new(
            None,
            None,
            "1.0 ZEC".to_string(),
            "0.0001 ZEC".to_string(),
            false,
        );
        assert!(pczt.get_transparent().is_none());
        assert!(pczt.get_orchard().is_none());
        assert_eq!(pczt.get_total_transfer_value(), "1.0 ZEC");
        assert_eq!(pczt.get_fee_value(), "0.0001 ZEC");
        assert!(!pczt.get_has_sapling());
    }

    #[test]
    fn test_parsed_transparent_add_from_and_to() {
        let mut transparent = ParsedTransparent::new(vec![], vec![]);
        assert_eq!(transparent.get_from().len(), 0);
        assert_eq!(transparent.get_to().len(), 0);

        let from = ParsedFrom::new(
            Some("addr1".to_string()),
            "1.0 ZEC".to_string(),
            100000000,
            true,
        );
        transparent.add_from(from);
        assert_eq!(transparent.get_from().len(), 1);

        let to = ParsedTo::new(
            "addr2".to_string(),
            "0.5 ZEC".to_string(),
            50000000,
            false,
            false,
            None,
        );
        transparent.add_to(to);
        assert_eq!(transparent.get_to().len(), 1);
    }

    #[test]
    fn test_parsed_from_fields() {
        let from = ParsedFrom::new(
            Some("test_address".to_string()),
            "2.5 ZEC".to_string(),
            250000000,
            true,
        );
        assert!(matches!(from.get_address(), Some(addr) if addr == "test_address"));
        assert_eq!(from.get_value(), "2.5 ZEC");
        assert_eq!(from.get_amount(), 250000000);
        assert!(from.get_is_mine());
    }

    #[test]
    fn test_parsed_from_without_address() {
        let from = ParsedFrom::new(None, "1.0 ZEC".to_string(), 100000000, false);
        assert!(from.get_address().is_none());
        assert!(!from.get_is_mine());
    }

    #[test]
    fn test_parsed_to_fields() {
        let to = ParsedTo::new(
            "recipient_address".to_string(),
            "3.0 ZEC".to_string(),
            300000000,
            true,
            false,
            Some("Test memo".to_string()),
        );
        assert_eq!(to.get_address(), "recipient_address");
        assert_eq!(to.get_value(), "3.0 ZEC");
        assert_eq!(to.get_amount(), 300000000);
        assert!(to.get_is_change());
        assert!(!to.get_is_dummy());
        assert!(matches!(to.get_memo(), Some(memo) if memo == "Test memo"));
    }

    #[test]
    fn test_parsed_to_dummy() {
        let to = ParsedTo::new(
            "dummy".to_string(),
            "0 ZEC".to_string(),
            0,
            false,
            true,
            None,
        );
        assert!(to.get_is_dummy());
        assert!(!to.get_is_change());
        assert_eq!(to.get_amount(), 0);
    }

    #[test]
    fn test_parsed_orchard_add_from_and_to() {
        let mut orchard = ParsedOrchard::new(vec![], vec![]);
        assert_eq!(orchard.get_from().len(), 0);
        assert_eq!(orchard.get_to().len(), 0);

        let from = ParsedFrom::new(None, "1.5 ZEC".to_string(), 150000000, true);
        orchard.add_from(from);
        assert_eq!(orchard.get_from().len(), 1);
        assert!(orchard.get_from().first().unwrap().get_is_mine());

        let to = ParsedTo::new(
            "<internal-address>".to_string(),
            "1.4 ZEC".to_string(),
            140000000,
            true,
            false,
            None,
        );
        orchard.add_to(to);
        assert_eq!(orchard.get_to().len(), 1);
        assert!(orchard.get_to().first().unwrap().get_is_change());
    }

    #[test]
    fn test_parsed_pczt_with_all_pools() {
        let transparent = ParsedTransparent::new(
            vec![ParsedFrom::new(
                Some("t_addr".to_string()),
                "1.0 ZEC".to_string(),
                100000000,
                true,
            )],
            vec![ParsedTo::new(
                "t_out".to_string(),
                "0.5 ZEC".to_string(),
                50000000,
                false,
                false,
                None,
            )],
        );
        let orchard = ParsedOrchard::new(
            vec![ParsedFrom::new(None, "0.5 ZEC".to_string(), 50000000, true)],
            vec![ParsedTo::new(
                "<internal>".to_string(),
                "0.4 ZEC".to_string(),
                40000000,
                true,
                false,
                None,
            )],
        );
        let pczt = ParsedPczt::new(
            Some(transparent),
            Some(orchard),
            "0.5 ZEC".to_string(),
            "0.1 ZEC".to_string(),
            false,
        );
        assert!(pczt.get_transparent().is_some());
        assert!(pczt.get_orchard().is_some());
        assert_eq!(pczt.get_total_transfer_value(), "0.5 ZEC");
        assert_eq!(pczt.get_fee_value(), "0.1 ZEC");
    }

    #[test]
    fn test_parsed_pczt_with_sapling() {
        let pczt = ParsedPczt::new(
            None,
            None,
            "5.0 ZEC".to_string(),
            "0.001 ZEC".to_string(),
            true,
        );
        assert!(pczt.get_has_sapling());
        assert!(pczt.get_transparent().is_none());
        assert!(pczt.get_orchard().is_none());
    }

    #[test]
    fn test_parsed_transparent_multiple_from() {
        let mut transparent = ParsedTransparent::new(vec![], vec![]);
        
        for i in 0..5 {
            let from = ParsedFrom::new(
                Some(alloc::format!("addr_{i}")),
                alloc::format!("{i}.0 ZEC"),
                (i as u64) * 100000000,
                i % 2 == 0,
            );
            transparent.add_from(from);
        }
        
        assert_eq!(transparent.get_from().len(), 5);
        assert_eq!(transparent.get_from()[0].get_amount(), 0);
        assert_eq!(transparent.get_from()[4].get_amount(), 400000000);
        assert!(transparent.get_from()[0].get_is_mine());
        assert!(!transparent.get_from()[1].get_is_mine());
    }

    #[test]
    fn test_parsed_transparent_multiple_to() {
        let mut transparent = ParsedTransparent::new(vec![], vec![]);
        
        for i in 0..3 {
            let to = ParsedTo::new(
                alloc::format!("recipient_{i}"),
                alloc::format!("{i}.5 ZEC"),
                (i as u64) * 50000000,
                i == 2,
                false,
                if i == 1 { Some("Memo for recipient 1".to_string()) } else { None },
            );
            transparent.add_to(to);
        }
        
        assert_eq!(transparent.get_to().len(), 3);
        assert!(!transparent.get_to()[0].get_is_change());
        assert!(transparent.get_to()[2].get_is_change());
        assert!(transparent.get_to()[1].get_memo().is_some());
        assert!(transparent.get_to()[0].get_memo().is_none());
    }

    #[test]
    fn test_parsed_orchard_multiple_from() {
        let mut orchard = ParsedOrchard::new(vec![], vec![]);
        
        for i in 0..4 {
            let from = ParsedFrom::new(
                None,
                alloc::format!("{}.{} ZEC", i, i * 10),
                (i as u64) * 100000000 + (i as u64) * 10000000,
                true,
            );
            orchard.add_from(from);
        }
        
        assert_eq!(orchard.get_from().len(), 4);
        assert!(orchard.get_from().iter().all(|f| f.get_address().is_none()));
        assert!(orchard.get_from().iter().all(|f| f.get_is_mine()));
    }

    #[test]
    fn test_parsed_orchard_multiple_to() {
        let mut orchard = ParsedOrchard::new(vec![], vec![]);
        
        for i in 0..6 {
            let to = ParsedTo::new(
                if i % 2 == 0 { "<internal>".to_string() } else { "<external>".to_string() },
                alloc::format!("{}.{} ZEC", i / 2, i * 5),
                (i as u64) * 25000000,
                i % 2 == 0,
                false,
                Some(alloc::format!("Memo {i}")),
            );
            orchard.add_to(to);
        }
        
        assert_eq!(orchard.get_to().len(), 6);
        assert_eq!(orchard.get_to().iter().filter(|t| t.get_is_change()).count(), 3);
        assert!(orchard.get_to().iter().all(|t| t.get_memo().is_some()));
    }

    #[test]
    fn test_parsed_from_zero_amount() {
        let from = ParsedFrom::new(
            Some("zero_addr".to_string()),
            "0 ZEC".to_string(),
            0,
            false,
        );
        assert_eq!(from.get_amount(), 0);
        assert_eq!(from.get_value(), "0 ZEC");
    }

    #[test]
    fn test_parsed_to_zero_amount() {
        let to = ParsedTo::new(
            "zero_recipient".to_string(),
            "0.0 ZEC".to_string(),
            0,
            false,
            false,
            Some("Zero amount".to_string()),
        );
        assert_eq!(to.get_amount(), 0);
        assert_eq!(to.get_value(), "0.0 ZEC");
    }

    #[test]
    fn test_parsed_to_large_amount() {
        let to = ParsedTo::new(
            "whale_address".to_string(),
            "21000000.0 ZEC".to_string(),
            21000000_00000000u64,
            false,
            false,
            None,
        );
        assert_eq!(to.get_amount(), 21000000_00000000u64);
        assert_eq!(to.get_value(), "21000000.0 ZEC");
    }

    #[test]
    fn test_parsed_to_with_long_memo() {
        let long_memo = "A".repeat(512);
        let to = ParsedTo::new(
            "addr_with_memo".to_string(),
            "1.0 ZEC".to_string(),
            100000000,
            false,
            false,
            Some(long_memo.clone()),
        );
        assert_eq!(to.get_memo().as_ref().map(|s| s.len()), Some(512));
    }

    #[test]
    fn test_parsed_to_with_empty_memo() {
        let to = ParsedTo::new(
            "addr".to_string(),
            "1.0 ZEC".to_string(),
            100000000,
            false,
            false,
            Some("".to_string()),
        );
        assert!(to.get_memo().is_some());
        assert_eq!(to.get_memo().unwrap(), "");
    }

    #[test]
    fn test_parsed_pczt_empty_values() {
        let pczt = ParsedPczt::new(
            None,
            None,
            "".to_string(),
            "".to_string(),
            false,
        );
        assert_eq!(pczt.get_total_transfer_value(), "");
        assert_eq!(pczt.get_fee_value(), "");
    }

    #[test]
    fn test_parsed_from_with_special_characters() {
        let from = ParsedFrom::new(
            Some("t1_address_123!@#".to_string()),
            "1.23456789 ZEC".to_string(),
            123456789,
            true,
        );
        assert_eq!(from.get_address().unwrap(), "t1_address_123!@#");
    }

    #[test]
    fn test_parsed_to_all_flags_true() {
        let to = ParsedTo::new(
            "address".to_string(),
            "1.0 ZEC".to_string(),
            100000000,
            true,
            true,
            Some("memo".to_string()),
        );
        assert!(to.get_is_change());
        assert!(to.get_is_dummy());
    }

    #[test]
    fn test_parsed_to_all_flags_false() {
        let to = ParsedTo::new(
            "address".to_string(),
            "1.0 ZEC".to_string(),
            100000000,
            false,
            false,
            None,
        );
        assert!(!to.get_is_change());
        assert!(!to.get_is_dummy());
    }

    #[test]
    fn test_complex_transaction() {
        let mut transparent = ParsedTransparent::new(vec![], vec![]);
        
        transparent.add_from(ParsedFrom::new(
            Some("t1abc123".to_string()),
            "5.0 ZEC".to_string(),
            500000000,
            true,
        ));
        transparent.add_from(ParsedFrom::new(
            Some("t1def456".to_string()),
            "3.5 ZEC".to_string(),
            350000000,
            true,
        ));
        
        transparent.add_to(ParsedTo::new(
            "t1output1".to_string(),
            "4.0 ZEC".to_string(),
            400000000,
            false,
            false,
            Some("Payment 1".to_string()),
        ));
        transparent.add_to(ParsedTo::new(
            "t1output2".to_string(),
            "2.0 ZEC".to_string(),
            200000000,
            false,
            false,
            Some("Payment 2".to_string()),
        ));
        transparent.add_to(ParsedTo::new(
            "t1change".to_string(),
            "2.4 ZEC".to_string(),
            240000000,
            true,
            false,
            None,
        ));
        
        let mut orchard = ParsedOrchard::new(vec![], vec![]);
        orchard.add_from(ParsedFrom::new(
            None,
            "10.0 ZEC".to_string(),
            1000000000,
            true,
        ));
        orchard.add_to(ParsedTo::new(
            "<internal>".to_string(),
            "9.9 ZEC".to_string(),
            990000000,
            true,
            false,
            Some("Orchard change".to_string()),
        ));
        
        let pczt = ParsedPczt::new(
            Some(transparent),
            Some(orchard),
            "6.0 ZEC".to_string(),
            "0.1 ZEC".to_string(),
            true,
        );
        
        assert!(pczt.get_transparent().is_some());
        assert!(pczt.get_orchard().is_some());
        assert!(pczt.get_has_sapling());
        
        let t = pczt.get_transparent().unwrap();
        assert_eq!(t.get_from().len(), 2);
        assert_eq!(t.get_to().len(), 3);
        assert_eq!(t.get_to().iter().filter(|to| to.get_is_change()).count(), 1);
        
        let o = pczt.get_orchard().unwrap();
        assert_eq!(o.get_from().len(), 1);
        assert_eq!(o.get_to().len(), 1);
    }

    #[test]
    fn test_parsed_pczt_only_transparent() {
        let transparent = ParsedTransparent::new(
            vec![ParsedFrom::new(
                Some("t_only".to_string()),
                "1.0 ZEC".to_string(),
                100000000,
                true,
            )],
            vec![],
        );
        let pczt = ParsedPczt::new(
            Some(transparent),
            None,
            "1.0 ZEC".to_string(),
            "0.0001 ZEC".to_string(),
            false,
        );
        assert!(pczt.get_transparent().is_some());
        assert!(pczt.get_orchard().is_none());
        assert!(!pczt.get_has_sapling());
    }

    #[test]
    fn test_parsed_pczt_only_orchard() {
        let orchard = ParsedOrchard::new(
            vec![ParsedFrom::new(None, "2.0 ZEC".to_string(), 200000000, true)],
            vec![],
        );
        let pczt = ParsedPczt::new(
            None,
            Some(orchard),
            "2.0 ZEC".to_string(),
            "0.0001 ZEC".to_string(),
            false,
        );
        assert!(pczt.get_transparent().is_none());
        assert!(pczt.get_orchard().is_some());
    }

    #[test]
    fn test_value_precision() {
        let from = ParsedFrom::new(
            Some("precise".to_string()),
            "0.00000001 ZEC".to_string(),
            1,
            true,
        );
        assert_eq!(from.get_amount(), 1);
        assert_eq!(from.get_value(), "0.00000001 ZEC");
    }

    #[test]
    fn test_memo_with_special_characters() {
        let special_memo = "Hello ! 🚀 #$%^&*()";
        let to = ParsedTo::new(
            "address".to_string(),
            "1.0 ZEC".to_string(),
            100000000,
            false,
            false,
            Some(special_memo.to_string()),
        );
        assert_eq!(to.get_memo().unwrap(), special_memo);
    }

    
}
