mod error;

use crate::error::OTAError;
use byteorder::{BigEndian, LittleEndian, ReadBytesExt};
use clap::Parser;
use hex;
use quicklz::decompress;
use secp256k1::hashes::sha256;
use secp256k1::Message;
use serde::{Deserialize, Serialize};
use std::fmt;
use std::fs::File;
use std::io::Cursor;
use std::io::Read;

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
async fn main() -> Result<(), OTAError> {
    let args = Args::parse();
    let result = get_decompress_file_sha256(&args.source).await?;
    println!(
        "Firmware checksum sha256: {} \nYou can check this value on your device.",
        result
    );
    Ok(())
}

#[derive(Serialize, Debug, Deserialize)]
struct Header {
    mark: String,
    file_size: u32,
    original_file_size: u32,
    compressed_hash: String,
    original_hash: String,
    encode: u32,
    encode_unit: u32,
    encrypt: u32,
    signature: Option<String>,
}

impl fmt::Display for Header {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "Header {{ mark: \"{}\", file_size: {:08X}, original_file_size: {:08X}, compressed_hash: {}, original_hash: {}, encode: {:X}, encode_unit: {}, encrypt: {}, signature: {:?} }}",
            self.mark,
            self.file_size,
            self.original_file_size,
            self.compressed_hash,
            self.original_hash,
            self.encode,
            self.encode_unit,
            self.encrypt,
            self.signature
        )
    }
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

fn qlz_size_compressed(source: &[u8]) -> usize {
    let n = if (source[0] & 2) == 2 { 4 } else { 1 };
    let r = fast_read(&source[1..], n);
    r as usize & (0xffffffff >> ((4 - n) * 8))
}

fn parse_binary_header(content: &[u8]) -> Result<Header, OTAError> {
    let mut cursor = Cursor::new(content);

    // Read mark (assuming it's a fixed length of 4 bytes)
    let mut mark_bytes = [0u8; 8];
    cursor
        .read_exact(&mut mark_bytes)
        .map_err(|_| OTAError::InvalidFormat)?;
    let mark = String::from_utf8_lossy(&mark_bytes).to_string();

    // Read other fields
    let file_size = cursor
        .read_u32::<BigEndian>()
        .map_err(|_| OTAError::InvalidFormat)?;
    let original_file_size = cursor
        .read_u32::<BigEndian>()
        .map_err(|_| OTAError::InvalidFormat)?;

    // Read hashes (assuming they're fixed length of 32 bytes each)
    let mut hash_bytes = [0u8; 32];
    cursor
        .read_exact(&mut hash_bytes)
        .map_err(|_| OTAError::InvalidFormat)?;
    let compressed_hash = hex::encode(hash_bytes);

    cursor
        .read_exact(&mut hash_bytes)
        .map_err(|_| OTAError::InvalidFormat)?;
    let original_hash = hex::encode(hash_bytes);

    let encode = cursor
        .read_u32::<BigEndian>()
        .map_err(|_| OTAError::InvalidFormat)?;
    let encode_unit = cursor
        .read_u32::<BigEndian>()
        .map_err(|_| OTAError::InvalidFormat)?;
    let encrypt = cursor
        .read_u32::<BigEndian>()
        .map_err(|_| OTAError::InvalidFormat)?;

    Ok(Header {
        mark,
        file_size,
        original_file_size,
        compressed_hash,
        original_hash,
        encode,
        encode_unit,
        encrypt,
        signature: None, // Binary format might not include signature
    })
}

async fn get_decompress_file_sha256(file_path: &str) -> Result<String, OTAError> {
    let mut file = File::open(file_path).map_err(|_| OTAError::NotExist)?;
    let mut content = Vec::new();
    file.read_to_end(&mut content).map_err(|_| OTAError::InvalidFormat)?;

    let head_size = Cursor::new(&content[0..4])
        .read_i32::<LittleEndian>()
        .map_err(|_| OTAError::InvalidFormat)?;
    let head_offset = head_size + 4;
    
    let header = parse_binary_header(&content[4..head_offset as usize])?;

    let data = &content[(head_offset + 1) as usize..];
    let decompressed_data = decompress_chunks(data, header.file_size)?;
    
    Ok(Message::from_hashed_data::<sha256::Hash>(&decompressed_data).to_string())
}

fn decompress_chunks(data: &[u8], total_size: u32) -> Result<Vec<u8>, OTAError> {
    const CHUNK_SIZE: usize = 16384;
    let mut offset = 0;
    let mut decompressed_data = Vec::with_capacity(total_size as usize);

    while offset < total_size as usize {
        let compressed_size = qlz_size_compressed(&data[offset..]);
        let compressed_chunk = &data[offset..offset + compressed_size];
        let decompressed_chunk = decompress(&mut Cursor::new(compressed_chunk), CHUNK_SIZE as u32)
            .map_err(|_| OTAError::DecompressionError)?;
        
        decompressed_data.extend_from_slice(&decompressed_chunk);
        offset += compressed_size;
    }

    Ok(decompressed_data)
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
