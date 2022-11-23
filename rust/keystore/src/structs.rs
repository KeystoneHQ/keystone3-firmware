extern crate alloc;

use alloc::string::String;

#[derive(Clone)]
pub struct AccountCert {
    pub index: u8,
    pub password: String,
}
