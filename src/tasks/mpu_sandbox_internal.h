#ifndef MPU_SANDBOX_INTERNAL_H
#define MPU_SANDBOX_INTERNAL_H

#include "mpu_sandbox_task.h"

#ifndef BTC_ONLY
#include "eapdu_framing.h"
#endif

#define MPU_SANDBOX_STACK_WORDS          1024U
#define MPU_SANDBOX_INPUT_SIZE           232U
#define MPU_SANDBOX_OUTPUT_SIZE          240U
#define MPU_SANDBOX_RW_REGION_SIZE       16384U
#define MPU_SANDBOX_TEXT_REGION_SIZE     16384U
#define MPU_SANDBOX_UR_INPUT_MAX_SIZE    1536U
#define MPU_SANDBOX_UR_CHUNK_HEADER_SIZE 4U
#define MPU_SANDBOX_UR_CHUNK_DATA_SIZE \
    (MPU_SANDBOX_INPUT_SIZE - MPU_SANDBOX_UR_CHUNK_HEADER_SIZE)
#define MPU_SANDBOX_CBOR_INPUT_MAX_SIZE        (14U * 1024U)
#define MPU_SANDBOX_EIP712_JSON_INPUT_MAX_SIZE MPU_SANDBOX_CBOR_INPUT_MAX_SIZE

typedef struct {
    volatile MpuSandboxRequest_t request;
    uint8_t input[MPU_SANDBOX_INPUT_SIZE];
} MpuSandboxInput_t;

typedef struct {
    volatile MpuSandboxResponse_t response;
    uint8_t output[MPU_SANDBOX_OUTPUT_SIZE];
} MpuSandboxOutput_t;

typedef struct {
    uint8_t data[MPU_SANDBOX_UR_INPUT_MAX_SIZE];
    uint16_t length;
    uint16_t total_length;
    bool active;
} MpuSandboxUrState_t;

typedef struct {
    uint32_t validation_status;
    uint32_t accepted_length;
} MpuSandboxUrChunkResult_t;

typedef struct {
    uint8_t data[MPU_SANDBOX_CBOR_INPUT_MAX_SIZE];
    uint16_t length;
    uint16_t total_length;
    bool active;
} MpuSandboxPayloadValidationState_t;

typedef enum {
    MPU_SANDBOX_PAYLOAD_NONE = 0,
    MPU_SANDBOX_PAYLOAD_EAPDU,
    MPU_SANDBOX_PAYLOAD_CBOR,
    MPU_SANDBOX_PAYLOAD_EIP712_JSON,
} MpuSandboxPayloadMode_t;

#ifndef BTC_ONLY
typedef union {
    EapduFramingState_t eapdu;
    MpuSandboxPayloadValidationState_t validation;
} MpuSandboxPayloadState_t;
#else
typedef union {
    MpuSandboxPayloadValidationState_t validation;
} MpuSandboxPayloadState_t;
#endif

typedef struct {
    MpuSandboxOutput_t output;
    MpuSandboxUrState_t ur;
    MpuSandboxPayloadState_t payload;
    MpuSandboxPayloadMode_t payload_mode;
} MpuSandboxRw_t;

_Static_assert(sizeof(MpuSandboxInput_t) == 256U,
               "sandbox input must occupy one MPU region");
_Static_assert(sizeof(MpuSandboxOutput_t) == 256U,
               "sandbox output header must remain fixed");
_Static_assert(sizeof(MpuSandboxRw_t) <= MPU_SANDBOX_RW_REGION_SIZE,
               "sandbox writable state must fit its MPU region");

extern MpuSandboxInput_t g_mpuSandboxInput;
extern MpuSandboxRw_t g_mpuSandboxRw;
void MpuSandboxRuntime(void *argument);

#endif
