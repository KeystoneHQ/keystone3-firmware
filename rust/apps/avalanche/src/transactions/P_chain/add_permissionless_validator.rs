use super::signer::Signer;
use super::validator::Validator;
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use crate::transactions::base_tx::BaseTx;
use crate::transactions::structs::{LengthPrefixedVec, ParsedSizeAble};
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
pub struct AddPermissLessionValidatorTx {
    base_tx: BaseTx,
    validator: Validator,
    subnet_id: SubnetId,
    signer: Signer,
    stack_out: LengthPrefixedVec<TransferableOutput>,
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

        let stack_out = LengthPrefixedVec::<TransferableOutput>::try_from(bytes.clone())?;
        bytes.advance(stack_out.parsed_size());

        // let subnet_auth = SubnetAuth::try_from(bytes.clone())?;
        // bytes.advance(SUBNET_AUTH_LEN);

        Ok(AddPermissLessionValidatorTx {
            base_tx,
            validator,
            subnet_id,
            signer,
            stack_out,
        })
    }
}

mod tests {
    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_add_permissionless_validator() {
        let input_bytes = "000000000019000000050000000000000000000000000000000000000000000000000000000000000000000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000005f028280000000000000000000000001000000018011c8bb6f8728d618700e599234dd2baaaf2e1e00000002ae86f67c0543b9dde63bbef01b0e8dc6e6ac8b5da70778c692c94e37ed7a8a4c000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000005faa5b400000000100000000c83e1f966af2f0a5e57c37cef4b3d66e0764483fde6c665872a1c9af3fa24d5b000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000500000000772651c000000001000000000000000067544e7262dc873ef41ce0f63d144b222866152800000000674fba5600000000675119e6000000007735940000000000000000000000000000000000000000000000000000000000000000000000001c984e21b049a6cb2700119e01c305ef401e91d6fd9ace4dbf46f468f33cf6b0b87ef33c715d7f1fe58128d7da5b9d3895b5a0f81b6b4a0c7631eb3db2b529e084a2eacda224b7a22926b8c8f61c7a6dc9eed7007448e0025f4e31b561a8f843470aa05a65d9ea9845f7da5f5b5fd21f2ce3632508d1b1e79a21e6942e3cff261d6ba39562e4c59ea976b8d95d536880bb000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000000a7d8c0000000000000000000000001000000018011c8bb6f8728d618700e599234dd2baaaf2e1e3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000700000000768dbb40000000000000000000000001000000018011c8bb6f8728d618700e599234dd2baaaf2e1e0000000b000000000000000000000001000000018011c8bb6f8728d618700e599234dd2baaaf2e1e0000000b000000000000000000000001000000018011c8bb6f8728d618700e599234dd2baaaf2e1e00004e2000000002000000090000000103c4ec1f83edd0142e180aa5b4a91411827681842ad997a3dc2f1c7cf9035d0a0bb7bb55f8eb9a6863bdafbe37035b49626fbd91944d12544d4d3b562338920000000000090000000103c4ec1f83edd0142e180aa5b4a91411827681842ad997a3dc2f1c7cf9035d0a0bb7bb55f8eb9a6863bdafbe37035b49626fbd91944d12544d4d3b562338920000951df0aa";
        let mut bytes = Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
        let result = AddPermissLessionValidatorTx::try_from(bytes.clone()).unwrap();
        println!("{:?}", result.validator);
        println!("{:?}", result.subnet_id);
        println!("{:?}", result.signer);
        println!("{:?}", result.stack_out);
        assert!(false);
    }
}
