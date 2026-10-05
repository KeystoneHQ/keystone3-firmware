#include "mpu_sandbox_task.h"
#include "mpu_sandbox_internal.h"
#include "mpu_sandbox_ur_validator.h"

#include <string.h>

#include "FreeRTOS.h"
#include "mhscpu.h"
#include "mpu_syscall_numbers.h"
#include "task.h"
#ifndef BTC_ONLY
#include "eapdu_framing.h"
#endif

#define MPU_SANDBOX_VALIDATION_TIMEOUT_TICKS pdMS_TO_TICKS(1000U)

_Static_assert(MPU_SANDBOX_STACK_WORDS * sizeof(StackType_t) == 4096U,
               "sandbox stack must occupy one MPU region");
_Static_assert(sizeof(MpuSandboxUrChunkResult_t) <= MPU_SANDBOX_OUTPUT_SIZE,
               "UR validation result must fit the sandbox output region");
#ifndef BTC_ONLY
_Static_assert(sizeof(MpuSandboxEapduResult_t) <= MPU_SANDBOX_OUTPUT_SIZE,
               "EAPDU result must fit the sandbox output region");
#endif

#ifndef BTC_ONLY
typedef struct {
    bool has_header;
    uint8_t cla;
    uint16_t command_type;
    uint16_t request_id;
    uint16_t total_packets;
    uint16_t packet_index;
} MpuSandboxTrustedFrame_t;

static void CaptureTrustedEapduFrame(const uint8_t *frame, size_t frame_length);
static bool IsEapduResultValid(const MpuSandboxEapduResult_t *result);
#endif

typedef enum {
    MPU_SANDBOX_OWNER_FREE = 0,
    MPU_SANDBOX_OWNER_TRANSPORT,
    MPU_SANDBOX_OWNER_PARSER,
    MPU_SANDBOX_OWNER_BUSINESS,
} MpuSandboxBlockOwner_t;

typedef struct {
    MpuSandboxBlockOwner_t owner;
    uint32_t request_id;
    uint32_t length;
    uint32_t capacity;
} MpuSandboxBlockMetadata_t;

typedef struct {
    uint32_t active_request_id;
    MpuSandboxOperation_t active_operation;
    MpuSandboxBlockMetadata_t blocks[2];
} MpuSandboxIpcMetadata_t;

static void MpuSandboxIpcReset(void);
static MpuSandboxBlockMetadata_t *MpuSandboxIpcLookup(MpuSandboxBlockId_t block_id);
static bool MpuSandboxIpcIsIdle(void);
static bool MpuSandboxIpcParserOwns(uint32_t request_id);
static bool MpuSandboxIpcBusinessOwns(uint32_t request_id);
static bool MpuSandboxIpcTransferToBusiness(uint32_t request_id, uint32_t result_length);
static void MpuSandboxIpcRelease(void);
static bool MpuSandboxOperationIsAllowed(MpuSandboxOperation_t operation);

static bool IsUrValidationStatusKnown(uint32_t status);
static bool MpuSandboxValidateBuffer(MpuSandboxOperation_t operation,
                                     size_t maximum_length,
                                     const uint8_t *input, size_t input_length,
                                     uint32_t *validation_status);
static void MpuSandboxValidationFailClosedReset(void) __attribute__((noreturn));

static StaticTask_t g_mpuSandboxTaskBuffer __attribute__((section("privileged_data")));
static TaskHandle_t g_mpuSandboxTaskHandle __attribute__((section("privileged_data")));
static uint32_t g_mpuSandboxNextRequestId __attribute__((section("privileged_data"))) = 1U;
static MpuSandboxIpcMetadata_t g_mpuSandboxIpc
    __attribute__((section("privileged_data")));
static StackType_t g_mpuSandboxStack[MPU_SANDBOX_STACK_WORDS]
    __attribute__((section(".mpu_sandbox_stack"), aligned(4096)));
MpuSandboxInput_t g_mpuSandboxInput
    __attribute__((section(".mpu_sandbox_input"), aligned(256)));
MpuSandboxRw_t g_mpuSandboxRw
    __attribute__((section(".mpu_sandbox_rw"), aligned(MPU_SANDBOX_RW_REGION_SIZE)));
#ifndef BTC_ONLY
static uint8_t g_mpuSandboxTrustedPayload[EAPDU_FRAMING_MAX_PAYLOAD_SIZE + 1U];
static MpuSandboxTrustedFrame_t g_mpuSandboxTrustedFrame
    __attribute__((section("privileged_data")));
static bool g_mpuSandboxEapduSuspended __attribute__((section("privileged_data")));
#endif

extern uint32_t __sandbox_text_start;

static const TaskParameters_t g_mpuSandboxTaskParameters = {
    .pvTaskCode = MpuSandboxRuntime,
    .pcName = "mpu_sandbox",
    .usStackDepth = MPU_SANDBOX_STACK_WORDS,
    .pvParameters = NULL,
    .uxPriority = 2U,
    .puxStackBuffer = g_mpuSandboxStack,
    .xRegions = {
        {
            .pvBaseAddress = &g_mpuSandboxInput,
            .ulLengthInBytes = sizeof(g_mpuSandboxInput),
            .ulParameters = portMPU_REGION_READ_ONLY | portMPU_REGION_EXECUTE_NEVER,
        },
        {
            .pvBaseAddress = &g_mpuSandboxRw,
            .ulLengthInBytes = MPU_SANDBOX_RW_REGION_SIZE,
            .ulParameters = portMPU_REGION_READ_WRITE | portMPU_REGION_EXECUTE_NEVER,
        },
        {
            .pvBaseAddress = &__sandbox_text_start,
            .ulLengthInBytes = MPU_SANDBOX_TEXT_REGION_SIZE,
            .ulParameters = portMPU_REGION_READ_ONLY,
        },
    },
    .pxTaskBuffer = &g_mpuSandboxTaskBuffer,
};

BaseType_t xApplicationIsSystemCallAllowed(TaskHandle_t task, BaseType_t task_is_privileged,
                                            uint8_t system_call_number)
    PRIVILEGED_FUNCTION;

BaseType_t xApplicationIsSystemCallAllowed(TaskHandle_t task, BaseType_t task_is_privileged,
                                            uint8_t system_call_number)
{
    if (task == (TaskHandle_t)&g_mpuSandboxTaskBuffer) {
        if (system_call_number == SYSTEM_CALL_xTaskGenericNotifyWait) {
            return pdTRUE;
        }
        return pdFALSE;
    }
    return task_is_privileged;
}

bool MpuSandboxTaskCreate(void)
{
    if (g_mpuSandboxTaskHandle != NULL) {
        return true;
    }

    memset(&g_mpuSandboxInput, 0, sizeof(g_mpuSandboxInput));
    memset(&g_mpuSandboxRw, 0, sizeof(g_mpuSandboxRw));
    memset(&g_mpuSandboxIpc, 0, sizeof(g_mpuSandboxIpc));
#ifndef BTC_ONLY
    memset(g_mpuSandboxTrustedPayload, 0, sizeof(g_mpuSandboxTrustedPayload));
    memset(&g_mpuSandboxTrustedFrame, 0, sizeof(g_mpuSandboxTrustedFrame));
    g_mpuSandboxEapduSuspended = false;
    EapduFramingReset(&g_mpuSandboxRw.payload.eapdu);
    g_mpuSandboxRw.payload_mode = MPU_SANDBOX_PAYLOAD_NONE;
#endif
    MpuSandboxIpcReset();

    return xTaskCreateRestrictedStatic(&g_mpuSandboxTaskParameters, &g_mpuSandboxTaskHandle) == pdPASS;
}

static void MpuSandboxIpcReset(void)
{
    memset(&g_mpuSandboxIpc, 0, sizeof(g_mpuSandboxIpc));
    g_mpuSandboxIpc.blocks[MPU_SANDBOX_BLOCK_INPUT - 1U].capacity =
        sizeof(g_mpuSandboxInput.input);
    g_mpuSandboxIpc.blocks[MPU_SANDBOX_BLOCK_RESULT - 1U].capacity =
        sizeof(g_mpuSandboxRw);
}

static MpuSandboxBlockMetadata_t *MpuSandboxIpcLookup(MpuSandboxBlockId_t block_id)
{
    if ((block_id < MPU_SANDBOX_BLOCK_INPUT) ||
            (block_id > MPU_SANDBOX_BLOCK_RESULT)) {
        return NULL;
    }
    return &g_mpuSandboxIpc.blocks[block_id - 1U];
}

static bool MpuSandboxIpcIsIdle(void)
{
    MpuSandboxBlockMetadata_t *input = MpuSandboxIpcLookup(MPU_SANDBOX_BLOCK_INPUT);
    MpuSandboxBlockMetadata_t *result = MpuSandboxIpcLookup(MPU_SANDBOX_BLOCK_RESULT);

    return (g_mpuSandboxIpc.active_request_id == 0U) &&
           (g_mpuSandboxIpc.active_operation == 0) &&
           (input != NULL) && (input->owner == MPU_SANDBOX_OWNER_FREE) &&
           (result != NULL) && (result->owner == MPU_SANDBOX_OWNER_FREE);
}

static bool MpuSandboxIpcOwnerMatches(uint32_t request_id,
                                      MpuSandboxBlockOwner_t owner)
{
    MpuSandboxBlockMetadata_t *input = MpuSandboxIpcLookup(MPU_SANDBOX_BLOCK_INPUT);
    MpuSandboxBlockMetadata_t *result = MpuSandboxIpcLookup(MPU_SANDBOX_BLOCK_RESULT);

    return (request_id != 0U) &&
           (g_mpuSandboxIpc.active_request_id == request_id) &&
           (input != NULL) && (input->owner == owner) &&
           (input->request_id == request_id) &&
           (result != NULL) && (result->owner == owner) &&
           (result->request_id == request_id);
}

static bool MpuSandboxIpcParserOwns(uint32_t request_id)
{
    return MpuSandboxIpcOwnerMatches(request_id, MPU_SANDBOX_OWNER_PARSER);
}

static bool MpuSandboxIpcBusinessOwns(uint32_t request_id)
{
    return MpuSandboxIpcOwnerMatches(request_id, MPU_SANDBOX_OWNER_BUSINESS);
}

static bool MpuSandboxIpcTransferToBusiness(uint32_t request_id, uint32_t result_length)
{
    MpuSandboxBlockMetadata_t *input = MpuSandboxIpcLookup(MPU_SANDBOX_BLOCK_INPUT);
    MpuSandboxBlockMetadata_t *result = MpuSandboxIpcLookup(MPU_SANDBOX_BLOCK_RESULT);

    if ((input == NULL) || (result == NULL) ||
            !MpuSandboxIpcParserOwns(request_id) ||
            (result_length > result->capacity)) {
        return false;
    }

    input->owner = MPU_SANDBOX_OWNER_BUSINESS;
    result->length = result_length;
    result->owner = MPU_SANDBOX_OWNER_BUSINESS;
    return true;
}

static void MpuSandboxIpcRelease(void)
{
    MpuSandboxIpcReset();
}

static bool MpuSandboxOperationIsAllowed(MpuSandboxOperation_t operation)
{
    switch (operation) {
    case MPU_SANDBOX_OP_PING:
    case MPU_SANDBOX_OP_ECHO:
    case MPU_SANDBOX_OP_UR_VALIDATE_CHUNK:
    case MPU_SANDBOX_OP_CBOR_VALIDATE_CHUNK:
    case MPU_SANDBOX_OP_EIP712_JSON_VALIDATE_CHUNK:
#ifndef BTC_ONLY
    case MPU_SANDBOX_OP_EAPDU_FRAME:
#endif
        return true;
    default:
        return false;
    }
}

bool MpuSandboxSubmit(MpuSandboxOperation_t operation, const uint8_t *input, size_t input_length,
                      uint32_t *request_id)
{
    uint32_t id;
    MpuSandboxBlockMetadata_t *inputBlock;
    MpuSandboxBlockMetadata_t *resultBlock;

    if ((g_mpuSandboxTaskHandle == NULL) || !MpuSandboxOperationIsAllowed(operation) ||
            (input_length > sizeof(g_mpuSandboxInput.input)) ||
            ((input_length > 0U) && (input == NULL))) {
        return false;
    }

    taskENTER_CRITICAL();
    if (!MpuSandboxIpcIsIdle() || (g_mpuSandboxInput.request.operation != 0)
#ifndef BTC_ONLY
            || g_mpuSandboxEapduSuspended
#endif
       ) {
        taskEXIT_CRITICAL();
        return false;
    }

    id = g_mpuSandboxNextRequestId++;
    if (g_mpuSandboxNextRequestId == 0U) {
        g_mpuSandboxNextRequestId = 1U;
    }

    inputBlock = MpuSandboxIpcLookup(MPU_SANDBOX_BLOCK_INPUT);
    resultBlock = MpuSandboxIpcLookup(MPU_SANDBOX_BLOCK_RESULT);
    inputBlock->owner = MPU_SANDBOX_OWNER_TRANSPORT;
    inputBlock->request_id = id;
    inputBlock->length = (uint32_t)input_length;
    resultBlock->request_id = id;
    resultBlock->length = 0U;

    memset(g_mpuSandboxInput.input, 0, sizeof(g_mpuSandboxInput.input));
    memset(&g_mpuSandboxRw.output, 0, sizeof(g_mpuSandboxRw.output));
    if (input_length > 0U) {
        memcpy(g_mpuSandboxInput.input, input, input_length);
    }
#ifndef BTC_ONLY
    if (operation == MPU_SANDBOX_OP_EAPDU_FRAME) {
        CaptureTrustedEapduFrame(input, input_length);
    }
#endif
    g_mpuSandboxRw.output.response.request_id = id;
    g_mpuSandboxRw.output.response.status = MPU_SANDBOX_STATUS_PENDING;
    g_mpuSandboxRw.output.response.output_length = 0U;
    g_mpuSandboxRw.output.response.result_block_id = MPU_SANDBOX_BLOCK_RESULT;
    g_mpuSandboxInput.request.request_id = id;
    g_mpuSandboxInput.request.input_block_id = MPU_SANDBOX_BLOCK_INPUT;
    g_mpuSandboxInput.request.result_block_id = MPU_SANDBOX_BLOCK_RESULT;
    g_mpuSandboxInput.request.input_length = (uint32_t) input_length;
    g_mpuSandboxInput.request.tick = 0U;
    g_mpuSandboxInput.request.reserved = 0U;
#ifndef BTC_ONLY
    if (operation == MPU_SANDBOX_OP_EAPDU_FRAME) {
        g_mpuSandboxInput.request.tick = (uint32_t)xTaskGetTickCount();
    }
#endif
    g_mpuSandboxIpc.active_request_id = id;
    g_mpuSandboxIpc.active_operation = operation;
    inputBlock->owner = MPU_SANDBOX_OWNER_PARSER;
    resultBlock->owner = MPU_SANDBOX_OWNER_PARSER;
    __asm volatile("dmb" ::: "memory");
    g_mpuSandboxInput.request.operation = operation;
    if (xTaskNotify(g_mpuSandboxTaskHandle, 1U, eSetBits) != pdPASS) {
        memset(&g_mpuSandboxInput, 0, sizeof(g_mpuSandboxInput));
        memset(&g_mpuSandboxRw.output, 0, sizeof(g_mpuSandboxRw.output));
        MpuSandboxIpcRelease();
        taskEXIT_CRITICAL();
        return false;
    }
    if (request_id != NULL) {
        *request_id = id;
    }
    taskEXIT_CRITICAL();
    return true;
}

bool MpuSandboxGetResponse(uint32_t request_id, MpuSandboxResponse_t *response, uint8_t *output,
                           size_t output_capacity)
{
    uint32_t output_length;

    if (response == NULL) {
        return false;
    }

    taskENTER_CRITICAL();
    __asm volatile("dmb" ::: "memory");
    if (!MpuSandboxIpcParserOwns(request_id) ||
            (g_mpuSandboxIpc.active_operation == MPU_SANDBOX_OP_EAPDU_FRAME) ||
            (g_mpuSandboxInput.request.request_id != request_id) ||
            (g_mpuSandboxInput.request.input_block_id != MPU_SANDBOX_BLOCK_INPUT) ||
            (g_mpuSandboxInput.request.result_block_id != MPU_SANDBOX_BLOCK_RESULT) ||
            (g_mpuSandboxRw.output.response.request_id != request_id) ||
            (g_mpuSandboxRw.output.response.result_block_id != MPU_SANDBOX_BLOCK_RESULT) ||
            (g_mpuSandboxRw.output.response.status == MPU_SANDBOX_STATUS_PENDING)) {
        taskEXIT_CRITICAL();
        return false;
    }
    output_length = g_mpuSandboxRw.output.response.output_length;
    if ((output_length > sizeof(g_mpuSandboxRw.output.output)) ||
            (output_length > output_capacity) || ((output_length > 0U) && (output == NULL))) {
        taskEXIT_CRITICAL();
        return false;
    }
    if (!MpuSandboxIpcTransferToBusiness(request_id, output_length)) {
        taskEXIT_CRITICAL();
        return false;
    }
    *response = g_mpuSandboxRw.output.response;
    if (output_length > 0U) {
        memcpy(output, g_mpuSandboxRw.output.output, output_length);
    }
    memset(&g_mpuSandboxInput, 0, sizeof(g_mpuSandboxInput));
    memset(&g_mpuSandboxRw.output, 0, sizeof(g_mpuSandboxRw.output));
    MpuSandboxIpcRelease();
    taskEXIT_CRITICAL();
    return true;
}

bool MpuSandboxValidateUr(const uint8_t *input, size_t input_length,
                          uint32_t *validation_status)
{
    return MpuSandboxValidateBuffer(MPU_SANDBOX_OP_UR_VALIDATE_CHUNK,
                                    MPU_SANDBOX_UR_INPUT_MAX_SIZE,
                                    input, input_length, validation_status);
}

bool MpuSandboxValidateCbor(const uint8_t *input, size_t input_length,
                            uint32_t *validation_status)
{
    return MpuSandboxValidateBuffer(MPU_SANDBOX_OP_CBOR_VALIDATE_CHUNK,
                                    MPU_SANDBOX_CBOR_INPUT_MAX_SIZE,
                                    input, input_length, validation_status);
}

bool MpuSandboxValidateEip712Json(const uint8_t *input, size_t input_length,
                                  uint32_t *validation_status)
{
    return MpuSandboxValidateBuffer(MPU_SANDBOX_OP_EIP712_JSON_VALIDATE_CHUNK,
                                    MPU_SANDBOX_EIP712_JSON_INPUT_MAX_SIZE,
                                    input, input_length, validation_status);
}

static bool MpuSandboxValidateBuffer(MpuSandboxOperation_t operation,
                                     size_t maximum_length,
                                     const uint8_t *input, size_t input_length,
                                     uint32_t *validation_status)
{
    uint8_t chunk[MPU_SANDBOX_INPUT_SIZE];
    size_t offset = 0U;
    uint32_t finalStatus = MPU_SANDBOX_UR_NOT_CHECKED;
    TickType_t started;

    if ((input == NULL) || (input_length == 0U) ||
            (input_length > maximum_length) ||
            (validation_status == NULL)) {
        return false;
    }

    started = xTaskGetTickCount();
    while (offset < input_length) {
        size_t chunkLength = input_length - offset;
        uint32_t requestId = 0U;
        MpuSandboxResponse_t response;
        MpuSandboxUrChunkResult_t result;

        if (chunkLength > MPU_SANDBOX_UR_CHUNK_DATA_SIZE) {
            chunkLength = MPU_SANDBOX_UR_CHUNK_DATA_SIZE;
        }
        memset(chunk, 0, sizeof(chunk));
        chunk[0] = (uint8_t)offset;
        chunk[1] = (uint8_t)(offset >> 8U);
        chunk[2] = (uint8_t)input_length;
        chunk[3] = (uint8_t)(input_length >> 8U);
        memcpy(chunk + MPU_SANDBOX_UR_CHUNK_HEADER_SIZE,
               input + offset, chunkLength);

        while (!MpuSandboxSubmit(operation,
                                 chunk,
                                 MPU_SANDBOX_UR_CHUNK_HEADER_SIZE + chunkLength,
                                 &requestId)) {
            if ((xTaskGetTickCount() - started) >= MPU_SANDBOX_VALIDATION_TIMEOUT_TICKS) {
                MpuSandboxValidationFailClosedReset();
            }
            vTaskDelay(1U);
        }

        while (!MpuSandboxGetResponse(requestId, &response,
                                      (uint8_t *)&result, sizeof(result))) {
            if ((xTaskGetTickCount() - started) >= MPU_SANDBOX_VALIDATION_TIMEOUT_TICKS) {
                MpuSandboxValidationFailClosedReset();
            }
            vTaskDelay(1U);
        }

        offset += chunkLength;
        if ((response.status != MPU_SANDBOX_STATUS_OK) ||
                (response.output_length != sizeof(result)) ||
                (result.accepted_length != offset)) {
            MpuSandboxValidationFailClosedReset();
        }
        if (offset < input_length) {
            if (result.validation_status != MPU_SANDBOX_UR_NOT_CHECKED) {
                MpuSandboxValidationFailClosedReset();
            }
        } else {
            if (!IsUrValidationStatusKnown(result.validation_status)) {
                MpuSandboxValidationFailClosedReset();
            }
            finalStatus = result.validation_status;
        }
    }

    memset(chunk, 0, sizeof(chunk));
    *validation_status = finalStatus;
    return true;
}

static void MpuSandboxValidationFailClosedReset(void)
{
    __DSB();
    NVIC_SystemReset();
    for (;;) {
    }
}

#ifndef BTC_ONLY
bool MpuSandboxGetEapduResult(uint32_t request_id, MpuSandboxEapduResult_t *result,
                              const uint8_t **payload)
{
    if ((result == NULL) || (payload == NULL)) {
        return false;
    }

    taskENTER_CRITICAL();
    __asm volatile("dmb" ::: "memory");
    if (!MpuSandboxIpcParserOwns(request_id) ||
            (g_mpuSandboxIpc.active_operation != MPU_SANDBOX_OP_EAPDU_FRAME) ||
            (g_mpuSandboxInput.request.operation != MPU_SANDBOX_OP_EAPDU_FRAME) ||
            (g_mpuSandboxInput.request.request_id != request_id) ||
            (g_mpuSandboxInput.request.input_block_id != MPU_SANDBOX_BLOCK_INPUT) ||
            (g_mpuSandboxInput.request.result_block_id != MPU_SANDBOX_BLOCK_RESULT) ||
            (g_mpuSandboxRw.output.response.request_id != request_id) ||
            (g_mpuSandboxRw.output.response.result_block_id != MPU_SANDBOX_BLOCK_RESULT) ||
            (g_mpuSandboxRw.output.response.status != MPU_SANDBOX_STATUS_OK) ||
            (g_mpuSandboxRw.output.response.output_length != sizeof(*result))) {
        taskEXIT_CRITICAL();
        return false;
    }

    if (!g_mpuSandboxEapduSuspended) {
        vTaskSuspend(g_mpuSandboxTaskHandle);
        g_mpuSandboxEapduSuspended = true;
    }
    taskEXIT_CRITICAL();

    memcpy(result, g_mpuSandboxRw.output.output, sizeof(*result));
    if (!IsEapduResultValid(result)) {
        return false;
    }
    if (result->payload_length > 0U) {
        memcpy(g_mpuSandboxTrustedPayload, g_mpuSandboxRw.payload.eapdu.packet_data,
               result->payload_length);
    }
    g_mpuSandboxTrustedPayload[result->payload_length] = '\0';

    taskENTER_CRITICAL();
    if (!MpuSandboxIpcTransferToBusiness(request_id, result->payload_length)) {
        taskEXIT_CRITICAL();
        return false;
    }
    taskEXIT_CRITICAL();
    *payload = g_mpuSandboxTrustedPayload;
    return true;
}

bool MpuSandboxReleaseEapduResult(uint32_t request_id, bool reset_framing)
{
    taskENTER_CRITICAL();
    if (!MpuSandboxIpcBusinessOwns(request_id) ||
            (g_mpuSandboxIpc.active_operation != MPU_SANDBOX_OP_EAPDU_FRAME) ||
            (g_mpuSandboxInput.request.operation != MPU_SANDBOX_OP_EAPDU_FRAME) ||
            (g_mpuSandboxRw.output.response.request_id != request_id) ||
            (g_mpuSandboxRw.output.response.status == MPU_SANDBOX_STATUS_PENDING) ||
            !g_mpuSandboxEapduSuspended) {
        taskEXIT_CRITICAL();
        return false;
    }
    g_mpuSandboxRw.output.response.status = MPU_SANDBOX_STATUS_PENDING;
    taskEXIT_CRITICAL();

    if (reset_framing) {
        EapduFramingReset(&g_mpuSandboxRw.payload.eapdu);
        g_mpuSandboxRw.payload_mode = MPU_SANDBOX_PAYLOAD_NONE;
    }
    memset(g_mpuSandboxInput.input, 0, sizeof(g_mpuSandboxInput.input));
    memset(g_mpuSandboxRw.output.output, 0, sizeof(g_mpuSandboxRw.output.output));
    memset(g_mpuSandboxTrustedPayload, 0, sizeof(g_mpuSandboxTrustedPayload));
    memset(&g_mpuSandboxTrustedFrame, 0, sizeof(g_mpuSandboxTrustedFrame));
    taskENTER_CRITICAL();
    g_mpuSandboxRw.output.response.status = MPU_SANDBOX_STATUS_IDLE;
    g_mpuSandboxRw.output.response.output_length = 0U;
    __asm volatile("dmb" ::: "memory");
    g_mpuSandboxInput.request.operation = 0;
    MpuSandboxIpcRelease();
    g_mpuSandboxEapduSuspended = false;
    taskEXIT_CRITICAL();
    vTaskResume(g_mpuSandboxTaskHandle);
    return true;
}

bool MpuSandboxResetEapduFraming(void)
{
    taskENTER_CRITICAL();
    if (!MpuSandboxIpcIsIdle() || (g_mpuSandboxInput.request.operation != 0) ||
            g_mpuSandboxEapduSuspended) {
        taskEXIT_CRITICAL();
        return false;
    }
    vTaskSuspend(g_mpuSandboxTaskHandle);
    g_mpuSandboxEapduSuspended = true;
    taskEXIT_CRITICAL();
    EapduFramingReset(&g_mpuSandboxRw.payload.eapdu);
    g_mpuSandboxRw.payload_mode = MPU_SANDBOX_PAYLOAD_NONE;
    memset(&g_mpuSandboxTrustedFrame, 0, sizeof(g_mpuSandboxTrustedFrame));
    taskENTER_CRITICAL();
    g_mpuSandboxEapduSuspended = false;
    taskEXIT_CRITICAL();
    vTaskResume(g_mpuSandboxTaskHandle);
    return true;
}

static void CaptureTrustedEapduFrame(const uint8_t *frame, size_t frame_length)
{
    memset(&g_mpuSandboxTrustedFrame, 0, sizeof(g_mpuSandboxTrustedFrame));
    if ((frame == NULL) || (frame_length < EAPDU_FRAMING_HEADER_SIZE)) {
        return;
    }

    g_mpuSandboxTrustedFrame.has_header = true;
    g_mpuSandboxTrustedFrame.cla = frame[0];
    g_mpuSandboxTrustedFrame.command_type = ((uint16_t)frame[1] << 8U) | frame[2];
    g_mpuSandboxTrustedFrame.total_packets = ((uint16_t)frame[3] << 8U) | frame[4];
    g_mpuSandboxTrustedFrame.packet_index = ((uint16_t)frame[5] << 8U) | frame[6];
    g_mpuSandboxTrustedFrame.request_id = ((uint16_t)frame[7] << 8U) | frame[8];
}

static bool IsEapduResultValid(const MpuSandboxEapduResult_t *result)
{
    EapduFramingStatus_t status = (EapduFramingStatus_t)result->framing_status;

    if ((status < EAPDU_FRAMING_WAITING) ||
            (status > EAPDU_FRAMING_ERROR_METADATA_MISMATCH) ||
            (result->timed_out > 1U) ||
            (result->payload_length > EAPDU_FRAMING_MAX_PAYLOAD_SIZE)) {
        return false;
    }

    if (g_mpuSandboxTrustedFrame.has_header) {
        if ((result->cla != g_mpuSandboxTrustedFrame.cla) ||
                (result->command_type != g_mpuSandboxTrustedFrame.command_type) ||
                (result->request_id != g_mpuSandboxTrustedFrame.request_id) ||
                (result->total_packets != g_mpuSandboxTrustedFrame.total_packets) ||
                (result->packet_index != g_mpuSandboxTrustedFrame.packet_index)) {
            return false;
        }
    } else if ((result->cla != 0U) || (result->command_type != 0U) ||
               (result->request_id != 0U) || (result->total_packets != 0U) ||
               (result->packet_index != 0U)) {
        return false;
    }

    if (status == EAPDU_FRAMING_COMPLETE) {
        if ((result->command_type == MPU_SANDBOX_RESOLVE_UR_COMMAND) &&
                !IsUrValidationStatusKnown(result->ur_validation_status)) {
            return false;
        }
        if ((result->command_type != MPU_SANDBOX_RESOLVE_UR_COMMAND) &&
                (result->ur_validation_status != MPU_SANDBOX_UR_NOT_CHECKED)) {
            return false;
        }
        return (result->total_packets > 0U) &&
               (result->total_packets <= EAPDU_FRAMING_MAX_PACKETS) &&
               (result->packet_index < result->total_packets) &&
               (result->payload_length <=
                ((uint32_t)result->total_packets * EAPDU_FRAMING_MAX_DATA_SIZE));
    }

    if (result->payload_length != 0U) {
        return false;
    }

    if ((status == EAPDU_FRAMING_WAITING) || (status == EAPDU_FRAMING_DUPLICATE)) {
        return (result->total_packets > 0U) &&
               (result->total_packets <= EAPDU_FRAMING_MAX_PACKETS) &&
               (result->packet_index < result->total_packets);
    }

    return true;
}

#endif

static bool IsUrValidationStatusKnown(uint32_t status)
{
    return (status >= MPU_SANDBOX_UR_OK) &&
           (status <= MPU_SANDBOX_JSON_ROOT_TYPE);
}
