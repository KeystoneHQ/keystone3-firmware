use crate::address::ripple_keypair::derive_address;
use crate::errors::{XRPError, R};
use alloc::format;
use alloc::string::{String, ToString};
use keystore::algorithms::secp256k1::derive_public_key;

pub(crate) mod ripple_address_codec;
pub(crate) mod ripple_keypair;

pub fn get_address(hd_path: &str, root_x_pub: &str, root_path: &str) -> R<String> {
    let root_path = if !root_path.ends_with("/") {
        root_path.to_string() + "/"
    } else {
        root_path.to_string()
    };
    let sub_path = hd_path
        .strip_prefix(&root_path)
        .ok_or(XRPError::InvalidHDPath(hd_path.to_string()))?;
    let pubkey = derive_public_key(&root_x_pub.to_string(), &format!("m/{}", sub_path))?;
    derive_address(&pubkey.serialize())
}

#[cfg(test)]
mod tests {
    extern crate std;

    use crate::address::get_address;
    use alloc::string::ToString;

    #[test]
    fn get_address_test() {
        let extended_pub_key = "xpub6CFKyZTfzj3cyeRLUDKwQQ5s1tqTTdVgywKMVkrB2i1taGFbhazkxDzWVsfBHZpv7rg6qpDBGYR5oA8iazEfa44CdQkkknPFHJ7YCzncCS9";
        let root_path = "44'/144'/0'";
        {
            let path = "44'/144'/0'/0/0";
            let address = get_address(path, &extended_pub_key, root_path).unwrap();
            assert_eq!("rHsMGQEkVNJmpGWs8XUBoTBiAAbwxZN5v3".to_string(), address);
        }
        {
            let path = "44'/144'/0'/0/1";
            let address = get_address(path, extended_pub_key, root_path).unwrap();
            assert_eq!("r3AgF9mMBFtaLhKcg96weMhbbEFLZ3mx17".to_string(), address);
        }
        {
            let path = "44'/144'/0'/0/2";
            let address = get_address(path, extended_pub_key, root_path).unwrap();
            assert_eq!("r4Sh61HP7nxB6mQxXSSeN2DCkG3sTrzb2c".to_string(), address);
        }
    }
}
