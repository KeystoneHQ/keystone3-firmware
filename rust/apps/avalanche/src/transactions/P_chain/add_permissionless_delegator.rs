use super::validator::Validator;
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use crate::transactions::base_tx::BaseTx;
use crate::transactions::subnet_auth::SubnetAuth;
use crate::transactions::subnet_id::SubnetId;
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
    subnet_auth: SubnetAuth,
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

        let subnet_auth = SubnetAuth::try_from(bytes.clone())?;
        bytes.advance(SUBNET_AUTH_LEN);

        Ok(AddPermissLessionDelegatorTx {
            base_tx,
            validator,
            subnet_id,
            subnet_auth,
        })
    }
}

mod tests {
    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_add_permissionless_delegator() {
        let input_bytes = "00000000001a000000050000000000000000000000000000000000000000000000000000000000000000000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000002faec7a5000000000000000000000001000000010969ea62e2bb30e66d82e82fe267edf6871ea5f70000000257d5e23e2e1f460b618bba1b55913ff3ceb315f0d1acc41fe6408edc4de9facd000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000003b9aca000000000100000000dcf4ca85474e87a743ec8feb54836d2b403b36c7c738c3e2498fdd346dac4774000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000002faef3a100000001000000000000000008e1ac7974a881471c6307919763cbd6e864ca7500000000674fb097000000006751028f000000003b9aca000000000000000000000000000000000000000000000000000000000000000000000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000003b9aca00000000000000000000000001000000010969ea62e2bb30e66d82e82fe267edf6871ea5f70000000b000000000000000000000001000000010969ea62e2bb30e66d82e82fe267edf6871ea5f700000002000000090000000128c2e89bdf40db8016b300ef82db0cd9c4dd83dc056db63f6a3de89535e04c8656e44f2d1297a3e53165c3f224a2f0a044e26313e0abc47bd202b1d73c628606000000000900000001ff2962796baf62808b6709e9463604e159a8e89b9fb8ac1ce03f5e72099df4132b91b53d1916a9678c25032e58972a8ba06c213586fb6d0cdd4aa5a8715bac08000a0daf59";
        let mut bytes = Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
        let result = AddPermissLessionDelegatorTx::try_from(bytes.clone()).unwrap();
        println!("{:?}", result);
        assert!(false);
    }
}
