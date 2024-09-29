// tokenAmount to human readable
pub fn token_amount_to_human_readable(token_amount: u64, decimals: u32) -> f64 {
    token_amount as f64 / 10u64.pow(decimals) as f64
}

#[cfg(test)]
mod tests {
    use super::*;
    // https://solscan.io/tx/63W8ceGvHycrseQaedkacoVnassLq4VpRdjx23fVYZnqRzwBQeTgjZL5GdNVf9B5gkJMgkF66W643T5BduWMWpis
    #[test]
    fn test_token_amount_to_human_readable() {
        assert_eq!(token_amount_to_human_readable(574810, 6), 0.57481);
    }

    #[test]
    fn test_token_amount_to_human_readable_2() {
        assert_eq!(token_amount_to_human_readable(10000, 8), 0.0001);
    }

    #[test]
    fn test_token_amount_to_human_readable_3() {
        assert_eq!(token_amount_to_human_readable(10000, 0), 10000.0);
    }
}
