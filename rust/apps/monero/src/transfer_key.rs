use crate::extra::*;
use crate::key::*;
use crate::transfer::{TxConstructionData, TxDestinationEntry};
use crate::utils::hash::*;
use crate::utils::*;
use alloc::vec;
use alloc::vec::Vec;
use curve25519_dalek::edwards::EdwardsPoint;
use curve25519_dalek::scalar::Scalar;
use monero_serai::primitives::Commitment;
use monero_serai::ringct::EncryptedAmount;
use monero_wallet::SharedKeyDerivations;
use rand_core::SeedableRng;
use zeroize::Zeroizing;

impl TxConstructionData {
    fn should_use_additional_keys(&self) -> bool {
        self.sources
            .iter()
            .any(|source| source.real_out_additional_tx_keys.len() > 0)
    }

    fn has_payments_to_subaddresses(&self) -> bool {
        self.splitted_dsts.iter().any(|dst| dst.is_subaddress)
    }

    pub fn transaction_keys(
        &self,
    ) -> (PrivateKey, Vec<PrivateKey>, EdwardsPoint, Vec<EdwardsPoint>) {
        let seed = keccak256(&self.extra);
        let mut rng = rand_chacha::ChaCha20Rng::from_seed(seed);
        let tx_key = generate_random_scalar(&mut rng);
        let mut additional_keys = vec![];
        if self.should_use_additional_keys() {
            for _ in 0..self.splitted_dsts.len() {
                additional_keys.push(PrivateKey::from_bytes(
                    generate_random_scalar(&mut rng).as_bytes(),
                ));
            }
        }
        let tx_key = PrivateKey::from_bytes(tx_key.as_bytes());

        let mut tx_key_pub = EdwardsPoint::mul_base(&tx_key.scalar);
        let mut additional_keys_pub = vec![];
        let has_payments_to_subaddresses = self.has_payments_to_subaddresses();
        let should_use_additional_keys = self.should_use_additional_keys();
        if has_payments_to_subaddresses && !should_use_additional_keys {
            let spend = self
                .splitted_dsts
                .iter()
                .find(|dest| dest.is_subaddress)
                .unwrap()
                .addr
                .spend_public_key;

            tx_key_pub = tx_key.scalar
                * PublicKey::from_bytes(&spend)
                    .unwrap()
                    .point
                    .decompress()
                    .unwrap();
        } else if should_use_additional_keys {
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
        }

        (tx_key, additional_keys, tx_key_pub, additional_keys_pub)
    }

    pub fn is_change_dest(&self, dest: &TxDestinationEntry) -> bool {
        dest == &self.change_dts
    }

    fn ecdhs(&self, keypair: &KeyPair) -> Vec<EdwardsPoint> {
        let (tx_key, additional_keys, tx_key_pub, _) = self.transaction_keys();
        let mut res = Vec::with_capacity(self.splitted_dsts.len());
        for (i, dest) in self.splitted_dsts.iter().enumerate() {
            let key_to_use = if dest.is_subaddress {
                additional_keys.get(i).unwrap_or(&tx_key)
            } else {
                &tx_key
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

    fn payment_id_xors(&self, keypair: &KeyPair) -> Vec<[u8; 8]> {
        let mut res = Vec::with_capacity(self.splitted_dsts.len());
        for ecdh in self.ecdhs(keypair) {
            res.push(SharedKeyDerivations::payment_id_xor(Zeroizing::new(ecdh)));
        }
        res
    }

    pub fn extra(&self, keypair: &KeyPair) -> Vec<u8> {
        let (_, _, tx_key, additional_keys) = self.transaction_keys();
        let payment_id_xors = self.payment_id_xors(keypair);
        let mut extra = Extra::new(tx_key, additional_keys);
        if self.splitted_dsts.len() == 2 {
            let (_, payment_id_xor) = self
                .splitted_dsts
                .iter()
                .zip(&payment_id_xors)
                .find(|(dest, _)| self.is_change_dest(dest))
                .expect("multiple change outputs?");
            let mut id_vec = Vec::with_capacity(1 + 8);
            id_vec.extend_from_slice(&[1]);
            id_vec.extend_from_slice(payment_id_xor);
            extra.push_nonce(id_vec);
        }

        extra.serialize()
    }

    pub fn shared_key_derivations(
        &self,
        keypair: &KeyPair,
    ) -> Vec<Zeroizing<SharedKeyDerivations>> {
        let ecdhs = self.ecdhs(keypair);
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
    ) -> Vec<(Commitment, EncryptedAmount)> {
        let shared_key_derivations = self.shared_key_derivations(keypair);

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

    pub fn sum_output_masks(&self, keypair: &KeyPair) -> Scalar {
        self.commitments_and_encrypted_amounts(keypair)
            .into_iter()
            .map(|(commitment, _)| commitment.mask)
            .sum()
    }
}
