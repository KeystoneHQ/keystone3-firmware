use alloc::string::String;
use serde_derive::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
pub struct PersonalMessageUtf8 {
    pub message: String,
}
