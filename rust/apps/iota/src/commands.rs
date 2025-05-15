use crate::base_type::ObjectID;
use crate::byte_reader::BytesReader;
use crate::errors::{IotaError, Result};
use alloc::vec::Vec;
use bytes::{Buf, Bytes};
use alloc::boxed::Box;
use alloc::string::String;
use alloc::string::ToString;

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
pub struct Identifier(pub String);

impl Identifier {
    fn parse(reader: &mut BytesReader) -> Result<Self> {
        let len = reader.read_u8()? as usize;
        let bytes = reader.read_bytes(len)?;
        let s = String::from_utf8(bytes)
            .map_err(|_| IotaError::InvalidField("Invalid UTF-8 in identifier".to_string()))?;
        Ok(Identifier(s))
    }
}

#[derive(Debug)]
pub struct TypeTag(pub String);

impl TypeTag {
    fn parse(reader: &mut BytesReader) -> Result<Self> {
        let len = reader.read_u8()? as usize;
        let bytes = reader.read_bytes(len)?;
        let s = String::from_utf8(bytes)
            .map_err(|_| IotaError::InvalidField("Invalid UTF-8 in type tag".to_string()))?;
        Ok(TypeTag(s))
    }
}

#[derive(Debug)]
pub struct ProgrammableMoveCall {
    /// The package containing the module and function.
    pub package: ObjectID,
    /// The specific module in the package containing the function.
    pub module: Identifier,
    /// The function to be called.
    pub function: Identifier,
    /// The type arguments to the function.
    pub type_arguments: Vec<TypeTag>,
    /// The arguments to the function.
    pub arguments: Vec<Argument>,
}

impl ProgrammableMoveCall {
    fn parse(reader: &mut BytesReader) -> Result<Self> {
        // Parse package ID (32 bytes)
        let mut package_bytes = [0u8; 32];
        let pkg_bytes = reader.read_bytes(32)?;
        package_bytes.copy_from_slice(&pkg_bytes);
        let package = ObjectID::new(package_bytes);

        // Parse module and function identifiers
        let module = Identifier::parse(reader)?;
        let function = Identifier::parse(reader)?;

        // Parse type arguments
        let type_arg_count = reader.read_u8()? as usize;
        let mut type_arguments = Vec::with_capacity(type_arg_count);
        for _ in 0..type_arg_count {
            type_arguments.push(TypeTag::parse(reader)?);
        }

        // Parse arguments
        let arg_count = reader.read_u8()? as usize;
        let mut arguments = Vec::with_capacity(arg_count);
        for _ in 0..arg_count {
            arguments.push(Argument::parse(reader)?);
        }

        Ok(ProgrammableMoveCall {
            package,
            module,
            function,
            type_arguments,
            arguments,
        })
    }
}

#[derive(Debug)]
pub enum Command {
    MoveCall(Box<ProgrammableMoveCall>),
    TransferObjects(Vec<Argument>, Argument),
    SplitCoins(Argument, Vec<Argument>),
    MergeCoins(Argument, Vec<Argument>),
    Publish(Vec<Vec<u8>>, Vec<ObjectID>),
    MakeMoveVec(Option<TypeTag>, Vec<Argument>),
}

impl Command {
    pub fn parse(reader: &mut BytesReader) -> Result<Self> {
        match reader.read_u8()? {
            0x00 => {
                // MoveCall
                let call = ProgrammableMoveCall::parse(reader)?;
                Ok(Command::MoveCall(Box::new(call)))
            }
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
            0x03 => {
                // MergeCoins
                let target = Argument::parse(reader)?;
                let count = reader.read_u8()? as usize;
                let mut coins = Vec::with_capacity(count);
                for _ in 0..count {
                    coins.push(Argument::parse(reader)?);
                }
                Ok(Command::MergeCoins(target, coins))
            }
            0x05 => {
                // MakeMoveVec
                let has_type = reader.read_u8()? != 0;
                let type_tag = if has_type {
                    Some(TypeTag::parse(reader)?)
                } else {
                    None
                };
                
                let arg_count = reader.read_u8()? as usize;
                let mut arguments = Vec::with_capacity(arg_count);
                for _ in 0..arg_count {
                    arguments.push(Argument::parse(reader)?);
                }
                Ok(Command::MakeMoveVec(type_tag, arguments))
            }
            tag => Err(IotaError::InvalidCommand(tag)),
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
    use alloc::format;

    #[test]
    fn test_command_parse_transaction() {
        let data = "0202000101000001010300000000010100";
        let mut bytes = Bytes::from(hex::decode(data).unwrap());
        let mut reader = BytesReader::new(&mut bytes);
        let commands = Commands::parse(&mut reader).unwrap();
        println!("{:?}", commands);
        assert!(false);
    }

    #[test]
    fn test_command_parse_stake() {
        let data = "020500010101000000000000000000000000000000000000000000000000000000000000000000030b696f74615f73797374656d1a726571756573745f6164645f7374616b655f6d756c5f636f696e0004010000020000010200010300";
        let mut bytes = Bytes::from(hex::decode(data).unwrap());
        let mut reader = BytesReader::new(&mut bytes);
        let commands = Commands::parse(&mut reader).unwrap();
        println!("{:?}", commands);
        assert!(false);
    }
}
