use crate::errors::BitcoinError;
use alloc::string::{String, ToString};
use core::str::FromStr;
use ur_registry::pb::protoc::sign_transaction::Transaction;
use ur_registry::pb::protoc::SignTransaction;

pub trait NetworkT {
    fn get_unit(&self) -> String;
    fn normalize(&self) -> String;
    fn large_fee_policy(&self) -> LargeFeePolicy;
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct LargeFeePolicy {
    /// Absolute fee threshold in the network's smallest unit.
    pub absolute_threshold: u64,
    /// Fee-rate threshold in the network's smallest unit per virtual byte.
    /// Some networks use a fee policy that is not meaningfully byte based.
    pub rate_threshold_per_vbyte: Option<u64>,
    /// Whether a fee larger than the transferred amount should be highlighted.
    /// This is not useful for networks such as Dogecoin where small transfers
    /// can legitimately cost more than the transferred amount.
    pub warn_fee_larger_than_amount: bool,
}

pub const UNSUPPORTED_LEGACY_UTXO_MESSAGE: &str =
    "unsupported legacy UTXO transaction; only BCH, DASH and LTC are allowed";

pub fn is_legacy_utxo_transaction(sign_tx: &SignTransaction) -> bool {
    matches!(
        sign_tx.transaction.as_ref(),
        Some(
            Transaction::BtcTx(_)
                | Transaction::BchTx(_)
                | Transaction::DashTx(_)
                | Transaction::LtcTx(_)
                | Transaction::DogeTx(_)
        )
    )
}

/// The deprecated raw-protobuf path is retained only for these transaction
/// variants. `coin_code` is deliberately not used as a discriminator.
pub fn is_supported_legacy_utxo_transaction(sign_tx: &SignTransaction) -> bool {
    matches!(
        sign_tx.transaction.as_ref(),
        Some(Transaction::BchTx(_) | Transaction::DashTx(_) | Transaction::LtcTx(_))
    )
}

#[derive(Debug, Clone)]
pub enum Network {
    Bitcoin,
    BitcoinTestnet,
    Litecoin,
    Dogecoin,
    Dash,
    BitcoinCash,
    AvaxBtcBridge,
    Zcash,
}

impl NetworkT for Network {
    fn get_unit(&self) -> String {
        match self {
            Network::Bitcoin | Network::AvaxBtcBridge => "BTC",
            Network::BitcoinTestnet => "tBTC",
            Network::Litecoin => "LTC",
            Network::Dogecoin => "DOGE",
            Network::Dash => "DASH",
            Network::BitcoinCash => "BCH",
            Network::Zcash => "ZEC",
        }
        .to_string()
    }

    fn normalize(&self) -> String {
        match self {
            Network::Bitcoin => "Bitcoin Mainnet",
            Network::BitcoinTestnet => "Bitcoin Testnet",
            Network::Litecoin => "Litecoin",
            Network::Dogecoin => "Dogecoin",
            Network::Dash => "Dash",
            Network::BitcoinCash => "Bitcoin Cash",
            Network::AvaxBtcBridge => "Avalanche BTC",
            Network::Zcash => "Zcash",
        }
        .to_string()
    }

    fn large_fee_policy(&self) -> LargeFeePolicy {
        match self {
            // BTC-like fee market: 0.05 BTC or 100 sat/vB.
            Network::Bitcoin | Network::BitcoinTestnet | Network::AvaxBtcBridge => LargeFeePolicy {
                absolute_threshold: 5_000_000,
                rate_threshold_per_vbyte: Some(100),
                warn_fee_larger_than_amount: true,
            },
            // Litecoin's normal relay/wallet fee scale is higher in litoshi/vB.
            Network::Litecoin => LargeFeePolicy {
                absolute_threshold: 10_000_000,
                rate_threshold_per_vbyte: Some(1_000),
                warn_fee_larger_than_amount: true,
            },
            // Dogecoin Core recommends a minimum of 0.01 DOGE/kB. Wallet and
            // swap transactions can legitimately pay substantially more, so
            // warn at 1 DOGE/kB or an absolute fee above 1 DOGE.
            Network::Dogecoin => LargeFeePolicy {
                absolute_threshold: 100_000_000,
                rate_threshold_per_vbyte: Some(100_000),
                warn_fee_larger_than_amount: false,
            },
            Network::Dash => LargeFeePolicy {
                absolute_threshold: 10_000_000,
                rate_threshold_per_vbyte: Some(100),
                warn_fee_larger_than_amount: true,
            },
            Network::BitcoinCash => LargeFeePolicy {
                absolute_threshold: 10_000_000,
                rate_threshold_per_vbyte: Some(100),
                warn_fee_larger_than_amount: true,
            },
            // Zcash conventional fees are action based rather than a simple
            // sat/vB-style market, so only use the absolute safety threshold.
            Network::Zcash => LargeFeePolicy {
                absolute_threshold: 10_000_000,
                rate_threshold_per_vbyte: None,
                warn_fee_larger_than_amount: true,
            },
        }
    }
}

impl Network {
    pub fn bip44_coin_type(&self) -> String {
        match self {
            Network::Bitcoin => 0,
            Network::BitcoinTestnet => 1,
            Network::Litecoin => 2,
            Network::Dogecoin => 3,
            Network::Dash => 5,
            Network::BitcoinCash => 145,
            Network::AvaxBtcBridge => 60,
            Network::Zcash => 133,
        }
        .to_string()
    }
}

impl FromStr for Network {
    type Err = BitcoinError;
    fn from_str(network: &str) -> Result<Self, BitcoinError> {
        match network {
            "BTC" => Ok(Self::Bitcoin),
            "tBTC" => Ok(Self::BitcoinTestnet),
            "LTC" => Ok(Self::Litecoin),
            "DOGE" => Ok(Self::Dogecoin),
            "DASH" => Ok(Self::Dash),
            "BCH" => Ok(Self::BitcoinCash),
            "BTC_NATIVE_SEGWIT" => Ok(Self::Bitcoin),
            "BTC_SEGWIT" => Ok(Self::Bitcoin),
            "BTC_LEGACY" => Ok(Self::Bitcoin),
            "AVAX" => Ok(Self::AvaxBtcBridge),
            _ => Err(BitcoinError::UnsupportedNetwork(format!("{network:?}"))),
        }
    }
}

#[derive(Debug, Clone, PartialEq)]
pub enum CustomNewNetwork {
    FractalBitcoin,
    FractalBitcoinTest,
}

impl NetworkT for CustomNewNetwork {
    fn get_unit(&self) -> String {
        match self {
            CustomNewNetwork::FractalBitcoin => "FB",
            CustomNewNetwork::FractalBitcoinTest => "tFB",
        }
        .to_string()
    }

    fn normalize(&self) -> String {
        match self {
            CustomNewNetwork::FractalBitcoin => "Fractal Bitcoin",
            CustomNewNetwork::FractalBitcoinTest => "Fractal Bitcoin Testnet",
        }
        .to_string()
    }

    fn large_fee_policy(&self) -> LargeFeePolicy {
        LargeFeePolicy {
            absolute_threshold: 5_000_000,
            rate_threshold_per_vbyte: Some(100),
            warn_fee_larger_than_amount: true,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{is_legacy_utxo_transaction, is_supported_legacy_utxo_transaction};
    use alloc::string::ToString;
    use ur_registry::pb::protoc::sign_transaction::Transaction;
    use ur_registry::pb::protoc::{BchTx, BtcTx, DashTx, DogeTx, LtcTx, SignTransaction};

    fn sign_tx(coin_code: &str, transaction: Transaction) -> SignTransaction {
        SignTransaction {
            coin_code: coin_code.to_string(),
            transaction: Some(transaction),
            ..Default::default()
        }
    }

    #[test]
    fn bch_dash_and_ltc_variants_are_supported_without_using_coin_code() {
        let bch = sign_tx("ignored", Transaction::BchTx(BchTx::default()));
        let dash = sign_tx("BTC", Transaction::DashTx(DashTx::default()));
        let ltc = sign_tx("DOGE", Transaction::LtcTx(LtcTx::default()));
        assert!(is_supported_legacy_utxo_transaction(&bch));
        assert!(is_supported_legacy_utxo_transaction(&dash));
        assert!(is_supported_legacy_utxo_transaction(&ltc));
    }

    #[test]
    fn bitcoin_and_dogecoin_variants_are_rejected_regardless_of_coin_code() {
        let btc = sign_tx("LTC", Transaction::BtcTx(BtcTx::default()));
        let doge = sign_tx("BCH", Transaction::DogeTx(DogeTx::default()));

        assert!(is_legacy_utxo_transaction(&btc));
        assert!(is_legacy_utxo_transaction(&doge));
        assert!(!is_supported_legacy_utxo_transaction(&btc));
        assert!(!is_supported_legacy_utxo_transaction(&doge));
    }
}
