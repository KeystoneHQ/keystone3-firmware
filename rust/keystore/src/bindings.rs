#[allow(dead_code)]
extern "C" {
    pub fn GetAccountSeed(account_index: u8, seed: *mut u8, password: *const cty::c_char) -> i32;
    pub fn GetAccountEntropy(
        account_index: u8,
        entropy: *mut u8,
        entropy_len: *mut u8,
        password: *const cty::c_char,
    ) -> i32;
}
