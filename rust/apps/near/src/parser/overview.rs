use alloc::string::String;
use alloc::vec::Vec;

#[derive(Debug, Clone)]
pub struct NearTxOverviewTransfer {
    pub value: String,
    pub main_action: String,
    pub from: String,
    pub to: String,
}

#[derive(Debug, Clone)]
pub struct NearTxOverviewGeneral {
    pub from: String,
    pub to: String,
    pub action_list: Vec<String>,
}

#[derive(Debug, Clone)]
pub enum NearTxOverview {
    Transfer(NearTxOverviewTransfer),
    General(NearTxOverviewGeneral),
}
