#ifndef EAPDU_FRAMING_H
#define EAPDU_FRAMING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define EAPDU_FRAMING_HEADER_SIZE       9U
#define EAPDU_FRAMING_MAX_PACKET_SIZE   64U
#define EAPDU_FRAMING_MAX_DATA_SIZE     (EAPDU_FRAMING_MAX_PACKET_SIZE - EAPDU_FRAMING_HEADER_SIZE)
#define EAPDU_FRAMING_MAX_PACKETS       200U
#define EAPDU_FRAMING_MAX_PAYLOAD_SIZE  (EAPDU_FRAMING_MAX_PACKETS * EAPDU_FRAMING_MAX_DATA_SIZE)
#define EAPDU_FRAMING_TIMEOUT_MS        5000U
#define EAPDU_FRAMING_TOTAL_TIMEOUT_MS  15000U

typedef enum {
    EAPDU_FRAMING_WAITING = 0,
    EAPDU_FRAMING_COMPLETE,
    EAPDU_FRAMING_DUPLICATE,
    EAPDU_FRAMING_ERROR_SHORT_FRAME,
    EAPDU_FRAMING_ERROR_TOTAL_PACKETS,
    EAPDU_FRAMING_ERROR_PACKET_INDEX,
    EAPDU_FRAMING_ERROR_PACKET_SIZE,
    EAPDU_FRAMING_ERROR_TOTAL_MISMATCH,
    EAPDU_FRAMING_ERROR_METADATA_MISMATCH,
} EapduFramingStatus_t;

typedef struct {
    EapduFramingStatus_t status;
    bool timed_out;
    uint8_t cla;
    uint16_t command_type;
    uint16_t request_id;
    uint16_t total_packets;
    uint16_t packet_index;
    uint16_t first_missing_packet;
    const uint8_t *payload;
    uint32_t payload_length;
} EapduFramingResult_t;

typedef struct {
    uint8_t packet_data[EAPDU_FRAMING_MAX_PAYLOAD_SIZE + 1U];
    uint8_t packet_lengths[EAPDU_FRAMING_MAX_PACKETS];
    uint8_t received_packets[EAPDU_FRAMING_MAX_PACKETS];
    uint32_t started_tick;
    uint32_t last_packet_tick;
    uint16_t total_packets;
    uint16_t received_count;
    uint16_t command_type;
    uint16_t request_id;
    uint8_t cla;
    bool active;
} EapduFramingState_t;

void EapduFramingReset(EapduFramingState_t *state);
EapduFramingResult_t EapduFramingPush(EapduFramingState_t *state, const uint8_t *frame,
                                      size_t frame_length, uint32_t tick);

#endif
