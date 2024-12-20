/// Returns how many bytes are needed to encode a value.
#[inline]
pub fn length(value: u64) -> usize {
    let zero_len = 64 - value.leading_zeros();
    let offset = if zero_len == 0 { 7 } else { 6 };
    ((offset + zero_len) / 7) as usize
}

/// Encode a `u64` integer to the byte slice. Returns how many bytes were
/// encoded.
#[inline]
pub fn encode(value: u64, buf: &mut [u8]) -> usize {
    encode_with_offset(value, buf, 0)
}

/// Encode a `u64` integer at a specific offset in the byte slice. Returns how
/// many bytes were encoded.
#[inline]
pub fn encode_with_offset(value: u64, buf: &mut [u8], offset: usize) -> usize {
    let mut off = offset;
    let mut val = value;
    while val > 127 {
        buf[off] = (val as u8) | 128;
        off += 1;
        val >>= 7;
    }
    buf[off] = val as u8;

    off + 1 - offset
}

/// Decode a byte slice into a `u64` integer. Returns how many bytes were
/// decoded.
#[inline]
pub fn decode(buf: &[u8], value: &mut u64) -> usize {
    decode_with_offset(buf, 0, value)
}

/// Decode a byte slice into a `u64` integer at a specific offset. Returns how
/// many bytes were decoded.
#[inline]
pub fn decode_with_offset(buf: &[u8], offset: usize, value: &mut u64) -> usize {
    let mut val = 0 as u64;
    let mut fac = 1 as u64;
    let mut off = offset;

    loop {
        let byte = buf[off];
        off += 1;
        val += fac * ((byte & 127) as u64);
        fac <<= 7;
        if byte & 128 == 0 {
            break;
        }
    }

    *value = val;

    off - offset
}

/// Returns how many bytes are needed to encode a value.
#[inline]
pub fn signed_length(value: i64) -> usize {
    length(unsign(value))
}

/// Encode a `i64` (signed) integer at a specific offset in the byte slice.
/// Returns how many bytes were encoded.
#[inline]
pub fn signed_encode(value: i64, buf: &mut [u8]) -> usize {
    encode_with_offset(unsign(value), buf, 0)
}

/// Encode a `i64` (signed) integer at a specific offset in the byte slice.
/// Returns how many bytes were encoded.
#[inline]
pub fn signed_encode_with_offset(value: i64, buf: &mut [u8], offset: usize) -> usize {
    encode_with_offset(unsign(value), buf, offset)
}

/// Decode a byte slice into a `i64` (signed) integer.  Returns how many bytes
/// were decoded.
#[inline]
pub fn signed_decode(buf: &[u8], value: &mut i64) -> usize {
    signed_decode_with_offset(buf, 0, value)
}

/// Decode a byte slice into a `i64` (signed) integer at a specific offset.
/// Returns how many bytes were decoded.
#[inline]
pub fn signed_decode_with_offset(buf: &[u8], offset: usize, value: &mut i64) -> usize {
    let mut val = 0;
    let off = decode_with_offset(buf, offset, &mut val);
    *value = sign(val);
    off
}

/// Convert an `i64` into a `u64`.
#[inline]
fn unsign(value: i64) -> u64 {
    if value >= 0 {
        (value * 2) as u64
    } else {
        (value * -2 - 1) as u64
    }
}

/// Convert a `u64` into a `i64`.
#[inline]
fn sign(value: u64) -> i64 {
    if value & 1 != 0 {
        -(((value + 1) / 2) as i64)
    } else {
        (value / 2) as i64
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_verinteger() {
        let mut buf = [0u8; 2];
        assert_eq!(encode(0x16f9, &mut buf), 2);
        assert_eq!(buf, [0xf9, 0x2d]);
    }

    #[test]
    fn test_decode() {
        let buf = [0x8c, 0x02];
        let mut value = 0;
        assert_eq!(decode(&buf, &mut value), 2);
        assert_eq!(value, 268);
    }
}
