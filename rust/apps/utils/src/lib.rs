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

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;

    #[test]
    fn test_normalize_path() {
        let path = "m/48'/0'/0'/2'".to_string();
        let result = normalize_path(&path);
        assert_eq!(result, path);

        let path = "48'/0'/0'/2'".to_string();
        let result = normalize_path(&path);
        assert_eq!(result, "m/48'/0'/0'/2'");
    }

    #[test]
    fn test_is_cjk() {
        let utf8_string = "你好";
        let result = is_cjk(utf8_string);
        assert_eq!(result, true);

        let utf8_string = "hello";
        let result = is_cjk(utf8_string);
        assert_eq!(result, false);

        let utf8_string = "こんにちは";
        let result = is_cjk(utf8_string);
        assert_eq!(result, true);

        let utf8_string = "안녕하세요";
        let result = is_cjk(utf8_string);
        assert_eq!(result, true);
    }
}
