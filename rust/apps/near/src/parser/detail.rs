use crate::parser::transaction::Action;
use alloc::string::String;
use serde::Serialize;

#[derive(Debug, Clone, Serialize)]
pub struct DetailHeader {
    pub from: String,
    pub to: String,
}

#[derive(Debug, Clone, Serialize)]
pub struct NearTxDetail {
    #[serde(flatten)]
    pub header: DetailHeader,
    #[serde(flatten)]
    pub kind: Action,
}
