#[derive(PartialEq, Copy, Clone, Debug)]
pub enum Network {
    Mainnet,
    Testnet,
    Stagenet,
}

#[derive(PartialEq, Copy, Clone, Debug)]

pub enum AddressType {
    Standard,
    Subaddress,
}
