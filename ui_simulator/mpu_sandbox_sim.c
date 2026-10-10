/* Simulator transport for the MPU sandbox.
 *
 * There is no MPU or sandbox task on the host, so the shared chunk loop
 * (src/tasks/mpu_sandbox_validate.c) is given a round trip that fills the
 * request block and calls the sandbox runtime synchronously. Everything the
 * device runs for a validation, other than the FreeRTOS hand-off, therefore
 * runs in the simulator too: chunk framing, size limits, streaming CBOR
 * validation, and the response protocol checks. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpu_sandbox_internal.h"

MpuSandboxInput_t g_mpuSandboxInput;
MpuSandboxRw_t g_mpuSandboxRw;
static uint32_t g_mpuSandboxSimNextRequestId = 1U;

bool MpuSandboxRoundTrip(MpuSandboxOperation_t operation, const uint8_t *chunk,
                         size_t chunk_length, MpuSandboxResponse_t *response,
                         MpuSandboxUrChunkResult_t *result)
{
    uint32_t requestId;

    if ((chunk == NULL) || (response == NULL) || (result == NULL) ||
            (chunk_length > sizeof(g_mpuSandboxInput.input))) {
        return false;
    }

    requestId = g_mpuSandboxSimNextRequestId++;
    if (g_mpuSandboxSimNextRequestId == 0U) {
        g_mpuSandboxSimNextRequestId = 1U;
    }

    memset(&g_mpuSandboxInput, 0, sizeof(g_mpuSandboxInput));
    memset(&g_mpuSandboxRw.output, 0, sizeof(g_mpuSandboxRw.output));
    memcpy(g_mpuSandboxInput.input, chunk, chunk_length);
    g_mpuSandboxInput.request.request_id = requestId;
    g_mpuSandboxInput.request.operation = operation;
    g_mpuSandboxInput.request.input_block_id = MPU_SANDBOX_BLOCK_INPUT;
    g_mpuSandboxInput.request.result_block_id = MPU_SANDBOX_BLOCK_RESULT;
    g_mpuSandboxInput.request.input_length = (uint32_t)chunk_length;
    g_mpuSandboxInput.request.tick = 0U;
    g_mpuSandboxInput.request.reserved = 0U;

    MpuSandboxProcessRequest(&g_mpuSandboxInput, &g_mpuSandboxRw);

    *response = g_mpuSandboxRw.output.response;
    if (response->output_length == sizeof(*result)) {
        memcpy(result, g_mpuSandboxRw.output.output, sizeof(*result));
    } else {
        memset(result, 0, sizeof(*result));
    }
    memset(&g_mpuSandboxInput, 0, sizeof(g_mpuSandboxInput));
    memset(&g_mpuSandboxRw.output, 0, sizeof(g_mpuSandboxRw.output));
    return true;
}

void MpuSandboxValidationFailClosed(void)
{
    fprintf(stderr, "mpu sandbox: validation protocol violation (device would reset)\n");
    abort();
}
