use third_party::cryptoxide::{hmac::Hmac, mac::Mac, pbkdf2::pbkdf2, sha2::Sha256};

pub fn hmac_sha512(key: &[u8], data: &[u8]) -> [u8; 64] {
    let digest = third_party::cryptoxide::sha2::Sha512::new();
    let mut hmac = third_party::cryptoxide::hmac::Hmac::new(digest, key);
    hmac.input(data);
    let mut output = [0u8; 64];
    hmac.raw_result(&mut output);
    output
}

pub fn hkdf(password: &[u8], salt: &[u8], iterations: u32) -> [u8; 32] {
    let mut output = [0u8; 32];
    pbkdf2(
        &mut Hmac::new(Sha256::new(), password),
        salt,
        iterations,
        &mut output,
    );
    output
}

pub fn hkdf64(password: &[u8], salt: &[u8], iterations: u32) -> [u8; 64] {
    let mut output = [0u8; 64];
    pbkdf2(
        &mut Hmac::new(Sha256::new(), password),
        salt,
        iterations,
        &mut output,
    );
    output
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;
    use third_party::hex;

    #[test]
    fn test_hkdf_should_work() {
        let password = [0u8; 32];
        let salt = [1u8; 32];
        let result = hkdf(&password, &salt, 700);

        let result_string = hex::encode(result);
        let expected =
            "6aefec5dba55456b76af351156665c5e4e0939d09426dff80f93e0960ba2fbd0".to_string();
        assert_eq!(result_string, expected);
    }
}
