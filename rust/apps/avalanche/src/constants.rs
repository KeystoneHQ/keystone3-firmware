pub const BLOCKCHAIN_ID_LEN: usize = 32;
pub const SUBNET_ID_LEN: usize = 32;
pub const SUBNET_AUTH_LEN: usize = 4;
pub const NODE_ID_LEN: usize = 20;
pub const PROOF_OF_POSESSION_PUBKEY_LEN: usize = 48;
pub const PROOF_OF_POSESSION_SIGNATURE_LEN: usize = 100;
pub const C_CHAIN_ADDRESS_LEN: usize = 20;
pub const ASSET_ID_LEN: usize = 32;
pub const ADDRESS_LEN: usize = 20;
pub const NAVAX_TO_AVAX_RATIO: f64 = 1_000_000_000.0;

pub const X_BLOCKCHAIN_ID: [u8; BLOCKCHAIN_ID_LEN] = [
    237, 95, 56, 52, 30, 67, 110, 93, 70, 226, 187, 0, 180, 93, 98, 174, 151, 209, 176, 80, 198,
    75, 198, 52, 174, 16, 98, 103, 57, 227, 92, 75,
];
pub const P_BLOCKCHAIN_ID: [u8; BLOCKCHAIN_ID_LEN] = [
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
];

pub const C_BLOCKCHAIN_ID: [u8; BLOCKCHAIN_ID_LEN] = [
    4, 39, 212, 178, 42, 42, 120, 188, 221, 212, 86, 116, 44, 175, 145, 181, 107, 173, 191, 249,
    133, 238, 25, 174, 241, 69, 115, 231, 52, 63, 214, 82,
];
