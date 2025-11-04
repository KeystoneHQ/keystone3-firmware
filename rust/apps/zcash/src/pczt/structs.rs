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
}
