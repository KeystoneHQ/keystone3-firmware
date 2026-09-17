#include "eapdu_framing.h"

enum {
    EAPDU_OFFSET_CLA = 0,
    EAPDU_OFFSET_INS = 1,
    EAPDU_OFFSET_TOTAL = 3,
    EAPDU_OFFSET_INDEX = 5,
    EAPDU_OFFSET_REQUEST_ID = 7,
};

static void CopyBytes(void *destination, const void *source, size_t length)
{
    uint8_t *destinationBytes = destination;
    const uint8_t *sourceBytes = source;

    for (size_t i = 0U; i < length; i++) {
        destinationBytes[i] = sourceBytes[i];
    }
}

static void MoveBytes(void *destination, const void *source, size_t length)
{
    uint8_t *destinationBytes = destination;
    const uint8_t *sourceBytes = source;

    if (destinationBytes <= sourceBytes) {
        CopyBytes(destination, source, length);
        return;
    }

    for (size_t i = length; i > 0U; i--) {
        destinationBytes[i - 1U] = sourceBytes[i - 1U];
    }
}

static void ZeroBytes(void *destination, size_t length)
{
    volatile uint8_t *destinationBytes = destination;

    for (size_t i = 0U; i < length; i++) {
        destinationBytes[i] = 0U;
    }
}

static uint16_t ReadU16(const uint8_t *data, size_t offset)
{
    return ((uint16_t)data[offset] << 8U) | data[offset + 1U];
}

static EapduFramingResult_t ResetWithError(EapduFramingState_t *state,
                                           EapduFramingResult_t result,
                                           EapduFramingStatus_t status)
{
    result.status = status;
    EapduFramingReset(state);
    return result;
}

void EapduFramingReset(EapduFramingState_t *state)
{
    if (state != NULL) {
        ZeroBytes(state, sizeof(*state));
    }
}

EapduFramingResult_t EapduFramingPush(EapduFramingState_t *state, const uint8_t *frame,
                                      size_t frame_length, uint32_t tick)
{
    EapduFramingResult_t result;
    size_t packet_data_length;
    size_t packet_offset;
    size_t compacted_length = 0U;

    ZeroBytes(&result, sizeof(result));
    result.status = EAPDU_FRAMING_WAITING;
    result.first_missing_packet = UINT16_MAX;

    if (state == NULL) {
        result.status = EAPDU_FRAMING_ERROR_SHORT_FRAME;
        return result;
    }

    if (state->active &&
            (((uint32_t)(tick - state->last_packet_tick) > EAPDU_FRAMING_TIMEOUT_MS) ||
             ((uint32_t)(tick - state->started_tick) > EAPDU_FRAMING_TOTAL_TIMEOUT_MS))) {
        EapduFramingReset(state);
        result.timed_out = true;
    }

    if ((frame == NULL) || (frame_length < EAPDU_FRAMING_HEADER_SIZE)) {
        return ResetWithError(state, result, EAPDU_FRAMING_ERROR_SHORT_FRAME);
    }

    result.cla = frame[EAPDU_OFFSET_CLA];
    result.command_type = ReadU16(frame, EAPDU_OFFSET_INS);
    result.total_packets = ReadU16(frame, EAPDU_OFFSET_TOTAL);
    result.packet_index = ReadU16(frame, EAPDU_OFFSET_INDEX);
    result.request_id = ReadU16(frame, EAPDU_OFFSET_REQUEST_ID);

    if ((result.total_packets == 0U) ||
            (result.total_packets > EAPDU_FRAMING_MAX_PACKETS)) {
        return ResetWithError(state, result, EAPDU_FRAMING_ERROR_TOTAL_PACKETS);
    }
    if (result.packet_index >= result.total_packets) {
        return ResetWithError(state, result, EAPDU_FRAMING_ERROR_PACKET_INDEX);
    }
    if (frame_length > EAPDU_FRAMING_MAX_PACKET_SIZE) {
        return ResetWithError(state, result, EAPDU_FRAMING_ERROR_PACKET_SIZE);
    }

    if (!state->active) {
        state->active = true;
        state->started_tick = tick;
        state->cla = result.cla;
        state->command_type = result.command_type;
        state->request_id = result.request_id;
        state->total_packets = result.total_packets;
    } else {
        if (state->total_packets != result.total_packets) {
            return ResetWithError(state, result, EAPDU_FRAMING_ERROR_TOTAL_MISMATCH);
        }
        if ((state->cla != result.cla) || (state->command_type != result.command_type) ||
                (state->request_id != result.request_id)) {
            return ResetWithError(state, result, EAPDU_FRAMING_ERROR_METADATA_MISMATCH);
        }
    }

    if (state->received_packets[result.packet_index] != 0U) {
        result.status = EAPDU_FRAMING_DUPLICATE;
        return result;
    }

    packet_data_length = frame_length - EAPDU_FRAMING_HEADER_SIZE;
    packet_offset = (size_t)result.packet_index * EAPDU_FRAMING_MAX_DATA_SIZE;
    if (packet_data_length > 0U) {
        CopyBytes(&state->packet_data[packet_offset], &frame[EAPDU_FRAMING_HEADER_SIZE],
                  packet_data_length);
    }
    state->packet_lengths[result.packet_index] = (uint8_t)packet_data_length;
    state->received_packets[result.packet_index] = 1U;
    state->received_count++;
    state->last_packet_tick = tick;

    if (state->received_count != state->total_packets) {
        for (uint16_t i = 0U; i < state->total_packets; i++) {
            if (state->received_packets[i] == 0U) {
                result.first_missing_packet = i;
                break;
            }
        }
        return result;
    }

    for (uint16_t i = 0U; i < state->total_packets; i++) {
        size_t source_offset = (size_t)i * EAPDU_FRAMING_MAX_DATA_SIZE;
        size_t source_length = state->packet_lengths[i];

        if (source_length > 0U) {
            MoveBytes(&state->packet_data[compacted_length], &state->packet_data[source_offset],
                      source_length);
        }
        compacted_length += source_length;
    }
    state->packet_data[compacted_length] = '\0';

    result.status = EAPDU_FRAMING_COMPLETE;
    result.cla = state->cla;
    result.command_type = state->command_type;
    result.request_id = state->request_id;
    result.total_packets = state->total_packets;
    result.payload = state->packet_data;
    result.payload_length = (uint32_t)compacted_length;
    return result;
}
