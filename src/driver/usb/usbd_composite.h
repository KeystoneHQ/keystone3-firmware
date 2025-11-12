#ifndef _USBD_COMPOSITE_H
#define _USBD_COMPOSITE_H

#include "usb_core.h"
#include <stdint.h>
#include <stdbool.h>
#include "circular_buffer.h"

extern USBD_Class_cb_TypeDef USBCompositeCb;

#define USB_RX_BUFFER_SIZE 64
#define USB_RX_BUFFER_MAX_SIZE (USB_RX_BUFFER_SIZE + 1)

typedef enum {
    USB_TRANSPORT_CDC = 0,
    USB_TRANSPORT_HID = 1,
} USB_TransportType_t;

typedef void (*USB_SendCallback_t)(const uint8_t *data, uint32_t len);

typedef struct {
    USB_TransportType_t type;
    USB_SendCallback_t callback;
    const char* name;
} USB_TransportConfig_t;

#ifdef USB_STATS_ENABLED
typedef struct {
    uint32_t total_processed;
    uint32_t cdc_processed;
    uint32_t hid_processed;
    uint32_t error_count;
    uint32_t buffer_overflow_count;
} USB_Stats_t;
#endif

void CompositeCbInit(void);
void GetHIDEventStats(uint32_t* total, uint32_t* success, uint32_t* failed, uint32_t* overflow);

const USB_TransportConfig_t* GetUSBTransportConfig(USB_TransportType_t type);
void ProcessUSBData(uint32_t dataLen, USB_TransportType_t transportType, cbuf_handle_t cBufHandle);

#ifdef USB_STATS_ENABLED
USB_Stats_t* GetUSBStats(void);
void ResetUSBStats(void);
void PrintUSBStatus(cbuf_handle_t cBufHandle);
void MonitorUSBPerformance(cbuf_handle_t cBufHandle);
#else
void PrintUSBStatus(cbuf_handle_t cBufHandle);
void MonitorUSBPerformance(cbuf_handle_t cBufHandle);
#endif

#ifdef DEBUG_USB_TRANSPORT
    #define USB_DEBUG_PRINT(fmt, ...) printf("[USB_DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
    #define USB_DEBUG_PRINT(fmt, ...) do {} while(0)
#endif

#endif
