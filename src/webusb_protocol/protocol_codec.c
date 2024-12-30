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
        if (index + 1 > dataLen) {
            break;
        }
        tlvArray[count].type = data[index++];

        if (index + 1 > dataLen) {
            break;
        }
        if (data[index] > 127) {
            if (index + 2 > dataLen) break;
            tlvArray[count].length = ((uint16_t)(data[index] & 0x7F) << 8) + data[index + 1];
            index += 2;
        } else {
            tlvArray[count].length = data[index++];
        }

        if (index + tlvArray[count].length > dataLen || tlvArray[count].length == 0) {
            break;
        }
        tlvArray[count].pValue = (uint8_t *)&data[index];
        index += tlvArray[count].length;

        count++;
    }

    return count;
}
