use crate::derivation_account_path;
use crate::errors::ErgoError;
use crate::errors::ErgoError::{DerivationError, TransactionParseError};
use crate::errors::Result;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use bitcoin::bip32::Xpub;
use core::str::FromStr;
use ergo_lib::ergotree_ir::chain::address::{
    base58_address_from_tree, Address, NetworkAddress, NetworkPrefix,
};
use ergo_lib::ergotree_ir::ergo_tree::ErgoTree;
use ergo_lib::ergotree_ir::serialization::SigmaSerializable;
use ergo_lib::wallet::derivation_path::DerivationPath;
use ergo_lib::wallet::ext_pub_key::ExtPubKey;
use hex;

pub fn get_address(hd_path: String, extended_pub_key: &String) -> Result<String> {
    let xpub = Xpub::from_str(extended_pub_key).map_err(|e| DerivationError(e.to_string()))?;

    let account_derivation_path_str = derivation_account_path!(hd_path)?;
    let account_derivation_path = DerivationPath::from_str(&account_derivation_path_str)
        .map_err(|e| DerivationError(e.to_string()))?;

    let ext_pub_key = ExtPubKey::new(
        xpub.public_key.serialize(),
        xpub.chain_code.to_bytes(),
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
    let tree_bytes = hex::decode(tree).map_err(|e| {
        TransactionParseError(format!(
            "could not decode tree from string {}",
            e.to_string()
        ))
    })?;
    let ergo_tree = ErgoTree::keystone_sigma_parse_bytes(&*tree_bytes).map_err(|e| {
        TransactionParseError(format!("could not parse tree from bytes {}", e.to_string()))
    })?;

    let address = base58_address_from_tree(NetworkPrefix::Mainnet, &ergo_tree).map_err(|e| {
        TransactionParseError(format!(
            "could not recreate address from tree {}",
            e.to_string()
        ))
    })?;
    Ok(address)
}

#[cfg(test)]
mod tests {
    use crate::address::get_address;
    use alloc::string::ToString;

    #[test]
    fn test_get_address() {
        let xpub_key_str = "xpub6CZReWJVzzJbfDSpfFyKSBhHTFKStaK2DocPmPveZnF22JiJjPtyL6ZhwfMGthbGytaL6C8D7b9sKNtQ4UmjguNs5khEw74mry4NeSoKMCG";
        let address =
            get_address("m/44'/429'/0'/0/0".to_string(), &xpub_key_str.to_string()).unwrap();
        assert_eq!(
            address,
            "9hKtJhaaDXzmHujpWofjZEGLo9vF4j6ueG9vvk9LzrGDKjjMVuo"
        )
    }
}
