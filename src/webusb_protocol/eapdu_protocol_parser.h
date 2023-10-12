#ifndef EAPDU_PROTOCOL_PARSER_H
#define EAPDU_PROTOCOL_PARSER_H

#include "protocol_parse.h"
#include "cJSON.h"

#define EAPDU_PROTOCOL_HEADER             0x00
#define EAPDU_PROTOCOL_PARSER_NAME        "eapdu_protocol_parser"

enum { OFFSET_CLA = 0, OFFSET_INS = 1, OFFSET_P1 = 3, OFFSET_P2 = 5, OFFSET_LC = 7, OFFSET_CDATA = 9 };

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
    // Resolve UR response status
    PRS_PARSING_REJECTED,
    PRS_PARSING_ERROR,
    PRS_PARSING_DISALLOWED,
    
    // 0xA0000001 and beyond are client error codes
    RSP_MAX_VALUE = 0xFFFFFFFF,
} StatusEnum;

typedef struct
{
    uint8_t cla;
    CommandType ins;
    uint16_t p1;
    uint16_t p2;
    uint16_t lc;
    uint8_t *data;
    uint32_t dataLen;
} EAPDUFrame_t;

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