// Copyright (c) Aptos
// SPDX-License-Identifier: Apache-2.0

use alloc::vec;
use alloc::vec::Vec;
use core::fmt;

use serde::{Deserialize, Serialize};

#[derive(Clone, Hash, Eq, PartialEq, Serialize, Deserialize)]
pub struct Module {
    #[serde(with = "serde_bytes")]
    code: Vec<u8>,
}

impl Module {
    pub fn new(code: Vec<u8>) -> Module {
        Module { code }
    }

    pub fn code(&self) -> &[u8] {
        &self.code
    }

    pub fn into_inner(self) -> Vec<u8> {
        self.code
    }
}

impl fmt::Debug for Module {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("Module")
            .field("code", &hex::encode(&self.code))
            .finish()
    }
}

#[derive(Clone, Hash, Eq, PartialEq, Serialize, Deserialize)]
pub struct ModuleBundle {
    codes: Vec<Module>,
}

impl ModuleBundle {
    pub fn new(codes: Vec<Vec<u8>>) -> ModuleBundle {
        ModuleBundle {
            codes: codes.into_iter().map(Module::new).collect(),
        }
    }

    pub fn singleton(code: Vec<u8>) -> ModuleBundle {
        ModuleBundle {
            codes: vec![Module::new(code)],
        }
    }

    pub fn into_inner(self) -> Vec<Vec<u8>> {
        self.codes.into_iter().map(Module::into_inner).collect()
    }

    pub fn iter(&self) -> impl Iterator<Item = &Module> {
        self.codes.iter()
    }
}

impl fmt::Debug for ModuleBundle {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("ModuleBundle")
            .field("codes", &self.codes)
            .finish()
    }
}

impl From<Module> for ModuleBundle {
    fn from(m: Module) -> ModuleBundle {
        ModuleBundle { codes: vec![m] }
    }
}

impl IntoIterator for ModuleBundle {
    type Item = Module;
    type IntoIter = vec::IntoIter<Self::Item>;

    fn into_iter(self) -> Self::IntoIter {
        self.codes.into_iter()
    }
}

#[cfg(test)]
mod tests {
    extern crate std;
    use super::*;

    #[test]
    fn test_module_new() {
        let code = vec![0x01, 0x02, 0x03];
        let module = Module::new(code.clone());
        assert_eq!(module.code(), &[0x01, 0x02, 0x03]);
    }

    #[test]
    fn test_module_code() {
        let code = vec![0x42];
        let module = Module::new(code);
        assert_eq!(module.code(), &[0x42]);
    }

    #[test]
    fn test_module_into_inner() {
        let code = vec![0x01, 0x02];
        let module = Module::new(code.clone());
        let inner = module.into_inner();
        assert_eq!(inner, code);
    }

    #[test]
    fn test_module_debug() {
        let code = vec![0x01, 0x02];
        let module = Module::new(code);
        let s = format!("{:?}", module);
        assert!(s.contains("Module"));
        assert!(s.contains("code"));
    }

    #[test]
    fn test_module_bundle_new() {
        let codes = vec![vec![0x01], vec![0x02, 0x03]];
        let bundle = ModuleBundle::new(codes);
        assert_eq!(bundle.iter().count(), 2);
    }

    #[test]
    fn test_module_bundle_singleton() {
        let code = vec![0x01, 0x02];
        let bundle = ModuleBundle::singleton(code.clone());
        let modules: Vec<_> = bundle.iter().collect();
        assert_eq!(modules.len(), 1);
        assert_eq!(modules[0].code(), code.as_slice());
    }

    #[test]
    fn test_module_bundle_into_inner() {
        let codes = vec![vec![0x01], vec![0x02]];
        let bundle = ModuleBundle::new(codes.clone());
        let inner = bundle.into_inner();
        assert_eq!(inner, codes);
    }

    #[test]
    fn test_module_bundle_iter() {
        let codes = vec![vec![0x01], vec![0x02]];
        let bundle = ModuleBundle::new(codes);
        let mut iter = bundle.iter();
        assert!(iter.next().is_some());
        assert!(iter.next().is_some());
        assert!(iter.next().is_none());
    }

    #[test]
    fn test_module_bundle_debug() {
        let codes = vec![vec![0x01]];
        let bundle = ModuleBundle::new(codes);
        let s = format!("{:?}", bundle);
        assert!(s.contains("ModuleBundle"));
    }

    #[test]
    fn test_module_from_module_bundle() {
        let code = vec![0x01];
        let module = Module::new(code);
        let bundle: ModuleBundle = module.into();
        assert_eq!(bundle.iter().count(), 1);
    }

    #[test]
    fn test_module_bundle_into_iterator() {
        let codes = vec![vec![0x01], vec![0x02]];
        let bundle = ModuleBundle::new(codes);
        let modules: Vec<Module> = bundle.into_iter().collect();
        assert_eq!(modules.len(), 2);
    }

    #[test]
    fn test_module_eq() {
        let code1 = vec![0x01];
        let code2 = vec![0x01];
        let module1 = Module::new(code1);
        let module2 = Module::new(code2);
        assert_eq!(module1, module2);
    }

    #[test]
    fn test_module_ne() {
        let code1 = vec![0x01];
        let code2 = vec![0x02];
        let module1 = Module::new(code1);
        let module2 = Module::new(code2);
        assert_ne!(module1, module2);
    }

    #[test]
    fn test_module_clone() {
        let code = vec![0x01];
        let module1 = Module::new(code);
        let module2 = module1.clone();
        assert_eq!(module1, module2);
    }

    #[test]
    fn test_module_bundle_eq() {
        let codes = vec![vec![0x01]];
        let bundle1 = ModuleBundle::new(codes.clone());
        let bundle2 = ModuleBundle::new(codes);
        assert_eq!(bundle1, bundle2);
    }

    #[test]
    fn test_module_bundle_clone() {
        let codes = vec![vec![0x01]];
        let bundle1 = ModuleBundle::new(codes);
        let bundle2 = bundle1.clone();
        assert_eq!(bundle1, bundle2);
    }
}
