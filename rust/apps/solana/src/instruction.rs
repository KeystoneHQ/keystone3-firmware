use alloc::string::{String, ToString};
use alloc::vec::Vec;

use crate::compact::Compact;
use crate::errors::SolanaError::ProgramError;
use crate::errors::{Result, SolanaError};
use crate::parser::detail::{
    CommonDetail, ProgramDetail, ProgramDetailComputeBudget, SolanaDetail,
};
use crate::resolvers;
use crate::solana_lib::solana_program::stake::instruction::StakeInstruction;
use crate::solana_lib::solana_program::system_instruction::SystemInstruction;
use crate::solana_lib::solana_program::vote::instruction::VoteInstruction;
use crate::solana_lib::traits::Dispatch;
use crate::Read;
#[derive(Debug, Clone)]
pub struct Instruction {
    pub(crate) program_index: u8,
    pub(crate) account_indexes: Vec<u8>,
    pub(crate) data: Vec<u8>,
}

impl Read<Instruction> for Instruction {
    fn read(raw: &mut Vec<u8>) -> Result<Instruction> {
        if raw.is_empty() {
            return Err(SolanaError::InvalidData("instruction".to_string()));
        }
        let program_index = raw.remove(0);
        let account_indexes = Compact::read(raw)?.data;
        let data = Compact::read(raw)?.data;
        Ok(Instruction {
            program_index,
            account_indexes,
            data,
        })
    }
}

#[derive(Debug, Clone)]
enum SupportedProgram {
    SystemProgram,
    VoteProgram,
    StakeProgram,
    TokenProgram,
    TokenSwapProgramV3,
    TokenLendingProgram,
    SquadsProgramV4,
    JupiterProgramV6,
    ComputeBudgetProgram,
}

impl SupportedProgram {
    pub fn from_program_id(program_id: String) -> Result<Self> {
        match program_id.as_str() {
            "11111111111111111111111111111111" => Ok(SupportedProgram::SystemProgram),
            "Vote111111111111111111111111111111111111111" => Ok(SupportedProgram::VoteProgram),
            "Stake11111111111111111111111111111111111111" => Ok(SupportedProgram::StakeProgram),
            "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA" => Ok(SupportedProgram::TokenProgram),
            "SwapsVeCiPHMUAtzQWZw7RjsKjgCjhwU55QGu4U1Szw" => {
                Ok(SupportedProgram::TokenSwapProgramV3)
            }

            "LendZqTs7gn5CTSJU1jWKhKuVpjJGom45nnwPb2AMTi" => {
                Ok(SupportedProgram::TokenLendingProgram)
            }
            "SQDS4ep65T869zMMBKyuUq6aD6EgTu8psMjkvj52pCf" => Ok(SupportedProgram::SquadsProgramV4),
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4" => Ok(SupportedProgram::JupiterProgramV6),
            "ComputeBudget111111111111111111111111111111" => {
                Ok(SupportedProgram::ComputeBudgetProgram)
            }
            x => Err(SolanaError::UnsupportedProgram(x.to_string())),
        }
    }
}

impl Instruction {
    pub fn parse(&self, program_id: &str, accounts: Vec<String>) -> Result<SolanaDetail> {
        let program = SupportedProgram::from_program_id(program_id.to_string())?;
        match program {
            SupportedProgram::SystemProgram => {
                let instruction =
                    Self::parse_native_program_instruction::<SystemInstruction>(self.data.clone())?;
                resolvers::system::resolve(instruction, accounts)
            }
            SupportedProgram::VoteProgram => {
                let instruction =
                    Self::parse_native_program_instruction::<VoteInstruction>(self.data.clone())?;
                resolvers::vote::resolve(instruction, accounts)
            }
            SupportedProgram::StakeProgram => {
                let instruction =
                    Self::parse_native_program_instruction::<StakeInstruction>(self.data.clone())?;
                resolvers::stake::resolve(instruction, accounts)
            }
            SupportedProgram::TokenProgram => {
                let instruction =
                    crate::solana_lib::spl::token::instruction::TokenInstruction::unpack(
                        self.data.clone().as_slice(),
                    )
                    .map_err(|e| ProgramError(e.to_string()))?;
                resolvers::token::resolve(instruction, accounts)
            }
            SupportedProgram::TokenSwapProgramV3 => {
                let instruction =
                    crate::solana_lib::spl::token_swap::instruction::SwapInstruction::unpack(
                        self.data.clone().as_slice(),
                    )
                    .map_err(|e| ProgramError(e.to_string()))?;
                resolvers::token_swap_v3::resolve(instruction, accounts)
            }
            SupportedProgram::TokenLendingProgram => {
                let instruction =
                    crate::solana_lib::spl::token_lending::instruction::LendingInstruction::unpack(
                        self.data.clone().as_slice(),
                    )
                    .map_err(|e| ProgramError(e.to_string()))?;
                resolvers::token_lending::resolve(instruction, accounts)
            }
            SupportedProgram::SquadsProgramV4 => {
                // todo implement squads program parser
                let instruction =
                    crate::solana_lib::squads_v4::instructions::SquadsInstructions::dispatch(
                        self.data.as_slice(),
                    )
                    .map_err(|e| ProgramError(e.to_string()))?;

                resolvers::squads_v4::resolve(instruction, accounts)
            }
            SupportedProgram::JupiterProgramV6 => {
                if self.data.len() < 8 {
                    return Err(SolanaError::InvalidData(
                        "Jupiter instruction discriminator is truncated".to_string(),
                    ));
                }
                let discriminator = &self.data[..8];
                let is_supported_instruction = matches!(
                    discriminator,
                    [0xc1, 0x20, 0x9b, 0x33, 0x41, 0xd6, 0x9c, 0x81]
                        | [0xe5, 0x17, 0xcb, 0x97, 0x7a, 0xe3, 0xad, 0x2a]
                        | [0xd0, 0x33, 0xef, 0x97, 0x7b, 0x2b, 0xed, 0x5c]
                        | [0xb0, 0xd1, 0x69, 0xa8, 0x9a, 0x7d, 0x45, 0x3e]
                );
                if !is_supported_instruction {
                    return Err(SolanaError::UnsupportedProgram(
                        "unsupported Jupiter v6 instruction".to_string(),
                    ));
                }
                let instruction =
                    crate::solana_lib::jupiter_v6::instructions::JupiterInstructions::dispatch(
                        self.data.as_slice(),
                    )
                    .map_err(|e| ProgramError(e.to_string()))?;
                resolvers::jupiter_v6::resolve(instruction, accounts)
            }
            SupportedProgram::ComputeBudgetProgram => {
                Self::parse_compute_budget_instruction(self.data.as_slice(), accounts)
            }
        }
    }

    fn parse_compute_budget_instruction(
        data: &[u8],
        accounts: Vec<String>,
    ) -> Result<SolanaDetail> {
        if !accounts.is_empty() || data.is_empty() {
            return Err(SolanaError::InvalidData(
                "invalid compute budget instruction".to_string(),
            ));
        }
        let mut detail = ProgramDetailComputeBudget::default();
        // Match the validator's unchecked Borsh decoding: require complete
        let method = match data[0] {
            0 if data.len() >= 9 => {
                detail.compute_unit_limit =
                    u32::from_le_bytes(data[1..5].try_into().map_err(|_| {
                        SolanaError::InvalidData("invalid compute unit limit".to_string())
                    })?)
                    .to_string();
                detail.additional_fee_lamports =
                    u32::from_le_bytes(data[5..9].try_into().map_err(|_| {
                        SolanaError::InvalidData("invalid additional fee".to_string())
                    })?)
                    .to_string();
                "RequestUnitsDeprecated"
            }
            1 if data.len() >= 5 => {
                detail.heap_frame_bytes = Self::read_compute_budget_u32(&data[1..5])?.to_string();
                "RequestHeapFrame"
            }
            2 if data.len() >= 5 => {
                detail.compute_unit_limit = Self::read_compute_budget_u32(&data[1..5])?.to_string();
                "SetComputeUnitLimit"
            }
            3 if data.len() >= 9 => {
                detail.compute_unit_price_micro_lamports =
                    u64::from_le_bytes(data[1..9].try_into().map_err(|_| {
                        SolanaError::InvalidData("invalid compute unit price".to_string())
                    })?)
                    .to_string();
                "SetComputeUnitPrice"
            }
            4 if data.len() >= 5 => {
                detail.loaded_accounts_data_size_limit =
                    Self::read_compute_budget_u32(&data[1..5])?.to_string();
                "SetLoadedAccountsDataSizeLimit"
            }
            _ => {
                return Err(SolanaError::InvalidData(
                    "unsupported compute budget instruction".to_string(),
                ));
            }
        };
        Ok(SolanaDetail {
            common: CommonDetail {
                program: "ComputeBudget".to_string(),
                method: method.to_string(),
            },
            kind: ProgramDetail::ComputeBudget(detail),
        })
    }

    fn read_compute_budget_u32(data: &[u8]) -> Result<u32> {
        Ok(u32::from_le_bytes(data.try_into().map_err(|_| {
            SolanaError::InvalidData("invalid compute budget value".to_string())
        })?))
    }

    fn parse_native_program_instruction<T: for<'de> serde::de::Deserialize<'de>>(
        instruction_data: Vec<u8>,
    ) -> Result<T> {
        // Copied from solana_sdk
        // pub const PACKET_DATA_SIZE: usize = 1280 - 40 - 8;
        crate::solana_lib::solana_program::program_utils::limited_deserialize::<T, 1232>(
            instruction_data.as_slice(),
        )
        .map_err(|e| ProgramError(e.to_string()))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const PROGRAM_IDS: [&str; 9] = [
        "11111111111111111111111111111111",
        "Vote111111111111111111111111111111111111111",
        "Stake11111111111111111111111111111111111111",
        "TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA",
        "SwapsVeCiPHMUAtzQWZw7RjsKjgCjhwU55QGu4U1Szw",
        "LendZqTs7gn5CTSJU1jWKhKuVpjJGom45nnwPb2AMTi",
        "SQDS4ep65T869zMMBKyuUq6aD6EgTu8psMjkvj52pCf",
        "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4",
        "ComputeBudget111111111111111111111111111111",
    ];

    #[test]
    fn recognizes_supported_program_ids_and_rejects_unknown_programs() {
        for program_id in PROGRAM_IDS {
            assert!(SupportedProgram::from_program_id(program_id.to_string()).is_ok());
        }

        let error = SupportedProgram::from_program_id("unknown".to_string()).unwrap_err();
        assert_eq!(error.to_string(), "Program `unknown` is not supported yet");
    }

    #[test]
    fn read_and_parse_reject_malformed_instruction_data() {
        assert!(matches!(
            Instruction::read(&mut vec![]),
            Err(SolanaError::InvalidData(_))
        ));

        for program_id in &PROGRAM_IDS[..6] {
            let instruction = Instruction {
                program_index: 0,
                account_indexes: vec![],
                data: vec![],
            };
            assert!(instruction.parse(program_id, vec![]).is_err());
        }
    }

    #[test]
    fn distinguishes_malformed_and_unknown_jupiter_instructions() {
        let jupiter_program = PROGRAM_IDS[7];
        let instruction = |data| Instruction {
            program_index: 0,
            account_indexes: vec![],
            data,
        };

        assert!(matches!(
            instruction(vec![0x42; 7]).parse(jupiter_program, vec![]),
            Err(SolanaError::InvalidData(_))
        ));
        assert!(matches!(
            instruction(vec![0x42; 8]).parse(jupiter_program, vec![]),
            Err(SolanaError::UnsupportedProgram(_))
        ));
        assert!(matches!(
            instruction(vec![0xc1, 0x20, 0x9b, 0x33, 0x41, 0xd6, 0x9c, 0x81])
                .parse(jupiter_program, vec![]),
            Err(SolanaError::ProgramError(_))
        ));
    }

    #[test]
    fn parses_compute_budget_instructions_with_trailing_bytes() {
        let cases = [
            (vec![0, 64, 13, 3, 0, 7, 0, 0, 0], "RequestUnitsDeprecated"),
            (vec![1, 0, 0, 4, 0], "RequestHeapFrame"),
            (vec![2, 64, 13, 3, 0], "SetComputeUnitLimit"),
            (vec![3, 5, 0, 0, 0, 0, 0, 0, 0], "SetComputeUnitPrice"),
            (vec![4, 0, 0, 16, 0], "SetLoadedAccountsDataSizeLimit"),
        ];
        for (data, method) in cases {
            let parsed = Instruction::parse_compute_budget_instruction(&data, vec![]).unwrap();
            assert_eq!("ComputeBudget", parsed.common.program);
            assert_eq!(method, parsed.common.method);
            let expected = serde_json::to_value(&parsed).unwrap();
            let mut padded = data.clone();
            padded.extend_from_slice(&[0, 0xff, 0x42, 0]);
            let parsed = Instruction::parse_compute_budget_instruction(&padded, vec![]).unwrap();
            assert_eq!(expected, serde_json::to_value(&parsed).unwrap());
            for length in 0..data.len() {
                assert!(
                    Instruction::parse_compute_budget_instruction(&data[..length], vec![]).is_err()
                );
            }
        }

        let parsed =
            Instruction::parse_compute_budget_instruction(&[1, 0, 0, 1, 0, 0, 0, 0, 0], vec![])
                .unwrap();
        let ProgramDetail::ComputeBudget(detail) = parsed.kind else {
            panic!("expected ComputeBudget detail");
        };
        assert_eq!("65536", detail.heap_frame_bytes);
        assert!(Instruction::parse_compute_budget_instruction(&[5, 0, 0, 0, 0], vec![]).is_err());
        assert!(Instruction::parse_compute_budget_instruction(
            &[2, 1, 0, 0, 0],
            vec!["unexpected".to_string()]
        )
        .is_err());
    }
}
