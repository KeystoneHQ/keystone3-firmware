// Standalone SDK/Ed25519 fixture generator; not part of the firmware build.
use ed25519_dalek::{Signer, SigningKey};
use hmac::{Hmac, Mac};
use sha2::Sha512;
use solana_address::Address;
use solana_hash::Hash;
use solana_message::{
    compiled_instruction::CompiledInstruction,
    v1::{Message, TransactionConfig},
    MessageHeader, VersionedMessage,
};
fn main() {
    let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
    let mut h = Hmac::<Sha512>::new_from_slice(b"ed25519 seed").unwrap();
    h.update(&seed);
    let mut key = h.finalize().into_bytes().to_vec();
    for index in [44u32, 501, 0] {
        let mut h = Hmac::<Sha512>::new_from_slice(&key[32..]).unwrap();
        h.update(&[0]);
        h.update(&key[..32]);
        h.update(&(index | 0x80000000).to_be_bytes());
        key = h.finalize().into_bytes().to_vec();
    }
    let signing = SigningKey::from_bytes(key[..32].try_into().unwrap());
    let mut data = 2u32.to_le_bytes().to_vec();
    data.extend(123456789u64.to_le_bytes());
    let m = Message {
        header: MessageHeader {
            num_required_signatures: 1,
            num_readonly_signed_accounts: 0,
            num_readonly_unsigned_accounts: 1,
        },
        config: TransactionConfig::empty()
            .with_priority_fee(9876543210)
            .with_compute_unit_limit(200000)
            .with_loaded_accounts_data_size_limit(65536)
            .with_heap_size(65536),
        lifetime_specifier: Hash::new_from_array([7; 32]),
        account_keys: vec![
            Address::new_from_array(signing.verifying_key().to_bytes()),
            Address::new_from_array([2; 32]),
            Address::new_from_array([0; 32]),
        ],
        instructions: vec![CompiledInstruction {
            program_id_index: 2,
            accounts: vec![0, 1],
            data,
        }],
    };
    let raw = VersionedMessage::V1(m).serialize();
    println!("message={}", hex::encode(&raw));
    println!("signature={}", hex::encode(signing.sign(&raw).to_bytes()));
}
