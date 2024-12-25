fn main() {
    return;
    let mut config = cbindgen::Config::default();
    config.language = cbindgen::Language::C;
    let includes = vec![
        "common_rust_c",
        "wallet_rust_c",
        "btc_only_wallet_rust_c",
        "multi_coins_wallet_rust_c",
        #[cfg(feature = "aptos")]
        "aptos_rust_c",
        #[cfg(feature = "simulator")]
        "simulator_rust_c",
        #[cfg(feature = "arweave")]
        "arweave_rust_c",
        #[cfg(feature = "bitcoin")]
        "bitcoin_rust_c",
        #[cfg(feature = "cardano")]
        "cardano_rust_c",
        #[cfg(feature = "cosmos")]
        "cosmos_rust_c",
        #[cfg(feature = "ethereum")]
        "ethereum_rust_c",
        #[cfg(feature = "near")]
        "near_rust_c",
        #[cfg(feature = "solana")]
        "solana_rust_c",
        #[cfg(feature = "stellar")]
        "stellar_rust_c",
        #[cfg(feature = "sui")]
        "sui_rust_c",
        #[cfg(feature = "ton")]
        "ton_rust_c",
        #[cfg(feature = "tron")]
        "tron_rust_c",
        #[cfg(feature = "xrp")]
        "xrp_rust_c",
        #[cfg(feature = "zcash")]
        "zcash_rust_c",
        #[cfg(feature = "test_cmd")]
        "test_cmd_rust_c",
        #[cfg(feature = "test_cmd")]
        "btc_test_cmd_rust_c",
        #[cfg(feature = "test_cmd")]
        "general_test_cmd_rust_c",
    ];

    let crates = includes
        .into_iter()
        .map(|s| s.to_string())
        .collect::<Vec<String>>();

    config.parse.include = Some(crates.clone());
    config.parse.parse_deps = true;
    config.parse.expand.crates = crates;
    let features = vec![
        #[cfg(feature = "btc-only")]
        "btc_only",
        #[cfg(feature = "multi-coins")]
        "multi_coins",
    ];

    config.parse.expand.features = Some(features.into_iter().map(|s| s.to_string()).collect());

    let builder = cbindgen::Builder::new();

    builder
        .with_crate(".")
        .with_config(config)
        .generate()
        .expect("Failed to generate bindings")
        .write_to_file("librust_c.h");
}
