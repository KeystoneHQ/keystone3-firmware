use alloc::string::String;
use alloc::string::ToString;
use bitcoin::base58;
use cryptoxide::digest::Digest;
use cryptoxide::sha2::Sha256;

pub trait Cb58Encodable {
    fn get_prefix(&self) -> &'static str;
    fn get_data(&self) -> &[u8];

    fn to_cb58(&self) -> String {
        let data = self.get_data();
        let mut hasher = Sha256::new();
        let mut checksum = [0u8; 32];
        hasher.input(data);
        hasher.result(&mut checksum);

        let mut with_checksum = data.to_vec();
        with_checksum.extend_from_slice(&checksum[checksum.len() - 4..]);

        format!(
            "{}-{}",
            self.get_prefix(),
            base58::encode(&with_checksum).to_string()
        )
    }
}
