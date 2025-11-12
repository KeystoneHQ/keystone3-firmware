#include "usbd_composite.h"
#include "circular_buffer.h"
#include "protocol_parse.h"
#include "user_msg.h"
#include <stdio.h>
#include <string.h>

void USBD_cdc_SendBuffer_Cb(const uint8_t *data, uint32_t len);
void USBD_cdc_SendHidBuffer_Cb(const uint8_t *data, uint32_t len);

static const USB_TransportConfig_t g_usbTransportConfigs[] = {
    {USB_TRANSPORT_CDC, USBD_cdc_SendBuffer_Cb, "CDC"},
    {USB_TRANSPORT_HID, USBD_cdc_SendHidBuffer_Cb, "HID"},
};

#ifdef USB_STATS_ENABLED
static USB_Stats_t g_usbStats = {0};

USB_Stats_t* GetUSBStats(void)
{
    return &g_usbStats;
}

void ResetUSBStats(void)
{
    memset(&g_usbStats, 0, sizeof(g_usbStats));
}

#define USB_STATS_INCREMENT(field) g_usbStats.field++
#define USB_STATS_GET() &g_usbStats
#else
#define USB_STATS_INCREMENT(field) do {} while(0)
#define USB_STATS_GET() NULL
#endif

const USB_TransportConfig_t* GetUSBTransportConfig(USB_TransportType_t type)
{
    for (int i = 0; i < sizeof(g_usbTransportConfigs)/sizeof(g_usbTransportConfigs[0]); i++) {
        if (g_usbTransportConfigs[i].type == type) {
            return &g_usbTransportConfigs[i];
        }
    }
    return NULL;
}

void ProcessUSBData(uint32_t dataLen, USB_TransportType_t transportType, cbuf_handle_t cBufHandle)
{
    if (dataLen == 0 || dataLen > USB_RX_BUFFER_SIZE) {
        USB_DEBUG_PRINT("Invalid data length: %u", dataLen);
        USB_STATS_INCREMENT(error_count);
        return;
    }

    const USB_TransportConfig_t* config = GetUSBTransportConfig(transportType);
    if (config == NULL || config->callback == NULL) {
        USB_DEBUG_PRINT("Invalid transport type: %d", transportType);
        USB_STATS_INCREMENT(error_count);
        return;
    }
    
    uint32_t availableData = circular_buf_size(cBufHandle);
    if (availableData < dataLen) {
        USB_DEBUG_PRINT("Insufficient data in buffer: required %u, available %u", 
               dataLen, availableData);
        USB_STATS_INCREMENT(buffer_overflow_count);
        return;
    }
    
    static uint8_t USB_Rx_Buffer[USB_RX_BUFFER_MAX_SIZE] = {0};
    
    uint32_t readCount = 0;
    bool readSuccess = true;
    
    for (uint32_t i = 0; i < dataLen && readSuccess; i++) {
        uint8_t tempByte;
        if (circular_buf_get(cBufHandle, &tempByte) == 0) {
            USB_Rx_Buffer[i] = tempByte;
            readCount++;
        } else {
            USB_DEBUG_PRINT("Failed to read data at index %u", i);
            readSuccess = false;
            break;
        }
    }
    
    if (!readSuccess || readCount != dataLen) {
        USB_DEBUG_PRINT("Data read mismatch: expected %u, got %u", dataLen, readCount);
        USB_STATS_INCREMENT(error_count);
        return;
    }
    
    ProtocolReceivedData(USB_Rx_Buffer, readCount, config->callback);
    
    USB_STATS_INCREMENT(total_processed);
    if (transportType == USB_TRANSPORT_CDC) {
        USB_STATS_INCREMENT(cdc_processed);
    } else if (transportType == USB_TRANSPORT_HID) {
        USB_STATS_INCREMENT(hid_processed);
    }
    
    USB_DEBUG_PRINT("Processed %u bytes via %s transport", readCount, config->name);
}

#ifdef USB_STATS_ENABLED
void PrintUSBStatus(cbuf_handle_t cBufHandle)
{
    USB_Stats_t* stats = GetUSBStats();
    uint32_t hid_total, hid_success, hid_failed, hid_overflow;
    
    GetHIDEventStats(&hid_total, &hid_success, &hid_failed, &hid_overflow);
    
    printf("=== USB Status Report ===\n");
    printf("Data Parser Stats:\n");
    printf("  Total processed: %u\n", stats->total_processed);
    printf("  CDC processed: %u\n", stats->cdc_processed);
    printf("  HID processed: %u\n", stats->hid_processed);
    printf("  Errors: %u\n", stats->error_count);
    printf("  Buffer overflows: %u\n", stats->buffer_overflow_count);
    
    printf("HID Event Stats:\n");
    printf("  Total events: %u\n", hid_total);
    printf("  Successful: %u\n", hid_success);
    printf("  Failed: %u\n", hid_failed);
    printf("  Buffer overflows: %u\n", hid_overflow);
    
    printf("Buffer Status:\n");
    printf("  Available data: %u\n", circular_buf_size(cBufHandle));
    printf("  Buffer capacity: %u\n", circular_buf_capacity(cBufHandle));
    printf("========================\n");
}

void MonitorUSBPerformance(cbuf_handle_t cBufHandle)
{
    static uint32_t last_total = 0;
    static uint32_t last_hid_total = 0;
    static uint32_t last_time = 0;
    
    uint32_t current_time = osKernelGetTickCount();
    USB_Stats_t* stats = GetUSBStats();
    uint32_t hid_total, hid_success, hid_failed, hid_overflow;
    
    GetHIDEventStats(&hid_total, &hid_success, &hid_failed, &hid_overflow);
    
    if (current_time - last_time >= 5000) { // 每5秒打印一次
        uint32_t data_rate = stats->total_processed - last_total;
        uint32_t hid_rate = hid_total - last_hid_total;
        
        printf("USB Performance (5s):\n");
        printf("  Data rate: %u packets/s\n", data_rate / 5);
        printf("  HID event rate: %u events/s\n", hid_rate / 5);
        printf("  Error rate: %.2f%%\n", 
               stats->total_processed > 0 ? 
               (float)stats->error_count * 100.0f / stats->total_processed : 0.0f);
        
        last_total = stats->total_processed;
        last_hid_total = hid_total;
        last_time = current_time;
    }
}
#else
void PrintUSBStatus(cbuf_handle_t cBufHandle)
{
    (void)cBufHandle;
    printf("USB statistics disabled\n");
}

void MonitorUSBPerformance(cbuf_handle_t cBufHandle)
{
    (void)cBufHandle;
}
#endif 