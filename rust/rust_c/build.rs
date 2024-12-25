use std::env;

fn main() {
    let mut config =
        cbindgen::Config::from_file("cbindgen.toml").expect("Failed to read cbindgen.toml");
    let features: Vec<&str> = vec![
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
    assert!(features.len() > 0, "No build variant enabled");
    assert!(features.len() == 1, "Multiple build variants enabled: {:?}", features);
    let output_target = env::var("CBINDGEN_BINDINGS_TARGET").unwrap_or(format!("bindings/{}/librust_c.h", features[0]));
    config.parse.expand.features = Some(features.into_iter().map(|s| s.to_string()).collect());

    let builder = cbindgen::Builder::new();

    builder
        .with_crate(".")
        .with_config(config)
        .generate()
        .expect("Failed to generate bindings")
        .write_to_file(output_target);
}
