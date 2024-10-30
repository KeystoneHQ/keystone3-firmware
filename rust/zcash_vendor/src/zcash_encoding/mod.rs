use alloc::vec::Vec;
use byteorder::LittleEndian;
use core::iter::FromIterator;
use core2::io::{self, Read, Write};

mod byteorder_io;

use self::byteorder_io::{ReadBytesExt, WriteBytesExt};

/// The maximum allowed value representable as a `[CompactSize]`
pub const MAX_COMPACT_SIZE: u32 = 0x02000000;

/// Namespace for functions for compact encoding of integers.
///
/// This codec requires integers to be in the range `0x0..=0x02000000`, for compatibility
/// with Zcash consensus rules.
pub struct CompactSize;

impl CompactSize {
    /// Reads an integer encoded in compact form.
    pub fn read<R: Read>(mut reader: R) -> io::Result<u64> {
        let flag = reader.read_u8()?;
        let result = if flag < 253 {
            Ok(flag as u64)
        } else if flag == 253 {
            match reader.read_u16::<LittleEndian>()? {
                n if n < 253 => Err(io::Error::new(
                    io::ErrorKind::InvalidInput,
                    "non-canonical CompactSize",
                )),
                n => Ok(n as u64),
            }
        } else if flag == 254 {
            match reader.read_u32::<LittleEndian>()? {
                n if n < 0x10000 => Err(io::Error::new(
                    io::ErrorKind::InvalidInput,
                    "non-canonical CompactSize",
                )),
                n => Ok(n as u64),
            }
        } else {
            match reader.read_u64::<LittleEndian>()? {
                n if n < 0x100000000 => Err(io::Error::new(
                    io::ErrorKind::InvalidInput,
                    "non-canonical CompactSize",
                )),
                n => Ok(n),
            }
        }?;

        match result {
            s if s > <u64>::from(MAX_COMPACT_SIZE) => Err(io::Error::new(
                io::ErrorKind::InvalidInput,
                "CompactSize too large",
            )),
            s => Ok(s),
        }
    }

    /// Reads an integer encoded in contact form and performs checked conversion
    /// to the target type.
    pub fn read_t<R: Read, T: TryFrom<u64>>(mut reader: R) -> io::Result<T> {
        let n = Self::read(&mut reader)?;
        <T>::try_from(n).map_err(|_| {
            io::Error::new(
                io::ErrorKind::InvalidInput,
                "CompactSize value exceeds range of target type.",
            )
        })
    }

    /// Writes the provided `usize` value to the provided Writer in compact form.
    pub fn write<W: Write>(mut writer: W, size: usize) -> io::Result<()> {
        match size {
            s if s < 253 => writer.write_u8(s as u8),
            s if s <= 0xFFFF => {
                writer.write_u8(253)?;
                writer.write_u16::<LittleEndian>(s as u16)
            }
            s if s <= 0xFFFFFFFF => {
                writer.write_u8(254)?;
                writer.write_u32::<LittleEndian>(s as u32)
            }
            s => {
                writer.write_u8(255)?;
                writer.write_u64::<LittleEndian>(s as u64)
            }
        }
    }

    /// Returns the number of bytes needed to encode the given size in compact form.
    pub fn serialized_size(size: usize) -> usize {
        match size {
            s if s < 253 => 1,
            s if s <= 0xFFFF => 3,
            s if s <= 0xFFFFFFFF => 5,
            _ => 9,
        }
    }
}

/// Namespace for functions that perform encoding of vectors.
///
/// The length of a vector is restricted to at most `0x02000000`, for compatibility with
/// the Zcash consensus rules.
pub struct Vector;

impl Vector {
    /// Reads a vector, assuming the encoding written by [`Vector::write`], using the provided
    /// function to decode each element of the vector.
    pub fn read<R: Read, E, F>(reader: R, func: F) -> io::Result<Vec<E>>
    where
        F: Fn(&mut R) -> io::Result<E>,
    {
        Self::read_collected(reader, func)
    }

    /// Reads a CompactSize-prefixed series of elements into a collection, assuming the encoding
    /// written by [`Vector::write`], using the provided function to decode each element.
    pub fn read_collected<R: Read, E, F, O: FromIterator<E>>(
        reader: R,
        func: F,
    ) -> io::Result<O>
    where
        F: Fn(&mut R) -> io::Result<E>,
    {
        Self::read_collected_mut(reader, func)
    }

    /// Reads a CompactSize-prefixed series of elements into a collection, assuming the encoding
    /// written by [`Vector::write`], using the provided function to decode each element.
    pub fn read_collected_mut<R: Read, E, F, O: FromIterator<E>>(
        mut reader: R,
        func: F,
    ) -> io::Result<O>
    where
        F: FnMut(&mut R) -> io::Result<E>,
    {
        let count: usize = CompactSize::read_t(&mut reader)?;
        Array::read_collected_mut(reader, count, func)
    }

    /// Writes a slice of values by writing [`CompactSize`]-encoded integer specifying the length
    /// of the slice to the stream, followed by the encoding of each element of the slice as
    /// performed by the provided function.
    pub fn write<W: Write, E, F>(writer: W, vec: &[E], func: F) -> io::Result<()>
    where
        F: Fn(&mut W, &E) -> io::Result<()>,
    {
        Self::write_sized(writer, vec.iter(), func)
    }

    /// Writes an iterator of values by writing [`CompactSize`]-encoded integer specifying
    /// the length of the iterator to the stream, followed by the encoding of each element
    /// of the iterator as performed by the provided function.
    pub fn write_sized<W: Write, E, F, I: Iterator<Item = E> + ExactSizeIterator>(
        mut writer: W,
        mut items: I,
        func: F,
    ) -> io::Result<()>
    where
        F: Fn(&mut W, E) -> io::Result<()>,
    {
        CompactSize::write(&mut writer, items.len())?;
        items.try_for_each(|e| func(&mut writer, e))
    }

    /// Returns the serialized size of a vector of `u8` as written by `[Vector::write]`.
    pub fn serialized_size_of_u8_vec(vec: &[u8]) -> usize {
        let length = vec.len();
        CompactSize::serialized_size(length) + length
    }
}

/// Namespace for functions that perform encoding of array contents.
///
/// This is similar to the [`Vector`] encoding except that no length information is
/// written as part of the encoding, so length must be statically known or obtained from
/// other parts of the input stream.
pub struct Array;

impl Array {
    /// Reads `count` elements from a stream into a vector, assuming the encoding written by
    /// [`Array::write`], using the provided function to decode each element.
    pub fn read<R: Read, E, F>(reader: R, count: usize, func: F) -> io::Result<Vec<E>>
    where
        F: Fn(&mut R) -> io::Result<E>,
    {
        Self::read_collected(reader, count, func)
    }

    /// Reads `count` elements into a collection, assuming the encoding written by
    /// [`Array::write`], using the provided function to decode each element.
    pub fn read_collected<R: Read, E, F, O: FromIterator<E>>(
        reader: R,
        count: usize,
        func: F,
    ) -> io::Result<O>
    where
        F: Fn(&mut R) -> io::Result<E>,
    {
        Self::read_collected_mut(reader, count, func)
    }

    /// Reads `count` elements into a collection, assuming the encoding written by
    /// [`Array::write`], using the provided function to decode each element.
    pub fn read_collected_mut<R: Read, E, F, O: FromIterator<E>>(
        mut reader: R,
        count: usize,
        mut func: F,
    ) -> io::Result<O>
    where
        F: FnMut(&mut R) -> io::Result<E>,
    {
        (0..count).map(|_| func(&mut reader)).collect()
    }

    /// Writes an iterator full of values to a stream by sequentially
    /// encoding each element using the provided function.
    pub fn write<W: Write, E, I: IntoIterator<Item = E>, F>(
        mut writer: W,
        vec: I,
        func: F,
    ) -> io::Result<()>
    where
        F: Fn(&mut W, &E) -> io::Result<()>,
    {
        vec.into_iter().try_for_each(|e| func(&mut writer, &e))
    }
}

/// Namespace for functions that perform encoding of [`Option`] values.
pub struct Optional;

impl Optional {
    /// Reads an optional value, assuming the encoding written by [`Optional::write`], using the
    /// provided function to decode the contained element if present.
    pub fn read<R: Read, T, F>(mut reader: R, func: F) -> io::Result<Option<T>>
    where
        F: Fn(R) -> io::Result<T>,
    {
        match reader.read_u8()? {
            0 => Ok(None),
            1 => Ok(Some(func(reader)?)),
            _ => Err(io::Error::new(
                io::ErrorKind::InvalidInput,
                "non-canonical Option<T>",
            )),
        }
    }

    /// Writes an optional value to a stream by writing a flag byte with a value of 0 if no value
    /// is present, or 1 if there is a value, followed by the encoding of the contents of the
    /// option as performed by the provided function.
    pub fn write<W: Write, T, F>(mut writer: W, val: Option<T>, func: F) -> io::Result<()>
    where
        F: Fn(W, T) -> io::Result<()>,
    {
        match val {
            None => writer.write_u8(0),
            Some(e) => {
                writer.write_u8(1)?;
                func(writer, e)
            }
        }
    }
}
