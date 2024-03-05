extern crate alloc;

pub(crate) mod address;
pub mod cashaddr;
mod constants;
mod encoding;
pub mod xyzpub;

use alloc::string::{String, ToString};

use crate::addresses::address::Address;
use crate::addresses::xyzpub::{convert_version, Version};
use crate::network::Network;
use crate::{derivation_account_path, derivation_address_path};
use alloc::vec::Vec;
use keystore::algorithms::secp256k1;
use third_party::bitcoin::PublicKey;

use crate::errors::{BitcoinError, Result};

pub fn derive_public_key(xpub: &String, path: String) -> Result<PublicKey> {
    let converted_xpub = convert_version(xpub, &Version::Xpub)
        .map_err(|_| BitcoinError::AddressError(String::from("xpub is not valid")))?;
    let secp256k1_pubkey = secp256k1::derive_public_key(&converted_xpub, &path).map_err(|_| {
        BitcoinError::AddressError(format!("failed to derive public key {:?}", xpub))
    })?;
    PublicKey::from_slice(secp256k1_pubkey.serialize().as_slice())
        .map_err(|e| BitcoinError::GetKeyError(e.to_string()))
}

pub fn get_address(hd_path: String, extended_pub_key: &String) -> Result<String> {
    let account_hd_path = derivation_account_path!(hd_path)?;
    let address_hd_path = derivation_address_path!(hd_path)?;
    let compressed_ecdsa_pubkey = derive_public_key(extended_pub_key, address_hd_path)?;
    let address = match account_hd_path.as_str() {
        "m/44'/0'/0'" => Address::p2pkh(&compressed_ecdsa_pubkey, Network::Bitcoin),
        "m/44'/1'/0'" => Address::p2pkh(&compressed_ecdsa_pubkey, Network::BitcoinTestnet),
        "m/49'/0'/0'" => Address::p2shp2wpkh(&compressed_ecdsa_pubkey, Network::Bitcoin),
        "m/49'/1'/0'" => Address::p2shp2wpkh(&compressed_ecdsa_pubkey, Network::BitcoinTestnet),
        "m/84'/0'/0'" => Address::p2wpkh(&compressed_ecdsa_pubkey, Network::Bitcoin),
        "m/84'/1'/0'" => Address::p2wpkh(&compressed_ecdsa_pubkey, Network::BitcoinTestnet),
        "m/49'/2'/0'" => Address::p2shp2wpkh(&compressed_ecdsa_pubkey, Network::Litecoin),
        "m/44'/3'/0'" => Address::p2pkh(&compressed_ecdsa_pubkey, Network::Dogecoin),
        "m/44'/5'/0'" => Address::p2pkh(&compressed_ecdsa_pubkey, Network::Dash),
        "m/44'/145'/0'" => Address::p2pkh(&compressed_ecdsa_pubkey, Network::BitcoinCash),
        "m/86'/1'/0'" => Address::p2tr_no_script(&compressed_ecdsa_pubkey, Network::BitcoinTestnet),
        "m/86'/0'/0'" => Address::p2tr_no_script(&compressed_ecdsa_pubkey, Network::Bitcoin),
        _ => Err(BitcoinError::AddressError(
            "network is not supported".to_string(),
        )),
    }?;
    Ok(address.to_string())
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::String;
    use alloc::string::ToString;

    #[test]
    fn test_btc_p2pkh_address() {
        let extended_pubkey = "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYJVVhhawA7d4R5WSWGFNbi8Aw6ZRc1brxMyWMzG3DSSSSoekkudhUd9yLb6qx39T9nMdj";
        let address = get_address(
            String::from("M/44'/0'/0'/0/0"),
            &extended_pubkey.to_string(),
        )
        .unwrap();
        assert_eq!(address, "1LqBGSKuX5yYUonjxT5qGfpUsXKYYWeabA".to_string())
    }

    #[test]
    fn test_btc_p2pkh_address_testnet() {
        let extended_pubkey = "tpubDDik32ahiYevQ3fTJJDhK3jacpCiHccBSDxbxrmcxUyEt1seiWE6B5REAzxs3upUa5AtG3BSAeY88kxKDwLGsK7e9a98EHkGA76s6d8oSAf";
        let address = get_address(
            String::from("M/44'/1'/0'/0/0"),
            &extended_pubkey.to_string(),
        )
        .unwrap();
        assert_eq!(address, "mszm85TQkAhvAigVfraWicXNnCypp1TTbH".to_string())
    }

    #[test]
    fn test_btc_p2sh_p2wpkh_address() {
        let extended_pubkey = "ypub6Ww3ibxVfGzLrAH1PNcjyAWenMTbbAosGNB6VvmSEgytSER9azLDWCxoJwW7Ke7icmizBMXrzBx9979FfaHxHcrArf3zbeJJJUZPf663zsP";
        let address =
            get_address("M/49'/0'/0'/0/0".to_string(), &extended_pubkey.to_string()).unwrap();
        assert_eq!(address, "37VucYSaXLCAsxYyAPfbSi9eh4iEcbShgf")
    }

    #[test]
    fn test_btc_p2sh_p2wpkh_address_testnet() {
        let extended_pubkey = "tpubDCkgEBaL4cWekguRDEBqqtCbzx5GB4p7qEZMtNauvSg9dREFU4e2azLXL1XwjY19iw3EBFZJN4zzuJ7Wwz9fkyjjKfRkGAGJrFzGnsTHUK3";
        let address =
            get_address("M/49'/1'/0'/0/0".to_string(), &extended_pubkey.to_string()).unwrap();
        assert_eq!(address, "2NCuF1UQSRXn4WTCKQRGBdUhuFtTg1VpjtK")
    }

    #[test]
    fn test_btc_p2wpkh_address() {
        let extended_pubkey = "zpub6rFR7y4Q2AijBEqTUquhVz398htDFrtymD9xYYfG1m4wAcvPhXNfE3EfH1r1ADqtfSdVCToUG868RvUUkgDKf31mGDtKsAYz2oz2AGutZYs";
        let address = get_address(
            String::from("M/84'/0'/0'/0/0"),
            &extended_pubkey.to_string(),
        )
        .unwrap();
        assert_eq!(address, "bc1qcr8te4kr609gcawutmrza0j4xv80jy8z306fyu")
    }

    #[test]
    fn test_btc_p2wpkh_address_testnet() {
        let extended_pubkey = "tpubDC3UtAdzUBgmKQdkWxm84kU3dWWp3Da9igpySqHoj8Vt9paj7YncFS7hHvobpfEW3UDkWyuRGXYhFqVgHi8fu2jxSy2LLggiYGKgFyri2wz";
        let address = get_address(
            String::from("M/84'/1'/0'/0/0"),
            &extended_pubkey.to_string(),
        )
        .unwrap();
        assert_eq!(address, "tb1q6sjunnh9w9epn9z7he2dxmklgfg7x38yefmld7")
    }

    #[test]
    fn test_bch_p2pkh_address() {
        let extended_pubkey = "xpub6ByHsPNSQXTWZ7PLESMY2FufyYWtLXagSUpMQq7Un96SiThZH2iJB1X7pwviH1WtKVeDP6K8d6xxFzzoaFzF3s8BKCZx8oEDdDkNnp4owAZ";
        let address = get_address(
            String::from("m/44'/145'/0'/0/0"),
            &extended_pubkey.to_string(),
        )
        .unwrap();
        assert_eq!(address, "qqyx49mu0kkn9ftfj6hje6g2wfer34yfnq5tahq3q6")
    }

    #[test]
    fn test_dash_p2pkh_address() {
        let extended_pubkey = "xpub6CYEjsU6zPM3sADS2ubu2aZeGxCm3C5KabkCpo4rkNbXGAH9M7rRUJ4E5CKiyUddmRzrSCopPzisTBrXkfCD4o577XKM9mzyZtP1Xdbizyk";
        let address = get_address(
            String::from("m/44'/5'/0'/0/0"),
            &extended_pubkey.to_string(),
        )
        .unwrap();
        assert_eq!(address, "XoJA8qE3N2Y3jMLEtZ3vcN42qseZ8LvFf5")
    }

    #[test]
    fn test_ltc_p2shwpkh_address() {
        let extended_pubkey = "ypub6WZ2nNciqS7sCCFCH64AswfvBu4pLXTdDQcvTkrSFyEbashNb6vEJwXTCB7axKdR4TSbNYTqnU7S6sYPs9afBYqTytiTdjzmcDVRuYcrtso";
        let address = get_address(
            String::from("m/49'/2'/0'/0/0"),
            &extended_pubkey.to_string(),
        )
        .unwrap();
        assert_eq!(address, "M7wtsL7wSHDBJVMWWhtQfTMSYYkyooAAXM")
    }

    #[test]
    fn test_btc_p2tr_no_script_address() {
        let extended_pubkey = "tpubDDfvzhdVV4unsoKt5aE6dcsNsfeWbTgmLZPi8LQDYU2xixrYemMfWJ3BaVneH3u7DBQePdTwhpybaKRU95pi6PMUtLPBJLVQRpzEnjfjZzX";
        let address =
            get_address("M/86'/1'/0'/0/0".to_string(), &extended_pubkey.to_string()).unwrap();
        assert_eq!(
            address,
            "tb1p8wpt9v4frpf3tkn0srd97pksgsxc5hs52lafxwru9kgeephvs7rqlqt9zj"
        );
    }
}
