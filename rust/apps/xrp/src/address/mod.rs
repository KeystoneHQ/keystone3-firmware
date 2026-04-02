use crate::address::ripple_keypair::derive_address;
use crate::errors::{XRPError, R};
use alloc::format;
use alloc::string::{String, ToString};
use keystore::algorithms::secp256k1::derive_public_key;

pub(crate) mod ripple_address_codec;
pub(crate) mod ripple_keypair;

pub fn get_address(hd_path: &str, root_x_pub: &str, root_path: &str) -> R<String> {
    let root_path = if !root_path.ends_with('/') {
        root_path.to_string() + "/"
    } else {
        root_path.to_string()
    };
    let sub_path = hd_path
        .strip_prefix(&root_path)
        .ok_or(XRPError::InvalidHDPath(hd_path.to_string()))?;
    let pubkey = derive_public_key(&root_x_pub.to_string(), &format!("m/{sub_path}"))?;
    derive_address(&pubkey.serialize())
}

#[cfg(test)]
mod tests {
    extern crate std;

    use crate::address::get_address;
    use crate::errors::XRPError;
    use alloc::string::ToString;

    #[test]
    fn get_address_test() {
        let extended_pub_key = "xpub6CFKyZTfzj3cyeRLUDKwQQ5s1tqTTdVgywKMVkrB2i1taGFbhazkxDzWVsfBHZpv7rg6qpDBGYR5oA8iazEfa44CdQkkknPFHJ7YCzncCS9";
        let root_path = "44'/144'/0'";
        {
            let path = "44'/144'/0'/0/0";
            let address = get_address(path, extended_pub_key, root_path).unwrap();
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

    #[test]
    fn test_invalid_hd_path_prefix() {
        let extended_pub_key = "xpub6CFKyZTfzj3cyeRLUDKwQQ5s1tqTTdVgywKMVkrB2i1taGFbhazkxDzWVsfBHZpv7rg6qpDBGYR5oA8iazEfa44CdQkkknPFHJ7YCzncCS9";
        let root_path = "44'/144'/0'";

        // HD path doesn't start with root path
        let path = "44'/144'/1'/0/0";
        let result = get_address(path, extended_pub_key, root_path);
        assert!(result.is_err());
        assert!(matches!(result.unwrap_err(), XRPError::InvalidHDPath(_)));
    }

    #[test]
    fn test_completely_different_path() {
        let extended_pub_key = "xpub6CFKyZTfzj3cyeRLUDKwQQ5s1tqTTdVgywKMVkrB2i1taGFbhazkxDzWVsfBHZpv7rg6qpDBGYR5oA8iazEfa44CdQkkknPFHJ7YCzncCS9";
        let root_path = "44'/144'/0'";

        // Completely different path
        let path = "44'/60'/0'/0/0";
        let result = get_address(path, extended_pub_key, root_path);
        assert!(result.is_err());
        assert!(matches!(result.unwrap_err(), XRPError::InvalidHDPath(_)));
    }

    #[test]
    fn test_empty_hd_path() {
        let extended_pub_key = "xpub6CFKyZTfzj3cyeRLUDKwQQ5s1tqTTdVgywKMVkrB2i1taGFbhazkxDzWVsfBHZpv7rg6qpDBGYR5oA8iazEfa44CdQkkknPFHJ7YCzncCS9";
        let root_path = "44'/144'/0'";

        // Empty HD path
        let path = "";
        let result = get_address(path, extended_pub_key, root_path);
        assert!(result.is_err());
    }

    #[test]
    fn test_empty_root_path() {
        let extended_pub_key = "xpub6CFKyZTfzj3cyeRLUDKwQQ5s1tqTTdVgywKMVkrB2i1taGFbhazkxDzWVsfBHZpv7rg6qpDBGYR5oA8iazEfa44CdQkkknPFHJ7YCzncCS9";

        // Empty root path should still work if path is also appropriate
        let root_path = "";
        let path = "0/0";
        let result = get_address(path, extended_pub_key, root_path);
        assert!(result.is_err());
    }

    #[test]
    fn test_invalid_extended_pub_key() {
        let root_path = "44'/144'/0'";
        let path = "44'/144'/0'/0/0";

        // Invalid extended public key format
        let invalid_xpub = "invalid_xpub_key";
        let result = get_address(path, invalid_xpub, root_path);
        assert!(result.is_err());
    }

    #[test]
    fn test_malformed_extended_pub_key() {
        let root_path = "44'/144'/0'";
        let path = "44'/144'/0'/0/0";

        // Malformed extended public key (too short)
        let invalid_xpub = "xpub";
        let result = get_address(path, invalid_xpub, root_path);
        assert!(result.is_err());
    }

    #[test]
    fn test_path_shorter_than_root() {
        let extended_pub_key = "xpub6CFKyZTfzj3cyeRLUDKwQQ5s1tqTTdVgywKMVkrB2i1taGFbhazkxDzWVsfBHZpv7rg6qpDBGYR5oA8iazEfa44CdQkkknPFHJ7YCzncCS9";
        let root_path = "44'/144'/0'/0/0";

        // Path is shorter than root path
        let path = "44'/144'/0'";
        let result = get_address(path, extended_pub_key, root_path);
        assert!(result.is_err());
        assert!(matches!(result.unwrap_err(), XRPError::InvalidHDPath(_)));
    }

    #[test]
    fn test_malformed_hd_path_format() {
        let extended_pub_key = "xpub6CFKyZTfzj3cyeRLUDKwQQ5s1tqTTdVgywKMVkrB2i1taGFbhazkxDzWVsfBHZpv7rg6qpDBGYR5oA8iazEfa44CdQkkknPFHJ7YCzncCS9";
        let root_path = "44'/144'/0'";

        // Malformed HD path (invalid characters)
        let path = "44'/144'/0'/0/abc";
        let result = get_address(path, extended_pub_key, root_path);
        assert!(result.is_err());
    }

    #[test]
    fn test_root_path_with_trailing_slash() {
        let extended_pub_key = "xpub6CFKyZTfzj3cyeRLUDKwQQ5s1tqTTdVgywKMVkrB2i1taGFbhazkxDzWVsfBHZpv7rg6qpDBGYR5oA8iazEfa44CdQkkknPFHJ7YCzncCS9";

        // Root path already has trailing slash - should still work
        let root_path = "44'/144'/0'/";
        let path = "44'/144'/0'/0/0";
        let result = get_address(path, extended_pub_key, root_path);
        assert!(result.is_ok());
        assert_eq!(
            "rHsMGQEkVNJmpGWs8XUBoTBiAAbwxZN5v3".to_string(),
            result.unwrap()
        );
    }

    #[test]
    fn test_special_characters_in_path() {
        let extended_pub_key = "xpub6CFKyZTfzj3cyeRLUDKwQQ5s1tqTTdVgywKMVkrB2i1taGFbhazkxDzWVsfBHZpv7rg6qpDBGYR5oA8iazEfa44CdQkkknPFHJ7YCzncCS9";
        let root_path = "44'/144'/0'";

        // Path with special characters
        let path = "44'/144'/0'/@#$/!@#";
        let result = get_address(path, extended_pub_key, root_path);
        assert!(result.is_err());
    }
}
