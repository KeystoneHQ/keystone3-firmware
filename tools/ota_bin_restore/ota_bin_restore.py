#!/usr/bin/env python3
"""Restore a Keystone OTA image using bounded QuickLZ level-3 decoding.

Usage: python3 ota_bin_restore.py <keystone3.bin> [uncompressed.bin]
Uses only the Python standard library. The default output is uncompressed.bin
next to the input. Input and output must be different files.

Lengths and SHA-256 hashes are checked, but the OTA signature is NOT verified;
this extraction tool does not establish firmware authenticity.
"""

import argparse
import hashlib
import os
import struct
import sys
import tempfile
from pathlib import Path

MAX_IMAGE_SIZE = 64 * 1024 * 1024
CHUNK_SIZE = 16384
OTA_HEADER = struct.Struct(">8sII32s32sIII")


class InvalidFirmware(ValueError):
    pass


def chunk_header(source):
    if not source:
        raise InvalidFirmware("missing QuickLZ header")
    flags = source[0]
    header_size = 9 if flags & 2 else 3
    if len(source) < header_size:
        raise InvalidFirmware("truncated QuickLZ header")
    if (flags & 0xFC) != 0x4C:
        raise InvalidFirmware("unsupported QuickLZ flags (requires level 3, no streaming)")
    width = 4 if flags & 2 else 1
    compressed = int.from_bytes(source[1:1 + width], "little")
    decompressed = int.from_bytes(source[1 + width:header_size], "little")
    if not header_size < compressed <= len(source):
        raise InvalidFirmware("invalid QuickLZ compressed length")
    if not 0 < decompressed <= CHUNK_SIZE:
        raise InvalidFirmware("invalid QuickLZ decompressed length")
    if not (flags & 1) and compressed != header_size + decompressed:
        raise InvalidFirmware("uncompressed QuickLZ block length mismatch")
    return header_size, compressed, decompressed


def decompress_chunk(chunk):
    header_size, compressed, expected = chunk_header(chunk)
    if compressed != len(chunk):
        raise InvalidFirmware("trailing data after QuickLZ chunk")
    if not (chunk[0] & 1):
        return bytes(chunk[header_size:])
    position = header_size
    control = 1
    output = bytearray()

    def read(width):
        nonlocal position
        if position + width > compressed:
            raise InvalidFirmware("truncated QuickLZ token")
        value = int.from_bytes(chunk[position:position + width], "little")
        position += width
        return value

    while len(output) < expected:
        if control == 1:
            control = read(4)
            if not (control & (1 << 31)):
                raise InvalidFirmware("invalid QuickLZ control word")
        match = control & 1
        control >>= 1
        if not match:
            output.append(read(1))
            continue
        first = read(1)
        if (first & 3) == 0:
            distance, length = first >> 2, 3
        elif (first & 2) == 0:
            value = first | (read(1) << 8)
            distance, length = value >> 2, 3
        elif (first & 1) == 0:
            value = first | (read(1) << 8)
            distance, length = value >> 6, ((value >> 2) & 15) + 3
        elif (first & 127) != 3:
            value = first | (read(2) << 8)
            distance, length = value >> 7, ((value >> 2) & 31) + 2
        else:
            value = first | (read(3) << 8)
            distance, length = value >> 15, ((value >> 7) & 255) + 3
        if not 3 <= distance <= len(output):
            raise InvalidFirmware("invalid QuickLZ back-reference")
        if len(output) + length > expected - 4:
            raise InvalidFirmware("QuickLZ match exceeds output boundary")
        for _ in range(length):
            output.append(output[-distance])
    if compressed != max(position, header_size + 9):
        raise InvalidFirmware("unexpected trailing QuickLZ payload")
    return bytes(output)


def restore(content):
    if not 4 <= len(content) <= MAX_IMAGE_SIZE:
        raise InvalidFirmware("invalid OTA file size")
    header_size = struct.unpack_from("<I", content)[0]
    data_offset = 4 + header_size + 1
    if header_size < OTA_HEADER.size or data_offset >= len(content):
        raise InvalidFirmware("invalid OTA header length")
    (mark, compressed_size, original_size, compressed_hash, original_hash,
     encoding, unit, encryption) = OTA_HEADER.unpack_from(content, 4)
    if mark != b"~fwdata!":
        raise InvalidFirmware("invalid OTA magic")
    if (encoding, unit, encryption) != (1, CHUNK_SIZE, 0):
        raise InvalidFirmware("unsupported OTA encoding, chunk size or encryption")
    if content[data_offset - 1] != 0:
        raise InvalidFirmware("invalid OTA padding byte")
    if not 0 < original_size <= MAX_IMAGE_SIZE:
        raise InvalidFirmware("invalid OTA original size")
    data = memoryview(content)[data_offset:]
    if compressed_size != len(data):
        raise InvalidFirmware("OTA compressed size mismatch")
    if hashlib.sha256(data).digest() != compressed_hash:
        raise InvalidFirmware("OTA compressed hash mismatch")
    output = bytearray()
    offset = 0
    while offset < len(data):
        _, size, expected = chunk_header(data[offset:])
        if expected != min(CHUNK_SIZE, original_size - len(output)):
            raise InvalidFirmware("OTA chunk output size mismatch")
        output.extend(decompress_chunk(data[offset:offset + size]))
        offset += size
    if len(output) != original_size:
        raise InvalidFirmware("OTA original size mismatch")
    if hashlib.sha256(output).digest() != original_hash:
        raise InvalidFirmware("OTA original hash mismatch")
    return bytes(output)


def restore_file(src, dst):
    src, dst = Path(src), Path(dst)
    if src.resolve() == dst.resolve() or (dst.exists() and os.path.samefile(src, dst)):
        raise InvalidFirmware("input and output must be different files")
    with src.open("rb") as source:
        content = source.read(MAX_IMAGE_SIZE + 1)
    restored = restore(content)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(
            dir=dst.parent, prefix=".ota-restore-", delete=False
        ) as out:
            temporary = out.name
            out.write(restored)
            out.flush()
            os.fsync(out.fileno())
        os.replace(temporary, dst)
        temporary = None
    finally:
        if temporary is not None:
            os.unlink(temporary)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", nargs="?", type=Path)
    args = parser.parse_args(argv)
    dst = (args.destination if args.destination is not None
           else args.source.with_name("uncompressed.bin"))
    try:
        restore_file(args.source, dst)
    except (InvalidFirmware, OSError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
