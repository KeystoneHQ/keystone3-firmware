use crate::solana_lib::solana_program::errors::ProgramError;

pub trait Dispatch {
    fn dispatch(data: &[u8]) -> Result<Self, ProgramError>
    where
        Self: Sized;
}
