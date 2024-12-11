use crate::errors::{MoneroError, Result};
use crate::key::{KeyPair, PrivateKey, PublicKey};
use crate::key_images::Keyimage;
use crate::slow_hash::cryptonight_hash_v0;
use crate::utils::sign::*;
use crate::utils::{constants::*, hash::*};
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;
use chacha20::cipher::{generic_array::GenericArray, KeyIvInit, StreamCipher};
use chacha20::ChaCha20Legacy;
use curve25519_dalek::edwards::EdwardsPoint;
use curve25519_dalek::scalar::Scalar;
use curve25519_dalek::traits::{IsIdentity, MultiscalarMul};
use monero_serai_mirror::transaction::Input;
use rand_core::{CryptoRng, RngCore, SeedableRng};

pub mod constants;
pub mod hash;
pub mod io;
pub mod sign;
pub mod varinteger;

pub struct DecryptUrData {
    pub pk1: Option<PublicKey>,
    pub pk2: Option<PublicKey>,
    pub data: Vec<u8>,
    pub nonce: Option<Vec<u8>>,
    pub magic: String,
    pub signature: Option<Vec<u8>>,
    pub hash: Vec<u8>,
}

pub fn generate_decrypt_key(pvk: [u8; PUBKEY_LEH]) -> [u8; 32] {
    cryptonight_hash_v0(&pvk)
}

pub fn encrypt_data_with_pincode(data: String, pin: [u8; 6]) -> Vec<u8> {
    let pin_hash = keccak256(&pin);
    let key = GenericArray::from_slice(&pin_hash);
    let nonce = GenericArray::from_slice(&[0; 8]);
    let mut cipher = ChaCha20Legacy::new(key, nonce);

    let mut buffer = data.into_bytes();
    cipher.apply_keystream(&mut buffer);

    buffer
}

pub fn decrypt_data_with_pincode(data: Vec<u8>, pin: [u8; 6]) -> String {
    let pin_hash = keccak256(&pin);
    let key = GenericArray::from_slice(&pin_hash);
    let nonce = GenericArray::from_slice(&[0; 8]);
    let mut cipher = ChaCha20Legacy::new(key, nonce);

    let mut buffer = data.clone();
    cipher.apply_keystream(&mut buffer);

    String::from_utf8(buffer).unwrap()
}

pub fn encrypt_data_with_pvk(keypair: KeyPair, data: Vec<u8>, magic: &str) -> Vec<u8> {
    let pvk_hash = cryptonight_hash_v0(&keypair.view.to_bytes());
    let magic_bytes = magic.as_bytes();
    let rng_seed = keccak256(&data.clone());
    let mut rng = rand_chacha::ChaCha20Rng::from_seed(rng_seed);
    let nonce_num = rng.next_u64().to_be_bytes();

    let key = GenericArray::from_slice(&pvk_hash);
    let nonce = GenericArray::from_slice(&nonce_num);
    let mut cipher = ChaCha20Legacy::new(key, nonce);

    let pk1 = keypair.spend.get_public_key();
    let pk2 = keypair.view.get_public_key();

    let mut buffer = Vec::new();
    if magic == KEY_IMAGE_EXPORT_MAGIC {
        buffer.extend_from_slice(&0u32.to_le_bytes());
    }
    if magic == OUTPUT_EXPORT_MAGIC || magic == KEY_IMAGE_EXPORT_MAGIC {
        buffer.extend_from_slice(&pk1.as_bytes());
        buffer.extend_from_slice(&pk2.as_bytes());
    }
    buffer.extend_from_slice(&data.clone());

    cipher.apply_keystream(&mut buffer);

    let mut unsigned_buffer = Vec::new();
    unsigned_buffer.extend_from_slice(&nonce_num.clone());
    unsigned_buffer.extend_from_slice(&buffer.clone());

    let rng_seed = keccak256(&data);
    let mut rng = rand_chacha::ChaCha20Rng::from_seed(rng_seed);

    let signature = generate_signature(
        &keccak256(&unsigned_buffer),
        &keypair.view.get_public_key(),
        &PrivateKey::from_bytes(&keypair.view.to_bytes()),
        &mut rng,
    )
    .unwrap();
    buffer.extend_from_slice(&signature.0);

    [magic_bytes, &nonce_num, &buffer].concat()
}

pub fn decrypt_data_with_pvk(
    pvk: [u8; PUBKEY_LEH],
    data: Vec<u8>,
    magic: &str,
) -> Result<DecryptUrData> {
    if pvk.len() != PUBKEY_LEH {
        return Err(MoneroError::InvalidPrivateViewKey);
    }
    let pvk_hash = cryptonight_hash_v0(&pvk);

    decrypt_data_with_decrypt_key(pvk_hash, pvk, data, magic)
}

pub fn decrypt_data_with_decrypt_key(
    decrypt_key: [u8; PUBKEY_LEH],
    pvk: [u8; PUBKEY_LEH],
    data: Vec<u8>,
    magic: &str,
) -> Result<DecryptUrData> {
    let key = GenericArray::from_slice(&decrypt_key);

    let magic_bytes = magic.as_bytes();

    let mut data = data.clone();
    let mut magic_bytes_found = true;
    for i in 0..magic_bytes.len() {
        if data[i] != magic_bytes[i] {
            magic_bytes_found = false;
            break;
        }
    }

    if magic_bytes_found {
        data = data[magic_bytes.len()..].to_vec();
    }

    let nonce_bytes = data[0..8].to_vec();
    let nonce = GenericArray::from_slice(&nonce_bytes);

    let raw_data = data[0..data.len() - 64].to_vec();
    let data = data[8..].to_vec();

    let mut cipher = ChaCha20Legacy::new(key, nonce);

    let mut buffer = data.clone();

    let signature = buffer[buffer.len() - 64..].to_vec();

    if !check_signature(
        &keccak256(&raw_data.clone()),
        &PrivateKey::from_bytes(&pvk).get_public_key(),
        &Signature(signature.clone().try_into().unwrap()),
    ) {
        return Err(MoneroError::DecryptInvalidSignature);
    }

    cipher.apply_keystream(&mut buffer);

    let start = match magic {
        KEY_IMAGE_EXPORT_MAGIC => 4,
        _ => 0,
    };
    let has_pubilc_keys = magic == OUTPUT_EXPORT_MAGIC || magic == KEY_IMAGE_EXPORT_MAGIC;

    let mut pk1 = None;
    let mut pk2 = None;
    if has_pubilc_keys {
        pk1 = Some(PublicKey::from_bytes(&buffer[start..start + PUBKEY_LEH]).unwrap());
        pk2 = Some(
            PublicKey::from_bytes(&buffer[start + PUBKEY_LEH..start + PUBKEY_LEH * 2]).unwrap(),
        );
    }
    Ok(DecryptUrData {
        pk1,
        pk2,
        data: buffer[(start + if has_pubilc_keys { PUBKEY_LEH * 2 } else { 0 })..buffer.len() - 64]
            .to_vec(),
        nonce: Some(nonce_bytes),
        magic: magic.to_string(),
        signature: Some(signature),
        hash: keccak256(&raw_data).to_vec(),
    })
}

pub fn generate_random_scalar<R: RngCore + CryptoRng>(rng: &mut R) -> Scalar {
    let mut scalar_bytes = [0u8; 64];
    rng.fill_bytes(&mut scalar_bytes);
    Scalar::from_bytes_mod_order_wide(&scalar_bytes)
}

pub fn get_key_image_from_input(input: Input) -> Result<Keyimage> {
    match input {
        Input::ToKey { key_image, .. } => Ok(Keyimage::new(key_image.compress().to_bytes())),
        _ => Err(MoneroError::UnsupportedInputType),
    }
}

pub fn fmt_monero_amount(value: u64) -> String {
    let value = value as f64 / 1_000_000_000_000.0;
    let value = format!("{:.12}", value);
    let value = value.trim_end_matches('0').to_string();
    if value.ends_with('.') {
        format!("{} XMR", value[..value.len() - 1].to_string())
    } else {
        format!("{} XMR", value)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use hex;

    #[test]
    fn test_verify() {
        let res1 = check_signature(
            &hex::decode("57fd3427123988a99aae02ce20312b61a88a39692f3462769947467c6e4c3961").unwrap(),
            &PublicKey::from_bytes(&hex::decode("a5e61831eb296ad2b18e4b4b00ec0ff160e30b2834f8d1eda4f28d9656a2ec75").unwrap()).unwrap(),
            &Signature(hex::decode("cd89c4cbb1697ebc641e77fdcd843ff9b2feaf37cfeee078045ef1bb8f0efe0bb5fd0131fbc314121d9c19e046aea55140165441941906a757e574b8b775c008").unwrap().try_into().unwrap())
        );
        assert!(res1); // true

        let res3 = check_signature(
            &hex::decode("f8628174b471912e7b51aceecd9373d22824065cee93ff899968819213d338c3").unwrap(),
            &PublicKey::from_bytes(&hex::decode("8a7d608934a96ae5f1f141f8aa45a2f0ba5819ad668b22d6a12ad6e366bbc467").unwrap()).unwrap(),
            &Signature(hex::decode("d7e827fbc168a81b401be58c919b7bcf2d7934fe10da6082970a1eb9d98ca609c660855ae5617aeed466c5fd832daa405ee83aef69f0c2661bfa7edf91ca6201").unwrap().try_into().unwrap())
        );
        assert!(res3); // true

        let res5 = check_signature(
            &hex::decode("114e8fffb137c2ce87dd59eff7f4b8e6cc167fdd28c3ea77d345d2c8c00989a1").unwrap(),
            &PublicKey::from_bytes(&hex::decode("d257f46216be34be5589e0b12094e643d1b31bc3c50e006d044d1ea885b5007d").unwrap()).unwrap(),
            &Signature(hex::decode("9579b6e8dc108633ac8b67004699921aef479b6e7ee9590073fbe1404ee4b3d533dec29fd35540f13ac531c3ae49abb62cbc11d36b0cc3353db77a294d8d3d92").unwrap().try_into().unwrap())
        );
        assert!(!res5);

        let res6 = check_signature(
            &hex::decode("ce03e1fa5476167c3ebce1a400ca1d2d375176b5cb9ed180913efa1a688ddc97").unwrap(),
            &PublicKey::from_bytes(&hex::decode("a05a3a6776f85c5d04c42fa2c6a731831c3d3a4e3a12f967f9ba0b1ecd1aee98").unwrap()).unwrap(),
            &Signature(hex::decode("4992de4fec265113710ec3a211e86784581f96241f0305d069a1e4629b504d03b3a1561fd9e73597db89ba00beeb60d2107c1f835176949bd354e8a173d46705").unwrap().try_into().unwrap())
        );
        assert!(res6);

        let res8 = check_signature(
            &hex::decode("6d18e81cf4dcd5dfea5b12c2287ef3317089aa5a5eeb813d4156ea08958db8a3").unwrap(),
            &PublicKey::from_bytes(&hex::decode("6e3649ed3894b5423adecdab1d1782be4640a92ed310aa2199c5861cb3405e96").unwrap()).unwrap(),
            &Signature(hex::decode("e99b6acc2c6169e1635adcfa55777c2c8b3023af17fb4fbcb2ed44435ac6da10afa8743f402cea715f4b59323ca6a3d74df2dfd955194f8c1574e4234ac66700").unwrap().try_into().unwrap())
        );
        assert!(!res8);

        let res9 = check_signature(
            &hex::decode("3b49a4ba1b62db697c7826a66b3a24f5c00054ba8c212ddf6094654059ce973e").unwrap(),
            &PublicKey::from_bytes(&hex::decode("aaa6eebac75c052fdf2abbe18e4718c3b388ff919bf4a514ab61bcac661b4409").unwrap()).unwrap(),
            &Signature(hex::decode("5d156005ee2588edcf470dc653a0635dbf3afc393eb2d89a75054a93b271ee02e46d532ac2d65d7f661113093a68d2ce6516a5abf08231104d0fdcbe6649e80f").unwrap().try_into().unwrap())
        );
        assert!(res9);
    }

    #[test]
    fn test_verify2() {
        let res2 = check_signature(
            &hex::decode("92c1259cddde43602eeac1ab825dc12ffc915c9cfe57abcca04c8405df338359").unwrap(),
            &PublicKey::from_bytes(&hex::decode("9fa6c7fd338517c7d45b3693fbc91d4a28cd8cc226c4217f3e2694ae89a6f3dc").unwrap()).unwrap(),
            &Signature(hex::decode("b027582f0d05bacb3ebe4e5f12a8a9d65e987cc1e99b759dca3fee84289efa5124ad37550b985ed4f2db0ab6f44d2ebbc195a7123fd39441d3a57e0f70ecf608").unwrap().try_into().unwrap())
        );
        assert!(!res2); // false

        let res4 = check_signature(
            &hex::decode("ec9deeaca9ce8f248337213e1411276b9c41e8d4369fc60981b0385653c0f170").unwrap(),
            &PublicKey::from_bytes(&hex::decode("df7f028022cb1b960f2bd740d13c9e44d25c344e57f8978459ffa3c384cd541c").unwrap()).unwrap(),
            &Signature(hex::decode("2c2c8e7c83b662b58e561871f4de4287576946f4e26545ba40e78354c6d0b36f69ea44892f39a46cf3fd5c2813cbc1c525dac199ada6fd5ca8e1e04cff947700").unwrap().try_into().unwrap())
        );
        assert!(!res4); // false

        let res7 = check_signature(
            &hex::decode("7db838c96a3e1fb14156986aef37b70f932ee79d3cbc8233cdd76997eaa0c0c2").unwrap(),
            &PublicKey::from_bytes(&hex::decode("306593abefdbe99beec4752ebb135131a93e8361fc35f60a1c56fc4501c6782f").unwrap()).unwrap(),
            &Signature(hex::decode("5bd47b285d25ede033bc5c2049edf3feb06fe29091e2c90ba25128c6c1a050713f28db1b9106013d22d5e0ba05bbaca43c4d30b6f0bbad8768e6cb89b205c20c").unwrap().try_into().unwrap())
        );
        assert!(!res7);
    }

    #[test]
    fn test_generate_signature() {
        let pvk = hex::decode("bb4346a861b208744ff939ff1faacbbe0c5298a4996f4de05e0d9c04c769d501")
            .unwrap();
        let data = hex::decode("4d6f6e65726f206f7574707574206578706f727404eb5fb0d1fc8358931053f6e24d93ec0766aad43a54453593287d0d3dcfdef9371f411a0e179a9c1b0da94a3fe3d51cccf3573c01b6f8d6ee215caf3238976d8e9af5347e44b0d575fa622accdd4b4d5d272e13d77ff897752f52d7617be986efb4d2b1f841bae6c1d041d6ff9df46262b1251a988d5b0fbe5012d2af7b9ff318381bfd8cbe06af6e0750c16ff7a61d31d36526d83d7b6b614b2fd602941f2e94de01d0e3fc5a84414cdeabd943e5d8f0226ab7bea5e47c97253bf2f062e92a6bf27b6099a47cb8bca47e5ad544049611d77bfeb5c16b5b7849ce5d46bb928ce2e9a2b6679653a769f53c7c17d3e91df35ae7b62a4cffcea2d25df1c2e21a58b1746aae00a273317ec3873c53d8ae71d89d70637a6bd1da974e548b48a0f96d119f0f7d04ff034bb7fed3dbe9081d3e3a3212d330328c0edbacad85bab43780f9b5dfd81f359b0827146ebc421e60dba0badab1941bc31a0086aac99d59f55f07d58c02a48a3e1f70222bae1a612dacd09d0b176345a115e6ae6523ecbc346d8a8078111da7f9932f31d6e35500f5195cfdfe6b6eb2b223d171430a1cb7e11a51ac41d06f3a81546378b1ff342a18fb1f01cfd10df9c1ac86531456f240e5500d9c7ba4c47ba8d4455ea2b7e460ee207c064b76019f6bb4efe5a3e27a126b0c8be6a2e6f3d7ede9580ff49598501aafa36187896e245d64461f9f1c24323b1271af9e0a7a9108422de5ecfdaccdcb2b4520a6d75b2511be6f17a272d21e05ead99818e697559714af0a220494004e393eeefdfe029cff0db22c3adadf6f00edbf6bf4fcbcfc1e225451be3c1c700fe796fce6480b02d0cb1f9fbcf6c05895df2eeb8192980df50a0523922c1247fef83a5f631cf64132125477e1a3b13bcbaa691da1e9b45288eb6c7669e7a7857f87ed45f74725b72b4604fda6b44d3999e1d6fab0786f9b14f00a6518ca3fbc5f865d9fc8acd6e5773208").unwrap();

        let rng_seed = [0u8; 32];
        let mut rng = rand_chacha::ChaCha20Rng::from_seed(rng_seed);

        let res = decrypt_data_with_pvk(
            pvk.clone().try_into().unwrap(),
            data.clone(),
            OUTPUT_EXPORT_MAGIC,
        )
        .unwrap();
        assert_eq!(
            hex::encode(res.hash.clone()),
            "5853d87db51d4d3c0a00b86d06897361b9e49f742fd02988abf6aeca585b988d"
        );
        assert_eq!(
            hex::encode(res.data.clone()),
            "03000707013e8c52245d21b22cbcb90f95270a7937d4974d726209f0a41fdefc7f9df01fde01c8b486383e45d72b841a8b76094dbaa26f9800aac4eaced3bc06122a3380bcf6c666d2281480a0b787e905000000012d58a6378c07f230148c11979cc6e3bec2719f0ec92de21f7fae02029ab025e000f385873857dc102abc6d35c878db7be629646658ae1a418afb27a943f8a2591be4f450e9148094ebdc03000001014ef323a52d2e048594ad73acbe5fb7e588b1859ec9aa02b2670f487660b2700901f485873857dc102abc6d35c878db7be629646658ae1a418afb27a943f8a2591be4f450e914c0b5809ce50500000001cb8ab3c1b4dd10404a4a3c9275a7e2e1e9bf2e4edf1c84f61952bb97965573a300d0c78a38bdd50fdc0367b3141fdc055dec3af5e3ac920dd55816823dfe02f70c3d1816431480c2d72f00000301dd8c2a791056760d903bf06e7930585201e0bd20bcba1e720b85ad0e4d628e4801d1c78a38bdd50fdc0367b3141fdc055dec3af5e3ac920dd55816823dfe02f70c3d18164314a0eec19e03000000019b65ada69049d73e4b049ebd50393410cdc05dad5314690d2b4a36628c4e257600a4909d385d43421399107bd34350b8938f9ff69da18e8f083e6522adf6aa270b3f370ed41480e8eda1ba01000100016311ba60a0a8c636806e232db3e1ad7f79e26df3d24258e264e4351e47f4374d01a5909d385d43421399107bd34350b8938f9ff69da18e8f083e6522adf6aa270b3f370ed414c0c2b383ae04000000"
        );

        let sig = generate_signature(
            &res.hash,
            &res.pk2.unwrap(),
            &PrivateKey::from_bytes(&pvk),
            &mut rng,
        )
        .unwrap();
        assert!(check_signature(&res.hash, &res.pk2.unwrap(), &sig));
    }

    #[test]
    fn test_generate_signature1() {
        let hash = hex::decode("f63c961bb5086f07773645716d9013a5169590fd7033a3bc9be571c7442c4c98")
            .unwrap();
        let pubkey =
            hex::decode("b8970905fbeaa1d0fd89659bab506c2f503e60670b7afd1cb56a4dfe8383f38f")
                .unwrap();
        let prvkey =
            hex::decode("7bb35441e077be8bb8d77d849c926bf1dd0e696c1c83017e648c20513d2d6907")
                .unwrap();

        let mut rng = rand_chacha::ChaCha20Rng::from_seed(hash.clone().try_into().unwrap());

        let sig = generate_signature(
            &hash,
            &PublicKey::from_bytes(&pubkey).unwrap(),
            &PrivateKey::from_bytes(&prvkey),
            &mut rng,
        )
        .unwrap();

        assert!(check_signature(
            &hash,
            &PublicKey::from_bytes(&pubkey).unwrap(),
            &sig
        ));
    }

    #[test]
    fn test_encrypt_data_with_pvk() {
        let sec_s_key = PrivateKey::from_bytes(
            &hex::decode("6ae3c3f834b39aa102158b3a54a6e9557f0ff71e196e7b08b89a11be5093ad03")
                .unwrap(),
        );
        let sec_v_key = PrivateKey::from_bytes(
            &hex::decode("bb4346a861b208744ff939ff1faacbbe0c5298a4996f4de05e0d9c04c769d501")
                .unwrap(),
        );
        let keypair = crate::key::KeyPair::new(sec_v_key.clone(), sec_s_key.clone());

        let data = hex::decode("03000707013e8c52245d21b22cbcb90f95270a7937d4974d726209f0a41fdefc7f9df01fde01c8b486383e45d72b841a8b76094dbaa26f9800aac4eaced3bc06122a3380bcf6c666d2281480a0b787e905000000012d58a6378c07f230148c11979cc6e3bec2719f0ec92de21f7fae02029ab025e000f385873857dc102abc6d35c878db7be629646658ae1a418afb27a943f8a2591be4f450e9148094ebdc03000001014ef323a52d2e048594ad73acbe5fb7e588b1859ec9aa02b2670f487660b2700901f485873857dc102abc6d35c878db7be629646658ae1a418afb27a943f8a2591be4f450e914c0b5809ce50500000001cb8ab3c1b4dd10404a4a3c9275a7e2e1e9bf2e4edf1c84f61952bb97965573a300d0c78a38bdd50fdc0367b3141fdc055dec3af5e3ac920dd55816823dfe02f70c3d1816431480c2d72f00000301dd8c2a791056760d903bf06e7930585201e0bd20bcba1e720b85ad0e4d628e4801d1c78a38bdd50fdc0367b3141fdc055dec3af5e3ac920dd55816823dfe02f70c3d18164314a0eec19e03000000019b65ada69049d73e4b049ebd50393410cdc05dad5314690d2b4a36628c4e257600a4909d385d43421399107bd34350b8938f9ff69da18e8f083e6522adf6aa270b3f370ed41480e8eda1ba01000100016311ba60a0a8c636806e232db3e1ad7f79e26df3d24258e264e4351e47f4374d01a5909d385d43421399107bd34350b8938f9ff69da18e8f083e6522adf6aa270b3f370ed414c0c2b383ae0400000063c57cc457a1485fc5f8e6dfc8b70430f41946a7d0cd51e84ef5ac819ff2b2c4bcec6f1e6dd57e7e791d8cca2091169bba53496d72375331f8d56cd33f5e0ca4").unwrap();
        let magic = OUTPUT_EXPORT_MAGIC;
        let bin_data = encrypt_data_with_pvk(keypair, data.clone(), magic);

        let keypair = crate::key::KeyPair::new(sec_v_key, sec_s_key);

        let res = decrypt_data_with_pvk(keypair.view.to_bytes(), bin_data.clone(), magic).unwrap();

        assert_eq!(hex::encode(res.data.clone()), hex::encode(data));
    }

    #[test]
    fn test_cryptonight_hash_v0() {
        let sec_v_key = PrivateKey::from_bytes(
            &hex::decode("bb4346a861b208744ff939ff1faacbbe0c5298a4996f4de05e0d9c04c769d501")
                .unwrap(),
        );
        let pvk_hash = cryptonight_hash_v0(&sec_v_key.to_bytes());

        assert_eq!(
            hex::encode(pvk_hash),
            "87ebc685e15f646cfd4c2fe94cb8325748fdc3e01e360bd474ff554edff370e6"
        );
    }

    #[test]
    fn test_fmt_monero_amount() {
        let amount = 10000000000001;
        let res = fmt_monero_amount(amount);
        assert_eq!(res, "10.000000000001 XMR");

        let amount = 0000000000001;
        let res = fmt_monero_amount(amount);
        assert_eq!(res, "0.000000000001 XMR");

        let amount = 1000000000000000000;
        let res = fmt_monero_amount(amount);
        assert_eq!(res, "1000000 XMR");

        let amount = 1000000000000001;
        let res = fmt_monero_amount(amount);
        assert_eq!(res, "1000.000000000001 XMR");
    }

    #[test]
    fn test_encrypt_data_with_pincode() {
        let data = "4Azmp3phZDz1Ae9g14Zp7mjVDko1qQRby76AkGp49J5j4tffM3rEG3jRgXCWNfSdLb7hhK87KSRWn9Fa66AbQTtdDWLVo9i";
        let res = encrypt_data_with_pincode(data.to_string(), [1, 2, 3, 4, 5, 6]);

        assert_eq!(
            hex::encode(res.clone()),
            "dce6e04f6a60fb15d3b00e8ef5e7252f8d7b39220fceff90f7aa82a15cdc9e60a4b4e979ab694355df405021bafde913739ddd82d5fdeef1f1c8b4198a833e204ee1ecc9a2641f9a5e121d6e312223170e3bb07c9aca199cadcb599f01caeb"
        );

        let res2 = decrypt_data_with_pincode(res, [1, 2, 3, 4, 5, 6]);

        assert_eq!(res2, data);
    }
}
