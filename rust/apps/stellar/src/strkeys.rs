use alloc::string::String;
use alloc::vec::Vec;
use keystore::algorithms::ed25519::slip10_ed25519::get_private_key_by_seed;
use keystore::errors::Result;
use third_party::cryptoxide::hashing;

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum StrKeyType {
    STRKEY_PUBKEY = 6 << 3,
    STRKEY_PRIVKEY = 18 << 3,
    STRKEY_PRE_AUTH_TX_KEY = 19 << 3,
    STRKEY_HASH_X = 23 << 3,
    STRKEY_MUXED_ACCOUNT = 12 << 3,
    STRKEY_SIGNED_PAYLOAD = 15 << 3,
}

const CRC_TABLE: [u16; 256] = [
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b,
    0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,
    0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738,
    0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96,
    0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,
    0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb,
    0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2,
    0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
    0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827,
    0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d,
    0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
    0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
];

pub fn calculate_crc16_checksum(payload: &[u8]) -> [u8; 2] {
    let mut crc: u16 = 0;

    for byte in payload {
        let lookup_index = ((crc >> 8) ^ (*byte as u16)) as usize;
        crc = (crc << 8) ^ CRC_TABLE[lookup_index];
    }

    let checksum_bytes = crc.to_le_bytes();
    [checksum_bytes[0], checksum_bytes[1]]
}

const BASE32_ALPHABET: &[u8; 32] = b"ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

pub fn encode_base32(input: &[u8]) -> String {
    let mut output = String::new();
    let mut bits = 0;
    let mut value = 0;

    for &byte in input {
        value = (value << 8) | byte as u32;
        bits += 8;

        while bits >= 5 {
            let index = (value >> (bits - 5)) & 0x1F;
            output.push(BASE32_ALPHABET[index as usize] as char);
            bits -= 5;
        }
    }

    if bits > 0 {
        let index = (value << (5 - bits)) & 0x1F;
        output.push(BASE32_ALPHABET[index as usize] as char);
    }

    output
}

pub fn generate_stellar_private_key(seed: &[u8], path: &String) -> Result<String> {
    let private_key = get_private_key_by_seed(seed, path)?;
    let key = [StrKeyType::STRKEY_PRIVKEY as u8]
        .iter()
        .chain(private_key.iter())
        .cloned()
        .collect::<Vec<u8>>();
    let checksum = calculate_crc16_checksum(&key);
    let data = key
        .iter()
        .chain(checksum.iter())
        .cloned()
        .collect::<Vec<u8>>();
    Ok(encode_base32(&data))
}

pub fn sign_hash(hash: &[u8], seed: &[u8], path: &String) -> Result<[u8; 64]> {
    keystore::algorithms::ed25519::slip10_ed25519::sign_message_by_seed(seed, path, &hash)
}

pub fn sign_signature_base(signature_base: &[u8], seed: &[u8], path: &String) -> Result<[u8; 64]> {
    let hash = hashing::sha256(signature_base);
    sign_hash(&hash, seed, path)
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::{String, ToString};
    use third_party::hex;

    #[test]
    fn test_stellar_private_key() {
        // Test1 of https://github.com/stellar/stellar-protocol/blob/master/ecosystem/sep-0005.md
        let seed = hex::decode("96063c45132c840f7e1665a3b97814d8eb2586f34bd945f06fa15b9327eebe355f654e81c6233a52149d7a95ea7486eb8d699166f5677e507529482599624cdc").unwrap();
        let path = "m/44'/148'/0'".to_string();
        let private_key = generate_stellar_private_key(&seed, &path).unwrap();
        assert_eq!(
            "SCIA76H6JJZI4O5TNYT7AHNNPAYE57NM66BZVA4VFUNILL3MYGRZWIG5",
            private_key
        );

        let private_key = get_private_key_by_seed(&seed, &path).unwrap();
        let (keypair, _) = third_party::cryptoxide::ed25519::keypair(&private_key);
        assert_eq!(
            "900ff8fe4a728e3bb36e27f01dad78304efdacf7839a83952d1a85af6cc1a39bd4b8322ed2ca75a7a8f7eb57057471b17bd7d5fea4f9a8a293636b4d653fcf3d",
            hex::encode(keypair)
        );
    }

    #[test]
    fn test_sign_hash() {
        let seed = hex::decode("96063c45132c840f7e1665a3b97814d8eb2586f34bd945f06fa15b9327eebe355f654e81c6233a52149d7a95ea7486eb8d699166f5677e507529482599624cdc")
            .unwrap();
        let path = "m/44'/148'/0'".to_string();
        let hash = hex::decode("5a859bfd9b8b7469d59e885ea3eff09050f0d7e1a9b496c91a5b82c9958b4fe7")
            .unwrap();
        let signed = sign_hash(&hash, &seed, &path).unwrap();
        assert_eq!(
            "baa7bcf26f8ed50d48e3d15d918f1ae684eaf7a2f876bd6913c78df59eeebcb9a5078628391c9e8d83430b9cc358a8548d0da6f0783a72743104a91e97c5f701",
            hex::encode(&signed)
        );
    }

    #[test]
    fn test_sign_base() {
        let seed = hex::decode("96063c45132c840f7e1665a3b97814d8eb2586f34bd945f06fa15b9327eebe355f654e81c6233a52149d7a95ea7486eb8d699166f5677e507529482599624cdc").unwrap();
        let path = "m/44'/148'/0'".to_string();
        let base = hex::decode("7ac33997544e3175d266bd022439b22cdb16508c01163f26e5cb2a3e1045a9790000000200000000d4b8322ed2ca75a7a8f7eb57057471b17bd7d5fea4f9a8a293636b4d653fcf3d000027100314996d0000000100000001000000000000000000000000664c6be3000000000000000100000000000000060000000155534443000000003b9911380efe988ba0a8900eb1cfe44f366f7dbe946bed077240f7f624df15c57fffffffffffffff00000000").unwrap();
        let signed = sign_signature_base(&base, &seed, &path).unwrap();
        assert_eq!(
            "baa7bcf26f8ed50d48e3d15d918f1ae684eaf7a2f876bd6913c78df59eeebcb9a5078628391c9e8d83430b9cc358a8548d0da6f0783a72743104a91e97c5f701",
            hex::encode(&signed)
        );
    }
}
