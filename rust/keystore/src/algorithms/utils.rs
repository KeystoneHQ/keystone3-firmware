use alloc::string::String;

pub fn normalize_path(path: &str) -> String {
    let mut p = path.to_lowercase();
    if !p.starts_with('m') {
        p = format!("{}{}", "m/", p);
    }
    p
}

pub fn is_all_zero_or_ff(bytes: &[u8]) -> bool {
    if bytes.is_empty() {
        return false;
    }
    bytes.iter().all(|b| *b == 0x00) || bytes.iter().all(|b| *b == 0xFF)
}
