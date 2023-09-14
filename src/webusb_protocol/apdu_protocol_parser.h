#ifndef APDU_PROTOCOL_PARSER_H
#define APDU_PROTOCOL_PARSER_H

#include "protocol_parse.h"

#define APDU_PROTOCOL_HEADER             0x00
#define APDU_PROTOCOL_PARSER_NAME        "apdu_protocol_parser"

struct ProtocolParser* NewApduProtocolParser();

#endif