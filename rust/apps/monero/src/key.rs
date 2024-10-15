use crate::utils::{calc_subaddress_m, hash_to_scalar};

use alloc::format;
use alloc::string::{String, ToString};
use curve25519_dalek::edwards::{CompressedEdwardsY, EdwardsPoint};
use curve25519_dalek::scalar::Scalar;

use third_party::bitcoin::{Network, PrivateKey as PrvKey};
use third_party::hex;

#[derive(Debug, Clone)]
pub struct PrivateKey {
    /// The actual curve25519 scalar.
    pub scalar: Scalar,
}

impl PrivateKey {
    pub fn new(scalar: Scalar) -> PrivateKey {
        PrivateKey { scalar }
    }

    pub fn to_bytes(&self) -> [u8; 32] {
        self.scalar.to_bytes()
    }

    pub fn get_public_key(&self) -> PublicKey {
        PublicKey::from(self.clone())
    }
}

pub struct PublicKey {
    /// The actual Ed25519 point.
    pub point: CompressedEdwardsY,
}

impl PublicKey {
    pub fn new(point: CompressedEdwardsY) -> PublicKey {
        PublicKey { point }
    }

    pub fn from_bytes(bytes: &[u8]) -> Result<PublicKey, String> {
        let point =
            CompressedEdwardsY::from_slice(bytes).map_err(|e| format!("decode error: {:?}", e))?;
        Ok(PublicKey { point })
    }

    pub fn from_str(s: &str) -> Result<PublicKey, String> {
        let bytes = hex::decode(s).map_err(|e| format!("decode error: {:?}", e))?;
        PublicKey::from_bytes(&bytes)
    }
}

impl ToString for PublicKey {
    fn to_string(&self) -> String {
        hex::encode(&self.point.to_bytes())
    }
}

impl From<PrivateKey> for PublicKey {
    fn from(private_key: PrivateKey) -> PublicKey {
        let scalar = Scalar::from_bytes_mod_order(private_key.to_bytes());
        PublicKey {
            point: EdwardsPoint::mul_base(&scalar).compress(),
        }
    }
}

impl PublicKey {
    pub fn as_bytes(&self) -> [u8; 32] {
        self.point.to_bytes()
    }
}

pub struct KeyPair {
    /// The private view key needed to recognize owned outputs.
    pub view: PrivateKey,
    /// The private spend key needed to spend owned outputs.
    pub spend: PrivateKey,
}

// KeyPair from raw private keys
impl KeyPair {
    pub fn from_raw_private_keys(raw_private_key: PrvKey) -> KeyPair {
        let secret_spend_key = hash_to_scalar(&raw_private_key.to_bytes());
        KeyPair {
            view: PrivateKey {
                scalar: hash_to_scalar(&secret_spend_key.to_bytes()),
            },
            spend: PrivateKey {
                scalar: secret_spend_key,
            },
        }
    }

    pub fn get_public_spend(&self) -> PublicKey {
        self.spend.get_public_key()
    }

    pub fn get_public_view(&self) -> PublicKey {
        self.view.get_public_key()
    }

    pub fn get_m(&self, major: u32, minor: u32) -> [u8; 32] {
        calc_subaddress_m(&self.view.to_bytes(), major, minor)
    }

    pub fn get_public_sub_spend(&self, major: u32, minor: u32) -> PublicKey {
        let point = self.get_public_spend().point.decompress().unwrap();
        let m = Scalar::from_bytes_mod_order(self.get_m(major, minor));

        PublicKey {
            point: (point + EdwardsPoint::mul_base(&m)).compress(),
        }
    }

    pub fn get_public_sub_view(&self, major: u32, minor: u32) -> PublicKey {
        let sub_spend_public_key = self.get_public_sub_spend(major, minor);

        PublicKey::new(
            (sub_spend_public_key.point.decompress().unwrap() * self.view.scalar).compress(),
        )
    }
}

pub fn generate_keypair(seed: &[u8], major: u32) -> KeyPair {
    let path = format!("m/44'/128'/{}'/0/0", major);
    let key =
        keystore::algorithms::secp256k1::get_private_key_by_seed(&seed, &path.to_string()).unwrap();
    let raw_private_key = PrvKey::new(key, Network::Bitcoin);

    KeyPair::from_raw_private_keys(raw_private_key)
}

pub fn generate_private_view_key(seed: &[u8], major: u32) -> PrivateKey {
    generate_keypair(seed, major).view
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_monero_keys() {
        // BIP39 Mnemonic: key stone key stone key stone key stone key stone key stone key stone success
        let seed = hex::decode("45a5056acbe881d7a5f2996558b303e08b4ad1daffacf6ffb757ff2a9705e6b9f806cffe3bd90ff8e3f8e8b629d9af78bcd2ed23e8c711f238308e65b62aa5f0").unwrap();
        let keypair = generate_keypair(&seed, 0);
        let public_spend_key = keypair.spend.get_public_key();
        let public_view_key = keypair.view.get_public_key();

        let private_view_key = generate_private_view_key(&seed, 0);

        assert_eq!(
            hex::encode(keypair.spend.to_bytes()),
            "6c3895c1dfd7c3ed22be481ed5ec7f40e3d8ded84f0a3d65a542915475ca6f0e"
        );
        assert_eq!(
            hex::encode(keypair.view.to_bytes()),
            "17921dbd51b4a1af0b4049bc13dc7048ace1dcd8be9b8669de95b8430924ea09"
        );
        assert_eq!(
            hex::encode(public_spend_key.as_bytes()),
            "12f38162635cf3aecf081d96158022b2a1517993100e54d62b17057f2443e749"
        );
        assert_eq!(
            hex::encode(public_view_key.as_bytes()),
            "e18a5360ae4b2ff71bf91c5a626e14fc2395608375b750526bc0962ed27237a1"
        );
        assert_eq!(
            hex::encode(private_view_key.to_bytes()),
            hex::encode(keypair.view.to_bytes())
        )
    }

    #[test]
    fn test_monero_subadd_keys() {
        // BIP39 Mnemonic: key stone key stone key stone key stone key stone key stone key stone success
        let seed = hex::decode("45a5056acbe881d7a5f2996558b303e08b4ad1daffacf6ffb757ff2a9705e6b9f806cffe3bd90ff8e3f8e8b629d9af78bcd2ed23e8c711f238308e65b62aa5f0").unwrap();
        let major = 0;
        let minor = 1;
        let keypair = generate_keypair(&seed, major);

        assert_eq!(
            hex::encode(keypair.get_m(major, minor)),
            "426543494cfc94803177f4ccffaee54275d9accb3f54a2caafa753ff62e8b400"
        );
        assert_eq!(
            hex::encode(keypair.get_public_sub_spend(major, minor).as_bytes()),
            "3dca752621e394b068c3bde78951d029778d822aee481a2b08dc21589a3c6693"
        );
        assert_eq!(
            hex::encode(keypair.get_public_sub_view(major, minor).as_bytes()),
            "33f3f7b3628e0587f23abec549a071fb420783de74858a1fba0d9e49f3c193f7"
        );
    }
}
