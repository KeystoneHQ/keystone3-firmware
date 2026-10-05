//! SIMD-0385 signable messages (version prefix included, signatures excluded).
//! https://github.com/solana-foundation/solana-improvement-documents/blob/main/proposals/0385-transaction-v1.md
use alloc::{string::ToString, vec::Vec};

use crate::errors::{Result, SolanaError};
use crate::instruction::Instruction;
use crate::message::{Account, BlockHash, Message, MessageHeader};
use crate::parser::detail::{
    CommonDetail, ProgramDetail, ProgramDetailComputeBudget, SolanaDetail,
};

#[derive(Clone, Debug)]
pub struct TransactionConfig {
    pub priority_fee_lamports: u64,
    pub compute_unit_limit: u32,
    pub loaded_accounts_data_size_limit: u32,
    pub heap_frame_bytes: u32,
}

impl TransactionConfig {
    pub(crate) fn to_detail(&self) -> SolanaDetail {
        SolanaDetail {
            common: CommonDetail {
                program: "TransactionConfig".to_string(),
                method: "V1".to_string(),
            },
            kind: ProgramDetail::ComputeBudget(ProgramDetailComputeBudget {
                priority_fee_lamports: self.priority_fee_lamports.to_string(),
                compute_unit_limit: self.compute_unit_limit.to_string(),
                loaded_accounts_data_size_limit: self.loaded_accounts_data_size_limit.to_string(),
                heap_frame_bytes: self.heap_frame_bytes.to_string(),
                ..Default::default()
            }),
        }
    }
}

fn invalid(reason: &str) -> SolanaError {
    SolanaError::InvalidData(reason.to_string())
}

// Borrow the input while checking every length; allocate only validated fields.
struct Cursor<'a>(&'a [u8]);
impl<'a> Cursor<'a> {
    fn take(&mut self, size: usize) -> Result<&'a [u8]> {
        if size > self.0.len() {
            return Err(invalid("truncated V1 message"));
        }
        let (value, rest) = self.0.split_at(size);
        self.0 = rest;
        Ok(value)
    }

    fn byte(&mut self) -> Result<u8> {
        Ok(self.take(1)?[0])
    }

    fn u32(&mut self) -> Result<u32> {
        let bytes = self.take(4)?;
        Ok(u32::from_le_bytes([bytes[0], bytes[1], bytes[2], bytes[3]]))
    }
}

impl Message {
    pub(crate) fn read_v1(raw: &mut Vec<u8>) -> Result<Self> {
        // The 4096-byte limit includes the trailing signatures on the wire.
        if raw.len() > 4096 {
            return Err(invalid("V1 transaction exceeds size limit"));
        }
        let mut cursor = Cursor(raw);
        cursor.byte()?; // 0x81, checked by Message::read
        let header = MessageHeader {
            num_required_signatures: cursor.byte()?,
            num_readonly_signed_accounts: cursor.byte()?,
            num_readonly_unsigned_accounts: cursor.byte()?,
        };
        if header.num_required_signatures > 12
            || header.num_readonly_signed_accounts >= header.num_required_signatures
        {
            return Err(invalid("invalid V1 signature header"));
        }
        let mask = cursor.u32()?;
        if mask & !0x1f != 0 || matches!(mask & 3, 1 | 2) {
            return Err(invalid("unsupported V1 transaction config mask"));
        }
        let block_hash = BlockHash {
            value: cursor.take(32)?.to_vec(),
        };
        let instruction_count = cursor.byte()? as usize;
        let account_count = cursor.byte()? as usize;
        if instruction_count > 64 || account_count > 64 {
            return Err(invalid("V1 account or instruction count exceeds limit"));
        }
        let mut accounts: Vec<Account> = Vec::with_capacity(account_count);
        for _ in 0..account_count {
            let value = cursor.take(32)?;
            if accounts.iter().any(|account| account.value == value) {
                return Err(invalid("duplicate V1 account"));
            }
            accounts.push(Account {
                value: value.to_vec(),
            });
        }
        let priority_fee_lamports = if mask & 3 == 3 {
            let low = cursor.u32()? as u64;
            low | ((cursor.u32()? as u64) << 32)
        } else {
            0
        };
        let config = TransactionConfig {
            priority_fee_lamports,
            compute_unit_limit: if mask & 4 != 0 { cursor.u32()? } else { 0 },
            loaded_accounts_data_size_limit: if mask & 8 != 0 { cursor.u32()? } else { 0 },
            heap_frame_bytes: if mask & 16 != 0 { cursor.u32()? } else { 32768 },
        };
        if !(32768..=262144).contains(&config.heap_frame_bytes)
            || config.heap_frame_bytes % 1024 != 0
        {
            return Err(invalid("invalid V1 heap size"));
        }
        // All instruction headers precede all instruction payloads.
        let headers = cursor.take(instruction_count * 4)?;
        let mut instructions = Vec::with_capacity(instruction_count);
        for header in headers.chunks_exact(4) {
            let program_index = header[0];
            if program_index == 0 {
                return Err(invalid("V1 program cannot be the fee payer"));
            }
            let account_indexes = cursor.take(header[1] as usize)?.to_vec();
            let data_len = u16::from_le_bytes([header[2], header[3]]) as usize;
            let data = cursor.take(data_len)?.to_vec();
            instructions.push(Instruction {
                program_index,
                account_indexes,
                data,
            });
        }
        let consumed = raw.len() - cursor.0.len();
        if consumed + header.num_required_signatures as usize * 64 > 4096 {
            return Err(invalid(
                "V1 transaction exceeds size limit including signatures",
            ));
        }
        let message = Self {
            transaction_config: Some(config),
            is_versioned: true,
            header,
            accounts,
            block_hash,
            instructions,
            address_table_lookups: None,
        };
        message.validate_structure()?;
        raw.drain(..consumed);
        Ok(message)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{classify_payload, parse, parse_for_signer, read::Read, SolanaPayloadType};
    use alloc::vec;

    fn encode(
        mask: u32,
        config: &[u8],
        accounts: &[[u8; 32]],
        instructions: &[Instruction],
    ) -> Vec<u8> {
        let mut raw = vec![0x81, 1, 0, 1];
        raw.extend(mask.to_le_bytes());
        raw.extend([7; 32]);
        raw.extend([instructions.len() as u8, accounts.len() as u8]);
        for account in accounts {
            raw.extend(account);
        }
        raw.extend(config);
        for instruction in instructions {
            raw.extend([
                instruction.program_index,
                instruction.account_indexes.len() as u8,
            ]);
            raw.extend((instruction.data.len() as u16).to_le_bytes());
        }
        for instruction in instructions {
            raw.extend(&instruction.account_indexes);
            raw.extend(&instruction.data);
        }
        raw
    }

    fn transfer() -> Instruction {
        let mut data = 2u32.to_le_bytes().to_vec();
        data.extend(123456789u64.to_le_bytes());
        Instruction {
            program_index: 2,
            account_indexes: vec![0, 1],
            data,
        }
    }

    fn sample() -> Vec<u8> {
        encode(0, &[], &[[1; 32], [2; 32], [0; 32]], &[transfer()])
    }

    #[test]
    fn transfer_and_defaults_are_reviewed() {
        let raw = sample();
        assert_eq!(classify_payload(&raw), SolanaPayloadType::Transaction);
        let parsed = parse_for_signer(&raw, &[1; 32]).unwrap();
        assert_eq!(parsed.display_type.to_string(), "Transfer");
        assert!(parsed.detail.contains("0.123456789 SOL"));
        let config = &parsed.additional_overviews[0];
        assert_eq!(config.instruction_index, 0);
        assert!(config.memo.contains("Priority Fee: 0 lamports"));
        assert!(config.memo.contains("Heap Size: 32768 bytes"));
        assert!(parse_for_signer(&raw, &[2; 32]).is_err());
        assert!(parse_for_signer(&raw, &[9; 32]).is_err());
    }

    #[test]
    fn every_config_mask_and_full_u64_fee() {
        for mask in 0u32..32 {
            if matches!(mask & 3, 1 | 2) {
                continue;
            }
            let mut values = Vec::new();
            if mask & 3 == 3 {
                values.extend(u64::MAX.to_le_bytes());
            }
            if mask & 4 != 0 {
                values.extend(200000u32.to_le_bytes());
            }
            if mask & 8 != 0 {
                values.extend(65536u32.to_le_bytes());
            }
            if mask & 16 != 0 {
                values.extend(65536u32.to_le_bytes());
            }
            let mut raw = encode(mask, &values, &[[1; 32], [2; 32], [0; 32]], &[transfer()]);
            let message = Message::read_exact(&mut raw).unwrap();
            let config = message.transaction_config.unwrap();
            assert_eq!(
                config.priority_fee_lamports,
                if mask & 3 == 3 { u64::MAX } else { 0 }
            );
            assert_eq!(
                config.compute_unit_limit,
                if mask & 4 != 0 { 200000 } else { 0 }
            );
            assert_eq!(
                config.loaded_accounts_data_size_limit,
                if mask & 8 != 0 { 65536 } else { 0 }
            );
            assert_eq!(
                config.heap_frame_bytes,
                if mask & 16 != 0 { 65536 } else { 32768 }
            );
        }
    }

    #[test]
    fn split_headers_and_compute_budget_noop() {
        let budget: [u8; 32] = bs58::decode("ComputeBudget111111111111111111111111111111")
            .into_vec()
            .unwrap()
            .try_into()
            .unwrap();
        for data in [
            vec![],
            vec![255],
            vec![3, 255, 255, 255, 255, 255, 255, 255, 255],
        ] {
            let raw = encode(
                3,
                &42u64.to_le_bytes(),
                &[[1; 32], [2; 32], [0; 32], budget],
                &[
                    transfer(),
                    Instruction {
                        program_index: 3,
                        account_indexes: vec![],
                        data,
                    },
                    transfer(),
                ],
            );
            let parsed = parse(&raw).unwrap();
            assert!(parsed.detail.contains("IgnoredInV1"));
            assert!(!parsed.detail.contains("compute_unit_price_micro_lamports"));
            assert!(parsed.detail.contains("\"priority_fee_lamports\":\"42\""));
            let message = Message::read_exact(&mut raw.clone()).unwrap();
            assert_eq!(message.instructions[0].data, message.instructions[2].data);
        }
    }

    fn rejected(raw: &[u8]) {
        assert!(Message::read_exact(&mut raw.to_vec()).is_err());
        assert_eq!(
            classify_payload(raw),
            SolanaPayloadType::MalformedTransaction
        );
        assert!(parse(&raw.to_vec()).is_err());
    }

    #[test]
    fn truncations_and_trailing_signatures_never_fall_back_to_message() {
        let raw = sample();
        for len in 1..raw.len() {
            rejected(&raw[..len]);
        }
        for suffix in [vec![0], vec![0; 64], vec![0; 128]] {
            let mut invalid = raw.clone();
            invalid.extend(suffix);
            rejected(&invalid);
        }
    }

    #[test]
    fn rejects_invalid_headers_masks_accounts_and_indices() {
        for (offset, value) in [
            (1, 0),
            (1, 13),
            (2, 1),
            (3, 3),
            (4, 1),
            (4, 2),
            (4, 32),
            (7, 128),
            (40, 65),
            (41, 65),
            (138, 0),
            (138, 3),
            (142, 3),
        ] {
            let mut raw = sample();
            raw[offset] = value;
            rejected(&raw);
        }
        let mut raw = sample();
        raw[74..106].fill(1);
        rejected(&raw);
        for heap in [0u32, 32767, 32769, 262145] {
            rejected(&encode(16, &heap.to_le_bytes(), &[[1; 32], [0; 32]], &[]));
        }
    }

    #[test]
    fn enforces_size_including_signatures_and_accepts_large_instruction() {
        // Header 42 + two addresses 64 + instruction header 4 + signature 64.
        for (data_len, valid) in [(1233, true), (3922, true), (3923, false), (4096, false)] {
            let raw = encode(
                0,
                &[],
                &[[1; 32], [2; 32]],
                &[Instruction {
                    program_index: 1,
                    account_indexes: vec![],
                    data: vec![3; data_len],
                }],
            );
            assert_eq!(Message::read_exact(&mut raw.clone()).is_ok(), valid);
            if !valid {
                rejected(&raw);
            }
        }
    }
    #[test]
    fn official_sdk_message_and_independent_signature_match() {
        let fixture: serde_json::Value =
            serde_json::from_str(include_str!("../tests/fixtures/v1-transfer.json")).unwrap();
        let raw = hex::decode(fixture["message"].as_str().unwrap()).unwrap();
        let seed = hex::decode(fixture["seed"].as_str().unwrap()).unwrap();
        let path = fixture["path"].as_str().unwrap().to_string();
        let signer = crate::get_public_key(&seed, &path).unwrap();
        let parsed = parse_for_signer(&raw, &signer).unwrap();
        assert_eq!(parsed.display_type.to_string(), "Transfer");
        assert!(parsed.additional_overviews[0]
            .memo
            .contains("9876543210 lamports"));
        let signature = crate::sign(raw.clone(), &path, &seed).unwrap();
        assert_eq!(
            hex::encode(signature),
            fixture["signature"].as_str().unwrap()
        );
        let message = Message::read_exact(&mut raw.clone()).unwrap();
        assert!(message.address_table_lookups.is_none());
        assert_eq!(message.instructions[0].data, transfer().data);
        assert_eq!(message.instructions[0].account_indexes, vec![0, 1]);
        assert_eq!(
            message.transaction_config.unwrap().compute_unit_limit,
            200000
        );
    }

    #[test]
    fn accepts_max_counts_and_readonly_cosigner() {
        let accounts: Vec<[u8; 32]> = (0..64).map(|i| [i; 32]).collect();
        let instructions = vec![Instruction {
            program_index: 63,
            account_indexes: vec![0; 255],
            data: vec![],
        }];
        let mut raw = encode(0, &[], &accounts, &instructions);
        raw[1] = 12;
        raw[2] = 11;
        let message = Message::read_exact(&mut raw).unwrap();
        assert!(message.validate_signer(&[11; 32]).is_ok());
        assert!(message.validate_signer(&[12; 32]).is_err());
        let instructions = vec![
            Instruction {
                program_index: 1,
                account_indexes: vec![],
                data: vec![],
            };
            64
        ];
        let raw = encode(0, &[], &[[1; 32], [2; 32]], &instructions);
        assert!(Message::read_exact(&mut raw.clone()).is_ok());
    }
}
