use alloc::string::String;

#[derive(Debug, Clone)]
pub struct XrpTxOverviewPayment {
    pub transaction_type: String,
    pub from: String,
    pub to: String,
    pub fee: String,
    pub value: String,
}

#[derive(Debug, Clone)]
pub struct XrpTxOverviewGeneral {
    pub transaction_type: String,
    pub from: String,
    pub fee: String,
    pub sequence: u64,
}

#[derive(Debug, Clone)]
pub enum XrpTxOverview {
    Payment(XrpTxOverviewPayment),
    General(XrpTxOverviewGeneral),
}
