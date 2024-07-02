use crate::errors::{ArweaveError, Result};
use alloc::string::ToString;
use alloc::vec;
use alloc::{string::String, vec::Vec};
use app_utils::impl_internal_struct;
use lazy_static::lazy_static;

impl_internal_struct!(TokenInfo {
    token_id: String,
    decimal: u8,
    symbol: String,
    name: String
});

impl TokenInfo {
    pub fn convert_quantity(&self, quantity: &str) -> Result<String> {
        let value = u64::from_str_radix(quantity, 10)
            .map_err(|_e| ArweaveError::ParseTxError(_e.to_string()))?;
        let divisor = 10u64.pow(self.get_decimal() as u32) as f64;
        return Ok(format!(
            "{} {}",
            (value as f64) / divisor,
            self.get_symbol()
        ));
    }
}

lazy_static! {
    static ref TOKENS: Vec<TokenInfo> = vec![
        TokenInfo::new(
            "xU9zFkq3X2ZQ6olwNVvr1vUWIjc3kXTWr7xKQD6dh10".to_string(),
            12,
            "AR".to_string(),
            "Wrapped AR".to_string(),
        ),
        TokenInfo::new(
            "8p7ApPZxC_37M06QHVejCQrKsHbcJEerd3jWNkDUWPQ".to_string(),
            3,
            "BRKTST".to_string(),
            "Bark".to_string(),
        ),
        TokenInfo::new(
            "097nIBc4YMMkNWNuNNOShi1Zt5VkNV5--lzj1hYR0vg".to_string(),
            12,
            "PNTS".to_string(),
            "Points".to_string(),
        ),
        TokenInfo::new(
            "Sa0iBLPNyJQrwpTTG-tWLQU-1QeUAJA73DdxGGiKoJc".to_string(),
            3,
            "testnet-AOCRED".to_string(),
            "AOCRED".to_string(),
        ),
        TokenInfo::new(
            "OT9qTE2467gcozb2g8R6D6N3nQS94ENcaAIJfUzHCww".to_string(),
            3,
            "TRUNK".to_string(),
            "TRUNK".to_string(),
        )
    ];
}

pub(crate) fn find_token(token_id: &str) -> Option<TokenInfo> {
    TOKENS
        .iter()
        .find(|v| v.get_token_id().eq(token_id))
        .map(|v| v.clone())
}
