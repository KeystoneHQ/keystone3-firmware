use crate::errors::{Result, TronError};
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use keystore::algorithms::secp256k1::derive_public_key;
use third_party::base58;
use third_party::cryptoxide::hashing::keccak256;

#[macro_export]
macro_rules! check_hd_path {
    ($t: expr) => {{
        let mut result: Result<()> = Ok(());
        if $t.len() != 6 {
            result = Err(TronError::InvalidHDPath(format!("{:?}", $t)));
        };
        result
    }};
}

#[macro_export]
macro_rules! derivation_address_path {
    ($t: expr) => {{
        let parts = $t.split("/").collect::<Vec<&str>>();
        let result: Result<String> = match crate::check_hd_path!(parts) {
            Ok(_) => {
                let path = parts.as_slice()[parts.len() - 2..].to_vec().join("/");
                Ok(format!("{}{}", "m/", path))
            }
            Err(e) => Err(e),
        };
        result
    }};
}

pub fn get_address(path: String, extended_pub_key: &String) -> Result<String> {
    let derivation_path = derivation_address_path!(path)?;
    let pubkey = derive_public_key(extended_pub_key, &derivation_path)?;
    let pubkey_bytes = pubkey.serialize_uncompressed();
    let hash = keccak256(&pubkey_bytes[1..]);
    let mut address_bytes = [0u8; 21];
    address_bytes[0] = 0x41;
    address_bytes[1..].copy_from_slice(&hash[hash.len() - 20..]);
    let address = base58::encode_check(&address_bytes);
    Ok(address.to_string())
}

#[cfg(test)]
mod tests {
    extern crate std;

    use crate::address::get_address;
    use alloc::string::ToString;

    #[test]
    fn get_address_test() {
        let extended_pub_key = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        {
            let path = "m/44'/195'/0'/0/0";
            let address = get_address(path.to_string(), &extended_pub_key.to_string()).unwrap();
            assert_eq!("TUEZSdKsoDHQMeZwihtdoBiN46zxhGWYdH".to_string(), address);
        }
        {
            let path = "m/44'/195'/0'/0/1";
            let address = get_address(path.to_string(), &extended_pub_key.to_string()).unwrap();
            assert_eq!("TSeJkUh4Qv67VNFwY8LaAxERygNdy6NQZK".to_string(), address);
        }
        {
            let path = "m/44'/195'/0'/0/2";
            let address = get_address(path.to_string(), &extended_pub_key.to_string()).unwrap();
            assert_eq!("TYJPRrdB5APNeRs4R7fYZSwW3TcrTKw2gx".to_string(), address);
        }
    }
}
