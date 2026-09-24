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
    /// Reported by the C side when a payload exceeds the sandbox input limit
    /// for its build variant (`MPU_SANDBOX_*_INPUT_MAX_SIZE`).
    InputTooLarge = 20,
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

// ---------------------------------------------------------------------------
// Streaming CBOR structure validator.
//
// Walks a CBOR item one byte at a time with an explicit stack, enforcing
// MAX_NESTING_DEPTH and MAX_ITEMS and reporting exactly the statuses a
// recursive-descent walk would (the recursive form is kept under `cfg(test)`
// as `reference` and the two are checked against each other). Memory is
// O(MAX_NESTING_DEPTH), independent of the payload: string bodies are skipped
// by counting, never buffered, so that the MPU sandbox can validate a multipart UR
// chunk by chunk without holding the reassembled payload.
//
// Sandbox constraints: no allocation, no panicking paths, and no bulk
// zeroing or copying that LLVM would lower to a memclr/memcpy call. Calls
// out of `.sandbox_text` are rejected by tools/check_mpu_sandbox_elf.py.
// This has two consequences:
// - The validator is never built as a struct literal: that makes LLVM zero
//   the whole thing with one memclr call, so `new` writes the scalar fields
//   one by one into a `MaybeUninit` slot.
// - The stack and head buffers are `MaybeUninit`: initializing them would be
//   that same bulk zeroing, and a `&mut` to the validator is only valid if
//   every plain field holds a value.
// Both buffers are written before they are read, and every index into them
// is bounds-checked explicitly.
// ---------------------------------------------------------------------------

use core::mem::MaybeUninit;

/// Definite-length array, map (remaining counts entries * 2) or tag (1).
const FRAME_DEFINITE: u8 = 1;
const FRAME_INDEF_ARRAY: u8 = 2;
const FRAME_INDEF_MAP: u8 = 3;
/// Indefinite-length byte/text string: `major` is the required chunk major.
const FRAME_INDEF_STRING: u8 = 4;

const PHASE_START: u8 = 0;
const PHASE_ITEM: u8 = 1;
const PHASE_DONE: u8 = 2;

/// Containers are rejected when opened at depth MAX_NESTING_DEPTH, so at most
/// that many array/map/tag frames are open, plus one indefinite-string frame
/// (strings cannot nest).
const STACK_CAPACITY: usize = MAX_NESTING_DEPTH as usize + 1;
/// A CBOR head is at most 1 initial byte + an 8-byte argument.
const HEAD_CAPACITY: usize = 9;

#[repr(C)]
#[derive(Clone, Copy)]
struct Frame {
    kind: u8,
    major: u8,
    /// Indefinite map: 1 while a value (not a key) is expected next.
    expect_value: u8,
    _pad: u8,
    /// Items still expected in a definite frame (saturated: any declared
    /// count beyond u32::MAX exceeds MAX_ITEMS long before it matters).
    remaining: u32,
}

#[repr(C, align(8))]
pub struct CborStreamValidator {
    skip_remaining: u64,
    /// Frames `0..sp` are initialized: `push_frame` writes each before `sp`
    /// covers it, and nothing above `sp` is ever read. `MaybeUninit` because
    /// initializing the array would be a memclr call (see sandbox
    /// constraints above).
    stack: MaybeUninit<[Frame; STACK_CAPACITY]>,
    sp: u32,
    /// Open array/map/tag frames (indefinite-string frames excluded).
    depth: u32,
    item_count: u32,
    /// Sticky failure status (0 = none), reported by `finish`.
    status: u32,
    /// Bytes `0..head_len` are initialized; `MaybeUninit` for the same
    /// reason as `stack` (see sandbox constraints above).
    head: MaybeUninit<[u8; HEAD_CAPACITY]>,
    head_len: u8,
    head_need: u8,
    phase: u8,
    /// The current skip is an indefinite-string chunk body, not an item.
    skip_is_chunk: u8,
}

const _: () =
    assert!(core::mem::size_of::<CborStreamValidator>() <= CborStreamValidator::STATE_SIZE);

impl CborStreamValidator {
    /// Bytes the C side must reserve (matches MPU_SANDBOX_CBOR_STREAM_STATE_SIZE).
    pub const STATE_SIZE: usize = 512;
    pub const STATE_ALIGN: usize = 8;

    /// Initializes the scalar fields of a validator in place, one volatile
    /// store each. A struct literal would not do: constructing the struct by
    /// value makes LLVM zero the whole thing with a memclr call, which the
    /// sandbox must not make (see sandbox constraints above). `stack` and
    /// `head` are left uninitialized; both are written before being read.
    ///
    /// # Safety
    /// `ptr` must be valid for writes of `Self`.
    unsafe fn init_raw(ptr: *mut Self) {
        use core::ptr::{addr_of_mut, write_volatile};
        write_volatile(addr_of_mut!((*ptr).skip_remaining), 0);
        write_volatile(addr_of_mut!((*ptr).sp), 0);
        write_volatile(addr_of_mut!((*ptr).depth), 0);
        write_volatile(addr_of_mut!((*ptr).item_count), 0);
        write_volatile(addr_of_mut!((*ptr).status), 0);
        write_volatile(addr_of_mut!((*ptr).head_len), 0);
        write_volatile(addr_of_mut!((*ptr).head_need), 0);
        write_volatile(addr_of_mut!((*ptr).phase), PHASE_START);
        write_volatile(addr_of_mut!((*ptr).skip_is_chunk), 0);
    }

    /// Creates a validator. It is built inside a `MaybeUninit` and then
    /// assumed initialized, because a struct literal would make LLVM zero the
    /// whole thing with a memclr call (see sandbox constraints above). Always
    /// inlined so that the slot becomes the caller's local and the return is
    /// not a copy.
    #[inline(always)]
    pub fn new() -> Self {
        let mut slot = MaybeUninit::<Self>::uninit();
        // SAFETY: `init_raw` initializes every field that is not itself
        // `MaybeUninit`, which is all `assume_init` requires.
        unsafe {
            Self::init_raw(slot.as_mut_ptr());
            slot.assume_init()
        }
    }

    fn head_byte(&self, index: usize) -> u8 {
        if index >= self.head_len as usize {
            return 0;
        }
        // SAFETY: `index < head_len <= HEAD_CAPACITY`, and bytes below
        // `head_len` were written by `set_head_byte`.
        unsafe { *(self.head.as_ptr() as *const u8).add(index) }
    }

    fn set_head_byte(&mut self, index: usize, value: u8) -> bool {
        if index >= HEAD_CAPACITY {
            return false;
        }
        // SAFETY: `index < HEAD_CAPACITY`.
        unsafe { (self.head.as_mut_ptr() as *mut u8).add(index).write(value) };
        true
    }

    /// Reinitializes validator state living in caller-provided memory (the
    /// sandbox RW region). Only scalars are written: frames are fully written
    /// on push and the head buffer is dead while `head_len == 0`.
    pub fn reset(&mut self) {
        // SAFETY: `self` is valid for writes of `Self`.
        unsafe { Self::init_raw(self) }
    }

    /// Initializes validator state in `capacity` bytes of caller-provided
    /// memory at `ptr`, after checking size and alignment; returns whether
    /// the validator fits. Only the scalar fields are written (see
    /// `init_raw`), so the memory need not be initialized beforehand.
    ///
    /// # Safety
    /// `ptr` must be valid for writes of `capacity` bytes.
    pub unsafe fn init_in_raw(ptr: *mut u8, capacity: usize) -> bool {
        if !Self::raw_fits(ptr, capacity) {
            return false;
        }
        Self::init_raw(ptr as *mut Self);
        true
    }

    /// Views validator state in caller-provided memory, after checking size
    /// and alignment.
    ///
    /// # Safety
    /// `ptr` must be valid for reads and writes of `capacity` bytes for the
    /// returned lifetime, and the memory must not be accessed otherwise during
    /// it. It must hold validator state: every field that is not `MaybeUninit`
    /// is initialized, and the frames below `sp` and the head bytes below
    /// `head_len` were written by this type. Memory initialized by
    /// `init_in_raw`, or fully initialized memory (for example zeroed) that has
    /// since been modified only through this type, satisfies this.
    pub unsafe fn from_raw<'a>(ptr: *mut u8, capacity: usize) -> Option<&'a mut Self> {
        if !Self::raw_fits(ptr, capacity) {
            return None;
        }
        Some(&mut *(ptr as *mut Self))
    }

    fn raw_fits(ptr: *mut u8, capacity: usize) -> bool {
        !ptr.is_null()
            && capacity >= core::mem::size_of::<Self>()
            && (ptr as usize) % core::mem::align_of::<Self>() == 0
    }

    pub fn push(&mut self, bytes: &[u8]) {
        for byte in bytes.iter().copied() {
            if self.status != 0 {
                return;
            }
            self.feed(byte);
        }
    }

    pub fn finish(&mut self) -> ValidationStatus {
        if let Some(status) = status_from_u32(self.status) {
            return status;
        }
        if self.phase == PHASE_DONE {
            ValidationStatus::Ok
        } else {
            ValidationStatus::CborUnexpectedEof
        }
    }

    fn fail(&mut self, status: ValidationStatus) {
        self.status = status as u32;
    }

    fn top(&mut self) -> Option<&mut Frame> {
        let index = (self.sp as usize).checked_sub(1)?;
        if index >= STACK_CAPACITY {
            return None;
        }
        // SAFETY: `index < sp <= STACK_CAPACITY`, and every frame below `sp`
        // was written by `push_frame` since the last `reset`.
        Some(unsafe { &mut *(self.stack.as_mut_ptr() as *mut Frame).add(index) })
    }

    fn top_kind(&self) -> u8 {
        match (self.sp as usize).checked_sub(1) {
            Some(index) if index < STACK_CAPACITY => {
                // SAFETY: `index < sp <= STACK_CAPACITY`, and every frame below
                // `sp` was written by `push_frame` since the last `reset`.
                unsafe { (*(self.stack.as_ptr() as *const Frame).add(index)).kind }
            }
            _ => 0,
        }
    }

    fn push_frame(&mut self, frame: Frame) -> bool {
        let index = self.sp as usize;
        if index >= STACK_CAPACITY {
            return false;
        }
        // SAFETY: `index < STACK_CAPACITY`; the slot is fully written before
        // `sp` is raised to cover it.
        unsafe {
            (self.stack.as_mut_ptr() as *mut Frame)
                .add(index)
                .write(frame)
        };
        self.sp += 1;
        true
    }

    fn pop_frame(&mut self) {
        self.sp = self.sp.saturating_sub(1);
    }

    fn feed(&mut self, byte: u8) {
        if self.phase == PHASE_DONE {
            self.fail(ValidationStatus::CborTrailingData);
            return;
        }
        if self.skip_remaining > 0 {
            self.skip_remaining -= 1;
            if self.skip_remaining == 0 && self.skip_is_chunk == 0 {
                self.complete_item();
            }
            return;
        }
        if self.head_len == 0 {
            self.begin_head(byte);
            return;
        }
        if !self.set_head_byte(self.head_len as usize, byte) {
            self.fail(ValidationStatus::CborInvalid);
            return;
        }
        self.head_len += 1;
        if self.head_len >= self.head_need {
            self.dispatch_head();
        }
    }

    /// First byte of a head: a break, an indefinite-string chunk head, or the
    /// start of an item (which is where items are counted).
    fn begin_head(&mut self, byte: u8) {
        self.phase = PHASE_ITEM;
        let kind = self.top_kind();
        if byte == 0xff {
            match kind {
                FRAME_INDEF_ARRAY => {
                    self.pop_frame();
                    self.depth = self.depth.saturating_sub(1);
                    self.complete_item();
                }
                FRAME_INDEF_MAP => {
                    let expect_value = self.top().map(|frame| frame.expect_value).unwrap_or(0);
                    if expect_value != 0 {
                        self.fail(ValidationStatus::CborInvalid);
                    } else {
                        self.pop_frame();
                        self.depth = self.depth.saturating_sub(1);
                        self.complete_item();
                    }
                }
                FRAME_INDEF_STRING => {
                    self.pop_frame();
                    self.complete_item();
                }
                // A break where an item is required (top level, definite
                // container, tag content) is major 7 with no argument.
                _ => self.fail(ValidationStatus::CborInvalid),
            }
            return;
        }
        if kind == FRAME_INDEF_STRING {
            // A chunk's major type is checked before its length is read, so a
            // wrong-major chunk is Invalid even if the input ends right here.
            let expected = self.top().map(|frame| frame.major).unwrap_or(0);
            if (byte >> 5) != expected {
                self.fail(ValidationStatus::CborInvalid);
                return;
            }
        } else {
            if self.item_count >= MAX_ITEMS {
                self.fail(ValidationStatus::CborItemLimit);
                return;
            }
            self.item_count += 1;
        }
        let need = match byte & 0x1f {
            0..=23 | 31 => 1,
            24 => 2,
            25 => 3,
            26 => 5,
            27 => 9,
            _ => {
                self.fail(ValidationStatus::CborInvalid);
                return;
            }
        };
        let _ = self.set_head_byte(0, byte);
        self.head_len = 1;
        self.head_need = need;
        if need == 1 {
            self.dispatch_head();
        }
    }

    fn head_argument(&self) -> Option<u64> {
        let additional = self.head_byte(0) & 0x1f;
        match additional {
            0..=23 => Some(u64::from(additional)),
            31 => None,
            _ => {
                let mut value = 0u64;
                let mut index = 1usize;
                while index < self.head_need as usize {
                    value = (value << 8) | u64::from(self.head_byte(index));
                    index += 1;
                }
                Some(value)
            }
        }
    }

    fn dispatch_head(&mut self) {
        let major = self.head_byte(0) >> 5;
        let argument = self.head_argument();
        self.head_len = 0;
        self.head_need = 0;

        if self.top_kind() == FRAME_INDEF_STRING {
            self.dispatch_string_chunk(major, argument);
            return;
        }

        match major {
            0 | 1 => {
                if argument.is_none() {
                    self.fail(ValidationStatus::CborInvalid);
                } else {
                    self.complete_item();
                }
            }
            2 | 3 => match argument {
                Some(0) => self.complete_item(),
                Some(length) => {
                    self.skip_remaining = length;
                    self.skip_is_chunk = 0;
                }
                None => {
                    if !self.push_frame(Frame {
                        kind: FRAME_INDEF_STRING,
                        major,
                        expect_value: 0,
                        _pad: 0,
                        remaining: 0,
                    }) {
                        self.fail(ValidationStatus::CborNestingLimit);
                    }
                }
            },
            4 | 5 => {
                if self.depth >= MAX_NESTING_DEPTH {
                    self.fail(ValidationStatus::CborNestingLimit);
                    return;
                }
                match argument {
                    Some(0) => self.complete_item(),
                    Some(count) => {
                        let entries = if major == 5 {
                            count.saturating_mul(2)
                        } else {
                            count
                        };
                        self.open_definite(saturate(entries));
                    }
                    None => {
                        let kind = if major == 5 {
                            FRAME_INDEF_MAP
                        } else {
                            FRAME_INDEF_ARRAY
                        };
                        if self.push_frame(Frame {
                            kind,
                            major: 0,
                            expect_value: 0,
                            _pad: 0,
                            remaining: 0,
                        }) {
                            self.depth += 1;
                        } else {
                            self.fail(ValidationStatus::CborNestingLimit);
                        }
                    }
                }
            }
            6 => {
                if argument.is_none() {
                    self.fail(ValidationStatus::CborInvalid);
                } else if self.depth >= MAX_NESTING_DEPTH {
                    self.fail(ValidationStatus::CborNestingLimit);
                } else {
                    self.open_definite(1);
                }
            }
            _ => {
                // Major 7: simple values and floats are complete; a break
                // (no argument) is only valid where a break was expected,
                // which `begin_head` already handled.
                if argument.is_none() {
                    self.fail(ValidationStatus::CborInvalid);
                } else {
                    self.complete_item();
                }
            }
        }
    }

    fn open_definite(&mut self, remaining: u32) {
        if self.push_frame(Frame {
            kind: FRAME_DEFINITE,
            major: 0,
            expect_value: 0,
            _pad: 0,
            remaining,
        }) {
            self.depth += 1;
            self.expect_definite_item();
        } else {
            self.fail(ValidationStatus::CborNestingLimit);
        }
    }

    /// A definite frame is about to read another item: the recursive walk
    /// checks the item budget before reading anything, so do the same here
    /// (this decides ItemLimit vs UnexpectedEof when input ends right there).
    fn expect_definite_item(&mut self) {
        if self.item_count >= MAX_ITEMS {
            self.fail(ValidationStatus::CborItemLimit);
        }
    }

    fn dispatch_string_chunk(&mut self, major: u8, argument: Option<u64>) {
        let expected = self.top().map(|frame| frame.major).unwrap_or(0);
        if major != expected {
            self.fail(ValidationStatus::CborInvalid);
            return;
        }
        match argument {
            None => self.fail(ValidationStatus::CborInvalid),
            Some(0) => {}
            Some(length) => {
                self.skip_remaining = length;
                self.skip_is_chunk = 1;
            }
        }
    }

    /// An item finished: close every definite frame it completes, then
    /// arm the next expectation.
    fn complete_item(&mut self) {
        loop {
            let kind = self.top_kind();
            match kind {
                0 => {
                    self.phase = PHASE_DONE;
                    return;
                }
                FRAME_DEFINITE => {
                    let remaining = match self.top() {
                        Some(frame) => {
                            frame.remaining = frame.remaining.saturating_sub(1);
                            frame.remaining
                        }
                        None => 0,
                    };
                    if remaining == 0 {
                        self.pop_frame();
                        self.depth = self.depth.saturating_sub(1);
                        continue;
                    }
                    self.expect_definite_item();
                    return;
                }
                FRAME_INDEF_ARRAY => return,
                FRAME_INDEF_MAP => {
                    if let Some(frame) = self.top() {
                        frame.expect_value ^= 1;
                    }
                    return;
                }
                _ => {
                    // Chunks of an indefinite string never reach here.
                    self.fail(ValidationStatus::CborInvalid);
                    return;
                }
            }
        }
    }
}

impl Default for CborStreamValidator {
    fn default() -> Self {
        Self::new()
    }
}

fn saturate(value: u64) -> u32 {
    if value > u64::from(u32::MAX) {
        u32::MAX
    } else {
        value as u32
    }
}

fn status_from_u32(value: u32) -> Option<ValidationStatus> {
    Some(match value {
        1 => ValidationStatus::Ok,
        2 => ValidationStatus::InvalidInput,
        3 => ValidationStatus::InvalidScheme,
        4 => ValidationStatus::InvalidType,
        5 => ValidationStatus::InvalidMultipart,
        6 => ValidationStatus::InvalidBytewordsLength,
        7 => ValidationStatus::InvalidBytewordsWord,
        8 => ValidationStatus::InvalidChecksum,
        9 => ValidationStatus::CborUnexpectedEof,
        10 => ValidationStatus::CborInvalid,
        11 => ValidationStatus::CborNestingLimit,
        12 => ValidationStatus::CborItemLimit,
        13 => ValidationStatus::CborTrailingData,
        14 => ValidationStatus::JsonUnexpectedEof,
        15 => ValidationStatus::JsonInvalid,
        16 => ValidationStatus::JsonNestingLimit,
        17 => ValidationStatus::JsonItemLimit,
        18 => ValidationStatus::JsonTrailingData,
        19 => ValidationStatus::JsonRootType,
        20 => ValidationStatus::InputTooLarge,
        _ => return None,
    })
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
        let mut validator = CborStreamValidator::new();
        let mut index = 0usize;
        while index < source.payload_len {
            let byte = source.decoded_byte(index)?;
            validator.push(&[byte]);
            index += 1;
        }
        match validator.finish() {
            ValidationStatus::Ok => Ok(()),
            status => Err(status),
        }
    })();

    match result {
        Ok(()) => ValidationStatus::Ok,
        Err(status) => status,
    }
}

pub fn validate_cbor(value: &[u8]) -> ValidationStatus {
    if value.is_empty() {
        return ValidationStatus::InvalidInput;
    }
    let mut validator = CborStreamValidator::new();
    validator.push(value);
    validator.finish()
}

/// `validate_cbor` fed in `chunk_len`-byte pieces, exactly as the MPU sandbox
/// receives a reassembled multipart UR. Lets host tests cover chunk-boundary
/// behavior of the same state machine the device runs.
pub fn validate_cbor_chunked(value: &[u8], chunk_len: usize) -> ValidationStatus {
    if value.is_empty() {
        return ValidationStatus::InvalidInput;
    }
    let mut validator = CborStreamValidator::new();
    for chunk in value.chunks(chunk_len.max(1)) {
        validator.push(chunk);
    }
    validator.finish()
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

/// The original recursive-descent validator, kept only as the oracle that
/// the streaming implementation is checked against.
#[cfg(test)]
mod reference {
    use super::{ValidationStatus, MAX_ITEMS, MAX_NESTING_DEPTH};

    struct Reader<'a> {
        source: &'a [u8],
        position: usize,
    }

    impl Reader<'_> {
        fn read(&mut self) -> Result<u8, ValidationStatus> {
            let value = self
                .source
                .get(self.position)
                .copied()
                .ok_or(ValidationStatus::CborUnexpectedEof)?;
            self.position += 1;
            Ok(value)
        }

        fn peek(&self) -> Result<u8, ValidationStatus> {
            self.source
                .get(self.position)
                .copied()
                .ok_or(ValidationStatus::CborUnexpectedEof)
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

    fn read_argument(reader: &mut Reader, additional: u8) -> Result<Option<u64>, ValidationStatus> {
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

    fn consume_indefinite_string(
        reader: &mut Reader,
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
            let length =
                read_argument(reader, initial & 0x1f)?.ok_or(ValidationStatus::CborInvalid)?;
            reader.skip(length)?;
        }
    }

    fn validate_item(
        reader: &mut Reader,
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
                            validate_item(reader, depth, item_count)?;
                            index += 1;
                        }
                    }
                    None => loop {
                        if reader.peek()? == 0xff {
                            reader.read()?;
                            break;
                        }
                        validate_item(reader, depth, item_count)?;
                    },
                }
            }
            5 => {
                let depth = child_depth(depth)?;
                match argument {
                    Some(length) => {
                        let mut index = 0u64;
                        while index < length {
                            validate_item(reader, depth, item_count)?;
                            validate_item(reader, depth, item_count)?;
                            index += 1;
                        }
                    }
                    None => loop {
                        if reader.peek()? == 0xff {
                            reader.read()?;
                            break;
                        }
                        validate_item(reader, depth, item_count)?;
                        if reader.peek()? == 0xff {
                            return Err(ValidationStatus::CborInvalid);
                        }
                        validate_item(reader, depth, item_count)?;
                    },
                }
            }
            6 => {
                if argument.is_none() {
                    return Err(ValidationStatus::CborInvalid);
                }
                validate_item(reader, child_depth(depth)?, item_count)?;
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

    pub fn validate_cbor(value: &[u8]) -> ValidationStatus {
        if value.is_empty() {
            return ValidationStatus::InvalidInput;
        }
        let mut reader = Reader {
            source: value,
            position: 0,
        };
        let mut item_count = 0u32;
        match validate_item(&mut reader, 0, &mut item_count) {
            Ok(()) if reader.position != value.len() => ValidationStatus::CborTrailingData,
            Ok(()) => ValidationStatus::Ok,
            Err(status) => status,
        }
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

    /// Regression for the 3.1.0 large-PCZT rejection: a `zcash-pczt` UR is a
    /// map wrapping one byte string; a ~22 KiB one must validate when fed in
    /// sandbox-sized chunks, with no payload buffer involved.
    #[test]
    fn validates_large_byte_string_payload_in_chunks() {
        let payload_len = 22732usize;
        let mut cbor = vec![0xa1, 0x01, 0x5a];
        cbor.extend_from_slice(&(payload_len as u32).to_be_bytes());
        cbor.extend(core::iter::repeat(0x5a).take(payload_len));
        assert_eq!(validate_cbor(&cbor), ValidationStatus::Ok);
        for chunk in [1usize, 7, 224, 4096] {
            assert_eq!(
                validate_cbor_chunked(&cbor, chunk),
                ValidationStatus::Ok,
                "chunk {chunk}"
            );
        }
        // Truncated body: every chunking must agree it is EOF, never Ok.
        cbor.truncate(cbor.len() - 1);
        for chunk in [1usize, 7, 224, 4096] {
            assert_eq!(
                validate_cbor_chunked(&cbor, chunk),
                ValidationStatus::CborUnexpectedEof
            );
        }
        assert!(core::mem::size_of::<CborStreamValidator>() <= CborStreamValidator::STATE_SIZE);
    }

    fn corpus() -> Vec<Vec<u8>> {
        #[rustfmt::skip]
        let mut cases: Vec<Vec<u8>> = vec![
            vec![],
            vec![0x00],
            vec![0x17],
            vec![0x18, 0xff],
            vec![0x19, 0x01],                      // truncated head
            vec![0x1b, 0, 0, 0, 0, 0, 0, 0, 1],
            vec![0x1c],                            // reserved additional
            vec![0x1f],                            // major 0 with no argument
            vec![0x3f],                            // negative int, no argument
            vec![0x40],                            // empty bytes
            vec![0x43, 1, 2, 3],
            vec![0x43, 1, 2],                      // short body
            vec![0x5a, 0, 0, 0, 5, 1, 2, 3, 4, 5],
            vec![0x5b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1], // huge length
            vec![0x5f, 0x41, 1, 0x42, 2, 3, 0xff], // indefinite bytes
            vec![0x5f, 0x41, 1, 0x61, b'a', 0xff], // wrong major chunk
            vec![0x5f, 0x78],                      // wrong major chunk, truncated head
            vec![0x5f, 0x5f, 0x41, 1, 0xff, 0xff], // nested indefinite chunk
            vec![0x5f, 0x40, 0xff],                // empty chunk
            vec![0x5f, 0x41, 1],                   // unterminated
            vec![0x5f, 0xff],
            vec![0x7f, 0x61, b'x', 0xff],
            vec![0x80],
            vec![0x81, 0x01],
            vec![0x81],                            // missing element
            vec![0x82, 0x01],
            vec![0x9f, 0xff],
            vec![0x9f, 0x01, 0x02, 0xff],
            vec![0x9f, 0x01],                      // unterminated indefinite array
            vec![0xa0],
            vec![0xa1, 0x01, 0x02],
            vec![0xa1, 0x01],                      // missing value
            vec![0xbf, 0xff],
            vec![0xbf, 0x01, 0x02, 0xff],
            vec![0xbf, 0x01, 0xff],                // odd indefinite map
            vec![0xbf, 0x01],
            vec![0xc0, 0x00],
            vec![0xc0],                            // tag without content
            vec![0xdf],                            // tag with no argument
            vec![
                0xd8, 0x25, 0x50, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            ],
            vec![0xf4],
            vec![0xf6],
            vec![0xf8, 0x20],
            vec![0xf9, 0x3c, 0x00],
            vec![0xfa, 0x3f, 0x80, 0x00, 0x00],
            vec![0xfb, 0x3f, 0xf0, 0, 0, 0, 0, 0, 0],
            vec![0xff],                            // break at top level
            vec![0x81, 0xff],                      // break inside definite array
            vec![0xa0, 0x00],                      // trailing data
            vec![0x00, 0x00],
            vec![
                0xa2, 0x01, 0xd8, 0x25, 0x50, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                0x02, 0x43, 0xaa, 0xbb, 0xcc,
            ],
        ];
        // Depth limit: exactly at, one over, and empty containers at the limit.
        for extra in 0..=1usize {
            let mut nested = vec![0x81; MAX_NESTING_DEPTH as usize + extra];
            nested.push(0xf6);
            cases.push(nested);
            let mut nested_maps = vec![0xa1, 0x01];
            nested_maps.clear();
            for _ in 0..(MAX_NESTING_DEPTH as usize + extra) {
                nested_maps.push(0xa1);
                nested_maps.push(0x01);
            }
            nested_maps.push(0xf6);
            cases.push(nested_maps);
            let mut tags = vec![0xc0; MAX_NESTING_DEPTH as usize + extra];
            tags.push(0x00);
            cases.push(tags);
            let mut empty_at_limit = vec![0x81; MAX_NESTING_DEPTH as usize + extra];
            empty_at_limit.push(0x80);
            cases.push(empty_at_limit);
            let mut indef_at_limit = vec![0x9f; MAX_NESTING_DEPTH as usize + extra];
            indef_at_limit.push(0xf6);
            indef_at_limit
                .extend(core::iter::repeat(0xff).take(MAX_NESTING_DEPTH as usize + extra));
            cases.push(indef_at_limit);
        }
        // Item limit: exactly at, one over, and ending right where the next
        // item would start (ItemLimit must win over UnexpectedEof there).
        for total in [
            MAX_ITEMS as usize - 1,
            MAX_ITEMS as usize,
            MAX_ITEMS as usize + 1,
        ] {
            let mut definite = vec![0x99, (total >> 8) as u8, total as u8];
            definite.extend(core::iter::repeat(0xf6).take(total));
            cases.push(definite);
            let mut truncated = vec![0x99, (total >> 8) as u8, total as u8];
            truncated.extend(core::iter::repeat(0xf6).take(total.saturating_sub(1)));
            cases.push(truncated);
            let mut indefinite = vec![0x9f];
            indefinite.extend(core::iter::repeat(0xf6).take(total));
            indefinite.push(0xff);
            cases.push(indefinite);
            let mut unterminated = vec![0x9f];
            unterminated.extend(core::iter::repeat(0xf6).take(total));
            cases.push(unterminated);
        }
        cases
    }

    /// Every corpus entry, and every prefix of it, must give the same status
    /// from the streaming validator (at several chunk sizes) as from the
    /// recursive oracle.
    #[test]
    fn streaming_matches_recursive_oracle_on_corpus() {
        for case in corpus() {
            for end in 0..=case.len() {
                let input = &case[..end];
                let expected = reference::validate_cbor(input);
                assert_eq!(validate_cbor(input), expected, "one-shot {input:02x?}");
                for chunk in [1usize, 2, 3, 7, 224] {
                    assert_eq!(
                        validate_cbor_chunked(input, chunk),
                        expected,
                        "chunk {chunk} {input:02x?}"
                    );
                }
            }
        }
    }

    /// Pseudo-random inputs (deterministic xorshift) biased towards CBOR
    /// head bytes so that containers, tags and breaks are dense.
    #[test]
    fn streaming_matches_recursive_oracle_on_random_inputs() {
        let interesting: [u8; 24] = [
            0x00, 0x18, 0x1a, 0x1f, 0x40, 0x41, 0x58, 0x5f, 0x60, 0x7f, 0x80, 0x81, 0x83, 0x98,
            0x9f, 0xa0, 0xa1, 0xbf, 0xc0, 0xd8, 0xf4, 0xf6, 0xf9, 0xff,
        ];
        let mut state = 0x9e37_79b9_7f4a_7c15u64;
        let mut next = || {
            state ^= state << 13;
            state ^= state >> 7;
            state ^= state << 17;
            state
        };
        for _ in 0..20_000 {
            let len = (next() % 40) as usize;
            let mut input = Vec::with_capacity(len);
            for _ in 0..len {
                let r = next();
                let byte = if r % 4 == 0 {
                    (r >> 8) as u8
                } else {
                    interesting[((r >> 8) % interesting.len() as u64) as usize]
                };
                input.push(byte);
            }
            let expected = reference::validate_cbor(&input);
            assert_eq!(validate_cbor(&input), expected, "one-shot {input:02x?}");
            let chunk = 1 + (next() % 9) as usize;
            assert_eq!(
                validate_cbor_chunked(&input, chunk),
                expected,
                "chunk {chunk} {input:02x?}"
            );
        }
    }

    #[test]
    fn stream_state_can_be_reset_and_reused_in_place() {
        let mut storage = [0u64; CborStreamValidator::STATE_SIZE / 8];
        let ptr = storage.as_mut_ptr() as *mut u8;
        assert!(unsafe { CborStreamValidator::init_in_raw(ptr, storage.len() * 8) });
        let validator = unsafe { CborStreamValidator::from_raw(ptr, storage.len() * 8) }
            .expect("aligned storage of STATE_SIZE bytes");
        validator.push(&[0x81]);
        assert_eq!(validator.finish(), ValidationStatus::CborUnexpectedEof);
        validator.reset();
        validator.push(&[0x81, 0x01]);
        assert_eq!(validator.finish(), ValidationStatus::Ok);
        // Too-small or misaligned storage is refused.
        assert!(!unsafe { CborStreamValidator::init_in_raw(ptr, 8) });
        assert!(!unsafe { CborStreamValidator::init_in_raw(ptr.add(1), 512) });
        assert!(
            unsafe { CborStreamValidator::from_raw(storage.as_mut_ptr() as *mut u8, 8) }.is_none()
        );
        assert!(unsafe {
            CborStreamValidator::from_raw((storage.as_mut_ptr() as *mut u8).add(1), 512)
        }
        .is_none());
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
