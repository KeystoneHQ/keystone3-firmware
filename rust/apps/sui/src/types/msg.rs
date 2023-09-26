use alloc::string::String;
use serde_derive::{Serialize, Deserialize};

#[derive(Serialize, Deserialize, Debug)]
pub struct PersonalMessageUtf8 {
  pub message: String,
}
