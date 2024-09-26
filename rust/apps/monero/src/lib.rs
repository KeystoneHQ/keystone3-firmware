#![no_std]
#![feature(error_in_core)]
extern crate alloc;

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;
    use alloc::format;
    use third_party::hex;
    use keystore::algorithms::crypto::hmac_sha512;
    use third_party::ed25519_bip32_core::{XPrv, DerivationScheme, XPub, XPRV_SIZE};
    use keystore::algorithms::ed25519::slip10_ed25519::{get_private_key_by_seed, get_public_key_by_seed};
    use keystore::algorithms::ed25519::bip32_ed25519;
    use third_party::cryptoxide::hashing::sha2::Sha512;
    use third_party::bitcoin::{script, Network, PrivateKey, Script, base58};
    use third_party::cryptoxide::sha3::Keccak256;
    use third_party::cryptoxide::digest::Digest;
    use alloc::vec::Vec;
    use curve25519_dalek::edwards::{CompressedEdwardsY, EdwardsPoint};
    use curve25519_dalek::scalar::Scalar;
    use base58_monero::{encode, decode, Error};

    #[test]
    fn test_monero_add_generator() {
        // BIP39 Mnemonic: key stone key stone key stone key stone key stone key stone key stone success
        let seed = hex::decode("45a5056acbe881d7a5f2996558b303e08b4ad1daffacf6ffb757ff2a9705e6b9f806cffe3bd90ff8e3f8e8b629d9af78bcd2ed23e8c711f238308e65b62aa5f0").unwrap();
        let path = "m/44'/128'/0'/0/0".to_string();

        // raw_private_key
        let key = keystore::algorithms::secp256k1::get_private_key_by_seed(
        &seed,
        &path.to_string(),
        ).unwrap();
        let raw_private_key = PrivateKey::new(key, Network::Bitcoin);

        // raw_secret_spend_key
        let mut hasher = Keccak256::new();
        hasher.input(&raw_private_key.to_bytes());
        let mut raw_secret_spend_key = [0u8; 32];
        hasher.result(&mut raw_secret_spend_key);

        // secret_spend_key
        let secret_spend_key = sc_reduce32(&raw_secret_spend_key);

        // secret_view_key
        let mut hasher = Keccak256::new();
        hasher.input(&secret_spend_key);
        let mut secret_view_key = [0u8; 32];
        hasher.result(&mut secret_view_key);
        secret_view_key = sc_reduce32(&secret_view_key);

        // publicSpendKey
        let scalar = Scalar::from_bytes_mod_order(secret_spend_key);
        let public_spend_key: CompressedEdwardsY = EdwardsPoint::mul_base(&scalar).compress();

        // publicViewKey
        let scalar = Scalar::from_bytes_mod_order(secret_view_key);
        let public_view_key: CompressedEdwardsY = EdwardsPoint::mul_base(&scalar).compress();

        // address
        let prefix = "12";
        let res_hex = format!("{}{}{}", prefix, hex::encode(public_spend_key.as_bytes()), hex::encode(public_view_key.as_bytes()));
        let mut hasher = Keccak256::new();
        hasher.input(&hex::decode(res_hex.clone()).unwrap());
        let mut checksum = [0u8; 32];
        hasher.result(&mut checksum);
        let res_hex = format!("{}{}", res_hex, hex::encode(&checksum[0..4]));
        let address = encode(&hex::decode(res_hex).unwrap()).unwrap();

        // result
        assert_eq!(hex::encode(raw_private_key.to_bytes()), "66ec3ba491849c927c9be0bd8387b0a7215c61c69854d53f6585630d4557e752");
        assert_eq!(hex::encode(raw_secret_spend_key), "62cf06d75043c5bedb51d3070297b164e4d8ded84f0a3d65a542915475ca6fee");
        assert_eq!(hex::encode(secret_spend_key), "6c3895c1dfd7c3ed22be481ed5ec7f40e3d8ded84f0a3d65a542915475ca6f0e");
        assert_eq!(hex::encode(secret_view_key), "17921dbd51b4a1af0b4049bc13dc7048ace1dcd8be9b8669de95b8430924ea09");
        assert_eq!(hex::encode(public_spend_key.as_bytes()), "12f38162635cf3aecf081d96158022b2a1517993100e54d62b17057f2443e749");
        assert_eq!(hex::encode(public_view_key.as_bytes()), "e18a5360ae4b2ff71bf91c5a626e14fc2395608375b750526bc0962ed27237a1");
        assert_eq!(address, "42LmACF1Ce6WEs5w1nNsoPWswJQzcRdZucphf75q1bzvDMjq1vJ2iJziLGdTn1JbcPjB4iiEagCMuEnazTfaryuQKG7sw7S");
}

fn load_3(input: &[u8]) -> i64 {
    (input[0] as i64) | ((input[1] as i64) << 8) | ((input[2] as i64) << 16)
}

fn load_4(input: &[u8]) -> i64 {
    (input[0] as i64) | ((input[1] as i64) << 8) | ((input[2] as i64) << 16) | ((input[3] as i64) << 24)
}

fn sc_reduce32(s: &[u8]) -> [u8; 32] {
    let mut result = [0u8; 32];

    let mut s0 = 2097151 & load_3(&s[0..]);
    let mut s1 = 2097151 & (load_4(&s[2..]) >> 5);
    let mut s2 = 2097151 & (load_3(&s[5..]) >> 2);
    let mut s3 = 2097151 & (load_4(&s[7..]) >> 7);
    let mut s4 = 2097151 & (load_4(&s[10..]) >> 4);
    let mut s5 = 2097151 & (load_3(&s[13..]) >> 1);
    let mut s6 = 2097151 & (load_4(&s[15..]) >> 6);
    let mut s7 = 2097151 & (load_3(&s[18..]) >> 3);
    let mut s8 = 2097151 & load_3(&s[21..]);
    let mut s9 = 2097151 & (load_4(&s[23..]) >> 5);
    let mut s10 = 2097151 & (load_3(&s[26..]) >> 2);
    let mut s11 = load_4(&s[28..]) >> 7;
    let mut s12 = 0i64;

    let mut carry0;
    let mut carry1;
    let mut carry2;
    let mut carry3;
    let mut carry4;
    let mut carry5;
    let mut carry6;
    let mut carry7;
    let mut carry8;
    let mut carry9;
    let mut carry10;
    let mut carry11;

    // Perform the carries similar to the C version
    carry0 = (s0 + (1 << 20)) >> 21; s1 += carry0; s0 -= carry0 << 21;
    carry2 = (s2 + (1 << 20)) >> 21; s3 += carry2; s2 -= carry2 << 21;
    carry4 = (s4 + (1 << 20)) >> 21; s5 += carry4; s4 -= carry4 << 21;
    carry6 = (s6 + (1 << 20)) >> 21; s7 += carry6; s6 -= carry6 << 21;
    carry8 = (s8 + (1 << 20)) >> 21; s9 += carry8; s8 -= carry8 << 21;
    carry10 = (s10 + (1 << 20)) >> 21; s11 += carry10; s10 -= carry10 << 21;

    carry1 = (s1 + (1 << 20)) >> 21; s2 += carry1; s1 -= carry1 << 21;
    carry3 = (s3 + (1 << 20)) >> 21; s4 += carry3; s3 -= carry3 << 21;
    carry5 = (s5 + (1 << 20)) >> 21; s6 += carry5; s5 -= carry5 << 21;
    carry7 = (s7 + (1 << 20)) >> 21; s8 += carry7; s7 -= carry7 << 21;
    carry9 = (s9 + (1 << 20)) >> 21; s10 += carry9; s9 -= carry9 << 21;
    carry11 = (s11 + (1 << 20)) >> 21; s12 += carry11; s11 -= carry11 << 21;

    s0 += s12 * 666643;
    s1 += s12 * 470296;
    s2 += s12 * 654183;
    s3 -= s12 * 997805;
    s4 += s12 * 136657;
    s5 -= s12 * 683901;
    s12 = 0;

    carry0 = s0 >> 21; s1 += carry0; s0 -= carry0 << 21;
    carry1 = s1 >> 21; s2 += carry1; s1 -= carry1 << 21;
    carry2 = s2 >> 21; s3 += carry2; s2 -= carry2 << 21;
    carry3 = s3 >> 21; s4 += carry3; s3 -= carry3 << 21;
    carry4 = s4 >> 21; s5 += carry4; s4 -= carry4 << 21;
    carry5 = s5 >> 21; s6 += carry5; s5 -= carry5 << 21;
    carry6 = s6 >> 21; s7 += carry6; s6 -= carry6 << 21;
    carry7 = s7 >> 21; s8 += carry7; s7 -= carry7 << 21;
    carry8 = s8 >> 21; s9 += carry8; s8 -= carry8 << 21;
    carry9 = s9 >> 21; s10 += carry9; s9 -= carry9 << 21;
    carry10 = s10 >> 21; s11 += carry10; s10 -= carry10 << 21;
    carry11 = s11 >> 21; s12 += carry11; s11 -= carry11 << 21;

    s0 += s12 * 666643;
    s1 += s12 * 470296;
    s2 += s12 * 654183;
    s3 -= s12 * 997805;
    s4 += s12 * 136657;
    s5 -= s12 * 683901;

    carry0 = s0 >> 21; s1 += carry0; s0 -= carry0 << 21;
    carry1 = s1 >> 21; s2 += carry1; s1 -= carry1 << 21;
    carry2 = s2 >> 21; s3 += carry2; s2 -= carry2 << 21;
    carry3 = s3 >> 21; s4 += carry3; s3 -= carry3 << 21;
    carry4 = s4 >> 21; s5 += carry4; s4 -= carry4 << 21;
    carry5 = s5 >> 21; s6 += carry5; s5 -= carry5 << 21;
    carry6 = s6 >> 21; s7 += carry6; s6 -= carry6 << 21;
    carry7 = s7 >> 21; s8 += carry7; s7 -= carry7 << 21;
    carry8 = s8 >> 21; s9 += carry8; s8 -= carry8 << 21;
    carry9 = s9 >> 21; s10 += carry9; s9 -= carry9 << 21;
    carry10 = s10 >> 21; s11 += carry10; s10 -= carry10 << 21;

    result[0] = (s0 >> 0) as u8;
    result[1] = (s0 >> 8) as u8;
    result[2] = ((s0 >> 16) | (s1 << 5)) as u8;
    result[3] = (s1 >> 3) as u8;
    result[4] = (s1 >> 11) as u8;
    result[5] = ((s1 >> 19) | (s2 << 2)) as u8;
    result[6] = (s2 >> 6) as u8;
    result[7] = ((s2 >> 14) | (s3 << 7)) as u8;
    result[8] = (s3 >> 1) as u8;
    result[9] = (s3 >> 9) as u8;
    result[10] = ((s3 >> 17) | (s4 << 4)) as u8;
    result[11] = (s4 >> 4) as u8;
    result[12] = (s4 >> 12) as u8;
    result[13] = ((s4 >> 20) | (s5 << 1)) as u8;
    result[14] = (s5 >> 7) as u8;
    result[15] = ((s5 >> 15) | (s6 << 6)) as u8;
    result[16] = (s6 >> 2) as u8;
    result[17] = (s6 >> 10) as u8;
    result[18] = ((s6 >> 18) | (s7 << 3)) as u8;
    result[19] = (s7 >> 5) as u8;
    result[20] = (s7 >> 13) as u8;
    result[21] = (s8 >> 0) as u8;
    result[22] = (s8 >> 8) as u8;
    result[23] = ((s8 >> 16) | (s9 << 5)) as u8;
    result[24] = (s9 >> 3) as u8;
    result[25] = (s9 >> 11) as u8;
    result[26] = ((s9 >> 19) | (s10 << 2)) as u8;
    result[27] = (s10 >> 6) as u8;
    result[28] = ((s10 >> 14) | (s11 << 7)) as u8;
    result[29] = (s11 >> 1) as u8;
    result[30] = (s11 >> 9) as u8;
    result[31] = (s11 >> 17) as u8;

    result
    }
}