use alloc::string::{String, ToString};
use alloc::vec::Vec;

use crate::compact::Compact;
use crate::errors::{Result, SolanaError};
use crate::errors::SolanaError::ProgramError;
use crate::parser::detail::SolanaDetail;
use crate::Read;
use crate::resolvers;
use crate::solana_lib::solana_program::stake::instruction::StakeInstruction;
use crate::solana_lib::solana_program::system_instruction::SystemInstruction;
use crate::solana_lib::solana_program::vote::instruction::VoteInstruction;
use crate::solana_lib::traits::Dispatch;
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
            "JUP6LkbZbjS1jKKwapdHNy74zcZ3tLUZoi5QNyVTaV4" => {
                Ok(SupportedProgram::JupiterProgramV6)
            }
            x => Err(SolanaError::UnsupportedProgram(x.to_string())),
        }
    }
}

impl Instruction {
    pub fn parse(&self, program_id: &String, accounts: Vec<String>) -> Result<SolanaDetail> {
        let program = SupportedProgram::from_program_id(program_id.clone())?;
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
                let instruction =
                    crate::solana_lib::jupiter_v6::instructions::JupiterInstructions::dispatch(
                        self.data.as_slice(),
                    )
                    .map_err(|e| ProgramError(e.to_string()))?;
                resolvers::jupiter_v6::resolve(instruction, accounts)
            }
        }
    }

    fn parse_native_program_instruction<T: for<'de> serde::de::Deserialize<'de>>(
        instruction_data: Vec<u8>,
    ) -> Result<T> {
        // Copied from solana_sdk
        // pub const PACKET_DATA_SIZE: usize = 1280 - 40 - 8;
        crate::solana_lib::solana_program::program_utils::limited_deserialize(
            instruction_data.as_slice(),
            1280 - 40 - 8,
        )
        .map_err(|e| ProgramError(e.to_string()))
    }
}
