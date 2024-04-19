#![no_std]

extern crate alloc;

use alloc::boxed::Box;
use alloc::string::ToString;
use app_arweave::{generate_public_key_from_primes, generate_secret, aes256_encrypt, aes256_decrypt};
use keystore::algorithms::ed25519::slip10_ed25519::get_private_key_by_seed;
use common_rust_c::structs::SimpleResponse;
use common_rust_c::utils::{convert_c_char, recover_c_char};
use common_rust_c::types::{PtrBytes, PtrString};
use cty::c_char;
use third_party::hex;
use core::slice;

fn generate_aes_key_iv(seed: &[u8]) -> ([u8; 32], [u8; 16]) {
    let key_path = "m/0'".to_string();
    let iv_path = "m/1'".to_string();
    let key = get_private_key_by_seed(seed, &key_path).unwrap();
    let (_, key_bytes) = third_party::cryptoxide::ed25519::keypair(&key);
    let iv = get_private_key_by_seed(seed, &iv_path).unwrap();
    let (_, iv) = third_party::cryptoxide::ed25519::keypair(&iv);
    let mut iv_bytes: [u8; 16] = [0; 16];
    iv_bytes.copy_from_slice(&iv[..16]);
    (key_bytes, iv_bytes)
}

#[no_mangle]
pub extern "C" fn generate_arweave_secret(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<u8> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let secret = generate_secret(seed).unwrap();
    let mut secret_bytes: [u8; 512] = [0; 512];
    secret_bytes[..256].copy_from_slice(&secret.primes()[0].to_bytes_be());
    secret_bytes[256..].copy_from_slice(&secret.primes()[1].to_bytes_be());

    SimpleResponse::success(Box::into_raw(Box::new(secret_bytes)) as *mut u8).simple_c_ptr()
}

#[no_mangle]
pub extern "C" fn generate_arweave_public_key_from_primes(
    p: PtrBytes,
    p_len: u32,
    q: PtrBytes,
    q_len: u32,
) -> *mut SimpleResponse<u8> {
    let p = unsafe { slice::from_raw_parts(p, p_len as usize) };
    let q = unsafe { slice::from_raw_parts(q, q_len as usize) };
    let public = generate_public_key_from_primes(p, q).unwrap();
    return SimpleResponse::success(Box::into_raw(Box::new(public)) as *mut u8).simple_c_ptr();
}

#[no_mangle]
pub extern "C" fn generate_rsa_public_key(
    p: PtrBytes,
    p_len: u32,
    q: PtrBytes,
    q_len: u32,
) -> *mut SimpleResponse<c_char> {
    let p = unsafe { slice::from_raw_parts(p, p_len as usize) };
    let q = unsafe { slice::from_raw_parts(q, q_len as usize) };
    let public = generate_public_key_from_primes(p, q).unwrap();
    return SimpleResponse::success(convert_c_char(hex::encode(public))).simple_c_ptr();
}

#[no_mangle]
pub extern "C" fn aes256_encrypt_primes(
    seed: PtrBytes,
    seed_len: u32,
    data: PtrBytes,
) -> *mut SimpleResponse<u8> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let data = unsafe { slice::from_raw_parts(data, 512) };
    let (key, iv) = generate_aes_key_iv(seed);
    let encrypted_data = aes256_encrypt(&key, &iv, data).unwrap();
    let mut result_bytes: [u8; 528] = [0; 528];
    result_bytes.copy_from_slice(&encrypted_data);
    return SimpleResponse::success(Box::into_raw(Box::new(result_bytes)) as *mut u8).simple_c_ptr();
}

#[no_mangle]
pub extern "C" fn aes256_decrypt_primes(
    seed: PtrBytes,
    seed_len: u32,
    data: PtrBytes,
) -> *mut SimpleResponse<u8> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let data = unsafe { slice::from_raw_parts(data, 528) };
    let (key, iv) = generate_aes_key_iv(seed);
    match aes256_decrypt(&key, &iv, data) {
        Ok(decrypted_data) => {
            let mut result_bytes: [u8; 512] = [0; 512];
            result_bytes.copy_from_slice(&decrypted_data);
            SimpleResponse::success(Box::into_raw(Box::new(result_bytes)) as *mut u8).simple_c_ptr()
        },
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn arweave_get_address(
    xpub: PtrString,
) -> *mut SimpleResponse<c_char> {
    let xpub = recover_c_char(xpub);
    let address = app_arweave::generate_address(hex::decode(xpub).unwrap()).unwrap();
    return SimpleResponse::success(convert_c_char(address)).simple_c_ptr();
}

#[cfg(test)]
mod tests {
    use super::*;
    extern crate std;

    #[test]
    fn test_generate_arweave_secret() {
        let seed = hex::decode("96063C45132C840F7E1665A3B97814D8EB2586F34BD945F06FA15B9327EEBE355F654E81C6233A52149D7A95EA7486EB8D699166F5677E507529482599624CDC").unwrap();
        let p = hex::decode("EA8E3612876ED1433E5909D25F699F7C5D4984CF0D2F268B185141F0E29CE65237EAD8236C94A0A9547F1FEABD4F54399C626C0FB813249BC74A3082F8637A9E9A3C9D4F6E1858ED29770FE95418ED66F07A9F2F378D43D31ED37A0E6942727394A87B93540E421742ADE9630E26500FD2C01502DF8E3F869C70DAA97D4583048DD367E2977851052F6A991182318015557EC81B58E81B668E3A715212C807A1D7835FCB2B87B5DEFAC0948B220D340D6B2DA0DCFC7123DE1F1424F6F5B8EAFA719B3DE8B9B6FEC196E2E393CE30204267A586625541C7B1433F8FA7873B51B3E65462831BF34A4E5912297A06B2E91B31657DFA3CCFDB5F94D438D9904CFD27").unwrap();
        let q = hex::decode("E360BFD757FF6FCF844AF226DCA7CFBD353A89112079C9C5A17C4F354DE0B1BE38BBFD73EAA77C4E2FFC681A79CEC8C8E79A5A00E32113A77748F435717BE6AD04AEF473BCE05DC3B742AAB853C02C565847133AFFD451B472B13031300978606F74BE8761A69733BEF8C2CCD6F396A0CCE23CDC73A8FF4609F47C18FE4626B788C4BFB73B9CF10BC5D6F80E9B9847530973CF5212D8EB142EAA155D774417D7BF89E1F229472926EA539AC3BAF42CF63EF18A6D85915727E9A77B4EA31B577B1E4A35C40CCCE72F5ECE426709E976DAEDBE7B76291F89EB85903182035CA98EB156563E392D0D1E427C59657B9EDF1DDB049BBB9620B881D8715982AD257D29").unwrap();
        let secret = generate_secret(&seed).unwrap();
        let mut secret_bytes: [u8; 512] = [0; 512];
        secret_bytes[..256].copy_from_slice(&secret.primes()[0].to_bytes_be());
        secret_bytes[256..].copy_from_slice(&secret.primes()[1].to_bytes_be());

        assert_eq!(secret_bytes[..256], p[..]);
        assert_eq!(secret_bytes[256..], q[..]);
    }

    #[test]
    fn test_aes256() {
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let data = hex::decode("fdec3a1aee520780ca4058402d0422b5cd5950b715728f532499dd4bbcb68e5d44650818b43656782237316c4b0e2faa2b15c245fb82d10cf4f5b420f1f293ba75b2c8d8cef6ad899c34ce9de482cb248cc5ab802fd93094a63577590d812d5dd781846ef7d4f5d9018199c293966371c2349b0f847c818ec99caad800116e02085d35a39a913bc735327705161761ae30a4ec775f127fbb5165418c0fe08e54ae0aff8b2dab2b82d3b4b9c807de5fae116096075cf6d5b77450d743d743e7dcc56e7cafdcc555f228e57b363488e171d099876993e93e37a94983ccc12dba894c58ca84ac154c1343922c6a99008fabd0fa7010d3cc34f69884fec902984771c5b50031ba31ab7c8b76453ce771f048b84fb89a3e4d44c222c3d8c823c683988b0dbf354d8b8cbf65f3db53e1365d3c5e043f0155b41d1ebeca6e20b2d6778600b5c98ffdba33961dae73b018307ef2bce9d217bbdf32964080f8db6f0cf7ef27ac825fcaf98d5143690a5d7e138f4875280ed6de581e66ed17f83371c268a073e4594814bcc88a33cbb4ec8819cc722ea15490312b85fed06e39274c4f73ac91c7f4d1b899729691cce616fb1a5feee1972456addcb51ac830e947fcc1b823468f0eefbaf195ac3b34f0baf96afc6fa77ee2e176081d6d91ce8c93c3d0f3547e48d059c9da447ba05ee3984703bebfd6d704b7f327ffaea7d0f63d0d3c6d65").unwrap();
        let encoded_data_test = hex::decode("51037008DCE04A37F040529EC58A3B2733DDD4E6801A26CEF906938FE81613F4C49C067C89245DCBCC594E0E2070232840010C54783E5841B4276539035D7D305E88A8DDAC9CA0FFAF4CF79F0AEB5F6752DADF5C2A9D5EAC3BBAB2CFAD7AD4F97D3A7D8BE831136670ED9368F2E485026683A61745B93CC856E5A98F678E50230DE47837A12D062F38FEFC2F065666FDF4CA5F741C82D91872E547D535915AD78490D27AD45B6FE0CFCEFE212E405723E5BFA3FCA32460B9712A6D20EB38D8478A961F75869EAC22A0A7D25831405A8E8E4D8C959F8175B12EE1C7EA7AABE081EE976390DF9B4AE73E51126C8EE060EE9F8E57F9F0DD17026FA3048531CE736C51F7FDFF91DB651E9FCFA8274AEA4D59D0E407A61F53BA373245C83F8D850DC5450726D921E230A4AAB6A8F156A639FAE037E38442542763587227FEF8D76AD513778CD6AF36A78345CDF815D92F2DDEBE897D4CEEA6E583877DA43B244A56B2EBF6E0415ED591D6C73A923CD97A8A512600C519B89C4E0EFC36C2C7307F34CAAF2739DCF86F23F8107472F3F7B483DB4CD87F66E7A37D8BADC8AC89CB4BED1A3130E6DC2F4C474AFD98A4C1F62F51AF36F857A01D96D728A6ADF94B0EA3C82A5749C3213025CA5F27CA02F59CA8E8E44548470A5D983298F54AB53A68037A2E0CF0F316BA6541CEE7C30D38AA5F96C90C4CB7EEAF312C9BDFE39BFE465089874FEDB51480368D43D897C8BF6B366F92").unwrap();
        assert_eq!(encoded_data_test.len(), 528);
        let (key, iv) = generate_aes_key_iv(&seed);
        let encrypted_data = aes256_encrypt(&key, &iv, &data).unwrap();
        let decrypted_data = aes256_decrypt(&key, &iv, &encrypted_data).unwrap();
        assert_eq!(data, decrypted_data);
        assert_eq!(encrypted_data.len(), 528);
        assert_eq!(decrypted_data.len(), 512);

        match aes256_decrypt(&key, &iv, &encoded_data_test) {
            Ok(decrypted_data2) => {
                assert_eq!(decrypted_data2.len(), 512);
                assert_eq!(decrypted_data, decrypted_data2);
            },
            Err(_) => {}
        }
    }
}
