use core::num::NonZeroU32;

use getrandom::{register_custom_getrandom, Error};

use crate::bindings::GenerateTRNGRandomness;

/// Same as in `keystore.h`.
const ENTROPY_MAX_LEN: usize = 32;

fn keystone_getrandom(dest: &mut [u8]) -> Result<(), Error> {
    for chunk in dest.chunks_mut(ENTROPY_MAX_LEN) {
        // SAFETY: `chunk.len()` is at most `ENTROPY_MAX_LEN` which fits in `u8`.
        let len = chunk.len() as u8;

        let ret = unsafe { GenerateTRNGRandomness(chunk.as_mut_ptr(), len) };

        // TODO: Determine how to correctly map error codes from the underlying hardware
        // into the `getrandom` custom error code space `Error::CUSTOM_START..=u32::MAX`.
        if ret != 0 {
            let error = NonZeroU32::new(Error::CUSTOM_START.saturating_add_signed(ret)).unwrap();
            return Err(Error::from(error));
        }
    }

    Ok(())
}

register_custom_getrandom!(keystone_getrandom);
