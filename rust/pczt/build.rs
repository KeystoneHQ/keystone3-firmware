fn main() -> Result<(), std::io::Error> {
    prost_build::Config::new().out_dir("src/protos").compile_protos(&["src/protos/pczt.proto"], &["src/"])?;
    Ok(())
}
