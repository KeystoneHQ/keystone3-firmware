// Copyright (c) Aptos
// SPDX-License-Identifier: Apache-2.0

use alloc::vec::Vec;
// use move_core_types::{
//     identifier::{Identifier, IdentStr},
//     language_storage::{ModuleId, TypeTag},
// };
use core::fmt;
use serde::{Deserialize, Serialize};

// pub use move_core_types::abi::{
//     ArgumentABI, ScriptFunctionABI as EntryFunctionABI, TransactionScriptABI, TypeArgumentABI,
// };
use crate::aptos_type::identifier::{IdentStr, Identifier};
use crate::aptos_type::language_storage::{ModuleId, TypeTag};
use crate::aptos_type::serde_helper::vec_bytes;
use crate::aptos_type::transaction_argument::TransactionArgument;

/// Call a Move script.
#[derive(Clone, Hash, Eq, PartialEq, Serialize, Deserialize)]
pub struct Script {
    #[serde(with = "serde_bytes")]
    code: Vec<u8>,
    ty_args: Vec<TypeTag>,
    args: Vec<TransactionArgument>,
}

impl Script {
    pub fn new(code: Vec<u8>, ty_args: Vec<TypeTag>, args: Vec<TransactionArgument>) -> Self {
        Script {
            code,
            ty_args,
            args,
        }
    }

    pub fn code(&self) -> &[u8] {
        &self.code
    }

    pub fn ty_args(&self) -> &[TypeTag] {
        &self.ty_args
    }

    pub fn args(&self) -> &[TransactionArgument] {
        &self.args
    }

    pub fn into_inner(self) -> (Vec<u8>, Vec<TypeTag>, Vec<TransactionArgument>) {
        (self.code, self.ty_args, self.args)
    }
}

impl fmt::Debug for Script {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("Script")
            .field("code", &third_party::hex::encode(&self.code))
            .field("ty_args", &self.ty_args)
            .field("args", &self.args)
            .finish()
    }
}

/// Call a Move entry function.
#[derive(Clone, Debug, Hash, Eq, PartialEq, Serialize, Deserialize)]
pub struct EntryFunction {
    module: ModuleId,
    function: Identifier,
    ty_args: Vec<TypeTag>,
    #[serde(with = "vec_bytes")]
    args: Vec<Vec<u8>>,
}

impl EntryFunction {
    pub fn new(
        module: ModuleId,
        function: Identifier,
        ty_args: Vec<TypeTag>,
        args: Vec<Vec<u8>>,
    ) -> Self {
        EntryFunction {
            module,
            function,
            ty_args,
            args,
        }
    }

    pub fn module(&self) -> &ModuleId {
        &self.module
    }

    pub fn function(&self) -> &IdentStr {
        &self.function
    }

    pub fn ty_args(&self) -> &[TypeTag] {
        &self.ty_args
    }

    pub fn args(&self) -> &[Vec<u8>] {
        &self.args
    }

    pub fn into_inner(self) -> (ModuleId, Identifier, Vec<TypeTag>, Vec<Vec<u8>>) {
        (self.module, self.function, self.ty_args, self.args)
    }
}
