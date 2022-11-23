use alloc::string::String;

#[derive(Clone)]
pub struct SyncInfo {
    pub name: String,
    pub hd_path: String,
    pub xpub: String,
}
