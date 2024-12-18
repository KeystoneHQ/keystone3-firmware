use alloc::vec::Vec;
use curve25519_dalek::edwards::EdwardsPoint;

#[derive(Clone, PartialEq, Eq, Debug)]
pub enum ExtraField {
    /// Padding.
    ///
    /// This is a block of zeroes within the TX extra.
    Padding(usize),
    /// The transaction key.
    ///
    /// This is a commitment to the randomness used for deriving outputs.
    PublicKey(EdwardsPoint),
    /// The nonce field.
    ///
    /// This is used for data, such as payment IDs.
    Nonce(Vec<u8>),
    /// The field for merge-mining.
    ///
    /// This is used within miner transactions who are merge-mining Monero to specify the foreign
    /// block they mined.
    MergeMining(usize, [u8; 32]),
    /// The additional transaction keys.
    ///
    /// These are the per-output commitments to the randomness used for deriving outputs.
    PublicKeys(Vec<EdwardsPoint>),
    /// The 'mysterious' Minergate tag.
    ///
    /// This was used by a closed source entity without documentation. Support for parsing it was
    /// added to reduce extra which couldn't be decoded.
    MysteriousMinergate(Vec<u8>),
}

#[derive(Clone, PartialEq, Eq, Debug)]
pub struct Extra(pub(crate) Vec<ExtraField>);

impl Extra {
    pub(crate) fn new(key: EdwardsPoint, additional: Vec<EdwardsPoint>) -> Extra {
        let mut res = Extra(Vec::with_capacity(3));
        // https://github.com/monero-project/monero/blob/cc73fe71162d564ffda8e549b79a350bca53c454
        //   /src/cryptonote_basic/cryptonote_format_utils.cpp#L627-L633
        // We only support pushing nonces which come after these in the sort order
        res.0.push(ExtraField::PublicKey(key));
        if !additional.is_empty() {
            res.0.push(ExtraField::PublicKeys(additional));
        }
        res
    }

    pub(crate) fn push_nonce(&mut self, nonce: Vec<u8>) {
        self.0.push(ExtraField::Nonce(nonce));
    }

    pub fn serialize(&self) -> Vec<u8> {
        let mut res = Vec::new();
        for field in &self.0 {
            match field {
                ExtraField::Padding(size) => {
                    res.push(0x00);
                    res.extend(core::iter::repeat(0).take(*size));
                }
                ExtraField::PublicKey(key) => {
                    res.push(0x01);
                    res.extend_from_slice(&key.compress().to_bytes());
                }
                ExtraField::Nonce(nonce) => {
                    res.push(0x02);
                    res.extend_from_slice(&[nonce.len() as u8]);
                    res.extend_from_slice(nonce);
                }
                ExtraField::MergeMining(size, data) => {
                    res.push(0x03);
                    res.extend_from_slice(&[(*size as u8)]);
                    res.extend_from_slice(data);
                }
                ExtraField::PublicKeys(keys) => {
                    res.push(0x04);
                    res.extend_from_slice(&[keys.len() as u8]);
                    for key in keys {
                        res.extend_from_slice(&key.compress().to_bytes());
                    }
                }
                ExtraField::MysteriousMinergate(data) => {
                    res.push(0xDE);
                    res.extend_from_slice(data);
                }
            }
        }
        res
    }
}
