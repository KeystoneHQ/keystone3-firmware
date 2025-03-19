# -*- coding: utf-8 -*-
# !/usr/bin/python

import sys
import hashlib
import os

MAGIC_NUMBER = b'mh1903bootupdate'
APP_END_NUMBER = b'mh1903append'
SIGNATURE_LENGTH = 0x134
BLOCK_SIZE = 4096
HASH_SIZE = 32

def write_checked(f, data):
    written = f.write(data)
    if written != len(data):
        raise IOError(f"Failed to write all data: {written} != {len(data)}")

def padding_sig_file(file_name, padding_file_name):
    if not os.path.exists("boot.sig"):
        return

    with open(file_name, 'rb') as src_file:
        content = src_file.read()

    with open(padding_file_name, 'wb') as dst_file:
        dst_file.write(content)

    with open("boot.sig", "rb") as sig_file:
        sig_content = sig_file.read()
        sig_len = len(sig_content)
        print("len = ", sig_len);

        # Calculate SHA256 for the entire signature file
        sha256_obj = hashlib.sha256()
        sha256_obj.update(sig_content)
        final_hash = sha256_obj.hexdigest()
        print(f'SHA256 hash: {final_hash}')

        with open(padding_file_name, "ab") as f:
            write_checked(f, MAGIC_NUMBER)
            write_checked(f, sig_len.to_bytes(4, byteorder='big'))
            write_checked(f, bytes.fromhex(final_hash))

            # Write first 0x134 bytes of signature
            write_checked(f, sig_content[:SIGNATURE_LENGTH])

            # Calculate and pad to 4K boundary
            current_pos = len(MAGIC_NUMBER) + 4 + HASH_SIZE + SIGNATURE_LENGTH # Length + SHA256 + First part of sig + magic_number
            padding_size = BLOCK_SIZE - (current_pos % BLOCK_SIZE)
            write_checked(f, b'\xff' * padding_size)

            # Write remaining signature content
            write_checked(f, sig_content[SIGNATURE_LENGTH:])
            current_size = f.tell()
            required_padding = BLOCK_SIZE - (current_size % BLOCK_SIZE)
            write_checked(f, b'\xff' * required_padding)
            
            f.close()


def padding_bin_file(file_name):
    with open(file_name, "ab") as f:
        current_size = f.tell()
        # Calculate required padding
        required_padding = BLOCK_SIZE - (current_size % BLOCK_SIZE)
        write_checked(f, b'\xff' * required_padding)
                
        # provides source code check for the app
        write_checked(f, APP_END_NUMBER)
        print('len = ', len(APP_END_NUMBER))

        write_checked(f, b'\xff' * (BLOCK_SIZE - len(APP_END_NUMBER)))

        f.close()

        full_name = "mh1903_full.bin"
        padding_sig_file(file_name, full_name)
        
        with open(full_name, "ab") as f:
            offset = current_size % BLOCK_SIZE
            write_checked(f, offset.to_bytes(2, byteorder='big'))
            write_checked(f, b'\xff' * (BLOCK_SIZE - 2))
            f.close()


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python padding_bin_file.py [file_name]")
        sys.exit()
    file_name = sys.argv[1]
    padding_bin_file(file_name)
