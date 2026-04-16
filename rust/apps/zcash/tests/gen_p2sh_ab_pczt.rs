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
        pczt::Bip32Derivation,
    },
    zcash_address::{ToAddress, ZcashAddress},
    zcash_protocol::{
        consensus::{MainNetwork, Parameters},
        value::Zatoshis,
    },
    zip32,
};

#[test]
fn gen_p2sh_ab_pczt() {
    let params = MainNetwork;
    let rng = OsRng;

    let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4")
        .expect("decode abandon-about seed hex");
    let account_sk = AccountPrivKey::from_seed(&params, &seed, zip32::AccountId::ZERO)
        .expect("account sk");
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
        value: Zatoshis::const_from_u64(300_000),
        script_pubkey: from_addr.script(),
    };

    let mut builder = Builder::new(
        &params,
        10_000_000.into(),
        BuildConfig::Standard {
            sapling_anchor: None,
            orchard_anchor: Some(orchard::Anchor::empty_tree()),
        },
    );

    builder
        .add_transparent_input(pubkey, transparent::OutPoint::fake(), prev_coin)
        .expect("add transparent input");

    let p2sh_hash = [0x11u8; 20];
    let to_p2sh = TransparentAddress::ScriptHash(p2sh_hash);

    builder
        .add_transparent_output(&to_p2sh, Zatoshis::const_from_u64(200_000))
        .expect("add transparent p2sh output");
    builder
        .add_transparent_output(&from_addr, Zatoshis::const_from_u64(90_000))
        .expect("add transparent change output");

    let PcztResult { pczt_parts, .. } = builder
        .build_for_pczt(rng, &zip317::FeeRule::standard())
        .expect("build for pczt");

    let t3 = ZcashAddress::from_transparent_p2sh(params.network_type(), p2sh_hash).encode();
    let from_pkh = match from_addr {
        TransparentAddress::PublicKeyHash(hash) => hash,
        _ => panic!("expected p2pkh source address"),
    };
    let t1_change = ZcashAddress::from_transparent_p2pkh(params.network_type(), from_pkh).encode();

    let base = Creator::build_from_parts(pczt_parts).expect("creator build from parts");

    let base = Updater::new(base)
        .update_transparent_with(|mut updater| {
            updater.update_output_with(0, |mut output| {
                output.set_user_address(t3.clone());
                Ok(())
            })?;
            updater.update_output_with(1, |mut output| {
                output.set_user_address(t1_change.clone());
                Ok(())
            })?;
            Ok(())
        })
        .expect("set transparent user_address")
        .finish();

    // A: no derivation on P2SH output
    std::fs::write(
        "/tmp/pczt_p2sh_ab_no_derivation.hex",
        hex::encode(base.serialize()),
    )
    .expect("write A");

    // B: add matching derivation on P2SH output
    let seed_fingerprint = keystore::algorithms::zcash::calculate_seed_fingerprint(&seed)
        .expect("seed fingerprint");
    let bip32 = Bip32Derivation::parse(
        seed_fingerprint,
        vec![
            44 | (1 << 31),
            133 | (1 << 31),
            0 | (1 << 31),
            0,
            0,
        ],
    )
    .expect("bip32 parse");

    let with_derivation = Updater::new(base)
        .update_transparent_with(|mut updater| {
            updater.update_output_with(0, |mut output| {
                output.set_bip32_derivation(pubkey.serialize(), bip32);
                Ok(())
            })?;
            Ok(())
        })
        .expect("set p2sh derivation")
        .finish();

    std::fs::write(
        "/tmp/pczt_p2sh_ab_with_derivation.hex",
        hex::encode(with_derivation.serialize()),
    )
    .expect("write B");

    println!("A=/tmp/pczt_p2sh_ab_no_derivation.hex");
    println!("B=/tmp/pczt_p2sh_ab_with_derivation.hex");
    println!("to={}", t3);
    println!("seed_fingerprint={}", hex::encode(seed_fingerprint));
}
