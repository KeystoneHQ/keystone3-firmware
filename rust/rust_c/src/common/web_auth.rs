use alloc::{
    format, slice,
    string::{String, ToString},
    vec,
};
use rsa::BigUint;
use {
    hex,
    serde_json::{from_slice, from_value, Value},
    ur_registry::bytes::Bytes,
};

use crate::extract_ptr_with_type;

use super::{
    errors::RustCError,
    types::{ConstPtrUR, PtrBytes, PtrString},
    utils::convert_c_char,
};

use sha1::Sha1;

#[no_mangle]
pub extern "C" fn calculate_auth_code(
    web_auth_data: ConstPtrUR,
    rsa_key_n: PtrBytes,
    rsa_key_n_len: u32,
    rsa_key_d: PtrBytes,
    rsa_key_d_len: u32,
) -> PtrString {
    let crypto_bytes = extract_ptr_with_type!(web_auth_data, Bytes);
    let json_bytes = crypto_bytes.get_bytes();
    let result = match from_slice::<Value>(&json_bytes) {
        Ok(_json) => {
            let data = _json.pointer("/data/data");
            if let Some(_data) = data {
                match from_value::<String>(_data.clone()) {
                    Ok(_hex) => match base64::decode(&_hex) {
                        Ok(_value) => unsafe {
                            let rsa_key_n =
                                slice::from_raw_parts(rsa_key_n, rsa_key_n_len as usize);
                            let rsa_key_d =
                                slice::from_raw_parts(rsa_key_d, rsa_key_d_len as usize);
                            match _calculate_auth_code(&_value, rsa_key_n, rsa_key_d) {
                                Ok(_result) => Ok(_result),
                                Err(_err) => {
                                    Err(RustCError::WebAuthFailed(format!("{}", _err.to_string())))
                                }
                            }
                        },
                        Err(_err) => {
                            Err(RustCError::WebAuthFailed(format!("{}", _err.to_string())))
                        }
                    },
                    Err(_err) => Err(RustCError::WebAuthFailed(format!("{}", _err.to_string()))),
                }
            } else {
                Err(RustCError::WebAuthFailed(format!("invalid json")))
            }
        }
        Err(_err) => Err(RustCError::WebAuthFailed(format!("{}", _err.to_string()))),
    };
    match result {
        Ok(_value) => convert_c_char(_value),
        Err(_err) => convert_c_char("".to_string()),
    }
}

fn _calculate_auth_code(
    data: &[u8],
    rsa_key_n: &[u8],
    rsa_key_d: &[u8],
) -> Result<String, RustCError> {
    let len = data.len();
    let encrypted_data = &data[0..(len - 64)];
    let k1_signature = &data[(len - 64)..];
    let msg_hash = cryptoxide::hashing::sha256(encrypted_data);
    let k1_pub_key =
        hex::decode("0454cc449aad7b4490ca4f395c3eff29fca0a899e289769dc680190e2e07cf9d0d08e52c6b8c096790a63931cbe4dc53cc7efeb1f58a203b03c6d27b57c998ebb7")
            .map_err(|_e| RustCError::WebAuthFailed("recover k1 pub key failed".to_string()))?;
    match keystore::algorithms::secp256k1::verify_signature(k1_signature, &msg_hash, &k1_pub_key) {
        Ok(true) => unsafe {
            let n = BigUint::from_bytes_be(rsa_key_n);
            let d = BigUint::from_bytes_be(rsa_key_d);
            let dummy_p = BigUint::from_bytes_be(&hex::decode("02").unwrap());
            let dummy_q = BigUint::from_bytes_be(&hex::decode("02").unwrap());
            match rsa::RsaPrivateKey::from_components(
                n,
                //default E
                BigUint::from(65537u64),
                d,
                vec![dummy_p, dummy_q],
            ) {
                Ok(_key) => match _key.decrypt(rsa::Oaep::new::<Sha1>(), &encrypted_data) {
                    Ok(_value) => String::from_utf8(_value.clone()).map_err(|_err| {
                        RustCError::WebAuthFailed(format!(
                            "Invalid utf8 hex: {}, {}",
                            hex::encode(_value),
                            _err.to_string()
                        ))
                    }),
                    Err(_err) => Err(RustCError::WebAuthFailed(format!(
                        "RSA decryption failed: {}",
                        _err.to_string()
                    ))),
                },
                Err(_err) => Err(RustCError::WebAuthFailed(format!(
                    "RSA key recovery error: {}",
                    _err.to_string()
                ))),
            }
        },
        _ => Err(RustCError::WebAuthFailed(format!("k1 verify failed"))),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use {base64, hex};

    extern crate std;
    use std::println;

    #[test]
    fn test() {
        let pubkey = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAgKyGG1lElm4ezSOTM7Z+i/NZDIo0SPJRIleURcHJB9AwxT+JBE2ZuzrgOhTTbDEOZq4o5Nox/VDp78ZZ33wUXRggc/YTQRN6Vwj85Ei8OxA3XiK/LA/lfkwulVKSjyakmsJWS3kBubhkA1ESXCsrUqFFZiQZcZf5npLWR1qIuJeXft6aGiYGMVupFu7LBguQ+ForOSScfYQm0G1zsipX8YKGRSNgs1N6JcmIu77y854+xEgq7jOMitBQoFDdBgQylOfAs5cX02ckc2zIbhL/YrsrOvaKa6zh8tqynqvVBcO6YvZz3p2L998QGGu9nZz8j2LQZ7DbrPqL6+npbW5PKwIDAQAB";
        let pk1 = base64::decode(pubkey).unwrap();
        let seckey = "MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCArIYbWUSWbh7NI5Mztn6L81kMijRI8lEiV5RFwckH0DDFP4kETZm7OuA6FNNsMQ5mrijk2jH9UOnvxlnffBRdGCBz9hNBE3pXCPzkSLw7EDdeIr8sD+V+TC6VUpKPJqSawlZLeQG5uGQDURJcKytSoUVmJBlxl/mektZHWoi4l5d+3poaJgYxW6kW7ssGC5D4Wis5JJx9hCbQbXOyKlfxgoZFI2CzU3olyYi7vvLznj7ESCruM4yK0FCgUN0GBDKU58CzlxfTZyRzbMhuEv9iuys69oprrOHy2rKeq9UFw7pi9nPenYv33xAYa72dnPyPYtBnsNus+ovr6eltbk8rAgMBAAECggEAV/4jI3azWa/qrlxCoN2OwoPihJnKUYwsAbEke2Oe5xHvGCRvvZqXo5sKlY2CW31cnMlCu+Wew91ebRMWiKcggd0q7EH+PsVkJzrFPhOjbTyUsHJQi4A/b1QHkyPJh81kSVTWkHwquuemtUHurUr0MGiKveO35p+IG3HMyTTXRGuhMQM2SeiOq/T3gJwhyRKyqQSp09yB2+OoQnHSxeJPWTaGwUVz+WBvkBQDPZ6QYuv0ShDHGPp1eTdRr11SWgvxCUDVhp64Rqr20lhxiy09GxfihwRKfjWcXqZyDBUYddFuS8X2Ke+gKCziISKNdHnjfgfsBMg/BH61W5C7sMHTiQKBgQDczAFE1XpWxj3CXx75UWeU5+2UfFE6x8No1n1B1FEzaWDDMTQjmMGZKRxi4cNbzKFlLOOqLkoN4XNZS4d3RhyKs6Kat4ZJshoDl4p++rKQI0uaeZPEAN0Gi6LPsB11OYzlLADRstzLR8DAoJLAc+OML32msiQhYVr+csYmjqeLFwKBgQCVMHLrmV13Q+8JUZh0fn4dY+kgE6dZOx3R+U84aJy64hZH5a6sV7FgNchnfdYEHp4UOeVxD1MRu7wIknpAgSfWL96d+3KEbKYBkPB+W8aFGbf/TXcUKAOK7ewrGILGh9iyTogtoIzBNvD4usLczb17uxHTYL8AT3DFbuobesEZDQKBgAeKN+jf2WOpNWaz1NzyuLTPDr/pVuTI+ZVU7cYXuAYnbRR9U6h7iPDwBHYj+4XhAQnxdHzVQ9Yhgszj1WZvUH40EDN0XPObT2QcArC/YuWlLunBiRhCZ0nX7uFkxDzCkFsrG4QNtZiDhMOPODO2QWkCHipZHv680lyqErYyXOpVAoGAX1j4iYSaDKhZNC527jUNqwD4tGx7LvcuRs08iUOqr3HhZY/wg+sDzQZyj1oIFtfuleSutARkRdgjA6OCCInDCQvNDlDuzp2VtziHy2oiRVy7kKpUqYgtQ3Rt1Nk0c2fM4aB5Phf2/WF8vqq18WJ4cISNjves+qjK3RPXATsCAF0CgYA3ESGih7R3/OyXoQPvutUiR1xkeo58LTSNTZK/NDx523IYLOsy8QEqQ88gpn5rChDZHlD0yhJyghYgDRvyjmdMVyxb0PIblDk5iuQpVwQqsNGPzZa+bZuJRV1d8SNLvkFm2JlssXSZ3cOIQwLfcq2g+4U/1PHu214j3x6EW+w7aQ==";
        let sk1 = base64::decode(seckey).unwrap();
        //308204bc020100300d06092a864886f70d0101010500048204a6308204a2020100028201010080ac861b5944966e1ecd239333b67e8bf3590c8a3448f25122579445c1c907d030c53f89044d99bb3ae03a14d36c310e66ae28e4da31fd50e9efc659df7c145d182073f61341137a5708fce448bc3b10375e22bf2c0fe57e4c2e9552928f26a49ac2564b7901b9b8640351125c2b2b52a1456624197197f99e92d6475a88b897977ede9a1a2606315ba916eecb060b90f85a2b39249c7d8426d06d73b22a57f18286452360b3537a25c988bbbef2f39e3ec4482aee338c8ad050a050dd06043294e7c0b39717d36724736cc86e12ff62bb2b3af68a6bace1f2dab29eabd505c3ba62f673de9d8bf7df10186bbd9d9cfc8f62d067b0dbacfa8bebe9e96d6e4f2b02030100010282010057fe232376b359afeaae5c42a0dd8ec283e28499ca518c2c01b1247b639ee711ef18246fbd9a97a39b0a958d825b7d5c9cc942bbe59ec3dd5e6d131688a72081dd2aec41fe3ec564273ac53e13a36d3c94b072508b803f6f54079323c987cd644954d6907c2abae7a6b541eead4af430688abde3b7e69f881b71ccc934d7446ba131033649e88eabf4f7809c21c912b2a904a9d3dc81dbe3a84271d2c5e24f593686c14573f9606f9014033d9e9062ebf44a10c718fa75793751af5d525a0bf10940d5869eb846aaf6d258718b2d3d1b17e287044a7e359c5ea6720c151875d16e4bc5f629efa0282ce221228d7479e37e07ec04c83f047eb55b90bbb0c1d38902818100dccc0144d57a56c63dc25f1ef9516794e7ed947c513ac7c368d67d41d451336960c331342398c199291c62e1c35bcca1652ce3aa2e4a0de173594b8777461c8ab3a29ab78649b21a03978a7efab290234b9a7993c400dd068ba2cfb01d75398ce52c00d1b2dccb47c0c0a092c073e38c2f7da6b22421615afe72c6268ea78b1702818100953072eb995d7743ef095198747e7e1d63e92013a7593b1dd1f94f38689cbae21647e5aeac57b16035c8677dd6041e9e1439e5710f5311bbbc08927a408127d62fde9dfb72846ca60190f07e5bc68519b7ff4d771428038aedec2b1882c687d8b24e882da08cc136f0f8bac2dccdbd7bbb11d360bf004f70c56eea1b7ac1190d028180078a37e8dfd963a93566b3d4dcf2b8b4cf0ebfe956e4c8f99554edc617b806276d147d53a87b88f0f0047623fb85e10109f1747cd543d62182cce3d5666f507e341033745cf39b4f641c02b0bf62e5a52ee9c18918426749d7eee164c43cc2905b2b1b840db5988384c38f3833b64169021e2a591efebcd25caa12b6325cea550281805f58f889849a0ca859342e76ee350dab00f8b46c7b2ef72e46cd3c8943aaaf71e1658ff083eb03cd06728f5a0816d7ee95e4aeb4046445d82303a3820889c3090bcd0e50eece9d95b73887cb6a22455cbb90aa54a9882d43746dd4d9347367cce1a0793e17f6fd617cbeaab5f1627870848d8ef7acfaa8cadd13d7013b02005d028180371121a287b477fcec97a103efbad522475c647a8e7c2d348d4d92bf343c79db72182ceb32f1012a43cf20a67e6b0a10d91e50f4ca12728216200d1bf28e674c572c5bd0f21b9439398ae42957042ab0d18fcd96be6d9b89455d5df1234bbe4166d8996cb17499ddc3884302df72ada0fb853fd4f1eedb5e23df1e845bec3b69
        //30820122300d06092a864886f70d01010105000382010f003082010a028201010080ac861b5944966e1ecd239333b67e8bf3590c8a3448f25122579445c1c907d030c53f89044d99bb3ae03a14d36c310e66ae28e4da31fd50e9efc659df7c145d182073f61341137a5708fce448bc3b10375e22bf2c0fe57e4c2e9552928f26a49ac2564b7901b9b8640351125c2b2b52a1456624197197f99e92d6475a88b897977ede9a1a2606315ba916eecb060b90f85a2b39249c7d8426d06d73b22a57f18286452360b3537a25c988bbbef2f39e3ec4482aee338c8ad050a050dd06043294e7c0b39717d36724736cc86e12ff62bb2b3af68a6bace1f2dab29eabd505c3ba62f673de9d8bf7df10186bbd9d9cfc8f62d067b0dbacfa8bebe9e96d6e4f2b0203010001
        println!("{}", hex::encode(pk1));
        println!("{}", hex::encode(sk1));
    }

    #[test]
    fn test_calculate_auth_code() {
        let data = hex::decode("7837ca892b090678e152449a9f9a052b6806a753b9662f16fb72757e06a842835b27a2a2bca9f625a8cd55936cef232231e592d3a574a78909cedcb829e18f690935aba889c94b5c3210b38328562fb1b21053b18e361d4eb7f3fa1551a32a989cbcc56a551150d5dd98da08866c9b60fab235af073ff0aa7eeedc6df1d01d0ce3e3620f2103f6df5b02ab751fae9449c0f80dfdddb55cd12254009834ed9b79f3f5f205c5ae50bc5769795ff393e1c9596476856b11ee6116cab92691fbe79e09ac43af917cf7abf3e15c0adb6a40746960bd725fe332055da73e9f7202bf0cf92feb3c8dc0914cf95e61bec4484fed02122ec4531e64851a8d3f79808bde42dcf82f9e1ce2d922cbac270b696b32bb49f1394ab9d223f5f29f4b6e42b7247f748f49d8574980429d75e4bb72c629fbf787c1a54e649b7572570384709d970d0dcdcf84261da4dae92daa75f80d9b4840623a987234f1ce526b608c480bf664a0973f669f9e43253dfc9542e421e118c6d5ca152b96325da1f7d30577f1b8945b35089a69fdf07c7da57feef525e877cc0eef91db293bc86bd3e4ec0398f14fea6efa5040365b83ba934d2b54d3e33ee8ad3bd7b584bf048fcd8ebf8d842848280275e2964d7549cc284561796c8aec12f9580504fb26ecb4c2ec9f8c6c7f7afdedc2e2958efd6c9cb2b33c6f4208269572213f919f903a0ecde401e7631792e644b1b681f0208bcb2becb7103c65b80d77e440b5ffe368c0d8229af7b3e4358db5f5b133d6a6947b555975fa10ef9baec86c79e3d7f1a8b7467beed23d4c60").unwrap();
        let rsa_key_n = hex::decode("85a75b4d50a5051f936fdefcb0534651abbe24414b5ca6299fdfa42b422a3c1bea1d342bd922ce35fe0ac374525037cd038ea70862e0b9aa78aab8c40cfa763e3d9da8dd1470a98a75b446678ff60196a89959b0065aef6e767a55fab61ab040f0edf1fc0046e9ba302977b786458590d04547c1596932fac01f5124002937d778c665de5e09108678cff4eb0958eb7a039ed62849e71bea3c71b426af92b61b14935ebd44ed919fff0bdb1923153ab6fcfc25807a753e2c8cf61f1c7a99af28664e05d83efd15ba0b4ece3bd0d06638538c2373775904d1479122b1f8f9c47218a397573cd2a86d9e603e5e4790e6439d2b0d7da786a268d776218eba7e47cee4e09859ded98bd991902941030aa1dc0541f646873eefe909eca303420b944404714f6ccacde1cf433c816b277e2305fcceac9920650cc86dd6ba995d89605b881b5bd6350d9ba47e870e8dc1048ab4fb9f259e55d34917ddfc85eb87448ae8ce67e5c17265df53adb119af4678bd75c71f5926faa70e3a745fdb553b74e072d732857acf4c65670ab43fcb3ccf9e035477a13c5c3bc544ca0cb85c4e0213414db676e9a294c3f5b1272bb65d92572de477fa49d96781478b11c54bbcfa8f8136e91f901f2be9bb2be2b67fafae67ac3496daf9990dee99fe47666d4e94d9b822233505e55566b1050769445b83f3b50eeb4c652e15fcc36b3efcbee7100bcb").unwrap();
        let rsa_key_d = hex::decode("7b84a25d7e953379ee112a7c4f1c76d54daa48e94b1391c0d02ea8130e6fe7e34866baeb771db283b5870f01767d0a44deefbc3084e8f8cdb1fcdd0ada8fb22d6d56425426eac3d4e158bce442e870a962a16e80be2ed62e3932703966889db136b840b5cb2fbe1bc31cdfb5ed525cd0eb99cb69c60ef1fd4906e83f2422f3a79fc7832791b240e275ce82a3a09cdbea84640bb96a2b3ae22c004d249f882fe2310ecd348ffa1a664a1813e15449ef5c63cb4781482b084db5ec53ee46e3b73bd7e86d805f1e18313af816a2b3259602a23bd61b8505f0ecd47e2e195a66c2054bfe0c04a4826f03f7a52c9e86f743c6e2fe81d649b7a46ace0d2f902f8c3b21742c7aa439fef71626879e058450f8c74fb910e316b7b09eba1cdedf76a57f854dc95260e200767a0f1fd765fb31868bc3d649b417687c81a926fdd77c754d7a099d25d4d41de594ed983a852cd52b4820fad9bd7e00838d05d9a53f33ffdb03c1eb86240647e5ceabdd66b526eaf8b67e1341d6085c6a01100e9a6ad25842830c3bf10a08520ca9aa5aa2f92bd568f28c795ae9785042ae6bbbf8ceab14710ec111a76019b24d609159835a4c7880af54934fa67919738a6579079f8b39df45912bd4d2fb9967b7f51d7756a34de6778dda602a0be55a0779e5ee71e96131fa7ef6c6645d20fe6927a6f60d3781b6bfdb896f4c1c31781dc62ead5c2081d759").unwrap();
        let result = _calculate_auth_code(&data, &rsa_key_n, &rsa_key_d).unwrap();
        assert_eq!("EX8Z9MWH", result);
    }
}
