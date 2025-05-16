use crate::account::AccountAddress;
use crate::base_type::{Digest, ObjectDigest, ObjectID, ObjectRef, SequenceNumber};
use crate::{
    byte_reader::BytesReader,
    commands::{Command, Commands},
    errors::{IotaError, Result},
};
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use blake2::{
    digest::{Update, VariableOutput},
    Blake2bVar,
};
use bytes::{Buf, Bytes};
use cryptoxide::hashing::keccak256;
use hex;
use serde::Serialize;

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

impl TransactionDataV1 {
    pub fn get_sender(&self) -> &[u8; 32] {
        &self.sender
    }

    pub fn get_gas_data(&self) -> &GasData {
        &self.gas_data
    }

    pub fn get_expiration(&self) -> &TransactionExpiration {
        &self.expiration
    }

    pub fn get_kind(&self) -> &TransactionKind {
        &self.kind
    }
}

#[derive(Debug)]
pub enum TransactionKind {
    ProgrammableTransaction(ProgrammableTransaction),
}

impl TransactionKind {
    pub fn get_inputs_string(&self) -> Vec<String> {
        match self {
            TransactionKind::ProgrammableTransaction(tx) => {
                tx.inputs.iter().map(|v| v.serialize().unwrap()).collect()
            }
        }
    }
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

impl TransactionInput {
    pub fn serialize(&self) -> Result<String> {
        match self {
            TransactionInput::Pure(data) => Ok(hex::encode(data)),
            TransactionInput::Object(arg) => arg.serialize(),
        }
    }
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

impl ObjectArg {
    pub fn serialize(&self) -> Result<String> {
        Ok("this is a test".to_string())
        // match self {
        //     ObjectArg::ImmOrOwnedObject(ref obj) => obj.serialize(),
        //     ObjectArg::SharedObject { id, initial_shared_version, mutable } => {
        //         Ok(format!("SharedObject{{id: {}, initial_shared_version: {}, mutable: {}}}", hex::encode(id), initial_shared_version, mutable))
        //     }
        //     ObjectArg::Receiving(ref obj) => obj.serialize(),
        // }
    }
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
                            object_id, sequence, digest,
                        ))))
                    }
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
                    }
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
                            object_id, sequence, digest,
                        ))))
                    }
                    other => Err(IotaError::InvalidField(format!(
                        "Invalid Object type: {}",
                        other
                    ))),
                }
            }
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

impl GasData {
    pub fn get_payments(&self) -> &Vec<([u8; 32], u64, [u8; 32])> {
        &self.payments
    }

    pub fn get_owner(&self) -> &[u8; 32] {
        &self.owner
    }

    pub fn get_price(&self) -> u64 {
        self.price
    }

    pub fn get_budget(&self) -> u64 {
        self.budget
    }
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
        Ok(TransactionKind::ProgrammableTransaction(
            ProgrammableTransaction::parse(reader)?,
        ))
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

pub fn parse_intent(intent: &[u8]) -> Result<TransactionData> {
    let mut buf = Bytes::copy_from_slice(intent);
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

// pub extern "C" fn sui_sign_hash(ptr: PtrUR, seed: PtrBytes, seed_len: u32) {

// }

// fn iota_sign_transaction(data: &[u8]) -> Result<String> {
//     let tx_hash = keccak256(data);
//     let tx_hash_str = hex::encode(tx_hash);
//     Ok(tx_hash_str)
// }

pub fn sign_intent(seed: &[u8], path: &String, intent: &[u8]) -> Result<[u8; 64]> {
    let mut hasher = Blake2bVar::new(32).unwrap();
    hasher.update(intent);
    let mut hash = [0u8; 32];
    hasher.finalize_variable(&mut hash).unwrap();
    println!("hash: {:?}", hex::encode(hash));
    let sig =
        keystore::algorithms::ed25519::slip10_ed25519::sign_message_by_seed(seed, path, &hash)
            .map_err(|e| IotaError::SignFailure(e.to_string()))?;
    Ok(sig)
}

#[cfg(test)]
mod test {
    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_iota_parse_transaction() {
        let data = "000002000800e40b54020000000020193a4811b7207ac7a861f840552f9c718172400f4c46bdef5935008a7977fb04020200010100000101030000000001010032bc9471570ca24fcd1fe5b201ea6894748aa0ddd44d20c68f1a4f99db513aa201b9ee296780e0b8e23456c605bacfaad200e468985e5e9a898c7b31919225066eef48630d0000000020b1d6709ed59ff892f9a86cd38493b45b7cbb96cf5a459fdc49cbdbc6e79921f832bc9471570ca24fcd1fe5b201ea6894748aa0ddd44d20c68f1a4f99db513aa2e80300000000000000e40b540200000000";
        let data = "0000020008001a7118020000000020a9b6de90462abd42d8fc56ab56674507599248fac694584d8e892c0f8b54e7ba020200010100000101020000010100193a4811b7207ac7a861f840552f9c718172400f4c46bdef5935008a7977fb04090266c887c1357c50c0b5e6fe7073a22f89deb9c3dfba9edf937edf6f2ca32cbedc42630d00000000206fba305d1986cc6f670d705a8f3cf26b924056aed263d2ab00ec5ad41e8e90691d2ce2ecd7bcf96a333b3ecb850fb57818e998e193ac5c363a831e74ceb0b362e048630d0000000020d154d59ca6838636b0301b1ec42c981628930efaee3afa70360765e85f6f73a723b37ad20af1ff2804bd5bb8d0511fa4bd61a6b069915a86d6c2d133ed4dd699ef48630d0000000020a11d126dc0d3c29fe24890fd4e92b46625dcc914503e5f06ecb847d3a80487755d374b5b4a251b954ccafed02d5d21d6288810b61171bc679c116d6b88b4dc44e148630d0000000020fa23490904041226c3c2d63d5e34ed9228e570f2425064a4f5fbfefa1f00e0347f8ac467b7ca30bf83da6674e5eaae3e534f30ffabdd0eb70cdbc176dec04233414d630d000000002002b9c3b4b45e5cb3498e075e74a6b029b60f096d52485af29688353c990f2c2588f1fc45ae31711da6b398f0ccc10a84979890e1ef7d4d5d7339a71241aa363ddf48630d00000000208e715afd576dc9d8e66ada27436390b4e2fd527cea9cba24cd6b551fb4ea9537915ef82c255b2d9daaefa83e60d0c51eef9cd292abdf8f866e45ba00f8669639f048630d0000000020d37b4af6e26dec342487c5ab5c4b7c3b3da3dfcf1657ee8b34fc3f3c95d0aea3b6c86132d3cf6b13124662c173fa1fc385d15d09d04310390b58331801238090424d630d0000000020dafec40eb97f6c5c6c6dd4440189cff0d9b6beea126d29f23a077a42d4b675c4ba3a5922cb826a3cc2ba0c0398ed8a513f46c867554820370744a5ceeb046864de48630d0000000020d22c4facbc674ccc489de2bbc768fd2a9a9784a539bf84148183cb014c075b70193a4811b7207ac7a861f840552f9c718172400f4c46bdef5935008a7977fb04e803000000000000e06f3c000000000000";
        let tx = parse_intent(hex::decode(data).unwrap().as_slice());
        println!("tx: {:?}", tx.unwrap());
        assert!(false);
    }

    // #[test]
    // fn test_iota_parse_stake_transaction() {
    //     let data = "0000040101000000000000000000000000000000000000000000000000000000000000000501000000000000000101008ad401f77067bcee6173e32269e57c11b9daa1c70456a6ce88646f42f585e4240f7aa71000000000200ae4b6b24c5924970d52b8ea3719c4cfbcaa96bd2ec9cedc8d526c6889c6d37700090100ca9a3b000000000020a276b4c076fff55588255630e9ee35cf0d07e8d80c78991cfd58b43b687b4206020500010101000000000000000000000000000000000000000000000000000000000000000000030b696f74615f73797374656d1a726571756573745f6164645f7374616b655f6d756c5f636f696e0004010000020000010200010300816b9e2fc2460bc7acf1fca2549cd1fa25197ab365d6294cc407ea97481c8de001be3d5998a9c68422681fa77b834d0ec35618b5a988c143279ddd4e1197dff7af0f7aa71000000000200369186897ac8e03297c1b5e75679193f516259756ce6c9afe39d8ba7a362cc3816b9e2fc2460bc7acf1fca2549cd1fa25197ab365d6294cc407ea97481c8de0e803000000000000404b4c000000000000";
    //     let tx = parse_intent(data);
    //     println!("tx: {:?}", tx.unwrap());
    //     assert!(false);
    // }

    #[test]
    fn test_sign_123() {
        let seed = hex::decode("1233e88dd9325e7d5b0a30aaa74df3a8d54b7ec0f3f6a2b7220f04d1120b02f5145706807bc4fa958093465835785c79cce0dbad18b2b5fe0d5a0607508dc927").unwrap();
        let hd_path = "m/44'/4218'/0'/0'/0'".to_string();
        let msg =  hex::decode("000000000002000801000000000000000020193a4811b7207ac7a861f840552f9c718172400f4c46bdef5935008a7977fb040202000101000001010300000000010100193a4811b7207ac7a861f840552f9c718172400f4c46bdef5935008a7977fb04010266c887c1357c50c0b5e6fe7073a22f89deb9c3dfba9edf937edf6f2ca32cbe424e630d0000000020afd57ea9cebffe7de3cbbb64344d6afa15d5a53ad682f525e5ccfbcbce138bf8193a4811b7207ac7a861f840552f9c718172400f4c46bdef5935008a7977fb04e803000000000000404b4c000000000000").unwrap();
        let signature = sign_intent(&seed, &hd_path, &msg).unwrap();
        assert_eq!(hex::encode(signature), "f4b79835417490958c72492723409289b444f3af18274ba484a9eeaca9e760520e453776e5975df058b537476932a45239685f694fc6362fe5af6ba714da6505");
    }
}
