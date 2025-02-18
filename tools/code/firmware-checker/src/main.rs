mod error;

use crate::error::OTAError;
use byteorder::{LittleEndian, ReadBytesExt};
use bytes::{BufMut, Bytes, BytesMut};
use clap::Parser;
use crc32fast::Hasher;
use quicklz::{compress, decompress, CompressionLevel};
use secp256k1::hashes::sha256;
use secp256k1::SecretKey;
use secp256k1::{Message, Secp256k1};
use serde::{Deserialize, Serialize};
use serde_json::{from_str, to_vec};
use std::fs::File;
use std::io::Cursor;
use std::io::{self, BufRead, BufReader, Read, Write};

#[derive(Parser, Debug)]
#[command(author, version, about, long_about=None)]
struct Args {
    #[arg(short, long)]
    source: String,

    #[arg(short, long)]
    destination: Option<String>,

    #[arg(short, long)]
    key: Option<String>,
}

#[tokio::main]
async fn main() -> std::io::Result<()> {
    let args = Args::parse();
    let source_path = args.source;

    match extract_and_verify_file(&source_path).await {
        Ok(content) => {
            let mut last_found_position = None;

            for i in (0..content.len()).step_by(4096) {
                if i + 4096 > content.len() {
                    break;
                }

                let block = &content[i..i + 4096];
                if block[2..].iter().all(|&x| x == 0xFF) {
                    let offset = ((block[0] as u16) << 8) | (block[1] as u16);
                    last_found_position = Some(i);
                }
            }

            if let (Some(position)) = last_found_position {
                // Print the first 32 bytes of the sector (including the offset)
                let original_size = position + 4096;
                let original_firmware = &content[..original_size];

                // Calculate SHA256 of the final mh1903.bin
                let hash = Message::from_hashed_data::<sha256::Hash>(original_firmware);
                let mut handle = io::stdout().lock();
                writeln!(
                    handle,
                    "Firmware checksum sha256: {} \nYou can check this value on your device.",
                    hash
                );
            }
            Ok(())
        }
        Err(e) => {
            eprintln!("Error extracting file: {:?}", e);
            Ok(())
        }
    }
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

async fn extract_and_verify_file(file_path: &String) -> Result<Vec<u8>, OTAError> {
    match File::open(file_path) {
        Ok(mut f) => {
            let mut content_string = Vec::new();
            f.read_to_end(&mut content_string).unwrap();
            let head_size: &[u8] = &content_string[0..4];
            let head_offset = Cursor::new(head_size)
                .read_i32::<LittleEndian>()
                .expect("Failed to read i32")
                + 4;
            let content = &content_string[4..head_offset as usize];
            let start = head_offset + 1;

            const CHUNK_SIZE: usize = 16384;
            if let Ok(s) = std::str::from_utf8(content) {
                let header: Header = from_str(s).expect("Failed to parse JSON");

                let data_string = &content_string[start as usize..];
                let mut offset = 0;
                let mut decompressed_data: Vec<u8> = Vec::new();

                // Decompress all chunks
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

                // Calculate SHA256 of the decompressed data
                Ok(decompressed_data)
            } else {
                Err(OTAError::InvalidHeader)
            }
        }
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
        assert_eq!(
            message.to_string(),
            "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1".to_string()
        );
    }
}
