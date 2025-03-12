use alloc::vec::Vec;
// follow the ledger signMessage logic
// https://github.com/LedgerHQ/app-cardano/blob/develop/src/signMsg.c#L328
use minicbor::encode::Write;
use minicbor::Encoder;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct CardanoCip8SigStructureLedgerType {
    pub address_field: Vec<u8>,
    pub payload: Vec<u8>,
}
// protectedHeader = {
//     1 : -8,                         // set algorithm to EdDSA
//     “address” : address_bytes       // raw address given by the user, or key hash
// }
fn create_protected_header(address_field_bytes: &[u8]) -> Vec<u8> {
    let mut buffer = Vec::new();
    let mut e = Encoder::new(&mut buffer);
    e.map(2).unwrap();
    e.i8(1).unwrap();
    e.i8(-8).unwrap();
    e.str("address").unwrap();
    e.bytes(address_field_bytes).unwrap();
    buffer
}

// Sig_structure = [
// 	   context : “Signature1”,
//     body_protected : CBOR_encode(protectedHeader),
//     external_aad : bstr,            // empty buffer here
//     payload : bstr                  // message or its hash as bytes
// ]
impl<C> minicbor::Encode<C> for CardanoCip8SigStructureLedgerType {
    fn encode<W: Write>(
        &self,
        e: &mut Encoder<W>,
        _ctx: &mut C,
    ) -> Result<(), minicbor::encode::Error<W::Error>> {
        e.array(4)?;
        e.str("Signature1")?;
        let protected_header = create_protected_header(&self.address_field);
        e.bytes(&protected_header)?;
        e.bytes(&[])?;
        e.bytes(&self.payload)?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use hex;
    #[test]
    fn test1() {
        // Signature1
        // A20127676164647265737358390014C16D7F43243BD81478E68B9DB53A8528FD4FB1078D58D54A7F11241D227AEFA4B773149170885AADBA30AAB3127CC611DDBC4999DEF61C
        // []
        // 42D1854B7D69E3B57C64FCC7B4F64171B47DFF43FBA6AC0499FF437F
        let cardano_address_hex = hex::decode("0014c16d7f43243bd81478e68b9db53a8528fd4fb1078d58d54a7f11241d227aefa4b773149170885aadba30aab3127cc611ddbc4999def61c".to_uppercase()).unwrap();
        // payload = blake2b(messageHex)
        let data = CardanoCip8SigStructureLedgerType {
            address_field: cardano_address_hex,
            payload: hex::decode("42D1854B7D69E3B57C64FCC7B4F64171B47DFF43FBA6AC0499FF437F")
                .unwrap(),
        };
        let encoded = minicbor::to_vec(data).unwrap();
        let encoded_hex = hex::encode(encoded).to_uppercase();
        assert_eq!("846A5369676E6174757265315846A20127676164647265737358390014C16D7F43243BD81478E68B9DB53A8528FD4FB1078D58D54A7F11241D227AEFA4B773149170885AADBA30AAB3127CC611DDBC4999DEF61C40581C42D1854B7D69E3B57C64FCC7B4F64171B47DFF43FBA6AC0499FF437F", encoded_hex);
    }
}
