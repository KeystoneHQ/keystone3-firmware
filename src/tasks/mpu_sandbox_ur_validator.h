#ifndef MPU_SANDBOX_UR_VALIDATOR_H
#define MPU_SANDBOX_UR_VALIDATOR_H

#include <stddef.h>
#include <stdint.h>

#define MPU_SANDBOX_RESOLVE_UR_COMMAND 0x0002U

/* Bytes reserved for a streaming CBOR validator state (see
 * sandbox_parser::CborStreamValidator::STATE_SIZE); must be 8-byte aligned. */
#define MPU_SANDBOX_CBOR_STREAM_STATE_SIZE 512U

typedef enum {
    MPU_SANDBOX_UR_NOT_CHECKED = 0,
    MPU_SANDBOX_UR_OK = 1,
    MPU_SANDBOX_UR_INVALID_INPUT = 2,
    MPU_SANDBOX_UR_INVALID_SCHEME = 3,
    MPU_SANDBOX_UR_INVALID_TYPE = 4,
    MPU_SANDBOX_UR_INVALID_MULTIPART = 5,
    MPU_SANDBOX_UR_INVALID_BYTEWORDS_LENGTH = 6,
    MPU_SANDBOX_UR_INVALID_BYTEWORDS_WORD = 7,
    MPU_SANDBOX_UR_INVALID_CHECKSUM = 8,
    MPU_SANDBOX_UR_CBOR_UNEXPECTED_EOF = 9,
    MPU_SANDBOX_UR_CBOR_INVALID = 10,
    MPU_SANDBOX_UR_CBOR_NESTING_LIMIT = 11,
    MPU_SANDBOX_UR_CBOR_ITEM_LIMIT = 12,
    MPU_SANDBOX_UR_CBOR_TRAILING_DATA = 13,
    MPU_SANDBOX_JSON_UNEXPECTED_EOF = 14,
    MPU_SANDBOX_JSON_INVALID = 15,
    MPU_SANDBOX_JSON_NESTING_LIMIT = 16,
    MPU_SANDBOX_JSON_ITEM_LIMIT = 17,
    MPU_SANDBOX_JSON_TRAILING_DATA = 18,
    MPU_SANDBOX_JSON_ROOT_TYPE = 19,
    /* The payload exceeds MPU_SANDBOX_*_INPUT_MAX_SIZE for this build. */
    MPU_SANDBOX_UR_INPUT_TOO_LARGE = 20,
} MpuSandboxUrValidationStatus_t;

uint32_t mpu_sandbox_validate_ur(const uint8_t *input, size_t input_len);
uint32_t mpu_sandbox_validate_cbor(const uint8_t *input, size_t input_len);
uint32_t mpu_sandbox_validate_eip712_json(uint8_t *input, size_t input_len);

/* Streaming CBOR validation over caller-owned state. `push` only accepts
 * bytes (verdicts are sticky and reported by `finish`), so the chunk protocol
 * keeps returning NOT_CHECKED until the final chunk. */
uint32_t mpu_sandbox_cbor_stream_init(uint8_t *state, size_t capacity);
uint32_t mpu_sandbox_cbor_stream_push(uint8_t *state, size_t capacity,
                                      const uint8_t *input, size_t input_len);
uint32_t mpu_sandbox_cbor_stream_finish(uint8_t *state, size_t capacity);

#endif
