#include <stdio.h>
#include <string.h>
#include "assert.h"
#include <stdint.h>
#include "protocol_codec.h"
#include "log_print.h"
#include "user_memory.h"
#include "user_utils.h"
#include "crc.h"
#include "protocol_parse.h"

void PrintFrameHead(const FrameHead_t *pHead)
{
    printf("head=0x%X\n", pHead->head);
    printf("version=%d\n", pHead->protocolVersion);
    printf("version=%d\n", pHead->packetIndex);
    printf("serviceId=%d\n", pHead->serviceId);
    printf("commandId=%d\n", pHead->commandId);
    printf("flag.ack=%d\n", pHead->flag.b.ack);
    printf("flag.isHost=%d\n", pHead->flag.b.isHost);
    printf("length=%d\n", pHead->length);
}

/// Calculate the total length of the frame based on the TLV array.
uint32_t GetFrameTotalLength(const Tlv_t tlvArray[], uint32_t tlvLen)
{
    uint32_t totalLen = sizeof(FrameHead_t) + 4; // Adding size for HEAD and CRC32

    for (uint32_t i = 0; i < tlvLen; i++) {
        totalLen++; // t
        totalLen += (tlvArray[i].length > 127) ? 2 : 1; // l
        totalLen += tlvArray[i].length; // v
    }

    return totalLen;
}

/// Encode a single TLV into the provided buffer.
static void EncodeTlv(uint8_t *buffer, uint32_t *index, const Tlv_t tlv, uint32_t totalLen)
{
    buffer[(*index)++] = tlv.type; // t

    if (tlv.length > 127) {
        buffer[(*index)++] = 0x80 | (tlv.length >> 8); // l high
        buffer[(*index)++] = tlv.length & 0xFF;        // l low
    } else {
        buffer[(*index)++] = tlv.length; // l
    }

    assert(tlv.length <= 0x7FFF);
    if (tlv.pValue == NULL) {
        assert(tlv.length <= 4);
        memcpy_s(&buffer[*index], totalLen - *index, &tlv.value, tlv.length);
    } else {
        memcpy_s(&buffer[*index], totalLen - *index, tlv.pValue, tlv.length);
    }
    *index += tlv.length; // v
}

/// Build frame from header and TLV array.
uint8_t *BuildFrame(FrameHead_t *pHead, const Tlv_t tlvArray[], uint32_t tlvLen)
{
    uint32_t totalLen = GetFrameTotalLength(tlvArray, tlvLen);
    uint8_t *sendData = SRAM_MALLOC(totalLen);

    pHead->head = PROTOCOL_HEADER;
    pHead->protocolVersion = PROTOCOL_VERSION;
    pHead->length = totalLen - sizeof(FrameHead_t) - 4;
    printf("Total frame length: %u\n", totalLen);

    memcpy_s(sendData, totalLen, pHead, sizeof(FrameHead_t));
    uint32_t index = sizeof(FrameHead_t);

    for (uint32_t i = 0; i < tlvLen; i++) {
        EncodeTlv(sendData, &index, tlvArray[i], totalLen);
    }

    uint32_t crc32Calc = crc32_ieee(0, sendData, totalLen - 4);
    printf("CRC32 Calculated: 0x%X\n", crc32Calc);
    memcpy_s(&sendData[index], 4, &crc32Calc, 4);

    return sendData;
}

/// Extract TLV array from provided data.
uint32_t GetTlvFromData(Tlv_t tlvArray[], uint32_t maxTlvLen, const uint8_t *data, uint32_t dataLen)
{
    uint32_t count = 0, index = 0;

    while (count < maxTlvLen && index < dataLen) {
        tlvArray[count].type = data[index++]; // t
        if (data[index] > 127) {
            tlvArray[count].length = ((uint16_t)(data[index] & 0x7F) << 8) + data[index + 1];
            index += 2; // l
        } else {
            tlvArray[count].length = data[index++]; // l
        }

        tlvArray[count].pValue = (uint8_t *)&data[index];
        index += tlvArray[count].length; // v
        assert(index <= dataLen);

        count++;
        if (index == dataLen) {
            break;
        }
    }

    return count;
}

static void ProtocolTestSend(const uint8_t *data, uint32_t len)
{
    PrintArray("protocol test send", data, len);
}

void ProtocolCodecTest(int argc, char *argv[])
{
    FrameHead_t head = {0};
    Tlv_t tlvArray[5] = {0};
    uint8_t *inData;
    uint32_t length;

    if (strcmp(argv[0], "build") == 0) {
        head.head = PROTOCOL_HEADER;
        head.protocolVersion = 0;
        head.packetIndex = 0x87;
        head.serviceId = 0x01;
        head.commandId = 0x02;
        head.flag.b.ack = 0;
        head.flag.b.isHost = 1;
        head.length = 1234;
        tlvArray[0].type = 1;
        tlvArray[0].length = strlen("Keystone 3Pro") + 1;
        tlvArray[0].pValue = "Keystone 3Pro";

        tlvArray[1].type = 2;
        tlvArray[1].length = strlen("0000000001") + 1;
        tlvArray[1].pValue = "0000000001";

        tlvArray[2].type = 3;
        tlvArray[2].length = strlen("EVT1") + 1;
        tlvArray[2].pValue = "EVT1";

        tlvArray[3].type = 4;
        tlvArray[3].length = strlen("0.6.8") + 1;
        tlvArray[3].pValue = "0.6.8";

        tlvArray[4].type = 5;
        tlvArray[4].length = 4;
        tlvArray[4].value = 888;
        // PrintProtocolFrame(&head, tlvArray, 5);
    } else if (strcmp(argv[0], "parse") == 0) {
        VALUE_CHECK(argc, 2);
        inData = SRAM_MALLOC(strlen(argv[1]) / 2);
        length = StrToHex(inData, argv[1]);
        printf("length=%d\n", length);
        ProtocolReceivedData(inData, length, ProtocolTestSend);
    }
}
