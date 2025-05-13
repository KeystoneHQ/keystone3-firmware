use crate::{
    errors::{IotaError, Result},
};
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use hex;
use bytes::{Buf, Bytes};
use serde::{Deserialize, Serialize};
use crate::base_type::{ObjectID, ObjectRef, SequenceNumber};

#[derive(Debug)]
pub enum TransactionData {
    V1(TransactionDataV1),
    // When new variants are introduced, it is important that we check version support
    // in the validity_check function based on the protocol config.
}

pub const IOTA_ADDRESS_LENGTH: usize = ObjectID::LENGTH;

#[derive(Debug)]
// #[derive(Debug, PartialEq, Eq, Hash, Clone, Serialize, Deserialize)]
pub struct TransactionDataV1 {
    pub kind: TransactionKind,
    pub sender: [u8; 32],
    pub gas_data: GasData,
    pub expiration: TransactionExpiration,
}

#[derive(Debug)]
pub enum TransactionKind {
    /// 可编程交易块
    ProgrammableTransaction(ProgrammableTransaction),
    // Future: MoveCallOnly(MoveCall), etc.
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
pub enum Command {
    SplitCoins {
        coin: CoinArg,
        amounts: Vec<CommandArgument>,
    },
    TransferObjects {
        objects: Vec<CommandArgument>,
        target: CommandArgument,
    },
}

#[derive(Debug)]
pub enum CoinArg {
    GasCoin,
    // Future: OtherCoin types...
}

#[derive(Debug)]
pub enum CommandArgument {
    Input(u16),
    NestedResult(u16, u16),
    GasCoin,
}

#[derive(Debug)]
pub struct GasData {
    pub payment: Vec<([u8; 32], u64, [u8; 32])>, // (object_id, version, digest)
    pub owner: [u8; 32],
    pub price: u64,
    pub budget: u64,
}

#[derive(Debug)]
pub enum TransactionExpiration {
    None,
    // Some(T) 如果有具体类型
}

/// 解析入口：根据版本 tag 分发
pub fn parse_transaction_data(hex_str: &str) -> Result<TransactionData> {
    // hex 解码
    let raw = hex::decode(hex_str)?;
    let mut buf = Bytes::from(raw);

    // 读版本 tag (u32 LE)
    if buf.remaining() < 4 {
        return Err(IotaError::UnexpectedEof);
    }
    let version = buf.get_u16_le();
    match version {
        0 => {
            let v1 = parse_v1(&mut buf)?;
            Ok(TransactionData::V1(v1))
        }
        other => Err(IotaError::InvalidField(format!("unsupported version {}", other))),
    }
}

/// 解析版本1 的内容
fn parse_v1(buf: &mut Bytes) -> Result<TransactionDataV1> {
    // 1. inputs: Vec<TransactionInput>
    if buf.remaining() < 4 { return Err(IotaError::UnexpectedEof); }
    let inp_cnt = buf.get_u8() as usize;
    println!("inp_cnt: {}", inp_cnt);
    let mut inputs = Vec::with_capacity(inp_cnt);
    for _ in 0..inp_cnt {
        if buf.remaining() < 4 { return Err(IotaError::UnexpectedEof); }
        println!("buf = {:?}", hex::encode(buf.clone()));
        let data_len = buf.get_u16() as usize;
        println!("data_len: {}", data_len);
        let mut data = vec![0u8; data_len];
        buf.copy_to_slice(&mut data);
        inputs.push(TransactionInput::Pure(data));
    }

    // 2. commands: Vec<Command>
    if buf.remaining() < 4 { return Err(IotaError::UnexpectedEof); }
    let cmd_cnt = buf.get_u32_le() as usize;
    let mut commands = Vec::with_capacity(cmd_cnt);
    for _ in 0..cmd_cnt {
        if buf.remaining() < 4 { return Err(IotaError::UnexpectedEof); }
        let cmd_tag = buf.get_u32_le();
        match cmd_tag {
            2 => {
                // SplitCoins
                if buf.remaining() < 4 { return Err(IotaError::UnexpectedEof); }
                let ctag = buf.get_u32_le();
                let coin = match ctag {
                    1 => CoinArg::GasCoin,
                    other => return Err(IotaError::InvalidField(format!("unknown CoinArg tag {}", other))),
                };
                if buf.remaining() < 4 { return Err(IotaError::UnexpectedEof); }
                let amt_cnt = buf.get_u32_le() as usize;
                let mut amounts = Vec::with_capacity(amt_cnt);
                for _ in 0..amt_cnt {
                    if buf.remaining() < 4 { return Err(IotaError::UnexpectedEof); }
                    let atag = buf.get_u32_le();
                    let arg = match atag {
                        0 => {
                            if buf.remaining() < 2 { return Err(IotaError::UnexpectedEof); }
                            let idx = buf.get_u16_le();
                            CommandArgument::Input(idx)
                        }
                        2 => CommandArgument::GasCoin,
                        other => return Err(IotaError::InvalidField(format!("unknown Arg tag {}", other))),
                    };
                    amounts.push(arg);
                }
                commands.push(Command::SplitCoins { coin, amounts });
            }
            1 => {
                // TransferObjects
                if buf.remaining() < 4 { return Err(IotaError::UnexpectedEof); }
                let obj_cnt = buf.get_u32_le() as usize;
                let mut objects = Vec::with_capacity(obj_cnt);
                for _ in 0..obj_cnt {
                    if buf.remaining() < 4 { return Err(IotaError::UnexpectedEof); }
                    let atag = buf.get_u32_le();
                    let arg = match atag {
                        0 => {
                            if buf.remaining() < 2 { return Err(IotaError::UnexpectedEof); }
                            let idx = buf.get_u16_le();
                            CommandArgument::Input(idx)
                        }
                        3 => {
                            if buf.remaining() < 4 { return Err(IotaError::UnexpectedEof); }
                            let a = buf.get_u16_le();
                            let b = buf.get_u16_le();
                            CommandArgument::NestedResult(a, b)
                        }
                        other => return Err(IotaError::InvalidField(format!("unknown ObjArg tag {}", other))),
                    };
                    objects.push(arg);
                }
                if buf.remaining() < 4 { return Err(IotaError::UnexpectedEof); }
                let ttag = buf.get_u32_le();
                let target = match ttag {
                    0 => {
                        if buf.remaining() < 2 { return Err(IotaError::UnexpectedEof); }
                        let idx = buf.get_u16_le();
                        CommandArgument::Input(idx)
                    }
                    other => return Err(IotaError::InvalidField(format!("unknown TargetArg tag {}", other))),
                };
                commands.push(Command::TransferObjects { objects, target });
            }
            other => return Err(IotaError::InvalidField(format!("unknown Command tag {}", other))),
        }
    }
    let kind = TransactionKind::ProgrammableTransaction(ProgrammableTransaction { inputs, commands });

    // 3. sender
    if buf.remaining() < 32 { return Err(IotaError::UnexpectedEof); }
    let mut sender = [0u8; 32];
    buf.copy_to_slice(&mut sender);

    // 4. gas_data
    if buf.remaining() < 4 { return Err(IotaError::UnexpectedEof); }
    let payment_cnt = buf.get_u32_le() as usize;
    let mut payment = Vec::with_capacity(payment_cnt);
    for _ in 0..payment_cnt {
        if buf.remaining() < 32 + 8 + 4 { return Err(IotaError::UnexpectedEof); }
        let mut oid = [0u8; 32];
        buf.copy_to_slice(&mut oid);
        let version = buf.get_u64_le();
        let dlen = buf.get_u32_le() as usize;
        if buf.remaining() < dlen { return Err(IotaError::UnexpectedEof); }
        let mut digest = [0u8; 32];
        buf.copy_to_slice(&mut digest[..dlen]);
        payment.push((oid, version, digest));
    }
    if buf.remaining() < 32 + 8 + 8 { return Err(IotaError::UnexpectedEof); }
    let mut owner = [0u8; 32];
    buf.copy_to_slice(&mut owner);
    let price  = buf.get_u64_le();
    let budget = buf.get_u64_le();
    let gas_data = GasData { payment, owner, price, budget };

    // 5. expiration
    if buf.remaining() < 4 { return Err(IotaError::UnexpectedEof); }
    let exp_tag = buf.get_u32_le();
    let expiration = match exp_tag {
        0 => TransactionExpiration::None,
        _ => TransactionExpiration::None,
    };

    Ok(TransactionDataV1 { kind, sender, gas_data, expiration })
}

fn parse_transaction(data: &mut Bytes) -> Result<String> {
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
        let data = "000002000800e40b54020000000020193a4811b7207ac7a861f840552f9c718172400f4c46bdef5935008a7977fb04020200010100000101030000000001010032bc9471570ca24fcd1fe5b201ea6894748aa0ddd44d20c68f1a4f99db513aa201b9ee296780e0b8e23456c605bacfaad200e468985e5e9a898c7b31919225066eef48630d0000000020b1d6709ed59ff892f9a86cd38493b45b7cbb96cf5a459fdc49cbdbc6e79921f832bc9471570ca24fcd1fe5b201ea6894748aa0ddd44d20c68f1a4f99db513aa2e80300000000000000e40b540200000000";
        let tx = parse_transaction_data(data);
        println!("tx: {:?}", tx.unwrap());
        assert!(false);
    }
}
