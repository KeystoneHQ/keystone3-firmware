use alloc::{string::ToString, vec::Vec};

use zcash_vendor::{
    bip32, secp256k1,
    transparent::address::TransparentAddress,
    zcash_address::ZcashAddress,
    zcash_protocol::consensus::{Network, Parameters},
    zcash_script::{descriptor, script},
};

use crate::{
    multi_sig::{
        wallet::{convert_version, MultiSigWalletConfig},
        MultiSigFormat,
    },
    ZcashError,
};

pub fn create_multi_sig_address_for_wallet(
    wallet: &MultiSigWalletConfig,
    change: u32,
    account: u32,
) -> Result<ZcashAddress, ZcashError> {
    let format = MultiSigFormat::from(&wallet.format)?;
    let pub_keys = wallet
        .xpub_items
        .iter()
        .map(|x| {
            let (xfp, xpub) = x.clone().into_parts();
            let convert_xpub = convert_version(xpub, bip32::Prefix::XPUB)?;
            let (prefix, key, child) = derive_pub_key(&convert_xpub, change, account)?;
            descriptor::KeyExpression::from_xpub(xfp, prefix, key, child).ok_or_else(|| {
                ZcashError::MultiSigWalletAddressCalError("xpub isn’t valid".to_string())
            })
        })
        .collect::<Result<Vec<_>, _>>()?;

    let p2ms = create_p2ms_script(&pub_keys, wallet.threshold)?;

    calculate_multi_address(&p2ms, format, wallet.get_network())
}

/// __NB__: Unlike the Bitcoin implementation, Zcash always sorts new multi_sig addresses.
pub fn create_multi_sig_address_for_pubkeys(
    threshold: u8,
    pub_keys: &[descriptor::KeyExpression],
    format: MultiSigFormat,
    network: &Network,
) -> Result<ZcashAddress, ZcashError> {
    let p2ms = create_p2ms_script(&pub_keys, threshold)?;

    calculate_multi_address(&p2ms, format, network)
}

pub fn calculate_multi_address(
    p2ms: &script::Redeem,
    format: MultiSigFormat,
    network: &Network,
) -> Result<ZcashAddress, ZcashError> {
    let script = match format {
        MultiSigFormat::P2sh => descriptor::sh(&p2ms),
    };

    TransparentAddress::from_script_pubkey(&script)
        .map(|taddr| taddr.to_zcash_address(network.network_type()))
        .ok_or(ZcashError::GenerateAddressError(
            "unrecognized script".to_string(),
        ))
}

fn derive_pub_key(
    xpub: &descriptor::Key,
    change: u32,
    account: u32,
) -> Result<
    (
        bip32::Prefix,
        bip32::ExtendedPublicKey<secp256k1::PublicKey>,
        Vec<bip32::ChildNumber>,
    ),
    ZcashError,
> {
    match xpub {
        descriptor::Key::Public { .. } => Err(ZcashError::MultiSigWalletAddressCalError(
            "key is not xpub".to_string(),
        )),
        descriptor::Key::Xpub { prefix, key, child } => Ok((*prefix, key.clone(), child.clone())),
    }
}

fn create_p2ms_script(
    pub_keys: &[descriptor::KeyExpression],
    threshold: u8,
) -> Result<script::Redeem, ZcashError> {
    descriptor::sortedmulti(threshold, pub_keys).map_err(ZcashError::MultiSigWalletCreateError)
}

#[cfg(test)]
mod tests {
    extern crate std;

    use alloc::{string::ToString, vec::Vec};

    use crate::multi_sig::{
        address::{create_multi_sig_address_for_pubkeys, create_multi_sig_address_for_wallet},
        wallet::{fixed_xfp, parse_wallet_config},
        MultiSigFormat, Network, ZcashError,
    };

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

            let config = parse_wallet_config(config, &fixed_xfp([0x73, 0xc5, 0xda, 0x0a])).unwrap();
            let address = create_multi_sig_address_for_wallet(&config, 0, 0).unwrap();
            assert_eq!("t3hsyk4WWFS9X2LQsNPdEkVa7wNLV23e9Ps", address.encode());
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

            let config = parse_wallet_config(config, &fixed_xfp([0x73, 0xc5, 0xda, 0x0a]));
            assert_eq!(
                Err(ZcashError::MultiSigUnsupportedWalletFormat(
                    "P2WSH-P2SH".to_string()
                )),
                config.map(|_| ()),
            );
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

            let config = parse_wallet_config(config, &fixed_xfp([0x73, 0xc5, 0xda, 0x0a]));
            assert_eq!(
                Err(ZcashError::MultiSigUnsupportedWalletFormat(
                    "P2WSH".to_string()
                )),
                config.map(|_| ()),
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

            let config = parse_wallet_config(config, &fixed_xfp([0x73, 0xc5, 0xda, 0x0a]));
            assert_eq!(
                Err(ZcashError::MultiSigUnsupportedWalletFormat(
                    "P2WSH-P2SH".to_string()
                )),
                config.map(|_| ()),
            );
        }
    }

    #[test]
    fn test_create_multi_sig_address_for_pubkeys() {
        let pubkey_str = vec![
            "03a0c95fd48f1a251c744629e19ad154dfe1d7fb992d6955d62c417ae4ac333340",
            "0361769c55b3035962fd3267da5cc4efa03cb400fe1971f5ec1c686d6b301ccd60",
            "021d24a7eda6ccbff4616d9965c9bb2a7871ce048b0161b71e91be83671be514d5",
        ];
        let pubkeys = pubkey_str
            .iter()
            .map(|s| s.parse())
            .collect::<Result<Vec<_>, _>>()
            .unwrap();
        let address = create_multi_sig_address_for_pubkeys(
            2,
            &pubkeys,
            MultiSigFormat::P2sh,
            &Network::MainNetwork,
        )
        .unwrap();
        assert_eq!("t3KHpYivED722kBUA1DsMso524LCksSMaox", address.encode());
    }
}
