#include "mpu_sandbox_internal.h"
#include "mpu_sandbox_ur_validator.h"

#ifndef COMPILE_SIMULATOR
#include "FreeRTOS.h"
#include "task.h"
#endif

static void MpuSandboxProcessUrChunk(MpuSandboxInput_t *input, MpuSandboxRw_t *rw);
static void MpuSandboxProcessCborStreamChunk(MpuSandboxInput_t *input, MpuSandboxRw_t *rw);
static void MpuSandboxProcessEip712JsonChunk(MpuSandboxInput_t *input, MpuSandboxRw_t *rw);
typedef enum {
    MPU_SANDBOX_VALIDATION_UR = 0,
    MPU_SANDBOX_VALIDATION_EIP712_JSON,
} MpuSandboxValidationKind_t;
static bool MpuSandboxParseChunkHeader(const MpuSandboxInput_t *input, uint32_t *offset,
                                       uint32_t *total_length, uint32_t *chunk_length);
static bool MpuSandboxProcessValidationChunk(MpuSandboxInput_t *input,
                                             MpuSandboxOutput_t *output,
                                             uint8_t *data, size_t capacity,
                                             uint32_t *length, uint32_t *total_length,
                                             bool *active, MpuSandboxValidationKind_t kind);
static void MpuSandboxWriteChunkResult(MpuSandboxOutput_t *output,
                                       const MpuSandboxUrChunkResult_t *result);
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

#ifndef COMPILE_SIMULATOR
void MpuSandboxRuntime(void *argument)
{
    (void)argument;
    for (;;) {
        if (xTaskNotifyWait(0U, UINT32_MAX, NULL, portMAX_DELAY) == pdPASS) {
            MpuSandboxProcessRequest(&g_mpuSandboxInput, &g_mpuSandboxRw);
        }
    }
}
#endif

void MpuSandboxProcessRequest(MpuSandboxInput_t *input, MpuSandboxRw_t *rw)
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
        MPU_SANDBOX_MEMORY_BARRIER();
        output->response.status = MPU_SANDBOX_STATUS_OK;
        break;
    case MPU_SANDBOX_OP_ECHO:
        SandboxCopy(output->output, input->input, inputLength);
        output->response.output_length = inputLength;
        MPU_SANDBOX_MEMORY_BARRIER();
        output->response.status = MPU_SANDBOX_STATUS_OK;
        break;
    case MPU_SANDBOX_OP_UR_VALIDATE_CHUNK:
        MpuSandboxProcessUrChunk(input, rw);
        break;
    case MPU_SANDBOX_OP_CBOR_VALIDATE_CHUNK:
        MpuSandboxProcessCborStreamChunk(input, rw);
        break;
    case MPU_SANDBOX_OP_EIP712_JSON_VALIDATE_CHUNK:
        MpuSandboxProcessEip712JsonChunk(input, rw);
        break;
#if !defined(BTC_ONLY) && !defined(COMPILE_SIMULATOR)
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
        MPU_SANDBOX_MEMORY_BARRIER();
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

static void MpuSandboxProcessEip712JsonChunk(MpuSandboxInput_t *input, MpuSandboxRw_t *rw)
{
    MpuSandboxPayloadValidationState_t *state = &rw->payload.validation;
    bool firstChunk = (input->request.input_length >= MPU_SANDBOX_UR_CHUNK_HEADER_SIZE) &&
                      (input->input[0] == 0U) && (input->input[1] == 0U) &&
                      (input->input[2] == 0U) && (input->input[3] == 0U);

    if (firstChunk) {
        if (rw->payload_mode != MPU_SANDBOX_PAYLOAD_NONE) {
            rw->output.response.status = MPU_SANDBOX_STATUS_INVALID_REQUEST;
            return;
        }
        SandboxZero(&rw->payload, sizeof(rw->payload));
        rw->payload_mode = MPU_SANDBOX_PAYLOAD_EIP712_JSON;
    } else if (rw->payload_mode != MPU_SANDBOX_PAYLOAD_EIP712_JSON) {
        rw->output.response.status = MPU_SANDBOX_STATUS_INVALID_REQUEST;
        return;
    }

    if (!MpuSandboxProcessValidationChunk(input, &rw->output,
                                          state->data, sizeof(state->data),
                                          &state->length, &state->total_length,
                                          &state->active,
                                          MPU_SANDBOX_VALIDATION_EIP712_JSON) ||
            !state->active) {
        SandboxZero(&rw->payload, sizeof(rw->payload));
        rw->payload_mode = MPU_SANDBOX_PAYLOAD_NONE;
    }
}

/* CBOR is validated as a stream: each chunk is pushed straight into the
 * O(1)-memory validator, so that the sandbox never holds a copy of the payload
 * and the payload size is bounded only by the chunk framing and the policy
 * limit MPU_SANDBOX_CBOR_INPUT_MAX_SIZE, not by the RW region. */
static void MpuSandboxProcessCborStreamChunk(MpuSandboxInput_t *input, MpuSandboxRw_t *rw)
{
    MpuSandboxCborStreamState_t *state = &rw->payload.cbor_stream;
    MpuSandboxUrChunkResult_t result = {
        .validation_status = MPU_SANDBOX_UR_NOT_CHECKED,
        .accepted_length = 0U,
    };
    uint32_t offset;
    uint32_t totalLength;
    uint32_t chunkLength;

    if (!MpuSandboxParseChunkHeader(input, &offset, &totalLength, &chunkLength) ||
            (totalLength > MPU_SANDBOX_CBOR_INPUT_MAX_SIZE)) {
        goto invalid;
    }
    if (offset == 0U) {
        if (rw->payload_mode != MPU_SANDBOX_PAYLOAD_NONE) {
            rw->output.response.status = MPU_SANDBOX_STATUS_INVALID_REQUEST;
            return;
        }
        SandboxZero(&rw->payload, sizeof(rw->payload));
        rw->payload_mode = MPU_SANDBOX_PAYLOAD_CBOR;
        if (mpu_sandbox_cbor_stream_init(state->state, sizeof(state->state)) != MPU_SANDBOX_UR_OK) {
            goto invalid;
        }
        state->length = 0U;
        state->total_length = totalLength;
        state->active = true;
    } else if ((rw->payload_mode != MPU_SANDBOX_PAYLOAD_CBOR) || !state->active ||
               (state->total_length != totalLength) || (state->length != offset)) {
        goto invalid;
    }

    if (mpu_sandbox_cbor_stream_push(state->state, sizeof(state->state),
                                     input->input + MPU_SANDBOX_UR_CHUNK_HEADER_SIZE,
                                     chunkLength) != MPU_SANDBOX_UR_OK) {
        goto invalid;
    }
    state->length = offset + chunkLength;
    result.accepted_length = state->length;
    if (state->length == state->total_length) {
        result.validation_status = mpu_sandbox_cbor_stream_finish(state->state,
                                                                  sizeof(state->state));
        SandboxZero(&rw->payload, sizeof(rw->payload));
        rw->payload_mode = MPU_SANDBOX_PAYLOAD_NONE;
    }
    MpuSandboxWriteChunkResult(&rw->output, &result);
    return;

invalid:
    SandboxZero(&rw->payload, sizeof(rw->payload));
    rw->payload_mode = MPU_SANDBOX_PAYLOAD_NONE;
    rw->output.response.status = MPU_SANDBOX_STATUS_INVALID_REQUEST;
}

/* Decodes and range-checks one chunk's framing (offset, total, data length). */
static bool MpuSandboxParseChunkHeader(const MpuSandboxInput_t *input, uint32_t *offset,
                                       uint32_t *total_length, uint32_t *chunk_length)
{
    uint32_t inputLength = input->request.input_length;

    if (inputLength < MPU_SANDBOX_UR_CHUNK_HEADER_SIZE) {
        return false;
    }
    MpuSandboxReadChunkHeader(input->input, offset, total_length);
    *chunk_length = inputLength - MPU_SANDBOX_UR_CHUNK_HEADER_SIZE;
    /* offset + chunk_length <= total_length, without the addition overflowing:
     * the second test only runs once the subtraction cannot wrap. */
    return (*total_length != 0U) &&
           (*chunk_length != 0U) &&
           (*chunk_length <= MPU_SANDBOX_UR_CHUNK_DATA_SIZE) &&
           (*chunk_length <= *total_length) &&
           (*offset <= *total_length - *chunk_length);
}

static void MpuSandboxWriteChunkResult(MpuSandboxOutput_t *output,
                                       const MpuSandboxUrChunkResult_t *result)
{
    SandboxCopy(output->output, result, sizeof(*result));
    output->response.output_length = sizeof(*result);
    MPU_SANDBOX_MEMORY_BARRIER();
    output->response.status = MPU_SANDBOX_STATUS_OK;
}

/* Buffered validation (single-part UR text and EIP-712 JSON): the payload is
 * reassembled in `data` and validated once complete. */
static bool MpuSandboxProcessValidationChunk(MpuSandboxInput_t *input,
                                             MpuSandboxOutput_t *output,
                                             uint8_t *data, size_t capacity,
                                             uint32_t *length, uint32_t *total_length,
                                             bool *active, MpuSandboxValidationKind_t kind)
{
    MpuSandboxUrChunkResult_t result = {
        .validation_status = MPU_SANDBOX_UR_NOT_CHECKED,
        .accepted_length = 0U,
    };
    uint32_t offset;
    uint32_t totalLength;
    uint32_t chunkLength;

    if (!MpuSandboxParseChunkHeader(input, &offset, &totalLength, &chunkLength) ||
            (totalLength > capacity)) {
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
    *length = offset + chunkLength;
    result.accepted_length = *length;
    if (*length == *total_length) {
        switch (kind) {
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
    MpuSandboxWriteChunkResult(output, &result);
    return true;

invalid:
    SandboxZero(data, capacity);
    *length = 0U;
    *total_length = 0U;
    *active = false;
    output->response.status = MPU_SANDBOX_STATUS_INVALID_REQUEST;
    return false;
}
