use std::fs;
use ur_parse_lib::keystone_ur_decoder::{probe_decode, MultiURParseResult, URParseResult};
use ur_registry::zcash::zcash_pczt::ZcashPczt;

fn main() {
    let txt = fs::read_to_string("/tmp/zcash_ur_parts_transparent_p2sh_strict2.txt")
        .expect("read strict2 parts");
    let parts: Vec<String> = txt
        .lines()
        .map(|s| s.trim().to_string())
        .filter(|s| !s.is_empty())
        .collect();

    assert!(parts.len() >= 2, "need at least 2 parts");

    let first: URParseResult<ZcashPczt> = probe_decode(parts[0].clone()).expect("probe first");
    println!("first_is_multi={} progress={}", first.is_multi_part, first.progress);

    if !first.is_multi_part {
        println!("single part decoded unexpectedly");
        return;
    }

    let mut decoder = first.decoder.expect("decoder from first part");
    let second: MultiURParseResult<ZcashPczt> = decoder
        .parse_ur(parts[1].clone())
        .expect("parse second part");

    println!("second_complete={} progress={}", second.is_complete, second.progress);
    if let Some(data) = second.data {
        println!("decoded_pczt_bytes={}", data.get_data().len());
    } else {
        println!("decoded data missing");
    }
}
