use alloc::string::{String, ToString};
use alloc::vec::Vec;

use crate::errors::Result;
use crate::parser::detail::{CommonDetail, ProgramDetail, SolanaDetail};
use crate::solana_lib::squads_v4::instructions::{
    MultisigCreateArgs, MultisigCreateArgsV2, ProposalCreateArgs, ProposalVoteArgs,
    SquadsInstructions, VaultTransactionCreateArgs,
};

static PROGRAM_NAME: &str = "SquadsV4";
pub fn resolve(instruction: SquadsInstructions, accounts: Vec<String>) -> Result<SolanaDetail> {
    match instruction {
        SquadsInstructions::MultisigCreate(arg) => resolve_multisig_create(arg),
        SquadsInstructions::MultisigCreateV2(arg) => resolve_multisig_create_v2(arg, accounts),
        SquadsInstructions::ProposalCreate(arg) => resolve_proposal_create(arg, accounts),
        SquadsInstructions::ProposalActivate => resolve_proposal_activate(accounts),
        SquadsInstructions::ProposalCancel(arg) => resolve_proposal_cancel(arg, accounts),
        SquadsInstructions::ProposalReject(arg) => resolve_proposal_reject(arg, accounts),
        SquadsInstructions::ProposalApprove(arg) => resolve_proposal_approve(arg, accounts),
        SquadsInstructions::VaultTransactionCreate(arg) => {
            resolve_vault_transaction_create(arg, accounts)
        }
        SquadsInstructions::VaultTransactionExecute => resolve_vault_transaction_execute(accounts),
    }
}

fn resolve_multisig_create(args: MultisigCreateArgs) -> Result<SolanaDetail> {
    let common_detail = CommonDetail {
        program: PROGRAM_NAME.to_string(),
        method: "MultisigCreate".to_string(),
    };
    let detail = SolanaDetail {
        common: common_detail,
        kind: ProgramDetail::SquadsV4MultisigCreate(args),
    };
    Ok(detail)
}

fn resolve_multisig_create_v2(
    args: MultisigCreateArgsV2,
    accounts: Vec<String>,
) -> Result<SolanaDetail> {
    let common_detail = CommonDetail {
        program: PROGRAM_NAME.to_string(),
        method: "MultisigCreateV2".to_string(),
    };
    let detail = SolanaDetail {
        common: common_detail,
        kind: ProgramDetail::SquadsV4MultisigCreateV2(args),
    };
    Ok(detail)
}

fn resolve_proposal_create(
    args: ProposalCreateArgs,
    accounts: Vec<String>,
) -> Result<SolanaDetail> {
    let common_detail = CommonDetail {
        program: PROGRAM_NAME.to_string(),
        method: "ProposalCreate".to_string(),
    };
    let detail = SolanaDetail {
        common: common_detail,
        kind: ProgramDetail::SquadsV4ProposalCreate(args),
    };
    Ok(detail)
}

fn resolve_proposal_activate(accounts: Vec<String>) -> Result<SolanaDetail> {
    let common_detail = CommonDetail {
        program: PROGRAM_NAME.to_string(),
        method: "ProposalActivate".to_string(),
    };
    let detail = SolanaDetail {
        common: common_detail,
        kind: ProgramDetail::SquadsV4ProposalActivate,
    };
    Ok(detail)
}

fn resolve_proposal_cancel(args: ProposalVoteArgs, accounts: Vec<String>) -> Result<SolanaDetail> {
    let common_detail = CommonDetail {
        program: PROGRAM_NAME.to_string(),
        method: "ProposalCancel".to_string(),
    };
    let detail = SolanaDetail {
        common: common_detail,
        kind: ProgramDetail::SquadsV4ProposalCancel(args),
    };
    Ok(detail)
}

fn resolve_proposal_reject(args: ProposalVoteArgs, accounts: Vec<String>) -> Result<SolanaDetail> {
    let common_detail = CommonDetail {
        program: PROGRAM_NAME.to_string(),
        method: "ProposalReject".to_string(),
    };
    let detail = SolanaDetail {
        common: common_detail,
        kind: ProgramDetail::SquadsV4ProposalReject(args),
    };
    Ok(detail)
}

fn resolve_proposal_approve(args: ProposalVoteArgs, accounts: Vec<String>) -> Result<SolanaDetail> {
    let common_detail = CommonDetail {
        program: PROGRAM_NAME.to_string(),
        method: "ProposalApprove".to_string(),
    };
    let detail = SolanaDetail {
        common: common_detail,
        kind: ProgramDetail::SquadsV4ProposalApprove(args),
    };
    Ok(detail)
}

fn resolve_vault_transaction_create(
    args: VaultTransactionCreateArgs,
    accounts: Vec<String>,
) -> Result<SolanaDetail> {
    let common_detail = CommonDetail {
        program: PROGRAM_NAME.to_string(),
        method: "VaultTransactionCreate".to_string(),
    };
    let detail = SolanaDetail {
        common: common_detail,
        kind: ProgramDetail::SquadsV4VaultTransactionCreate(args),
    };
    Ok(detail)
}

fn resolve_vault_transaction_execute(accounts: Vec<String>) -> Result<SolanaDetail> {
    let common_detail = CommonDetail {
        program: PROGRAM_NAME.to_string(),
        method: "VaultTransactionExecute".to_string(),
    };
    let detail = SolanaDetail {
        common: common_detail,
        kind: ProgramDetail::SquadsV4VaultTransactionExecute,
    };
    Ok(detail)
}
