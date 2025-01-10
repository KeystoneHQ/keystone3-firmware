use super::{output_owner::OutputOwner, validator::Validator};
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use crate::transactions::base_tx::BaseTx;
use crate::transactions::structs::{
    AvaxFromToInfo, AvaxMethodInfo, AvaxTxInfo, LengthPrefixedVec, ParsedSizeAble,
};
use crate::transactions::subnet_auth::SubnetAuth;
use crate::transactions::subnet_id::SubnetId;
use crate::transactions::transferable::TransferableOutput;
use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug)]
pub struct AddPermissLessionDelegatorTx {
    base_tx: BaseTx,
    validator: Validator,
    subnet_id: SubnetId,
    stake_out: LengthPrefixedVec<TransferableOutput>,
    delegator_owner: OutputOwner,
}

impl AvaxTxInfo for AddPermissLessionDelegatorTx {
    fn get_total_input_amount(&self) -> u64 {
        self.base_tx.get_total_input_amount()
    }

    fn get_total_output_amount(&self) -> u64 {
        self.base_tx.get_total_output_amount()
            + self
                .stake_out
                .iter()
                .fold(0, |acc, item| acc + item.get_amount())
    }

    fn get_outputs_addresses(&self) -> Vec<AvaxFromToInfo> {
        self.base_tx
            .get_outputs_addresses()
            .into_iter()
            .chain(self.stake_out.iter().map(|output| {
                AvaxFromToInfo::from(
                    format!("{} AVAX", output.get_amount() as f64 / NAVAX_TO_AVAX_RATIO),
                    output.get_addresses(),
                )
            }))
            .collect()
    }

    fn get_method_info(&self) -> Option<AvaxMethodInfo> {
        Some(AvaxMethodInfo::from(
            "AddPermlessDelegator".to_string(),
            self.validator.start_time,
            self.validator.end_time,
        ))
    }

    fn get_network(&self) -> Option<String> {
        Some("Avalanche P-Chain".to_string())
    }

    fn get_subnet_id(&self) -> Option<String> {
        Some("Primary Subnet".to_string())
    }

    fn get_reward_address(&self) -> Option<String> {
        Some(
            self.delegator_owner
                .addresses
                .get(0)
                .and_then(|addr| Some(addr.encode()))
                .unwrap_or_default(),
        )
    }
}

impl TryFrom<Bytes> for AddPermissLessionDelegatorTx {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let base_tx = BaseTx::try_from(bytes.clone())?;
        bytes.advance(base_tx.parsed_size());

        let validator = Validator::try_from(bytes.clone())?;
        bytes.advance(validator.parsed_size());

        let subnet_id = SubnetId::try_from(bytes.clone())?;
        bytes.advance(SUBNET_ID_LEN);

        let stake_out = LengthPrefixedVec::<TransferableOutput>::try_from(bytes.clone())?;
        bytes.advance(stake_out.parsed_size());

        let delegator_owner = OutputOwner::try_from(bytes.clone())?;
        bytes.advance(delegator_owner.parsed_size());

        Ok(AddPermissLessionDelegatorTx {
            base_tx,
            validator,
            subnet_id,
            stake_out,
            delegator_owner,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_add_permissionless_delegator() {
        let input_bytes = "00000000001a000000050000000000000000000000000000000000000000000000000000000000000000000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000003b9a9e0400000000000000000000000100000001e0beb088f94b8224eb5d6f1115561d7173cd6e7f00000002295a7b15e26c6cafda8883afd0f724e0e0b1dad4517148711434cb96fb3c8a61000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000003b9aca0000000001000000006109bc613691602ca0811312357676416252412a87ded6c56c240baba1afe042000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000003b9aca000000000100000000000000007072a3df0cd056d9b9ef00c09630bad3027dc312000000006760c3b100000000676215a9000000003b9aca000000000000000000000000000000000000000000000000000000000000000000000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000003b9aca0000000000000000000000000100000001e0beb088f94b8224eb5d6f1115561d7173cd6e7f0000000b00000000000000000000000100000001a0f4d4d9a0ea219da5ed5499ad083e1942a0846a000000020000000900000001438c3a393f49bb27791ca830effec456c2642a487ee4ce89300dd2e591fc22ab6b2aa8e08515ca229f2a2f14168700e05a1f96bd61d1fc3ab31e9e71ef9f16bb000000000900000001438c3a393f49bb27791ca830effec456c2642a487ee4ce89300dd2e591fc22ab6b2aa8e08515ca229f2a2f14168700e05a1f96bd61d1fc3ab31e9e71ef9f16bb005c3d047c";
        let mut bytes = Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
        let result = AddPermissLessionDelegatorTx::try_from(bytes.clone()).unwrap();
        println!("result: {:?}", result);
        assert_eq!(result.base_tx.get_blockchain_id(), P_BLOCKCHAIN_ID);
    }
}
