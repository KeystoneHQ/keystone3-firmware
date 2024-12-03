use alloc::vec::Vec;

#[derive(Debug, Clone)]
pub struct LengthPrefixedVec<T> {
    pub index: u32,
    pub items: Vec<T>,
}
