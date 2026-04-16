use keystore::algorithms::zcash::calculate_seed_fingerprint;

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.len() < 2 {
        eprintln!("usage: calc_zcash_fp <seed_hex>");
        std::process::exit(1);
    }

    let seed = hex::decode(args[1].trim()).expect("invalid seed hex");
    let fp = calculate_seed_fingerprint(&seed).expect("calculate seed fingerprint failed");
    println!("seed_len={}", seed.len());
    println!("fingerprint={}", hex::encode(fp));
}
