use alloc::string::{String, ToString};
use alloc::vec::Vec;
use third_party::bitcoin::{opcodes, script, PublicKey, ScriptBuf};
use third_party::itertools::Itertools;

use crate::addresses::address::Address;
use crate::addresses::xyzpub::Version;
use crate::addresses::{derive_public_key, xyzpub};
use crate::multi_sig::wallet::MultiSigWalletConfig;
use crate::multi_sig::{MultiSigFormat, Network};
use crate::{network, BitcoinError};

pub fn create_multi_sig_address_for_wallet(
    wallet: &MultiSigWalletConfig,
    change: u32,
    account: u32,
) -> Result<String, BitcoinError> {
    let format = MultiSigFormat::from(&wallet.format)?;
    let pub_keys = wallet
        .xpub_items
        .iter()
        .map(|x| {
            let convert_xpub = xyzpub::convert_version(&x.xpub, &Version::Xpub)?;
            derive_pub_key(&convert_xpub, change, account)
        })
        .collect::<Result<Vec<_>, _>>()?;

    let ordered_pub_keys = pub_keys.iter().sorted().collect::<Vec<_>>();

    let p2ms = crate_p2ms_script(&ordered_pub_keys, wallet.threshold);

    calculate_multi_address(&p2ms, format, wallet.get_network())
}

pub fn calculate_multi_address(
    p2ms: &ScriptBuf,
    format: MultiSigFormat,
    network: &Network,
) -> Result<String, BitcoinError> {
    let script = match format {
        MultiSigFormat::P2sh => ScriptBuf::new_p2sh(&p2ms.script_hash()),
        MultiSigFormat::P2wshP2sh => {
            let p2wsh = ScriptBuf::new_p2wsh(&p2ms.wscript_hash());
            ScriptBuf::new_p2sh(&p2wsh.script_hash())
        }
        MultiSigFormat::P2wsh => ScriptBuf::new_p2wsh(&p2ms.wscript_hash()),
    };

    let network = if *network == Network::TestNet {
        network::Network::BitcoinTestnet
    } else {
        network::Network::Bitcoin
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

    use crate::multi_sig::address::create_multi_sig_address_for_wallet;
    use crate::multi_sig::wallet::parse_wallet_config;
    use crate::multi_sig::Network;

    #[test]
    fn test_create_multi_sig_address_for_wallet() {
        // P2SH
        {
            let config = r#"# Keystone Multisig setup file (created by Sparrow)
            #
            Name: testnet1
            Policy: 2 of 2
            Derivation: m/45'
            Format: P2SH

            C45358FA: xpub69LCBLAWzswdEpR3NrkKLYfE4o14hR5MxbKQbnR5aAjZvNZLc1ein6v1hdQZg59bLapjKb6XH9r7wQF5NUW6D6EFniSYESszdT5bXrn35WE
            73C5DA0A: xpub68jrRzQopSUSiczuqjRwvVn3CFtSEZY6a3jbT66LM3tvt1rXtYT7Udi8dt3m1qj3q8pKZjt7tqrSt7bRN4LD2vSVq1167PSA5AyM31FUHwU
            "#;

            let config = parse_wallet_config(config, "73C5DA0A").unwrap();
            let address = create_multi_sig_address_for_wallet(&config, 0, 0).unwrap();
            assert_eq!("3HzJmPbAqp1mdd6Xj4e5L6PpYSQb35WiWp", address);
        }

        // P2WSH-P2SH
        {
            let config = r#"# Keystone Multisig setup file (created by Sparrow)
            #
            Name: testnet1
            Policy: 2 of 2
            Derivation: m/48'/1'/0'/1'
            Format: P2WSH-P2SH

            C45358FA: xpub6EPLqn31iS769Y2jsFSHgTo9Nih2fKjyjvLm2pcPa2RoEytW9LdrdFXfyxHuayyjG75veoY6dhHdKETpEYWGcSgpEB1fC3BKL5VDGcyrEAq
            73C5DA0A: xpub6EuX7TBEwhFghVDjgJsj4CvAqnqxLxnJC1Rtr4Y3T6VvpzKqRfTZQ7HnJeM3uxwLJ1MZeKMGFHZFAxhRCyxiyDWng7pnYQzvBsMSdmog9kW
            "#;

            let config = parse_wallet_config(config, "73C5DA0A").unwrap();
            let address = create_multi_sig_address_for_wallet(&config, 0, 0).unwrap();
            assert_eq!("3FZ2KV6x4BwzDFaPhLaJnAMJ284LGAsKV6", address);
        }

        // P2WSH
        {
            let config = r#"# Keystone Multisig setup file (created by Sparrow)
            #
            Name: testnet1
            Policy: 2 of 2
            Derivation: m/48'/1'/0'/2'
            Format: P2WSH

            C45358FA: xpub6EPLqn31iS76BrJ5NTw23spMSHhVQzLgLaeFY1SkWawJjUaxqwMtZaxAjaXtWWDZniVAJhgtAxpb8c1KhtuXR9FgraTjPpdaDKDhxSXRk4R
            73C5DA0A: xpub6EuX7TBEwhFgifQY24vFeMRqeWHGyGCupztDxk7G2ECAqGQ22Fik8E811p8GrM2LfajQzLidXy4qECxhdcxChkjiKhnq2fiVMVjdfSoZQwg
            "#;

            let config = parse_wallet_config(config, "73C5DA0A").unwrap();
            let address = create_multi_sig_address_for_wallet(&config, 0, 0).unwrap();
            assert_eq!(
                "bc1qr7y0qr6uqyspjtst0sex8hyj3g47dfz0v5njs9x22kk6jzz3ee4q6ukvh3",
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
            let address = create_multi_sig_address_for_wallet(&config, 0, 0).unwrap();
            assert_eq!("3A3vK8133WTMePMpPDmZSqSqK3gobohtG8", address);
        }
    }
}
