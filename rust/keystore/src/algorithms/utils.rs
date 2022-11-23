use alloc::string::String;

pub fn normalize_path(path: &String) -> String {
    let mut p = path.to_lowercase();
    if !p.starts_with('m') {
        p = format!("{}{}", "m/", p);
    }
    p
}
