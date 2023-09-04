/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: device info protocol service.
 * Author: leon sun
 * Create: 2023-6-29
 ************************************************************************************************/


#include "service_device_info.h"
#include "stdio.h"
#include "protocol_codec.h"
#include "string.h"
#include "log_print.h"
#include "presetting.h"
#include "hardware_version.h"
#include "version.h"


#define TYPE_DEVICE_MODEL                       1
#define TYPE_DEVICE_SERIAL_NUMBER               2
#define TYPE_DEVICE_HARDWARE_VERSION            3
#define TYPE_DEVICE_FIRMWARE_VERSION            4


static uint8_t *ServiceDeviceInfoBasic(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen);
static uint8_t *ServiceDeviceInfoRunning(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen);


const ProtocolServiceCallbackFunc_t g_deviceInfoServiceFunc[] = {
    NULL,                                       //1.0
    ServiceDeviceInfoBasic,                     //1.1
    ServiceDeviceInfoRunning,                   //1.2
};


static uint8_t *ServiceDeviceInfoBasic(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen)
{
    Tlv_t tlvArray[4] = {0};
    char model[] = "Kv3A";
    char serialNumber[SERIAL_NUMBER_MAX_LEN];
    char version[32];
    FrameHead_t sendHead = {0};

    printf("ServiceDeviceInfoBasic\n");
    PrintArray("tlvData", tlvData, head->length);
    sendHead.packetIndex = head->packetIndex;
    sendHead.serviceId = head->serviceId;
    sendHead.commandId = head->commandId;
    sendHead.flag.b.ack = 0;
    sendHead.flag.b.isHost = 0;

    tlvArray[0].type = TYPE_DEVICE_MODEL;
    tlvArray[0].length = strlen(model) + 1;
    tlvArray[0].pValue = model;

    GetSerialNumber(serialNumber);
    tlvArray[1].type = TYPE_DEVICE_SERIAL_NUMBER;
    tlvArray[1].length = strlen(serialNumber) + 1;
    tlvArray[1].pValue = serialNumber;

    tlvArray[2].type = TYPE_DEVICE_HARDWARE_VERSION;
    tlvArray[2].length = strlen(GetHardwareVersionString()) + 1;
    tlvArray[2].pValue = GetHardwareVersionString();

    GetSoftWareVersionNumber(version);
    tlvArray[3].type = TYPE_DEVICE_FIRMWARE_VERSION;
    tlvArray[3].length = strlen(version) + 1;
    tlvArray[3].pValue = version;

    *outLen = GetFrameTotalLength(tlvArray, 4);
    return BuildFrame(&sendHead, tlvArray, 4);
}


static uint8_t *ServiceDeviceInfoRunning(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen)
{
    printf("ServiceDeviceInfoRunning\n");
    PrintArray("tlvData", tlvData, head->length);
    return NULL;
}

