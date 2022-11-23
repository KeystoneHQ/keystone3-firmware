#ifndef _HMAC_H
#define _HMAC_H

#include "stdlib.h"

struct sha256;
struct sha512;

/**
 * hmac_sha256 - Compute an HMAC using SHA-256
 *
 * @sha: Destination for the resulting HMAC.
 * @key: The key for the hash
 * @key_len: The length of @key in bytes.
 * @msg: The message to hash
 * @msg_len: The length of @msg in bytes.
 */
void hmac_sha256_impl(struct sha256 *sha,
                      const unsigned char *key, size_t key_len,
                      const unsigned char *msg, size_t msg_len);

/**
 * hmac_sha512 - Compute an HMAC using SHA-512
 *
 * @sha: Destination for the resulting HMAC.
 * @key: The key for the hash
 * @key_len: The length of @key in bytes.
 * @msg: The message to hash
 * @msg_len: The length of @msg in bytes.
 */
void hmac_sha512_impl(struct sha512 *sha,
                      const unsigned char *key, size_t key_len,
                      const unsigned char *msg, size_t msg_len);


/**
 * Compute an HMAC using SHA-256.
 *
 * :param key: The key for the hash.
 * :param key_len: The length of ``key`` in bytes.
 * :param bytes: The message to hash.
 * :param bytes_len: The length of ``bytes`` in bytes.
 * :param bytes_out: Destination for the resulting HMAC.
 */
int hmac_sha256(
    const unsigned char *key,
    size_t key_len,
    const unsigned char *bytes,
    size_t bytes_len,
    unsigned char *bytes_out);


/**
 * Compute an HMAC using SHA-512.
 *
 * :param key: The key for the hash.
 * :param key_len: The length of ``key`` in bytes.
 * :param bytes: The message to hash.
 * :param bytes_len: The length of ``bytes`` in bytes.
 * :param bytes_out: Destination for the resulting HMAC.
 */
int hmac_sha512(
    const unsigned char *key,
    size_t key_len,
    const unsigned char *bytes,
    size_t bytes_len,
    unsigned char *bytes_out);

#endif
