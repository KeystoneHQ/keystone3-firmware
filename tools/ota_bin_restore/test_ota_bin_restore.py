import base64
import hashlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

import ota_bin_restore as ota


def raw_chunk(data, long_header=False):
    if long_header or len(data) + 3 > 255:
        return b'\x4e' + struct.pack('<II', len(data) + 9, len(data)) + data
    return bytes([0x4c, len(data) + 3, len(data)]) + data


def compressed_chunk(payload, output_size):
    payload = payload.ljust(9, b'\0')
    return b'\x4f' + struct.pack('<II', len(payload) + 9, output_size) + payload


def package(raw, chunks=None, **overrides):
    if chunks is None:
        chunks = b''.join(raw_chunk(raw[i:i + ota.CHUNK_SIZE])
                          for i in range(0, len(raw), ota.CHUNK_SIZE))
    fields = dict(mark=b'~fwdata!', size=len(chunks), original_size=len(raw),
                  compressed_hash=hashlib.sha256(chunks).digest(),
                  original_hash=hashlib.sha256(raw).digest(), encoding=1,
                  unit=ota.CHUNK_SIZE, encryption=0)
    fields.update(overrides)
    header = ota.OTA_HEADER.pack(*fields.values())
    return struct.pack('<I', len(header)) + header + b'\0' + chunks


class DecodeTests(unittest.TestCase):
    def test_independent_reference_vectors(self):
        vectors = json.loads(Path(__file__).with_name('quicklz_vectors.json').read_text())
        for vector in vectors['vectors']:
            with self.subTest(vector=vector['name']):
                chunk = base64.b64decode(vector['chunk_base64'])
                decoded = ota.decompress_chunk(chunk)
                self.assertEqual(len(decoded), vector['size'])
                self.assertEqual(hashlib.sha256(decoded).hexdigest(), vector['sha256'])
                self.assertEqual(ota.restore(package(decoded, chunk)), decoded)

    def test_raw_blocks_and_multiple_chunks(self):
        for raw in [b'x', bytes(range(100)), bytes(range(256)) * 130]:
            self.assertEqual(ota.restore(package(raw)), raw)
        self.assertEqual(ota.decompress_chunk(raw_chunk(b'x', True)), b'x')

    def test_all_five_back_reference_forms_and_overlap(self):
        tokens = [(bytes([3 << 2]), 3),
                  (struct.pack('<H', (3 << 2) | 1), 3),
                  (struct.pack('<H', (3 << 6) | ((6 - 3) << 2) | 2), 6),
                  (((3 << 7) | ((10 - 2) << 2) | 3).to_bytes(3, 'little'), 10),
                  (struct.pack('<I', (3 << 15) | ((40 - 3) << 7) | 3), 40)]
        for token, length in tokens:
            expected = b'abc' + (b'abc' * 20)[:length] + b'WXYZ'
            payload = struct.pack('<I', 0x80000008) + b'abc' + token + b'WXYZ'
            self.assertEqual(ota.decompress_chunk(compressed_chunk(payload, len(expected))), expected)

    def test_bad_chunk_headers(self):
        cases = [b'', b'\x4c', b'\x4c\x03', b'\x4f' + b'\0' * 7,
                 b'\x4c\x00\x01', b'\x4c\x02\x01', b'\x4c\x09\x01x',
                 b'\x4c\x04\x00x', b'\x4c\x05\x01xy',
                 b'\x4f' + struct.pack('<II', 10, ota.CHUNK_SIZE + 1) + b'x']
        for flags in [0, 0x48, 0x5c, 0xcc]:
            cases.append(bytes([flags, 4, 1]) + b'x')
        for case in cases:
            with self.subTest(case=case.hex()), self.assertRaises(ota.InvalidFirmware):
                ota.decompress_chunk(case)

    def test_invalid_back_reference_and_output_overrun(self):
        for token, expected in [(b'\0', 20), (b'\x10', 20),
                                (struct.pack('<I', (3 << 15) | (250 << 7) | 3), 20)]:
            payload = struct.pack('<I', 0x80000008) + b'abc' + token + b'WXYZ'
            with self.assertRaises(ota.InvalidFirmware):
                ota.decompress_chunk(compressed_chunk(payload, expected))

    def test_truncated_tokens_controls_and_trailing_payload(self):
        cases = [compressed_chunk(b'\0' * 9, 10),
                 compressed_chunk(struct.pack('<I', 0x80000000) + b'12345', 20),
                 compressed_chunk(struct.pack('<I', 0x80000000) + b'12345EXTRA', 5)]
        cases.append(b'\x4f' + struct.pack('<II', 17, 20)
                     + struct.pack('<I', 0x80000008) + b'abc\x03')
        for case in cases:
            with self.assertRaises(ota.InvalidFirmware):
                ota.decompress_chunk(case)

    def test_every_truncated_ota_is_rejected(self):
        data = package(b'firmware')
        for end in range(len(data)):
            with self.subTest(end=end), self.assertRaises(ota.InvalidFirmware):
                ota.restore(data[:end])

    def test_ota_header_bounds(self):
        data = package(b'firmware')
        for size in [0, 91, len(data), 0xFFFFFFFF]:
            with self.assertRaises(ota.InvalidFirmware):
                ota.restore(struct.pack('<I', size) + data[4:])
        for changes in [dict(mark=b'invalid!'), dict(encoding=0), dict(unit=0),
                        dict(encryption=1), dict(size=1), dict(original_size=0),
                        dict(original_size=ota.MAX_IMAGE_SIZE + 1),
                        dict(compressed_hash=b'\0' * 32), dict(original_hash=b'\0' * 32)]:
            with self.subTest(changes=changes), self.assertRaises(ota.InvalidFirmware):
                ota.restore(package(b'firmware', **changes))
        damaged = bytearray(data)
        damaged[4 + ota.OTA_HEADER.size] = 1
        with self.assertRaises(ota.InvalidFirmware):
            ota.restore(damaged)

    def test_extra_or_missing_chunk_and_total_output_bounds(self):
        raw = b'x' * ota.CHUNK_SIZE + b'y'
        for chunks in [raw_chunk(raw[:ota.CHUNK_SIZE]),
                       raw_chunk(raw[:ota.CHUNK_SIZE]) + raw_chunk(b'yz'),
                       raw_chunk(b'x') + raw_chunk(b'y')]:
            with self.assertRaises(ota.InvalidFirmware):
                ota.restore(package(raw, chunks))
        data = package(b'firmware')
        with patch.object(ota, 'MAX_IMAGE_SIZE', len(data) - 1):
            with self.assertRaises(ota.InvalidFirmware):
                ota.restore(data)

    def test_mutated_chunks_are_bounded(self):
        rng = random.Random(71)
        vectors = json.loads(Path(__file__).with_name('quicklz_vectors.json').read_text())
        original = base64.b64decode(vectors['vectors'][1]['chunk_base64'])
        for _ in range(500):
            chunk = bytearray(original)
            for _ in range(rng.randrange(1, 5)):
                chunk[rng.randrange(len(chunk))] = rng.randrange(256)
            try:
                decoded = ota.decompress_chunk(chunk)
            except ota.InvalidFirmware:
                continue
            self.assertLessEqual(len(decoded), ota.CHUNK_SIZE)


class FileTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.src = self.root / 'keystone3.bin'
        self.dst = self.root / 'uncompressed.bin'
        self.image = package(b'firmware')
        self.src.write_bytes(self.image)

    def test_success_and_existing_destination(self):
        self.dst.write_bytes(b'old')
        ota.restore_file(self.src, self.dst)
        self.assertEqual(self.dst.read_bytes(), b'firmware')
        self.assertEqual(self.src.read_bytes(), self.image)

    def test_same_path_symlink_and_hardlink_preserve_input(self):
        for alias in [self.src, self.root / 'link', self.root / 'hardlink']:
            if alias.name == 'link':
                alias.symlink_to(self.src)
            elif alias.name == 'hardlink':
                os.link(self.src, alias)
            with self.assertRaises(ota.InvalidFirmware):
                ota.restore_file(self.src, alias)
            self.assertEqual(self.src.read_bytes(), self.image)

    def test_default_output_cannot_overwrite_input(self):
        self.dst.write_bytes(self.image)
        result = subprocess.run([sys.executable, str(Path(ota.__file__)), str(self.dst)], capture_output=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn(b'different files', result.stderr)
        self.assertEqual(self.dst.read_bytes(), self.image)

    def test_corruption_leaves_destination_untouched(self):
        self.dst.write_bytes(b'old')
        self.src.write_bytes(b'bad')
        with self.assertRaises(ota.InvalidFirmware):
            ota.restore_file(self.src, self.dst)
        self.assertEqual(self.dst.read_bytes(), b'old')
        self.assertEqual(list(self.root.glob('.ota-restore-*')), [])

    def test_failed_flush_or_replace_leaves_destination_untouched(self):
        for target in ['fsync', 'replace']:
            self.dst.write_bytes(b'old')
            with patch.object(ota.os, target, side_effect=OSError('disk failure')):
                with self.assertRaises(OSError):
                    ota.restore_file(self.src, self.dst)
            self.assertEqual(self.dst.read_bytes(), b'old')
            self.assertEqual(list(self.root.glob('.ota-restore-*')), [])

    def test_cli_default_output_and_invalid_arguments(self):
        command = [sys.executable, str(Path(ota.__file__))]
        result = subprocess.run(command + [str(self.src)], capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(self.dst.read_bytes(), b'firmware')
        self.assertEqual(subprocess.run(command, capture_output=True).returncode, 2)
        result = subprocess.run(command + [str(self.root / 'missing')], capture_output=True)
        self.assertEqual(result.returncode, 1)
        self.assertNotIn(b'Traceback', result.stderr)


if __name__ == '__main__':
    unittest.main()
