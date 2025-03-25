use crate::errors::ErgoError::DerivationError;
use crate::errors::Result;

use alloc::string::{String, ToString};
use core::str::FromStr;
use ergo_lib::wallet::derivation_path::DerivationPath;
use ergo_lib::wallet::ext_secret_key::ExtSecretKey;
use ergo_lib::wallet::mnemonic::MnemonicSeed;
use hex;

const VERSION_BYTES_MAINNET_PUBLIC: [u8; 4] = [0x04, 0x88, 0xB2, 0x1E];
const PLACEHOLDER_BYTES: [u8; 4] = [0x00, 0x00, 0x00, 0x00];

pub fn ergo_master_seed_to_extended_pubkey(
    seed: &[u8],
    derivation_path_str: String,
) -> Result<String> {
    let _seed: [u8; 64] = seed
        .try_into()
        .map_err(|_| DerivationError("invalid seed".to_string()))?;
    let master_key = ExtSecretKey::derive_master(MnemonicSeed::from(_seed))
        .map_err(|e| DerivationError(e.to_string()))?;
    let derivation_path = DerivationPath::from_str(&*derivation_path_str)
        .map_err(|e| DerivationError(e.to_string()))?;

    let derived_key = master_key
        .derive(derivation_path)
        .map_err(|e| DerivationError(e.to_string()))?;
    let derived_pubkey = derived_key
        .public_key()
        .map_err(|e| DerivationError(e.to_string()))?;

    let mut ret: [u8; 78] = [0; 78];
    ret[0..4].copy_from_slice(&VERSION_BYTES_MAINNET_PUBLIC); // Ergo Mainnet
    ret[4] = derived_key.path().depth() as u8;
    // both ignored on import: https://github.com/ergoplatform/eips/blob/master/eip-0003.md#export-and-import-of-public-keys
    ret[5..9].copy_from_slice(&PLACEHOLDER_BYTES);
    ret[9..13].copy_from_slice(&PLACEHOLDER_BYTES);
    ret[13..45].copy_from_slice(derived_pubkey.chain_code().as_slice());
    ret[45..78].copy_from_slice(derived_pubkey.pub_key_bytes().as_slice());
    Ok(hex::encode(ret))
}

#[cfg(test)]
mod tests {
    use crate::mnemonic::ergo_master_seed_to_extended_pubkey;
    use alloc::string::ToString;

    #[test]
    fn test_ergo_master_seed_to_extended_pubkey() {
        let seed: [u8; 64] = [
            89, 218, 62, 125, 241, 91, 168, 234, 228, 236, 37, 226, 207, 147, 146, 54, 18, 249,
            149, 130, 132, 49, 185, 116, 81, 79, 48, 111, 93, 172, 122, 203, 51, 108, 240, 107, 8,
            140, 24, 249, 33, 255, 46, 241, 130, 90, 235, 135, 162, 15, 206, 189, 128, 18, 229,
            217, 204, 248, 77, 255, 114, 24, 146, 35,
        ];
        let ext_pub_key =
            ergo_master_seed_to_extended_pubkey(&seed, "m/44'/429'/0'/0".to_string()).unwrap();
        assert_eq!(ext_pub_key, "0488b21e0400000000000000002d0cc9b6af65042bf8e3bb907d128b903a6de0f378f78b885b199f2e47eae06202ebdd9cf568085464198cb836882a02065589f5378943911cf737802b77e44efc")
    }
}
