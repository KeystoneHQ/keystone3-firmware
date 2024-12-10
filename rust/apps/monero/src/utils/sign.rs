use alloc::vec::Vec;
use crate::key::*;
use crate::utils::*;

pub struct Signature(pub [u8; 64]);

impl From<Signature> for Vec<u8> {
    fn from(sig: Signature) -> Vec<u8> {
        sig.0.to_vec()
    }
}

pub fn generate_signature<R: RngCore + CryptoRng>(
  hash: &[u8],
  pubkey: &PublicKey,
  seckey: &PrivateKey,
  rng: &mut R,
) -> Option<Signature> {
  if seckey.get_public_key().point != pubkey.point {
      return None;
  }
  let mut c;
  let mut r;

  loop {
      let k = generate_random_scalar(rng);
      let temp3 = EdwardsPoint::mul_base(&k);

      let data = [hash, pubkey.point.as_bytes(), temp3.compress().as_bytes()].concat();
      c = hash_to_scalar(&data);
      if c == Scalar::ZERO {
          continue;
      }
      r = k - c * seckey.scalar;
      if r == Scalar::ZERO {
          continue;
      }
      break;
  }

  Some(Signature(
      [c.to_bytes(), r.to_bytes()].concat().try_into().unwrap(),
  ))
}

pub fn check_signature(hash: &[u8], pubkey: &PublicKey, sig: &Signature) -> bool {
  let sig = sig.0.to_vec();
  let c = &sig[..32];
  let r = &sig[32..];
  let point = pubkey.point.decompress().unwrap();

  let scalar_a = Scalar::from_canonical_bytes(c.try_into().unwrap());
  let scalar_b = Scalar::from_canonical_bytes(r.try_into().unwrap());
  let is_valid_a: bool = scalar_a.is_some().into();
  let is_valid_b: bool = scalar_b.is_some().into();
  if !is_valid_a || !is_valid_b || scalar_b.unwrap() == Scalar::ZERO {
      return false;
  }
  let result_point = EdwardsPoint::vartime_double_scalar_mul_basepoint(
      &scalar_a.unwrap(),
      &point,
      &scalar_b.unwrap(),
  );

  if result_point.is_identity() {
      return false;
  }

  let data = [
      hash,
      pubkey.point.as_bytes(),
      result_point.compress().as_bytes(),
  ]
  .concat();
  let c2 = hash_to_scalar(&data);

  let res = c2 - Scalar::from_bytes_mod_order(c.try_into().unwrap());

  res == Scalar::ZERO
}

pub fn generate_ring_signature<R: RngCore + CryptoRng>(
  prefix_hash: &[u8; 32],
  key_image: &EdwardsPoint,
  pubs: Vec<PublicKey>,
  sec: &PrivateKey,
  sec_idx: usize,
  rng: &mut R,
) -> Vec<[Scalar; 2]> {
  if sec_idx >= pubs.len() {
      panic!("Invalid sec_idx");
  }

  let buffer_len = 32 + 2 * 32 * pubs.len();
  let mut sig = vec![[Scalar::ZERO, Scalar::ZERO]; pubs.len()];
  let mut buff = Vec::new();
  buff.extend_from_slice(prefix_hash);
  let mut sum = Scalar::ZERO;
  let mut k = Scalar::ZERO;

  for index in 0..pubs.len() {
      if index == sec_idx {
          k = generate_random_scalar(rng);
          let tmp3 = EdwardsPoint::mul_base(&k);
          buff.extend_from_slice(&tmp3.compress().0);

          let tmp3 = monero_generators_mirror::hash_to_point(pubs[index].point.0);
          let temp2 = k * tmp3;
          buff.extend_from_slice(&temp2.compress().0);
      } else {
          sig[index][0] = generate_random_scalar(rng);
          sig[index][1] = generate_random_scalar(rng);
          let tmp3 = pubs[index].point.decompress().unwrap();
          let temp2 = EdwardsPoint::vartime_double_scalar_mul_basepoint(
              &sig[index][0],
              &tmp3,
              &sig[index][1],
          );
          buff.extend_from_slice(&temp2.compress().0);
          let tmp3 = monero_generators_mirror::hash_to_point(tmp3.compress().0);
          let tmp2 = EdwardsPoint::multiscalar_mul(
              &[sig[index][1], sig[index][0]],
              &[tmp3, key_image.clone()],
          );
          buff.extend_from_slice(&tmp2.compress().0);
          sum += sig[index][0];
      }
  }

  let h = hash_to_scalar(&buff);
  sig[sec_idx][0] = h - sum;
  sig[sec_idx][1] = k - sig[sec_idx][0] * sec.scalar;

  if buffer_len != buff.len() {
      panic!("Invalid buffer_len");
  }

  sig
}
