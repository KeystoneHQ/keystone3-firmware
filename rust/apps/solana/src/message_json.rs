use alloc::{string::String, vec::Vec};
use bitcoin::base58;
use serde_json::{json, Value};

use crate::{errors::Result, message::Message};

impl Message {
    pub(crate) fn to_message_json(&self) -> Result<String> {
        let header = json!({
            "num_required_signatures": self.header.num_required_signatures,
            "num_readonly_signed_accounts": self.header.num_readonly_signed_accounts,
            "num_readonly_unsigned_accounts": self.header.num_readonly_unsigned_accounts,
        });
        let addresses: Vec<String> = self
            .accounts
            .iter()
            .map(|a| base58::encode(&a.value))
            .collect();
        let blockhash = base58::encode(&self.block_hash.value);
        let value = if let Some(config) = &self.transaction_config {
            let mut words = Vec::<u32>::new();
            if config.mask & 3 == 3 {
                words.push(config.priority_fee_lamports as u32);
                words.push((config.priority_fee_lamports >> 32) as u32);
            }
            if config.mask & 4 != 0 {
                words.push(config.compute_unit_limit);
            }
            if config.mask & 8 != 0 {
                words.push(config.loaded_accounts_data_size_limit);
            }
            if config.mask & 16 != 0 {
                words.push(config.heap_frame_bytes);
            }
            json!({
                "version": 1,
                "version_byte": 129,
                "header": header,
                "transaction_config_mask": config.mask,
                "lifetime_specifier": blockhash,
                "num_instructions": self.instructions.len(),
                "num_addresses": addresses.len(),
                "addresses": addresses,
                "config_values": words,
                "instruction_data_encoding": "base58",
                "instruction_headers": self.instructions.iter().map(|i| json!({
                    "program_account_index": i.program_index,
                    "num_instruction_accounts": i.account_indexes.len(),
                    "num_instruction_data_bytes": i.data.len(),
                })).collect::<Vec<Value>>(),
                "instruction_payloads": self.instructions.iter().map(|i| json!({
                    "instruction_account_indexes": i.account_indexes,
                    "instruction_data": base58::encode(&i.data),
                })).collect::<Vec<Value>>(),
            })
        } else {
            let mut value = json!({
                "version": if self.is_versioned { json!(0) } else { json!("legacy") },
                "header": header,
                "account_keys": addresses,
                "recent_blockhash": blockhash,
                "instruction_data_encoding": "base58",
                "instructions": self.instructions.iter().map(|i| json!({
                    "program_id_index": i.program_index,
                    "accounts": i.account_indexes,
                    "data": base58::encode(&i.data),
                })).collect::<Vec<Value>>(),
            });
            if let Some(lookups) = &self.address_table_lookups {
                value["version_byte"] = json!(128);
                value["address_table_lookups"] = json!(lookups
                    .iter()
                    .map(|lookup| json!({
                        "account_key": base58::encode(&lookup.account_key.value),
                        "writable_indexes": lookup.writable_indexes,
                        "readonly_indexes": lookup.readonly_indexes,
                    }))
                    .collect::<Vec<Value>>());
            }
            value
        };
        Ok(serde_json::to_string_pretty(&value)?)
    }
}

#[cfg(test)]
pub(crate) mod tests {
    use super::*;

    fn number(value: &Value) -> u64 {
        value.as_u64().unwrap()
    }
    fn byte(value: &Value) -> u8 {
        number(value).try_into().unwrap()
    }
    fn bytes(value: &Value) -> Vec<u8> {
        value.as_array().unwrap().iter().map(byte).collect()
    }
    fn decode(value: &Value) -> Vec<u8> {
        base58::decode(value.as_str().unwrap()).unwrap()
    }
    fn shortvec(out: &mut Vec<u8>, mut length: usize) {
        loop {
            let mut next = (length & 127) as u8;
            length >>= 7;
            if length != 0 {
                next |= 128;
            }
            out.push(next);
            if length == 0 {
                break;
            }
        }
    }

    pub(crate) fn assert_json_roundtrip(original: &[u8], rendered: &str) {
        let v: Value = serde_json::from_str(rendered).unwrap();
        let mut out = Vec::new();
        if v["version"] != "legacy" {
            out.push(byte(&v["version_byte"]));
        }
        for key in [
            "num_required_signatures",
            "num_readonly_signed_accounts",
            "num_readonly_unsigned_accounts",
        ] {
            out.push(byte(&v["header"][key]));
        }
        assert_eq!(v["instruction_data_encoding"], "base58");
        if v["version"] == 1 {
            let mask: u32 = number(&v["transaction_config_mask"]).try_into().unwrap();
            out.extend(mask.to_le_bytes());
            out.extend(decode(&v["lifetime_specifier"]));
            out.push(byte(&v["num_instructions"]));
            out.push(byte(&v["num_addresses"]));
            let addresses = v["addresses"].as_array().unwrap();
            assert_eq!(addresses.len(), number(&v["num_addresses"]) as usize);
            for address in addresses {
                out.extend(decode(address));
            }
            let words = v["config_values"].as_array().unwrap();
            assert_eq!(words.len(), mask.count_ones() as usize);
            for word in words {
                let word: u32 = number(word).try_into().unwrap();
                out.extend(word.to_le_bytes());
            }
            let headers = v["instruction_headers"].as_array().unwrap();
            let payloads = v["instruction_payloads"].as_array().unwrap();
            assert_eq!(headers.len(), number(&v["num_instructions"]) as usize);
            assert_eq!(headers.len(), payloads.len());
            for h in headers {
                out.push(byte(&h["program_account_index"]));
                out.push(byte(&h["num_instruction_accounts"]));
                let length: u16 = number(&h["num_instruction_data_bytes"]).try_into().unwrap();
                out.extend(length.to_le_bytes());
            }
            for (h, p) in headers.iter().zip(payloads) {
                let accounts = bytes(&p["instruction_account_indexes"]);
                let data = decode(&p["instruction_data"]);
                assert_eq!(
                    accounts.len(),
                    number(&h["num_instruction_accounts"]) as usize
                );
                assert_eq!(
                    data.len(),
                    number(&h["num_instruction_data_bytes"]) as usize
                );
                out.extend(accounts);
                out.extend(data);
            }
        } else {
            let accounts = v["account_keys"].as_array().unwrap();
            shortvec(&mut out, accounts.len());
            for account in accounts {
                out.extend(decode(account));
            }
            out.extend(decode(&v["recent_blockhash"]));
            let instructions = v["instructions"].as_array().unwrap();
            shortvec(&mut out, instructions.len());
            for i in instructions {
                out.push(byte(&i["program_id_index"]));
                let accounts = bytes(&i["accounts"]);
                shortvec(&mut out, accounts.len());
                out.extend(accounts);
                let data = decode(&i["data"]);
                shortvec(&mut out, data.len());
                out.extend(data);
            }
            if v["version"] == 0 {
                let lookups = v["address_table_lookups"].as_array().unwrap();
                shortvec(&mut out, lookups.len());
                for lookup in lookups {
                    out.extend(decode(&lookup["account_key"]));
                    for key in ["writable_indexes", "readonly_indexes"] {
                        let indexes = bytes(&lookup[key]);
                        shortvec(&mut out, indexes.len());
                        out.extend(indexes);
                    }
                }
            }
        }
        assert_eq!(
            out, original,
            "Details must preserve the signable message bytes"
        );
    }
}
