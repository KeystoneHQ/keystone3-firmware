# -*- coding: utf-8 -*-
# !/usr/bin/python

import sys
import hashlib
import os


def padding_sig_file(file_name):
    if not os.path.exists("boot.sig"):
        return

    new_file_name = "mh1903_padding_boot.bin"

    with open(file_name, 'rb') as src_file:
        content = src_file.read()

    with open(new_file_name, 'wb') as dst_file:
        dst_file.write(content)

    with open("boot.sig", "rb") as sig_file:
        sig_content = sig_file.read()
        sig_len = len(sig_content)

        # Calculate SHA256 for the entire signature file
        sha256_obj = hashlib.sha256()
        sha256_obj.update(sig_content)
        final_hash = sha256_obj.hexdigest()
        print(f'SHA256 hash: {final_hash}')

        with open(new_file_name, "ab") as f:
            f.write(sig_len.to_bytes(4, byteorder='big'))
            f.write(bytes.fromhex(final_hash))

            # Write first 0x134 bytes of signature
            f.write(sig_content[:0x134])

            # Calculate and pad to 4K boundary
            current_pos = 4 + 32 + 0x134  # Length + SHA256 + First part of sig
            padding_size = 4096 - (current_pos % 4096)
            f.write(b'\xff' * padding_size)

            # Write remaining signature content
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

        f.close()

        padding_sig_file(file_name)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python padding_bin_file.py [file_name]")
        sys.exit()
    file_name = sys.argv[1]
    padding_bin_file(file_name)
