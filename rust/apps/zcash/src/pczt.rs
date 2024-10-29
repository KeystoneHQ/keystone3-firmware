use keystore::algorithms::zcash::vendor::{orchard::keys::FullViewingKey, pczt::Pczt};
use third_party::bitcoin::bip32::ExtendendPubKey;

trait PcztTrait {
    fn sign(&self) -> Result<Pczt>;
    fn verify(&self) -> Result<bool>;
    fn hash(&self) -> Result<Vec<u8>>;
}

fn verify_transparent_part(pczt: &Pczt, xpub: &ExtendendPubKey) -> Result<bool> {
    
    Ok(true)
}

fn verify_orchard_part(pczt: &Pczt, fvk: &FullViewingKey) -> Result<bool> {
    Ok(true)
}

impl PcztTrait for Pczt {
    fn sign(&self) -> Result<Pczt> {
        Ok(self.clone())
    }

    fn verify(&self) -> Result<bool> {
        Ok(true)
    }

    fn hash(&self) -> Result<Vec<u8>> {
        Ok(vec![])
    }
}
