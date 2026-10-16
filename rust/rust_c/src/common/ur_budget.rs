use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;

const MAX_PARTS: usize = 256;
const MAX_WORK: usize = 512 * 1024;
const MAX_UR_TEXT_LEN: usize = 1536;

#[cfg(feature = "tron")]
const MAX_TRON_CBOR_BYTES: usize = app_tron::MAX_SIGN_DATA_LEN + 1024;
#[cfg(feature = "tron")]
const MAX_TRON_PARTS: usize = 128;
#[cfg(feature = "tron")]
const MAX_TRON_WORK: usize = 192 * 1024;

#[cfg(feature = "tron")]
pub(super) fn is_tron(value: &str) -> bool {
    let prefix = "ur:tron-sign-request/";
    value
        .get(..prefix.len())
        .is_some_and(|s| s.eq_ignore_ascii_case(prefix))
}

pub(super) fn validate_text_len(value: &str) -> Result<(), String> {
    if value.len() > MAX_UR_TEXT_LEN {
        return Err(format!("UR fragment exceeds {MAX_UR_TEXT_LEN}-byte limit"));
    }
    Ok(())
}

#[derive(Default)]
pub(super) struct FragmentBudget {
    identity: Option<(String, u32, u32, u32, usize)>,
    work: usize,
    seen_sequences: Vec<u32>,
    failed: bool,
}

impl FragmentBudget {
    pub(super) fn receive(&mut self, value: &str) -> Result<(), String> {
        if self.failed {
            return Err("UR session rejected; restart scanning".into());
        }
        let result = self.check(value);
        if result.is_err() {
            self.failed = true;
        }
        result
    }

    fn check(&mut self, value: &str) -> Result<(), String> {
        validate_text_len(value)?;
        let prefix = value.split('/').next().ok_or("missing UR type")?;
        let limit = if cfg!(feature = "cypherpunk")
            && (prefix.eq_ignore_ascii_case("ur:zcash-pczt")
                || prefix.eq_ignore_ascii_case("ur:zcash-sign-batch"))
        {
            32 * 1024
        } else {
            14 * 1024
        };
        let (max_parts, max_work) = (MAX_PARTS, MAX_WORK);
        #[cfg(feature = "tron")]
        let (limit, max_parts, max_work) = if is_tron(value) {
            (MAX_TRON_CBOR_BYTES, MAX_TRON_PARTS, MAX_TRON_WORK)
        } else {
            (limit, max_parts, max_work)
        };
        #[cfg(feature = "monero")]
        let (limit, max_parts, max_work) = if prefix
            .get(..7)
            .is_some_and(|s| s.eq_ignore_ascii_case("ur:xmr-"))
        {
            (app_monero::utils::io::MAX_REQUEST_BYTES, 128, 192 * 1024)
        } else {
            (limit, max_parts, max_work)
        };
        let (kind, cbor) = ur::decode(value).map_err(|_| "invalid UR fragment")?;
        if kind != ur::ur::Kind::MultiPart {
            return Err("expected multipart UR".into());
        }
        let mut d = minicbor::Decoder::new(&cbor);
        let mut read = || -> Result<_, minicbor::decode::Error> {
            if d.array()? != Some(5) {
                return Err(minicbor::decode::Error::message("invalid fountain part"));
            }
            Ok((d.u32()?, d.u32()?, d.u32()?, d.u32()?, d.bytes()?.len()))
        };
        let (sequence, parts, message_len, checksum, bytes) =
            read().map_err(|_| "invalid fountain metadata")?;
        if message_len as usize > limit {
            return Err(format!(
                "reconstructed CBOR exceeds device size limit: type={}, bytes={}, limit={}",
                prefix, message_len, limit
            ));
        }
        if sequence == 0
            || parts == 0
            || parts as usize > max_parts
            || message_len == 0
            || bytes == 0
            || d.position() != cbor.len()
            || message_len as usize > parts as usize * bytes
            || message_len as usize <= (parts as usize - 1) * bytes
        {
            return Err(format!("invalid fountain dimensions: sequence={}, parts={} (max {}), message_bytes={}, fragment_bytes={}", sequence, parts, max_parts, message_len, bytes));
        }
        if let Some((ref old_type, old_parts, old_length, old_checksum, old_bytes)) = self.identity
        {
            if !old_type.eq_ignore_ascii_case(prefix)
                || old_parts != parts
                || old_length != message_len
                || old_checksum != checksum
                || old_bytes != bytes
            {
                return Err("UR fragment changed session metadata".into());
            }
        } else {
            self.identity = Some((prefix.to_string(), parts, message_len, checksum, bytes));
        }
        if self.seen_sequences.contains(&sequence) {
            return Ok(());
        }
        let index_bound = if sequence <= parts { 1 } else { parts as usize };
        self.work = self
            .work
            .checked_add(bytes + index_bound * 8 + 128)
            .ok_or("UR fragment budget overflow")?;
        if self.work > max_work {
            return Err("UR fragment work budget exhausted; restart scanning".into());
        }
        self.seen_sequences.push(sequence);
        Ok(())
    }
}
