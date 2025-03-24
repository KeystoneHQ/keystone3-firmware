mod error;

use crate::error::OTAError;
use bytes::{BufMut, Bytes, BytesMut};
use clap::Parser;
use crc32fast::Hasher;
use quicklz::{compress, CompressionLevel};
use serde::Serialize;
use serde_json::to_vec;
use sha2::{Sha256, Digest};
use std::fmt;
use std::fs::File;
use std::io::{Read, Write};
use hex;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about=None)]
struct Args {
    #[arg(short, long)]
    source: String,

    #[arg(short, long)]
    destination: String,
}

fn main() -> std::io::Result<()> {
    let args = Args::parse();

    let source_path = args.source;

    let target_path = args.destination;

    let result = reader_file(&source_path).unwrap();

    let mut file = File::create(target_path)?;

    file.write_all(&result)?;

    Ok(())
}
#[derive(Serialize, Debug)]
struct Header {
    mark: String,
    fileSize: u32,
    originalFileSize: u32,
    compressedHash: [u8; 32],
    originalHash: [u8; 32],
    encode: u32,
    encodeUnit: u32,
    encrypt: u32,
    signature: Option<String>,
}

impl fmt::Display for Header {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "Header {{ mark: \"{}\", file_size: {:08X}, original_file_size: {:08X}, compressedHash: {:?}, originalHash: {:?}, encode: {:X}, encode_unit: {}, encrypt: {}, signature: {:?} }}",
            self.mark,
            self.fileSize,
            self.originalFileSize,
            hex::encode(self.compressedHash),
            hex::encode(self.originalHash),
            self.encode,
            self.encodeUnit,
            self.encrypt,
            self.signature
        )
    }
}

impl Header {
    fn to_bytes(&self) -> Vec<u8> {
        let mut bytes = Vec::new();

        bytes.extend(self.mark.as_bytes());
        bytes.extend(&self.fileSize.to_be_bytes());
        bytes.extend(&self.originalFileSize.to_be_bytes());
        bytes.extend(&self.compressedHash);
        bytes.extend(&self.originalHash);
        bytes.extend(&self.encode.to_be_bytes());
        bytes.extend(&self.encodeUnit.to_be_bytes());
        bytes.extend(&self.encrypt.to_be_bytes());

        match &self.signature {
            Some(sig) => {
                bytes.extend(sig.as_bytes());
            }
            None => {
                bytes.extend(vec![0x0; 128]);
            }
        }

        bytes
    }
}

fn reader_file(file_path: &String) -> Result<Vec<u8>, OTAError> {
    match File::open(file_path) {
        Ok(mut f) => {
            let mut content_string = Vec::new();

            f.read_to_end(&mut content_string).unwrap();

            let content = content_string;
            let original_bytes = Bytes::copy_from_slice(&content);

            let mut hasher = Sha256::new();
            hasher.update(&original_bytes);
            let original_sha256: [u8; 32] = hasher.finalize().into();

            let mut compressed_bytes = BytesMut::with_capacity(0);

            let original_length: u32 = original_bytes.len().try_into().unwrap();

            const CHUNK_SIZE: usize = 16384;
            let mut chunks = original_bytes.chunks(CHUNK_SIZE);

            loop {
                match chunks.next() {
                    Some(chunck_data) => {
                        let tmp = compress(chunck_data, CompressionLevel::Lvl3);
                        compressed_bytes.extend_from_slice(&tmp[..]);
                    }
                    None => break,
                }
            }

            let compressed_length = compressed_bytes.len().try_into().unwrap();

            let mut hasher = Sha256::new();
            hasher.update(&compressed_bytes);
            let compressed_hash: [u8; 32] = hasher.finalize().into();

            let header = Header {
                mark: "~fwdata!".to_string(),
                fileSize: compressed_length,
                originalFileSize: original_length,
                compressedHash: compressed_hash,
                originalHash: original_sha256,
                encode: 1,
                encodeUnit: 16384,
                encrypt: 0,
                signature: Some(String::from("")),
            };

            let header_string = header.to_bytes();
            let hex_string: String = header_string
                .iter()
                .map(|byte| format!("{:02X}, ", byte))
                .collect::<Vec<String>>()
                .join("");

            let head_length: u32 = header_string.len().try_into().unwrap();
            let mut result = BytesMut::new();
            let mut head_bytes = head_length.to_be_bytes();
            head_bytes.reverse();

            result.extend_from_slice(&head_bytes);
            result.extend_from_slice(&header_string);
            result.put_u8(0);
            result.extend_from_slice(&compressed_bytes);
            Ok(result.freeze().to_vec())
        }
        Err(_) => Err(OTAError::NotExist),
    }
}
