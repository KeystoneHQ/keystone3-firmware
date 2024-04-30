use third_party::cryptoxide::{hmac::Hmac, pbkdf2, sha2::Sha512};

pub(crate) fn pbkdf2_sha512(key: &[u8], salt: &[u8], iterations: u32, output: &mut [u8]) {
    let mut hmac = Hmac::new(Sha512::new(), key);
    pbkdf2::pbkdf2(&mut hmac, salt, iterations, output)
}
