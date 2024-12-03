
fn cb58_encode(data: &[u8]) -> String {
    let checksum = Sha256::digest(data);
    
    let mut with_checksum = data.to_vec();
    with_checksum.extend_from_slice(&checksum[checksum.len()-4..]);

    bs58::encode(with_checksum).into_string()
}