use crate::crypto::keccak256;
use alloc::format;
use alloc::string::{String, ToString};
use bitcoin::secp256k1::PublicKey;
use hex;
use keystore::algorithms::secp256k1::derive_public_key;

use crate::{EthereumError, Result};

pub fn generate_address(key: PublicKey) -> Result<String> {
    let hash: [u8; 32] = keccak256(&key.serialize_uncompressed()[1..]);
    checksum_address(&hex::encode(&hash[12..]))
}

fn checksum_address(address: &str) -> Result<String> {
    let address = address.trim_start_matches("0x").to_lowercase();
    let address_hash = hex::encode(keccak256(address.as_bytes()));
    address
        .char_indices()
        .fold(Ok(String::from("0x")), |acc, (index, address_char)| {
            let n = u8::from_str_radix(&address_hash[index..index + 1], 16)
                .map_err(|e| EthereumError::InvalidAddressError(e.to_string()));
            acc.and_then(|mut v1| {
                n.map(|v2| {
                    if v2 >= 8 {
                        v1.push_str(&address_char.to_uppercase().to_string());
                        v1
                    } else {
                        v1.push(address_char);
                        v1
                    }
                })
            })
        })
}

pub fn derive_address(hd_path: &str, root_x_pub: &str, root_path: &str) -> Result<String> {
    let root_path = if !root_path.ends_with('/') {
        root_path.to_string() + "/"
    } else {
        root_path.to_string()
    };
    let sub_path = hd_path
        .strip_prefix(&root_path)
        .ok_or(EthereumError::InvalidHDPath(hd_path.to_string()))?;
    derive_public_key(&root_x_pub.to_string(), &format!("m/{}", sub_path))
        .map(generate_address)
        .map_err(EthereumError::from)?
}

#[cfg(test)]
mod tests {

    use super::*;
    extern crate std;
    use core::str::FromStr;
    use std::println;

    #[test]
    fn test_generate_address() {
        let root_x_pub = "xpub6BtigCpsVJrCGVhsuMuAshHuQctVUKUeumxP4wkUtypFpXatQ44ZCHwZi6w4Gf5kMN3vpfyGnHo5hLvgjs2NnkewYHSdVHX4oUbR1Xzxc7E";
        let root_path = "44'/60'/0'";

        // bip44_standard path
        let hd_path = "44'/60'/0'/0/0";
        let result = derive_address(hd_path, root_x_pub, root_path).unwrap();
        assert_eq!("0x3660cc029702A0AEB125B380f5FA549Cc7Ad076e", result);

        // ledger_legacy path
        let hd_path = "44'/60'/0'/0";
        let result = derive_address(hd_path, root_x_pub, root_path).unwrap();
        assert_eq!("0xDcDFDD4fCF2653A8032a1615bEbf22E62241F074", result);

        // ledger_live path
        let hd_path = "44'/60'/1'/0/0";
        let root_x_pub = "xpub6BtigCpsVJrCJZsM7fwcwCX8dhAn5Drg5QnMQY1wgzX1BMHGHPHB9qjmvnqgK6BECXyVTkGdr4CTyNyhaMXKdSmEVkSd4w7ePqaBvzjMxJ9";
        let root_path = "44'/60'/1'";
        let result = derive_address(hd_path, root_x_pub, root_path).unwrap();
        assert_eq!("0x31eA4a0976ceE79AF136B1Cfa914e20E87546156", result);
    }

    fn get_avax_address(public_key: &PublicKey) -> String {
        // 初始化 secp256k1
        // let secp = Secp256k1::new();

        // 从私钥生成公钥
        // let secret_key = SecretKey::from_slice(private_key).unwrap();
        // let public_key = PublicKey::from_secret_key(&secp, &secret_key);
        // println!("public_key11 = {:?}", public_key);

        // 序列化公钥并去掉第一个字节
        // let pub_key_bytes = public_key.serialize_uncompressed();
        // let pub_key_without_prefix = &pub_key_bytes[1..];

        // 计算 Keccak-256 哈希
        // let mut keccak = Keccak::v256();
        // let mut hash = [0u8; 32];
        // keccak.update(pub_key_without_prefix);
        // keccak.finalize(&mut hash);
        let hash: [u8; 32] = keccak256(&public_key.serialize_uncompressed()[1..]);
        println!("hash = {}", hex::encode(hash));

        // 取最后20字节作为地址
        let address = &hash[12..];

        let address = [
            63, 229, 19, 56, 153, 45, 145, 59, 106, 153, 150, 147, 215, 195, 69, 100, 108, 163,
            187, 76,
        ];

        // 转换为16进制并添加0x前缀
        format!("0x{}", hex::encode(&address))
    }

    #[test]
    fn test_avax_address() {
        let pubkey = PublicKey::from_str(
            "02f01fe98afe9db6a160ec7fd57d4de1dcf9848c97307c4e4babc29ee34074a606",
        )
        .unwrap();
        println!("{}", pubkey);
        let address = get_avax_address(&pubkey);
        println!("address = {}", address);
        let result = generate_address(pubkey).unwrap();
        println!("{}", result);
        assert_eq!(2, 1);
    }
}
