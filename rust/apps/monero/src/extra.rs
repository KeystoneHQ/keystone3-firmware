use crate::utils::io::write_varinteger;
use alloc::vec::Vec;
use curve25519_dalek::edwards::EdwardsPoint;

#[derive(Clone, PartialEq, Eq, Debug)]
#[allow(unused)]
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

    pub(crate) fn push_padding(&mut self, size: usize) {
        if size > 0 {
            self.0.push(ExtraField::Padding(size));
        }
    }

    pub fn serialize(&self) -> Vec<u8> {
        let mut res = Vec::new();
        for field in &self.0 {
            match field {
                ExtraField::Padding(size) => {
                    res.extend(core::iter::repeat_n(0, *size));
                }
                ExtraField::PublicKey(key) => {
                    res.push(0x01);
                    res.extend_from_slice(&key.compress().to_bytes());
                }
                ExtraField::Nonce(nonce) => {
                    res.push(0x02);
                    res.extend_from_slice(&write_varinteger(nonce.len() as u64));
                    res.extend_from_slice(nonce);
                }
                ExtraField::MergeMining(size, data) => {
                    res.push(0x03);
                    res.extend_from_slice(&[(*size as u8)]);
                    res.extend_from_slice(data);
                }
                ExtraField::PublicKeys(keys) => {
                    res.push(0x04);
                    res.extend_from_slice(&write_varinteger(keys.len() as u64));
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

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::vec;
    use curve25519_dalek::scalar::Scalar;

    #[test]
    fn serialize_all_extra_field_types() {
        let public_key = EdwardsPoint::mul_base(&Scalar::ONE);
        let additional_key = EdwardsPoint::mul_base(&Scalar::from(2u64));
        let merge_mining_data = [0xAB; 32];
        let extra = Extra(vec![
            ExtraField::Padding(2),
            ExtraField::PublicKey(public_key),
            ExtraField::Nonce(vec![0x10, 0x20]),
            ExtraField::MergeMining(32, merge_mining_data),
            ExtraField::PublicKeys(vec![additional_key]),
            ExtraField::MysteriousMinergate(vec![0xCA, 0xFE]),
        ]);

        let serialized = extra.serialize();
        let mut expected = vec![0x00, 0x00];
        expected.push(0x01);
        expected.extend_from_slice(&public_key.compress().to_bytes());
        expected.extend_from_slice(&[0x02, 0x02, 0x10, 0x20]);
        expected.extend_from_slice(&[0x03, 32]);
        expected.extend_from_slice(&merge_mining_data);
        expected.extend_from_slice(&[0x04, 0x01]);
        expected.extend_from_slice(&additional_key.compress().to_bytes());
        expected.extend_from_slice(&[0xDE, 0xCA, 0xFE]);

        assert_eq!(serialized, expected);
    }

    #[test]
    fn new_and_push_nonce_preserve_field_order() {
        let public_key = EdwardsPoint::mul_base(&Scalar::ONE);
        let additional_key = EdwardsPoint::mul_base(&Scalar::from(2u64));

        let mut without_additional = Extra::new(public_key, vec![]);
        without_additional.push_nonce(vec![7]);
        assert!(matches!(
            without_additional.0.as_slice(),
            [ExtraField::PublicKey(_), ExtraField::Nonce(nonce)] if nonce == &[7]
        ));

        let with_additional = Extra::new(public_key, vec![additional_key]);
        assert!(matches!(
            with_additional.0.as_slice(),
            [ExtraField::PublicKey(_), ExtraField::PublicKeys(keys)] if keys == &[additional_key]
        ));
    }
}
