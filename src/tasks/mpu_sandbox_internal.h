#ifndef MPU_SANDBOX_INTERNAL_H
#define MPU_SANDBOX_INTERNAL_H

#include "mpu_sandbox_task.h"
#include "mpu_sandbox_ur_validator.h"

#ifndef BTC_ONLY
#include "eapdu_framing.h"
#endif

#define MPU_SANDBOX_STACK_WORDS          1024U
#define MPU_SANDBOX_INPUT_SIZE           232U
#define MPU_SANDBOX_OUTPUT_SIZE          240U
#define MPU_SANDBOX_RW_REGION_SIZE       16384U
#define MPU_SANDBOX_TEXT_REGION_SIZE     16384U
#define MPU_SANDBOX_UR_INPUT_MAX_SIZE    1536U

/* Chunk header: 32-bit little-endian byte offset, then 32-bit total length. */
#define MPU_SANDBOX_UR_CHUNK_HEADER_SIZE 8U
#define MPU_SANDBOX_UR_CHUNK_DATA_SIZE \
    (MPU_SANDBOX_INPUT_SIZE - MPU_SANDBOX_UR_CHUNK_HEADER_SIZE)

/* EIP-712 JSON is validated from a buffer inside the sandbox RW region, so
 * this limit is that buffer's size. */
#define MPU_SANDBOX_EIP712_JSON_INPUT_MAX_SIZE (14U * 1024U)

/* CBOR (reassembled multipart URs) is validated as a stream with O(1) state
 * (MpuSandboxCborStreamState_t), so this is not a buffer size but a policy
 * limit on the reassembled UR, bounding the heap the privileged side then
 * spends parsing it (measured ~10x the payload for a Zcash PCZT).
 * Cypherpunk builds back the Rust heap with the 8 MiB PSRAM, and their Zcash
 * PCZTs carry larger shielded transactions (an example Orchard -> Ironwood
 * migration was ~23 KiB; a batch may reach ZCASH_BATCH_MAX_TOTAL_BYTES);
 * other builds use the 424 KiB SRAM heap. */
#ifdef CYPHERPUNK_VERSION
#define MPU_SANDBOX_CBOR_INPUT_MAX_SIZE  (512U * 1024U)
#else
#define MPU_SANDBOX_CBOR_INPUT_MAX_SIZE  (32U * 1024U)
#endif

/* Placed between the last store to a response payload and the store of its
 * status. Both parties are FreeRTOS tasks on the one Cortex-M core, which
 * observes its own accesses in program order, so the barrier's job is to
 * keep the compiler from sinking the (non-volatile) payload stores past the
 * volatile status store; the reader needs no acquire barrier after it loads
 * the status. That stops holding on a multi-core part, or if another master
 * (a DMA engine, say) ever produced the payload: the reader would then need
 * a barrier after the status load as well. */
#ifdef COMPILE_SIMULATOR
#define MPU_SANDBOX_MEMORY_BARRIER() __atomic_thread_fence(__ATOMIC_SEQ_CST)
#else
#define MPU_SANDBOX_MEMORY_BARRIER() __asm volatile("dmb" ::: "memory")
#endif

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
    uint32_t length;
    uint32_t total_length;
    bool active;
} MpuSandboxUrState_t;

typedef struct {
    uint32_t validation_status;
    uint32_t accepted_length;
} MpuSandboxUrChunkResult_t;

typedef struct {
    uint8_t data[MPU_SANDBOX_EIP712_JSON_INPUT_MAX_SIZE];
    uint32_t length;
    uint32_t total_length;
    bool active;
} MpuSandboxPayloadValidationState_t;

typedef struct {
    uint8_t state[MPU_SANDBOX_CBOR_STREAM_STATE_SIZE] __attribute__((aligned(8)));
    uint32_t length;
    uint32_t total_length;
    bool active;
} MpuSandboxCborStreamState_t;

typedef enum {
    MPU_SANDBOX_PAYLOAD_NONE = 0,
    MPU_SANDBOX_PAYLOAD_EAPDU,
    MPU_SANDBOX_PAYLOAD_CBOR,
    MPU_SANDBOX_PAYLOAD_EIP712_JSON,
} MpuSandboxPayloadMode_t;

#if !defined(BTC_ONLY) && !defined(COMPILE_SIMULATOR)
typedef union {
    EapduFramingState_t eapdu;
    MpuSandboxPayloadValidationState_t validation;
    MpuSandboxCborStreamState_t cbor_stream;
} MpuSandboxPayloadState_t;
#else
typedef union {
    MpuSandboxPayloadValidationState_t validation;
    MpuSandboxCborStreamState_t cbor_stream;
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
_Static_assert(MPU_SANDBOX_CBOR_INPUT_MAX_SIZE <= UINT32_MAX,
               "CBOR input limit must fit the 32-bit chunk framing");

/* Shared by the privileged chunk loop and the sandbox runtime so that both
 * sides of the IPC agree on the framing (and the simulator exercises the same code). */
static inline void MpuSandboxWriteChunkHeader(uint8_t *chunk, uint32_t offset,
                                              uint32_t total_length)
{
    chunk[0] = (uint8_t)offset;
    chunk[1] = (uint8_t)(offset >> 8U);
    chunk[2] = (uint8_t)(offset >> 16U);
    chunk[3] = (uint8_t)(offset >> 24U);
    chunk[4] = (uint8_t)total_length;
    chunk[5] = (uint8_t)(total_length >> 8U);
    chunk[6] = (uint8_t)(total_length >> 16U);
    chunk[7] = (uint8_t)(total_length >> 24U);
}

static inline void MpuSandboxReadChunkHeader(const uint8_t *chunk, uint32_t *offset,
                                             uint32_t *total_length)
{
    *offset = (uint32_t)chunk[0] | ((uint32_t)chunk[1] << 8U) |
              ((uint32_t)chunk[2] << 16U) | ((uint32_t)chunk[3] << 24U);
    *total_length = (uint32_t)chunk[4] | ((uint32_t)chunk[5] << 8U) |
                    ((uint32_t)chunk[6] << 16U) | ((uint32_t)chunk[7] << 24U);
}

extern MpuSandboxInput_t g_mpuSandboxInput;
extern MpuSandboxRw_t g_mpuSandboxRw;
void MpuSandboxRuntime(void *argument);
/* One request, synchronously: the sandbox task calls this on notification;
 * the simulator calls it directly in place of the task. */
void MpuSandboxProcessRequest(MpuSandboxInput_t *input, MpuSandboxRw_t *rw);

/* Transport hooks used by the chunk loop in mpu_sandbox_validate.c. On the
 * device (mpu_sandbox_task.c) a round trip is submit + poll with a per-chunk
 * timeout and fail-closed is a system reset; the simulator
 * (ui_simulator/mpu_sandbox_sim.c) calls the runtime directly and aborts. */
bool MpuSandboxRoundTrip(MpuSandboxOperation_t operation, const uint8_t *chunk,
                         size_t chunk_length, MpuSandboxResponse_t *response,
                         MpuSandboxUrChunkResult_t *result);
void MpuSandboxValidationFailClosed(void) __attribute__((noreturn));
/* True for every MpuSandboxUrValidationStatus_t value other than NOT_CHECKED. */
bool MpuSandboxIsUrValidationStatusKnown(uint32_t status);

#endif
