use crate::aptos_type::safe_serialize;
use alloc::borrow::ToOwned;
use alloc::boxed::Box;
use alloc::string::String;
use alloc::vec::Vec;
use alloc::{format, vec};

use crate::aptos_type::account_address::AccountAddress;
use crate::aptos_type::identifier::{IdentStr, Identifier};
use crate::aptos_type::parser::{parse_struct_tag, parse_type_tag};
use core::{
    fmt::{Display, Formatter},
    str::FromStr,
};
use serde::{Deserialize, Serialize};

pub const CODE_TAG: u8 = 0;
pub const RESOURCE_TAG: u8 = 1;

/// Hex address: 0x1
pub const CORE_CODE_ADDRESS: AccountAddress = AccountAddress::ONE;

#[derive(Serialize, Deserialize, Debug, PartialEq, Hash, Eq, Clone, PartialOrd, Ord)]
pub enum TypeTag {
    // alias for compatibility with old json serialized data.
    #[serde(rename = "bool", alias = "Bool")]
    Bool,
    #[serde(rename = "u8", alias = "U8")]
    U8,
    #[serde(rename = "u64", alias = "U64")]
    U64,
    #[serde(rename = "u128", alias = "U128")]
    U128,
    #[serde(rename = "address", alias = "Address")]
    Address,
    #[serde(rename = "signer", alias = "Signer")]
    Signer,
    #[serde(rename = "vector", alias = "Vector")]
    Vector(
        #[serde(
            serialize_with = "safe_serialize::type_tag_recursive_serialize",
            deserialize_with = "safe_serialize::type_tag_recursive_deserialize"
        )]
        // #[serde(skip_serializing)]
        Box<TypeTag>,
    ),
    #[serde(rename = "struct", alias = "Struct")]
    Struct(
        #[serde(
            serialize_with = "safe_serialize::type_tag_recursive_serialize",
            deserialize_with = "safe_serialize::type_tag_recursive_deserialize"
        )]
        // #[serde(skip_serializing)]
        Box<StructTag>,
    ),
}

impl FromStr for TypeTag {
    type Err = crate::errors::AptosError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        parse_type_tag(s)
    }
}

#[derive(Serialize, Deserialize, Debug, PartialEq, Hash, Eq, Clone, PartialOrd, Ord)]
pub struct StructTag {
    pub address: AccountAddress,
    pub module: Identifier,
    pub name: Identifier,
    // alias for compatibility with old json serialized data.
    #[serde(rename = "type_args", alias = "type_params")]
    pub type_params: Vec<TypeTag>,
}

impl StructTag {
    pub fn access_vector(&self) -> Vec<u8> {
        let mut key = vec![RESOURCE_TAG];
        key.append(&mut bcs::to_bytes(self).unwrap());
        key
    }

    pub fn module_id(&self) -> ModuleId {
        ModuleId::new(self.address, self.module.to_owned())
    }
}

impl FromStr for StructTag {
    type Err = crate::errors::AptosError;

    fn from_str(s: &str) -> crate::errors::Result<Self> {
        parse_struct_tag(s)
    }
}

/// Represents the initial key into global storage where we first index by the address, and then
/// the struct tag
#[derive(Serialize, Deserialize, Debug, PartialEq, Hash, Eq, Clone, PartialOrd, Ord)]
pub struct ResourceKey {
    pub address: AccountAddress,
    pub type_: StructTag,
}

impl ResourceKey {
    pub fn address(&self) -> AccountAddress {
        self.address
    }

    pub fn type_(&self) -> &StructTag {
        &self.type_
    }
}

impl ResourceKey {
    pub fn new(address: AccountAddress, type_: StructTag) -> Self {
        ResourceKey { address, type_ }
    }
}

/// Represents the initial key into global storage where we first index by the address, and then
/// the struct tag
#[derive(Serialize, Deserialize, Debug, PartialEq, Hash, Eq, Clone, PartialOrd, Ord)]
pub struct ModuleId {
    address: AccountAddress,
    name: Identifier,
}

impl From<ModuleId> for (AccountAddress, Identifier) {
    fn from(module_id: ModuleId) -> Self {
        (module_id.address, module_id.name)
    }
}

impl ModuleId {
    pub fn new(address: AccountAddress, name: Identifier) -> Self {
        ModuleId { address, name }
    }

    pub fn name(&self) -> &IdentStr {
        &self.name
    }

    pub fn address(&self) -> &AccountAddress {
        &self.address
    }

    pub fn access_vector(&self) -> Vec<u8> {
        let mut key = vec![CODE_TAG];
        key.append(&mut bcs::to_bytes(self).unwrap());
        key
    }
}

impl Display for ModuleId {
    fn fmt(&self, f: &mut Formatter) -> core::fmt::Result {
        write!(f, "{}::{}", self.address, self.name)
    }
}

impl ModuleId {
    pub fn short_str_lossless(&self) -> String {
        format!("0x{}::{}", self.address.short_str_lossless(), self.name)
    }
}

impl Display for StructTag {
    fn fmt(&self, f: &mut Formatter) -> core::fmt::Result {
        write!(
            f,
            "0x{}::{}::{}",
            self.address.short_str_lossless(),
            self.module,
            self.name
        )?;
        if let Some(first_ty) = self.type_params.first() {
            write!(f, "<")?;
            write!(f, "{first_ty}")?;
            for ty in self.type_params.iter().skip(1) {
                write!(f, ", {ty}")?;
            }
            write!(f, ">")?;
        }
        Ok(())
    }
}

impl Display for TypeTag {
    fn fmt(&self, f: &mut Formatter) -> core::fmt::Result {
        match self {
            TypeTag::Struct(s) => write!(f, "{s}"),
            TypeTag::Vector(ty) => write!(f, "vector<{ty}>"),
            TypeTag::U8 => write!(f, "u8"),
            TypeTag::U64 => write!(f, "u64"),
            TypeTag::U128 => write!(f, "u128"),
            TypeTag::Address => write!(f, "address"),
            TypeTag::Signer => write!(f, "signer"),
            TypeTag::Bool => write!(f, "bool"),
        }
    }
}

impl Display for ResourceKey {
    fn fmt(&self, f: &mut Formatter) -> core::fmt::Result {
        write!(f, "0x{}/{}", self.address.short_str_lossless(), self.type_)
    }
}

impl From<StructTag> for TypeTag {
    fn from(t: StructTag) -> TypeTag {
        TypeTag::Struct(Box::new(t))
    }
}

#[cfg(test)]
mod tests {
    extern crate std;
    use super::*;
    use crate::aptos_type::account_address::AccountAddress;
    use crate::aptos_type::identifier::Identifier;

    #[test]
    fn test_struct_tag_access_vector() {
        let address = AccountAddress::ONE;
        let module = Identifier::new("test").unwrap();
        let name = Identifier::new("Test").unwrap();
        let struct_tag = StructTag {
            address,
            module,
            name,
            type_params: vec![],
        };
        let access_vector = struct_tag.access_vector();
        assert!(!access_vector.is_empty());
        assert_eq!(access_vector[0], RESOURCE_TAG);
    }

    #[test]
    fn test_struct_tag_module_id() {
        let address = AccountAddress::ONE;
        let module = Identifier::new("test").unwrap();
        let name = Identifier::new("Test").unwrap();
        let struct_tag = StructTag {
            address,
            module: module.clone(),
            name,
            type_params: vec![],
        };
        let module_id = struct_tag.module_id();
        assert_eq!(module_id.address(), &AccountAddress::ONE);
        assert_eq!(module_id.name().as_str(), "test");
    }

    #[test]
    fn test_resource_key_new() {
        let address = AccountAddress::ONE;
        let module = Identifier::new("test").unwrap();
        let name = Identifier::new("Test").unwrap();
        let struct_tag = StructTag {
            address,
            module,
            name,
            type_params: vec![],
        };
        let resource_key = ResourceKey::new(address, struct_tag);
        assert_eq!(resource_key.address(), AccountAddress::ONE);
    }

    #[test]
    fn test_resource_key_address() {
        let address = AccountAddress::ONE;
        let module = Identifier::new("test").unwrap();
        let name = Identifier::new("Test").unwrap();
        let struct_tag = StructTag {
            address,
            module,
            name,
            type_params: vec![],
        };
        let resource_key = ResourceKey::new(address, struct_tag);
        assert_eq!(resource_key.address(), AccountAddress::ONE);
    }

    #[test]
    fn test_resource_key_type() {
        let address = AccountAddress::ONE;
        let module = Identifier::new("test").unwrap();
        let name = Identifier::new("Test").unwrap();
        let struct_tag = StructTag {
            address,
            module,
            name,
            type_params: vec![],
        };
        let resource_key = ResourceKey::new(address, struct_tag);
        assert_eq!(resource_key.type_().module.as_str(), "test");
    }

    #[test]
    fn test_module_id_new() {
        let address = AccountAddress::ONE;
        let name = Identifier::new("test").unwrap();
        let module_id = ModuleId::new(address, name);
        assert_eq!(module_id.address(), &AccountAddress::ONE);
    }

    #[test]
    fn test_module_id_name() {
        let address = AccountAddress::ONE;
        let name = Identifier::new("test").unwrap();
        let module_id = ModuleId::new(address, name);
        assert_eq!(module_id.name().as_str(), "test");
    }

    #[test]
    fn test_module_id_address() {
        let address = AccountAddress::ONE;
        let name = Identifier::new("test").unwrap();
        let module_id = ModuleId::new(address, name);
        assert_eq!(module_id.address(), &AccountAddress::ONE);
    }

    #[test]
    fn test_module_id_access_vector() {
        let address = AccountAddress::ONE;
        let name = Identifier::new("test").unwrap();
        let module_id = ModuleId::new(address, name);
        let access_vector = module_id.access_vector();
        assert!(!access_vector.is_empty());
        assert_eq!(access_vector[0], CODE_TAG);
    }

    #[test]
    fn test_module_id_display() {
        let address = AccountAddress::ONE;
        let name = Identifier::new("test").unwrap();
        let module_id = ModuleId::new(address, name);
        let s = format!("{}", module_id);
        assert!(s.contains("test"));
    }

    #[test]
    fn test_module_id_short_str_lossless() {
        let address = AccountAddress::ONE;
        let name = Identifier::new("test").unwrap();
        let module_id = ModuleId::new(address, name);
        let s = module_id.short_str_lossless();
        assert!(s.contains("test"));
        assert!(s.starts_with("0x"));
    }

    #[test]
    fn test_module_id_from_tuple() {
        let address = AccountAddress::ONE;
        let name = Identifier::new("test").unwrap();
        let module_id = ModuleId::new(address, name.clone());
        let (addr, ident): (AccountAddress, Identifier) = module_id.into();
        assert_eq!(addr, AccountAddress::ONE);
        assert_eq!(ident.as_str(), "test");
    }

    #[test]
    fn test_struct_tag_display() {
        let address = AccountAddress::ONE;
        let module = Identifier::new("test").unwrap();
        let name = Identifier::new("Test").unwrap();
        let struct_tag = StructTag {
            address,
            module,
            name,
            type_params: vec![],
        };
        let s = format!("{}", struct_tag);
        assert!(s.contains("test"));
        assert!(s.contains("Test"));
    }

    #[test]
    fn test_struct_tag_display_with_type_params() {
        let address = AccountAddress::ONE;
        let module = Identifier::new("test").unwrap();
        let name = Identifier::new("Test").unwrap();
        let struct_tag = StructTag {
            address,
            module,
            name,
            type_params: vec![TypeTag::Bool],
        };
        let s = format!("{}", struct_tag);
        assert!(s.contains("<"));
    }

    #[test]
    fn test_type_tag_display() {
        assert_eq!(format!("{}", TypeTag::Bool), "bool");
        assert_eq!(format!("{}", TypeTag::U8), "u8");
        assert_eq!(format!("{}", TypeTag::U64), "u64");
        assert_eq!(format!("{}", TypeTag::U128), "u128");
        assert_eq!(format!("{}", TypeTag::Address), "address");
        assert_eq!(format!("{}", TypeTag::Signer), "signer");
    }

    #[test]
    fn test_type_tag_display_vector() {
        let vec_tag = TypeTag::Vector(Box::new(TypeTag::Bool));
        let s = format!("{}", vec_tag);
        assert!(s.contains("vector"));
        assert!(s.contains("bool"));
    }

    #[test]
    fn test_type_tag_display_struct() {
        let address = AccountAddress::ONE;
        let module = Identifier::new("test").unwrap();
        let name = Identifier::new("Test").unwrap();
        let struct_tag = StructTag {
            address,
            module,
            name,
            type_params: vec![],
        };
        let struct_type = TypeTag::Struct(Box::new(struct_tag));
        let s = format!("{}", struct_type);
        assert!(s.contains("test"));
    }

    #[test]
    fn test_type_tag_from_struct_tag() {
        let address = AccountAddress::ONE;
        let module = Identifier::new("test").unwrap();
        let name = Identifier::new("Test").unwrap();
        let struct_tag = StructTag {
            address,
            module,
            name,
            type_params: vec![],
        };
        let type_tag: TypeTag = struct_tag.into();
        match type_tag {
            TypeTag::Struct(_) => {}
            _ => panic!("Expected Struct variant"),
        }
    }

    #[test]
    fn test_resource_key_display() {
        let address = AccountAddress::ONE;
        let module = Identifier::new("test").unwrap();
        let name = Identifier::new("Test").unwrap();
        let struct_tag = StructTag {
            address,
            module,
            name,
            type_params: vec![],
        };
        let resource_key = ResourceKey::new(address, struct_tag);
        let s = format!("{}", resource_key);
        assert!(s.contains("test"));
    }

    #[test]
    fn test_struct_tag_eq() {
        let address = AccountAddress::ONE;
        let module = Identifier::new("test").unwrap();
        let name = Identifier::new("Test").unwrap();
        let struct_tag1 = StructTag {
            address,
            module: module.clone(),
            name: name.clone(),
            type_params: vec![],
        };
        let struct_tag2 = StructTag {
            address,
            module,
            name,
            type_params: vec![],
        };
        assert_eq!(struct_tag1, struct_tag2);
    }

    #[test]
    fn test_resource_key_eq() {
        let address = AccountAddress::ONE;
        let module = Identifier::new("test").unwrap();
        let name = Identifier::new("Test").unwrap();
        let struct_tag = StructTag {
            address,
            module: module.clone(),
            name: name.clone(),
            type_params: vec![],
        };
        let key1 = ResourceKey::new(address, struct_tag.clone());
        let key2 = ResourceKey::new(address, struct_tag);
        assert_eq!(key1, key2);
    }

    #[test]
    fn test_module_id_eq() {
        let address = AccountAddress::ONE;
        let name = Identifier::new("test").unwrap();
        let module_id1 = ModuleId::new(address, name.clone());
        let module_id2 = ModuleId::new(address, name);
        assert_eq!(module_id1, module_id2);
    }
}
