use alloc::boxed::Box;
use alloc::string::ToString;
use alloc::vec::Vec;

use app_wallets::keplr::sync_info::SyncInfo;
use ur_registry::extend::key_derivation_schema::{Curve, DerivationAlgo};
use ur_registry::extend::qr_hardware_call::{CallParams, CallType, QRHardwareCall};

use crate::common::errors::RustCError;
use crate::common::ffi::VecFFI;
use crate::common::free::Free;
use crate::common::structs::Response;
use crate::common::types::{Ptr, PtrString, PtrT};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};

#[repr(C)]
pub struct QRHardwareCallData {
    pub call_type: PtrString,
    pub origin: PtrString,
    pub key_derivation: Ptr<KeyDerivationRequestData>,
    pub version: PtrString,
}

#[repr(C)]
pub struct KeyDerivationRequestData {
    schemas: Ptr<VecFFI<KeyDerivationSchema>>,
}

impl_c_ptr!(KeyDerivationRequestData);

#[repr(C)]
pub struct KeyDerivationSchema {
    key_path: PtrString,
    curve: PtrString,
    algo: PtrString,
    chain_type: PtrString,
    is_ada: bool,
}

impl TryFrom<&mut QRHardwareCall> for QRHardwareCallData {
    type Error = RustCError;
    fn try_from(value: &mut QRHardwareCall) -> Result<Self, Self::Error> {
        match value.get_params() {
            CallParams::KeyDerivation(data) => {
                let schemas = data
                    .get_schemas()
                    .iter()
                    .map(KeyDerivationSchema::try_from)
                    .collect::<Result<Vec<KeyDerivationSchema>, RustCError>>()?;

                // todo check path is valid
                Ok(Self {
                    call_type: convert_c_char(match value.get_call_type() {
                        CallType::KeyDerivation => "key_derivation".to_string(),
                    }),
                    origin: convert_c_char(value.get_origin().unwrap_or("unknown".to_string())),
                    key_derivation: KeyDerivationRequestData {
                        schemas: VecFFI::from(schemas).c_ptr(),
                    }
                    .c_ptr(),
                    // todo version1
                    version: convert_c_char(value.get_version().to_string()),
                })
            }
        }
    }
}

impl Free for QRHardwareCallData {
    unsafe fn free(&self) {
        free_str_ptr!(self.call_type);
        free_str_ptr!(self.origin);
        let data = Box::from_raw(self.key_derivation);
        let vec = Box::from_raw(data.schemas);
        let v = Vec::from_raw_parts(vec.data, vec.size, vec.cap);
        v.iter().for_each(|v| v.free());
        free_str_ptr!(self.version);
    }
}

impl TryFrom<&ur_registry::extend::key_derivation_schema::KeyDerivationSchema>
    for KeyDerivationSchema
{
    type Error = RustCError;
    fn try_from(
        value: &ur_registry::extend::key_derivation_schema::KeyDerivationSchema,
    ) -> Result<Self, Self::Error> {
        let path = value
            .get_key_path()
            .get_path()
            .ok_or(RustCError::InvalidData(
                "key derivation request not specified key path".to_string(),
            ))?;
        Ok(Self {
            key_path: convert_c_char(path),
            algo: convert_c_char(match value.get_algo_or_default() {
                DerivationAlgo::Bip32Ed25519 => "BIP32-ED25519".to_string(),
                DerivationAlgo::Slip10 => "SLIP10".to_string(),
            }),
            curve: convert_c_char(match value.get_curve_or_default() {
                Curve::Ed25519 => "ED25519".to_string(),
                Curve::Secp256k1 => "Secp256k1".to_string(),
            }),
            chain_type: convert_c_char(value.get_chain_type().unwrap_or("UNKNOWN".to_string())),
            is_ada: value
                .get_chain_type()
                .unwrap_or("UNKNOWN".to_string())
                .starts_with("ADA"),
        })
    }
}

impl Free for KeyDerivationSchema {
    unsafe fn free(&self) {
        free_str_ptr!(self.algo);
        free_str_ptr!(self.curve);
        free_str_ptr!(self.key_path);
        free_str_ptr!(self.chain_type);
    }
}

make_free_method!(Response<QRHardwareCallData>);
