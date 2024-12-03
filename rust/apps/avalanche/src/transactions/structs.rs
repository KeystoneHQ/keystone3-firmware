use alloc::vec::Vec;

#[derive(Debug, Clone)]
pub struct LengthPrefixedVec<T> {
    pub len: u32,
    pub items: Vec<T>,
}

impl<T> LengthPrefixedVec<T> {
    pub fn get_len(&self) -> u32 {
        self.len
    }
    
    pub fn get(&self, index: usize) -> Option<&T> {
        self.items.get(index)
    }
}

pub trait ByteDecoder: Sized {
    fn decode_from_bytes(bytes: &[u8]) -> Result<(Self, &[u8]), &'static str>;
}