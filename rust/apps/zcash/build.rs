use std::env;
use std::fs;
use std::io::Write;
use std::path::Path;

fn main() {
    let version_h = Path::new("../../../src/config/version.h");
    println!("cargo:rerun-if-changed={}", version_h.display());

    let contents = fs::read_to_string(version_h).unwrap_or_else(|e| {
        panic!(
            "Failed to read {}: {e}. \
             build.rs expects to run from rust/apps/zcash/ with the repo root three levels up.",
            version_h.display()
        )
    });

    let mut major: Option<u8> = None;
    let mut minor: Option<u8> = None;
    let mut build: Option<u8> = None;

    for line in contents.lines() {
        let line = line.trim();
        if let Some(val) = line.strip_prefix("#define SOFTWARE_VERSION_MAJOR ") {
            if !val.starts_with('(') {
                major = Some(val.trim().parse().expect("SOFTWARE_VERSION_MAJOR is not a valid u8"));
            }
        } else if let Some(val) = line.strip_prefix("#define SOFTWARE_VERSION_MINOR ") {
            minor = Some(val.trim().parse().expect("SOFTWARE_VERSION_MINOR is not a valid u8"));
        } else if let Some(val) = line.strip_prefix("#define SOFTWARE_VERSION_BUILD ") {
            build = Some(val.trim().parse().expect("SOFTWARE_VERSION_BUILD is not a valid u8"));
        }
    }

    let major = major.expect("SOFTWARE_VERSION_MAJOR not found in version.h");
    let minor = minor.expect("SOFTWARE_VERSION_MINOR not found in version.h");
    let build = build.expect("SOFTWARE_VERSION_BUILD not found in version.h");

    let out_dir = env::var("OUT_DIR").unwrap();
    let out_path = Path::new(&out_dir).join("version_generated.rs");
    let mut f = fs::File::create(&out_path).unwrap();
    writeln!(
        f,
        "/// Auto-generated from src/config/version.h — do not edit.\n\
         pub const KEYSTONE_FW_VERSION: Version = Version({major}, {minor}, {build});"
    )
    .unwrap();
}
