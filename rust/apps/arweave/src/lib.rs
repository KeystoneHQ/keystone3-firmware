#![no_std]

pub mod ao_transaction;
pub mod data_item;
pub mod deep_hash;
pub mod errors;
mod tokens;
pub mod transaction;

#[macro_use]
extern crate alloc;

use crate::errors::{ArweaveError, Result};
use aes::cipher::block_padding::Pkcs7;
use aes::cipher::{generic_array::GenericArray, BlockDecryptMut, BlockEncryptMut, KeyIvInit};
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use data_item::DataItem;

use keystore::algorithms::rsa::get_rsa_secret_from_seed;
use rsa::{BigUint, RsaPrivateKey};

use serde_json::{json, Value};

use sha2::Digest;
use transaction::{Base64, Transaction};

type Aes256CbcEnc = cbc::Encryptor<aes::Aes256>;
type Aes256CbcDec = cbc::Decryptor<aes::Aes256>;

pub fn aes256_encrypt(key: &[u8], iv: &[u8], data: &[u8]) -> Result<Vec<u8>> {
    let iv = GenericArray::from_slice(iv);
    let key = GenericArray::from_slice(key);

    let ct = Aes256CbcEnc::new(key, iv).encrypt_padded_vec_mut::<Pkcs7>(data);
    Ok(ct)
}

pub fn aes256_decrypt(key: &[u8], iv: &[u8], data: &[u8]) -> Result<Vec<u8>> {
    let iv = GenericArray::from_slice(iv);
    let key = GenericArray::from_slice(key);

    match Aes256CbcDec::new(key, iv).decrypt_padded_vec_mut::<Pkcs7>(data) {
        Ok(pt) => Ok(pt),
        Err(e) => Err(ArweaveError::KeystoreError(format!(
            "aes256_decrypt failed {:?}",
            e.to_string()
        ))),
    }
}

pub fn generate_address(owner: Vec<u8>) -> Result<String> {
    let mut hasher = sha2::Sha256::new();
    hasher.update(owner);
    let owner_base64url_sha256 = hasher.finalize();
    Ok(base64_url(owner_base64url_sha256.to_vec()))
}

pub fn base64_url(hash: Vec<u8>) -> String {
    let address = base64::encode_config(hash.as_slice(), base64::URL_SAFE_NO_PAD);
    address
}

pub fn generate_public_key_from_primes(p: &[u8], q: &[u8]) -> Result<Vec<u8>> {
    let p = BigUint::from_bytes_be(p);
    let q = BigUint::from_bytes_be(q);
    let n = p * q;
    Ok(n.to_bytes_be())
}

pub fn generate_secret(seed: &[u8]) -> Result<RsaPrivateKey> {
    match get_rsa_secret_from_seed(seed) {
        Ok(secret) => Ok(secret),
        Err(e) => Err(ArweaveError::KeystoreError(format!(
            "generate secret failed {:?}",
            e.to_string()
        ))),
    }
}

fn u64_to_ar(value: u64) -> String {
    let value = value as f64 / 1_000_000_000_000.0;
    let value = format!("{value:.12}");
    let value = value.trim_end_matches('0').to_string();
    if value.ends_with('.') {
        format!("{} AR", &value[..value.len() - 1])
    } else {
        format!("{value} AR")
    }
}

pub fn owner_to_address(owner: Base64) -> Result<String> {
    let owner_bytes = owner.clone().0;
    let address = generate_address(owner_bytes)?;
    Ok(fix_address(&address))
}

pub fn fix_address(address: &str) -> String {
    let mut result = String::new();
    let mut count = 0;
    for c in address.chars() {
        result.push(c);
        count += 1;
        if count == 25 {
            result.push('\n');
            count = 0;
        }
    }
    result
}

pub fn parse(data: &Vec<u8>) -> Result<String> {
    let tx = match serde_json::from_slice::<Transaction>(data) {
        Ok(tx) => {
            let tags_json = tx
                .tags
                .iter()
                .map(|tag| {
                    let name =
                        base64::encode_config(tag.name.0.as_slice(), base64::URL_SAFE_NO_PAD);
                    let value =
                        base64::encode_config(tag.value.0.as_slice(), base64::URL_SAFE_NO_PAD);
                    json!({
                        "name": name,
                        "value": value
                    })
                })
                .collect::<Vec<Value>>();
            let tags_json = serde_json::to_string(&tags_json).unwrap();
            json!({
                "raw_json": tx,
                "formatted_json": {
                    "detail": tags_json,
                    "owner": tx.owner,
                    "from": owner_to_address(tx.owner.clone())?,
                    "target": fix_address(&tx.target.clone().to_string()),
                    "quantity": u64_to_ar(tx.quantity),
                    "reward": u64_to_ar(tx.reward),
                    "data_size": tx.data_size,
                    "signature_data": tx.deep_hash().map_or_else(|e| format!("unable to deep hash transaction, reason: {e}"), hex::encode),
                },
                "status": "success"
            })
        }
        Err(e) => {
            let readable = format!("unable to deserialize, reason: {e}");
            json!({
                "status": "failed",
                "reason": readable
            })
        }
    };
    Ok(tx.to_string())
}

pub fn parse_data_item(serial: &[u8]) -> Result<DataItem> {
    DataItem::deserialize(serial)
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::borrow::ToOwned;
    use hex::ToHex;
    use {hex, rsa::PublicKeyParts};

    #[test]
    fn test_generate_address() {
        let seed = hex::decode("2f1986623bdc5d4f908e5be9d6fa00ec").unwrap();
        let result = generate_secret(seed.as_slice()).unwrap();
        let address = generate_address(result.n().to_bytes_be()).unwrap();
        assert_eq!(address, "ICwtdLdGrJJ5bIe7rTWS1dd2_8tpOv1ZZIKnChvb19Y");
    }

    #[test]
    fn test_generate_secret() {
        let seed = hex::decode("2f1986623bdc5d4f908e5be9d6fa00ec").unwrap();
        let p = hex::decode("f1fc92541273845b7b55d125b99839306d6815ccf905ab80daff13794f40616f8653c3356e25ec4fb899e12b3147a66ddb5c2b8daf8eb8a72909709d05e69c39c16742afdcae9899d97d8619bee42342deabc75b30c2673d6f4c981eb17c00e11b2621ed89f46772132ed56907027c8e4b2ae5e3d86702ee0d8060ed7e143c60793fcacd61dc31d3021637e13f0724e66baf92aeb321620136f4357c365cf6a7ec96feaa0428b7bfac7c82d6e35fecaf4b4c0dcd8531e4ac5c1db29296e5dbe557aa1be4b9da853ae4543bc8254bb77fdabc617434f23cae4b08b68c9a5c467ef46198f1cb76702d82cb2a4dd7aa29dcef478f9dc5feb8b43eddb5c5683ca027").unwrap();
        let q = hex::decode("ed7d44523fa5fe95c1d084261329296ca8e55cb26d9f6742be0ecd9996f1aacaa22b5acd1208a118e1b7da83d9052cc76b67966f616ceb02e33cb0d2d9a4e5b4cf8e0ee68272d528f5a5ad2d75ee275c9cc3cd061c17bc9517606a74d749b9b1156f58647645326130109e1d32c00472026794ce45dd2225c4a93b09b0bf05d0369aff2692038d040553aa7ea059e6610a084e34a20a3b9ebff2bb586b78aae8ebc15621f8e7bcb2fa2cd6b51a63d42aebad67b17c200a7515d89f8fda4380e3815bbae1a4bdda1b6ceddeeeaf68bd27ce38a5ec46ac43c77f0c7392c4196260992df9393965676469ee8c34f2711af6c88e2d96702857eb08f9cc65ca361fa5").unwrap();
        let result = generate_secret(seed.as_slice()).unwrap();

        let public_key = generate_public_key_from_primes(p.as_slice(), q.as_slice()).unwrap();

        assert_eq!(
            hex::encode(result.primes()[0].to_bytes_be()),
            hex::encode(&p)
        );
        assert_eq!(
            hex::encode(result.primes()[1].to_bytes_be()),
            hex::encode(&q)
        );
        assert_eq!(
            result.n().to_owned().to_bytes_be(),
            BigUint::from_bytes_be(public_key.as_slice()).to_bytes_be()
        );
    }

    #[test]
    fn test_aes128() {
        let key = hex::decode("5eb00bbddcf069084889a8ab915556815eb00bbddcf069084889a8ab91555681")
            .unwrap();
        let iv = hex::decode("65f5c453ccb85e70811aaed6f6da5fc1").unwrap();
        let data = hex::decode("fdec3a1aee520780ca4058402d0422b5cd5950b715728f532499dd4bbcb68e5d44650818b43656782237316c4b0e2faa2b15c245fb82d10cf4f5b420f1f293ba75b2c8d8cef6ad899c34ce9de482cb248cc5ab802fd93094a63577590d812d5dd781846ef7d4f5d9018199c293966371c2349b0f847c818ec99caad800116e02085d35a39a913bc735327705161761ae30a4ec775f127fbb5165418c0fe08e54ae0aff8b2dab2b82d3b4b9c807de5fae116096075cf6d5b77450d743d743e7dcc56e7cafdcc555f228e57b363488e171d099876993e93e37a94983ccc12dba894c58ca84ac154c1343922c6a99008fabd0fa7010d3cc34f69884fec902984771c5b50031ba31ab7c8b76453ce771f048b84fb89a3e4d44c222c3d8c823c683988b0dbf354d8b8cbf65f3db53e1365d3c5e043f0155b41d1ebeca6e20b2d6778600b5c98ffdba33961dae73b018307ef2bce9d217bbdf32964080f8db6f0cf7ef27ac825fcaf98d5143690a5d7e138f4875280ed6de581e66ed17f83371c268a073e4594814bcc88a33cbb4ec8819cc722ea15490312b85fed06e39274c4f73ac91c7f4d1b899729691cce616fb1a5feee1972456addcb51ac830e947fcc1b823468f0eefbaf195ac3b34f0baf96afc6fa77ee2e176081d6d91ce8c93c3d0f3547e48d059c9da447ba05ee3984703bebfd6d704b7f327ffaea7d0f63d0d3c6d65").unwrap();
        let encrypted_data =
            aes256_encrypt(key.as_slice(), iv.as_slice(), data.as_slice()).unwrap();
        assert_eq!(encrypted_data.len(), 528);
        let decrypted_data =
            aes256_decrypt(key.as_slice(), iv.as_slice(), encrypted_data.as_slice()).unwrap();
        assert_eq!(data, decrypted_data);
    }

    #[test]
    fn test_parse_tx_without_data() {
        // {"format":2,"id":"ydBnEDXkltirHF_89nDaAeJQsyh6ocVFH6w3bGdb4EQ","last_tx":"IgIzkTrNA1SG-aYLVGQM1eUEm_b-7Jq3F8_BCU_j6Uv25rz80zFGgC4Qd_Qazsy-","owner":"sSWWDVR8jBbeoGemy4M1VuhUM_0CusXRXdTavi46PUVcP814Km3szmVlk71l_-rBYjXNyrMr25_LWMHEOIQyfq2Dn-29iBbYqk608SGj3YHI2LnR6z3dxLBVxwT5atTu5iZnMfZ--NQinA64iIGrTQ3nG2A31s5lDT58VQsX7AF_eaVCwWYoyCKvUjKeduNmUSIjGl7sitr-tooTdcbAhshka55LCqd6brO24ZzB0iieWwrwAyN4LZpcjx36dMaR9q1L5nY5d-BnSZhVk2-_A8S-P7WfRrudTSUZMF8fV9o2Cd55T-h5wlHP5Xxh5BO45TZemwFRzFKX3qIEwKNZx_uCDhTlMmKpxtMaj2i0-gxI1QeXrHYv76KfgZy2U2XMW2H4Mpfr_WO2KM2_b6cWQUenPDnqzgRXsq6GXdHUFgV-qi1M_i4MCE6AD5-QlPQ_QcBa_ZfldowFL2PAC_hykEyDfRS7Mwx_Fw47g70sVbWE1DP9MEfxGC9vmOiDedroG2EqvT0VpM-xIzzhpGPWH_zNFcNSqj8s_zSqqtnXtlcTtbk76IpK8tKjDfIHq1JHZg__wobyfM0fiAqH6fJatUCLgWPGTbJ9M46uJw8bFI72py_2MdrxBkW-sNBoa21_g-6FUNH4dHWA8gGEwka5-BMnvaMYpAKAqQ2-GGYw1pE","tags":[{"name":"QXBwLU5hbWU","value":"QXJDb25uZWN0"},{"name":"QXBwLVZlcnNpb24","value":"MC40LjI"},{"name":"Q29udGVudC1UeXBl","value":"dGV4dC9wbGFpbg"}],"target":"gH04U_MDvhKdEeSVHqFnkx7xi4dsuKj94O9qRh50LSQ","quantity":"10000000","data":"","data_size":"0","data_root":"","reward":"1410507854"}
        let tx_bytes = hex::decode("7b22666f726d6174223a322c226964223a227964426e4544586b6c74697248465f38396e446141654a51737968366f6356464836773362476462344551222c226c6173745f7478223a224967497a6b54724e413153472d61594c5647514d316555456d5f622d374a713346385f4243555f6a3655763235727a38307a464767433451645f51617a73792d222c226f776e6572223a2273535757445652386a4262656f47656d79344d31567568554d5f304375735852586454617669343650555663503831344b6d33737a6d566c6b37316c5f2d7242596a584e79724d7232355f4c574d48454f495179667132446e2d323969426259716b36303853476a33594849324c6e52367a3364784c4256787754356174547535695a6e4d665a2d2d4e51696e413634694947725451336e473241333173356c44543538565173583741465f656156437757596f79434b76556a4b6564754e6d5553496a476c37736974722d746f6f54646362416873686b6135354c4371643662724f32345a7a42306969655777727741794e344c5a70636a783336644d61523971314c356e5935642d426e535a68566b322d5f4138532d50375766527275645453555a4d46386656396f3243643535542d6835776c48503558786835424f3435545a656d7746527a464b5833714945774b4e5a785f75434468546c4d6d4b7078744d616a3269302d677849315165587248597637364b66675a79325532584d573248344d7066725f574f324b4d325f623663575155656e50446e717a67525873713647586448554667562d7169314d5f69344d4345364144352d516c50515f516342615f5a666c646f77464c325041435f68796b457944665253374d77785f467734376737307356625745314450394d456678474339766d4f69446564726f4732457176543056704d2d78497a7a6870475057485f7a4e46634e53716a38735f7a537171746e58746c635474626b373649704b38744b6a4466494871314a485a675f5f776f6279664d30666941714836664a617455434c6757504754624a394d3436754a7738624649373270795f324d647278426b572d734e426f6132315f672d3646554e48346448574138674745776b61352d424d6e76614d5970414b417151322d47475977317045222c2274616773223a5b7b226e616d65223a22515842774c553568625755222c2276616c7565223a2251584a44623235755a574e30227d2c7b226e616d65223a22515842774c565a6c636e4e70623234222c2276616c7565223a224d4334304c6a49227d2c7b226e616d65223a225132397564475675644331556558426c222c2276616c7565223a226447563464433977624746706267227d5d2c22746172676574223a2267483034555f4d4476684b64456553564871466e6b78377869346473754b6a39344f3971526835304c5351222c227175616e74697479223a223130303030303030222c2264617461223a22222c22646174615f73697a65223a2230222c22646174615f726f6f74223a22222c22726577617264223a2231343130353037383534227d").unwrap();
        let tx = parse(&tx_bytes).unwrap();
        let parsed_tx: Value = serde_json::from_str(&tx).unwrap();
        assert_eq!(parsed_tx["formatted_json"]["signature_data"], "87a9bb37c154b9dc27b4203c54b68301df51811383da8cd217a163356b3fa6325bc9ab6dd6e2f63f1200ac295646fc98");
    }

    #[test]
    fn test_parse_tx_with_data_1() {
        // let tx_bytes = hex::decode("3f45ca7ce6f2600d2a0b9a3710e9c8924d9a2b19fda2cf42d3e3b1aa2bd010d92b8bd4d2db72bfff1c8a168204fda5b6").unwrap();
        let tx_bytes = hex::decode("7b22666f726d6174223a322c226964223a22222c226c6173745f7478223a224b797341664d666231784a6b316a653263783974463065514a736a486f685f72524f435a6558304739434e6f4736533665594c7043754c5f4a6d444d5a6e474b222c226f776e6572223a227730536a66512d6947784a5a465448496c6f69464734614878414b6f507948793154454e75646936456c695369504273436764674e6a6c37632d4b6374506b416f3046677759643231506e584834484c7747485974506a333874546e514f7643727552756d776e2d33454f53756a39656b503279696b35486c577a384a496339755f6944435a766961346369354e6a325f33505f324c304e674e7056766a6f727853447673362d483746746f6f614d53774d48774c36684f4745646d726c6969484f5a37704c49696f71624156346a37474a70744451373772554b54674e436f354951377a4b6745756765474b537064354b6849544e734477676e6769542d49514b38424c637a3747566761546a7a6f53747a513338576c68454750764c7472735a4154424452567878577a653973746a4534344f30456f4e5a64795442455239687863683561465a4b4e6b546541777954506838534d303276774952367469416c37793757576d65474576366e75454c47624f5461504f52356242543348775034714a36664941464d624f684b50384467506c555a354f654f64634b5644486c4f65626f7368533231655948496e7775647561737a6474334630457a76477957364335434a7330727a34776f4a62636947564c31637435496777374e4451636d7230584e72666b552d58634e3636526c45466633744158783655426651307669644e557a3678556e474e58726b2d694a36476163656a7679352d4e6d35736a3433725f4f44374f4b376662787137383377764d50776f50735965624569582d6351504435355f4e6b68374d3054396268734d4d66464f6f6237525853496637617061446b73477661325f536d787a35485a4861787a6178304a68374d2d67695678696a78744439504b744734756d374c4c393956503756596146703049306f50375546476335526a4955222c2274616773223a5b5d2c22746172676574223a2247556937747151337a4a5732435779773245527777756e4357336f576f493548417369454e4872527a3938222c227175616e74697479223a223130303030303030303030222c2264617461223a22222c22646174615f73697a65223a2230222c22646174615f726f6f74223a22222c22726577617264223a22343632323430222c227369676e6174757265223a22227d").unwrap();
        let tx = parse(&tx_bytes).unwrap();
        let parsed_tx: Value = serde_json::from_str(&tx).unwrap();
        assert_eq!(parsed_tx["formatted_json"]["signature_data"], "68447623f73b73251aa62405b62b4493e05ad0ba90b48f6bd16ae8c59812b5344a8c53aec25a7f972cff9c2edeba8ae0");
    }

    #[test]
    fn test_parse_tx_with_data_2() {
        let tx_bytes = hex::decode("7b22666f726d6174223a322c226964223a22222c226c6173745f7478223a22454f62386c626f6b634633655f515f685737346e5959365f4c62694f676254345f4a2d6530333670746137794f585f6b517a4a66734d724c7466474e53626a66222c226f776e6572223a227730536a66512d6947784a5a465448496c6f69464734614878414b6f507948793154454e75646936456c695369504273436764674e6a6c37632d4b6374506b416f3046677759643231506e584834484c7747485974506a333874546e514f7643727552756d776e2d33454f53756a39656b503279696b35486c577a384a496339755f6944435a766961346369354e6a325f33505f324c304e674e7056766a6f727853447673362d483746746f6f614d53774d48774c36684f4745646d726c6969484f5a37704c49696f71624156346a37474a70744451373772554b54674e436f354951377a4b6745756765474b537064354b6849544e734477676e6769542d49514b38424c637a3747566761546a7a6f53747a513338576c68454750764c7472735a4154424452567878577a653973746a4534344f30456f4e5a64795442455239687863683561465a4b4e6b546541777954506838534d303276774952367469416c37793757576d65474576366e75454c47624f5461504f52356242543348775034714a36664941464d624f684b50384467506c555a354f654f64634b5644486c4f65626f7368533231655948496e7775647561737a6474334630457a76477957364335434a7330727a34776f4a62636947564c31637435496777374e4451636d7230584e72666b552d58634e3636526c45466633744158783655426651307669644e557a3678556e474e58726b2d694a36476163656a7679352d4e6d35736a3433725f4f44374f4b376662787137383377764d50776f50735965624569582d6351504435355f4e6b68374d3054396268734d4d66464f6f6237525853496637617061446b73477661325f536d787a35485a4861787a6178304a68374d2d67695678696a78744439504b744734756d374c4c393956503756596146703049306f50375546476335526a4955222c2274616773223a5b7b226e616d65223a226447567a6441222c2276616c7565223a22625756755a334a31227d5d2c22746172676574223a2247556937747151337a4a5732435779773245527777756e4357336f576f493548417369454e4872527a3938222c227175616e74697479223a223130303030303030303030222c2264617461223a22222c22646174615f73697a65223a2230222c22646174615f726f6f74223a22222c22726577617264223a22343534353336222c227369676e6174757265223a22227d").unwrap();
        let tx = parse(&tx_bytes).unwrap();
        let parsed_tx: Value = serde_json::from_str(&tx).unwrap();
        assert_eq!(parsed_tx["formatted_json"]["signature_data"], "56149d23cc905f5249c0cb80728c841657a065d05ed249f42a670aa9239c9005a2b35a9eb39252de429442e6fbda9f20");
    }

    #[test]
    fn test_u64_to_ar() {
        let value = 1000000000000;
        let ar = u64_to_ar(value);
        assert_eq!(ar, "1 AR");

        let value = 1000000000001;
        let ar = u64_to_ar(value);
        assert_eq!(ar, "1.000000000001 AR");

        let value = 1000000000000000000;
        let ar = u64_to_ar(value);
        assert_eq!(ar, "1000000 AR");
    }
}
