# -*- coding: utf-8 -*-
# !/usr/bin/python

import sys
import hashlib


def padding_sig_file(f):
    # Read and process boot.sig file
    with open("boot.sig", "rb") as sig_file:
        # Read entire file content
        sig_content = sig_file.read()
        sig_len = len(sig_content)
        print(f"Signature length: {sig_len}")
        f.write(sig_len.to_bytes(4, byteorder='big'))

        # Calculate SHA256 for the entire file
        sha256_obj = hashlib.sha256()
        sha256_obj.update(sig_content)
        final_hash = sha256_obj.hexdigest()
        print(f'SHA256 hash: {final_hash}')
        f.write(bytes.fromhex(final_hash))

        # Write first 0x134 bytes
        f.write(sig_content[:0x134])

        # Pad with 0xFF up to 4K boundary
        padding_size = 4096 - 0x134 - 32 - 4
        f.write(b'\xff' * padding_size)

        # Write remaining content
        f.write(sig_content[0x134:])


def padding_bin_file(file_name):
    with open(file_name, "ab") as f:
        current_size = f.tell()
        # Calculate required padding
        required_padding = 4096 - (current_size % 4096)
        f.write(b'\xff' * required_padding)

        # Write offset and padding
        offset = current_size % 4096
        f.write(offset.to_bytes(2, byteorder='big'))
        f.write(b'\xff' * (4096 - 2))

        # padding_sig_file(f)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python padding_bin_file.py [file_name]")
        sys.exit()
    file_name = sys.argv[1]
    padding_bin_file(file_name)
