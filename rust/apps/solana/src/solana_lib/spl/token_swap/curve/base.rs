use crate::solana_lib::solana_program::errors::ProgramError;
use crate::solana_lib::solana_program::program_pack::{Pack, Sealed};
use crate::solana_lib::spl::token_swap::curve::calculator::CurveCalculator;
use crate::solana_lib::spl::token_swap::curve::constant_price::ConstantPriceCurve;
use crate::solana_lib::spl::token_swap::curve::constant_product::ConstantProductCurve;
use crate::solana_lib::spl::token_swap::curve::offset::OffsetCurve;
use crate::solana_lib::spl::token_swap::curve::stable::StableCurve;
use alloc::sync::Arc;
use arrayref::{array_mut_ref, array_ref, array_refs, mut_array_refs};

/// Curve types supported by the token-swap program.
#[cfg_attr(feature = "fuzz", derive(Arbitrary))]
#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum CurveType {
    /// Uniswap-style constant product curve, invariant = token_a_amount * token_b_amount
    ConstantProduct,
    /// Flat line, always providing 1:1 from one token to another
    ConstantPrice,
    /// Stable, like uniswap, but with wide zone of 1:1 instead of one point
    Stable,
    /// Offset curve, like Uniswap, but the token B side has a faked offset
    Offset,
}

/// Concrete struct to wrap around the trait object which performs calculation.
#[repr(C)]
#[derive(Debug)]
pub struct SwapCurve {
    /// The type of curve contained in the calculator, helpful for outside
    /// queries
    pub curve_type: CurveType,
    /// The actual calculator, represented as a trait object to allow for many
    /// different types of curves
    pub calculator: Arc<dyn CurveCalculator + Sync + Send>,
}

impl Sealed for SwapCurve {}
impl Pack for SwapCurve {
    /// Size of encoding of all curve parameters, which include fees and any other
    /// constants used to calculate swaps, deposits, and withdrawals.
    /// This includes 1 byte for the type, and 72 for the calculator to use as
    /// it needs.  Some calculators may be smaller than 72 bytes.
    const LEN: usize = 33;

    /// Unpacks a byte buffer into a SwapCurve
    fn unpack_from_slice(input: &[u8]) -> Result<Self, ProgramError> {
        let input = array_ref![input, 0, 33];
        #[allow(clippy::ptr_offset_with_cast)]
        let (curve_type, calculator) = array_refs![input, 1, 32];
        let curve_type = curve_type[0].try_into()?;
        Ok(Self {
            curve_type,
            calculator: match curve_type {
                CurveType::ConstantProduct => {
                    Arc::new(ConstantProductCurve::unpack_from_slice(calculator)?)
                }
                CurveType::ConstantPrice => {
                    Arc::new(ConstantPriceCurve::unpack_from_slice(calculator)?)
                }
                CurveType::Stable => Arc::new(StableCurve::unpack_from_slice(calculator)?),
                CurveType::Offset => Arc::new(OffsetCurve::unpack_from_slice(calculator)?),
            },
        })
    }

    /// Pack SwapCurve into a byte buffer
    fn pack_into_slice(&self, output: &mut [u8]) {
        let output = array_mut_ref![output, 0, 33];
        let (curve_type, calculator) = mut_array_refs![output, 1, 32];
        curve_type[0] = self.curve_type as u8;
        self.calculator.pack_into_slice(&mut calculator[..]);
    }
}

impl TryFrom<u8> for CurveType {
    type Error = ProgramError;

    fn try_from(curve_type: u8) -> Result<Self, Self::Error> {
        match curve_type {
            0 => Ok(CurveType::ConstantProduct),
            1 => Ok(CurveType::ConstantPrice),
            2 => Ok(CurveType::Stable),
            3 => Ok(CurveType::Offset),
            _ => Err(ProgramError::InvalidAccountData),
        }
    }
}
