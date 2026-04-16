use std::env;
use std::fs;

use ur_registry::traits::RegistryItem;
use ur_registry::zcash::zcash_pczt::ZcashPczt;

fn read_hex_arg(raw: &str) -> Result<String, String> {
    if raw.starts_with('@') {
        let path = raw.trim_start_matches('@');
        fs::read_to_string(path).map_err(|e| format!("failed to read input file: {e}"))
    } else {
        Ok(raw.to_string())
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        eprintln!("Usage: zcash_pczt_to_ur <pczt_hex_or_@file> [fragment_len]");
        std::process::exit(1);
    }

    let hex_raw = match read_hex_arg(&args[1]) {
        Ok(v) => v,
        Err(e) => {
            eprintln!("{e}");
            std::process::exit(2);
        }
    };

    let fragment_len = args
        .get(2)
        .and_then(|v| v.parse::<usize>().ok())
        .unwrap_or(200);

    let pczt_bytes = match hex::decode(hex_raw.trim()) {
        Ok(v) => v,
        Err(e) => {
            eprintln!("hex decode failed: {e}");
            std::process::exit(3);
        }
    };

    let data: Vec<u8> = match ZcashPczt::new(pczt_bytes).try_into() {
        Ok(v) => v,
        Err(e) => {
            eprintln!("zcash-pczt CBOR encode failed: {e}");
            std::process::exit(4);
        }
    };

    let ur_type = ZcashPczt::get_registry_type().get_type();
    let encoded = match ur_parse_lib::keystone_ur_encoder::probe_encode(&data, fragment_len, ur_type)
    {
        Ok(v) => v,
        Err(e) => {
            eprintln!("UR encode failed: {e}");
            std::process::exit(5);
        }
    };

    if encoded.is_multi_part {
        println!("# multipart");
        println!("{}", encoded.data.to_uppercase());

        let mut encoder = encoded.encoder.expect("encoder missing in multipart mode");
        for _ in 0..2000 {
            match encoder.next_part() {
                Ok(part) => println!("{}", part.to_uppercase()),
                Err(_) => break,
            }
        }
    } else {
        println!("# single");
        println!("{}", encoded.data.to_uppercase());
    }
}
