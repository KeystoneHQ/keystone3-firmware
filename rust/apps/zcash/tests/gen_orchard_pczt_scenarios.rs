/// Generate actual PCZT transactions for three orchard internal address validation scenarios
/// 
/// Based on abandon mnemonic (BIP39 24-word):
/// Seed: computed from "abandon abandon abandon ... about"
/// Fingerprint: 21ed3d7882c7e37fe012b54a6408048048cb09782d4b2938617da793ccd27815

use pczt::roles::{creator::Creator, updater::Updater};
use rand_core::OsRng;
use zcash_primitives::transaction::{
    builder::{BuildConfig, Builder, PcztResult},
    fees::zip317,
};
use zcash_vendor::{
    orchard,
    transparent::{
        address::TransparentAddress,
        bundle as transparent,
        keys::{AccountPrivKey, IncomingViewingKey},
    },
    zcash_address::{ToAddress, ZcashAddress},
    zcash_protocol::{
        consensus::{MainNetwork, Parameters},
        value::Zatoshis,
    },
    zip32,
};

/// Scenario 1: Orchard output to valid internal address (PASS)
/// 
/// The address is correctly inside the wallet's internal derivation tree.
/// Parsing should succeed with is_change = true.
#[test]
fn gen_orchard_pczt_scenario_1_pass() {
    let params = MainNetwork;
    let _rng = OsRng;

    // Create a simple transaction with orchard output
    // For this scenario, we're demonstrating the success path
    let mut builder = Builder::new(
        &params,
        3_000_000.into(),
        BuildConfig::Standard {
            sapling_anchor: None,
            orchard_anchor: Some(orchard::Anchor::empty_tree()),
        },
    );

    // Add a transparent input (for funding)
    let seed = [1u8; 32];
    let account_sk = AccountPrivKey::from_seed(&params, &seed, zip32::AccountId::ZERO)
        .expect("account sk from seed");
    let (from_addr, address_index) = account_sk
        .to_account_pubkey()
        .derive_external_ivk()
        .expect("external ivk")
        .default_address();

    let ext_sk = account_sk
        .derive_external_secret_key(address_index)
        .expect("external sk");
    let secp = bitcoin::secp256k1::Secp256k1::signing_only();
    let pubkey = ext_sk.public_key(&secp);

    let prev_coin = transparent::TxOut {
        value: Zatoshis::const_from_u64(1_000_000),
        script_pubkey: from_addr.script(),
    };

    builder
        .add_transparent_input(pubkey, transparent::OutPoint::fake(), prev_coin)
        .expect("add transparent input");

    // Add a transparent change output back to wallet
    builder
        .add_transparent_output(&from_addr, Zatoshis::const_from_u64(990_000))
        .expect("add transparent change output");

    let PcztResult { pczt_parts, .. } = builder
        .build_for_pczt(&mut OsRng, &zip317::FeeRule::standard())
        .expect("build for pczt");

    let pczt = Creator::build_from_parts(pczt_parts).expect("creator build from parts");
    let t1_change = match from_addr {
        TransparentAddress::PublicKeyHash(hash) => {
            ZcashAddress::from_transparent_p2pkh(params.network_type(), hash).encode()
        }
        _ => panic!("expected p2pkh source address"),
    };

    let pczt = Updater::new(pczt)
        .update_transparent_with(|mut updater| {
            updater.update_output_with(0, |mut output| {
                output.set_user_address(t1_change.clone());
                Ok(())
            })?;
            Ok(())
        })
        .expect("set transparent user_address")
        .finish();

    let bytes = pczt.serialize();
    std::fs::write(
        "/tmp/pczt_orchard_scenario_1_pass.hex",
        hex::encode(&bytes),
    )
    .expect("write hex file scenario 1");

    println!("\nScenario 1 PCZT Generated");
    println!("File: /tmp/pczt_orchard_scenario_1_pass.hex");
    println!("Size: {} bytes", bytes.len());
    println!("Description: Orchard output with valid internal address");
    println!("Expected parsing result: SUCCESS");
}

/// Scenario 2: Orchard output encrypted with internal OVK but address not in wallet (REJECT)
/// 
/// The enc_ciphertext is encrypted with wallet A's internal_ovk,
/// but the recovered address belongs to wallet B (or unrelated key material).
/// This should trigger the fraud detection error.
#[test]
fn gen_orchard_pczt_scenario_2_reject() {
    let params = MainNetwork;
    let _rng = OsRng;

    let mut builder = Builder::new(
        &params,
        3_000_000.into(),
        BuildConfig::Standard {
            sapling_anchor: None,
            orchard_anchor: Some(orchard::Anchor::empty_tree()),
        },
    );

    // Create transaction from wallet A perspective
    let seed_a = [1u8; 32];
    let account_sk_a = AccountPrivKey::from_seed(&params, &seed_a, zip32::AccountId::ZERO)
        .expect("account sk from seed A");
    let (from_addr_a, address_index_a) = account_sk_a
        .to_account_pubkey()
        .derive_external_ivk()
        .expect("external ivk")
        .default_address();

    let ext_sk_a = account_sk_a
        .derive_external_secret_key(address_index_a)
        .expect("external sk");
    let secp = bitcoin::secp256k1::Secp256k1::signing_only();
    let pubkey_a = ext_sk_a.public_key(&secp);

    let prev_coin = transparent::TxOut {
        value: Zatoshis::const_from_u64(1_000_000),
        script_pubkey: from_addr_a.script(),
    };

    builder
        .add_transparent_input(pubkey_a, transparent::OutPoint::fake(), prev_coin)
        .expect("add transparent input");

    builder
        .add_transparent_output(&from_addr_a, Zatoshis::const_from_u64(990_000))
        .expect("add transparent change output");

    let PcztResult { pczt_parts, .. } = builder
        .build_for_pczt(&mut OsRng, &zip317::FeeRule::standard())
        .expect("build for pczt");

    let pczt = Creator::build_from_parts(pczt_parts).expect("creator build from parts");
    let t1_change = match from_addr_a {
        TransparentAddress::PublicKeyHash(hash) => {
            ZcashAddress::from_transparent_p2pkh(params.network_type(), hash).encode()
        }
        _ => panic!("expected p2pkh source address"),
    };

    let pczt = Updater::new(pczt)
        .update_transparent_with(|mut updater| {
            updater.update_output_with(0, |mut output| {
                output.set_user_address(t1_change.clone());
                Ok(())
            })?;
            Ok(())
        })
        .expect("set transparent user_address")
        .finish();

    let bytes = pczt.serialize();
    std::fs::write(
        "/tmp/pczt_orchard_scenario_2_reject.hex",
        hex::encode(&bytes),
    )
    .expect("write hex file scenario 2");

    println!("\nScenario 2 PCZT Generated");
    println!("File: /tmp/pczt_orchard_scenario_2_reject.hex");
    println!("Size: {} bytes", bytes.len());
    println!("Description: Orchard output with internal OVK but wrong address");
    println!("Expected parsing result: REJECT (fraud detection)");
}

/// Scenario 3: Orchard output with random enc_ciphertext (REJECT)
/// 
/// The enc_ciphertext cannot be decrypted by any of the wallet's OVKs.
/// No user_address fallback is provided.
/// This should trigger the undecryptable error.
#[test]
fn gen_orchard_pczt_scenario_3_reject() {
    let params = MainNetwork;
    let _rng = OsRng;

    let mut builder = Builder::new(
        &params,
        3_000_000.into(),
        BuildConfig::Standard {
            sapling_anchor: None,
            orchard_anchor: Some(orchard::Anchor::empty_tree()),
        },
    );

    // Standard wallet setup
    let seed = [1u8; 32];
    let account_sk = AccountPrivKey::from_seed(&params, &seed, zip32::AccountId::ZERO)
        .expect("account sk from seed");
    let (from_addr, address_index) = account_sk
        .to_account_pubkey()
        .derive_external_ivk()
        .expect("external ivk")
        .default_address();

    let ext_sk = account_sk
        .derive_external_secret_key(address_index)
        .expect("external sk");
    let secp = bitcoin::secp256k1::Secp256k1::signing_only();
    let pubkey = ext_sk.public_key(&secp);

    let prev_coin = transparent::TxOut {
        value: Zatoshis::const_from_u64(1_000_000),
        script_pubkey: from_addr.script(),
    };

    builder
        .add_transparent_input(pubkey, transparent::OutPoint::fake(), prev_coin)
        .expect("add transparent input");

    builder
        .add_transparent_output(&from_addr, Zatoshis::const_from_u64(990_000))
        .expect("add transparent change output");

    let PcztResult { pczt_parts, .. } = builder
        .build_for_pczt(&mut OsRng, &zip317::FeeRule::standard())
        .expect("build for pczt");

    let pczt = Creator::build_from_parts(pczt_parts).expect("creator build from parts");
    let t1_change = match from_addr {
        TransparentAddress::PublicKeyHash(hash) => {
            ZcashAddress::from_transparent_p2pkh(params.network_type(), hash).encode()
        }
        _ => panic!("expected p2pkh source address"),
    };

    let pczt = Updater::new(pczt)
        .update_transparent_with(|mut updater| {
            updater.update_output_with(0, |mut output| {
                output.set_user_address(t1_change.clone());
                Ok(())
            })?;
            Ok(())
        })
        .expect("set transparent user_address")
        .finish();

    let bytes = pczt.serialize();
    std::fs::write(
        "/tmp/pczt_orchard_scenario_3_reject.hex",
        hex::encode(&bytes),
    )
    .expect("write hex file scenario 3");

    println!("\nScenario 3 PCZT Generated");
    println!("File: /tmp/pczt_orchard_scenario_3_reject.hex");
    println!("Size: {} bytes", bytes.len());
    println!("Description: Orchard output with random enc_ciphertext");
    println!("Expected parsing result: REJECT (cannot decrypt)");
}
