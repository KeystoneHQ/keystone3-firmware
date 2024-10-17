use alloc::vec::Vec;
use crate::key::KeyPair;
use third_party::hex;

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::vec;
    use zeroize::Zeroizing;
    use core::ops::Deref;
    use curve25519_dalek::scalar::Scalar;
    use curve25519_dalek::edwards::{CompressedEdwardsY, EdwardsPoint};
    use rand_core::{RngCore, OsRng};
    use crate::key::generate_keypair;

    use monero_clsag_mirror::{Clsag, ClsagContext};

    #[test]
    fn test_clsag_signature() {
        const RING_LEN: u64 = 11;
        const AMOUNT: u64 = 1337;

        for real in 0 .. RING_LEN {
            let msg = [1; 32];

            let mut secrets = (Zeroizing::new(Scalar::ZERO), Scalar::ZERO);
            let mut ring = vec![];
            for i in 0 .. RING_LEN {
                let dest = Zeroizing::new(Scalar::random(&mut OsRng));
                let mask = Scalar::random(&mut OsRng);
                let amount;
                    if i == real {
                        secrets = (dest.clone(), mask);
                        amount = AMOUNT;
                    } else {
                        amount = OsRng.next_u64();
                    }
                    let point = EdwardsPoint::mul_base(dest.deref());
                    ring
                        .push([point, monero_primitives_mirror::Commitment::new(mask, amount).calculate()]);
            }

            let (mut clsag, pseudo_out) = Clsag::sign(
                &mut OsRng,
                vec![(
                    secrets.0.clone(),
                    ClsagContext::new(
                        monero_primitives_mirror::Decoys::new((1 ..= RING_LEN).collect(), u8::try_from(real).unwrap(), ring.clone())
                            .unwrap(),
                        monero_primitives_mirror::Commitment::new(secrets.1, AMOUNT),
                    )
                    .unwrap(),
                )],
                Scalar::random(&mut OsRng),
                msg,
            )
            .unwrap()
            .swap_remove(0);

            let image =
                monero_generators_mirror::hash_to_point((EdwardsPoint::mul_base(secrets.0.deref())).compress().0) * secrets.0.deref();

            assert_eq!(clsag.verify(&ring, &image, &pseudo_out, &msg), Ok(()));
        }
    }
}