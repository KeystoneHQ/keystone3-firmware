use alloc::string::{String, ToString};
use alloc::vec::Vec;
use third_party::bitcoin::{opcodes, script, PublicKey, ScriptBuf};
use third_party::itertools::Itertools;

use crate::addresses::address::Address;
use crate::addresses::xyzpub::Version;
use crate::addresses::{derive_public_key, xyzpub};
use crate::multi_sig::wallet::MultiSigWalletConfig;
use crate::multi_sig::Network;
use crate::{network, BitcoinError};

pub fn create_multi_sig_address(
    wallet: &MultiSigWalletConfig,
    change: u32,
    account: u32,
) -> Result<String, BitcoinError> {
    let pub_keys = wallet
        .xpub_items
        .iter()
        .map(|x| {
            let convert_xpub = xyzpub::convert_version(&x.xpub, &Version::Xpub)?;
            derive_pub_key(&convert_xpub, change, account)
        })
        .collect::<Result<Vec<_>, _>>()?;
    let ordered_pub_keys = pub_keys.iter().sorted().collect::<Vec<_>>();

    let network = wallet.get_network();
    let network = if *network == Network::TestNet {
        network::Network::BitcoinTestnet
    } else {
        network::Network::Bitcoin
    };
    let script = crate_p2ms_script(&ordered_pub_keys, wallet.threshold);

    let script = match wallet.format.as_str() {
        "P2SH" => ScriptBuf::new_p2sh(&script.script_hash()),
        "P2WSH-P2SH" => {
            let p2wsh = ScriptBuf::new_p2wsh(&script.wscript_hash());
            ScriptBuf::new_p2sh(&p2wsh.script_hash())
        }
        "P2WSH" => ScriptBuf::new_p2wsh(&script.wscript_hash()),
        _ => {
            return Err(BitcoinError::MultiSigWalletAddressCalError(format!(
                "not support this format {}",
                wallet.format.as_str()
            )))
        }
    };
    Ok(Address::from_script(script.as_script(), network)?.to_string())
}

fn derive_pub_key(xpub: &String, change: u32, account: u32) -> Result<PublicKey, BitcoinError> {
    Ok(derive_public_key(
        xpub,
        format!("m/{}/{}", change, account),
    )?)
}

fn crate_p2ms_script(pub_keys: &Vec<&PublicKey>, threshold: u32) -> ScriptBuf {
    let builder = pub_keys.iter().fold(
        script::Builder::new().push_int(threshold as i64),
        |builder, key| builder.push_key(key),
    );

    builder
        .push_int(pub_keys.len() as i64)
        .push_opcode(opcodes::all::OP_CHECKMULTISIG)
        .into_script()
}

#[cfg(test)]
mod tests {
    extern crate std;

    use crate::multi_sig::address::create_multi_sig_address;
    use crate::multi_sig::wallet::parse_wallet_config;

    #[test]
    fn test_crate_p2ms_script() {
        // P2SH
        {
            let config = r#"# Keystone Multisig setup file (created by Sparrow)
            #
            Name: testnet1
            Policy: 2 of 2
            Derivation: m/45'
            Format: P2SH

            C45358FA: tpubD9hphZzCi9u5Wcbtq3jQYTzbPv6igoaRWDuhxLUDv5VTffE3gEVovYaqwfVMCa6q8VMdwAcPpFgAdajgmLML6XgYrKBquyYEDQg1HnKm3wQ
            73C5DA0A: tpubD97UxEEVXiRtzRBmHvR38R7QXNz6Dx3A7gKtoe9UgxepdJXExmJCd5Nxsv8YYLgHd3MEBKPzRwgVaJ62kvBSvMtntbkPnv6Pf8Zkny5rC89
            "#;

            let config = parse_wallet_config(config, "73C5DA0A").unwrap();
            let address = create_multi_sig_address(&config, 0, 0).unwrap();
            assert_eq!("2N9YWq8XCTGX7qQj5QCFwx3P5knckrspFBH", address);
        }

        // P2WSH-P2SH
        {
            let config = r#"# Keystone Multisig setup file (created by Sparrow)
            #
            Name: testnet1
            Policy: 2 of 2
            Derivation: m/48'/1'/0'/1'
            Format: P2WSH-P2SH

            C45358FA: tpubDEkyN1rhRi4YRLDbKSRNtP8WhqngeiF3HYw4PNfXuwBgzGZDDZUwmhCWDzNh7Uvy41cqGP3yAo7g1QxRdQMWVt97HmkxsZqYv35d2diniQS
            73C5DA0A: tpubDFH9dgzveyD8yHQb8VrpG8FYAuwcLMHMje2CCcbBo1FpaGzYVtJeYYxcYgRqSTta5utUFts8nPPHs9C2bqoxrey5jia6Dwf9mpwrPq7YvcJ
            "#;

            let config = parse_wallet_config(config, "73C5DA0A").unwrap();
            let address = create_multi_sig_address(&config, 0, 0).unwrap();
            assert_eq!("2N77EPE2yfeTLR3CwNUCBQ7LZEUGW6N9B6y", address);
        }

        // P2WSH
        {
            let config = r#"# Keystone Multisig setup file (created by Sparrow)
            #
            Name: testnet1
            Policy: 2 of 2
            Derivation: m/48'/1'/0'/2'
            Format: P2WSH

            C45358FA: tpubDEkyN1rhRi4YTeUvpev7Fo9imQo9QNqjtDEYtZVtrVhCUmFfvACyi2czyccg31Aoad24vHCki4edpnVw6kkmJahyvBD35MHooGp7iSXcCjk
            73C5DA0A: tpubDFH9dgzveyD8zTbPUFuLrGmCydNvxehyNdUXKJAQN8x4aZ4j6UZqGfnqFrD4NqyaTVGKbvEW54tsvPTK2UoSbCC1PJY8iCNiwTL3RWZEheQ
            "#;

            let config = parse_wallet_config(config, "73C5DA0A").unwrap();
            let address = create_multi_sig_address(&config, 0, 0).unwrap();
            assert_eq!(
                "tb1qr7y0qr6uqyspjtst0sex8hyj3g47dfz0v5njs9x22kk6jzz3ee4qd5qrd7",
                address
            );
        }

        // Mixed path
        {
            let config = r#"# Coldcard Multisig setup file (exported from unchained-wallets)
            # https://github.com/unchained-capital/unchained-wallets
            # v1.0.0
            #
            Name: My Multisig test Wallet
            Policy: 2 of 3
            Format: P2WSH-P2SH

            Derivation: m/48'/0'/0'/1'/12/32/5
            748cc6aa: xpub6KMfgiWkVW33LfMbZoGjk6M3CvdZtrzkn38RP2SjbGGU9E85JTXDaX6Jn6bXVqnmq2EnRzWTZxeF3AZ1ZLcssM4DT9GY5RSuJBt1GRx3xm2
            Derivation: m/48'/0'/0'/1'/5/6/7
            5271c071: xpub6LfFMiP3hcgrKeTrho9MgKj2zdKGPsd6ufJzrsQNaHSFZ7uj8e1vnSwibBVQ33VfXYJM5zn9G7E9VrMkFPVcdRtH3Brg9ndHLJs8v2QtwHa
            Derivation: m/48'/1'/0'/2'
            73C5DA0A: xpub6EuX7TBEwhFgifQY24vFeMRqeWHGyGCupztDxk7G2ECAqGQ22Fik8E811p8GrM2LfajQzLidXy4qECxhdcxChkjiKhnq2fiVMVjdfSoZQwg
            "#;

            let config = parse_wallet_config(config, "73C5DA0A").unwrap();
            let address = create_multi_sig_address(&config, 0, 0).unwrap();
            assert_eq!("3A3vK8133WTMePMpPDmZSqSqK3gobohtG8", address);
        }
    }
}
