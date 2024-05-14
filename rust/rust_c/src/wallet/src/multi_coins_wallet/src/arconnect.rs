use alloc::slice;
use alloc::vec::Vec;
use app_arweave::generate_public_key_from_primes;
use common_rust_c::types::{Ptr, PtrBytes, PtrString};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::recover_c_char;
use cty::uint32_t;
use third_party::hex;
use third_party::ur_registry::arweave::arweave_crypto_account::ArweaveCryptoAccount;
use third_party::ur_registry::traits::{RegistryItem, To};

fn vec_to_array(vec: Vec<u8>) -> Result<[u8; 4], &'static str> {
    if vec.len() == 4 {
        let mut arr = [0u8; 4];
        for (i, byte) in vec.into_iter().enumerate() {
            arr[i] = byte;
        }
        Ok(arr)
    } else {
        Err("Vec<u8> must contain exactly 4 elements")
    }
}

#[no_mangle]
pub extern "C" fn get_connect_arconnect_wallet_ur(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: uint32_t,
    p: PtrBytes,
    p_len: uint32_t,
    q: PtrBytes,
    q_len: uint32_t,
) -> Ptr<UREncodeResult> {
    unsafe {
        let mfp = unsafe {
            slice::from_raw_parts(master_fingerprint, master_fingerprint_length as usize)
        };
        let p = unsafe { slice::from_raw_parts(p, p_len as usize) };
        let q = unsafe { slice::from_raw_parts(q, q_len as usize) };
        let public_key = generate_public_key_from_primes(&p, &q).unwrap();
        let device = Option::None;
        let arweave_account = ArweaveCryptoAccount::new(
            vec_to_array(mfp.to_vec()).unwrap(),
            public_key.to_vec(),
            device,
        );
        return UREncodeResult::encode(
            arweave_account.to_bytes().unwrap(),
            ArweaveCryptoAccount::get_registry_type().get_type(),
            FRAGMENT_MAX_LENGTH_DEFAULT,
        )
        .c_ptr();
    }
}

#[no_mangle]
pub extern "C" fn get_connect_arconnect_wallet_ur_from_xpub(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: uint32_t,
    xpub: PtrString,
) -> Ptr<UREncodeResult> {
    unsafe {
        let mfp = unsafe {
            slice::from_raw_parts(master_fingerprint, master_fingerprint_length as usize)
        };
        let xpub = recover_c_char(xpub);
        let device = Option::None;
        let arweave_account = ArweaveCryptoAccount::new(
            vec_to_array(mfp.to_vec()).unwrap(),
            hex::decode(xpub).unwrap(),
            device,
        );
        return UREncodeResult::encode(
            arweave_account.to_bytes().unwrap(),
            ArweaveCryptoAccount::get_registry_type().get_type(),
            FRAGMENT_MAX_LENGTH_DEFAULT,
        )
        .c_ptr();
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    extern crate std;

    #[test]
    fn test_get_connect_arconnect_wallet_ur() {
        let mut master_fingerprint = [52, 74, 47, 03];
        let master_fingerprint_length = 4;
        let mut p = hex::decode("fdec3a1aee520780ca4058402d0422b5cd5950b715728f532499dd4bbcb68e5d44650818b43656782237316c4b0e2faa2b15c245fb82d10cf4f5b420f1f293ba75b2c8d8cef6ad899c34ce9de482cb248cc5ab802fd93094a63577590d812d5dd781846ef7d4f5d9018199c293966371c2349b0f847c818ec99caad800116e02085d35a39a913bc735327705161761ae30a4ec775f127fbb5165418c0fe08e54ae0aff8b2dab2b82d3b4b9c807de5fae116096075cf6d5b77450d743d743e7dcc56e7cafdcc555f228e57b363488e171d099876993e93e37a94983ccc12dba894c58ca84ac154c1343922c6a99008fabd0fa7010d3cc34f69884fec902984771").unwrap();
        let mut q = hex::decode("c5b50031ba31ab7c8b76453ce771f048b84fb89a3e4d44c222c3d8c823c683988b0dbf354d8b8cbf65f3db53e1365d3c5e043f0155b41d1ebeca6e20b2d6778600b5c98ffdba33961dae73b018307ef2bce9d217bbdf32964080f8db6f0cf7ef27ac825fcaf98d5143690a5d7e138f4875280ed6de581e66ed17f83371c268a073e4594814bcc88a33cbb4ec8819cc722ea15490312b85fed06e39274c4f73ac91c7f4d1b899729691cce616fb1a5feee1972456addcb51ac830e947fcc1b823468f0eefbaf195ac3b34f0baf96afc6fa77ee2e176081d6d91ce8c93c3d0f3547e48d059c9da447ba05ee3984703bebfd6d704b7f327ffaea7d0f63d0d3c6d65").unwrap();
        let result = get_connect_arconnect_wallet_ur(
            master_fingerprint.as_mut_ptr(),
            master_fingerprint_length,
            p.as_mut_ptr(),
            p.len() as uint32_t,
            q.as_mut_ptr(),
            q.len() as uint32_t,
        );
        unsafe {
            let data = (*result).data;
            let data = std::ffi::CStr::from_ptr(data).to_str().unwrap();
            assert_eq!(data, "UR:ARWEAVE-CRYPTO-ACCOUNT/1-3/LPADAXCFAOBDCYBGMNJLWEHDPEOEADCYEEGEDLAXAOHKAOAESSCYGDWECLGOONJYBDFEURMNETBZKTGTJELGCFFMHTTPBNNNZSPEJNJNAOGUWFGDSPHPWPWFNNRLAHJNKPLRCTIMAMGETKLSLYETFMTOPRCSVYISHKWSJPRNJPJKEYCYDNFDGOROKGSWWNGSJKGLDRNSMHLPBNEEPDNBOXDINYSORNEHLNPFLNUYHPDYDLRPLYKOQZSEZEVLEMFEJZFWYTJPSTNLFHHSMYUEUOBDWNIHLKDPHKTKDWBNIMSRCYHSPSBGHNVTZCGEKOCEOTJOKBDIHSCEBBQZSWRPPYVAMKSEBEASUTYKTTGYCYVEKBOEJSATNDISGOWZVDDW");
        }
    }
}
