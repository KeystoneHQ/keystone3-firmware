use alloc::string::{String, ToString};

use crate::errors::{KeystoreError, Result};

pub fn normalize_path(path: &str) -> String {
    let mut p = path.to_lowercase();
    if !p.starts_with('m') {
        p = format!("{}{}", "m/", p);
    }
    p
}

pub fn ensure_valid_seed(seed: &[u8]) -> Result<()> {
    if !matches!(seed.len(), 16 | 32 | 64) {
        return Err(KeystoreError::SeedError(format!(
            "invalid seed length: {}, expected 16, 32, or 64 bytes",
            seed.len()
        )));
    }
    if is_all_zero_or_ff(seed) {
        return Err(KeystoreError::SeedError("invalid seed".to_string()));
    }
    Ok(())
}

pub fn is_all_zero_or_ff(bytes: &[u8]) -> bool {
    if bytes.is_empty() {
        return false;
    }
    bytes.iter().all(|b| *b == 0x00) || bytes.iter().all(|b| *b == 0xFF)
}
