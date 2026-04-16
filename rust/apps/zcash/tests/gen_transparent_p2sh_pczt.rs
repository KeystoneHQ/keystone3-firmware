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

#[test]
fn gen_transparent_p2sh_pczt() {
    let params = MainNetwork;
    let rng = OsRng;

    let account_sk = AccountPrivKey::from_seed(&params, &[1u8; 32], zip32::AccountId::ZERO)
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
    let to = TransparentAddress::ScriptHash(p2sh_hash);

    builder
        .add_transparent_output(&to, Zatoshis::const_from_u64(200_000))
        .expect("add transparent output");
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

    let pczt = Creator::build_from_parts(pczt_parts).expect("creator build from parts");
    let pczt = Updater::new(pczt)
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

    let bytes = pczt.serialize();
    std::fs::write("/tmp/pczt_transparent_p2sh.hex", hex::encode(&bytes)).expect("write hex");

    println!("to={}", t3);
    println!("pczt_hex_path=/tmp/pczt_transparent_p2sh.hex");
    println!("transparent_inputs={}", pczt.transparent().inputs().len());
    println!("transparent_outputs={}", pczt.transparent().outputs().len());
    println!("orchard_actions={}", pczt.orchard().actions().len());
    println!("sapling_spends={}", pczt.sapling().spends().len());
    println!("sapling_outputs={}", pczt.sapling().outputs().len());
}
