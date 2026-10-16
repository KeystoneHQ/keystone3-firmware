use crate::errors::{MoneroError, Result};
use crate::extra::*;
use crate::key::*;
use crate::transfer::{AccountPublicAddress, TxConstructionData, TxDestinationEntry};
use crate::utils::*;
use alloc::vec;
use alloc::vec::Vec;
use curve25519_dalek::edwards::EdwardsPoint;
use curve25519_dalek::scalar::Scalar;
use monero_serai::primitives::Commitment;
use monero_serai::ringct::EncryptedAmount;
use monero_wallet::SharedKeyDerivations;
use rand_core::OsRng;
use zeroize::Zeroizing;

#[derive(Debug, Default, Clone)]
pub(crate) struct RequestedExtra {
    pub nonce: Option<Vec<u8>>,
    pub padding: usize,
}

#[derive(Debug, Default, Clone, Copy)]
struct DestinationClassification {
    num_standard: usize,
    num_subaddress: usize,
    single_subaddress_spend: Option<[u8; 32]>,
}

impl TxConstructionData {
    fn classify_destinations(&self) -> DestinationClassification {
        let change_addr = self.change_dts.addr;
        let mut unique: Vec<AccountPublicAddress> = vec![];
        let mut res = DestinationClassification::default();

        for dst in self.splitted_dsts.iter() {
            if dst.addr == change_addr || unique.contains(&dst.addr) {
                continue;
            }
            unique.push(dst.addr);

            if dst.is_subaddress {
                res.num_subaddress += 1;
                res.single_subaddress_spend = Some(dst.addr.spend_public_key);
            } else {
                res.num_standard += 1;
            }
        }

        res
    }

    fn should_use_additional_keys(&self) -> bool {
        let classification = self.classify_destinations();
        classification.num_subaddress > 0
            && (classification.num_standard > 0 || classification.num_subaddress > 1)
    }

    pub fn transaction_keys(
        &self,
    ) -> (PrivateKey, Vec<PrivateKey>, EdwardsPoint, Vec<EdwardsPoint>) {
        let classification = self.classify_destinations();
        let should_use_additional_keys = self.should_use_additional_keys();

        let tx_key = PrivateKey::from_bytes(generate_random_scalar(&mut OsRng).as_bytes());
        let mut additional_keys = vec![];
        if should_use_additional_keys {
            for _ in 0..self.splitted_dsts.len() {
                additional_keys.push(PrivateKey::from_bytes(
                    generate_random_scalar(&mut OsRng).as_bytes(),
                ));
            }
        }

        let mut tx_key_pub = EdwardsPoint::mul_base(&tx_key.scalar);
        let mut additional_keys_pub = vec![];

        if should_use_additional_keys {
            for (additional_key, dest) in
                additional_keys.clone().into_iter().zip(&self.splitted_dsts)
            {
                let spend = PublicKey::from_bytes(&dest.addr.spend_public_key).unwrap();
                if dest.is_subaddress {
                    additional_keys_pub
                        .push(additional_key.scalar * spend.point.decompress().unwrap());
                } else {
                    additional_keys_pub.push(EdwardsPoint::mul_base(&additional_key.scalar))
                }
            }
        } else if classification.num_standard == 0 && classification.num_subaddress == 1 {
            if let Some(spend) = classification.single_subaddress_spend {
                tx_key_pub = tx_key.scalar
                    * PublicKey::from_bytes(&spend)
                        .unwrap()
                        .point
                        .decompress()
                        .unwrap();
            }
        }

        (tx_key, additional_keys, tx_key_pub, additional_keys_pub)
    }

    pub fn is_change_dest(&self, dest: &TxDestinationEntry) -> bool {
        dest.addr == self.change_dts.addr
    }

    pub fn has_change_output(&self) -> bool {
        let change_addr = self.change_dts.addr;
        self.splitted_dsts
            .iter()
            .any(|dst| dst.addr == change_addr && dst.amount > 0)
    }

    fn ecdhs(
        &self,
        keypair: &KeyPair,
        tx_key: &PrivateKey,
        additional_keys: &Vec<PrivateKey>,
        tx_key_pub: &EdwardsPoint,
    ) -> Vec<EdwardsPoint> {
        let mut res = Vec::with_capacity(self.splitted_dsts.len());
        for (i, dest) in self.splitted_dsts.iter().enumerate() {
            let key_to_use = if dest.is_subaddress {
                additional_keys.get(i).unwrap_or(tx_key)
            } else {
                tx_key
            };
            res.push(if !self.is_change_dest(dest) {
                key_to_use.scalar
                    * PublicKey::from_bytes(&dest.addr.view_public_key)
                        .unwrap()
                        .point
                        .decompress()
                        .unwrap()
            } else {
                keypair.view.scalar * tx_key_pub
            });
        }

        res
    }

    fn payment_id_xors(
        &self,
        keypair: &KeyPair,
        tx_key: &PrivateKey,
        additional_keys: &Vec<PrivateKey>,
        tx_key_pub: &EdwardsPoint,
    ) -> Vec<[u8; 8]> {
        let mut res = Vec::with_capacity(self.splitted_dsts.len());
        for ecdh in self.ecdhs(keypair, tx_key, additional_keys, tx_key_pub) {
            res.push(SharedKeyDerivations::payment_id_xor(Zeroizing::new(ecdh)));
        }
        res
    }

    pub(crate) fn requested_extra(&self) -> Result<RequestedExtra> {
        let mut res = RequestedExtra::default();
        use crate::utils::io::Reader;
        let mut reader = Reader::new(&self.extra);
        while reader.remaining() > 0 {
            match reader.read_u8()? {
                0x00 => res.padding += 1,
                0x01 => {
                    reader.read_bytes(32)?;
                }
                0x02 => {
                    res.nonce = Some(reader.read_vec_u8(255)?);
                }
                0x03 => {
                    let length = reader.read_count(self.extra.len())?;
                    let mut field = Reader::new(reader.read_bytes(length)?);
                    field.read_varint()?;
                    field.read_bytes(32)?;
                    if field.remaining() != 0 {
                        return Err(MoneroError::FormatError);
                    }
                }
                0x04 => {
                    let count = reader.read_count(self.extra.len() / 32)?;
                    reader.read_bytes(count.checked_mul(32).ok_or(MoneroError::FormatError)?)?;
                }
                _ => return Err(MoneroError::FormatError),
            }
        }

        Ok(res)
    }

    pub(crate) fn payment_id_destination_index(&self) -> Option<usize> {
        let change_addr = self.change_dts.addr;
        let mut unique: Vec<AccountPublicAddress> = vec![];
        let mut chosen = None;
        for (i, dst) in self.splitted_dsts.iter().enumerate() {
            if dst.amount == 0 || dst.addr == change_addr || unique.contains(&dst.addr) {
                continue;
            }
            unique.push(dst.addr);
            if unique.len() > 1 {
                return None;
            }
            chosen = Some(i);
        }
        chosen.or_else(|| {
            self.splitted_dsts
                .iter()
                .position(|dst| dst.addr == change_addr)
        })
    }

    fn destination_payment_id_xor(&self, payment_id_xors: &[[u8; 8]]) -> Option<[u8; 8]> {
        self.payment_id_destination_index()
            .and_then(|index| payment_id_xors.get(index).copied())
    }

    pub fn extra(
        &self,
        keypair: &KeyPair,
        tx_key: &PrivateKey,
        additional_keys: &Vec<PrivateKey>,
        tx_key_pub: &EdwardsPoint,
        additional_keys_pub: &Vec<EdwardsPoint>,
    ) -> Result<Vec<u8>> {
        let payment_id_xors = self.payment_id_xors(keypair, tx_key, additional_keys, tx_key_pub);
        let mut extra = Extra::new(*tx_key_pub, additional_keys_pub.clone());

        let requested = self.requested_extra().unwrap_or_default();
        let requested_nonce = requested.nonce;
        match requested_nonce {
            Some(nonce) if nonce.first() == Some(&1) && nonce.len() == 9 => {
                let xor = self
                    .destination_payment_id_xor(&payment_id_xors)
                    .ok_or(MoneroError::FormatError)?;
                let mut id_vec = Vec::with_capacity(1 + 8);
                id_vec.push(1);
                id_vec.extend(nonce[1..9].iter().zip(xor.iter()).map(|(a, b)| a ^ b));
                extra.push_nonce(id_vec);
            }
            Some(nonce) => extra.push_nonce(nonce),
            None => {
                if self.splitted_dsts.len() <= 2 {
                    if let Some(payment_id_xor) = self.destination_payment_id_xor(&payment_id_xors)
                    {
                        let mut id_vec = Vec::with_capacity(1 + 8);
                        id_vec.push(1);
                        id_vec.extend_from_slice(&payment_id_xor);
                        extra.push_nonce(id_vec);
                    }
                }
            }
        }

        extra.push_padding(requested.padding);

        Ok(extra.serialize())
    }

    pub fn shared_key_derivations(
        &self,
        keypair: &KeyPair,
        tx_key: &PrivateKey,
        additional_keys: &Vec<PrivateKey>,
        tx_key_pub: &EdwardsPoint,
    ) -> Vec<Zeroizing<SharedKeyDerivations>> {
        let ecdhs = self.ecdhs(keypair, tx_key, additional_keys, tx_key_pub);
        let mut res = Vec::with_capacity(self.splitted_dsts.len());
        for (i, (_, ecdh)) in self.splitted_dsts.iter().zip(ecdhs).enumerate() {
            res.push(SharedKeyDerivations::output_derivations(
                None,
                Zeroizing::new(ecdh),
                i,
            ));
        }

        res
    }

    pub fn commitments_and_encrypted_amounts(
        &self,
        keypair: &KeyPair,
        tx_key: &PrivateKey,
        additional_keys: &Vec<PrivateKey>,
        tx_key_pub: &EdwardsPoint,
    ) -> Vec<(Commitment, EncryptedAmount)> {
        let shared_key_derivations =
            self.shared_key_derivations(keypair, tx_key, additional_keys, tx_key_pub);

        let mut res = Vec::with_capacity(self.splitted_dsts.len());
        for (dest, shared_key_derivation) in self.splitted_dsts.iter().zip(shared_key_derivations) {
            let amount = dest.amount;

            let commitment = Commitment::new(shared_key_derivation.commitment_mask(), amount);
            let encrypted_amount = EncryptedAmount::Compact {
                amount: shared_key_derivation.compact_amount_encryption(amount),
            };
            res.push((commitment, encrypted_amount));
        }

        res
    }

    pub fn sum_output_masks(
        &self,
        keypair: &KeyPair,
        tx_key: &PrivateKey,
        additional_keys: &Vec<PrivateKey>,
        tx_key_pub: &EdwardsPoint,
    ) -> Scalar {
        self.commitments_and_encrypted_amounts(keypair, tx_key, additional_keys, tx_key_pub)
            .into_iter()
            .map(|(commitment, _)| commitment.mask)
            .sum()
    }
}
