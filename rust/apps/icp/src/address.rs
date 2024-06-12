use crate::errors::{ICPError, Result};
use alloc::string::{String};
use keystore::algorithms::secp256k1::derive_public_key;
use third_party::secp256k1::PublicKey;
use alloc::vec::Vec;
use pkcs8::EncodePublicKey;
use third_party::hex;
use crate::account_id::AccountIdentifier;
use crate::errors;
use crate::principal_id::PrincipalId;
use crate::types::ICPPublicKey;
#[macro_export]
macro_rules! check_hd_path {
    ($t: expr) => {{
        let mut result: Result<()> = Ok(());
        if $t.len() != 6 {
            result = Err(ICPError::InvalidHDPath(format!("{:?}", $t)));
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

impl TryFrom<PublicKey> for PrincipalId {
    type Error = ICPError;
    fn try_from(value: PublicKey) -> core::result::Result<Self, Self::Error> {
        let icp_public_key = ICPPublicKey::try_from(value)?;
        let der_public_key = icp_public_key.to_public_key_der().map_err(|e| {
            errors::ICPError::KeystoreError(format!("Could not convert public key to der: `{}`", e))
        })?;
        let principal_id: PrincipalId =
            PrincipalId::new_self_authenticating(&der_public_key.as_bytes());
        Ok(principal_id)
    }
}

impl TryFrom<PrincipalId> for AccountIdentifier {
    type Error = ICPError;
    fn try_from(value: PrincipalId) -> core::result::Result<Self, Self::Error> {
        let account_id: AccountIdentifier = AccountIdentifier::new(value.0, None);
        Ok(account_id)
    }
}

impl Into<String> for AccountIdentifier {
    fn into(self) -> String {
        self.to_hex()
    }
}

impl TryInto<String> for PrincipalId {
    type Error = ICPError;
    fn try_into(self) -> Result<String> {
        Ok(self.0.to_text())
    }
}

fn generate_principal_id_str(public_key: &String) -> Result<String> {
    let public_key_bytes = hex::decode(public_key).map_err(|e| {
        errors::ICPError::KeystoreError(format!("Could not convert public key from str: `{}`", e))
    })?;
    let public_key = PublicKey::from_slice(&public_key_bytes).map_err(|e| {
        errors::ICPError::KeystoreError(format!(
            "Could not convert publicKey from public str: `{}`",
            e
        ))
    })?;
    let principal_id = PrincipalId::try_from(public_key)?;

    Ok(principal_id.0.to_text())
}

pub fn generate_address(public_key: &String) -> Result<String> {
    let public_key_bytes = hex::decode(public_key).map_err(|e| {
        errors::ICPError::KeystoreError(format!("Could not convert public key from str: `{}`", e))
    })?;
    let public_key = PublicKey::from_slice(&public_key_bytes).map_err(|e| {
        errors::ICPError::KeystoreError(format!(
            "Could not convert publicKey from public str: `{}`",
            e
        ))
    })?;
    let principal_id = PrincipalId::try_from(public_key)?;
    let account_id = AccountIdentifier::try_from(principal_id)?;
    Ok(account_id.into())
}


pub fn generate_principal_id_from_public_key(public_key: PublicKey) -> Result<String> {
    let principal_id = PrincipalId::try_from(public_key)?;
    Ok(principal_id.0.to_text())
}

pub fn generate_address_from_public_key(public_key: PublicKey) -> Result<String> {
    let principal_id = PrincipalId::try_from(public_key)?;
    let account_id = AccountIdentifier::try_from(principal_id)?;
    Ok(account_id.into())
}


pub fn get_address(path: String, extended_pub_key: &String) -> Result<(String,String)> {
    let derivation_path = derivation_address_path!(path)?;
    let pubkey = derive_public_key(extended_pub_key, &derivation_path)?;
    let principal_id = PrincipalId::try_from(pubkey)?;
    let account_id = AccountIdentifier::try_from(principal_id)?;
    Ok(( principal_id.0.to_text(), account_id.into()))
}


#[cfg(test)]
mod tests {
    extern crate std;

    use crate::address::get_address;
    use alloc::string::ToString;
    use super::*;

    use crate::principal_id::PrincipalId;
    use alloc::str::FromStr;
    use third_party::hex;

    #[test]
    fn test_get_address() {
        let xpub = "xpub6BjfS8SrTv5qZ9QotEKMrdFLCnjNARKQ2mf6MmzzfiXeSWuzPxADyudaSC6dD3Tr6r9oQi8M7Kv6qpBvuWrzdCAvfyLASduDRThvbV5romw".to_string();
        let hd_path =  "m/44'/223'/0'/0/0".to_string();
        let result = get_address(hd_path, &xpub).unwrap();
        assert_eq!("x3imh-nlbtk-skuqi-drvjd-j3yqv-ci5l2-t75lk-fjsnk-lgtyk-nhyu4-2ae".to_string(), result.0);
        assert_eq!("fe1f869a7c914bd002812202d1cb954481fe85365fc763e969ac329c69de78dd".to_string(), result.1);
    }

    #[test]
    fn test_public_key_principal_id() {
        let public_key_bytes = [
            3, 59, 101, 243, 36, 181, 142, 146, 174, 252, 195, 17, 7, 16, 168, 230, 66, 167, 206,
            106, 94, 101, 65, 150, 148, 1, 255, 96, 112, 12, 5, 244, 57,
        ];
        let public_key = PublicKey::from_slice(&public_key_bytes).unwrap();
        let principal_id = PrincipalId::try_from(public_key).unwrap();
        assert_eq!(
            "7rtqo-ah3ki-saurz-utzxq-o4yhl-so2yx-iardd-mktej-x4k24-ijen6-dae".to_string(),
            principal_id.0.to_text()
        );
    }

    #[test]
    fn test_from_principal_id_to_account_id() {
        let principal_id_str = "7rtqo-ah3ki-saurz-utzxq-o4yhl-so2yx-iardd-mktej-x4k24-ijen6-dae";
        let principal_id = PrincipalId::from_str(&principal_id_str).unwrap();
        let account_id = AccountIdentifier::try_from(principal_id).unwrap();
        let account_id_str:String = account_id.into();
        assert_eq!(
            "33a807e6078195d2bbe1904b0ed0fc65b8a3a437b43831ccebba2b7b6d393bd6".to_string(),
            account_id_str
        );
    }

    #[test]
    fn test_from_public_key_to_address() {
        let public_key_bytes = [
            3, 59, 101, 243, 36, 181, 142, 146, 174, 252, 195, 17, 7, 16, 168, 230, 66, 167, 206,
            106, 94, 101, 65, 150, 148, 1, 255, 96, 112, 12, 5, 244, 57,
        ];
        let public_key = PublicKey::from_slice(&public_key_bytes).unwrap();
        let public_key_str = hex::encode(public_key.serialize());
        let address = generate_address(&public_key_str).unwrap();
        assert_eq!(
            "33a807e6078195d2bbe1904b0ed0fc65b8a3a437b43831ccebba2b7b6d393bd6".to_string(),
            address
        );
    }

    #[test]
    fn test_generate_principal_id_str() {
        let public_key_bytes = [
            3, 59, 101, 243, 36, 181, 142, 146, 174, 252, 195, 17, 7, 16, 168, 230, 66, 167, 206,
            106, 94, 101, 65, 150, 148, 1, 255, 96, 112, 12, 5, 244, 57,
        ];
        let public_key = PublicKey::from_slice(&public_key_bytes).unwrap();
        let public_key_str = hex::encode(public_key.serialize());
        let principal_id_str = generate_principal_id_str(&public_key_str).unwrap();
        assert_eq!(
            "7rtqo-ah3ki-saurz-utzxq-o4yhl-so2yx-iardd-mktej-x4k24-ijen6-dae".to_string(),
            principal_id_str
        );
    }
    #[test]
    fn get_address_test() {
        let extended_pub_key = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        {
            let path = "m/44'/195'/0'/0/0";
            let (principal_id, address) = get_address(path.to_string(), &extended_pub_key.to_string()).unwrap();
        }
    }
}

