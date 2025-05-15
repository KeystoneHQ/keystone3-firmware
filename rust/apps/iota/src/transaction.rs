use crate::base_type::{ObjectID, ObjectRef, SequenceNumber, ObjectDigest, Digest};
use crate::account::AccountAddress;
use crate::{
    byte_reader::BytesReader,
    commands::{Command, Commands},
    errors::{IotaError, Result},
};
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use bytes::{Buf, Bytes};
use hex;

#[derive(Debug)]
pub enum TransactionData {
    V1(TransactionDataV1),
}

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
    Object(ObjectArg),
}

#[derive(Debug)]
pub enum ObjectArg {
    // A Move object, either immutable, or owned mutable.
    ImmOrOwnedObject(ObjectRef),
    // A Move object that's shared.
    // SharedObject::mutable controls whether caller asks for a mutable reference to shared
    // object.
    SharedObject {
        id: ObjectID,
        initial_shared_version: SequenceNumber,
        mutable: bool,
    },
    // A Move object that can be received in this transaction.
    Receiving(ObjectRef),
}

impl TransactionInput {
    pub fn parse(reader: &mut BytesReader) -> Result<Self> {
        let input_type = reader.read_u8()?;
        match input_type {
            0 => {
                // Pure type
                let data_len = reader.read_u8()? as usize;
                let data = reader.read_bytes(data_len)?;
                Ok(TransactionInput::Pure(data))
            }
            1 => {
                // Object type
                let object_type = reader.read_u8()?;
                match object_type {
                    0 => {
                        // ImmOrOwnedObject
                        let mut addr_bytes = [0u8; 32];
                        let id_bytes = reader.read_bytes(32)?;
                        addr_bytes.copy_from_slice(&id_bytes);
                        let object_id = ObjectID::new(addr_bytes);

                        let sequence = SequenceNumber(reader.read_u64_le()?);

                        let len = reader.read_u8()? as usize;
                        let mut digest_bytes = [0u8; 32];
                        let digest_data = reader.read_bytes(32)?;
                        digest_bytes.copy_from_slice(&digest_data);
                        let digest = ObjectDigest(Digest(digest_bytes));

                        Ok(TransactionInput::Object(ObjectArg::ImmOrOwnedObject((
                            object_id,
                            sequence,
                            digest,
                        ))))
                    },
                    1 => {
                        // SharedObject
                        let mut id = [0u8; 32];
                        let id_bytes = reader.read_bytes(32)?;
                        id.copy_from_slice(&id_bytes);
                        let object_id = ObjectID::new(id);

                        let initial_shared_version = SequenceNumber(reader.read_u64_le()?);
                        let mutable = reader.read_u8()? != 0;

                        Ok(TransactionInput::Object(ObjectArg::SharedObject {
                            id: object_id,
                            initial_shared_version,
                            mutable,
                        }))
                    },
                    2 => {
                        // Receiving
                        let mut object_id = [0u8; 32];
                        let object_id_bytes = reader.read_bytes(32)?;
                        object_id.copy_from_slice(&object_id_bytes);
                        let object_id = ObjectID::new(object_id);

                        let sequence = SequenceNumber(reader.read_u64_le()?);

                        let mut digest_bytes = [0u8; 32];
                        let digest_data = reader.read_bytes(32)?;
                        digest_bytes.copy_from_slice(&digest_data);
                        let digest = ObjectDigest(Digest(digest_bytes));

                        Ok(TransactionInput::Object(ObjectArg::Receiving((
                            object_id,
                            sequence,
                            digest,
                        ))))
                    },
                    other => Err(IotaError::InvalidField(format!(
                        "Invalid Object type: {}",
                        other
                    ))),
                }
            },
            other => Err(IotaError::InvalidField(format!(
                "Invalid TransactionInput type: {}",
                other
            ))),
        }
    }
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
    fn parse(reader: &mut BytesReader) -> Result<Self> {
        let kind = TransactionKind::parse(reader)?;
        let mut sender = [0u8; 32];
        let sender_bytes = reader.read_bytes(32)?;
        sender.copy_from_slice(&sender_bytes);
        let gas_data = GasData::parse(reader)?;
        let expiration = TransactionExpiration::parse(reader)?;

        Ok(TransactionDataV1 {
            kind,
            sender,
            gas_data,
            expiration,
        })
    }
}

impl TransactionKind {
    fn parse(reader: &mut BytesReader) -> Result<Self> {
        Ok(TransactionKind::ProgrammableTransaction(ProgrammableTransaction::parse(reader)?))
    }
}

impl ProgrammableTransaction {
    fn parse(reader: &mut BytesReader) -> Result<Self> {
        let inp_cnt: usize = reader.read_u8()? as usize;
        let mut inputs = Vec::with_capacity(inp_cnt);
        for _ in 0..inp_cnt {
            inputs.push(TransactionInput::parse(reader)?);
        }

        Ok(ProgrammableTransaction {
            inputs,
            commands: Commands::parse(reader)?.commands,
        })
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

            let object_len = reader.read_u8()? as usize;
            let mut object_digest = [0u8; 32];
            let object_digest_bytes = reader.read_bytes(object_len)?;
            object_digest.copy_from_slice(&object_digest_bytes);

            payments.push((payment, sequence_number, object_digest));
        }

        let mut owner = [0u8; 32];
        let owner_bytes = reader.read_bytes(32)?;
        owner.copy_from_slice(&owner_bytes);

        let price = reader.read_u64_le()?;
        let budget = reader.read_u64_le()?;

        Ok(GasData {
            payments,
            owner,
            price,
            budget,
        })
    }
}

impl TransactionExpiration {
    fn parse(reader: &mut BytesReader) -> Result<Self> {
        let _exp_tag = reader.read_u8()?;
        Ok(TransactionExpiration::None)
    }
}

pub fn parse_transaction_data(hex_str: &str) -> Result<TransactionData> {
    let raw = hex::decode(hex_str)?;
    let mut buf = Bytes::from(raw);
    let mut reader = BytesReader::new(&mut buf);

    let version = reader.read_u16_le()?;
    match version {
        0 => {
            let v1 = TransactionDataV1::parse(&mut reader)?;
            Ok(TransactionData::V1(v1))
        }
        other => Err(IotaError::InvalidField(format!(
            "unsupported version {}",
            other
        ))),
    }
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
    fn test_iota_parse_stake_transaction() {
        let data = "0000040101000000000000000000000000000000000000000000000000000000000000000501000000000000000101008ad401f77067bcee6173e32269e57c11b9daa1c70456a6ce88646f42f585e4240f7aa71000000000200ae4b6b24c5924970d52b8ea3719c4cfbcaa96bd2ec9cedc8d526c6889c6d37700090100ca9a3b000000000020a276b4c076fff55588255630e9ee35cf0d07e8d80c78991cfd58b43b687b4206020500010101000000000000000000000000000000000000000000000000000000000000000000030b696f74615f73797374656d1a726571756573745f6164645f7374616b655f6d756c5f636f696e0004010000020000010200010300816b9e2fc2460bc7acf1fca2549cd1fa25197ab365d6294cc407ea97481c8de001be3d5998a9c68422681fa77b834d0ec35618b5a988c143279ddd4e1197dff7af0f7aa71000000000200369186897ac8e03297c1b5e75679193f516259756ce6c9afe39d8ba7a362cc3816b9e2fc2460bc7acf1fca2549cd1fa25197ab365d6294cc407ea97481c8de0e803000000000000404b4c000000000000";
        let tx = parse_transaction_data(data);
        println!("tx: {:?}", tx.unwrap());
        assert!(false);
    }
}
