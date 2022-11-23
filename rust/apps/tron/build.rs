fn main() {
    #[cfg(feature = "generate_pb")]
    prost_build::Config::new()
        .btree_map(&["."])
        .out_dir("src/pb")
        .compile_protos(&["tron.proto", "contract.proto"], &["./proto/"])
        .unwrap();
}
