#![no_std]

pub const MAX_NESTING_DEPTH: u32 = 32;
pub const MAX_ITEMS: u32 = 1024;
pub const MAX_JSON_NESTING_DEPTH: u32 = 32;
pub const MAX_JSON_ITEMS: u32 = 1024;
const LOOKUP_WIDTH: usize = 26;
const LOOKUP_SIZE: usize = LOOKUP_WIDTH * LOOKUP_WIDTH;
const INVALID_WORD: u16 = 256;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(u32)]
pub enum ValidationStatus {
    Ok = 1,
    InvalidInput = 2,
    InvalidScheme = 3,
    InvalidType = 4,
    InvalidMultipart = 5,
    InvalidBytewordsLength = 6,
    InvalidBytewordsWord = 7,
    InvalidChecksum = 8,
    CborUnexpectedEof = 9,
    CborInvalid = 10,
    CborNestingLimit = 11,
    CborItemLimit = 12,
    CborTrailingData = 13,
    JsonUnexpectedEof = 14,
    JsonInvalid = 15,
    JsonNestingLimit = 16,
    JsonItemLimit = 17,
    JsonTrailingData = 18,
    JsonRootType = 19,
}

const MINIMAL_WORDS: &[u8] = b"aeadaoaxaaahamatayasbkbdbnbtbabsbebybgbwbbbzcmchcscfcycwcecackctcxclcpcndkdadsdidedtdrdndwdpdmdldyeheyeoeeecenemetesftfrfnfsfmfhfzfpfwfxfyfefgflfdgagegrgsgtglgwgdgygmgughgohfhghdhkhthphhhlhyhehnhsidiaieihiyioisinimjejzjnjtjljojsjpjkjykpkoktkskkknkgkekikblblalylflslrlplnltloldlelulklgmnmymhmemomumwmdmtmsmknlnyndnsntnnnenboyoeotoxonolospdptpkpypspmplpepfpaprqdqzrerprlrorhrdrkrfryrnrsrtsesasrssskswstspsosgsbsfsntotktitttdtetytltbtstptatnuyuoutueurvtvyvovlvevwvavdvswlwdwmwpwewywswtwnwzwfwkykynylyaytzszoztzczezm";

const fn build_minimal_lookup() -> [u16; LOOKUP_SIZE] {
    let mut lookup = [INVALID_WORD; LOOKUP_SIZE];
    let mut value = 0usize;
    while value < 256 {
        let first = MINIMAL_WORDS[value * 2];
        let second = MINIMAL_WORDS[value * 2 + 1];
        let index = ((first - b'a') as usize) * LOOKUP_WIDTH + (second - b'a') as usize;
        lookup[index] = value as u16;
        value += 1;
    }
    lookup
}

static MINIMAL_LOOKUP: [u16; LOOKUP_SIZE] = build_minimal_lookup();

fn ascii_lower(value: u8) -> u8 {
    if value.is_ascii_uppercase() {
        value + (b'a' - b'A')
    } else {
        value
    }
}

fn decode_word(first: u8, second: u8) -> Result<u8, ValidationStatus> {
    let first = ascii_lower(first);
    let second = ascii_lower(second);
    if !first.is_ascii_lowercase() || !second.is_ascii_lowercase() {
        return Err(ValidationStatus::InvalidBytewordsWord);
    }
    let index = usize::from(first - b'a') * LOOKUP_WIDTH + usize::from(second - b'a');
    let value = *MINIMAL_LOOKUP
        .get(index)
        .ok_or(ValidationStatus::InvalidBytewordsWord)?;
    if value == INVALID_WORD {
        Err(ValidationStatus::InvalidBytewordsWord)
    } else {
        Ok(value as u8)
    }
}

fn crc32_update(mut crc: u32, value: u8) -> u32 {
    crc ^= u32::from(value);
    let mut bit = 0;
    while bit < 8 {
        let mask = 0u32.wrapping_sub(crc & 1);
        crc = (crc >> 1) ^ (0xedb8_8320 & mask);
        bit += 1;
    }
    crc
}

#[derive(Clone, Copy)]
struct Bytewords<'a> {
    encoded: &'a [u8],
    payload_len: usize,
}

impl<'a> Bytewords<'a> {
    fn new(encoded: &'a [u8]) -> Result<Self, ValidationStatus> {
        if (encoded.len() & 1) != 0 {
            return Err(ValidationStatus::InvalidBytewordsLength);
        }
        let decoded_len = encoded.len() / 2;
        let candidate = Self {
            encoded,
            payload_len: decoded_len.saturating_sub(4),
        };
        let mut index = 0usize;
        while index < decoded_len {
            candidate.decoded_byte(index)?;
            index += 1;
        }
        if decoded_len < 4 {
            return Err(ValidationStatus::InvalidChecksum);
        }
        let payload_len = decoded_len - 4;
        let candidate = Self {
            encoded,
            payload_len,
        };

        let mut crc = 0xffff_ffff;
        index = 0;
        while index < payload_len {
            crc = crc32_update(crc, candidate.decoded_byte(index)?);
            index += 1;
        }
        crc = !crc;

        let mut checksum = 0u32;
        while index < decoded_len {
            checksum = (checksum << 8) | u32::from(candidate.decoded_byte(index)?);
            index += 1;
        }
        if crc != checksum {
            return Err(ValidationStatus::InvalidChecksum);
        }
        Ok(candidate)
    }

    fn decoded_byte(&self, index: usize) -> Result<u8, ValidationStatus> {
        let encoded_index = index
            .checked_mul(2)
            .ok_or(ValidationStatus::InvalidBytewordsLength)?;
        let first = *self
            .encoded
            .get(encoded_index)
            .ok_or(ValidationStatus::CborUnexpectedEof)?;
        let second = *self
            .encoded
            .get(encoded_index + 1)
            .ok_or(ValidationStatus::CborUnexpectedEof)?;
        decode_word(first, second)
    }
}

trait CborSource {
    fn len(&self) -> usize;
    fn byte(&self, index: usize) -> Result<u8, ValidationStatus>;
}

impl CborSource for Bytewords<'_> {
    fn len(&self) -> usize {
        self.payload_len
    }

    fn byte(&self, index: usize) -> Result<u8, ValidationStatus> {
        self.decoded_byte(index)
    }
}

struct RawCbor<'a>(&'a [u8]);

impl CborSource for RawCbor<'_> {
    fn len(&self) -> usize {
        self.0.len()
    }

    fn byte(&self, index: usize) -> Result<u8, ValidationStatus> {
        self.0
            .get(index)
            .copied()
            .ok_or(ValidationStatus::CborUnexpectedEof)
    }
}

struct CborReader<S> {
    source: S,
    position: usize,
}

impl<S: CborSource> CborReader<S> {
    fn read(&mut self) -> Result<u8, ValidationStatus> {
        if self.position >= self.source.len() {
            return Err(ValidationStatus::CborUnexpectedEof);
        }
        let value = self.source.byte(self.position)?;
        self.position += 1;
        Ok(value)
    }

    fn peek(&self) -> Result<u8, ValidationStatus> {
        if self.position >= self.source.len() {
            return Err(ValidationStatus::CborUnexpectedEof);
        }
        self.source.byte(self.position)
    }

    fn skip(&mut self, length: u64) -> Result<(), ValidationStatus> {
        let remaining = self.source.len() - self.position;
        if length > remaining as u64 {
            return Err(ValidationStatus::CborUnexpectedEof);
        }
        self.position += length as usize;
        Ok(())
    }
}

fn read_argument<S: CborSource>(
    reader: &mut CborReader<S>,
    additional: u8,
) -> Result<Option<u64>, ValidationStatus> {
    match additional {
        0..=23 => Ok(Some(u64::from(additional))),
        24 => Ok(Some(u64::from(reader.read()?))),
        25..=27 => {
            let byte_count = 1u8 << (additional - 24);
            let mut value = 0u64;
            let mut index = 0u8;
            while index < byte_count {
                value = (value << 8) | u64::from(reader.read()?);
                index += 1;
            }
            Ok(Some(value))
        }
        31 => Ok(None),
        _ => Err(ValidationStatus::CborInvalid),
    }
}

fn child_depth(depth: u32) -> Result<u32, ValidationStatus> {
    if depth >= MAX_NESTING_DEPTH {
        Err(ValidationStatus::CborNestingLimit)
    } else {
        Ok(depth + 1)
    }
}

fn consume_indefinite_string<S: CborSource>(
    reader: &mut CborReader<S>,
    expected_major: u8,
) -> Result<(), ValidationStatus> {
    loop {
        let initial = reader.peek()?;
        if initial == 0xff {
            reader.read()?;
            return Ok(());
        }
        reader.read()?;
        if (initial >> 5) != expected_major {
            return Err(ValidationStatus::CborInvalid);
        }
        let length = read_argument(reader, initial & 0x1f)?.ok_or(ValidationStatus::CborInvalid)?;
        reader.skip(length)?;
    }
}

fn validate_cbor_item<S: CborSource>(
    reader: &mut CborReader<S>,
    depth: u32,
    item_count: &mut u32,
) -> Result<(), ValidationStatus> {
    if *item_count >= MAX_ITEMS {
        return Err(ValidationStatus::CborItemLimit);
    }
    *item_count += 1;

    let initial = reader.read()?;
    let major = initial >> 5;
    let argument = read_argument(reader, initial & 0x1f)?;
    match major {
        0 | 1 => {
            if argument.is_none() {
                return Err(ValidationStatus::CborInvalid);
            }
        }
        2 | 3 => match argument {
            Some(length) => reader.skip(length)?,
            None => consume_indefinite_string(reader, major)?,
        },
        4 => {
            let depth = child_depth(depth)?;
            match argument {
                Some(length) => {
                    let mut index = 0u64;
                    while index < length {
                        validate_cbor_item(reader, depth, item_count)?;
                        index += 1;
                    }
                }
                None => loop {
                    if reader.peek()? == 0xff {
                        reader.read()?;
                        break;
                    }
                    validate_cbor_item(reader, depth, item_count)?;
                },
            }
        }
        5 => {
            let depth = child_depth(depth)?;
            match argument {
                Some(length) => {
                    let mut index = 0u64;
                    while index < length {
                        validate_cbor_item(reader, depth, item_count)?;
                        validate_cbor_item(reader, depth, item_count)?;
                        index += 1;
                    }
                }
                None => loop {
                    if reader.peek()? == 0xff {
                        reader.read()?;
                        break;
                    }
                    validate_cbor_item(reader, depth, item_count)?;
                    if reader.peek()? == 0xff {
                        return Err(ValidationStatus::CborInvalid);
                    }
                    validate_cbor_item(reader, depth, item_count)?;
                },
            }
        }
        6 => {
            if argument.is_none() {
                return Err(ValidationStatus::CborInvalid);
            }
            validate_cbor_item(reader, child_depth(depth)?, item_count)?;
        }
        7 => {
            if argument.is_none() {
                return Err(ValidationStatus::CborInvalid);
            }
        }
        _ => return Err(ValidationStatus::CborInvalid),
    }
    Ok(())
}

fn find_byte(value: &[u8], needle: u8) -> Option<usize> {
    let mut index = 0usize;
    while index < value.len() {
        if value.get(index).copied() == Some(needle) {
            return Some(index);
        }
        index += 1;
    }
    None
}

fn parse_decimal_u16(value: &[u8]) -> Result<u16, ValidationStatus> {
    if value.is_empty() {
        return Err(ValidationStatus::InvalidMultipart);
    }
    let mut result = 0u16;
    let mut index = 0usize;
    while index < value.len() {
        let digit = value
            .get(index)
            .copied()
            .ok_or(ValidationStatus::InvalidMultipart)?;
        if !digit.is_ascii_digit() {
            return Err(ValidationStatus::InvalidMultipart);
        }
        result = result
            .checked_mul(10)
            .and_then(|number| number.checked_add(u16::from(digit - b'0')))
            .ok_or(ValidationStatus::InvalidMultipart)?;
        index += 1;
    }
    Ok(result)
}

fn validate_multipart_indices(value: &[u8]) -> Result<(), ValidationStatus> {
    let separator = find_byte(value, b'-').ok_or(ValidationStatus::InvalidMultipart)?;
    let first = value
        .get(..separator)
        .ok_or(ValidationStatus::InvalidMultipart)?;
    let second = value
        .get(separator + 1..)
        .ok_or(ValidationStatus::InvalidMultipart)?;
    if find_byte(second, b'-').is_some() {
        return Err(ValidationStatus::InvalidMultipart);
    }
    parse_decimal_u16(first)?;
    parse_decimal_u16(second)?;
    Ok(())
}

fn trim_ascii(mut value: &[u8]) -> &[u8] {
    while value
        .first()
        .copied()
        .is_some_and(|byte| byte.is_ascii_whitespace())
    {
        value = value.get(1..).unwrap_or(&[]);
    }
    while value
        .last()
        .copied()
        .is_some_and(|byte| byte.is_ascii_whitespace())
    {
        value = value.get(..value.len() - 1).unwrap_or(&[]);
    }
    value
}

fn validate_scheme(value: &[u8]) -> bool {
    value.len() == 3
        && value.first().copied().map(ascii_lower) == Some(b'u')
        && value.get(1).copied().map(ascii_lower) == Some(b'r')
        && value.get(2).copied() == Some(b':')
}

fn parse_ur_body(value: &[u8]) -> Result<&[u8], ValidationStatus> {
    let value = trim_ascii(value);
    let scheme = value.get(..3).ok_or(ValidationStatus::InvalidScheme)?;
    if !validate_scheme(scheme) {
        return Err(ValidationStatus::InvalidScheme);
    }
    let remainder = value.get(3..).ok_or(ValidationStatus::InvalidScheme)?;
    let type_end = find_byte(remainder, b'/').ok_or(ValidationStatus::InvalidType)?;
    let ur_type = remainder
        .get(..type_end)
        .ok_or(ValidationStatus::InvalidType)?;
    if ur_type.is_empty()
        || ur_type.iter().copied().any(|byte| {
            let byte = ascii_lower(byte);
            !byte.is_ascii_lowercase() && !byte.is_ascii_digit() && byte != b'-'
        })
    {
        return Err(ValidationStatus::InvalidType);
    }

    let payload = remainder
        .get(type_end + 1..)
        .ok_or(ValidationStatus::InvalidInput)?;
    if let Some(index_end) = find_byte(payload, b'/') {
        let indices = payload
            .get(..index_end)
            .ok_or(ValidationStatus::InvalidMultipart)?;
        let body = payload
            .get(index_end + 1..)
            .ok_or(ValidationStatus::InvalidMultipart)?;
        if find_byte(body, b'/').is_some() {
            return Err(ValidationStatus::InvalidMultipart);
        }
        validate_multipart_indices(indices)?;
        Ok(body)
    } else {
        Ok(payload)
    }
}

pub fn validate_ur(value: &[u8]) -> ValidationStatus {
    let result = (|| {
        let body = parse_ur_body(value)?;
        let source = Bytewords::new(body)?;
        let mut reader = CborReader {
            source,
            position: 0,
        };
        let mut item_count = 0u32;
        validate_cbor_item(&mut reader, 0, &mut item_count)?;
        if reader.position != reader.source.len() {
            return Err(ValidationStatus::CborTrailingData);
        }
        Ok(())
    })();

    match result {
        Ok(()) => ValidationStatus::Ok,
        Err(status) => status,
    }
}

pub fn validate_cbor(value: &[u8]) -> ValidationStatus {
    let result = (|| {
        if value.is_empty() {
            return Err(ValidationStatus::InvalidInput);
        }
        let mut reader = CborReader {
            source: RawCbor(value),
            position: 0,
        };
        let mut item_count = 0u32;
        validate_cbor_item(&mut reader, 0, &mut item_count)?;
        if reader.position != reader.source.len() {
            return Err(ValidationStatus::CborTrailingData);
        }
        Ok(())
    })();

    match result {
        Ok(()) => ValidationStatus::Ok,
        Err(status) => status,
    }
}

fn hex_digit(value: u8) -> Result<u16, ValidationStatus> {
    match value {
        b'0'..=b'9' => Ok(u16::from(value - b'0')),
        b'a'..=b'f' => Ok(u16::from(value - b'a') + 10),
        b'A'..=b'F' => Ok(u16::from(value - b'A') + 10),
        _ => Err(ValidationStatus::JsonInvalid),
    }
}

fn consume_json_item(item_count: &mut u32) -> Result<(), ValidationStatus> {
    if *item_count >= MAX_JSON_ITEMS {
        return Err(ValidationStatus::JsonItemLimit);
    }
    *item_count += 1;
    Ok(())
}

fn read_json_hex_quad(value: &[u8], position: &mut usize) -> Result<u16, ValidationStatus> {
    let mut result = 0u16;
    let mut index = 0;
    while index < 4 {
        let byte = value
            .get(*position)
            .copied()
            .ok_or(ValidationStatus::JsonUnexpectedEof)?;
        *position += 1;
        result = (result << 4) | hex_digit(byte)?;
        index += 1;
    }
    Ok(result)
}

fn decode_json_string_in_place(
    value: &mut [u8],
    start: usize,
    end: usize,
) -> Result<usize, ValidationStatus> {
    let mut read = start + 1;
    let mut write = 0usize;
    while read < end {
        let byte = *value.get(read).ok_or(ValidationStatus::JsonUnexpectedEof)?;
        read += 1;
        if byte == b'"' {
            if read != end {
                return Err(ValidationStatus::JsonTrailingData);
            }
            return Ok(write);
        }
        if byte < 0x20 {
            return Err(ValidationStatus::JsonInvalid);
        }
        if byte != b'\\' {
            *value.get_mut(write).ok_or(ValidationStatus::JsonInvalid)? = byte;
            write += 1;
            continue;
        }

        let escape = value
            .get(read)
            .copied()
            .ok_or(ValidationStatus::JsonUnexpectedEof)?;
        read += 1;
        let decoded = match escape {
            b'"' => b'"',
            b'\\' => b'\\',
            b'/' => b'/',
            b'b' => 0x08,
            b'f' => 0x0c,
            b'n' => b'\n',
            b'r' => b'\r',
            b't' => b'\t',
            b'u' => {
                let first = read_json_hex_quad(value, &mut read)?;
                let scalar = if (0xd800..=0xdbff).contains(&first) {
                    let pair_end = read
                        .checked_add(2)
                        .ok_or(ValidationStatus::JsonUnexpectedEof)?;
                    if value.get(read..pair_end) != Some(b"\\u") {
                        return Err(ValidationStatus::JsonInvalid);
                    }
                    read = pair_end;
                    let second = read_json_hex_quad(value, &mut read)?;
                    if !(0xdc00..=0xdfff).contains(&second) {
                        return Err(ValidationStatus::JsonInvalid);
                    }
                    0x1_0000 + ((u32::from(first) - 0xd800) << 10) + (u32::from(second) - 0xdc00)
                } else if (0xdc00..=0xdfff).contains(&first) {
                    return Err(ValidationStatus::JsonInvalid);
                } else {
                    u32::from(first)
                };
                let character = char::from_u32(scalar).ok_or(ValidationStatus::JsonInvalid)?;
                let mut encoded = [0u8; 4];
                let encoded = character.encode_utf8(&mut encoded).as_bytes();
                for byte in encoded.iter().copied() {
                    *value.get_mut(write).ok_or(ValidationStatus::JsonInvalid)? = byte;
                    write += 1;
                }
                continue;
            }
            _ => return Err(ValidationStatus::JsonInvalid),
        };
        *value.get_mut(write).ok_or(ValidationStatus::JsonInvalid)? = decoded;
        write += 1;
    }
    Err(ValidationStatus::JsonUnexpectedEof)
}

fn validate_json_structure(value: &[u8]) -> Result<(), ValidationStatus> {
    if value.first().copied() != Some(b'{') {
        return Err(ValidationStatus::JsonRootType);
    }

    let mut stack = [0u8; MAX_JSON_NESTING_DEPTH as usize];
    let mut depth = 0usize;
    let mut item_count = 0u32;
    let mut in_string = false;
    let mut escaped = false;
    let mut complete = false;

    for byte in value.iter().copied() {
        if complete {
            if !byte.is_ascii_whitespace() {
                return Err(ValidationStatus::JsonTrailingData);
            }
            continue;
        }
        if in_string {
            if escaped {
                escaped = false;
            } else if byte == b'\\' {
                escaped = true;
            } else if byte == b'"' {
                in_string = false;
            } else if byte < 0x20 {
                return Err(ValidationStatus::JsonInvalid);
            }
            continue;
        }

        match byte {
            b'"' => in_string = true,
            b'{' | b'[' => {
                if depth >= stack.len() {
                    return Err(ValidationStatus::JsonNestingLimit);
                }
                *stack
                    .get_mut(depth)
                    .ok_or(ValidationStatus::JsonNestingLimit)? =
                    if byte == b'{' { b'}' } else { b']' };
                depth += 1;
                consume_json_item(&mut item_count)?;
            }
            b'}' | b']' => {
                let expected = depth
                    .checked_sub(1)
                    .and_then(|index| stack.get(index))
                    .copied()
                    .ok_or(ValidationStatus::JsonInvalid)?;
                if expected != byte {
                    return Err(ValidationStatus::JsonInvalid);
                }
                depth -= 1;
                if depth == 0 {
                    complete = true;
                }
            }
            b':' | b',' => consume_json_item(&mut item_count)?,
            _ => {}
        }
    }

    if in_string || escaped || depth != 0 || !complete {
        return Err(ValidationStatus::JsonUnexpectedEof);
    }
    Ok(())
}

pub fn validate_eip712_json(value: &mut [u8]) -> ValidationStatus {
    let result = (|| {
        let start = value
            .iter()
            .position(|byte| !byte.is_ascii_whitespace())
            .ok_or(ValidationStatus::InvalidInput)?;
        let end = value
            .iter()
            .rposition(|byte| !byte.is_ascii_whitespace())
            .ok_or(ValidationStatus::InvalidInput)?
            + 1;
        match value.get(start).copied() {
            Some(b'{') => {
                validate_json_structure(value.get(start..end).ok_or(ValidationStatus::JsonInvalid)?)
            }
            Some(b'"') => {
                let length = decode_json_string_in_place(value, start, end)?;
                validate_json_structure(value.get(..length).ok_or(ValidationStatus::JsonInvalid)?)
            }
            Some(_) => Err(ValidationStatus::JsonRootType),
            None => Err(ValidationStatus::InvalidInput),
        }
    })();

    match result {
        Ok(()) => ValidationStatus::Ok,
        Err(status) => status,
    }
}

#[cfg(test)]
extern crate std;

#[cfg(test)]
mod tests {
    use super::*;
    use std::string::String;
    use std::vec;
    use std::vec::Vec;

    fn encode_bytewords(payload: &[u8]) -> String {
        let mut crc = 0xffff_ffff;
        for byte in payload {
            crc = crc32_update(crc, *byte);
        }
        let checksum = (!crc).to_be_bytes();
        let mut result = String::new();
        for byte in payload.iter().chain(checksum.iter()) {
            let index = usize::from(*byte) * 2;
            result.push(MINIMAL_WORDS[index] as char);
            result.push(MINIMAL_WORDS[index + 1] as char);
        }
        result
    }

    fn encode_ur(payload: &[u8]) -> String {
        let mut result = String::from("ur:bytes/");
        result.push_str(&encode_bytewords(payload));
        result
    }

    fn validate_json(value: &[u8]) -> ValidationStatus {
        validate_eip712_json(&mut value.to_vec())
    }

    #[test]
    fn accepts_one_complete_cbor_object() {
        assert_eq!(
            validate_ur(encode_ur(&[0xa0]).as_bytes()),
            ValidationStatus::Ok
        );
        let request_shaped = [
            0xa2, 0x01, 0xd8, 0x25, 0x50, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x02, 0x43, 0xaa, 0xbb, 0xcc,
        ];
        assert_eq!(
            validate_ur(encode_ur(&request_shaped).as_bytes()),
            ValidationStatus::Ok
        );
        let multipart = encode_ur(&[0xa0]).replacen("ur:bytes/", "ur:bytes/1-2/", 1);
        assert_eq!(validate_ur(multipart.as_bytes()), ValidationStatus::Ok);

        assert_eq!(
            validate_ur(b"ur:bytes/iehsjyhspmwfwfia"),
            ValidationStatus::CborUnexpectedEof
        );
    }

    #[test]
    fn rejects_bytewords_errors() {
        assert_eq!(
            validate_ur(b"ur:eth-sign-request/zz"),
            ValidationStatus::InvalidBytewordsWord
        );
        assert_eq!(
            validate_ur(b"ur:bytes/aea"),
            ValidationStatus::InvalidBytewordsLength
        );

        let mut invalid_word = encode_ur(&[0xa0]).into_bytes();
        let length = invalid_word.len();
        invalid_word[length - 2] = b'?';
        invalid_word[length - 1] = b'?';
        assert_eq!(
            validate_ur(&invalid_word),
            ValidationStatus::InvalidBytewordsWord
        );

        let mut bad_checksum = encode_ur(&[0xa0]).into_bytes();
        let length = bad_checksum.len();
        bad_checksum[length - 2] = b'a';
        bad_checksum[length - 1] = if bad_checksum[length - 1] == b'e' {
            b'd'
        } else {
            b'e'
        };
        assert_eq!(
            validate_ur(&bad_checksum),
            ValidationStatus::InvalidChecksum
        );
    }

    #[test]
    fn enforces_cbor_complexity_and_trailing_data() {
        let mut at_depth_limit = vec![0x81; MAX_NESTING_DEPTH as usize];
        at_depth_limit.push(0xf6);
        assert_eq!(
            validate_ur(encode_ur(&at_depth_limit).as_bytes()),
            ValidationStatus::Ok
        );

        let mut over_depth_limit = vec![0x81; MAX_NESTING_DEPTH as usize + 1];
        over_depth_limit.push(0xf6);
        assert_eq!(
            validate_ur(encode_ur(&over_depth_limit).as_bytes()),
            ValidationStatus::CborNestingLimit
        );

        let mut over_item_limit = Vec::from([0x99, 0x04, 0x00]);
        over_item_limit.extend(core::iter::repeat(0xf6).take(MAX_ITEMS as usize));
        assert_eq!(
            validate_ur(encode_ur(&over_item_limit).as_bytes()),
            ValidationStatus::CborItemLimit
        );

        assert_eq!(
            validate_ur(encode_ur(&[0xa0, 0x00]).as_bytes()),
            ValidationStatus::CborTrailingData
        );
        assert_eq!(validate_cbor(&at_depth_limit), ValidationStatus::Ok);
        assert_eq!(
            validate_cbor(&over_depth_limit),
            ValidationStatus::CborNestingLimit
        );
        assert_eq!(
            validate_cbor(&over_item_limit),
            ValidationStatus::CborItemLimit
        );
        assert_eq!(
            validate_cbor(&[0xa0, 0x00]),
            ValidationStatus::CborTrailingData
        );
    }

    #[test]
    fn validates_direct_and_stringified_eip712_json() {
        assert_eq!(validate_json(br#"{}"#), ValidationStatus::Ok);
        assert_eq!(validate_json(br#""{}""#), ValidationStatus::Ok);
        assert_eq!(
            validate_json(br#""{\"message\":{\"text\":\"\u4f60\u597d\"}}""#),
            ValidationStatus::Ok
        );
        assert_eq!(
            validate_json(br#"{"message":"[[[[[[[[[["}"#),
            ValidationStatus::Ok
        );
        assert_eq!(validate_json(br#"[]"#), ValidationStatus::JsonRootType);
        assert_eq!(
            validate_json(br#"{} trailing"#),
            ValidationStatus::JsonTrailingData
        );
    }

    #[test]
    fn enforces_eip712_json_complexity_budgets() {
        let mut at_depth_limit = String::from("{\"value\":");
        at_depth_limit.push_str(&"[".repeat((MAX_JSON_NESTING_DEPTH - 1) as usize));
        at_depth_limit.push_str("null");
        at_depth_limit.push_str(&"]".repeat((MAX_JSON_NESTING_DEPTH - 1) as usize));
        at_depth_limit.push('}');
        assert_eq!(
            validate_json(at_depth_limit.as_bytes()),
            ValidationStatus::Ok
        );

        let mut over_depth_limit = String::from("{\"value\":");
        over_depth_limit.push_str(&"[".repeat(MAX_JSON_NESTING_DEPTH as usize));
        over_depth_limit.push_str("null");
        over_depth_limit.push_str(&"]".repeat(MAX_JSON_NESTING_DEPTH as usize));
        over_depth_limit.push('}');
        assert_eq!(
            validate_json(over_depth_limit.as_bytes()),
            ValidationStatus::JsonNestingLimit
        );

        let mut over_item_limit = String::from("{\"value\":[");
        let mut index = 0;
        while index < MAX_JSON_ITEMS {
            if index != 0 {
                over_item_limit.push(',');
            }
            over_item_limit.push_str("null");
            index += 1;
        }
        over_item_limit.push_str("]}");
        assert_eq!(
            validate_json(over_item_limit.as_bytes()),
            ValidationStatus::JsonItemLimit
        );
    }
}
