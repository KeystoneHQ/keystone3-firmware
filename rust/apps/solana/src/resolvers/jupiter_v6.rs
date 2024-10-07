use alloc::string::{String, ToString};
use alloc::vec::Vec;

use crate::errors::Result;
use crate::parser::detail::{
    CommonDetail, JupiterV6ExactOutRouteDetail, JupiterV6RouteDetail,
    JupiterV6SharedAccountsExactOutRouteDetail, JupiterV6SharedAccountsRouteDetail, ProgramDetail,
    SolanaDetail,
};
use crate::solana_lib::jupiter_v6::instructions::{
    ExactOutRouteArgs, JupiterInstructions, RouteArgs, SharedAccountsExactOutRouteArgs,
    SharedAccountsRouteArgs,
};

static PROGRAM_NAME: &str = "JupiterV6";
pub fn resolve(instruction: JupiterInstructions, accounts: Vec<String>) -> Result<SolanaDetail> {
    match instruction {
        JupiterInstructions::SharedAccountsRoute(arg) => {
            resolve_shared_accounts_route(accounts, arg)
        }
        JupiterInstructions::SharedAccountsExactOutRoute(arg) => {
            resolve_shared_accounts_exact_out_route(accounts, arg)
        }
        JupiterInstructions::Route(arg) => resolve_route(accounts, arg),
        JupiterInstructions::ExactOutRoute(arg) => resolve_exact_out_route(accounts, arg),
    }
}

fn resolve_shared_accounts_route(
    accounts: Vec<String>,
    args: SharedAccountsRouteArgs,
) -> Result<SolanaDetail> {
    let common_detail = CommonDetail {
        program: PROGRAM_NAME.to_string(),
        method: "SharedAccountsRoute".to_string(),
    };
    let detail = SolanaDetail {
        common: common_detail,
        kind: ProgramDetail::JupiterV6SharedAccountsRoute(JupiterV6SharedAccountsRouteDetail {
            accounts,
            args,
        }),
    };
    Ok(detail)
}

fn resolve_shared_accounts_exact_out_route(
    accounts: Vec<String>,
    args: SharedAccountsExactOutRouteArgs,
) -> Result<SolanaDetail> {
    let common_detail = CommonDetail {
        program: PROGRAM_NAME.to_string(),
        method: "SharedAccountsExactOutRoute".to_string(),
    };
    let detail = SolanaDetail {
        common: common_detail,
        kind: ProgramDetail::JupiterV6SharedAccountsExactOutRoute(
            JupiterV6SharedAccountsExactOutRouteDetail { accounts, args },
        ),
    };
    Ok(detail)
}

fn resolve_route(accounts: Vec<String>, args: RouteArgs) -> Result<SolanaDetail> {
    let common_detail = CommonDetail {
        program: PROGRAM_NAME.to_string(),
        method: "Route".to_string(),
    };
    let detail = SolanaDetail {
        common: common_detail,
        kind: ProgramDetail::JupiterV6Route(JupiterV6RouteDetail { accounts, args }),
    };
    Ok(detail)
}

fn resolve_exact_out_route(accounts: Vec<String>, args: ExactOutRouteArgs) -> Result<SolanaDetail> {
    let common_detail = CommonDetail {
        program: PROGRAM_NAME.to_string(),
        method: "ExactOutRoute".to_string(),
    };
    let detail = SolanaDetail {
        common: common_detail,
        kind: ProgramDetail::JupiterV6ExactOutRoute(JupiterV6ExactOutRouteDetail {
            accounts,
            args,
        }),
    };
    Ok(detail)
}
