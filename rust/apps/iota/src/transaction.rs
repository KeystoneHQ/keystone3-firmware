use crate::{
    errors::{IotaError, Result},
    byte_reader::BytesReader,
    commands::{Command, Commands},
};
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use hex;
use bytes::{Buf, Bytes};
use serde::{Deserialize, Serialize};
use crate::base_type::{ObjectID, ObjectRef, SequenceNumber};
use std::io::{self, Write};

#[derive(Debug)]
pub enum TransactionData {
    V1(TransactionDataV1),
}

pub const IOTA_ADDRESS_LENGTH: usize = ObjectID::LENGTH;

#[derive(Debug)]
pub struct TransactionDataV1 {
    pub kind: TransactionKind,
    pub sender: [u8; 32],
    pub gas_data: GasData,
    pub expiration: TransactionExpiration,
}

#[derive(Debug)]
pub enum TransactionKind {
    ProgrammableTransaction(ProgrammableTransaction),
}

#[derive(Debug)]
pub struct ProgrammableTransaction {
    pub inputs: Vec<TransactionInput>,
    pub commands: Vec<Command>,
}

#[derive(Debug)]
pub enum TransactionInput {
    Pure(Vec<u8>),
}

#[derive(Debug)]
pub struct GasData {
    pub payments: Vec<([u8; 32], u64, [u8; 32])>, // (object_id, version, digest)
    pub owner: [u8; 32],
    pub price: u64,
    pub budget: u64,
}

#[derive(Debug)]
pub enum TransactionExpiration {
    None,
}

impl TransactionDataV1 {
    fn parse(reader: &mut BytesReader) -> Result<String> {
    // fn parse(reader: &mut BytesReader) -> Result<Self> {
        // Parse inputs and commands
        let kind = TransactionKind::parse(reader)?;
        println!("kind: {:?}", kind);
        // Parse sender
        let mut sender = [0u8; 32];
        let sender_bytes = reader.read_bytes(32)?;
        sender.copy_from_slice(&sender_bytes);
        
        // // Parse gas_data
        let gas_data = GasData::parse(reader)?;
        
        // // Parse expiration
        // let expiration = TransactionExpiration::parse(reader)?;
        return Ok("".to_string());
        
        // Ok(TransactionDataV1 {
            //     kind,
            //     sender,
            //     gas_data,
            //     expiration,
            // })
        }
    }
    
    impl TransactionKind {
        fn parse(reader: &mut BytesReader) -> Result<Self> {
            let tx = ProgrammableTransaction::parse(reader)?;
            println!("tx: {:?}", tx);
        Ok(TransactionKind::ProgrammableTransaction(tx))
    }
}
const MAX_INPUT_SIZE: usize = 1024;
impl ProgrammableTransaction {
    fn parse(reader: &mut BytesReader) -> Result<Self> {
        // Parse inputs
        println!("{}:{}, reader.bytes: {:?}", file!(), line!(), hex::encode(reader.bytes.clone()));
        let inp_cnt: usize = reader.read_u8()? as usize;
        println!("inp_cnt: {:?}", inp_cnt);
        // let data = reader.read_bytes(2 + 8 + 2 + 32);
        println!("{}:{}, reader.bytes: {:?}", file!(), line!(), hex::encode(reader.bytes.clone()));
        let mut inputs = Vec::with_capacity(inp_cnt);
        for _ in 0..1 {
            println!("Raw bytes: {:02x?}", &reader.peek_bytes(2)?);
            io::stdout().flush().ok();
            
            let data_len = reader.read_u16()? as usize;
            println!("data_len: {:?}", data_len);
            io::stdout().flush().ok();
            
            if data_len > MAX_INPUT_SIZE {
                return Err(IotaError::InvalidField(format!("Input size too large: {}", data_len)));
            }
            
            let data = reader.read_bytes(data_len)?;
            println!("data: {:?}", data);
            io::stdout().flush().ok();
            
            inputs.push(TransactionInput::Pure(data));
        }
        
        for _ in 0..1 {
            let data_len = reader.read_u16()? as usize;
            if data_len > MAX_INPUT_SIZE {
                return Err(IotaError::InvalidField(format!("Input size too large: {}", data_len)));
            }
            let data = reader.read_bytes(data_len)?;
            
            inputs.push(TransactionInput::Pure(data));
        }

        println!("{}:{}, reader.bytes: {:?}", file!(), line!(), hex::encode(reader.bytes.clone()));
        let commands = Commands::parse(reader)?.commands;
        println!("commands: {:?}", commands);

        Ok(ProgrammableTransaction { inputs, commands })
    }
}

impl GasData {
    fn parse(reader: &mut BytesReader) -> Result<Self> {
        let payment_cnt = reader.read_u8()? as usize;
        let mut payments = Vec::with_capacity(payment_cnt);
        for _ in 0..payment_cnt {
            let mut payment = [0u8; 32];
            let payment_bytes = reader.read_bytes(32)?;
            payment.copy_from_slice(&payment_bytes);
            
            let sequence_number = reader.read_u64_le()?;
            println!("sequence_number: {:?}", sequence_number);

            let mut object_digest = [0u8; 32];
            let object_digest_bytes = reader.read_bytes(32)?;
            object_digest.copy_from_slice(&object_digest_bytes);

            payments.push((payment, sequence_number, object_digest));
        }

        let mut owner = [0u8; 32];
        let owner_bytes = reader.read_bytes(32)?;
        owner.copy_from_slice(&owner_bytes);
        
        let price = reader.read_u64_le()?;
        reader.read_u8()?;
        let budget = reader.read_u64_le()?;
        println!("budget: {:?}", budget);
        println!("owner: {:?}", hex::encode(owner));

        Ok(GasData { payments, owner, price, budget })
    }
}

impl TransactionExpiration {
    fn parse(reader: &mut BytesReader) -> Result<Self> {
        let _exp_tag = reader.read_u32_le()?;
        Ok(TransactionExpiration::None)
    }
}

// pub fn parse_transaction_data(hex_str: &str) -> Result<TransactionData> {
pub fn parse_transaction_data(hex_str: &str) -> Result<String> {
    let raw = hex::decode(hex_str)?;
    let mut buf = Bytes::from(raw);
    let mut reader = BytesReader::new(&mut buf);

    let version = reader.read_u16_le()?;
    // reader.read_bytes(1 + 2 + 8 + 2 + 32);
    TransactionDataV1::parse(&mut reader)?;
    // match version {
    //     0 => {
    //         let v1 = TransactionDataV1::parse(&mut reader)?;
    //         Ok(TransactionData::V1(v1))
    //     }
    //     other => Err(IotaError::InvalidField(format!("unsupported version {}", other))),
    // }
    Ok("".to_string())
}

#[cfg(test)]
mod test {
    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_iota_parse_transaction() {
        let data = "000002000800e40b54020000000020193a4811b7207ac7a861f840552f9c718172400f4c46bdef5935008a7977fb04020200010100000101030000000001010032bc9471570ca24fcd1fe5b201ea6894748aa0ddd44d20c68f1a4f99db513aa201b9ee296780e0b8e23456c605bacfaad200e468985e5e9a898c7b31919225066eef48630d0000000020b1d6709ed59ff892f9a86cd38493b45b7cbb96cf5a459fdc49cbdbc6e79921f832bc9471570ca24fcd1fe5b201ea6894748aa0ddd44d20c68f1a4f99db513aa2e80300000000000000e40b540200000000";
        let tx = parse_transaction_data(data);
        println!("tx: {:?}", tx.unwrap());
        assert!(false);
    }

    #[test]
    fn test_iota_copy_to_slice() {
        let mut buf = Bytes::from(hex::decode("193a4811b7207ac7a861f840552f9c718172400f4c46bdef5935008a7977fb04").unwrap());
        let mut data = vec![0u8; 32];
        buf.copy_to_slice(&mut data);
        assert_eq!(data, vec![1, 2, 3]);
    }
}