use alloc::string::ToString;
use app_xrp::address::get_address;
use hex;
use keystore::algorithms::secp256k1::derive_public_key;
use serde_json::{json, Value};
use ur_registry::bytes::Bytes;
use ur_registry::error::URError::UrEncodeError;
use ur_registry::error::URResult;

pub fn generate_sync_ur(hd_path: &str, root_x_pub: &str, root_path: &str) -> URResult<Bytes> {
    let root_path = if !root_path.ends_with("/") {
        root_path.to_string() + "/"
    } else {
        root_path.to_string()
    };
    let sub_path = hd_path.strip_prefix(&root_path).unwrap();
    if let (Ok(address), Ok(pubkey)) = (
        get_address(hd_path, root_x_pub, root_path.as_str()),
        derive_public_key(&root_x_pub.to_string(), &format!("m/{}", sub_path)),
    ) {
        let v: Value = json!({
            "address": address,
            "pubkey": hex::encode(pubkey.serialize())
        });
        let input_bytes = v.to_string().into_bytes();
        return Ok(Bytes::new(input_bytes));
    }
    return Err(UrEncodeError("invalid data".to_string()));
}
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_generate_xrp_toolkit_sync_ur() {
        let hd_path = "44'/144'/0'/0/0";
        let root_x_pub="xpub6CFKyZTfzj3cyeRLUDKwQQ5s1tqTTdVgywKMVkrB2i1taGFbhazkxDzWVsfBHZpv7rg6qpDBGYR5oA8iazEfa44CdQkkknPFHJ7YCzncCS9";
        let root_path = "44'/144'/0'";
        let result = generate_sync_ur(hd_path, root_x_pub, root_path).unwrap();
        assert_eq!("7b2261646472657373223a227248734d4751456b564e4a6d70475773385855426f54426941416277785a4e357633222c227075626b6579223a22303331643638626331613134326536373636623262646662303036636366653133356566326530653265393461626235636635633961623631303437373666626165227d",hex::encode(result.get_bytes()));
    }
}
