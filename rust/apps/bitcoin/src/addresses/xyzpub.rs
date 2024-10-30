use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::str::FromStr;

use crate::errors::BitcoinError;
use bitcoin::base58;

/// Version bytes xpub: bitcoin mainnet public key P2PKH or P2SH
pub const VERSION_XPUB: [u8; 4] = [0x04, 0x88, 0xB2, 0x1E];

/// Version bytes xprv: bitcoin mainnet private key P2PKH or P2SH
pub const VERSION_XPRV: [u8; 4] = [0x04, 0x88, 0xAD, 0xE4];

/// Version bytes ypub: bitcoin mainnet public key P2WPKH in P2SH
pub const VERSION_YPUB: [u8; 4] = [0x04, 0x9D, 0x7C, 0xB2];

/// Version bytes yprv: bitcoin mainnet private key P2WPKH in P2SH
pub const VERSION_YPRV: [u8; 4] = [0x04, 0x9D, 0x78, 0x78];

/// Version bytes zpub: bitcoin mainnet public key P2WPKH
pub const VERSION_ZPUB: [u8; 4] = [0x04, 0xB2, 0x47, 0x46];

/// Version bytes zprv: bitcoin mainnet private key P2WPKH
pub const VERSION_ZPRV: [u8; 4] = [0x04, 0xB2, 0x43, 0x0C];

/// Version bytes Ypub: bitcoin mainnet public key for multi-signature P2WSH in P2SH
pub const VERSION_YPUB_MULTISIG: [u8; 4] = [0x02, 0x95, 0xb4, 0x3f];

/// Version bytes Yprv: bitcoin mainnet private key for multi-signature P2WSH in P2SH
pub const VERSION_YPRV_MULTISIG: [u8; 4] = [0x02, 0x95, 0xb0, 0x05];

/// Version bytes Zpub: bitcoin mainnet public key for multi-signature P2WSH
pub const VERSION_ZPUB_MULTISIG: [u8; 4] = [0x02, 0xaa, 0x7e, 0xd3];

/// Version bytes Zprv: bitcoin mainnet private key for multi-signature P2WSH
pub const VERSION_ZPRV_MULTISIG: [u8; 4] = [0x02, 0xaa, 0x7a, 0x99];

/// Version bytes tpub: bitcoin testnet public key for P2PKH or P2SH
pub const VERSION_TPUB: [u8; 4] = [0x04, 0x35, 0x87, 0xCF];

/// Version bytes tprv: bitcoin testnet private key for P2PKH or P2SH
pub const VERSION_TPRV: [u8; 4] = [0x04, 0x35, 0x83, 0x94];

/// Version bytes upub: bitcoin testnet public key for P2WPKH in P2SH
pub const VERSION_UPUB: [u8; 4] = [0x04, 0x4A, 0x52, 0x62];

/// Version bytes uprv: bitcoin testnet private key for P2WPKH in P2SH
pub const VERSION_UPRV: [u8; 4] = [0x04, 0x4A, 0x4E, 0x28];

/// Version bytes vpub: bitcoin testnet public key for P2WPKH
pub const VERSION_VPUB: [u8; 4] = [0x04, 0x5F, 0x1C, 0xF6];

/// Version bytes vprv: bitcoin testnet private key for P2WPKH
pub const VERSION_VPRV: [u8; 4] = [0x04, 0x5F, 0x18, 0xBC];

/// Version bytes Upub: bitcoin testnet public key for multi-signature P2WSH in P2SH
pub const VERSION_UPUB_MULTISIG: [u8; 4] = [0x02, 0x42, 0x89, 0xef];

/// Version bytes Uprv: bitcoin testnet private key for multi-signature P2WSH in P2SH
pub const VERSION_UPRV_MULTISIG: [u8; 4] = [0x02, 0x42, 0x85, 0xb5];

/// Version bytes Vpub: bitcoin testnet public key for multi-signature P2WSH
pub const VERSION_VPUB_MULTISIG: [u8; 4] = [0x02, 0x57, 0x54, 0x83];

/// Version bytes Vprv: bitcoin testnet private key for multi-signature P2WSH
pub const VERSION_VPRV_MULTISIG: [u8; 4] = [0x02, 0x57, 0x50, 0x48];

#[derive(Debug, Clone, PartialEq, Eq)]
/// Enum for version bytes.
pub enum Version {
    /// Version bytes xpub: bitcoin mainnet public key P2PKH or P2SH
    Xpub,

    /// Version bytes ypub: bitcoin mainnet public key P2WPKH in P2SH
    Ypub,

    /// Version bytes zpub: bitcoin mainnet public key P2WPKH
    Zpub,

    /// Version bytes tpub: bitcoin testnet public key for P2PKH or P2SH
    Tpub,

    /// Version bytes upub: bitcoin testnet public key for P2WPKH in P2SH
    Upub,

    /// Version bytes vpub: bitcoin testnet public key for P2WPKH
    Vpub,

    /// Version bytes xprv: bitcoin mainnet private key P2PKH or P2SH
    Xprv,

    /// Version bytes yprv: bitcoin mainnet private key P2WPKH in P2SH
    Yprv,

    /// Version bytes zpub: bitcoin mainnet public key P2WPKH
    Zprv,

    /// Version bytes tprv: bitcoin testnet private key for P2PKH or P2SH
    Tprv,

    /// Version bytes uprv: bitcoin testnet private key for P2WPKH in P2SH
    Uprv,

    /// Version bytes vprv: bitcoin testnet private key for P2WPKH
    Vprv,

    /// Version bytes Ypub: bitcoin mainnet public key for multi-signature P2WSH in P2SH
    YpubMultisig,

    /// Version bytes Zpub: bitcoin mainnet public key for multi-signature P2WSH
    ZpubMultisig,

    /// Version bytes Upub: bitcoin testnet public key for multi-signature P2WSH in P2SH
    UpubMultisig,

    /// Version bytes Vpub: bitcoin testnet public key for multi-signature P2WSH
    VpubMultisig,

    /// Version bytes Yprv: bitcoin mainnet private key for multi-signature P2WSH in P2SH
    YprvMultisig,

    /// Version bytes Zprv: bitcoin mainnet private key for multi-signature P2WSH
    ZprvMultisig,

    /// Version bytes Uprv: bitcoin testnet private key for multi-signature P2WSH in P2SH
    UprvMultisig,

    /// Version bytes Vprv: bitcoin testnet private key for multi-signature P2WSH
    VprvMultisig,
}

impl Version {
    /// Returns the version bytes.
    pub fn bytes(&self) -> [u8; 4] {
        match self {
            Version::Xpub => VERSION_XPUB,
            Version::Ypub => VERSION_YPUB,
            Version::Zpub => VERSION_ZPUB,
            Version::Tpub => VERSION_TPUB,
            Version::Upub => VERSION_UPUB,
            Version::Vpub => VERSION_VPUB,
            Version::Xprv => VERSION_XPRV,
            Version::Yprv => VERSION_YPRV,
            Version::Zprv => VERSION_ZPRV,
            Version::Tprv => VERSION_TPRV,
            Version::Uprv => VERSION_UPRV,
            Version::Vprv => VERSION_VPRV,
            Version::YpubMultisig => VERSION_YPUB_MULTISIG,
            Version::ZpubMultisig => VERSION_ZPUB_MULTISIG,
            Version::UpubMultisig => VERSION_UPUB_MULTISIG,
            Version::VpubMultisig => VERSION_VPUB_MULTISIG,
            Version::YprvMultisig => VERSION_YPRV_MULTISIG,
            Version::ZprvMultisig => VERSION_ZPRV_MULTISIG,
            Version::UprvMultisig => VERSION_UPRV_MULTISIG,
            Version::VprvMultisig => VERSION_VPRV_MULTISIG,
        }
    }
}

impl FromStr for Version {
    type Err = BitcoinError;
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "xpub" => Ok(Version::Xpub),
            "ypub" => Ok(Version::Ypub),
            "zpub" => Ok(Version::Zpub),
            "tpub" => Ok(Version::Tpub),
            "upub" => Ok(Version::Upub),
            "vpub" => Ok(Version::Vpub),
            "xprv" => Ok(Version::Xprv),
            "yprv" => Ok(Version::Yprv),
            "zprv" => Ok(Version::Zprv),
            "tprv" => Ok(Version::Tprv),
            "uprv" => Ok(Version::Uprv),
            "vprv" => Ok(Version::Vprv),
            "Ypub" => Ok(Version::YpubMultisig),
            "Zpub" => Ok(Version::ZpubMultisig),
            "Upub" => Ok(Version::UpubMultisig),
            "Vpub" => Ok(Version::VpubMultisig),
            "Yprv" => Ok(Version::YprvMultisig),
            "Zprv" => Ok(Version::ZprvMultisig),
            "Uprv" => Ok(Version::UprvMultisig),
            "Vprv" => Ok(Version::VprvMultisig),
            _ => Err(Self::Err::Base58Error(format!("unknown version prefix"))),
        }
    }
}

/// Replaces the first 4 bytes of a byte slice with the target's version and returns a new byte vec.
/// Does not check if extended public/private key is valid and only replaces the version bytes.
pub fn replace_version_bytes<B: AsRef<[u8]>>(
    bytes: B,
    target: &Version,
) -> Result<Vec<u8>, BitcoinError> {
    let mut vec = bytes.as_ref().to_vec();
    if vec.len() < 4 {
        return Err(BitcoinError::Base58Error(vec.len().to_string()));
    }
    vec[0..4].copy_from_slice(&target.bytes());

    Ok(vec)
}

/// Replaces the first 4 bytes of a base58 string with the target's version and returns the new string.
/// Also checks if the input is a correct address.
pub fn convert_version<S: AsRef<str>>(str: S, target: &Version) -> Result<String, BitcoinError> {
    let bytes = base58::decode_check(str.as_ref())?;
    let replaced = replace_version_bytes(bytes, target)?;

    Ok(base58::encode_check(&replaced))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn err_when_too_short() {
        let short = "abc";

        let result = convert_version(short, &Version::Zpub);

        assert!(result.is_err());
    }

    #[test]
    fn err_when_too_few_bytes() {
        let short = vec![0x35];

        let result = replace_version_bytes(short, &Version::Zpub);

        assert!(result.is_err());
    }

    #[test]
    fn xpub_not_valid() {
        let xpub = "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYekkudhUd9yLb6qx39T9nMdj";

        let result = convert_version(xpub, &Version::Ypub);

        assert!(result.is_err());
    }

    #[test]
    fn xpub_to_ypub() {
        let xpub = "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYJVVhhawA7d4R5WSWGFNbi8Aw6ZRc1brxMyWMzG3DSSSSoekkudhUd9yLb6qx39T9nMdj";
        let expected_ypub = "ypub6We8xsTdpgW67F3ZpmWU77JgfS29gpX5Y2u6HfSBsW5ae2yLsiae8WAQGaZJ85b1y4ipMLYvSAiY9Kq1A8rpSzSWW3B3jtA5Na1gXzZ8iqF";

        let result = convert_version(xpub, &Version::Ypub).unwrap();

        assert_eq!(result, expected_ypub);
    }

    #[test]
    fn xpub_to_zpub() {
        let xpub = "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYJVVhhawA7d4R5WSWGFNbi8Aw6ZRc1brxMyWMzG3DSSSSoekkudhUd9yLb6qx39T9nMdj";
        let expected_zpub = "zpub6qUQGY8YyN3ZxYEgf8J6KCQBqQAbdSWaT9RK54L5FWTTh8na8NkCkZpYHnWt7zEwNhqd6p9Utq562cSZsqGqFE87NNsUKnyZeJ5KvbhfC8E";

        let result = convert_version(xpub, &Version::Zpub).unwrap();

        assert_eq!(result, expected_zpub);
    }

    #[test]
    fn zpub_to_xpub() {
        let zpub = "zpub6rFR7y4Q2AijBEqTUquhVz398htDFrtymD9xYYfG1m4wAcvPhXNfE3EfH1r1ADqtfSdVCToUG868RvUUkgDKf31mGDtKsAYz2oz2AGutZYs";
        let expected_xpub = "xpub6CatWdiZiodmUeTDp8LT5or8nmbKNcuyvz7WyksVFkKB4RHwCD3XyuvPEbvqAQY3rAPshWcMLoP2fMFMKHPJ4ZeZXYVUhLv1VMrjPC7PW6V";

        let result = convert_version(zpub, &Version::Xpub).unwrap();

        assert_eq!(result, expected_xpub);
    }

    #[test]
    fn xpub_to_tpub() {
        let xpub = "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYJVVhhawA7d4R5WSWGFNbi8Aw6ZRc1brxMyWMzG3DSSSSoekkudhUd9yLb6qx39T9nMdj";
        let expected_tpub = "tpubDCBWBScQPGv4Xk3JSbhw6wYYpayMjb2eAYyArpbSqQTbLDpphHGAetB6VQgVeftLML8vDSUEWcC2xDi3qJJ3YCDChJDvqVzpgoYSuT52MhJ";

        let result = convert_version(xpub, &Version::Tpub).unwrap();

        assert_eq!(result, expected_tpub);
    }
}
