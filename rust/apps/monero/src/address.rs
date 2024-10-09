use crate::key::*;
use crate::structs::Network;
use crate::utils::{calc_subaddress_m, keccak256};
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use base58_monero::{decode, encode};
use curve25519_dalek::scalar::Scalar;

use third_party::hex;

pub fn get_address_from_seed(
    seed: &[u8],
    is_subaddress: bool,
    net: Network,
    major: u32,
    minor: u32,
) -> String {
    let keypair = generate_keypair(seed, major);
    let mut public_spend_key = keypair.spend.get_public_key();
    let mut public_view_key = keypair.view.get_public_key();
    if is_subaddress {
        public_spend_key = keypair.get_public_sub_spend(major, minor);
        public_view_key = keypair.get_public_sub_view(major, minor);
    }
    pub_keys_to_address(net, is_subaddress, &public_spend_key, &public_view_key)
}

pub fn pub_keys_to_address(
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

#[cfg(test)]
mod tests {
    use super::*;
    use curve25519_dalek::scalar::Scalar;

    #[test]
    fn test_get_address_from_seed() {
        // BIP39 Mnemonic: key stone key stone key stone key stone key stone key stone key stone success
        let seed = hex::decode("45a5056acbe881d7a5f2996558b303e08b4ad1daffacf6ffb757ff2a9705e6b9f806cffe3bd90ff8e3f8e8b629d9af78bcd2ed23e8c711f238308e65b62aa5f0").unwrap();
        let address = get_address_from_seed(&seed, false, Network::Mainnet, 0, 0);
        let address2 = get_address_from_seed(&seed, true, Network::Mainnet, 0, 0);
        let address3 = get_address_from_seed(&seed, true, Network::Mainnet, 0, 1);

        assert_eq!(address, "42LmACF1Ce6WEs5w1nNsoPWswJQzcRdZucphf75q1bzvDMjq1vJ2iJziLGdTn1JbcPjB4iiEagCMuEnazTfaryuQKG7sw7S");
        assert_eq!(address2, "83TbYo3XUKrgCLJzos7zGiYfRJVwVUWbXVoEZxAiMaEg5ZPsXjZB1ZEXG9UNY58CX2d2RUWMhWSmjK3oSVjmio3e7LJ6KD8");
        assert_eq!(address3, "84o4iSLUprPWWPeu4ZZPFm7wHMDkwCm9b8CVQ4YUko9PRd453PvhZ8YPjrDRJ4VPrGj2Wxx7KJgFT6JnnbEfapZGUvPSFuM");
    }
}
