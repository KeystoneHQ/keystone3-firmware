use core::ops::{Add, ShlAssign};

use alloc::vec;
use alloc::vec::Vec;
use num_bigint::BigUint;
use num_traits::Zero;

#[derive(Clone)]
pub(crate) struct BitString {
    value: BigUint,
    bit_len: usize,
}

impl BitString {
    pub fn new() -> Self {
        BitString {
            value: BigUint::zero(),
            bit_len: 0,
        }
    }

    pub fn shl_assign_and_add(&mut self, rhs: usize, val: BigUint) {
        self.value.shl_assign(rhs);
        self.value += val;
        self.bit_len += rhs;
    }

    pub fn shl_assign_and_fill(&mut self, rhs: usize) {
        let val = create_biguint_with_ones(rhs);
        self.shl_assign_and_add(rhs, val)
    }

    pub fn shl_assign(&mut self, rhs: usize) {
        self.value.shl_assign(rhs);
        self.bit_len += rhs;
    }

    pub fn bit_len(&self) -> usize {
        self.bit_len
    }

    pub fn get_value_as_bytes(&self) -> Vec<u8> {
        self.value.to_bytes_be()
    }
}

impl Add<BigUint> for BitString {
    type Output = BitString;
    fn add(mut self, other: BigUint) -> BitString {
        self.value += other;
        self
    }
}
fn create_biguint_with_ones(n: usize) -> BigUint {
    let mut msb = vec![(1u8 << (n % 8)) - 1];
    let lsb = vec![0xffu8; n / 8];
    msb.extend(lsb);
    BigUint::from_bytes_be(&msb)
}
