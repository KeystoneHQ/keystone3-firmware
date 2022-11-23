#ifndef __PBKDF2_H
#define __PBKDF2_H

#include "stdlib.h"
#include "stdint.h"

/**
 * Derive a pseudorandom key from inputs using HMAC SHA-512.
 *
 * :param pass: Password to derive from.
 * :param pass_len: Length of ``pass`` in bytes.
 * :param salt: Salt to derive from.
 * :param salt_len: Length of ``salt`` in bytes.
 * :param flags: Reserved, must be 0.
 * :param cost: The cost of the function. The larger this number, the
 *|     longer the key will take to derive.
 * :param bytes_out: Destination for the derived pseudorandom key.
 * FIXED_SIZED_OUTPUT(len, bytes_out, PBKDF2_HMAC_SHA512_LEN)
 */
int pbkdf2_hmac_sha512(
    const unsigned char *pass,
    size_t pass_len,
    const unsigned char *salt,
    size_t salt_len,
    uint32_t flags,
    uint32_t cost,
    unsigned char *bytes_out,
    size_t len);

#endif
