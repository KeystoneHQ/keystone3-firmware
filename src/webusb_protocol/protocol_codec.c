#include "protocol_codec.h"
#include "stdio.h"
#include "string.h"
#include "log_print.h"
#include "user_memory.h"
#include "user_utils.h"
#include "assert.h"
#include "crc.h"
#include "protocol_parse.h"



/// @brief Build frame.
/// @param pHead
/// @param tlvArray
/// @param tlvLen
/// @return full frame data which needs be freed manually.
uint8_t *BuildFrame(FrameHead_t *pHead, const Tlv_t tlvArray[], uint32_t tlvLen)
{
    uint32_t totalLen, i, index, crc32Calc;
    uint8_t *sendData;

    pHead->head = PROTOCOL_HEADER;
    pHead->protocolVersion = PROTOCOL_VERSION;
    totalLen = GetFrameTotalLength(tlvArray, tlvLen);
    pHead->length = totalLen - sizeof(FrameHead_t) - 4;
    printf("totalLen=%d\n", totalLen);
    sendData = SRAM_MALLOC(totalLen);
    memcpy(sendData, pHead, sizeof(FrameHead_t));
    index = sizeof(FrameHead_t);
    for (i = 0; i < tlvLen; i++) {
        //t
        sendData[index++] = tlvArray[i].type;
        //l
        ASSERT(tlvArray[i].length <= 0x7FFF);
        if (tlvArray[i].length > 127) {
            sendData[index++] = 0x80 | (tlvArray[i].length >> 8);
            sendData[index++] = tlvArray[i].length & 0xFF;
        } else {
            sendData[index++] = tlvArray[i].length;
        }
        //v
        if (tlvArray[i].pValue == NULL) {
            ASSERT(tlvArray[i].length <= 4);
            memcpy(&sendData[index], &tlvArray[i].value, tlvArray[i].length);
        } else {
            memcpy(&sendData[index], tlvArray[i].pValue, tlvArray[i].length);
        }
        index += tlvArray[i].length;
    }
    crc32Calc = crc32_ieee(0, sendData, totalLen - 4);
    printf("crc32Calc=0x%X\n", crc32Calc);
    memcpy(&sendData[index], &crc32Calc, 4);
    //PrintArray("sendData", sendData, totalLen);
    return sendData;
}


/// @brief Get the frame total length by tlv array.
/// @param tlvArray
/// @return The frame total length.
uint32_t GetFrameTotalLength(const Tlv_t tlvArray[], uint32_t tlvLen)
{
    uint32_t totalLen, i;

    totalLen = sizeof(FrameHead_t);                         //HEAD
    for (i = 0; i < tlvLen; i++) {
        totalLen++;                                         //t
        totalLen += tlvArray[i].length > 127 ? 2 : 1;       //l
        totalLen += tlvArray[i].length;                     //v
    }
    totalLen += 4;                                           //CRC32

    return totalLen;
}


/// @brief Get TLV array from data.
/// @param[out] tlvArray
/// @param[in] maxTlvLen
/// @param[in] data
/// @param[in] dataLen
/// @return TLV array number.
uint32_t GetTlvFromData(Tlv_t tlvArray[], uint32_t maxTlvLen, const uint8_t *data, uint32_t dataLen)
{
    uint32_t count, index = 0;

    for (count = 0; count < maxTlvLen; count++) {
        tlvArray[count].type = data[index++];
        if (data[index] > 127) {
            tlvArray[count].length = ((uint16_t)(data[index] & 0x7F) << 8) + data[index + 1];
            index += 2;
        } else {
            tlvArray[count].length = data[index];
            index++;
        }
        tlvArray[count].pValue = (uint8_t *)&data[index];
        index += tlvArray[count].length;
        ASSERT(index <= dataLen);
        //printf("type=%d\n", tlvArray[count].type);
        //PrintArray("v", tlvArray[count].pValue, tlvArray[count].length);
        if (index == dataLen) {
            //printf("TLV complete\n");
            count++;
            break;
        }
    }
    return count;
}


void PrintProtocolFrame(FrameHead_t *pHead, const Tlv_t tlvArray[], uint32_t tlvLen)
{
    uint32_t totalLen, i, index, crc32Calc;
    uint8_t *sendData;

    totalLen = sizeof(FrameHead_t);                         //HEAD
    for (i = 0; i < tlvLen; i++) {
        totalLen++;                                         //t
        totalLen += tlvArray[i].length > 127 ? 2 : 1;       //l
        totalLen += tlvArray[i].length;                     //v
    }
    totalLen += 4;                                           //CRC32
    pHead->length = totalLen - sizeof(FrameHead_t) - 4;
    printf("totalLen=%d\n", totalLen);
    sendData = SRAM_MALLOC(totalLen);
    memcpy(sendData, pHead, sizeof(FrameHead_t));
    index = sizeof(FrameHead_t);
    for (i = 0; i < tlvLen; i++) {
        //t
        sendData[index++] = tlvArray[i].type;
        //l
        ASSERT(tlvArray[i].length <= 0x7FFF);
        if (tlvArray[i].length > 127) {
            sendData[index++] = 0x80 | (tlvArray[i].length >> 8);
            sendData[index++] = tlvArray[i].length & 0xFF;
        } else {
            sendData[index++] = tlvArray[i].length;
        }
        //v
        if (tlvArray[i].pValue == NULL) {
            ASSERT(tlvArray[i].length <= 4);
            memcpy(&sendData[index], &tlvArray[i].value, tlvArray[i].length);
        } else {
            memcpy(&sendData[index], tlvArray[i].pValue, tlvArray[i].length);
        }
        index += tlvArray[i].length;
    }
    crc32Calc = crc32_ieee(0, sendData, totalLen - 4);
    printf("crc32Calc=0x%X\n", crc32Calc);
    memcpy(&sendData[index], &crc32Calc, 4);
    PrintArray("sendData", sendData, totalLen);
    SRAM_FREE(sendData);
}


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
        PrintProtocolFrame(&head, tlvArray, 5);
    } else if (strcmp(argv[0], "parse") == 0) {
        VALUE_CHECK(argc, 2);
        inData = SRAM_MALLOC(strlen(argv[1]) / 2);
        length = StrToHex(inData, argv[1]);
        printf("length=%d\n", length);
        ProtocolReceivedData(inData, length, ProtocolTestSend);
    }
}


