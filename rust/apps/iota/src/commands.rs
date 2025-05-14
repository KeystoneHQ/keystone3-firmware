use bytes::{Buf, Bytes};
use core::convert::TryFrom;
use crate::errors::{IotaError, Result};
use alloc::vec::Vec;

#[derive(Debug, Clone)]
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

impl Argument {
    fn try_from(bytes: &mut Bytes) -> Result<(Self, usize)> {
        if bytes.remaining() < 1 {
            return Err(IotaError::UnexpectedEof);
        }

        match bytes.get_u8() {
            0x00 => {
                Ok((Argument::GasCoin, 1))
            }
            0x01 => {
                if bytes.remaining() < 2 {
                    return Err(IotaError::UnexpectedEof);
                }
                Ok((Argument::Input(bytes.get_u16()), 2))
            }
            0x02 => {
                if bytes.remaining() < 2 {
                    return Err(IotaError::UnexpectedEof);
                }
                Ok((Argument::Result(bytes.get_u16()), 2))
            }
            0x03 => {
                if bytes.remaining() < 4 {
                    return Err(IotaError::UnexpectedEof);
                }
                Ok((Argument::NestedResult(bytes.get_u16(), bytes.get_u16()), 4))
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

        match bytes.get_u8() {
            0x01 => {
                if bytes.remaining() < 1 {
                    return Err(IotaError::UnexpectedEof);
                }
                let count = bytes.get_u8() as usize;
                let mut v = Vec::with_capacity(count);
                
                for _ in 0..count {
                    let (arg, size) = Argument::try_from(bytes)?;
                    v.push(arg);
                }
                
                let (recipient, _) = Argument::try_from(bytes)?;
                Ok(Command::TransferObjects(v, recipient))
            }
            0x02 => {
                let (coin, size) = Argument::try_from(bytes)?;
                bytes.advance(size);
                
                if bytes.remaining() < 1 {
                    return Err(IotaError::UnexpectedEof);
                }
                let amount_count = bytes.get_u8() as usize;
                
                let mut amounts = Vec::with_capacity(amount_count);
                for _ in 0..amount_count {
                    let (amount, size) = Argument::try_from(bytes)?;
                    amounts.push(amount);
                    bytes.advance(size);
                }
                
                Ok(Command::SplitCoins(coin, amounts))
            }
            tag => Err(IotaError::InvalidCommand(tag))
        }
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
