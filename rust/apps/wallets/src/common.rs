use third_party::ur_registry::{
    crypto_key_path::PathComponent,
    error::{URError, URResult},
};

pub fn get_path_component(index: Option<u32>, hardened: bool) -> URResult<PathComponent> {
    PathComponent::new(index, hardened).map_err(|e| URError::CborEncodeError(e))
}
