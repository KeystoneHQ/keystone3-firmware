use crate::errors::ICPError;
use crate::principal::Principal;
use alloc::str::FromStr;
use alloc::vec::Vec;
use serde::Deserialize;
use sha2::Digest;
use sha2::Sha224;

#[derive(Clone, Copy, Deserialize)]
#[repr(transparent)]
#[serde(transparent)]
pub struct PrincipalId(pub Principal);

impl PrincipalId {
    pub const MAX_LENGTH_IN_BYTES: usize = 29;

    pub fn as_slice(&self) -> &[u8] {
        self.0.as_slice()
    }

    pub fn to_vec(&self) -> Vec<u8> {
        self.as_slice().to_vec()
    }

    pub fn into_vec(self) -> Vec<u8> {
        self.to_vec()
    }
}

impl From<PrincipalId> for Vec<u8> {
    fn from(val: PrincipalId) -> Self {
        val.to_vec()
    }
}

impl AsRef<[u8]> for PrincipalId {
    fn as_ref(&self) -> &[u8] {
        self.as_slice()
    }
}

impl FromStr for PrincipalId {
    type Err = ICPError;

    fn from_str(input: &str) -> Result<Self, Self::Err> {
        Principal::from_str(input)
            .map(Self)
            .map_err(|e| ICPError::KeystoreError(format!("Could not parse principal id: `{}`", e)))
    }
}

fn hash(data: &[u8]) -> [u8; 28] {
    let mut sha224 = Sha224::default();
    sha224.update(data);
    sha224.finalize().into()
}

impl PrincipalId {
    const TYPE_SELF_AUTH: u8 = 0x02;

    pub fn new_self_authenticating(pubkey: &[u8]) -> Self {
        let mut id: [u8; 29] = [0; 29];
        id[..28].copy_from_slice(&hash(pubkey));
        id[28] = Self::TYPE_SELF_AUTH;
        // id has fixed length of 29, safe to unwrap here
        PrincipalId(Principal::try_from_slice(&id).unwrap())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    extern crate std;
    use std::string::ToString;

    #[test]
    fn parse_self_authenticating_id_ok() {
        let key = [
            0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
            0x11, 0x00, 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44,
            0x33, 0x22, 0x11, 0x00,
        ];
        let id = PrincipalId::new_self_authenticating(&key);
        assert_eq!(
            "bngem-gzprz-dtr6o-xnali-fgmfi-fjgpb-rya7j-x2idk-3eh6u-4v7tx-hqe".to_string(),
            id.0.to_text()
        );
    }

    #[test]
    fn parse_from_principal_id_str() {
        let principal_id_str = "7rtqo-ah3ki-saurz-utzxq-o4yhl-so2yx-iardd-mktej-x4k24-ijen6-dae";
        let principal_id = PrincipalId::from_str(&principal_id_str).unwrap();
        assert_eq!(
            "7rtqo-ah3ki-saurz-utzxq-o4yhl-so2yx-iardd-mktej-x4k24-ijen6-dae".to_string(),
            principal_id.0.to_text()
        );
    }
}
