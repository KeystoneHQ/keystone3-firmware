use crate::{ValidationStatus as V, MAX_ITEMS, MAX_NESTING_DEPTH};

const DEFINITE: u32 = 1;
const ARRAY: u32 = 2;
const MAP_KEY: u32 = 3;
const MAP_VALUE: u32 = 4;
const BYTES: u32 = 5;
const TEXT: u32 = 6;

#[repr(C)]
struct Frame {
    remaining: u32,
    kind: u32,
}

#[repr(C, align(8))]
pub struct State {
    argument: u64,
    remaining_bytes: u64,
    frames: [Frame; MAX_NESTING_DEPTH as usize + 1],
    height: u32,
    depth: u32,
    items: u32,
    error: u32,
    head: u32,
    argument_bytes: u32,
    chunk_body: u32,
    done: u32,
}

pub const STORAGE_BYTES: usize = 512;
const _: () = assert!(core::mem::size_of::<State>() <= STORAGE_BYTES);

impl State {
    fn top(&self) -> u32 {
        self.height
            .checked_sub(1)
            .and_then(|i| self.frames.get(i as usize))
            .map_or(0, |f| f.kind)
    }

    fn open(&mut self, kind: u32, remaining: u32) -> Result<(), V> {
        if kind < BYTES && self.depth >= MAX_NESTING_DEPTH {
            return Err(V::CborNestingLimit);
        }
        let frame = self
            .frames
            .get_mut(self.height as usize)
            .ok_or(V::CborNestingLimit)?;
        frame.kind = kind;
        frame.remaining = remaining;
        self.height += 1;
        if kind < BYTES {
            self.depth += 1;
        }
        if kind == DEFINITE && remaining == 0 {
            self.close();
            self.complete()?;
        } else if kind == DEFINITE && self.items >= MAX_ITEMS {
            return Err(V::CborItemLimit);
        }
        Ok(())
    }

    fn close(&mut self) {
        if self.top() < BYTES {
            self.depth = self.depth.saturating_sub(1);
        }
        self.height = self.height.saturating_sub(1);
    }

    fn complete(&mut self) -> Result<(), V> {
        loop {
            if self.height == 0 {
                self.done = 1;
                return Ok(());
            }
            let frame = self
                .frames
                .get_mut((self.height - 1) as usize)
                .ok_or(V::CborInvalid)?;
            match frame.kind {
                DEFINITE => {
                    frame.remaining = frame.remaining.checked_sub(1).ok_or(V::CborInvalid)?;
                    if frame.remaining != 0 {
                        return if self.items >= MAX_ITEMS {
                            Err(V::CborItemLimit)
                        } else {
                            Ok(())
                        };
                    }
                    self.close();
                }
                MAP_KEY => {
                    frame.kind = MAP_VALUE;
                    return Ok(());
                }
                MAP_VALUE => {
                    frame.kind = MAP_KEY;
                    return Ok(());
                }
                ARRAY => return Ok(()),
                _ => return Err(V::CborInvalid),
            }
        }
    }

    fn dispatch(&mut self) -> Result<(), V> {
        let major = self.head >> 5;
        let indefinite = self.head & 31 == 31;
        if self.top() >= BYTES {
            if indefinite {
                return Err(V::CborInvalid);
            }
            self.remaining_bytes = self.argument;
            self.chunk_body = 1;
            return Ok(());
        }
        match major {
            0 | 1 | 7 => {
                if indefinite {
                    return Err(V::CborInvalid);
                }
                self.complete()
            }
            2 | 3 => {
                if indefinite {
                    self.open(if major == 2 { BYTES } else { TEXT }, 0)
                } else {
                    self.remaining_bytes = self.argument;
                    self.chunk_body = 0;
                    if self.remaining_bytes == 0 {
                        self.complete()?;
                    }
                    Ok(())
                }
            }
            4 | 5 => {
                if indefinite {
                    self.open(if major == 4 { ARRAY } else { MAP_KEY }, 0)
                } else {
                    let count = if major == 5 {
                        self.argument.saturating_mul(2)
                    } else {
                        self.argument
                    };
                    self.open(DEFINITE, count.min(u32::MAX as u64) as u32)
                }
            }
            6 => {
                if indefinite {
                    return Err(V::CborInvalid);
                }
                self.open(DEFINITE, 1)
            }
            _ => Err(V::CborInvalid),
        }
    }

    fn byte(&mut self, byte: u8) -> Result<(), V> {
        if self.done != 0 {
            return Err(V::CborTrailingData);
        }
        if self.remaining_bytes != 0 {
            self.remaining_bytes -= 1;
            if self.remaining_bytes == 0 && self.chunk_body == 0 {
                self.complete()?;
            }
            return Ok(());
        }
        if self.argument_bytes != 0 {
            self.argument = (self.argument << 8) | u64::from(byte);
            self.argument_bytes -= 1;
            return if self.argument_bytes == 0 {
                self.dispatch()
            } else {
                Ok(())
            };
        }
        let top = self.top();
        if byte == 0xff {
            if !matches!(top, ARRAY | MAP_KEY | BYTES | TEXT) {
                return Err(V::CborInvalid);
            }
            self.close();
            return self.complete();
        }
        if top >= BYTES {
            let expected = if top == BYTES { 2 } else { 3 };
            if byte >> 5 != expected {
                return Err(V::CborInvalid);
            }
        } else {
            if self.items >= MAX_ITEMS {
                return Err(V::CborItemLimit);
            }
            self.items += 1;
        }
        self.head = u32::from(byte);
        self.argument = 0;
        match byte & 31 {
            n @ 0..=23 => {
                self.argument = u64::from(n);
                self.dispatch()
            }
            n @ 24..=27 => {
                self.argument_bytes = 1 << (n - 24);
                Ok(())
            }
            31 => self.dispatch(),
            _ => Err(V::CborInvalid),
        }
    }

    pub fn push(&mut self, bytes: &[u8]) {
        for &byte in bytes {
            if self.error != 0 {
                break;
            }
            if let Err(error) = self.byte(byte) {
                self.error = error as u32;
            }
        }
    }

    pub fn finish(&self) -> u32 {
        if self.error != 0 {
            return self.error;
        }
        if self.items == 0 {
            return V::InvalidInput as u32;
        }
        if self.done == 0 {
            return V::CborUnexpectedEof as u32;
        }
        V::Ok as u32
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::vec;

    fn state() -> State {
        unsafe { core::mem::zeroed() }
    }

    fn check_chunks(input: &[u8], expected: V) {
        assert_eq!(crate::validate_cbor(input), expected);
        for size in [1, 2, 7, 228] {
            let mut parser = state();
            for chunk in input.chunks(size) {
                parser.push(chunk);
            }
            assert_eq!(parser.finish(), expected as u32, "chunk size {size}");
        }
    }

    #[test]
    fn length_arguments_survive_chunk_boundaries() {
        for input in [
            &b"\x58\x01x"[..],
            &b"\x59\x00\x01x"[..],
            &b"\x5a\x00\x00\x00\x01x"[..],
            &b"\x5b\x00\x00\x00\x00\x00\x00\x00\x01x"[..],
        ] {
            check_chunks(input, V::Ok);
            for split in 1..input.len() {
                let mut parser = state();
                parser.push(&input[..split]);
                assert_eq!(parser.finish(), V::CborUnexpectedEof as u32);
                parser.push(&input[split..]);
                assert_eq!(parser.finish(), V::Ok as u32);
            }
        }
    }

    #[test]
    fn large_body_truncation_and_trailing_data() {
        let mut input = vec![0xa1, 0x01, 0x59, 0x7f, 0xfb];
        input.resize(32 * 1024, 0xab);
        check_chunks(&input, V::Ok);
        input.pop();
        check_chunks(&input, V::CborUnexpectedEof);
        input.extend_from_slice(&[0xab, 0x00]);
        check_chunks(&input, V::CborTrailingData);

        let mut parser = state();
        parser.push(&input[..input.len() - 1]);
        assert_eq!(parser.finish(), V::Ok as u32);
        parser.push(&[0x00]);
        assert_eq!(parser.finish(), V::CborTrailingData as u32);
    }

    #[test]
    fn indefinite_containers_require_complete_values_and_matching_strings() {
        let valid = b"\xbf\x01\x9f\x5f\x41x\x40\xff\x7f\x61y\x60\xff\xff\xff";
        check_chunks(valid, V::Ok);
        check_chunks(&[], V::InvalidInput);
        for end in 1..valid.len() {
            check_chunks(&valid[..end], V::CborUnexpectedEof);
        }
        for invalid in [
            &b"\xbf\x01\xff"[..],
            &b"\xa1\x01\xff"[..],
            &b"\x5f\x61x\xff"[..],
            &b"\x7f\x41x\xff"[..],
            &b"\x5f\x5f\xff\xff"[..],
            &b"\x81\xff"[..],
        ] {
            check_chunks(invalid, V::CborInvalid);
        }
    }

    #[test]
    fn parser_error_is_preserved_across_later_chunks() {
        let mut parser = state();
        parser.push(b"\xbf\x01");
        parser.push(b"\xff");
        assert_eq!(parser.finish(), V::CborInvalid as u32);
        parser.push(b"\x00\xff");
        assert_eq!(parser.finish(), V::CborInvalid as u32);
    }

    #[test]
    fn complexity_limits_apply_across_chunks() {
        for (depth, expected) in [
            (MAX_NESTING_DEPTH, V::Ok),
            (MAX_NESTING_DEPTH + 1, V::CborNestingLimit),
        ] {
            let mut input = vec![0x81; depth as usize];
            input.push(0x00);
            check_chunks(&input, expected);
        }
        for (count, expected) in [(MAX_ITEMS - 1, V::Ok), (MAX_ITEMS, V::CborItemLimit)] {
            let mut input = vec![0x99, (count >> 8) as u8, count as u8];
            input.resize(input.len() + count as usize, 0x00);
            check_chunks(&input, expected);
        }
    }
}
