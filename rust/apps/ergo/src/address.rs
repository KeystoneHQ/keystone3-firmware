use crate::errors::ErgoError;
use crate::errors::ErgoError::{DerivationError, TransactionParseError};
use crate::errors::Result;
use crate::{derivation_account_path};
use alloc::vec::Vec;
use alloc::string::{String, ToString};
use core::str::FromStr;
use ergo_lib::ergotree_ir::chain::address::{Address, NetworkAddress, NetworkPrefix};
use ergo_lib::ergotree_ir::ergo_tree::ErgoTree;
use ergo_lib::ergotree_ir::serialization::SigmaSerializable;
use ergo_lib::wallet::derivation_path::DerivationPath;
use ergo_lib::wallet::ext_pub_key::ExtPubKey;
use hex;

pub fn get_address(hd_path: String, extended_pub_key: &String) -> Result<String> {
    let xpub_bytes = hex::decode(extended_pub_key).map_err(|e| DerivationError(e.to_string()))?;

    if xpub_bytes.len() != 78 {
        return Err(DerivationError("invalid pubkey".to_string()));
    }

    let chain_code_bytes = xpub_bytes[13..45].to_vec();
    let extended_public_key_bytes = xpub_bytes[45..78].to_vec();

    let account_derivation_path_str = derivation_account_path!(hd_path)?;
    let account_derivation_path = DerivationPath::from_str(&account_derivation_path_str)
        .map_err(|e| DerivationError(e.to_string()))?;

    let ext_pub_key = ExtPubKey::new(
        extended_public_key_bytes.try_into().unwrap(),
        chain_code_bytes.try_into().unwrap(),
        account_derivation_path,
    )
    .map_err(|e| DerivationError(e.to_string()))?;

    let address_derivation_path =
        DerivationPath::from_str(&*hd_path).map_err(|e| DerivationError(e.to_string()))?;
    let derived_pub_key = ext_pub_key
        .derive(address_derivation_path)
        .map_err(|e| DerivationError(e.to_string()))?;

    let address = Address::from(derived_pub_key);
    let network_address = NetworkAddress::new(NetworkPrefix::Mainnet, &address);
    Ok(network_address.to_base58())
}

pub fn ergo_tree_to_address(tree: String) -> Result<String> {
    let tree_bytes = hex::decode(tree)
        .map_err(|e| TransactionParseError(format!("could not decode tree from string {}", e.to_string())))?;
    let ergo_tree = ErgoTree::sigma_parse_bytes(&*tree_bytes)
        .map_err(|e| TransactionParseError(format!("could not parse tree from bytes {}", e.to_string())))?;

    let address = Address::recreate_from_ergo_tree(&ergo_tree)
        .map_err(|e| TransactionParseError(format!("could not recreate address from tree {}", e.to_string())))?;
    Ok(NetworkAddress::new(NetworkPrefix::Mainnet, &address).to_base58())
}

#[cfg(test)]
mod tests {
    use crate::address::get_address;
    use alloc::string::ToString;

    #[test]
    fn test_get_address() {
        let pub_key_str = "0488b21e030000000000000000fb3010c71c8cd02f74c188e41aac28c171feb6895e0473a1a5009314115d2b9c021c73cd9103dff64df560acf68a9d60dd74dad282caeff8bba53fcbd55185ebbd";
        let address =
            get_address("m/44'/429'/0'/0/0".to_string(), &pub_key_str.to_string()).unwrap();
        assert_eq!(
            address,
            "9hKtJhaaDXzmHujpWofjZEGLo9vF4j6ueG9vvk9LzrGDKjjMVuo"
        )
    }
}
