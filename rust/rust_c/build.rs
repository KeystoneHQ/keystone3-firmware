use std::env;

fn main() {
    let mut config =
        cbindgen::Config::from_file("cbindgen.toml").expect("Failed to read cbindgen.toml");
    let features: Vec<&str> = vec![
        #[cfg(feature = "production-multi-coins")]
        "production-multi-coins",
        #[cfg(feature = "simulator-multi-coins")]
        "simulator-multi-coins",
        #[cfg(feature = "debug-multi-coins")]
        "debug-multi-coins",
    ];

    //feature toggle
    config.after_includes = config.after_includes.map(|mut v| {
        #[cfg(feature = "multi-coins")]
        v.push_str("#define BUILD_MULTI_COINS\n");

        #[cfg(feature = "aptos")]
        v.push_str("#define FEATURE_APTOS\n");
        #[cfg(feature = "arweave")]
        v.push_str("#define FEATURE_ARWEAVE\n");
        #[cfg(feature = "bch")]
        v.push_str("#define FEATURE_BCH\n");
        #[cfg(feature = "bitcoin")]
        v.push_str("#define FEATURE_BITCOIN\n");
        #[cfg(feature = "cardano")]
        v.push_str("#define FEATURE_CARDANO\n");
        #[cfg(feature = "cosmos")]
        v.push_str("#define FEATURE_COSMOS\n");
        #[cfg(feature = "dash")]
        v.push_str("#define FEATURE_DASH\n");
        #[cfg(feature = "ethereum")]
        v.push_str("#define FEATURE_ETHEREUM\n");
        #[cfg(feature = "ltc")]
        v.push_str("#define FEATURE_LTC\n");
        #[cfg(feature = "doge")]
        v.push_str("#define FEATURE_DOGE\n");
        #[cfg(feature = "near")]
        v.push_str("#define FEATURE_NEAR\n");
        #[cfg(feature = "solana")]
        v.push_str("#define FEATURE_SOLANA\n");
        #[cfg(feature = "stellar")]
        v.push_str("#define FEATURE_STELLAR\n");
        #[cfg(feature = "ton")]
        v.push_str("#define FEATURE_TON\n");
        #[cfg(feature = "tron")]
        v.push_str("#define FEATURE_TRON\n");
        #[cfg(feature = "xrp")]
        v.push_str("#define FEATURE_XRP\n");
        #[cfg(feature = "avalanche")]
        v.push_str("#define FEATURE_AVAX\n");
        #[cfg(feature = "iota")]
        v.push_str("#define FEATURE_IOTA\n");
        #[cfg(feature = "zcash")]
        v.push_str("#define FEATURE_ZCASH\n");
        #[cfg(feature = "monero")]
        v.push_str("#define FEATURE_MONERO\n");
        v
    });
    assert!(!features.is_empty(), "No build variant enabled");
    assert!(
        features.len() == 1,
        "Multiple build variants enabled: {:?}",
        features
    );
    let output_target = env::var("CBINDGEN_BINDINGS_TARGET")
        .unwrap_or(format!("bindings/{}/librust_c.h", features[0]));
    config.parse.expand.features = Some(features.into_iter().map(|s| s.to_string()).collect());

    let builder = cbindgen::Builder::new();

    builder
        .with_crate(".")
        .with_config(config)
        .generate()
        .expect("Failed to generate bindings")
        .write_to_file(output_target);
}
