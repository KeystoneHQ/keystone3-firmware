use super::errors::ProgramError;

pub enum ComputeBudgetInstruction {
    Unused, // deprecated variant, reserved value.
    /// Request a specific transaction-wide program heap region size in bytes.
    /// The value requested must be a multiple of 1024. This new heap region
    /// size applies to each program executed in the transaction, including all
    /// calls to CPIs.
    RequestHeapFrame(u32),
    /// Set a specific compute unit limit that the transaction is allowed to consume.
    SetComputeUnitLimit(u32),
    /// Set a compute unit price in "micro-lamports" to pay a higher transaction
    /// fee for higher transaction prioritization.
    SetComputeUnitPrice(u64),
    /// Set a specific transaction-wide account data size limit, in bytes, is allowed to load.
    SetLoadedAccountsDataSizeLimit(u32),
}

impl ComputeBudgetInstruction {
    pub fn unpack(input: &[u8]) -> Result<Self, ProgramError> {
        let tag = input[0];
        match tag {
            0 => Ok(ComputeBudgetInstruction::Unused),
            1 => Ok(ComputeBudgetInstruction::RequestHeapFrame(
                u32::from_le_bytes(input[1..5].try_into().unwrap()),
            )),
            2 => Ok(ComputeBudgetInstruction::SetComputeUnitLimit(
                u32::from_le_bytes(input[1..5].try_into().unwrap()),
            )),
            3 => Ok(ComputeBudgetInstruction::SetComputeUnitPrice(
                u64::from_le_bytes(input[1..9].try_into().unwrap()),
            )),
            4 => Ok(ComputeBudgetInstruction::SetLoadedAccountsDataSizeLimit(
                u32::from_le_bytes(input[1..5].try_into().unwrap()),
            )),
            _ => Err(ProgramError::InvalidAccountData),
        }
    }
}
