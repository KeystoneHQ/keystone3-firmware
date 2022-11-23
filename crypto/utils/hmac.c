#include "hmac.h"
#include "string.h"
#include "stdlib.h"
#include <sha256.h>
#include <sha512.h>
#include <stdbool.h>

#define alignment_ok(p, n) ((size_t)(p) % (n) == 0)

#ifdef SHA_T
#undef SHA_T
#endif
#define SHA_T sha256
#define SHA_CTX_MEMBER u32
#define SHA_PRE(name) sha256 ## name
#define HMAC_FUNCTION hmac_sha256_impl
#define _HMAC_FUNCTION hmac_sha256
#include "hmac.inl"

#undef SHA_T
#define SHA_T sha512
#undef SHA_CTX_MEMBER
#define SHA_CTX_MEMBER u64
#undef SHA_PRE
#define SHA_PRE(name) sha512 ## name
#undef HMAC_FUNCTION
#define HMAC_FUNCTION hmac_sha512_impl
#undef _HMAC_FUNCTION
#define _HMAC_FUNCTION hmac_sha512
#include "hmac.inl"
