#include "mpu_sandbox_internal.h"
#include "mpu_sandbox_ur_validator.h"

#include "FreeRTOS.h"
#include "task.h"

static void MpuSandboxProcessRequest(MpuSandboxInput_t *input, MpuSandboxRw_t *rw);
static void MpuSandboxProcessUrChunk(MpuSandboxInput_t *input, MpuSandboxRw_t *rw);
static void MpuSandboxProcessCborChunk(MpuSandboxInput_t *input, MpuSandboxRw_t *rw);
static void MpuSandboxProcessEip712JsonChunk(MpuSandboxInput_t *input, MpuSandboxRw_t *rw);
typedef enum {
    MPU_SANDBOX_VALIDATION_UR = 0,
    MPU_SANDBOX_VALIDATION_CBOR,
    MPU_SANDBOX_VALIDATION_EIP712_JSON,
} MpuSandboxValidationKind_t;
static bool MpuSandboxProcessValidationChunk(MpuSandboxInput_t *input,
                                             MpuSandboxOutput_t *output,
                                             uint8_t *data, size_t capacity,
                                             uint16_t *length, uint16_t *total_length,
                                             bool *active, MpuSandboxValidationKind_t kind);
static void SandboxCopy(void *destination, const void *source, size_t length)
{
    uint8_t *destinationBytes = destination;
    const uint8_t *sourceBytes = source;

    for (size_t i = 0U; i < length; i++) {
        destinationBytes[i] = sourceBytes[i];
    }
}

static void SandboxZero(void *destination, size_t length)
{
    volatile uint8_t *destinationBytes = destination;

    for (size_t i = 0U; i < length; i++) {
        destinationBytes[i] = 0U;
    }
}

void MpuSandboxRuntime(void *argument)
{
    (void)argument;
    for (;;) {
        if (xTaskNotifyWait(0U, UINT32_MAX, NULL, portMAX_DELAY) == pdPASS) {
            MpuSandboxProcessRequest(&g_mpuSandboxInput, &g_mpuSandboxRw);
        }
    }
}

static void MpuSandboxProcessRequest(MpuSandboxInput_t *input, MpuSandboxRw_t *rw)
{
    MpuSandboxRequest_t request = input->request;
    MpuSandboxOperation_t operation = request.operation;
    uint32_t inputLength = request.input_length;
    MpuSandboxOutput_t *output = &rw->output;

    output->response.request_id = request.request_id;
    output->response.output_length = 0U;
    output->response.result_block_id = request.result_block_id;
    output->response.reserved = 0U;
    if ((request.request_id == 0U) ||
            (request.input_block_id != MPU_SANDBOX_BLOCK_INPUT) ||
            (request.result_block_id != MPU_SANDBOX_BLOCK_RESULT) ||
            (inputLength > sizeof(input->input))) {
        output->response.status = MPU_SANDBOX_STATUS_INVALID_REQUEST;
        return;
    }

    switch (operation) {
    case MPU_SANDBOX_OP_PING:
        output->output[0] = 'P';
        output->output[1] = 'O';
        output->output[2] = 'N';
        output->output[3] = 'G';
        output->response.output_length = 4U;
        __asm volatile("dmb" ::: "memory");
        output->response.status = MPU_SANDBOX_STATUS_OK;
        break;
    case MPU_SANDBOX_OP_ECHO:
        SandboxCopy(output->output, input->input, inputLength);
        output->response.output_length = inputLength;
        __asm volatile("dmb" ::: "memory");
        output->response.status = MPU_SANDBOX_STATUS_OK;
        break;
    case MPU_SANDBOX_OP_UR_VALIDATE_CHUNK:
        MpuSandboxProcessUrChunk(input, rw);
        break;
    case MPU_SANDBOX_OP_CBOR_VALIDATE_CHUNK:
        MpuSandboxProcessCborChunk(input, rw);
        break;
    case MPU_SANDBOX_OP_EIP712_JSON_VALIDATE_CHUNK:
        MpuSandboxProcessEip712JsonChunk(input, rw);
        break;
#ifndef BTC_ONLY
    case MPU_SANDBOX_OP_EAPDU_FRAME: {
        uint32_t urValidationStatus = MPU_SANDBOX_UR_NOT_CHECKED;
        if ((rw->payload_mode != MPU_SANDBOX_PAYLOAD_NONE) &&
                (rw->payload_mode != MPU_SANDBOX_PAYLOAD_EAPDU)) {
            output->response.status = MPU_SANDBOX_STATUS_INVALID_REQUEST;
            break;
        }
        if (rw->payload_mode == MPU_SANDBOX_PAYLOAD_NONE) {
            SandboxZero(&rw->payload, sizeof(rw->payload));
            rw->payload_mode = MPU_SANDBOX_PAYLOAD_EAPDU;
        }
        EapduFramingResult_t framing = EapduFramingPush(&rw->payload.eapdu, input->input,
                                                        inputLength,
                                                        request.tick);
        if ((framing.status == EAPDU_FRAMING_COMPLETE) &&
                (framing.command_type == MPU_SANDBOX_RESOLVE_UR_COMMAND)) {
            urValidationStatus = mpu_sandbox_validate_ur(rw->payload.eapdu.packet_data,
                                                         framing.payload_length);
        }
        MpuSandboxEapduResult_t result = {
            .framing_status = (uint32_t)framing.status,
            .timed_out = framing.timed_out ? 1U : 0U,
            .payload_length = framing.payload_length,
            .command_type = framing.command_type,
            .request_id = framing.request_id,
            .total_packets = framing.total_packets,
            .packet_index = framing.packet_index,
            .first_missing_packet = framing.first_missing_packet,
            .cla = framing.cla,
            .ur_validation_status = urValidationStatus,
        };
        SandboxCopy(output->output, &result, sizeof(result));
        output->response.output_length = sizeof(result);
        __asm volatile("dmb" ::: "memory");
        output->response.status = MPU_SANDBOX_STATUS_OK;
        break;
    }
#endif
    default:
        output->response.status = MPU_SANDBOX_STATUS_INVALID_REQUEST;
        break;
    }
}
static void MpuSandboxProcessUrChunk(MpuSandboxInput_t *input, MpuSandboxRw_t *rw)
{
    MpuSandboxUrState_t *state = &rw->ur;
    if (!MpuSandboxProcessValidationChunk(input, &rw->output,
                                          state->data, sizeof(state->data),
                                          &state->length, &state->total_length,
                                          &state->active, MPU_SANDBOX_VALIDATION_UR) ||
            !state->active) {
        SandboxZero(state, sizeof(*state));
    }
}

static void MpuSandboxProcessPayloadValidationChunk(MpuSandboxInput_t *input,
                                                    MpuSandboxRw_t *rw,
                                                    MpuSandboxPayloadMode_t mode,
                                                    MpuSandboxValidationKind_t kind)
{
    MpuSandboxPayloadValidationState_t *state = &rw->payload.validation;
    bool firstChunk = (input->request.input_length >= MPU_SANDBOX_UR_CHUNK_HEADER_SIZE) &&
                      (input->input[0] == 0U) && (input->input[1] == 0U);

    if (firstChunk) {
        if (rw->payload_mode != MPU_SANDBOX_PAYLOAD_NONE) {
            rw->output.response.status = MPU_SANDBOX_STATUS_INVALID_REQUEST;
            return;
        }
        SandboxZero(&rw->payload, sizeof(rw->payload));
        rw->payload_mode = mode;
    } else if (rw->payload_mode != mode) {
        rw->output.response.status = MPU_SANDBOX_STATUS_INVALID_REQUEST;
        return;
    }

    if (!MpuSandboxProcessValidationChunk(input, &rw->output,
                                          state->data, sizeof(state->data),
                                          &state->length, &state->total_length,
                                          &state->active, kind) || !state->active) {
        SandboxZero(&rw->payload, sizeof(rw->payload));
        rw->payload_mode = MPU_SANDBOX_PAYLOAD_NONE;
    }
}

static void MpuSandboxProcessCborChunk(MpuSandboxInput_t *input, MpuSandboxRw_t *rw)
{
    MpuSandboxProcessPayloadValidationChunk(input, rw,
                                            MPU_SANDBOX_PAYLOAD_CBOR,
                                            MPU_SANDBOX_VALIDATION_CBOR);
}

static void MpuSandboxProcessEip712JsonChunk(MpuSandboxInput_t *input, MpuSandboxRw_t *rw)
{
    MpuSandboxProcessPayloadValidationChunk(input, rw,
                                            MPU_SANDBOX_PAYLOAD_EIP712_JSON,
                                            MPU_SANDBOX_VALIDATION_EIP712_JSON);
}

static bool MpuSandboxProcessValidationChunk(MpuSandboxInput_t *input,
                                             MpuSandboxOutput_t *output,
                                             uint8_t *data, size_t capacity,
                                             uint16_t *length, uint16_t *total_length,
                                             bool *active, MpuSandboxValidationKind_t kind)
{
    uint32_t inputLength = input->request.input_length;
    MpuSandboxUrChunkResult_t result = {
        .validation_status = MPU_SANDBOX_UR_NOT_CHECKED,
        .accepted_length = 0U,
    };

    if (inputLength < MPU_SANDBOX_UR_CHUNK_HEADER_SIZE) {
        goto invalid;
    }
    uint16_t offset = (uint16_t)input->input[0] |
                      ((uint16_t)input->input[1] << 8U);
    uint16_t totalLength = (uint16_t)input->input[2] |
                           ((uint16_t)input->input[3] << 8U);
    uint32_t chunkLength = inputLength - MPU_SANDBOX_UR_CHUNK_HEADER_SIZE;
    if ((totalLength == 0U) || (totalLength > capacity) ||
            (chunkLength == 0U) ||
            (chunkLength > MPU_SANDBOX_UR_CHUNK_DATA_SIZE) ||
            ((uint32_t)offset + chunkLength > totalLength)) {
        goto invalid;
    }
    if (offset == 0U) {
        SandboxZero(data, capacity);
        *length = 0U;
        *total_length = totalLength;
        *active = true;
    } else if (!*active || (*total_length != totalLength) || (*length != offset)) {
        goto invalid;
    }

    SandboxCopy(data + offset,
                input->input + MPU_SANDBOX_UR_CHUNK_HEADER_SIZE,
                chunkLength);
    *length = (uint16_t)(offset + chunkLength);
    result.accepted_length = *length;
    if (*length == *total_length) {
        switch (kind) {
        case MPU_SANDBOX_VALIDATION_CBOR:
            result.validation_status = mpu_sandbox_validate_cbor(data, *length);
            break;
        case MPU_SANDBOX_VALIDATION_EIP712_JSON:
            result.validation_status = mpu_sandbox_validate_eip712_json(data, *length);
            break;
        case MPU_SANDBOX_VALIDATION_UR:
        default:
            result.validation_status = mpu_sandbox_validate_ur(data, *length);
            break;
        }
        *active = false;
    }
    SandboxCopy(output->output, &result, sizeof(result));
    output->response.output_length = sizeof(result);
    __asm volatile("dmb" ::: "memory");
    output->response.status = MPU_SANDBOX_STATUS_OK;
    return true;

invalid:
    SandboxZero(data, capacity);
    *length = 0U;
    *total_length = 0U;
    *active = false;
    output->response.status = MPU_SANDBOX_STATUS_INVALID_REQUEST;
    return false;
}
