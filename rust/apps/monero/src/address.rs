use crate::key::*;
use crate::structs::{AddressType, Network};
use crate::utils::{calc_subaddress_m, keccak256};
use alloc::format;
use alloc::string::{String, ToString};
use base58_monero::{decode, encode};
use curve25519_dalek::edwards::EdwardsPoint;
use curve25519_dalek::scalar::Scalar;

use third_party::hex;

pub struct Address {
    /// The network on which the address is valid and should be used.
    pub network: Network,
    /// The address type.
    pub addr_type: AddressType,
    /// The address spend public key.
    pub public_spend: PublicKey,
    /// The address view public key.
    pub public_view: PublicKey,
}

impl Address {
    pub fn new(
        network: Network,
        addr_type: AddressType,
        public_spend: PublicKey,
        public_view: PublicKey,
    ) -> Address {
        Address {
            network,
            addr_type,
            public_spend,
            public_view,
        }
    }

    pub fn from_str(address: &str) -> Result<Address, String> {
        let decoded = decode(address).map_err(|e| format!("decode error: {:?}", e))?;
        let prefix = hex::encode(&decoded[0..1]).to_uppercase();
        let net = match prefix.as_str() {
            "12" => Network::Mainnet,
            "35" => Network::Testnet,
            "18" => Network::Stagenet,
            "2A" => Network::Mainnet,
            "3F" => Network::Testnet,
            "24" => Network::Stagenet,
            _ => return Err("invalid prefix".to_string()),
        };
        let is_subaddress = prefix == "2A" || prefix == "3F" || prefix == "24";
        let public_spend =
            PublicKey::from_bytes(&decoded[1..33]).map_err(|e| format!("decode error: {:?}", e))?;
        let public_view = PublicKey::from_bytes(&decoded[33..65])
            .map_err(|e| format!("decode error: {:?}", e))?;
        Ok(Address {
            network: net,
            addr_type: if is_subaddress {
                AddressType::Subaddress
            } else {
                AddressType::Standard
            },
            public_spend,
            public_view,
        })
    }
}

impl ToString for Address {
    fn to_string(&self) -> String {
        pub_keys_to_address(
            self.network,
            self.addr_type == AddressType::Subaddress,
            &self.public_spend,
            &self.public_view,
        )
    }
}

pub fn get_address_from_seed(
    seed: &[u8],
    is_subaddress: bool,
    net: Network,
    major: u32,
    minor: u32,
) -> Address {
    let keypair = generate_keypair(seed, major);
    let mut public_spend_key = keypair.spend.get_public_key();
    let mut public_view_key = keypair.view.get_public_key();
    if is_subaddress {
        public_spend_key = keypair.get_public_sub_spend(major, minor);
        public_view_key = keypair.get_public_sub_view(major, minor);
    }
    Address {
        network: net,
        addr_type: if is_subaddress {
            AddressType::Subaddress
        } else {
            AddressType::Standard
        },
        public_spend: public_spend_key,
        public_view: public_view_key,
    }
    // pub_keys_to_address(net, is_subaddress, &public_spend_key, &public_view_key)
}

pub fn pub_keyring_to_address(
    net: Network,
    is_subaddress: bool,
    pub_keyring: String,
) -> Option<String> {
    if pub_keyring.len() != 128 {
        return None;
    }
    let pub_spend_key = PublicKey::from_bytes(&hex::decode(&pub_keyring[0..64]).unwrap()).unwrap();
    let pub_view_key = PublicKey::from_bytes(&hex::decode(&pub_keyring[64..128]).unwrap()).unwrap();
    Some(pub_keys_to_address(
        net,
        is_subaddress,
        &pub_spend_key,
        &pub_view_key,
    ))
}

fn pub_keys_to_address(
    net: Network,
    is_subaddress: bool,
    pub_spend_key: &PublicKey,
    pub_view_key: &PublicKey,
) -> String {
    let prefix = match net {
        Network::Mainnet => {
            if is_subaddress {
                "2A"
            } else {
                "12"
            }
        }
        Network::Testnet => {
            if is_subaddress {
                "3F"
            } else {
                "35"
            }
        }
        Network::Stagenet => {
            if is_subaddress {
                "24"
            } else {
                "18"
            }
        }
    };
    let mut res_hex = format!(
        "{}{}{}",
        prefix,
        hex::encode(pub_spend_key.as_bytes()),
        hex::encode(pub_view_key.as_bytes())
    );
    let mut checksum = keccak256(&hex::decode(res_hex.clone()).unwrap());
    res_hex = format!("{}{}", res_hex, hex::encode(&checksum[0..4]));
    encode(&hex::decode(res_hex).unwrap()).unwrap()
}

pub fn generate_subaddress(
    public_spend_key: &PublicKey,
    private_view_key: &PrivateKey,
    major: u32,
    minor: u32,
) -> String {
    let point = public_spend_key.point.decompress().unwrap();
    let m = Scalar::from_bytes_mod_order(calc_subaddress_m(
        &private_view_key.to_bytes(),
        major,
        minor,
    ));
    let pub_spend_key = PublicKey {
        point: (point + EdwardsPoint::mul_base(&m)).compress(),
    };
    let pub_view_point = PublicKey {
        point: (pub_spend_key.point.decompress().unwrap() * private_view_key.scalar).compress(),
    };

    pub_keys_to_address(Network::Mainnet, true, &pub_spend_key, &pub_view_point)
}

#[cfg(test)]
mod tests {
    use super::*;
    use curve25519_dalek::scalar::Scalar;
    use third_party::bitcoin::address;

    #[test]
    fn test_get_address_from_seed() {
        // BIP39 Mnemonic: key stone key stone key stone key stone key stone key stone key stone success
        let seed = hex::decode("45a5056acbe881d7a5f2996558b303e08b4ad1daffacf6ffb757ff2a9705e6b9f806cffe3bd90ff8e3f8e8b629d9af78bcd2ed23e8c711f238308e65b62aa5f0").unwrap();
        let address = get_address_from_seed(&seed, false, Network::Mainnet, 0, 0);
        let address2 = get_address_from_seed(&seed, true, Network::Mainnet, 0, 0);
        let address3 = get_address_from_seed(&seed, true, Network::Mainnet, 0, 1);

        assert_eq!(
            address.public_spend.to_string(),
            "12f38162635cf3aecf081d96158022b2a1517993100e54d62b17057f2443e749"
        );
        assert_eq!(
            address.public_view.to_string(),
            "e18a5360ae4b2ff71bf91c5a626e14fc2395608375b750526bc0962ed27237a1"
        );
        assert_eq!(address.to_string(), "42LmACF1Ce6WEs5w1nNsoPWswJQzcRdZucphf75q1bzvDMjq1vJ2iJziLGdTn1JbcPjB4iiEagCMuEnazTfaryuQKG7sw7S");
        assert_eq!(address2.to_string(), "83TbYo3XUKrgCLJzos7zGiYfRJVwVUWbXVoEZxAiMaEg5ZPsXjZB1ZEXG9UNY58CX2d2RUWMhWSmjK3oSVjmio3e7LJ6KD8");
        assert_eq!(
            address3.public_spend.to_string(),
            "3dca752621e394b068c3bde78951d029778d822aee481a2b08dc21589a3c6693"
        );
        assert_eq!(
            address3.public_view.to_string(),
            "33f3f7b3628e0587f23abec549a071fb420783de74858a1fba0d9e49f3c193f7"
        );
        assert_eq!(address3.to_string(), "84o4iSLUprPWWPeu4ZZPFm7wHMDkwCm9b8CVQ4YUko9PRd453PvhZ8YPjrDRJ4VPrGj2Wxx7KJgFT6JnnbEfapZGUvPSFuM");

        let address4 = Address::from_str("84o4iSLUprPWWPeu4ZZPFm7wHMDkwCm9b8CVQ4YUko9PRd453PvhZ8YPjrDRJ4VPrGj2Wxx7KJgFT6JnnbEfapZGUvPSFuM").unwrap();
        assert_eq!(address3.network, address4.network);
        assert_eq!(address3.addr_type, address4.addr_type);
        assert_eq!(
            address3.public_spend.to_string(),
            address4.public_spend.to_string()
        );
        assert_eq!(
            address3.public_view.to_string(),
            address4.public_view.to_string()
        );
    }

    #[test]
    fn test_pub_keyring_to_address() {
        // kid m/44'/128'/1'
        let keyring = "ca977af9ef22115fede5e19c03aea87a4b50b276e0198901424831c49b61c3b4b686d1921ac09d53369a00dea6b92d803f11a32df99d97b0aacb2059d2c5bba6";
        let address = pub_keyring_to_address(Network::Mainnet, false, keyring.to_string()).unwrap();
        assert_eq!(
            address,
            "49JPkSbqqnYH3dngTbHfUKMTQbvUbBpr41DDMgPf7wiJXE9G6aMDoxpEvGqnyKZxPNNT9iqZnJWj8WYtnHne9vAEKpYpbc9"
        );
    }

    #[test]
    fn test_generate_subaddress() {
        // BIP39 Mnemonic: key stone key stone key stone key stone key stone key stone key stone success
        let seed = hex::decode("45a5056acbe881d7a5f2996558b303e08b4ad1daffacf6ffb757ff2a9705e6b9f806cffe3bd90ff8e3f8e8b629d9af78bcd2ed23e8c711f238308e65b62aa5f0").unwrap();
        let major = 0;
        let minor = 1;
        let keypair = generate_keypair(&seed, major);

        let public_spend_key = keypair.spend.get_public_key();
        let private_view_key = keypair.view;

        let address = generate_subaddress(&public_spend_key, &private_view_key, major, minor);

        assert_eq!(
            address,
            "84o4iSLUprPWWPeu4ZZPFm7wHMDkwCm9b8CVQ4YUko9PRd453PvhZ8YPjrDRJ4VPrGj2Wxx7KJgFT6JnnbEfapZGUvPSFuM"
        );
    }

    #[test]
    fn test_calc_subaddress_m() {
        let seed = hex::decode("45a5056acbe881d7a5f2996558b303e08b4ad1daffacf6ffb757ff2a9705e6b9f806cffe3bd90ff8e3f8e8b629d9af78bcd2ed23e8c711f238308e65b62aa5f0").unwrap();
        let major = 0;
        let keypair = generate_keypair(&seed, major);

        let point = keypair.get_public_spend().point.decompress().unwrap();

        let m = Scalar::from_bytes_mod_order(calc_subaddress_m(
            &keypair.view.to_bytes(),
            1,
            0,
        ));
        let pub_spend_key = PublicKey {
            point: (point + EdwardsPoint::mul_base(&m)).compress(),
        };
        let pub_view_point = PublicKey {
            point: (pub_spend_key.point.decompress().unwrap() * keypair.view.scalar).compress(),
        };

        assert_eq!(
            pub_spend_key.to_string(),
            "da84d414ef5cdeed0ae9a19259e9b18482650efcadd371ba3ef51819f391260f"
        );

        assert_eq!(
            pub_view_point.to_string(),
            "5a69bc37d807013f80e10959bc7855419f1b0b47258a64a6a8c440ffd223070f"
        );

        let sun_account = generate_subaddress(&keypair.get_public_spend(), &keypair.view, 1, 0);

        assert_eq!(
            sun_account,
            "8AjYV2hmNQugecQSpckuiGPAWPdW5NxzcY9pVRzLArA13Zwp7KLcB8UBd4eSqH4xLLBycRmwzqNxjUsobmUv8rSr2j73xbR"
        );

        let sun_account_sub_address_1 = generate_subaddress(&keypair.get_public_spend(), &keypair.view, 1, 1);

        assert_eq!(
            sun_account_sub_address_1,
            "83fsQ5aW7PMXxC3NjDiZKdC2LYmjgBgCcix1oFZ51NgfES3YAKC27zxXqVkukpKuhsQzWKcKPMLPpJjVJyTcCphZRCBQtTw"
        )

    }
}
