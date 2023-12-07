mod error;

use crate::error::OTAError; 
use clap::Parser;
use std::io::{Write, Read};
use std::fs::File;
use bytes::{ Bytes, BytesMut, BufMut};
use quicklz::{compress, CompressionLevel};
use crc32fast::Hasher;
use serde::{Serialize};
use serde_json::to_vec;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about=None)]
struct Args {
    #[arg(short, long)]
    source: String,
    
    #[arg(short, long)]
    destination:String,
}

fn main() -> std::io::Result<()>{
    
    let args = Args::parse();

    let source_path = args.source;

    let target_path = args.destination;

    let result = reader_file(&source_path).unwrap();

    let mut file  = File::create(target_path)?;

    file.write_all(&result)?;

    Ok(())

}
#[derive(Serialize)]
struct Header {
    mark: String,
    fileSize: u32,
    originalFileSize: u32,
    crc32: u32,
    originalCrc32: u32,
    encode: u32,
    encodeUnit: u32,
    encrypt: u32,
    signature: Option<String>,
}


fn reader_file (file_path: &String) -> Result<Vec<u8>, OTAError> {
    match File::open(file_path) {
        Ok(mut f) => {

            let mut content_string = Vec::new();

            f.read_to_end(&mut content_string).unwrap();

            let content = content_string;
            let original_bytes = Bytes::copy_from_slice(&content);
            
            let mut hasher = Hasher::new();
            hasher.update(&original_bytes);
            let original_crc32 = hasher.finalize();

            let mut compressed_bytes = BytesMut::with_capacity(0);

            let original_length: u32 = original_bytes.len().try_into().unwrap();
            
            const CHUNK_SIZE: usize = 16384;
            let mut chunks = original_bytes.chunks(CHUNK_SIZE);
            
            loop {
                match chunks.next() {
                    Some(chunck_data) => {
                        let tmp = compress(chunck_data, CompressionLevel::Lvl3);
                        compressed_bytes.extend_from_slice(&tmp[..]);                        
                    },
                    None => break,
                }
            }

            let compressed_length = compressed_bytes.len().try_into().unwrap();

            let mut hasher = Hasher::new();
            hasher.update(&compressed_bytes);
            let compressed_crc32 = hasher.finalize();
        
            let header = Header {
                mark: "~update!".to_string(),
                fileSize: compressed_length,
                originalFileSize: original_length,
                crc32: compressed_crc32,
                originalCrc32: original_crc32,
                encode: 1,
                encodeUnit: 16384,
                encrypt: 0,
                signature: None
            };

            let json_vec = to_vec(&header).map_err(|_| OTAError::HeaderError)?;
            let json_length:u32 = json_vec.len().try_into().unwrap();
            let mut result = BytesMut::new();
            let mut json_bytes = json_length.to_be_bytes();
            json_bytes.reverse();

            result.extend_from_slice(&json_bytes);
            result.extend_from_slice(&json_vec);
            result.put_u8(0);
            result.extend_from_slice(&compressed_bytes);
            Ok(result.freeze().to_vec())
        },
        Err(_) => Err(OTAError::NotExist),
    }
}
