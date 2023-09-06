/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Protocol codec.
 * Author: leon sun
 * Create: 2023-6-27
 ************************************************************************************************/


#ifndef _PROTOCOL_CODEC_H
#define _PROTOCOL_CODEC_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

#define PROTOCOL_MAX_LENGTH         4500

#define PROTOCOL_HEADER             0x6B
#define PROTOCOL_VERSION            0

typedef union {
    struct {
        uint16_t ack        : 1;
        uint16_t isHost     : 1;
    } b;
    uint16_t u16;
} FrameHeadFlag_t;

#pragma pack(1)
typedef struct {
    uint8_t head;
    uint8_t protocolVersion;
    uint16_t packetIndex;
    uint8_t serviceId;
    uint8_t commandId;
    FrameHeadFlag_t flag;
    uint16_t length;
} FrameHead_t;
#pragma pack()


typedef struct {
    uint8_t type;
    uint16_t length;
    void *pValue;
    uint32_t value;
} Tlv_t;


uint8_t *BuildFrame(FrameHead_t *pHead, const Tlv_t tlvArray[], uint32_t tlvLen);
uint32_t GetFrameTotalLength(const Tlv_t tlvArray[], uint32_t tlvLen);
uint32_t GetTlvFromData(Tlv_t tlvArray[], uint32_t maxTlvLen, const uint8_t *data, uint32_t dataLen);
void PrintProtocolFrame(FrameHead_t *pHead, const Tlv_t tlvArray[], uint32_t tlvLen);
void PrintFrameHead(const FrameHead_t *pHead);

void ProtocolCodecTest(int argc, char *argv[]);

#endif
