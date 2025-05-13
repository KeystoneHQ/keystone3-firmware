use bytes::{Buf, Bytes};
use core::convert::TryFrom;
use crate::errors::{IotaError, Result};
use alloc::vec::Vec;

#[derive(Debug)]
pub enum Argument {
    /// The gas coin. The gas coin can only be used by-ref, except for with
    /// `TransferObjects`, which can use it by-value.
    GasCoin,
    /// One of the input objects or primitive values (from
    /// `ProgrammableTransaction` inputs)
    Input(u16),
    /// The result of another command (from `ProgrammableTransaction` commands)
    Result(u16),
    /// Like a `Result` but it accesses a nested result. Currently, the only
    /// usage of this is to access a value from a Move call with multiple
    /// return values.
    NestedResult(u16, u16),
}

impl TryFrom<&mut Bytes> for Argument {
    type Error = IotaError;

    fn try_from(bytes: &mut Bytes) -> Result<Self> {
        if bytes.remaining() < 1 {
            return Err(IotaError::UnexpectedEof);
        }

        match bytes.get_u8() {
            0x00 => {
                bytes.advance(1);
                Ok(Argument::GasCoin)
            }
            0x01 => {
                if bytes.remaining() < 2 {
                    return Err(IotaError::UnexpectedEof);
                }
                bytes.advance(2);
                Ok(Argument::Input(bytes.get_u16()))
            }
            0x02 => {
                if bytes.remaining() < 2 {
                    return Err(IotaError::UnexpectedEof);
                }
                bytes.advance(2);
                Ok(Argument::Result(bytes.get_u16()))
            }
            0x03 => {
                if bytes.remaining() < 4 {
                    return Err(IotaError::UnexpectedEof);
                }
                bytes.advance(4);
                Ok(Argument::NestedResult(bytes.get_u16(), bytes.get_u16()))
            }
            tag => Err(IotaError::InvalidCommand(tag)),
        }
    }
}

#[derive(Debug)]
pub enum Command {
    // MoveCall(Box<ProgrammableMoveCall>),
    TransferObjects(Vec<Argument>, Argument),
    SplitCoins(Argument, Vec<Argument>),
    // MergeCoins(Argument, Vec<Argument>),
    // Publish(Vec<Vec<u8>>, Vec<ObjectID>),
    // MakeMoveVec(Option<TypeTag>, Vec<Argument>),
    // Upgrade(Vec<Vec<u8>>, Vec<ObjectID>, ObjectID, Argument),
}

impl TryFrom<&mut Bytes> for Command {
    type Error = IotaError;

    fn try_from(bytes: &mut Bytes) -> Result<Self> {
        if bytes.remaining() < 1 {
            return Err(IotaError::UnexpectedEof);
        }
        println!("command: {:?}", bytes.clone());

        let command = match bytes.get_u8() {
            0x01 => {
                let amount_count = bytes.get_u8() as usize;
                let mut amounts = Vec::with_capacity(amount_count);
                for _ in 0..amount_count {
                    amounts.push(Argument::try_from(bytes.clone())?);
                }
                let argument = Argument::try_from(bytes)?;
                println!("argument: {:?}", argument);
                Command::TransferObjects(amounts, argument)
            }
            0x02 => {
                let argument = Argument::try_from(bytes)?;
                println!("argument: {:?}", argument);
                let amount_count = bytes.get_u8() as usize;
                let mut amounts = Vec::with_capacity(amount_count);
                for _ in 0..amount_count {
                    amounts.push(Argument::try_from(bytes)?);
                }
                Command::SplitCoins(argument, amounts)
            }
            _ => return Err(IotaError::InvalidCommand(bytes.get_u8()))
        }
        Ok(command)
    }
}

#[derive(Debug)]
pub struct Commands {
    commands: Vec<Command>,
}

impl TryFrom<Bytes> for Commands {
    type Error = IotaError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        if bytes.remaining() < 1 {
            return Err(IotaError::UnexpectedEof);
        }

        let mut commands = Vec::new();
        let command_count = bytes.get_u8();
        println!("command_count: {:?}", command_count);
        for _ in 0..command_count {
            commands.push(Command::try_from(&mut bytes)?);
        }
        Ok(Commands { commands })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use bytes::Bytes;

    #[test]
    fn test_command_try_from() {
        let data = "0202000101000001010300000000010100";
        let command_bytes = Bytes::from(hex::decode(data).unwrap());
        let commands = Commands::try_from(command_bytes).unwrap();
        println!("{:?}", commands);
        assert!(false);
    }
}
