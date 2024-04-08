use crate::errors::{KeystoreError, Result};

use alloc::string::ToString;
use alloc::vec::Vec;
use arrayref::array_ref;
use rand_chacha::ChaCha20Rng;
use rand_core::SeedableRng;

use sha2;
use sha2::{Digest, Sha256};

use third_party::rsa::{rand_core, BigUint, PublicKeyParts, RsaPrivateKey};

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
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::borrow::ToOwned;
    use third_party::{base64, hex};

    #[test]
    fn test_private_key_recover() {
        let p = hex::decode("EA8E3612876ED1433E5909D25F699F7C5D4984CF0D2F268B185141F0E29CE65237EAD8236C94A0A9547F1FEABD4F54399C626C0FB813249BC74A3082F8637A9E9A3C9D4F6E1858ED29770FE95418ED66F07A9F2F378D43D31ED37A0E6942727394A87B93540E421742ADE9630E26500FD2C01502DF8E3F869C70DAA97D4583048DD367E2977851052F6A991182318015557EC81B58E81B668E3A715212C807A1D7835FCB2B87B5DEFAC0948B220D340D6B2DA0DCFC7123DE1F1424F6F5B8EAFA719B3DE8B9B6FEC196E2E393CE30204267A586625541C7B1433F8FA7873B51B3E65462831BF34A4E5912297A06B2E91B31657DFA3CCFDB5F94D438D9904CFD27").unwrap();
        let q = hex::decode("E360BFD757FF6FCF844AF226DCA7CFBD353A89112079C9C5A17C4F354DE0B1BE38BBFD73EAA77C4E2FFC681A79CEC8C8E79A5A00E32113A77748F435717BE6AD04AEF473BCE05DC3B742AAB853C02C565847133AFFD451B472B13031300978606F74BE8761A69733BEF8C2CCD6F396A0CCE23CDC73A8FF4609F47C18FE4626B788C4BFB73B9CF10BC5D6F80E9B9847530973CF5212D8EB142EAA155D774417D7BF89E1F229472926EA539AC3BAF42CF63EF18A6D85915727E9A77B4EA31B577B1E4A35C40CCCE72F5ECE426709E976DAEDBE7B76291F89EB85903182035CA98EB156563E392D0D1E427C59657B9EDF1DDB049BBB9620B881D8715982AD257D29").unwrap();
        let seed = hex::decode("96063C45132C840F7E1665A3B97814D8EB2586F34BD945F06FA15B9327EEBE355F654E81C6233A52149D7A95EA7486EB8D699166F5677E507529482599624CDC").unwrap();
        let result = get_rsa_secret_from_seed(seed.as_slice()).unwrap();

        assert_eq!(result.primes()[0], BigUint::from_bytes_be(&p));
        assert_eq!(result.primes()[1], BigUint::from_bytes_be(&q));
    }
}
