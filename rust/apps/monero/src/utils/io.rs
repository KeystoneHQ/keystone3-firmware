use alloc::vec;
use alloc::vec::Vec;
use alloc::string::String;
use crate::utils::varinteger::*;
use crate::transfer::{
  TxConstructionData, TxDestinationEntry, OutputEntry,
  TxSourceEntry, AccountPublicAddress,
};

pub fn read_varinteger(data: &[u8], offset: &mut usize) -> u64 {
  let mut value = 0u64;
  *offset += decode_with_offset(data, *offset, &mut value);

  value
}

pub fn write_varinteger(value: u64) -> Vec<u8> {
  let mut buffer = vec![0u8; length(value)];
  encode_with_offset(value, &mut buffer, 0);
  buffer
}


pub fn read_next_u8(bytes: &[u8], offset: &mut usize) -> u8 {
  let value = u8::from_le_bytes(bytes[*offset..*offset + 1].try_into().unwrap());
  *offset += 1;
  value
}

pub fn read_next_u32(bytes: &[u8], offset: &mut usize) -> u32 {
  let value = u32::from_le_bytes(bytes[*offset..*offset + 4].try_into().unwrap());
  *offset += 4;
  value
}

pub fn read_next_u64(bytes: &[u8], offset: &mut usize) -> u64 {
  let value = u64::from_le_bytes(bytes[*offset..*offset + 8].try_into().unwrap());
  *offset += 8;
  value
}

pub fn read_next_bool(bytes: &[u8], offset: &mut usize) -> bool {
  read_next_u8(bytes, offset) != 0
}

pub fn read_next_vec_u8(bytes: &[u8], offset: &mut usize, len: usize) -> Vec<u8> {
  let value = bytes[*offset..*offset + len].to_vec();
  *offset += len;
  value
}

pub fn read_next_tx_destination_entry(bytes: &[u8], offset: &mut usize) -> TxDestinationEntry {
  let original_len = read_varinteger(bytes, offset) as usize;
  let original = String::from_utf8(bytes[*offset..*offset + original_len].to_vec()).unwrap();
  *offset += original_len;
  let amount = read_varinteger(bytes, offset);
  let mut spend_public_key = [0u8; 32];
  spend_public_key.copy_from_slice(&bytes[*offset..*offset + 32]);
  *offset += 32;
  let mut view_public_key = [0u8; 32];
  view_public_key.copy_from_slice(&bytes[*offset..*offset + 32]);
  *offset += 32;
  let is_subaddress = read_next_bool(bytes, offset);
  let is_integrated = read_next_bool(bytes, offset);
  TxDestinationEntry {
      original,
      amount,
      addr: AccountPublicAddress { spend_public_key, view_public_key },
      is_subaddress,
      is_integrated,
  }
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
  buffer.extend_from_slice(&write_varinteger(entry.real_out_additional_tx_keys.len() as u64));
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
  buffer.push(data.use_rct as u8);
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