pub mod btc_only_wallet;
pub use btc_only_wallet::*;
#[cfg(feature = "multi-coins")]
pub mod multi_coins_wallet;