use alloc::vec::Vec;
use pasta_curves::{Fp, Fq};
use ff::Field;

use super::{Mds, Spec};

#[derive(Debug)]
pub struct P128Pow5T3;

impl Spec<Fp, 3, 2> for P128Pow5T3 {
    fn full_rounds() -> usize {
        8
    }

    fn partial_rounds() -> usize {
        56
    }

    fn sbox(val: Fp) -> Fp {
        val.pow_vartime([5])
    }

    fn secure_mds() -> usize {
        unimplemented!()
    }

    fn constants() -> (Vec<[Fp; 3]>, Mds<Fp, 3>, Mds<Fp, 3>) {
        (
            super::fp::ROUND_CONSTANTS[..].to_vec(),
            super::fp::MDS,
            super::fp::MDS_INV,
        )
    }
}

impl Spec<Fq, 3, 2> for P128Pow5T3 {
    fn full_rounds() -> usize {
        8
    }

    fn partial_rounds() -> usize {
        56
    }

    fn sbox(val: Fq) -> Fq {
        val.pow_vartime([5])
    }

    fn secure_mds() -> usize {
        unimplemented!()
    }

    fn constants() -> (Vec<[Fq; 3]>, Mds<Fq, 3>, Mds<Fq, 3>) {
        (
            super::fq::ROUND_CONSTANTS[..].to_vec(),
            super::fq::MDS,
            super::fq::MDS_INV,
        )
    }
}