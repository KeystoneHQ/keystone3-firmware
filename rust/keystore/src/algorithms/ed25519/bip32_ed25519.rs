use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::str::FromStr;
use third_party::cryptoxide::mac::Mac;

use crate::algorithms::utils::normalize_path;
use third_party::bitcoin::bip32::{ChildNumber, DerivationPath};
use third_party::cryptoxide::hmac;
use third_party::cryptoxide::pbkdf2;
use third_party::cryptoxide::sha2::{Sha256, Sha512};
use third_party::ed25519_bip32_core::{DerivationScheme, XPrv, XPub};

use crate::errors::{KeystoreError, Result};

pub fn get_extended_public_key_by_entropy(
    entropy: &[u8],
    passphrase: &[u8],
    path: &String,
) -> Result<XPub> {
    let xprv = get_extended_private_key_by_entropy(entropy, passphrase, path)?;
    Ok(xprv.public())
}

pub fn get_extended_private_key_by_entropy(
    entropy: &[u8],
    passphrase: &[u8],
    path: &String,
) -> Result<XPrv> {
    let icarus_master_key = get_icarus_master_key_by_entropy(entropy, passphrase)?;
    let path = normalize_path(path);
    let derivation_path = DerivationPath::from_str(path.as_str())
        .map_err(|e| KeystoreError::InvalidDerivationPath(format!("{}", e)))?;
    let childrens: Vec<ChildNumber> = derivation_path.into();
    let key = childrens
        .iter()
        .fold(icarus_master_key, |acc, cur| match cur {
            ChildNumber::Hardened { index } => acc.derive(DerivationScheme::V2, index + 0x80000000),
            ChildNumber::Normal { index } => acc.derive(DerivationScheme::V2, *index),
        });
    Ok(key)
}

pub fn sign_message_by_entropy(
    entropy: &[u8],
    passphrase: &[u8],
    message: &[u8],
    path: &String,
) -> Result<[u8; 64]> {
    let xprv = get_extended_private_key_by_entropy(entropy, passphrase, path)?;
    let sig = xprv.sign::<Vec<u8>>(message);
    Ok(*sig.to_bytes())
}

//https://cips.cardano.org/cips/cip3/icarus.md
pub fn get_icarus_master_key_by_entropy(entropy: &[u8], passphrase: &[u8]) -> Result<XPrv> {
    let mut hash = [0u8; 96];
    let digest = Sha512::new();
    let iter_count = 4096;
    pbkdf2::pbkdf2(
        &mut hmac::Hmac::new(digest, passphrase),
        entropy,
        iter_count,
        &mut hash,
    );
    Ok(XPrv::normalize_bytes_force3rd(hash))
}

fn split_in_half(data: Vec<u8>) -> (Vec<u8>, Vec<u8>) {
    let mid = data.len() / 2;
    (data[0..mid].to_vec(), data[mid..].to_vec())
}

// https://github.com/cardano-foundation/CIPs/blob/49c64d4a7ce9f200e6fab7bd3fa855a5f1cd880a/CIP-0003/Ledger_BitBox02.md
pub fn get_ledger_bitbox02_master_key_by_mnemonic(
    passphrase: &[u8],
    mnemonic_words: String,
) -> Result<XPrv> {
    let salt = ["mnemonic".as_bytes(), passphrase].concat();
    let mut key = vec![0u8; 64];
    let digest = Sha512::new();
    let hmac = &mut hmac::Hmac::new(digest, mnemonic_words.as_bytes());
    pbkdf2::pbkdf2(hmac, &salt, 2048, &mut key);

    let digest = Sha512::new();
    let hmac = &mut hmac::Hmac::new(digest, "ed25519 seed".as_bytes());
    hmac.input(&key);
    let (mut i_l, mut i_r) = split_in_half(hmac.result().code().to_vec());
    loop {
        if i_l[31] & 0b0010_0000 == 0 {
            break;
        }
        let digest = Sha512::new();
        let hmac = &mut hmac::Hmac::new(digest, "ed25519 seed".as_bytes());
        hmac.input(&[i_l, i_r].concat());
        let ret = hmac.result();
        let result: &[u8] = ret.code();
        i_l = result[0..32].to_vec();
        i_r = result[32..64].to_vec();
    }

    let digest = Sha256::new();
    let hmac = &mut hmac::Hmac::new(digest, "ed25519 seed".as_bytes());
    hmac.input(&[vec![1], key].concat());
    let cc = hmac.result().code().to_vec();

    let ret = [i_l, i_r, cc].concat();

    Ok(XPrv::normalize_bytes_force3rd(ret.try_into().unwrap()))
}

pub fn sign_message_by_icarus_master_key(
    master_key: &[u8],
    message: &[u8],
    path: &String,
) -> Result<[u8; 64]> {
    let xprv = derive_extended_privkey_by_icarus_master_key(master_key, path)?;
    let sig = xprv.sign::<Vec<u8>>(message);
    Ok(*sig.to_bytes())
}

pub fn sign_message_by_xprv(xprv: &XPrv, message: &[u8], path: &String) -> Result<[u8; 64]> {
    let xprv = derive_extended_privkey_by_xprv(xprv, path)?;
    let sig = xprv.sign::<Vec<u8>>(message);
    Ok(*sig.to_bytes())
}

pub fn derive_extended_pubkey_by_icarus_master_key(
    master_key: &[u8],
    path: &String,
) -> Result<XPub> {
    let privkey = derive_extended_privkey_by_icarus_master_key(master_key, path)?;
    Ok(privkey.public())
}

pub fn derive_extended_pubkey_by_xprv(xprv: &XPrv, path: &String) -> Result<XPub> {
    let privkey = derive_extended_privkey_by_xprv(xprv, path)?;
    Ok(privkey.public())
}

pub fn derive_extended_privkey_by_icarus_master_key(
    master_key: &[u8],
    path: &String,
) -> Result<XPrv> {
    let xprv = XPrv::from_slice_verified(master_key)
        .map_err(|e| KeystoreError::DerivationError(e.to_string()))?;
    derive_bip32_ed25519_privkey(xprv, path)
}

pub fn derive_extended_privkey_by_xprv(xprv: &XPrv, path: &String) -> Result<XPrv> {
    derive_bip32_ed25519_privkey(xprv.clone(), path)
}

fn derive_bip32_ed25519_privkey(root: XPrv, path: &String) -> Result<XPrv> {
    let path = normalize_path(path);
    let derivation_path = DerivationPath::from_str(path.as_str())
        .map_err(|e| KeystoreError::InvalidDerivationPath(format!("{}", e)))?;
    let childrens: Vec<ChildNumber> = derivation_path.into();
    let key = childrens.iter().fold(root, |acc, cur| match cur {
        ChildNumber::Hardened { index } => acc.derive(DerivationScheme::V2, index + 0x80000000),
        ChildNumber::Normal { index } => acc.derive(DerivationScheme::V2, *index),
    });
    Ok(key)
}

#[cfg(test)]
mod tests {

    use super::*;
    use std::string::ToString;
    use third_party::hex;

    extern crate std;

    #[test]
    fn test_icarus_master_key() {
        let entropy = hex::decode("00000000000000000000000000000000").unwrap();
        let icarus_master_key = get_icarus_master_key_by_entropy(entropy.as_slice(), b"").unwrap();
        assert_eq!("60ce7dbec3616e9fc17e0c32578b3f380337b1b61a1f3cb9651aee30670e6f53970419a23a2e4e4082d12bf78faa8645dfc882cee2ae7179e2b07fe88098abb2072310084784c7308182dbbdb1449b2706586f1ff5cbf13d15e9b6e78c15f067",
                   icarus_master_key.to_string())
    }

    #[test]
    fn test_icarus_master_key_with_passphrase() {
        let entropy = hex::decode("46e62370a138a182a498b8e2885bc032379ddf38").unwrap();
        let mut icarus_master_key =
            get_icarus_master_key_by_entropy(entropy.as_slice(), b"").unwrap();
        assert_eq!("c065afd2832cd8b087c4d9ab7011f481ee1e0721e78ea5dd609f3ab3f156d245d176bd8fd4ec60b4731c3918a2a72a0226c0cd119ec35b47e4d55884667f552a23f7fdcd4a10c6cd2c7393ac61d877873e248f417634aa3d812af327ffe9d620",
                   icarus_master_key.to_string());

        icarus_master_key = get_icarus_master_key_by_entropy(entropy.as_slice(), b"foo").unwrap();
        assert_eq!("70531039904019351e1afb361cd1b312a4d0565d4ff9f8062d38acf4b15cce41d7b5738d9c893feea55512a3004acb0d222c35d3e3d5cde943a15a9824cbac59443cf67e589614076ba01e354b1a432e0e6db3b59e37fc56b5fb0222970a010e",
                   icarus_master_key.to_string())
    }

    #[test]
    fn test_get_extended_private_key() {
        {
            let entropy = hex::decode("00000000000000000000000000000000").unwrap();
            let key =
                get_extended_private_key_by_entropy(entropy.as_slice(), b"", &"m/0'".to_string())
                    .unwrap();
            assert_eq!("8872ff61b06281da05205ffb765c256175cc2aaab52cd7176b5d80286c0e6f539e936c7b5f018c935544e3dff339dfe739c47ae8b364330a3162f028c658257b35a473378343fcc479fd326bc2c2a23f183ce682d514bd5a5b1d9a14ff8297cf",
                       key.to_string())
        }
        {
            let entropy = hex::decode("00000000000000000000000000000000").unwrap();
            let key = get_extended_public_key_by_entropy(
                entropy.as_slice(),
                b"",
                &"m/1852'/1815'/0'".to_string(),
            )
            .unwrap();
            assert_eq!("beb7e770b3d0f1932b0a2f3a63285bf9ef7d3e461d55446d6a3911d8f0ee55c0b0e2df16538508046649d0e6d5b32969555a23f2f1ebf2db2819359b0d88bd16",
                    key.to_string())
        }
    }

    #[test]
    fn test_get_ledger_icarus_master_key_by_entropy() {
        {
            let mnemonic_words = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon art";
            let master_key = get_ledger_bitbox02_master_key_by_mnemonic(
                "foo".as_bytes(),
                mnemonic_words.to_string(),
            )
            .unwrap();
            assert_eq!("f053a1e752de5c26197b60f032a4809f08bb3e5d90484fe42024be31efcba7578d914d3ff992e21652fee6a4d99f6091006938fac2c0c0f9d2de0ba64b754e92a4f3723f23472077aa4cd4dd8a8a175dba07ea1852dad1cf268c61a2679c3890",
                master_key.to_string());
        }
        {
            let mnemonic_words = "correct cherry mammal bubble want mandate polar hazard crater better craft exotic choice fun tourist census gap lottery neglect address glow carry old business";
            let master_key =
                get_ledger_bitbox02_master_key_by_mnemonic("".as_bytes(), mnemonic_words.to_string())
                    .unwrap();
            assert_eq!("587c6774357ecbf840d4db6404ff7af016dace0400769751ad2abfc77b9a3844cc71702520ef1a4d1b68b91187787a9b8faab0a9bb6b160de541b6ee62469901fc0beda0975fe4763beabd83b7051a5fd5cbce5b88e82c4bbaca265014e524bd",
                master_key.to_string());
        }
        {
            let mnemonic_words = "recall grace sport punch exhibit mad harbor stand obey short width stem awkward used stairs wool ugly trap season stove worth toward congress jaguar";
            let master_key =
                get_ledger_bitbox02_master_key_by_mnemonic("".as_bytes(), mnemonic_words.to_string())
                    .unwrap();
            assert_eq!("a08cf85b564ecf3b947d8d4321fb96d70ee7bb760877e371899b14e2ccf88658104b884682b57efd97decbb318a45c05a527b9cc5c2f64f7352935a049ceea60680d52308194ccef2a18e6812b452a5815fbd7f5babc083856919aaf668fe7e4",
                master_key.to_string());
        }
    }
}
