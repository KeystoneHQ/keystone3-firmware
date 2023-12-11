mod error;

use crate::error::OTAError; 
use clap::Parser;
use std::io::{BufReader,BufRead, Write, Read, self};
use std::fs::File;
use bytes::{ Bytes, BytesMut, BufMut};
use quicklz::{compress, CompressionLevel, decompress};
use crc32fast::Hasher;
use serde::{Serialize, Deserialize};
use serde_json::{from_str, to_vec};
use secp256k1::{Secp256k1, Message};
use secp256k1::hashes::sha256;
use secp256k1::{SecretKey};
use byteorder::{LittleEndian, ReadBytesExt};
use std::io::Cursor;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about=None)]
struct Args {
    #[arg(short, long)]
    source: String,
    
    #[arg(short, long)]
    destination:Option<String>,

    #[arg(short, long)]
    key: Option<String>,
}

#[tokio::main]
async fn main() -> std::io::Result<()>{
    let args = Args::parse();
    let source_path = args.source;
    
    let result = get_decompress_file_sha256(&source_path).await.unwrap();
    let mut handle = io::stdout().lock();
    writeln!(handle, "Firmware checksum sha256: {} \nYou can check this value on your device.", result)
}
#[derive(Serialize, Debug, Deserialize)]
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

fn fast_read(src: &[u8], bytes: usize) -> u32 {
    if bytes >= 1 && bytes <= 4 {
        // Use little-endian byte order for the conversion
        match bytes {
            1 => u32::from_le_bytes([src[0], 0, 0, 0]),
            2 => u32::from_le_bytes([src[0], src[1], 0, 0]),
            3 => u32::from_le_bytes([src[0], src[1], src[2], 0]),
            4 => u32::from_le_bytes([src[0], src[1], src[2], src[3]]),
            _ => 0,
        }
    } else {
        0
    }
}

fn qlz_size_decompressed(source: &[u8]) -> usize {
    let n = if (source[0] & 2) == 2 { 4 } else { 1 };
    let r = fast_read(&source[1 + n..], n);
    r as usize & (0xffffffff >> ((4 - n) * 8))
}

fn qlz_size_compressed(source: &[u8]) -> usize {
    let n = if (source[0] & 2) == 2 { 4 } else { 1 };
    let r = fast_read(&source[1..], n);
    r as usize & (0xffffffff >> ((4 - n) * 8))
}

async fn get_decompress_file_sha256 (file_path: &String) -> Result<String, OTAError> {
    let result:String;
    match File::open(file_path) {
        Ok(mut f) => {
            let mut content_string = Vec::new();
            f.read_to_end(&mut content_string).unwrap();
            let head_size: &[u8] = &content_string[0..4];
            let head_offset = Cursor::new(head_size).read_i32::<LittleEndian>().expect("Failed to read i32") + 4;
            let content = &content_string[4..head_offset as usize];
            let start = head_offset + 1;

            const CHUNK_SIZE: usize = 16384;
            if let Ok(s) = std::str::from_utf8(content) {
                let header: Header = from_str(s).expect("Failed to parse JSON");

                let data_string = &content_string[start as usize..];
                let mut offset = 0;
                let mut decompressed_data: Vec<u8> = Vec::new();

                loop {
                    let compressed_size = qlz_size_compressed(&data_string[offset..]);
                    let compressed_data = &data_string[offset..offset + compressed_size];
                    let mut r = Cursor::new(compressed_data.as_ref());
                    let decompressed_chunk = decompress(&mut r, CHUNK_SIZE as u32).unwrap();
                    decompressed_data.extend_from_slice(&decompressed_chunk);
                    offset += compressed_size;
                    if offset == header.fileSize as usize {
                        break;
                    }
                }

                let message = Message::from_hashed_data::<sha256::Hash>(&decompressed_data);
                result =  message.to_string();
            } else {
                result = "Invalid UTF-8 data".to_string();
            }
            Ok(result)
        },
        Err(_) => Err(OTAError::NotExist),
    }
}

#[cfg(test)]
mod tests {
    use super::*;    
    #[test]
    fn test_hash_data() {
        let test = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
        let a = test.as_bytes();
        let message = Message::from_hashed_data::<sha256::Hash>(a);
        assert_eq!(message.to_string(), "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1".to_string());
    }
}