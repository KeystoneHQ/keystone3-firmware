fn main() {
    let mut config =
        cbindgen::Config::from_file("cbindgen.toml").expect("Failed to read cbindgen.toml");
    let features = vec![
        #[cfg(feature = "production-multi-coins")]
        "production-multi-coins",
        #[cfg(feature = "production-btc-only")]
        "production-btc-only",
        #[cfg(feature = "simulator-multi-coins")]
        "simulator-multi-coins",
        #[cfg(feature = "simulator-btc-only")]
        "simulator-btc-only",
        #[cfg(feature = "debug-multi-coins")]
        "debug-multi-coins",
        #[cfg(feature = "debug-btc-only")]
        "debug-btc-only",
    ];
    println!("cargo:warning={}", format!("features: {:?}", features));
    config.parse.expand.features = Some(features.into_iter().map(|s| s.to_string()).collect());

    let builder = cbindgen::Builder::new();

    builder
        .with_crate(".")
        .with_config(config)
        .generate()
        .expect("Failed to generate bindings")
        .write_to_file("librust_c.h");
}
