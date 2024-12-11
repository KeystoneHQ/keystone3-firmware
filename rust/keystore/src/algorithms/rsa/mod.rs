use crate::errors::{KeystoreError, Result};

use alloc::string::ToString;
use alloc::vec::Vec;
use arrayref::array_ref;
use rand_chacha::ChaCha20Rng;
use rand_core::SeedableRng;

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
    let rsa_seed = get_rsa_seed(seed)?;
    let mut rng = ChaCha20Rng::from_seed(rsa_seed);
    let private_key = RsaPrivateKey::new(&mut rng, MODULUS_LENGTH).map_err(|_| {
        KeystoreError::GenerateSigningKeyError("generate rsa private key failed".to_string())
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
        SigningOption::PSS { salt_len } => {
            let parsed_salt_len: usize = (*salt_len)
                .try_into()
                .map_err(|_| KeystoreError::RSASignError)?;
            let signing_key = SigningKey::<Sha256>::new_with_salt_len(private_key, parsed_salt_len);
            let mut rng = ChaCha20Rng::from_seed([42; 32]);
            let mut digest = sha2::Sha256::new();
            digest.update(data);
            let signature = signing_key.sign_digest_with_rng(&mut rng, digest);
            Ok(Vec::from(signature.to_bytes()))
        }
        SigningOption::RSA { salt_len } => {
            let parsed_salt_len: usize = (*salt_len)
                .try_into()
                .map_err(|_| KeystoreError::RSASignError)?;
            let signing_key = SigningKey::<Sha256>::new_with_salt_len(private_key, parsed_salt_len);
            let mut rng = ChaCha20Rng::from_seed([42; 32]);
            let mut digest = sha2::Sha256::new();
            digest.update(data);
            let hash = digest.finalize().to_vec();
            let result = signing_key.sign_with_rng(&mut rng, &hash);
            Ok(Vec::from(result.to_bytes()))
        } // _ => Err(KeystoreError::RSASignError),
    }
}

pub fn build_rsa_private_key_from_primes(p: &[u8], q: &[u8]) -> Result<RsaPrivateKey> {
    let n = BigUint::from_bytes_be(p) * BigUint::from_bytes_be(q);
    let p = BigUint::from_bytes_be(p);
    let q = BigUint::from_bytes_be(q);
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

#[allow(dead_code)]
pub struct RSA {
    private_key: RsaPrivateKey,
}

#[allow(dead_code)]
impl RSA {
    fn from_secret(secret: &[u8]) -> Result<RSA> {
        if secret.len() != SECRET_LENGTH_IN_BYTE {
            return Err(KeystoreError::GenerateSigningKeyError(format!(
                "invalid secret length, expected is {:?}, got {:?}",
                SECRET_LENGTH_IN_BYTE,
                secret.len()
            )));
        }
        let p = array_ref![secret, 0, PRIME_LENGTH_IN_BYTE];
        let q = array_ref![secret, PRIME_LENGTH_IN_BYTE, PRIME_LENGTH_IN_BYTE];
        let d = array_ref![secret, PRIME_LENGTH_IN_BYTE * 2, MODULUS_LENGTH_IN_BYTE];
        let n = array_ref![
            secret,
            PRIME_LENGTH_IN_BYTE * 2 + MODULUS_LENGTH_IN_BYTE,
            MODULUS_LENGTH_IN_BYTE
        ];
        let e = vec![01, 00, 01];
        let private_key = RsaPrivateKey::from_components(
            BigUint::from_bytes_be(n),
            BigUint::from_bytes_be(&e),
            BigUint::from_bytes_be(d),
            [BigUint::from_bytes_be(p), BigUint::from_bytes_be(q)].to_vec(),
        )
        .map_err(|_| {
            KeystoreError::GenerateSigningKeyError("failed to compose rsa signing key".to_string())
        })?;
        Ok(Self { private_key })
    }
}

#[derive(Clone, Copy)]
pub enum SigningOption {
    PSS { salt_len: i32 },
    RSA { salt_len: i32 },
}

#[cfg(test)]
mod tests {
    use super::*;
    use bitcoin::hex::DisplayHex;
    use hex;
    use rand_core::RngCore;
    use rsa::pkcs1v15::SigningKey;
    use rsa::signature::{Keypair, RandomizedSigner, SignatureEncoding, Verifier};
    use sha2::Sha256;

    #[test]
    fn test_private_key_recover() {
        let p = hex::decode("FDEC3A1AEE520780CA4058402D0422B5CD5950B715728F532499DD4BBCB68E5D44650818B43656782237316C4B0E2FAA2B15C245FB82D10CF4F5B420F1F293BA75B2C8D8CEF6AD899C34CE9DE482CB248CC5AB802FD93094A63577590D812D5DD781846EF7D4F5D9018199C293966371C2349B0F847C818EC99CAAD800116E02085D35A39A913BC735327705161761AE30A4EC775F127FBB5165418C0FE08E54AE0AFF8B2DAB2B82D3B4B9C807DE5FAE116096075CF6D5B77450D743D743E7DCC56E7CAFDCC555F228E57B363488E171D099876993E93E37A94983CCC12DBA894C58CA84AC154C1343922C6A99008FABD0FA7010D3CC34F69884FEC902984771").unwrap();
        let q = hex::decode("C5B50031BA31AB7C8B76453CE771F048B84FB89A3E4D44C222C3D8C823C683988B0DBF354D8B8CBF65F3DB53E1365D3C5E043F0155B41D1EBECA6E20B2D6778600B5C98FFDBA33961DAE73B018307EF2BCE9D217BBDF32964080F8DB6F0CF7EF27AC825FCAF98D5143690A5D7E138F4875280ED6DE581E66ED17F83371C268A073E4594814BCC88A33CBB4EC8819CC722EA15490312B85FED06E39274C4F73AC91C7F4D1B899729691CCE616FB1A5FEEE1972456ADDCB51AC830E947FCC1B823468F0EEFBAF195AC3B34F0BAF96AFC6FA77EE2E176081D6D91CE8C93C3D0F3547E48D059C9DA447BA05EE3984703BEBFD6D704B7F327FFAEA7D0F63D0D3C6D65").unwrap();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let result = get_rsa_secret_from_seed(seed.as_slice()).unwrap();
        let p = p.to_upper_hex_string();
        let q = q.to_upper_hex_string();
        assert_eq!(result.primes()[0].to_bytes_be().to_upper_hex_string(), p);
        assert_eq!(result.primes()[1].to_bytes_be().to_upper_hex_string(), q);
    }

    #[test]
    fn test_sign_message() {
        let p = hex::decode("EA8E3612876ED1433E5909D25F699F7C5D4984CF0D2F268B185141F0E29CE65237EAD8236C94A0A9547F1FEABD4F54399C626C0FB813249BC74A3082F8637A9E9A3C9D4F6E1858ED29770FE95418ED66F07A9F2F378D43D31ED37A0E6942727394A87B93540E421742ADE9630E26500FD2C01502DF8E3F869C70DAA97D4583048DD367E2977851052F6A991182318015557EC81B58E81B668E3A715212C807A1D7835FCB2B87B5DEFAC0948B220D340D6B2DA0DCFC7123DE1F1424F6F5B8EAFA719B3DE8B9B6FEC196E2E393CE30204267A586625541C7B1433F8FA7873B51B3E65462831BF34A4E5912297A06B2E91B31657DFA3CCFDB5F94D438D9904CFD27").unwrap();
        let q = hex::decode("E360BFD757FF6FCF844AF226DCA7CFBD353A89112079C9C5A17C4F354DE0B1BE38BBFD73EAA77C4E2FFC681A79CEC8C8E79A5A00E32113A77748F435717BE6AD04AEF473BCE05DC3B742AAB853C02C565847133AFFD451B472B13031300978606F74BE8761A69733BEF8C2CCD6F396A0CCE23CDC73A8FF4609F47C18FE4626B788C4BFB73B9CF10BC5D6F80E9B9847530973CF5212D8EB142EAA155D774417D7BF89E1F229472926EA539AC3BAF42CF63EF18A6D85915727E9A77B4EA31B577B1E4A35C40CCCE72F5ECE426709E976DAEDBE7B76291F89EB85903182035CA98EB156563E392D0D1E427C59657B9EDF1DDB049BBB9620B881D8715982AD257D29").unwrap();
        let data = b"hello world";
        let result = sign_message(
            data,
            p.as_slice(),
            q.as_slice(),
            &SigningOption::PSS { salt_len: 32 },
        )
        .unwrap();
        let result = hex::encode(result);
        assert_eq!(result, "986ffe8b2da3326c82ff6972b1ed4d510498ee0fb8abb3c30c7a952a14357aa5d7cb9bfcb0c5b2223ceb586f86e6dfac2c57acc98f5dad881853dc996e1e6fbcec167e4bc7deb1290352f221301ae89c1a336a57dc2980485ceac90a688a85cce497774db51a207620e185d84176e312c94ba3188afbb0885b6f6bcf42e40039269b7bf2550c68aecc0122901f707d61ab0c92174e4f463a446cdeff85ac8d1cfc828fba08cd6ab97a19ac79a1be85567bda4829096d188eab045d1967c245210e51ef7add10dd0c93adbae7829ad7ae3ad291633d4fb007f931a5def9ffb79198dde26b7ab6a76ab54f2ea4fd0b4b658aae37f37481bc253f335a549562c729df1b1736c09c74212133ccd1580c79f6e90a229c64edbb232f2685ebae72466d9312365560c88e5dffd576526b38cb899287aafb2e99698981cf595d582903d850181e25535899fc50105540e37068f9dcdd1cc1f490e72a80120550612e3c08b3b648b7edbcb5bb033ab7ca42cc5cb3ce3aa0f218d4cd46b947cc2bd484db55fa1cfa1e76f33ebea3ec4887247a00be1128f633167efede7ad420ac59f6d667b60cdf2aa444d8645966d931d62f20534c0247ee28d9d93b71d67a05c0b0285a1921471973142764c130c230de0cfd52d3e7b817555272762fc53a00711a8600990490414bcd36188c203e9454f6ccf28b79a08e0df867a3030be4162300d639");
    }

    #[test]
    fn test_sign_verify_salt_zero() {
        let p = hex::decode("FDEC3A1AEE520780CA4058402D0422B5CD5950B715728F532499DD4BBCB68E5D44650818B43656782237316C4B0E2FAA2B15C245FB82D10CF4F5B420F1F293BA75B2C8D8CEF6AD899C34CE9DE482CB248CC5AB802FD93094A63577590D812D5DD781846EF7D4F5D9018199C293966371C2349B0F847C818EC99CAAD800116E02085D35A39A913BC735327705161761AE30A4EC775F127FBB5165418C0FE08E54AE0AFF8B2DAB2B82D3B4B9C807DE5FAE116096075CF6D5B77450D743D743E7DCC56E7CAFDCC555F228E57B363488E171D099876993E93E37A94983CCC12DBA894C58CA84AC154C1343922C6A99008FABD0FA7010D3CC34F69884FEC902984771").unwrap();
        let q = hex::decode("C5B50031BA31AB7C8B76453CE771F048B84FB89A3E4D44C222C3D8C823C683988B0DBF354D8B8CBF65F3DB53E1365D3C5E043F0155B41D1EBECA6E20B2D6778600B5C98FFDBA33961DAE73B018307EF2BCE9D217BBDF32964080F8DB6F0CF7EF27AC825FCAF98D5143690A5D7E138F4875280ED6DE581E66ED17F83371C268A073E4594814BCC88A33CBB4EC8819CC722EA15490312B85FED06E39274C4F73AC91C7F4D1B899729691CCE616FB1A5FEEE1972456ADDCB51AC830E947FCC1B823468F0EEFBAF195AC3B34F0BAF96AFC6FA77EE2E176081D6D91CE8C93C3D0F3547E48D059C9DA447BA05EE3984703BEBFD6D704B7F327FFAEA7D0F63D0D3C6D65").unwrap();

        let message = hex::decode("00f41cfa7bfad3d7b097fcc28ed08cb4ca7d0c544ec760cc6cc5c4f3780d0ec43cc011eaaab0868393c3c813ab8c04df").unwrap();
        let signing_option = SigningOption::PSS { salt_len: 0 };
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let signature = sign_message(
            message.as_slice(),
            p.as_slice(),
            q.as_slice(),
            &signing_option,
        )
        .unwrap();
        assert_eq!(hex::encode(signature.clone()), "a8e58c9aa9a74039f239f49adca18ea5d54b9d28852b7d39b098a96230ebe4b07bf1f66eea2ef3ee29ab912f90508917703ca9838f228b0f75014ea5d41101f7dff194d8086010aa92b6e6d04a56ed6cb7bd63c3dc15f833c0fcbeb03a16892ed715f7b178c20dbb6cd9923ddd0ab4b1c8753a554a8165ff34224fb630445582d3b588581deca41dbcf2144dcf10a362510178af9923e9f6cdf30dfaafa5642a20f777a4a9bff7170517d9a4347a2f0e360a38bf90a8b5d10f80f2581422798aa7b77d959f237a77d71b35558349e35f9c1193154bcf252d79171abeec6f37858584f878503af44a3553eb218b86dc31dfcca66dea947364580515bb2543d2403d53866ee16bba1b8e51ba060a5ecfef3ef4617d96fa3a3f67176621e638ad7e33bf08c56409f0ce01ef345ac4b49ba4fd94dbaf11b544f4ce089d9adcebf5b592afd2f8cecf22f21539975e50441fe3bf5f77d7d0fcfa2bd3c6e2cbf1bb59ed141b5c0f257be5958c5b46c9f08ec1e912b7fa6ff7182aa9010ce9f0cd6fc4845760a37f97197ea8ad3fa8a75b742e9ad61f877acd5771e7c43e0c75a422eb7d96153d4c561469c0f6011d0fe74f718b2db26894e3c5daf72784d34374c4dab78c3ff7619f883085a45efe1781cfcdb80b64b4c8aa96f86225144ca9430a499e96c607a77538ad7fb920fdd1126cdc8c5574ed3c2b1fb1dadac51ad4e13fdd9d");
        // let result = verify(signature.as_slice(), message.as_slice());
        // println!("verify result {:?}", result);
        // assert_eq!(result.ok(), Some(()));
    }

    #[test]
    fn test_sign_verify_salt_32() {
        let p = hex::decode("FDEC3A1AEE520780CA4058402D0422B5CD5950B715728F532499DD4BBCB68E5D44650818B43656782237316C4B0E2FAA2B15C245FB82D10CF4F5B420F1F293BA75B2C8D8CEF6AD899C34CE9DE482CB248CC5AB802FD93094A63577590D812D5DD781846EF7D4F5D9018199C293966371C2349B0F847C818EC99CAAD800116E02085D35A39A913BC735327705161761AE30A4EC775F127FBB5165418C0FE08E54AE0AFF8B2DAB2B82D3B4B9C807DE5FAE116096075CF6D5B77450D743D743E7DCC56E7CAFDCC555F228E57B363488E171D099876993E93E37A94983CCC12DBA894C58CA84AC154C1343922C6A99008FABD0FA7010D3CC34F69884FEC902984771").unwrap();
        let q = hex::decode("C5B50031BA31AB7C8B76453CE771F048B84FB89A3E4D44C222C3D8C823C683988B0DBF354D8B8CBF65F3DB53E1365D3C5E043F0155B41D1EBECA6E20B2D6778600B5C98FFDBA33961DAE73B018307EF2BCE9D217BBDF32964080F8DB6F0CF7EF27AC825FCAF98D5143690A5D7E138F4875280ED6DE581E66ED17F83371C268A073E4594814BCC88A33CBB4EC8819CC722EA15490312B85FED06E39274C4F73AC91C7F4D1B899729691CCE616FB1A5FEEE1972456ADDCB51AC830E947FCC1B823468F0EEFBAF195AC3B34F0BAF96AFC6FA77EE2E176081D6D91CE8C93C3D0F3547E48D059C9DA447BA05EE3984703BEBFD6D704B7F327FFAEA7D0F63D0D3C6D65").unwrap();

        let signing_option = SigningOption::PSS { salt_len: 32 };
        let message = hex::decode("00f41cfa7bfad3d7b097fcc28ed08cb4ca7d0c544ec760cc6cc5c4f3780d0ec43cc011eaaab0868393c3c813ab8c04df").unwrap();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let signature = sign_message(
            message.as_slice(),
            p.as_slice(),
            q.as_slice(),
            &signing_option,
        )
        .unwrap();
        // let result = verify(&signature.as_ref(), message.as_slice());
        // assert_eq!(result.ok(), Some(()));
    }
}
