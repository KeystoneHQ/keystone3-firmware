//! The device random number generator, for crates that use the `rand_core`
//! 0.10 traits.

/// The device RNG (`rand_core::OsRng`, which reads the hardware TRNG on the
/// device), behind the `rand_core` 0.10 traits that orchard and the
/// transaction builder require.
///
/// Like `OsRng`, it panics if the TRNG fails.
pub(crate) struct DeviceRng;

impl rand_core_010::TryRng for DeviceRng {
    type Error = core::convert::Infallible;

    fn try_next_u32(&mut self) -> Result<u32, Self::Error> {
        Ok(rand_core::RngCore::next_u32(&mut rand_core::OsRng))
    }

    fn try_next_u64(&mut self) -> Result<u64, Self::Error> {
        Ok(rand_core::RngCore::next_u64(&mut rand_core::OsRng))
    }

    fn try_fill_bytes(&mut self, dst: &mut [u8]) -> Result<(), Self::Error> {
        rand_core::RngCore::fill_bytes(&mut rand_core::OsRng, dst);
        Ok(())
    }
}

impl rand_core_010::TryCryptoRng for DeviceRng {}

#[cfg(test)]
mod tests {
    use super::DeviceRng;
    use rand_core_010::Rng;

    #[test]
    fn fills_bytes() {
        let mut a = [0u8; 32];
        let mut b = [0u8; 32];
        DeviceRng.fill_bytes(&mut a);
        DeviceRng.fill_bytes(&mut b);
        assert_ne!(a, b);
    }
}
