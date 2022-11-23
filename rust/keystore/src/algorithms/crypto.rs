use third_party::cryptoxide::mac::Mac;

pub fn hmac_sha512(key: &[u8], data: &[u8]) -> [u8; 64] {
    let digest = third_party::cryptoxide::sha2::Sha512::new();
    let mut hmac = third_party::cryptoxide::hmac::Hmac::new(digest, key);
    hmac.input(data);
    let mut output = [0u8; 64];
    hmac.raw_result(&mut output);
    output
}
