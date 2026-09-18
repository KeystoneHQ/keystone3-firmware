#ifndef MPU_SANDBOX_TASK_H
#define MPU_SANDBOX_TASK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    MPU_SANDBOX_OP_PING = 1,
    MPU_SANDBOX_OP_ECHO,
    MPU_SANDBOX_OP_EAPDU_FRAME,
    MPU_SANDBOX_OP_UR_VALIDATE_CHUNK,
    MPU_SANDBOX_OP_CBOR_VALIDATE_CHUNK,
    MPU_SANDBOX_OP_EIP712_JSON_VALIDATE_CHUNK,
} MpuSandboxOperation_t;

typedef enum {
    MPU_SANDBOX_STATUS_IDLE = 0,
    MPU_SANDBOX_STATUS_PENDING,
    MPU_SANDBOX_STATUS_OK,
    MPU_SANDBOX_STATUS_INVALID_REQUEST,
    MPU_SANDBOX_STATUS_OUTPUT_TOO_SMALL,
} MpuSandboxStatus_t;

typedef uint16_t MpuSandboxBlockId_t;

enum {
    MPU_SANDBOX_BLOCK_INVALID = 0U,
    MPU_SANDBOX_BLOCK_INPUT = 1U,
    MPU_SANDBOX_BLOCK_RESULT = 2U,
};

typedef struct {
    uint32_t request_id;
    MpuSandboxOperation_t operation;
    MpuSandboxBlockId_t input_block_id;
    MpuSandboxBlockId_t result_block_id;
    uint32_t input_length;
    uint32_t tick;
    uint32_t reserved;
} MpuSandboxRequest_t;

typedef struct {
    uint32_t request_id;
    MpuSandboxStatus_t status;
    uint32_t output_length;
    MpuSandboxBlockId_t result_block_id;
    uint16_t reserved;
} MpuSandboxResponse_t;

#ifndef BTC_ONLY
typedef struct {
    uint32_t framing_status;
    uint32_t timed_out;
    uint32_t payload_length;
    uint16_t command_type;
    uint16_t request_id;
    uint16_t total_packets;
    uint16_t packet_index;
    uint16_t first_missing_packet;
    uint8_t cla;
    uint8_t reserved;
    uint32_t ur_validation_status;
} MpuSandboxEapduResult_t;
#endif

bool MpuSandboxTaskCreate(void);
bool MpuSandboxSubmit(MpuSandboxOperation_t operation, const uint8_t *input, size_t input_length,
                      uint32_t *request_id);
bool MpuSandboxGetResponse(uint32_t request_id, MpuSandboxResponse_t *response, uint8_t *output,
                           size_t output_capacity);
bool MpuSandboxValidateUr(const uint8_t *input, size_t input_length,
                          uint32_t *validation_status);
bool MpuSandboxValidateCbor(const uint8_t *input, size_t input_length,
                            uint32_t *validation_status);
bool MpuSandboxValidateEip712Json(const uint8_t *input, size_t input_length,
                                  uint32_t *validation_status);

#ifndef BTC_ONLY
bool MpuSandboxGetEapduResult(uint32_t request_id, MpuSandboxEapduResult_t *result,
                              const uint8_t **payload);
bool MpuSandboxReleaseEapduResult(uint32_t request_id, bool reset_framing);
bool MpuSandboxResetEapduFraming(void);
#endif

#endif
