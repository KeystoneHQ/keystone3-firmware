#![no_std]
#![feature(error_in_core)]
extern crate alloc;

pub mod keystone;
pub mod macros;
pub use paste;

use alloc::format;
use alloc::string::String;
use third_party::unicode_blocks;

pub fn normalize_path(path: &String) -> String {
    let mut p = path.to_lowercase();
    if !p.starts_with("m/") {
        p = format!("{}{}", "m/", p);
    }
    if !p.starts_with("m") {
        p = format!("{}{}", "m", p);
    }
    p
}

pub fn is_cjk(utf8_string: &str) -> bool {
    for c in utf8_string.chars() {
        if unicode_blocks::is_cjk(c) {
            return true;
        }
    }
    return false;
}
