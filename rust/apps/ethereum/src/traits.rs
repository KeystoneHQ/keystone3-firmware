use crate::structs::TransactionAction;
use crate::Bytes;

pub trait BaseTransaction {
    fn get_action(&self) -> TransactionAction;
    fn get_to(&self) -> Bytes {
        match self.get_action() {
            TransactionAction::Call(hash) => hash.to_fixed_bytes().to_vec(),
            TransactionAction::Create => [0u8; 20].to_vec(),
        }
    }
}

#[macro_export]
macro_rules! impl_base_transaction {
    ($t: ident) => {
        impl BaseTransaction for $t {
            fn get_action(&self) -> TransactionAction {
                self.action.clone()
            }
        }
    };
}
