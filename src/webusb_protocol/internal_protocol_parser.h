#ifndef INTERNAL_PROTOCOL_PARSER_H
#define INTERNAL_PROTOCOL_PARSER_H

#include "protocol_parse.h"

#define INTERNAL_PROTOCOL_HEADER             0x6B
#define INTERNAL_PROTOCOL_PARSER_NAME        "internal_protocol_parser"

struct ProtocolParser* NewInternalProtocolParser();

#endif