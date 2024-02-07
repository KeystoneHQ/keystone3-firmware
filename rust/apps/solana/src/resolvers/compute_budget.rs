use crate::errors::Result;
use crate::parser::detail::{CommonDetail, ProgramDetail, ProgramDetailCumputeBudget};
use crate::{
    parser::detail::SolanaDetail,
    solana_lib::solana_program::compute_budget_instruction::ComputeBudgetInstruction,
};
use alloc::string::ToString;
use alloc::{string::String, vec::Vec};

static PROGRAM_NAME: &str = "ComputeBudget";

pub fn resolve(
    instruction: ComputeBudgetInstruction,
    _accounts: Vec<String>,
) -> Result<SolanaDetail> {
    match instruction {
        ComputeBudgetInstruction::Unused => Ok(SolanaDetail {
            common: CommonDetail {
                method: "Unused".to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::ComputeBudgetUnused,
        }),
        ComputeBudgetInstruction::RequestHeapFrame(n) => Ok(SolanaDetail {
            common: CommonDetail {
                method: "RequestHeapFrame".to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::ComputeBudgetRequestHeapFrame(ProgramDetailCumputeBudget {
                value: n.to_string(),
            }),
        }),
        ComputeBudgetInstruction::SetComputeUnitPrice(n) => Ok(SolanaDetail {
            common: CommonDetail {
                method: "SetComputeUnitPrice".to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::ComputeBudgetSetComputeUnitPrice(ProgramDetailCumputeBudget {
                value: n.to_string(),
            }),
        }),
        ComputeBudgetInstruction::SetLoadedAccountsDataSizeLimit(n) => Ok(SolanaDetail {
            common: CommonDetail {
                method: "SetLoadedAccountsDataSizeLimit".to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::ComputeBudgetSetLoadedAccountsDataSizeLimit(
                ProgramDetailCumputeBudget {
                    value: n.to_string(),
                },
            ),
        }),
        ComputeBudgetInstruction::SetComputeUnitLimit(n) => Ok(SolanaDetail {
            common: CommonDetail {
                method: "SetComputeUnitLimit".to_string(),
                program: PROGRAM_NAME.to_string(),
            },
            kind: ProgramDetail::ComputeBudgetSetComputeUnitLimit(ProgramDetailCumputeBudget {
                value: n.to_string(),
            }),
        }),
    }
}
