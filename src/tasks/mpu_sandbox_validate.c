/* Privileged-side chunk loop for sandbox payload validation.
 *
 * Splits a payload into MPU_SANDBOX_UR_CHUNK_DATA_SIZE chunks, frames each
 * with MpuSandboxWriteChunkHeader and hands it to the sandbox through
 * MpuSandboxRoundTrip. On the device the round trip is the FreeRTOS submit /
 * poll pair in mpu_sandbox_task.c; the simulator (ui_simulator/
 * mpu_sandbox_sim.c) calls the sandbox runtime directly, so this file and the
 * runtime it drives are the same code on both. Any protocol violation is a
 * fail-closed event (MpuSandboxValidationFailClosed). */

#include <string.h>

#include "mpu_sandbox_internal.h"

bool MpuSandboxIsUrValidationStatusKnown(uint32_t status)
{
    return (status >= (uint32_t)MPU_SANDBOX_UR_OK) &&
           (status <= (uint32_t)MPU_SANDBOX_UR_INPUT_TOO_LARGE);
}

static bool MpuSandboxValidateBuffer(MpuSandboxOperation_t operation,
                                     size_t maximum_length,
                                     const uint8_t *input, size_t input_length,
                                     uint32_t *validation_status)
{
    uint8_t chunk[MPU_SANDBOX_INPUT_SIZE];
    size_t offset = 0U;
    uint32_t finalStatus = MPU_SANDBOX_UR_NOT_CHECKED;

    if ((input == NULL) || (input_length == 0U) || (validation_status == NULL)) {
        return false;
    }
    if ((input_length > maximum_length) || (input_length > UINT32_MAX)) {
        /* The payload was reassembled fine; it only exceeds this build's
         * policy limit, which has its own status. */
        *validation_status = MPU_SANDBOX_UR_INPUT_TOO_LARGE;
        return true;
    }

    while (offset < input_length) {
        size_t chunkLength = input_length - offset;
        MpuSandboxResponse_t response;
        MpuSandboxUrChunkResult_t result;

        if (chunkLength > MPU_SANDBOX_UR_CHUNK_DATA_SIZE) {
            chunkLength = MPU_SANDBOX_UR_CHUNK_DATA_SIZE;
        }
        memset(chunk, 0, sizeof(chunk));
        MpuSandboxWriteChunkHeader(chunk, (uint32_t)offset, (uint32_t)input_length);
        memcpy(chunk + MPU_SANDBOX_UR_CHUNK_HEADER_SIZE, input + offset, chunkLength);

        if (!MpuSandboxRoundTrip(operation, chunk,
                                 MPU_SANDBOX_UR_CHUNK_HEADER_SIZE + chunkLength,
                                 &response, &result)) {
            MpuSandboxValidationFailClosed();
        }

        offset += chunkLength;
        if ((response.status != MPU_SANDBOX_STATUS_OK) ||
                (response.output_length != sizeof(result)) ||
                (result.accepted_length != offset)) {
            MpuSandboxValidationFailClosed();
        }
        if (offset < input_length) {
            if (result.validation_status != MPU_SANDBOX_UR_NOT_CHECKED) {
                MpuSandboxValidationFailClosed();
            }
        } else {
            if (!MpuSandboxIsUrValidationStatusKnown(result.validation_status)) {
                MpuSandboxValidationFailClosed();
            }
            finalStatus = result.validation_status;
        }
    }

    memset(chunk, 0, sizeof(chunk));
    *validation_status = finalStatus;
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
