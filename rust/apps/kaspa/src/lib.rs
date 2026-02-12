#![no_std]

#[allow(unused_imports)]
#[macro_use]
extern crate alloc;
extern crate core;
#[cfg(test)]
#[macro_use]
extern crate std;

pub mod addresses;
pub mod errors;
pub mod pskt;

pub use addresses::{get_address, get_kaspa_derivation_path};
pub use pskt::{Pskt, PsktSigner};

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_kaspa_derivation_path() {
        let path = get_kaspa_derivation_path(0, 0, 0);
        assert_eq!(path, "m/44'/111111'/0'/0/0");
    }
}
