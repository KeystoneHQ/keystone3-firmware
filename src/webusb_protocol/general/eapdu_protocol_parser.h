#ifndef EAPDU_PROTOCOL_PARSER_H
#define EAPDU_PROTOCOL_PARSER_H

#include "protocol_parse.h"
#include "cJSON.h"
#include "librust_c.h"
// #include "gui_framework.h"
// #include "gui_views.h"

#define EAPDU_PROTOCOL_HEADER             0x00
#define EAPDU_PROTOCOL_PARSER_NAME        "eapdu_protocol_parser"

enum { OFFSET_CLA = 0, OFFSET_INS = 1, OFFSET_P1 = 3, OFFSET_P2 = 5, OFFSET_LC = 7, OFFSET_CDATA = 9 };

typedef enum {
    CMD_ECHO_TEST = 0x00000001,    // Command to test echo
    CMD_RESOLVE_UR,                // Command to resolve UR
    CMD_CHECK_LOCK_STATUS,         // Command to check lock status
    CMD_EXPORT_ADDRESS,            // Command to export address
    CMD_GET_DEVICE_INFO,           // Command to get device info
    CMD_GET_DEVICE_USB_PUBKEY,     // Command to get device public key

    CMD_MAX_VALUE = 0xFFFFFFFF,    // The maximum value for command
} CommandType;

typedef enum {
    RSP_SUCCESS_CODE = 0x00000000, // Response success code
    RSP_FAILURE_CODE,              // Response failure code
    PRS_INVALID_TOTAL_PACKETS,
    PRS_INVALID_INDEX,
    // Resolve UR response status
    PRS_PARSING_REJECTED,
    PRS_PARSING_ERROR,
    PRS_PARSING_DISALLOWED,
    PRS_PARSING_UNMATCHED,
    PRS_PARSING_MISMATCHED_WALLET,
    PRS_PARSING_VERIFY_PASSWORD_ERROR,
    PRS_EXPORT_ADDRESS_UNSUPPORTED_CHAIN,
    PRS_EXPORT_ADDRESS_INVALID_PARAMS,
    PRS_EXPORT_ADDRESS_ERROR,
    PRS_EXPORT_ADDRESS_DISALLOWED,
    PRS_EXPORT_ADDRESS_REJECTED,
    PRS_EXPORT_ADDRESS_BUSY,
    PRS_EXPORT_HARDWARE_CALL_SUCCESS,
    // 0xA0000001 and beyond are client error codes
    RSP_MAX_VALUE = 0xFFFFFFFF,
} StatusEnum;

typedef struct {
    uint8_t cla;
    CommandType ins;
    uint16_t p1;
    uint16_t p2;
    uint16_t lc;
    uint8_t *data;
    uint32_t dataLen;
} EAPDUFrame_t;

typedef struct {
    uint8_t cla;
    uint8_t *data;
    uint32_t dataLen;
    uint16_t requestID; // lc
    CommandType commandType; // ins
} EAPDURequestPayload_t;

typedef struct {
    uint8_t cla;
    uint8_t *data;
    uint32_t dataLen;
    StatusEnum status;
    uint16_t requestID; // lc
    CommandType commandType; // ins
} EAPDUResponsePayload_t;

typedef struct {
    CommandType command;
    uint32_t error_code;
    PtrString error_message;
} EAPDUResultPage_t;

struct ProtocolParser* NewEApduProtocolParser();
void SendEApduResponse(EAPDUResponsePayload_t *payload);
void GotoResultPage(EAPDUResultPage_t *resultPageParams);
void SendEApduResponseError(uint8_t cla, CommandType ins, uint16_t requestID, StatusEnum status, char *error);

#endif