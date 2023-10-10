#ifndef EAPDU_PROTOCOL_PARSER_H
#define EAPDU_PROTOCOL_PARSER_H

#include "protocol_parse.h"

#define EAPDU_PROTOCOL_HEADER             0x00
#define EAPDU_PROTOCOL_PARSER_NAME        "eapdu_protocol_parser"

enum { OFFSET_CLA = 0, OFFSET_INS, OFFSET_P1, OFFSET_P2, OFFSET_LC, OFFSET_CDATA };

typedef enum {
    CMD_ECHO_TEST = 0x00000001,    // Command to test echo
    CMD_RESOLVE_UR,                // Command to resolve UR
    CMD_CHECK_LOCK_STATUS,         // Command to check lock status
    //
    CMD_MAX_VALUE = 0xFFFFFFFF,    // The maximum value for command
} CommandType;

typedef enum {
    RSP_SUCCESS_CODE = 0x00000000, // Response success code
    RSP_FAILURE_CODE,              // Response failure code
    //
    RSP_MAX_VALUE = 0xFFFFFFFF,
} StatusEnum;

typedef struct
{
    uint8_t *data;
    uint32_t dataLen;
} EAPDURequestPayload_t;

typedef struct
{
    uint8_t *data;
    uint32_t dataLen;
    StatusEnum status;
} EAPDUResponsePayload_t;

struct ProtocolParser* NewEApduProtocolParser();
void SendEApduResponse(uint8_t cla, CommandType ins, EAPDUResponsePayload_t *payload);

#endif