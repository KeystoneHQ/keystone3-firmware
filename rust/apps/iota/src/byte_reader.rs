use bytes::{Buf, Bytes, BytesMut};
use crate::errors::{IotaError, Result};
use alloc::vec::Vec;

pub struct BytesReader<'a> {
    pub bytes: &'a mut Bytes,
    consumed: usize,
}

impl<'a> BytesReader<'a> {
    pub fn new(bytes: &'a mut Bytes) -> Self {
        Self { bytes, consumed: 0 }
    }

    pub fn read_u8(&mut self) -> Result<u8> {
        if self.bytes.remaining() < 1 {
            return Err(IotaError::UnexpectedEof);
        }
        self.consumed += 1;
        Ok(self.bytes.get_u8())
    }

    pub fn read_u16_le(&mut self) -> Result<u16> {
        if self.bytes.remaining() < 2 {
            return Err(IotaError::UnexpectedEof);
        }
        self.consumed += 2;
        Ok(self.bytes.get_u16_le())
    }

    pub fn read_u16(&mut self) -> Result<u16> {
        if self.bytes.remaining() < 2 {
            return Err(IotaError::UnexpectedEof);
        }
        self.consumed += 2;
        Ok(self.bytes.get_u16())
    }

    pub fn peek_bytes(&self, len: usize) -> Result<Vec<u8>> {
        if self.bytes.remaining() < len {
            return Err(IotaError::UnexpectedEof);
        }
        let mut buf = vec![0u8; len];
        buf.copy_from_slice(&self.bytes.chunk()[..len]);
        Ok(buf)
    }

    pub fn read_u32_le(&mut self) -> Result<u32> {
        if self.bytes.remaining() < 4 {
            return Err(IotaError::UnexpectedEof);
        }
        self.consumed += 4;
        Ok(self.bytes.get_u32_le())
    }

    pub fn read_u64_le(&mut self) -> Result<u64> {
        if self.bytes.remaining() < 8 {
            return Err(IotaError::UnexpectedEof);
        }
        self.consumed += 8;
        Ok(self.bytes.get_u64_le())
    }

    pub fn read_bytes(&mut self, len: usize) -> Result<Vec<u8>> {
        if self.bytes.remaining() < len {
            return Err(IotaError::UnexpectedEof);
        }
        let mut buf = vec![0u8; len];
        self.bytes.copy_to_slice(&mut buf);
        self.consumed += len;
        Ok(buf)
    }

    pub fn consumed(&self) -> usize {
        self.consumed
    }

    pub fn remaining(&self) -> usize {
        self.bytes.remaining()
    }
}