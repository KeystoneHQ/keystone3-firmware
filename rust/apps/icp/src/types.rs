use pkcs8::der;
use pkcs8::spki::AssociatedAlgorithmIdentifier;
use pkcs8::spki::{AlgorithmIdentifier, ObjectIdentifier};
use pkcs8::EncodePublicKey;
use third_party::secp256k1::PublicKey;

use crate::errors::ICPError;
use crate::errors::Result;

pub const ALGORITHM_OID: pkcs8::ObjectIdentifier =
    pkcs8::ObjectIdentifier::new_unwrap("1.2.840.10045.2.1");
pub const SECP256K1_OID: pkcs8::ObjectIdentifier =
    pkcs8::ObjectIdentifier::new_unwrap("1.3.132.0.10");

pub struct ICPPublicKey(pub PublicKey);

impl AssociatedAlgorithmIdentifier for ICPPublicKey {
    type Params = ObjectIdentifier;

    const ALGORITHM_IDENTIFIER: AlgorithmIdentifier<ObjectIdentifier> = AlgorithmIdentifier {
        oid: ALGORITHM_OID,
        parameters: Some(SECP256K1_OID),
    };
}

impl EncodePublicKey for ICPPublicKey {
    fn to_public_key_der(&self) -> pkcs8::spki::Result<der::Document> {
        let public_key_bytes = self.0.serialize_uncompressed();
        let subject_public_key = der::asn1::BitStringRef::new(0, &public_key_bytes)?;

        pkcs8::SubjectPublicKeyInfo {
            algorithm: Self::ALGORITHM_IDENTIFIER,
            subject_public_key,
        }
        .try_into()
    }
}

impl TryFrom<PublicKey> for ICPPublicKey {
    type Error = ICPError;

    fn try_from(public_key: PublicKey) -> Result<Self> {
        Ok(Self(public_key))
    }
}
