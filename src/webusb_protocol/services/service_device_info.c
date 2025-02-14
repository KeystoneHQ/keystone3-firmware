#include "service_device_info.h"
#include "stdio.h"
#include "protocol_codec.h"
#include "string.h"
#include "log_print.h"
#include "presetting.h"
#include "hardware_version.h"
#include "version.h"
#include "safe_str_lib.h"

#define TYPE_DEVICE_MODEL                       1
#define TYPE_DEVICE_SERIAL_NUMBER               2
#define TYPE_DEVICE_HARDWARE_VERSION            3
#define TYPE_DEVICE_FIRMWARE_VERSION            4
#define TYPE_DEVICE_BOOT_VERSION                5

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
    const char model[] = "Kv3A";
    char serialNumber[SERIAL_NUMBER_MAX_LEN];
    char version[SOFTWARE_VERSION_MAX_LEN];
    char bootVersion[SOFTWARE_VERSION_MAX_LEN];

    FrameHead_t sendHead = {0};

    printf("ServiceDeviceInfoBasic\n");
    PrintArray("tlvData", tlvData, head->length);
    sendHead.packetIndex = head->packetIndex;
    sendHead.serviceId = head->serviceId;
    sendHead.commandId = head->commandId;
    sendHead.flag.b.ack = 0;
    sendHead.flag.b.isHost = 0;

    tlvArray[0].type = TYPE_DEVICE_MODEL;
    tlvArray[0].length = strnlen_s(model, 8) + 1;
    tlvArray[0].pValue = (void *)model;

    GetSerialNumber(serialNumber);
    tlvArray[1].type = TYPE_DEVICE_SERIAL_NUMBER;
    tlvArray[1].length = strnlen_s(serialNumber, SERIAL_NUMBER_MAX_LEN) + 1;
    tlvArray[1].pValue = serialNumber;

    tlvArray[2].type = TYPE_DEVICE_HARDWARE_VERSION;
    tlvArray[2].length = strnlen_s(GetHardwareVersionString(), 16) + 1;
    tlvArray[2].pValue = GetHardwareVersionString();

    GetUpdateVersionNumber(version);
    tlvArray[3].type = TYPE_DEVICE_FIRMWARE_VERSION;
    tlvArray[3].length = strnlen_s(version, SOFTWARE_VERSION_MAX_LEN) + 1;
    tlvArray[3].pValue = version;

    uint32_t major, minor, build;
    GetBootSoftwareVersion(&major, &minor, &build);
    snprintf_s(bootVersion, sizeof(bootVersion), "%d.%d.%d", major, minor, build);
    tlvArray[4].type = TYPE_DEVICE_BOOT_VERSION;
    tlvArray[4].length = strnlen_s(bootVersion, SOFTWARE_VERSION_MAX_LEN) + 1;
    tlvArray[4].pValue = bootVersion;

    *outLen = GetFrameTotalLength(tlvArray, 5);
    return BuildFrame(&sendHead, tlvArray, 5);
}

static uint8_t *ServiceDeviceInfoRunning(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen)
{
    printf("ServiceDeviceInfoRunning\n");
    PrintArray("tlvData", tlvData, head->length);
    return NULL;
}
