use std::fs;

use keystore::algorithms::zcash::{calculate_seed_fingerprint, derive_ufvk};
use ur_parse_lib::keystone_ur_decoder::{probe_decode, MultiURParseResult, URParseResult};
use ur_registry::zcash::zcash_pczt::ZcashPczt;
use zcash_vendor::{pczt::Pczt, zcash_protocol::consensus::MainNetwork};

fn decode_ur_parts(parts_file: &str) -> Result<Vec<u8>, String> {
    let txt = fs::read_to_string(parts_file).map_err(|e| format!("read parts file failed: {e}"))?;
    let parts: Vec<String> = txt
        .lines()
        .map(|s| s.trim().to_string())
        .filter(|s| !s.is_empty())
        .collect();

    if parts.is_empty() {
        return Err("parts file is empty".to_string());
    }

    let first: URParseResult<ZcashPczt> = probe_decode(parts[0].clone())
        .map_err(|e| format!("probe first part failed: {e:?}"))?;

    if !first.is_multi_part {
        let data = first
            .data
            .ok_or_else(|| "single-part decode returned no data".to_string())?;
        return Ok(data.get_data());
    }

    let mut decoder = first
        .decoder
        .ok_or_else(|| "multi-part decode returned no decoder".to_string())?;

    let mut last_data: Option<ZcashPczt> = None;
    for part in parts.iter().skip(1) {
        let r: MultiURParseResult<ZcashPczt> = decoder
            .parse_ur(part.clone())
            .map_err(|e| format!("parse part failed: {e:?}"))?;
        if r.is_complete {
            last_data = r.data;
            break;
        }
    }

    let data = last_data.ok_or_else(|| "decoder did not complete with given parts".to_string())?;
    Ok(data.get_data())
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let parts_file = args
        .get(1)
        .map(|s| s.as_str())
        .unwrap_or("/tmp/zcash_ur_parts_transparent_p2sh_len80_clean.txt");

    let decoded_pczt = match decode_ur_parts(parts_file) {
        Ok(v) => v,
        Err(e) => {
            eprintln!("decode failed: {e}");
            std::process::exit(1);
        }
    };

    let original_hex = fs::read_to_string("/tmp/pczt_transparent_p2sh.hex")
        .expect("read /tmp/pczt_transparent_p2sh.hex");
    let original_pczt = hex::decode(original_hex.trim()).expect("decode original pczt hex");

    println!("decoded_bytes={}", decoded_pczt.len());
    println!("original_bytes={}", original_pczt.len());
    println!("bytes_equal={}", decoded_pczt == original_pczt);

    let _pczt = Pczt::parse(&decoded_pczt).expect("Pczt::parse from decoded bytes");
    println!("pczt_parse_ok=true");

    let seed = [1u8; 32];
    let ufvk = derive_ufvk(&MainNetwork, &seed, "m/32'/133'/0'").expect("derive ufvk");
    let fp = calculate_seed_fingerprint(&seed).expect("seed fingerprint");

    let parsed = app_zcash::parse_pczt_cypherpunk(&MainNetwork, &decoded_pczt, &ufvk, &fp)
        .expect("app_zcash parse pczt");

    println!("has_sapling={}", parsed.get_has_sapling());
    println!("total_transfer={}", parsed.get_total_transfer_value());
    println!("fee={}", parsed.get_fee_value());

    if let Some(t) = parsed.get_transparent() {
        println!("transparent_from_count={}", t.get_from().len());
        println!("transparent_to_count={}", t.get_to().len());
        for (i, to) in t.get_to().iter().enumerate() {
            println!(
                "to[{i}] addr={} amount={} change={} dummy={}",
                to.get_address(),
                to.get_value(),
                to.get_is_change(),
                to.get_is_dummy()
            );
        }
    } else {
        println!("transparent_section=none");
    }

    println!("validation=ok");
}
