use crate::errors::{KeystoreError, Result};

use alloc::string::ToString;
use alloc::vec::Vec;
use rand_chacha::ChaCha20Rng;
use rand_core::{OsRng, SeedableRng};
use zeroize::Zeroize;

use num_bigint_dig::traits::ModInverse;
use num_bigint_dig::BigUint;
use rsa::pss::SigningKey;
use rsa::signature::{RandomizedDigestSigner, RandomizedSigner, SignatureEncoding};
use rsa::{rand_core, PublicKeyParts, RsaPrivateKey};
use sha2;
use sha2::{Digest, Sha256};

pub const MODULUS_LENGTH: usize = 4096;
pub const PRIME_LENGTH_IN_BYTE: usize = MODULUS_LENGTH / 8 / 2;
pub const MODULUS_LENGTH_IN_BYTE: usize = MODULUS_LENGTH / 8;
pub const SECRET_LENGTH_IN_BYTE: usize = PRIME_LENGTH_IN_BYTE * 2 + MODULUS_LENGTH_IN_BYTE * 2;

fn get_rsa_seed(seed: &[u8]) -> Result<[u8; 32]> {
    let mut intermediate;
    let mut hash = seed;
    for _ in 0..2 {
        intermediate = Sha256::digest(hash);
        hash = &intermediate[..];
    }
    let result: [u8; 32] = hash.try_into().map_err(|_| {
        KeystoreError::GenerateSigningKeyError("get_rsa_master_key failed".to_string())
    })?;
    Ok(result)
}

pub fn get_rsa_secret_from_seed(seed: &[u8]) -> Result<RsaPrivateKey> {
    // bip39 seed length is 64, slip39 seed length is 16 or 32
    let seed_len = seed.len();
    if !matches!(seed.len(), 16 | 32 | 64) {
        return Err(KeystoreError::GenerateSigningKeyError(format!(
            "Invalid seed length: {seed_len}, expected 16, 32, or 64 bytes"
        )));
    }
    let mut rsa_seed = get_rsa_seed(seed)?;
    let mut rng = ChaCha20Rng::from_seed(rsa_seed);
    rsa_seed.zeroize();
    let private_key = RsaPrivateKey::new(&mut rng, MODULUS_LENGTH).map_err(|e| {
        KeystoreError::GenerateSigningKeyError(format!("generate rsa private key failed: {e}"))
    })?;
    Ok(private_key)
}

pub fn sign_message(
    data: &[u8],
    p: &[u8],
    q: &[u8],
    signing_option: &SigningOption,
) -> Result<Vec<u8>> {
    let private_key = build_rsa_private_key_from_primes(p, q)?;
    match signing_option {
        SigningOption::Transaction { salt_len } | SigningOption::DataItem { salt_len } => {
            let parsed_salt_len: usize = (*salt_len)
                .try_into()
                .map_err(|_| KeystoreError::RSASignError)?;
            let signing_key = SigningKey::<Sha256>::new_with_salt_len(private_key, parsed_salt_len);
            let mut digest = sha2::Sha256::new();
            digest.update(data);
            let signature = signing_key.sign_digest_with_rng(&mut OsRng, digest);
            Ok(Vec::from(signature.to_bytes()))
        }
        SigningOption::Message { salt_len } => {
            let parsed_salt_len: usize = (*salt_len)
                .try_into()
                .map_err(|_| KeystoreError::RSASignError)?;
            let signing_key = SigningKey::<Sha256>::new_with_salt_len(private_key, parsed_salt_len);
            let mut digest = sha2::Sha256::new();
            digest.update(data);
            let hash = digest.finalize().to_vec();
            let result = signing_key.sign_with_rng(&mut OsRng, &hash);
            Ok(Vec::from(result.to_bytes()))
        }
    }
}

pub fn build_rsa_private_key_from_primes(p: &[u8], q: &[u8]) -> Result<RsaPrivateKey> {
    let p_len = p.len();
    let q_len = q.len();
    if p_len != PRIME_LENGTH_IN_BYTE {
        return Err(KeystoreError::GenerateSigningKeyError(format!(
            "Invalid prime P length: {p_len}, expected {PRIME_LENGTH_IN_BYTE} bytes"
        )));
    }

    if q_len != PRIME_LENGTH_IN_BYTE {
        return Err(KeystoreError::GenerateSigningKeyError(format!(
            "Invalid prime Q length: {q_len}, expected {PRIME_LENGTH_IN_BYTE} bytes",
        )));
    }

    let n = BigUint::from_bytes_be(p) * BigUint::from_bytes_be(q);
    let p = BigUint::from_bytes_be(p);
    let q = BigUint::from_bytes_be(q);
    // e = 65537
    let e = BigUint::from_bytes_be(&[0x01, 0x00, 0x01]);
    let d = e
        .clone()
        .mod_inverse((p.clone() - 1u8) * (q.clone() - 1u8))
        .ok_or_else(|| {
            KeystoreError::GenerateSigningKeyError(
                "failed to calculate rsa private key".to_string(),
            )
        })?;
    let private_key = RsaPrivateKey::from_components(n, e, d.to_biguint().unwrap(), vec![p, q])
        .map_err(|_| {
            KeystoreError::GenerateSigningKeyError("failed to compose rsa signing key".to_string())
        })?;
    Ok(private_key)
}

pub fn get_rsa_pubkey_by_seed(seed: &[u8]) -> Result<Vec<u8>> {
    let private_key = get_rsa_secret_from_seed(seed)?;
    Ok(private_key.to_public_key().n().to_bytes_be())
}

#[derive(Clone, Copy)]
pub enum SigningOption {
    Transaction { salt_len: i32 },
    DataItem { salt_len: i32 },
    Message { salt_len: i32 },
}

#[cfg(test)]
mod tests {
    use super::*;
    use bitcoin::hex::DisplayHex;
    use hex;
    use rsa::pss::{Signature as PssSignature, VerifyingKey};
    use rsa::signature::Verifier;

    // random quick seeds:
    // Seed: ffa41e446bcd9aa4d245607da395a16f, Time: 8.856716042s
    // Seed: ffce250799aa9e495fe19b4f8d13bd3d, Time: 10.709974083s
    // Seed: ed0b92515425e600d469253af4f4ba4e, Time: 7.341941458s
    // Seed: 71e8b47b48f5041e4f883980815db6e6, Time: 7.613651792s
    // Seed: 2f1986623bdc5d4f908e5be9d6fa00ec, Time: 6.1233855s
    // Seed: 72cc31ba74feb35327da05247045c978, Time: 7.62863725s
    // Seed: b87ff97df474c47beb0cfee2fa38394b, Time: 6.754611291s
    // Seed: ee21b42aedcbbd035f4b17d4d14c95bd, Time: 6.861722958s

    #[test]
    fn test_private_key_recover() {
        let p = hex::decode("F1FC92541273845B7B55D125B99839306D6815CCF905AB80DAFF13794F40616F8653C3356E25EC4FB899E12B3147A66DDB5C2B8DAF8EB8A72909709D05E69C39C16742AFDCAE9899D97D8619BEE42342DEABC75B30C2673D6F4C981EB17C00E11B2621ED89F46772132ED56907027C8E4B2AE5E3D86702EE0D8060ED7E143C60793FCACD61DC31D3021637E13F0724E66BAF92AEB321620136F4357C365CF6A7EC96FEAA0428B7BFAC7C82D6E35FECAF4B4C0DCD8531E4AC5C1DB29296E5DBE557AA1BE4B9DA853AE4543BC8254BB77FDABC617434F23CAE4B08B68C9A5C467EF46198F1CB76702D82CB2A4DD7AA29DCEF478F9DC5FEB8B43EDDB5C5683CA027").unwrap();
        let q = hex::decode("ED7D44523FA5FE95C1D084261329296CA8E55CB26D9F6742BE0ECD9996F1AACAA22B5ACD1208A118E1B7DA83D9052CC76B67966F616CEB02E33CB0D2D9A4E5B4CF8E0EE68272D528F5A5AD2D75EE275C9CC3CD061C17BC9517606A74D749B9B1156F58647645326130109E1D32C00472026794CE45DD2225C4A93B09B0BF05D0369AFF2692038D040553AA7EA059E6610A084E34A20A3B9EBFF2BB586B78AAE8EBC15621F8E7BCB2FA2CD6B51A63D42AEBAD67B17C200A7515D89F8FDA4380E3815BBAE1A4BDDA1B6CEDDEEEAF68BD27CE38A5EC46AC43C77F0C7392C4196260992DF9393965676469EE8C34F2711AF6C88E2D96702857EB08F9CC65CA361FA5").unwrap();
        let seed = hex::decode("2f1986623bdc5d4f908e5be9d6fa00ec").unwrap();
        let result = get_rsa_secret_from_seed(seed.as_slice()).unwrap();
        let p = p.to_upper_hex_string();
        let q = q.to_upper_hex_string();
        assert_eq!(result.primes()[0].to_bytes_be().to_upper_hex_string(), p);
        assert_eq!(result.primes()[1].to_bytes_be().to_upper_hex_string(), q);
    }

    #[test]
    fn test_get_rsa_secret_from_seed_valid_lengths() {
        let seed_16 = hex::decode("2f1986623bdc5d4f908e5be9d6fa00ec").unwrap();
        let result_16 = get_rsa_secret_from_seed(seed_16.as_slice()).unwrap();
        assert_eq!(result_16.size(), MODULUS_LENGTH_IN_BYTE);

        let error_seed = hex::decode("0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f4041").unwrap();
        let result = get_rsa_secret_from_seed(error_seed.as_slice()).unwrap_err();
        assert_eq!(
            result.to_string(),
            "GenerateSigningKeyError: Invalid seed length: 65, expected 16, 32, or 64 bytes"
        );
    }

    #[test]
    fn test_get_rsa_secret_from_seed_invalid_inputs() {
        // Test empty seed
        let empty_seed = [];
        let result = get_rsa_secret_from_seed(&empty_seed).unwrap_err();
        assert!(result
            .to_string()
            .contains("Invalid seed length: 0, expected 16, 32, or 64 bytes"));

        // Test invalid length (15 bytes)
        let invalid_seed = hex::decode("0102030405060708090a0b0c0d0e0f").unwrap();
        let result = get_rsa_secret_from_seed(invalid_seed.as_slice()).unwrap_err();
        assert!(result
            .to_string()
            .contains("Invalid seed length: 15, expected 16, 32, or 64 bytes"));

        // Test invalid length (33 bytes)
        let invalid_seed_33 =
            hex::decode("0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f2021")
                .unwrap();
        let result = get_rsa_secret_from_seed(invalid_seed_33.as_slice()).unwrap_err();
        assert!(result
            .to_string()
            .contains("Invalid seed length: 33, expected 16, 32, or 64 bytes"));

        // Test invalid length (65 bytes)
        let invalid_seed_65 = hex::decode("0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f4041").unwrap();
        let result = get_rsa_secret_from_seed(invalid_seed_65.as_slice()).unwrap_err();
        assert!(result
            .to_string()
            .contains("Invalid seed length: 65, expected 16, 32, or 64 bytes"));
    }

    #[test]
    fn test_sign_message() {
        let p = hex::decode("EA8E3612876ED1433E5909D25F699F7C5D4984CF0D2F268B185141F0E29CE65237EAD8236C94A0A9547F1FEABD4F54399C626C0FB813249BC74A3082F8637A9E9A3C9D4F6E1858ED29770FE95418ED66F07A9F2F378D43D31ED37A0E6942727394A87B93540E421742ADE9630E26500FD2C01502DF8E3F869C70DAA97D4583048DD367E2977851052F6A991182318015557EC81B58E81B668E3A715212C807A1D7835FCB2B87B5DEFAC0948B220D340D6B2DA0DCFC7123DE1F1424F6F5B8EAFA719B3DE8B9B6FEC196E2E393CE30204267A586625541C7B1433F8FA7873B51B3E65462831BF34A4E5912297A06B2E91B31657DFA3CCFDB5F94D438D9904CFD27").unwrap();
        let q = hex::decode("E360BFD757FF6FCF844AF226DCA7CFBD353A89112079C9C5A17C4F354DE0B1BE38BBFD73EAA77C4E2FFC681A79CEC8C8E79A5A00E32113A77748F435717BE6AD04AEF473BCE05DC3B742AAB853C02C565847133AFFD451B472B13031300978606F74BE8761A69733BEF8C2CCD6F396A0CCE23CDC73A8FF4609F47C18FE4626B788C4BFB73B9CF10BC5D6F80E9B9847530973CF5212D8EB142EAA155D774417D7BF89E1F229472926EA539AC3BAF42CF63EF18A6D85915727E9A77B4EA31B577B1E4A35C40CCCE72F5ECE426709E976DAEDBE7B76291F89EB85903182035CA98EB156563E392D0D1E427C59657B9EDF1DDB049BBB9620B881D8715982AD257D29").unwrap();
        let data = b"hello world";
        let salt_len = 32;
        let sig = sign_message(
            data,
            p.as_slice(),
            q.as_slice(),
            &SigningOption::Message { salt_len },
        )
        .unwrap();

        let sk = build_rsa_private_key_from_primes(p.as_slice(), q.as_slice()).unwrap();
        let pk = sk.to_public_key();
        let vk = VerifyingKey::<Sha256>::new(pk);

        let mut digest = Sha256::new();
        digest.update(data);
        let hash = digest.finalize().to_vec();

        assert!(vk
            .verify(&hash, &PssSignature::try_from(sig.as_slice()).unwrap())
            .is_ok());
    }

    #[test]
    fn test_sign_verify_salt_zero() {
        let p = hex::decode("FDEC3A1AEE520780CA4058402D0422B5CD5950B715728F532499DD4BBCB68E5D44650818B43656782237316C4B0E2FAA2B15C245FB82D10CF4F5B420F1F293BA75B2C8D8CEF6AD899C34CE9DE482CB248CC5AB802FD93094A63577590D812D5DD781846EF7D4F5D9018199C293966371C2349B0F847C818EC99CAAD800116E02085D35A39A913BC735327705161761AE30A4EC775F127FBB5165418C0FE08E54AE0AFF8B2DAB2B82D3B4B9C807DE5FAE116096075CF6D5B77450D743D743E7DCC56E7CAFDCC555F228E57B363488E171D099876993E93E37A94983CCC12DBA894C58CA84AC154C1343922C6A99008FABD0FA7010D3CC34F69884FEC902984771").unwrap();
        let q = hex::decode("C5B50031BA31AB7C8B76453CE771F048B84FB89A3E4D44C222C3D8C823C683988B0DBF354D8B8CBF65F3DB53E1365D3C5E043F0155B41D1EBECA6E20B2D6778600B5C98FFDBA33961DAE73B018307EF2BCE9D217BBDF32964080F8DB6F0CF7EF27AC825FCAF98D5143690A5D7E138F4875280ED6DE581E66ED17F83371C268A073E4594814BCC88A33CBB4EC8819CC722EA15490312B85FED06E39274C4F73AC91C7F4D1B899729691CCE616FB1A5FEEE1972456ADDCB51AC830E947FCC1B823468F0EEFBAF195AC3B34F0BAF96AFC6FA77EE2E176081D6D91CE8C93C3D0F3547E48D059C9DA447BA05EE3984703BEBFD6D704B7F327FFAEA7D0F63D0D3C6D65").unwrap();

        let transaction = hex::decode("00f41cfa7bfad3d7b097fcc28ed08cb4ca7d0c544ec760cc6cc5c4f3780d0ec43cc011eaaab0868393c3c813ab8c04df").unwrap();
        let signing_option = SigningOption::Transaction { salt_len: 0 };
        let sig = sign_message(
            transaction.as_slice(),
            p.as_slice(),
            q.as_slice(),
            &signing_option,
        )
        .unwrap();
        let sk = build_rsa_private_key_from_primes(p.as_slice(), q.as_slice()).unwrap();
        let pk = sk.to_public_key();
        let vk = VerifyingKey::<Sha256>::new(pk);

        assert!(vk
            .verify(
                transaction.as_slice(),
                &PssSignature::try_from(sig.as_slice()).unwrap()
            )
            .is_ok());
    }

    #[test]
    fn test_sign_verify_salt_32() {
        let p = hex::decode("FDEC3A1AEE520780CA4058402D0422B5CD5950B715728F532499DD4BBCB68E5D44650818B43656782237316C4B0E2FAA2B15C245FB82D10CF4F5B420F1F293BA75B2C8D8CEF6AD899C34CE9DE482CB248CC5AB802FD93094A63577590D812D5DD781846EF7D4F5D9018199C293966371C2349B0F847C818EC99CAAD800116E02085D35A39A913BC735327705161761AE30A4EC775F127FBB5165418C0FE08E54AE0AFF8B2DAB2B82D3B4B9C807DE5FAE116096075CF6D5B77450D743D743E7DCC56E7CAFDCC555F228E57B363488E171D099876993E93E37A94983CCC12DBA894C58CA84AC154C1343922C6A99008FABD0FA7010D3CC34F69884FEC902984771").unwrap();
        let q = hex::decode("C5B50031BA31AB7C8B76453CE771F048B84FB89A3E4D44C222C3D8C823C683988B0DBF354D8B8CBF65F3DB53E1365D3C5E043F0155B41D1EBECA6E20B2D6778600B5C98FFDBA33961DAE73B018307EF2BCE9D217BBDF32964080F8DB6F0CF7EF27AC825FCAF98D5143690A5D7E138F4875280ED6DE581E66ED17F83371C268A073E4594814BCC88A33CBB4EC8819CC722EA15490312B85FED06E39274C4F73AC91C7F4D1B899729691CCE616FB1A5FEEE1972456ADDCB51AC830E947FCC1B823468F0EEFBAF195AC3B34F0BAF96AFC6FA77EE2E176081D6D91CE8C93C3D0F3547E48D059C9DA447BA05EE3984703BEBFD6D704B7F327FFAEA7D0F63D0D3C6D65").unwrap();

        let signing_option = SigningOption::Transaction { salt_len: 32 };
        let transaction = hex::decode("00f41cfa7bfad3d7b097fcc28ed08cb4ca7d0c544ec760cc6cc5c4f3780d0ec43cc011eaaab0868393c3c813ab8c04df").unwrap();
        let sig = sign_message(
            transaction.as_slice(),
            p.as_slice(),
            q.as_slice(),
            &signing_option,
        )
        .unwrap();

        let sk = build_rsa_private_key_from_primes(p.as_slice(), q.as_slice()).unwrap();
        let pk = sk.to_public_key();
        let vk = VerifyingKey::<Sha256>::new(pk);
        assert!(vk
            .verify(
                transaction.as_slice(),
                &PssSignature::try_from(sig.as_slice()).unwrap()
            )
            .is_ok());
    }

    #[test]
    fn test_build_rsa_private_key_from_primes_invalid_lengths() {
        let valid_q = hex::decode("C5B50031BA31AB7C8B76453CE771F048B84FB89A3E4D44C222C3D8C823C683988B0DBF354D8B8CBF65F3DB53E1365D3C5E043F0155B41D1EBECA6E20B2D6778600B5C98FFDBA33961DAE73B018307EF2BCE9D217BBDF32964080F8DB6F0CF7EF27AC825FCAF98D5143690A5D7E138F4875280ED6DE581E66ED17F83371C268A073E4594814BCC88A33CBB4EC8819CC722EA15490312B85FED06E39274C4F73AC91C7F4D1B899729691CCE616FB1A5FEEE1972456ADDCB51AC830E947FCC1B823468F0EEFBAF195AC3B34F0BAF96AFC6FA77EE2E176081D6D91CE8C93C3D0F3547E48D059C9DA447BA05EE3984703BEBFD6D704B7F327FFAEA7D0F63D0D3C6D65").unwrap();
        let invalid_p_short = hex::decode("0102030405060708090a0b0c0d0e0f").unwrap();

        let result =
            build_rsa_private_key_from_primes(invalid_p_short.as_slice(), valid_q.as_slice())
                .unwrap_err();
        assert!(result
            .to_string()
            .contains("Invalid prime P length: 15, expected 256 bytes"));

        let valid_p = hex::decode("FDEC3A1AEE520780CA4058402D0422B5CD5950B715728F532499DD4BBCB68E5D44650818B43656782237316C4B0E2FAA2B15C245FB82D10CF4F5B420F1F293BA75B2C8D8CEF6AD899C34CE9DE482CB248CC5AB802FD93094A63577590D812D5DD781846EF7D4F5D9018199C293966371C2349B0F847C818EC99CAAD800116E02085D35A39A913BC735327705161761AE30A4EC775F127FBB5165418C0FE08E54AE0AFF8B2DAB2B82D3B4B9C807DE5FAE116096075CF6D5B77450D743D743E7DCC56E7CAFDCC555F228E57B363488E171D099876993E93E37A94983CCC12DBA894C58CA84AC154C1343922C6A99008FABD0FA7010D3CC34F69884FEC902984771").unwrap();
        let invalid_q_long = hex::decode("0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3b4b5b6b7b8b9babbbcbdbebfc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff").unwrap(); // 257 bytes

        let result =
            build_rsa_private_key_from_primes(valid_p.as_slice(), invalid_q_long.as_slice())
                .unwrap_err();
        assert!(result
            .to_string()
            .contains("Invalid prime Q length: 255, expected 256 bytes"));
    }
}
