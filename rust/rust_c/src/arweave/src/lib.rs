#![no_std]

extern crate alloc;

use core::slice;
use alloc::boxed::Box;
use app_arweave::generate_secret;
use common_rust_c::structs::SimpleResponse;
use common_rust_c::types::PtrBytes;

#[no_mangle]
pub extern "C" fn generate_arweave_secret(seed: PtrBytes, seed_len: u32) -> *mut SimpleResponse<u8> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let secret = generate_secret(seed).unwrap();
    let mut secret_bytes: [u8; 512] = [0; 512];
    secret_bytes[..256].copy_from_slice(&secret.primes()[0].to_bytes_be());
    secret_bytes[256..].copy_from_slice(&secret.primes()[1].to_bytes_be());

    SimpleResponse::success(Box::into_raw(Box::new(secret_bytes)) as *mut u8).simple_c_ptr()
}

#[cfg(test)]
mod tests {
    use super::*;
    extern crate std;
    use third_party::hex;

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

        let mut secret_bytes_clone = secret_bytes.clone();
        let ret = SimpleResponse::success(Box::into_raw(Box::new(secret_bytes)) as *mut u8).simple_c_ptr();
        
        let ret_bytes = unsafe { Box::from_raw(ret) };
        
        let data_len = secret_bytes_clone.len();
        unsafe {
            let ret_slice = std::slice::from_raw_parts(ret_bytes.data, data_len);
            let secret_slice = std::slice::from_raw_parts(secret_bytes_clone.as_mut_ptr(), data_len);
            assert_eq!(ret_slice, secret_slice);
            assert_eq!(ret_slice.len(), 512);
        }
    }
}