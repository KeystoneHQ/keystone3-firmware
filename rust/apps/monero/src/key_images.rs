use crate::errors::{MoneroError, Result};
use crate::key::{generate_key_image_from_priavte_key, KeyPair, PrivateKey, PublicKey};
use crate::outputs::{ExportedTransferDetail, ExportedTransferDetails};
use crate::utils::{
    constants::*,
    decrypt_data_with_pvk, encrypt_data_with_pvk,
    hash::{hash_to_scalar, keccak256},
    sign::generate_ring_signature,
    varinteger::*,
};
use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;
use curve25519_dalek::edwards::CompressedEdwardsY;
use curve25519_dalek::scalar::Scalar;
use curve25519_dalek::EdwardsPoint;
use hex;
use rand_core::SeedableRng;
use rand_core::{CryptoRng, RngCore};

#[derive(Debug, Clone, Copy)]
pub struct Keyimage(pub [u8; PUBKEY_LEH]);

impl Keyimage {
    pub fn new(key_image: [u8; PUBKEY_LEH]) -> Keyimage {
        Keyimage(key_image)
    }

    pub fn to_point(&self) -> EdwardsPoint {
        CompressedEdwardsY::from_slice(&self.0)
            .unwrap()
            .decompress()
            .unwrap()
    }

    pub fn to_bytes(&self) -> Vec<u8> {
        self.0.to_vec()
    }
}

impl ToString for Keyimage {
    fn to_string(&self) -> String {
        hex::encode(self.0)
    }
}

pub struct KeyImageAndSignature {
    pub image: Keyimage,
    pub signature: [u8; 64],
}

impl KeyImageAndSignature {
    pub fn new(image: [u8; PUBKEY_LEH], signature: [u8; 64]) -> KeyImageAndSignature {
        KeyImageAndSignature {
            image: Keyimage::new(image),
            signature,
        }
    }
}

pub struct KeyImages(Vec<KeyImageAndSignature>);

impl KeyImages {
    pub fn len(&self) -> usize {
        self.0.len()
    }

    pub fn get(&self, index: usize) -> &KeyImageAndSignature {
        &self.0[index]
    }

    pub fn to_bytes(&self) -> Vec<u8> {
        let mut data = vec![];
        for key_image in self.0.iter() {
            data.extend_from_slice(&key_image.image.0);
            data.extend_from_slice(&key_image.signature);
        }
        data
    }
}

impl From<&Vec<u8>> for KeyImages {
    fn from(data: &Vec<u8>) -> Self {
        let mut key_images = vec![];
        let mut i = 0;
        while (data.len() - i) > 64 {
            let image = Keyimage::new(data[i..i + PUBKEY_LEH].try_into().unwrap());
            let signature = data[i + PUBKEY_LEH..i + 96].try_into().unwrap();
            key_images.push(KeyImageAndSignature { image, signature });
            i += 96;
        }
        KeyImages(key_images)
    }
}

fn calc_output_key_offset(
    keypair: &KeyPair,
    tx_pubkey: &[u8; 32],
    internal_output_index: u64,
    major: u32,
    minor: u32,
) -> Scalar {
    let recv_derivation = (keypair.view.scalar
        * PublicKey::from_bytes(tx_pubkey)
            .unwrap()
            .point
            .decompress()
            .unwrap())
    .mul_by_cofactor();

    let mut output_index_buf = vec![0; length(internal_output_index)];
    encode(internal_output_index, &mut output_index_buf);
    let scalar = output_index_buf.to_vec();

    let mut key_offset =
        hash_to_scalar(&[&recv_derivation.compress().0, scalar.as_slice()].concat());

    if major != 0 || minor != 0 {
        key_offset = key_offset + Scalar::from_bytes_mod_order(keypair.get_m(major, minor));
    }

    key_offset
}

fn calc_key_image_private_key(
    keypair: &KeyPair,
    tx_pubkey: &[u8; 32],
    internal_output_index: u64,
    major: u32,
    minor: u32,
) -> PrivateKey {
    let key_offsset =
        calc_output_key_offset(keypair, tx_pubkey, internal_output_index, major, minor);

    let prv_key = keypair.spend.scalar + key_offsset;

    PrivateKey::new(prv_key)
}

fn generate_key_image<R: RngCore + CryptoRng>(
    keypair: &KeyPair,
    tx_pubkey: &[u8; 32],
    pubkey: &[u8; 32],
    additional_tx_keys: Vec<PublicKey>,
    internal_output_index: u64,
    major: u32,
    minor: u32,
    mut rng: R,
) -> KeyImageAndSignature {
    let mut additional_tx_pub_key = None;
    if additional_tx_keys.len() == 1 {
        additional_tx_pub_key = Some(additional_tx_keys[0]);
    } else if !additional_tx_keys.is_empty() {
        if internal_output_index as usize >= additional_tx_keys.len() {
            panic!("Wrong number of additional derivations");
        }
        additional_tx_pub_key = Some(additional_tx_keys[internal_output_index as usize]);
    }

    let key_to_use = match additional_tx_pub_key {
        Some(key) => &key.as_bytes(),
        None => tx_pubkey,
    };

    let prvkey =
        calc_key_image_private_key(keypair, key_to_use, internal_output_index, major, minor);

    let image = generate_key_image_from_priavte_key(&prvkey.clone());

    let signature = generate_ring_signature(
        &image.clone().compress().0,
        &image.clone(),
        vec![PublicKey::from_bytes(pubkey).unwrap()],
        &prvkey,
        0,
        &mut rng,
    );

    let signature = [signature[0][0].to_bytes(), signature[0][1].to_bytes()].concat();

    KeyImageAndSignature::new(image.compress().0, signature.try_into().unwrap())
}

fn generate_key_image_from_offset(
    private_spend_key: &PrivateKey,
    key_offset: &Scalar,
    output_public_key: &PublicKey,
) -> Option<EdwardsPoint> {
    let input_key = private_spend_key.scalar + key_offset;

    if EdwardsPoint::mul_base(&input_key) != output_public_key.point.decompress().unwrap() {
        return None;
    }

    Some(input_key * monero_generators_mirror::hash_to_point(output_public_key.point.to_bytes()))
}

pub fn try_to_generate_image(
    keypair: &KeyPair,
    tx_pubkey: &[u8; 32],
    output_pubkey: &[u8; 32],
    internal_output_index: u64,
    major: u32,
    optional_minors: Vec<u32>,
) -> Result<(Keyimage, Scalar)> {
    for minor in optional_minors {
        let offset =
            calc_output_key_offset(keypair, tx_pubkey, internal_output_index, major, minor);
        match generate_key_image_from_offset(
            &keypair.spend,
            &offset,
            &PublicKey::from_bytes(output_pubkey).unwrap(),
        ) {
            Some(image) => return Ok((Keyimage::new(image.compress().to_bytes()), offset)),
            None => continue,
        };
    }

    Err(MoneroError::GenerateKeyImageError)
}

impl ExportedTransferDetail {
    pub fn key_image<R: RngCore + CryptoRng>(
        &self,
        keypair: &KeyPair,
        rng: R,
    ) -> KeyImageAndSignature {
        generate_key_image(
            keypair,
            &self.tx_pubkey,
            &self.pubkey,
            self.additional_tx_keys.clone(),
            self.internal_output_index,
            self.major,
            self.minor,
            rng,
        )
    }

    pub fn generate_key_image_without_signature(&self, keypair: &KeyPair) -> (PublicKey, Keyimage) {
        let mut additional_tx_pub_key = None;
        let additional_tx_keys = self.additional_tx_keys.clone();
        if additional_tx_keys.len() == 1 {
            additional_tx_pub_key = Some(additional_tx_keys[0]);
        } else if !additional_tx_keys.is_empty() {
            if self.internal_output_index as usize >= additional_tx_keys.len() {
                panic!("Wrong number of additional derivations");
            }
            additional_tx_pub_key = Some(additional_tx_keys[self.internal_output_index as usize]);
        }

        let key_to_use = match additional_tx_pub_key {
            Some(key) => &key.as_bytes(),
            None => &self.tx_pubkey,
        };

        let prvkey = calc_key_image_private_key(
            keypair,
            key_to_use,
            self.internal_output_index,
            self.major,
            self.minor,
        );

        (
            prvkey.get_public_key(),
            Keyimage::new(generate_key_image_from_priavte_key(&prvkey).compress().0),
        )
    }
}

pub fn generate_export_ur_data(keypair: KeyPair, request_data: Vec<u8>) -> Result<Vec<u8>> {
    let decrypted_data = match decrypt_data_with_pvk(
        keypair.view.to_bytes().try_into().unwrap(),
        request_data.clone(),
        OUTPUT_EXPORT_MAGIC,
    ) {
        Ok(data) => data,
        Err(e) => return Err(e),
    };

    if decrypted_data.pk1 != Some(keypair.get_public_spend()) {
        panic!("Public spend key does not match");
    }
    if decrypted_data.pk2 != Some(keypair.get_public_view()) {
        panic!("Public view key does not match");
    }

    let outputs = match ExportedTransferDetails::from_bytes(&decrypted_data.data) {
        Ok(data) => data,
        Err(e) => return Err(e),
    };

    let mut key_images: KeyImages = KeyImages(vec![]);
    let rng_seed = keccak256(request_data.as_slice());
    let mut rng = rand_chacha::ChaCha20Rng::from_seed(rng_seed.try_into().unwrap());
    for output in outputs.details.iter() {
        key_images
            .0
            .push(output.key_image(&keypair.clone(), &mut rng));
    }

    Ok(encrypt_data_with_pvk(
        keypair,
        key_images.to_bytes(),
        KEY_IMAGE_EXPORT_MAGIC,
    ))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_keyimages() {
        let pvk = hex::decode("bb4346a861b208744ff939ff1faacbbe0c5298a4996f4de05e0d9c04c769d501")
            .unwrap();
        let data = hex::decode("4d6f6e65726f206b657920696d616765206578706f727403c2b43f259084a0587d10ce88fddf607949d1448ef00dfad2d82447c669cafc712403d954e3fa901554b67ec80b7dcf1a8b95beaa0d9b27c478b2917ce28934d8252b4903999e98de1bdf03d536fb40893dcaf16b3118a325f261de607ac3c0b7b4b4f11cf3e6e95f8c21756e49287596d4da6997f6943d561f1fbf8f5cffb76274307545a0890da57022e80e31eeb483f27c5ebc6481ca202b7e4431465d40ac54c7b74b439b13de79b3985a01caaadf1f41073f7ddeadf8198eb8d21482ef633313e9ae9feae092118772201ff309b42d364f6e34211a5b4abd13087908989e16ad1bb0fbc87494a86889a8d7a6d6dfcfe078f1f767643dd03bffc8adc18c6736bb03ac8f47f2d708b02d1138427f58270d8176b37a2d970f7ede15b2697f38d0f3dc74e8de8014477cf01c6445047bc6fedebefac3d9619b8ec4f42bbef03140e16fdfb5e836f785f379a6a17f760619f135c9fba9c299d76da04f3cf30868bead8e39fb4b56b2eccfb2fd208cbc47e37a95fbc3084426ef10e19118c4fc080a47052006558f4f7f731e3aafc261bf1136a57d7fb34ceb271f0b5c4b5a5f3e98063aaf50423111bc17f97862921cf9e4e28c83d67f622d54851f4ee81549d857203373cc8c3033494a6a1ccf5779cd4f84c7c1a3b879d640b6916ceef91c441da88c542a3cf3862820da4bc3e634638e969c439641e417982382e6bc71100752afcaed1426881003de976966b707c0e057b73a70e94e5635405a437aab095791b232348e39916e22e9168a948a94b71cbc503065c232f5301840dc8a4cb81ca3eb2a324060e764c15f4a59c9560c67460251ca6095cb6ff959c19851e69f0783667029514612e3c9722cb280e2119b9cbdcbae0b91dcd303ed573edb2af19758b73853954c486c1a4a082ece9c84cc4695cbc3a73caeca2fcdce509df2378c1743edf87239adc846de146e3993b43f58a42d65d64575509c73b2621179b20e4e0edb695a2901f55643a926df34e64de4c4fc514f77e31dbb9b1748834e75a7cbb89d9c21c3842de7cf4a7776936175784c3a93c3486f0ff5cdb0f05cfb01097a5e338522342fa3bba623d5dacd8fc77fd4ea4b552df00e3cfb7f4490f6ba9f8c101cd1a98c4ce8a97ac04111020f0d624f9df35c6764b898ea0826404bfbdb96175e7672089ea0a408095a9ac97c943b9383a08b32f9ae0b8e45edf3659e7f714341139f1073c7be2c46357881cc8373ce5667f36b2bbe5f435ad740884be1a9f1ec1611bee3df4c78d7646e8d83ec03472a2b35711aa0e75bfaafef66a89b934ab487ad01468d5730eb3fe69b4adf86a97adb141d891edffe092a137d7b51a7184624586317e3aae8c6b128516b92462cd5856ab30514f678a9a1aa5469279d7cfee6b86fe2e12c0c383d077940857cf4bc526cb2f947692409de52ec82658819bb6ee78bebc21134aab4c04b0f5bee12ad5e037fd786c70dcfe83f20b7dcf6be765414561167a2bcb0b7ad17906f87fdc913746320bdc3287e6f0417c7d33debc15011706584b9d1acb42b721008447c2434a3a745855408b6bb0f86a00caf06e280b64032623082f3f60d3fd4c62458778cae87198bbd19c185148c0854d532f64593911a85d8f93230b8d49aa7928d220a32bacb93c105983749f5e9f441e903a5b7a1ec2f03d44f5ec06673fffd6b6e7df2cbde6a33aeb2c9d2d892c5d13fb2c7b0e36f1b5b535e860fe34dc0e578bb7d052aa922fb2fec3e8a5df184f24d96ac11f3b35d2e1843077b7bedcf4854a763318443be315e134a15915d360ec47373d00b861dcb8b1303").unwrap();

        let res =
            decrypt_data_with_pvk(pvk.try_into().unwrap(), data, KEY_IMAGE_EXPORT_MAGIC).unwrap();

        assert_eq!(
            hex::encode(res.pk1.unwrap().as_bytes()),
            "fc6339d8849cae36319535ee50950ffc586aca1678529bd79161e158fc1ba298"
        );
        assert_eq!(
            hex::encode(res.pk2.unwrap().as_bytes()),
            "1981d791ec8683dd818a5d7ef99d5fe1ada7fc71f7518d230af1daf12b6debe1"
        );
        assert_eq!(hex::encode(res.data.clone()), "74eb40693189117c28da0c668c349c6b0ef6f666830e5b2d89f00b9376564043d404db8a0b51b53db3a8e6eae4220abc1d0c685fe2503b31e2c4a07de4e8b1044dee522354434680f5975611954ad3a30647e6d46afe5019eb3ce61854118d0cbeb8ac9b3bf90af5a2f5eb6c400a461b9f17a63519ee86f23cc989270f7df5c0dbc454782bde6b3640f5781a8fdd737ed00ba6a09d78b078c3406ea5fbd7b3044da425c1688946b082a180b652c628d0e247c83881e59120c0ede62117b40e05a75182bcaa45f7906dc6588c3b762dd16c74984f710437e24ac5b8800f8567be9ab07c0c25dd6d6fe72e51849a48bd940e2e73a3374e4db72ed177ef34109c0d967f4ce8433e49fb65c6b0235cb6de6665e33386b75cace149228589b321080c3a2f0a5bc7e81226c038b180152c271645d45b7c8a4a63db192a5324b908fe02f767b4053a461aa707af616f3eacc0e002325265d8a65e1d8b136a5fc023f2020df7c8930e9f6df0bd3c271dbf349af021a95c0076460478ccc5e5d730b4bf0ff6911bbb7e37bc1573fddf4551c8b823dbbb20aae1acd487f52e00b0f9b9a0552e8e7a1fa2d3f8daab49f4c3138b36bd6fcb3cf5929f9869041a45be24cecb055e92f17fb399db547618b52d40fd787aeb99c54b656cb7ec95aa8e91440d000342d72f82c2b47e0a7aa430c7f44fc58af7e58c9af085286725051806231165de0290a22df7503c0cf9f14773daabe2aa6c911e8865c62958e594f10954d0030a0d00fc233bc83c4e51ec5ed8399fd22193efa055cc6a508c19007afe7511350271c67b251a02ce9c806e53f5b13d4b7123f0c6da1d0566bfba456ea029ca72a4a46d4a1728475599e6ce13814e41467a1e004d140e60c37cbf4dbc32de628f09e74228205e0b664b283c4268218d7489182876872a7ac8dcaa4ea5bb6b817f05c9e940bff25f013a01f311561554b3f33906fa2bf0fc192faae7aff4d735cd6f6b308c93b4355490f450f1b591a82ccfdc15b32b9cb55e0923bb47bf9dd8ab04df93e6e26ae1dff6d041cdf1daffdfb88a85d814f98ef0fffc41ab600218b70e4bc84a67ac4dd151b98640ebe0fd251ac8f4f40142faef458c870d0dbad0ca7b26eaab5a710566cbb98f050a01c5f4bf67922bd32a894e3bdc9210ba69a3e304be744626741bbb353d94ee7aabe76ba43ab123d6c45b63cf1a0d3089cf50750579b156c82006bba69721bc33f4319174dbb3434108f53dbb4f4482cd269e906c224c6465633e27deea95216e949354c198ec69992830c6d7e01a8206a0fcf70a84ef3125ec4e142335169ac0b0c818f56b691c028a81da769a7c47f05ffc980a78f5808a3a8b3f5ed7ad742ac8a2e90e9dfb39cada70a5ee03d84c9ad96dac5b1a7d400be237275deaee9a7bcdb5419985618f85824a04c11b9282b4b847c508f10c1edc67b29fc46ef6000203a70f20abdb645c6fbe1dd8b72c2d9c93f6910acf71e2b71e50608de9ac3a0019ee6ca8eeb7d47687d03cf471169732153bc6f1ca7f3b96d1b1ed0e15f33dd2adb0cc36023930d36010a3f6ef6da87aa82f510ce8c2e9b0ed542a51972bbb96ac741c79b273bbf1d871bebc5c0d86c2cee1ab08");

        let key_images = KeyImages::from(&res.data);

        assert_eq!(key_images.len(), 12);
        assert_eq!(
            hex::encode(key_images.get(0).image.0),
            "74eb40693189117c28da0c668c349c6b0ef6f666830e5b2d89f00b9376564043"
        );
        assert_eq!(
            hex::encode(key_images.get(0).signature),
            "d404db8a0b51b53db3a8e6eae4220abc1d0c685fe2503b31e2c4a07de4e8b1044dee522354434680f5975611954ad3a30647e6d46afe5019eb3ce61854118d0c"
        );
        assert_eq!(
            hex::encode(key_images.get(1).image.0),
            "beb8ac9b3bf90af5a2f5eb6c400a461b9f17a63519ee86f23cc989270f7df5c0"
        );
        assert_eq!(
            hex::encode(key_images.get(1).signature),
            "dbc454782bde6b3640f5781a8fdd737ed00ba6a09d78b078c3406ea5fbd7b3044da425c1688946b082a180b652c628d0e247c83881e59120c0ede62117b40e05"
        );
        assert_eq!(
            hex::encode(key_images.get(7).image.0),
            "c9e940bff25f013a01f311561554b3f33906fa2bf0fc192faae7aff4d735cd6f"
        );
        assert_eq!(
            hex::encode(key_images.get(9).image.0),
            "79b156c82006bba69721bc33f4319174dbb3434108f53dbb4f4482cd269e906c"
        );
        assert_eq!(
            hex::encode(key_images.get(10).image.0),
            "78f5808a3a8b3f5ed7ad742ac8a2e90e9dfb39cada70a5ee03d84c9ad96dac5b"
        );
        assert_eq!(
            hex::encode(key_images.get(11).image.0),
            "cf71e2b71e50608de9ac3a0019ee6ca8eeb7d47687d03cf471169732153bc6f1"
        );
        assert_eq!(
            hex::encode(key_images.get(11).signature),
            "ca7f3b96d1b1ed0e15f33dd2adb0cc36023930d36010a3f6ef6da87aa82f510ce8c2e9b0ed542a51972bbb96ac741c79b273bbf1d871bebc5c0d86c2cee1ab08"
        );
    }

    #[test]
    fn test_build_key_images_response() {
        let data = hex::decode("4d6f6e65726f206f7574707574206578706f727404a66c8ac44d24aefcdc62411394362a9ca0a5622a0f9be2ea6af704e9ffa43d53a139338aa1ae8b86f8f2c4cef08bed7f059f8ea7adc6760e894acd4f7d67c5b4e60e4fbd16a9f5ba34196b42899a8a1bed460e12d37f6a9e9e57305ab2d227a0ee2142d18444e396e60ad70f8cc8d22f6195391ed8e770755f64dacf9768a34946e1094692ec12dc2dc4430f").unwrap();

        let sec_s_key = PrivateKey::from_bytes(
            &hex::decode("5385a6c16c84e4c6450ec4df4aad857de294e46abf56a09f5438a07f5e167202")
                .unwrap(),
        );
        let sec_v_key = PrivateKey::from_bytes(
            &hex::decode("9128db9621015042d5eb96078b7b86aec79e6fb63b75affbd33138ba25f10d02")
                .unwrap(),
        );
        let keypair = crate::key::KeyPair::new(sec_v_key.clone(), sec_s_key.clone());

        let key_images_export_data =
            generate_export_ur_data(keypair.clone(), data.clone()).unwrap();

        assert_eq!(hex::encode(key_images_export_data), "4d6f6e65726f206b657920696d616765206578706f727403a7f77b9eb360d066d49f2eaa597fe16862b5c1c90eba00af226a1e6c43b774b2b468994d6ff7ee2a7d829812c2d6adedcb9131133f043ff98223531f2b721ff7c1468885baea1a7acd4d6c929ea8ce07161c7f443e9e6ed19677c6c6f53185a50a0418f14ce26d7988c2190e09a04809346d6d7aabdfe929ce88bed228531a44d4c9f1ee2826dcd2f4d78900");
    }

    #[test]
    fn test_include_additional_keys() {
        let sec_s_key = PrivateKey::from_bytes(
            &hex::decode("a57cfb517deb1c35a4b8208847f8e5d54a3a54bc82e72f1b6d21e849934e9e06")
                .unwrap(),
        );
        let sec_v_key = PrivateKey::from_bytes(
            &hex::decode("c665da45f363a80a637740721e97b0c8249fe2599c14eeac73131438c0b92503")
                .unwrap(),
        );
        let keypair = crate::key::KeyPair::new(sec_v_key.clone(), sec_s_key.clone());

        let rng_seed = [0; 32];
        let rng = rand_chacha::ChaCha20Rng::from_seed(rng_seed.try_into().unwrap());
        let ki = generate_key_image(
            &keypair,
            &PublicKey::from_bytes(
                &hex::decode("3e2ebe8773322defc39251cdb18f0500398e525b6720815e95aced3b24375fcc")
                    .unwrap(),
            )
            .unwrap()
            .as_bytes(),
            &PublicKey::from_bytes(
                &hex::decode("2a6780ff9f7ef4b88e60ee4a5731f359a1b3104580bfa304bc534e786af6f74d")
                    .unwrap(),
            )
            .unwrap()
            .as_bytes(),
            vec![
                PublicKey::from_bytes(
                    &hex::decode(
                        "2b6b1b1988a9bc82fe611ccaf699b45cdec3171f6fa3f052790ff46d70aea4ea",
                    )
                    .unwrap(),
                )
                .unwrap(),
                PublicKey::from_bytes(
                    &hex::decode(
                        "36d47bbd7f8ba629faef59ab50f0943f42fd84317d4dd59e65733d8ce1823068",
                    )
                    .unwrap(),
                )
                .unwrap(),
                PublicKey::from_bytes(
                    &hex::decode(
                        "c0d92d8c0104b1a4b9f1b744f4c0cebc3499bf45e32127720040ead9a8867570",
                    )
                    .unwrap(),
                )
                .unwrap(),
            ],
            1,
            0,
            0,
            rng,
        );

        assert_eq!(
            hex::encode(ki.image.0),
            "bd4054a880249da808cf609472f4341b5303cd63fb208f1791492bdd7d7c2a8b"
        );
    }

    #[test]
    fn test_include_additional_keys_full() {
        let sec_s_key = PrivateKey::from_bytes(
            &hex::decode("a57cfb517deb1c35a4b8208847f8e5d54a3a54bc82e72f1b6d21e849934e9e06")
                .unwrap(),
        );
        let sec_v_key = PrivateKey::from_bytes(
            &hex::decode("c665da45f363a80a637740721e97b0c8249fe2599c14eeac73131438c0b92503")
                .unwrap(),
        );
        let keypair = crate::key::KeyPair::new(sec_v_key.clone(), sec_s_key.clone());

        let request = hex::decode("4d6f6e65726f206f7574707574206578706f7274041b341bf5245ce4695c65269995920d92cf58bbc4db89cab0cb54c1e6551ae7873859b62e1c30e0dd941b5152ed0aa9ed9f07d5112801d25c59ae4f4293f2ea350d16f311f57160f1a102290abcd473809e75f48690c75af61645623b2976efb048cd0645765f94b98d336daa2ae84577eaf9bc8bcb1673a8110a361e26eb252a387f168759276b72ffe6bdb53356e27bdb23e94abe7489630d49462ead062e8646e19589dbd7b42374bc03f0a0aa97880aebb4bab8525eb79ed4a60a9b909698e150c68aefa4f1b21f0765ad81a1a129ab04a306cdbf00ac988674c114e88b7b1561d14bca0d3988eae4c465563e8706ef34ec659a244b4e284619c980011dc5a1ec18ceabae38972eb8ddab6547359282fc704d97c21d3dceeed74f2de80c2954ceabc0c7207b2f03fd9a50bcb0bc31f6e3aebeab79f14615bb9d0a9c6d1e7b202abd85004536d5ffdcc9a1df35380c461f1a3b9786b5d511354d4418c12f0ab7a3c61bf16e79b3bb74b72b9f4891b269d49bf3af8fd660cf03348fa71aed636bb7c6ba0f7b34188c1e008a96d930c93f52fccf2c8cd0bd69cac4a75cf0ee96d006824194ef93fbc2f589627e4b4962c5cacf183295f3b9cf7fd5c692a98fd184566229eafc28582822c23a181108e9fe16e993545cd98942acecdf6ab85b4b057de3a3a00b18e499e8314d00c3ca10a39381496e597731f41576a8541ed26e57882d333e7ed68f9c50f34c294b6b18b196c5722dd20d51bbcc4fc13d247f8002af9fd0c6d378dea77d0b04ac8d76b365a84d44a66d5afc7c75736e87e98643a36bdf331b8944206c9ad2a8674fcdcbba44adbb9eb4611ee159a12c6c5c6a32aeb19cb301f1a6d4f1dde50ceafe437a468acb4ba76de67ce48737077d79a6c74465d761d695e4f24f45bcc9190ba1d7441451a1a1eb68e0c376bd459e0129b74319f677adc7b3e45e959a5a6bed9d158eea830fbcf8ee4daee614ac6b7a6a280fdec9406ae724e2574b6368abe0a1b3bf9b3d175f71d85f01c64fd0185a6ee94181a0b7fb9bed43345b211980bb050f8b903a1352d774b187550ea6c98b7fe5df9f3150342d5c52dea4f57cdf17041a31624a195ce91b42f2502af4ca73cea7a95ce539db1fda8bac49e9b380bf51e760b1a4104f7421b98f965267579bf827bc756c8e062daf1113311dbd55457d57755e1d27a932c3711addceddfe79968652ae4db2aabd249e57130ef52fdc3097f2e0287aad3b9b98fcbaada2fc8379b2684137efe2e98c6a4d83f64e5d095e6a730af1243768e7e1b69e1d116fa8c82faf407b3dac920439babfeb433502f276f3a260b012c70a4f4a4ec441e14ad7c909123ebeb81bd3e20dfe5a399e75aa374cb78a28171d0d").unwrap();
        let response = "4d6f6e65726f206b657920696d616765206578706f7274030fa260e0b8424999f9c4400d824de57112f408998e61b5bc174bf0899ecae877f7fd4c621665c9efb650ae75a9d667b78e4e5e985b1d06742f14e2cdbd0d1f0e694701ff48412feadd5d76595ba7d3b1e2ad8c5285e43a5e11674e731b3587d9ecbd19819ef678cd62d2890cb2a88b2296c4d3842b236c6ec108943b7f1d99bded561ac8e90be44cee5133d0d2af73b413d0bf8cec51c20115548998486aedf13ad1fa04967ac9660179e32a15d8287995add8217f8b34640870123755813f0c768016bf55ee83891e4bca53404e0b5ba370ed17e67ce119e3dcd20e4039fa8b4a62d245bf5b75b85fb1aad534ee80f6c76cd1d12fbd570b12e78f12c70af3ba512507bcc2700d6182af7b736581e3b44e98992f4552428f7beff79a64a25899d0d81f891c79b44333963648d125af50f039f22c235cb6675ab636a567ce135f9e9f18c3e64609c49498f274ace86ea36bd37b09032ccf55011e5d9a3d654d95eda7b0cead97fa9f2e038b62e3cc7c97c70e72377f7ef6978f07fbe1d966c573080e38e26239177031aad70be00b24546d7507591d73aa17cf0f8f8636419cc4fba60bc54b9e5d7e780bf61ae273a5d8ce01bea3bfba7c489369a5e8029648ba6a7263ca34ec3bdf0390f61e7ab23c8e630da4afee2b31a47c2b11bb2e2f369c480d13537be479f76f3f7422d2b225af7a9ccf312277395b1afbdc05b50c38f5d6f753c0e7eceb64d8ee77db76eddf05c3f213974c52d21f76be45a44aec7893681c880f886f71b0a7e45f9e7cd4ce826c65ab3fec155c6333db6f770458f840b7287db8a3b63657953e386bf16748842e137a5128249c8618238806a70ec8b13ace8eee64d1d05bfeb0d597a2336d29b58e0dd2ed11d2dacab234d5bfcdfe1cbed511592bc1f7fb610df9c1d281e1d96ebe51590eaea230d7dd9433efc608a36263905936ac27a362c7448b337762cafee262af9ef7a0d37d7763587ddfe7891459f8146b67cb8bd15e261352b12535d0ad3952b37d11194f207b3574559a2ee86e6d4e4ff8c1c9771837ac857f21d8e5079c80d3e2de0339661d56341d7d2375ee8c77cdba7a8990bbb0003e25f413bab2c7a28d57009d537c16821c84fb6afae2fcc428194abf26669605";

        let res = generate_export_ur_data(keypair.clone(), request.clone()).unwrap();

        let response_data = decrypt_data_with_pvk(
            keypair.view.to_bytes(),
            hex::decode(response).unwrap(),
            KEY_IMAGE_EXPORT_MAGIC,
        )
        .unwrap();
        let data =
            decrypt_data_with_pvk(keypair.view.to_bytes(), res, KEY_IMAGE_EXPORT_MAGIC).unwrap();

        assert_eq!(hex::encode(data.data), hex::encode(response_data.data));
    }
}
