use super::output_owner::OutputOwner;
use super::signer::Signer;
use super::validator::Validator;
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use crate::transactions::base_tx::BaseTx;
use crate::transactions::structs::{
    AvaxFromToInfo, AvaxMethodInfo, AvaxTxInfo, LengthPrefixedVec, ParsedSizeAble,
};
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
pub struct AddPermissLessionValidatorTx {
    base_tx: BaseTx,
    validator: Validator,
    subnet_id: SubnetId,
    signer: Signer,
    stake_out: LengthPrefixedVec<TransferableOutput>,
    validator_owner: OutputOwner,
    delegator_owner: OutputOwner,
    delegator_share: u32,
}

impl AvaxTxInfo for AddPermissLessionValidatorTx {
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
                    output.get_amount(),
                    output.get_addresses(),
                    X_P_CHAIN_PREFIX.to_string(),
                )
            }))
            .collect()
    }

    fn get_method_info(&self) -> Option<AvaxMethodInfo> {
        Some(AvaxMethodInfo::from(
            "AddPermlessValidator".to_string(),
            self.validator.start_time,
            self.validator.end_time,
        ))
    }

    fn get_subnet_id(&self) -> Option<String> {
        Some("Primary Subnet".to_string())
    }

    fn get_network(&self) -> Option<String> {
        Some("Avalanche P-Chain".to_string())
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

impl TryFrom<Bytes> for AddPermissLessionValidatorTx {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let base_tx = BaseTx::try_from(bytes.clone())?;
        bytes.advance(base_tx.parsed_size());

        let validator = Validator::try_from(bytes.clone())?;
        bytes.advance(validator.parsed_size());

        let subnet_id = SubnetId::try_from(bytes.clone())?;
        bytes.advance(SUBNET_ID_LEN);

        let signer = Signer::try_from(bytes.clone())?;
        bytes.advance(PROOF_OF_POSESSION_PUBKEY_LEN + PROOF_OF_POSESSION_SIGNATURE_LEN);

        let stake_out = LengthPrefixedVec::<TransferableOutput>::try_from(bytes.clone())?;
        bytes.advance(stake_out.parsed_size());

        let validator_owner = OutputOwner::try_from(bytes.clone())?;
        bytes.advance(validator_owner.parsed_size());

        let delegator_owner = OutputOwner::try_from(bytes.clone())?;
        bytes.advance(delegator_owner.parsed_size());

        let delegator_share = bytes.get_u32();

        Ok(AddPermissLessionValidatorTx {
            base_tx,
            validator,
            subnet_id,
            signer,
            stake_out,
            validator_owner,
            delegator_owner,
            delegator_share,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_add_permissionless_validator() {
        // 23HbZUQ7ijjrDHfqnjKpd4MTMRY18Gc2JxCz79ZBwZAsCLfntb
        let input_bytes = "000000000019000000050000000000000000000000000000000000000000000000000000000000000000000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000005e57a370000000000000000000000010000000161cd7d26c72edc631d4114d6eef2c4e069ec9206000000020033b7653ffbf19a2352591e8b6aea8e7c75f38d8e8f5f781cf15aad8425010a000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000003b9ab9d9000000010000000029ec95b1c9df6cd2598852d78fea7766c1aece1a7b5d24f6cf58adc98107f927000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000003b8ab46a0000000100000000000000009e843011540909cc62c64dc85a8a1507813777410000000067614160000000006851245000000000713fb30000000000000000000000000000000000000000000000000000000000000000000000001c87c87cef2e92bface778c711c752168a6e858d58ba62463e8bab336f9b05c98c695acf3c7da02b05c667ce5627e63a60ad53ad7da84734084394dedf6b3c4bb6c85922c2b08b09c55508d49d348ad0dcd9678be58197fef69bad862b1d170f4b0c24f189d9d4b6b5103d28b5e8146d305e28d3dcfb3279f089c7152535a24800c7a1a212868a5c76e3559ea9d4a64d9d000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000700000000713fb3000000000000000000000000010000000161cd7d26c72edc631d4114d6eef2c4e069ec92060000000b0000000000000000000000010000000161cd7d26c72edc631d4114d6eef2c4e069ec92060000000b0000000000000000000000010000000161cd7d26c72edc631d4114d6eef2c4e069ec92060000c350000000020000000900000001b52c42f9bfd2c417d2e4c3251f3ea17ce969ae0579011989a57a3e023793d2226103151470494540b3386a10907ec483c6678ae0fd2681b22a57025632321407000000000900000001b52c42f9bfd2c417d2e4c3251f3ea17ce969ae0579011989a57a3e023793d2226103151470494540b3386a10907ec483c6678ae0fd2681b22a5702563232140700f62a80bf";
        let mut bytes = Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
        let result = AddPermissLessionValidatorTx::try_from(bytes.clone()).unwrap();
        assert_eq!(
            "fuji1v8xh6fk89mwxx82pzntwaukyup57eysxj5xrzz".to_string(),
            result.get_reward_address().unwrap()
        );
    }
}
