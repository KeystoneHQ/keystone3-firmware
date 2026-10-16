use crate::errors::{MoneroError, Result};
use crate::transfer::{
    AccountPublicAddress, OutputEntry, TxConstructionData, TxDestinationEntry, TxSourceEntry,
};
use crate::utils::varinteger::*;
use alloc::string::String;
use alloc::vec;
use alloc::vec::Vec;

pub const MAX_REQUEST_BYTES: usize = 14 * 1024;

pub const MAX_TXES: usize = 256;
pub const MAX_SOURCES: usize = 4096;
pub const MAX_RING_SIZE: usize = 4096;
pub const MAX_DESTINATIONS: usize = 4096;
pub const MAX_ADDITIONAL_KEYS: usize = 4096;
pub const MAX_SELECTED_TRANSFERS: usize = 4096;
pub const MAX_SUBADDR_INDICES: usize = 4096;
pub const MAX_EXTRA_LEN: usize = MAX_REQUEST_BYTES;
pub const MAX_ADDRESS_STRING: usize = 512;

pub struct Reader<'a> {
    data: &'a [u8],
    offset: usize,
}

impl<'a> Reader<'a> {
    pub fn new(data: &'a [u8]) -> Self {
        Reader { data, offset: 0 }
    }

    pub fn offset(&self) -> usize {
        self.offset
    }

    pub fn remaining(&self) -> usize {
        self.data.len().saturating_sub(self.offset)
    }

    pub fn remaining_bytes(&self) -> &'a [u8] {
        let start = self.offset.min(self.data.len());
        &self.data[start..]
    }

    pub fn read_bytes(&mut self, len: usize) -> Result<&'a [u8]> {
        let end = self
            .offset
            .checked_add(len)
            .ok_or(MoneroError::FormatError)?;
        if end > self.data.len() {
            return Err(MoneroError::FormatError);
        }
        let out = &self.data[self.offset..end];
        self.offset = end;
        Ok(out)
    }

    pub fn read_u8(&mut self) -> Result<u8> {
        Ok(self.read_bytes(1)?[0])
    }

    pub fn read_u32(&mut self) -> Result<u32> {
        let bytes: [u8; 4] = self
            .read_bytes(4)?
            .try_into()
            .map_err(|_| MoneroError::FormatError)?;
        Ok(u32::from_le_bytes(bytes))
    }

    pub fn read_u64(&mut self) -> Result<u64> {
        let bytes: [u8; 8] = self
            .read_bytes(8)?
            .try_into()
            .map_err(|_| MoneroError::FormatError)?;
        Ok(u64::from_le_bytes(bytes))
    }

    pub fn read_bool(&mut self) -> Result<bool> {
        Ok(self.read_u8()? != 0)
    }

    pub fn read_array32(&mut self) -> Result<[u8; 32]> {
        let bytes: [u8; 32] = self
            .read_bytes(32)?
            .try_into()
            .map_err(|_| MoneroError::FormatError)?;
        Ok(bytes)
    }

    pub fn read_varint(&mut self) -> Result<u64> {
        let mut value = 0u64;
        let mut shift = 0u32;

        for _ in 0..10 {
            let byte = self.read_u8()?;
            let low = (byte & 0x7f) as u64;
            if shift == 63 && low > 1 {
                return Err(MoneroError::FormatError);
            }
            value |= low << shift;
            if byte & 0x80 == 0 {
                return Ok(value);
            }
            shift += 7;
        }

        Err(MoneroError::FormatError)
    }

    pub fn read_count(&mut self, max: usize) -> Result<usize> {
        let value = self.read_varint()?;
        if value > max as u64 {
            return Err(MoneroError::FormatError);
        }
        let value = value as usize;
        if value > self.remaining().saturating_add(1) {
            return Err(MoneroError::FormatError);
        }
        Ok(value)
    }

    pub fn read_string(&mut self, max: usize) -> Result<String> {
        let len = self.read_count(max)?;
        let bytes = self.read_bytes(len)?;
        String::from_utf8(bytes.to_vec()).map_err(|_| MoneroError::FormatError)
    }

    pub fn read_vec_u8(&mut self, max: usize) -> Result<Vec<u8>> {
        let len = self.read_count(max)?;
        Ok(self.read_bytes(len)?.to_vec())
    }

    pub fn read_vec_array32(&mut self, max: usize) -> Result<Vec<[u8; 32]>> {
        let len = self.read_count(max)?;
        let mut res = Vec::with_capacity(len.min(64));
        for _ in 0..len {
            res.push(self.read_array32()?);
        }
        Ok(res)
    }

    pub fn read_tx_destination_entry(&mut self) -> Result<TxDestinationEntry> {
        let original = self.read_string(MAX_ADDRESS_STRING)?;
        let amount = self.read_varint()?;
        let spend_public_key = self.read_array32()?;
        let view_public_key = self.read_array32()?;
        let is_subaddress = self.read_bool()?;
        let is_integrated = self.read_bool()?;

        Ok(TxDestinationEntry {
            original,
            amount,
            addr: AccountPublicAddress {
                spend_public_key,
                view_public_key,
            },
            is_subaddress,
            is_integrated,
        })
    }
}

pub fn write_varinteger(value: u64) -> Vec<u8> {
    let mut buffer = vec![0u8; length(value)];
    encode_with_offset(value, &mut buffer, 0);
    buffer
}

pub fn write_tx_destination_entry(entry: &TxDestinationEntry) -> Vec<u8> {
    let mut buffer = Vec::new();
    buffer.extend_from_slice(&write_varinteger(entry.original.len() as u64));
    buffer.extend_from_slice(entry.original.as_bytes());
    buffer.extend_from_slice(&write_varinteger(entry.amount));
    buffer.extend_from_slice(&entry.addr.spend_public_key);
    buffer.extend_from_slice(&entry.addr.view_public_key);
    buffer.push(entry.is_subaddress as u8);
    buffer.push(entry.is_integrated as u8);
    buffer
}

pub fn write_output_entry(entry: &OutputEntry) -> Vec<u8> {
    let mut buffer = Vec::new();
    buffer.push(2u8);
    buffer.extend_from_slice(&write_varinteger(entry.index));
    buffer.extend_from_slice(&entry.key.dest);
    buffer.extend_from_slice(&entry.key.mask);
    buffer
}

pub fn write_tx_source_entry(entry: &TxSourceEntry) -> Vec<u8> {
    let mut buffer = Vec::new();
    buffer.extend_from_slice(&write_varinteger(entry.outputs.len() as u64));
    for output in entry.outputs.iter() {
        buffer.extend_from_slice(&write_output_entry(output));
    }
    buffer.extend_from_slice(&entry.real_output.to_le_bytes());
    buffer.extend_from_slice(&entry.real_out_tx_key);
    buffer.extend_from_slice(&write_varinteger(
        entry.real_out_additional_tx_keys.len() as u64
    ));
    for key in entry.real_out_additional_tx_keys.iter() {
        buffer.extend_from_slice(key);
    }
    buffer.extend_from_slice(&entry.real_output_in_tx_index.to_le_bytes());
    buffer.extend_from_slice(&entry.amount.to_le_bytes());
    buffer.push(entry.rct as u8);
    buffer.extend_from_slice(&entry.mask);
    buffer.extend_from_slice(&entry.multisig_kLRki.k);
    buffer.extend_from_slice(&entry.multisig_kLRki.L);
    buffer.extend_from_slice(&entry.multisig_kLRki.R);
    buffer.extend_from_slice(&entry.multisig_kLRki.ki);
    buffer
}

pub fn write_tx_construction_data(data: &TxConstructionData) -> Vec<u8> {
    let mut buffer = Vec::new();
    buffer.extend_from_slice(&write_varinteger(data.sources.len() as u64));
    for source in data.sources.iter() {
        buffer.extend_from_slice(&write_tx_source_entry(source));
    }
    buffer.extend_from_slice(&write_tx_destination_entry(&data.change_dts));
    buffer.extend_from_slice(&write_varinteger(data.splitted_dsts.len() as u64));
    for dst in data.splitted_dsts.iter() {
        buffer.extend_from_slice(&write_tx_destination_entry(dst));
    }
    buffer.extend_from_slice(&write_varinteger(data.selected_transfers.len() as u64));
    for transfer in data.selected_transfers.iter() {
        buffer.extend_from_slice(&write_varinteger(*transfer as u64));
    }
    buffer.extend_from_slice(&write_varinteger(data.extra.len() as u64));
    buffer.extend_from_slice(&data.extra);
    buffer.extend_from_slice(&data.unlock_time.to_le_bytes());
    buffer.push(data.use_rct);
    buffer.extend_from_slice(&write_varinteger(data.rct_config.version as u64));
    buffer.extend_from_slice(&write_varinteger(data.rct_config.range_proof_type as u64));
    buffer.extend_from_slice(&write_varinteger(data.rct_config.bp_version as u64));
    // dests
    buffer.extend_from_slice(&write_varinteger(data.dests.len() as u64));
    for dest in data.dests.iter() {
        buffer.extend_from_slice(&write_tx_destination_entry(dest));
    }
    buffer.extend_from_slice(&data.subaddr_account.to_le_bytes());
    buffer.extend_from_slice(&write_varinteger(data.subaddr_indices.len() as u64));
    for index in data.subaddr_indices.iter() {
        buffer.extend_from_slice(&write_varinteger(*index as u64));
    }
    buffer
}

pub fn read_next_u8_32(bytes: &[u8], offset: &mut usize) -> [u8; 32] {
    let mut data = [0u8; 32];
    data.copy_from_slice(&bytes[*offset..*offset + 32]);
    *offset += 32;

    data
}

#[cfg(test)]
mod tests {
    use super::*;

    fn encoded_destination() -> Vec<u8> {
        let spend = [0x11u8; 32];
        let view = [0x22u8; 32];
        let mut bytes = write_varinteger(6);
        bytes.extend_from_slice(b"48A1-t");
        bytes.extend_from_slice(&write_varinteger(12_345));
        bytes.extend_from_slice(&spend);
        bytes.extend_from_slice(&view);
        bytes.push(1);
        bytes.push(0);
        bytes
    }

    #[test]
    fn primitives_read_and_advance_the_offset() {
        let mut bytes = vec![0x7F];
        bytes.extend_from_slice(&0x1234_5678u32.to_le_bytes());
        bytes.extend_from_slice(&0x0123_4567_89AB_CDEFu64.to_le_bytes());
        bytes.push(1);
        bytes.extend_from_slice(&[0xAA; 32]);

        let mut reader = Reader::new(&bytes);
        assert_eq!(reader.read_u8().unwrap(), 0x7F);
        assert_eq!(reader.read_u32().unwrap(), 0x1234_5678);
        assert_eq!(reader.read_u64().unwrap(), 0x0123_4567_89AB_CDEF);
        assert!(reader.read_bool().unwrap());
        assert_eq!(reader.read_array32().unwrap(), [0xAA; 32]);
        assert_eq!(reader.offset(), bytes.len());
        assert_eq!(reader.remaining(), 0);
    }

    #[test]
    fn varinteger_roundtrip_updates_the_offset() {
        for value in [0, 127, 128, 16_384, u32::MAX as u64, u64::MAX] {
            let encoded = write_varinteger(value);
            let mut reader = Reader::new(&encoded);
            assert_eq!(reader.read_varint().unwrap(), value);
            assert_eq!(reader.offset(), encoded.len());
        }
    }

    #[test]
    fn destination_entry_roundtrip() {
        let encoded = encoded_destination();
        let mut reader = Reader::new(&encoded);
        let decoded = reader.read_tx_destination_entry().unwrap();

        assert_eq!(decoded.original, "48A1-t");
        assert_eq!(decoded.amount, 12_345);
        assert_eq!(decoded.addr.spend_public_key, [0x11u8; 32]);
        assert_eq!(decoded.addr.view_public_key, [0x22u8; 32]);
        assert!(decoded.is_subaddress);
        assert!(!decoded.is_integrated);
        assert_eq!(reader.offset(), encoded.len());
    }

    #[test]
    fn truncated_reads_return_an_error_instead_of_panicking() {
        let mut reader = Reader::new(&[0x80]);
        assert!(reader.read_varint().is_err());

        let mut reader = Reader::new(&[0u8; 4]);
        assert!(reader.read_array32().is_err());

        let mut reader = Reader::new(&[0xff, 0xff, 0xff, 0xff, 0x0f]);
        assert!(reader.read_count(MAX_SOURCES).is_err());

        let mut reader = Reader::new(&[0x80, 0x80, 0x80, 0x80, 0x08]);
        assert!(reader.read_count(16).is_err());

        let mut bytes = write_varinteger(1);
        bytes.push(0xFF);
        let mut reader = Reader::new(&bytes);
        assert!(reader.read_tx_destination_entry().is_err());
    }

    #[test]
    fn oversized_varints_are_rejected() {
        let bytes = [0xFFu8; 10];
        let mut reader = Reader::new(&bytes);
        assert!(reader.read_varint().is_err());
    }
}
