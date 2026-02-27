#include "string.h"
#include "stdlib.h"
#include "hmac.h"
#include "endian.h"
#include "sha256.h"
#include "sha512.h"
#include "user_memory.h"

#define alignment_ok(p, n) ((size_t)(p) % (n) == 0)

#ifdef SHA_T
#undef SHA_T
#endif
#define SHA_T sha256
#define SHA_ALIGN_T uint32_t
#define SHA_MEM u32
#define SHA_POST(name) name ## sha256
#define SHA_POST_IMPL(name) name ## sha256_impl
#define PBKDF2_HMAC_SHA_LEN 32
#include "pbkdf2.inl"

#undef SHA_T
#define SHA_T sha512
#undef SHA_ALIGN_T
#define SHA_ALIGN_T uint64_t
#undef SHA_MEM
#define SHA_MEM u64
#undef SHA_POST
#define SHA_POST(name) name ## sha512
#undef SHA_POST_IMPL
#define SHA_POST_IMPL(name) name ## sha512_impl
#undef PBKDF2_HMAC_SHA_LEN
#define PBKDF2_HMAC_SHA_LEN 64
#include "pbkdf2.inl"
