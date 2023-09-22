#ifndef APDU_PROTOCOL_PARSER_H
#define APDU_PROTOCOL_PARSER_H

#include "protocol_parse.h"

#define APDU_PROTOCOL_HEADER             0x00
#define APDU_PROTOCOL_PARSER_NAME        "apdu_protocol_parser"

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
} APDURequestPayload_t;

typedef struct
{
    uint8_t *data;
    uint32_t dataLen;
    StatusEnum status;
} APDUResponsePayload_t;

typedef struct {
    uint8_t cla; // Class
    uint8_t ins; // Instruction (CommandEnum)
    uint8_t p1; // Parameter 1 (Packet number for multi-packet commands)
    uint8_t p2; // Parameter 2
    uint8_t lc; // Length of Command Data
    uint8_t data[58]; // Command data (max data length is 58 to fit the 64-byte limit)
    uint8_t le; // Expected length of response data
} APDUFrame;

struct ProtocolParser* NewApduProtocolParser();

#endif