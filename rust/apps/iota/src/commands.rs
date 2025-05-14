use bytes::{Buf, Bytes};
use crate::errors::{IotaError, Result};
use alloc::vec::Vec;
use crate::byte_reader::BytesReader;

#[derive(Debug, Clone)]
pub enum Argument {
    GasCoin,
    Input(u16),
    Result(u16),
    NestedResult(u16, u16),
}

impl Argument {
    pub fn parse(reader: &mut BytesReader) -> Result<Self> {
        match reader.read_u8()? {
            0x00 => Ok(Argument::GasCoin),
            0x01 => Ok(Argument::Input(reader.read_u16_le()?)),
            0x02 => Ok(Argument::Result(reader.read_u16_le()?)),
            0x03 => {
                let a = reader.read_u16_le()?;
                let b = reader.read_u16_le()?;
                Ok(Argument::NestedResult(a, b))
            }
            tag => Err(IotaError::InvalidCommand(tag)),
        }
    }
}

#[derive(Debug)]
pub enum Command {
    TransferObjects(Vec<Argument>, Argument),
    SplitCoins(Argument, Vec<Argument>),
}

impl Command {
    pub fn parse(reader: &mut BytesReader) -> Result<Self> {
        match reader.read_u8()? {
            0x01 => {
                // TransferObjects
                let count = reader.read_u8()? as usize;
                let mut objects = Vec::with_capacity(count);
                
                for _ in 0..count {
                    objects.push(Argument::parse(reader)?);
                }
                
                let recipient = Argument::parse(reader)?;
                Ok(Command::TransferObjects(objects, recipient))
            }
            0x02 => {
                // SplitCoins
                let coin = Argument::parse(reader)?;
                let amount_count = reader.read_u8()? as usize;
                
                let mut amounts = Vec::with_capacity(amount_count);
                for _ in 0..amount_count {
                    amounts.push(Argument::parse(reader)?);
                }
                
                Ok(Command::SplitCoins(coin, amounts))
            }
            tag => Err(IotaError::InvalidCommand(tag))
        }
    }
}

#[derive(Debug)]
pub struct Commands {
    pub commands: Vec<Command>,
}

impl Commands {
    pub fn parse(reader: &mut BytesReader) -> Result<Self> {
        let command_count = reader.read_u8()? as usize;
        let mut commands = Vec::with_capacity(command_count);
        
        for _ in 0..command_count {
            commands.push(Command::parse(reader)?);
        }
        
        Ok(Commands { commands })
    }
}

impl TryFrom<Bytes> for Commands {
    type Error = IotaError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let mut reader = BytesReader::new(&mut bytes);
        Commands::parse(&mut reader)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use bytes::Bytes;

    #[test]
    fn test_command_try_from() {
        let data = "0202000101000001010300000000010100";
        let mut bytes = Bytes::from(hex::decode(data).unwrap());
        let mut reader = BytesReader::new(&mut bytes);
        let commands = Commands::parse(&mut reader).unwrap();
        println!("{:?}", commands);
        assert!(false);
    }
}