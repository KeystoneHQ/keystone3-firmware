#include "usbd_composite.h"
#include "stdio.h"
#include "usb_core.h"
#include "usbd_msc_core.h"
#include "usbd_cdc_core.h"
#include "usbd_hid_core.h"
#include "usbd_desc.h"
#include "log_print.h"
#include "usb_task.h"
#include "usbd_def.h"
#include "user_msg.h"
#include "user_delay.h"

#define USB_COMPOSITE_CONFIG_DESC_MAX_SIZE 192

static uint8_t CompositeInit(void *pdev, uint8_t cfgidx);
static uint8_t CompositeDeInit(void *pdev, uint8_t cfgidx);
static uint8_t CompositeSetup(void *pdev, USB_SETUP_REQ *req);
static uint8_t CompositeEP0_TxSent(void *pdev);
static uint8_t CompositeEP0_RxReady(void *pdev);
static uint8_t CompositeDataIn(void *pdev, uint8_t epnum);
static uint8_t CompositeDataOut(void *pdev, uint8_t epnum);
static uint8_t CompositeSOF(void *pdev);
static uint8_t *GetCompositeConfigDescriptor(uint8_t speed, uint16_t *length);
static uint8_t *USBD_Composite_GetDeviceQualifierDescriptor(uint16_t *length);
static uint8_t *USBD_Composite_WinUSBOSStrDescriptor(uint16_t *length);

__ALIGN_BEGIN static uint8_t CompositeConfigDescriptor[USB_COMPOSITE_CONFIG_DESC_MAX_SIZE] __ALIGN_END = {
    0x09,                              /* bLength: Configuration Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
    0x00,                              /* wTotalLength:no of returned bytes */
    0x00,                              /* */
    0x00,                              /*bNumInterfaces: X interface*/
    0x01,                              /*bConfigurationValue: Configuration value*/
    USBD_IDX_CONFIG_STR,                              /*iConfiguration: Index of string descriptor describing the configuration*/
    0xE0,                              /*bmAttributes: bus powered and Support Remote Wake-up */
    0xC8,                              /*MaxPower 400 mA: this current is used for detecting Vbus*/

    //0x09,   /* bLength: Configuration Descriptor size */
    //USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
    //0x00,                /* wTotalLength:no of returned bytes */
    //0x00,
    //0x02,   /* bNumInterfaces: 1 interface for WINUSB */
    //0x01,   /* bConfigurationValue: Configuration value */
    //USBD_IDX_PRODUCT_STR,   /* iConfiguration: Index of string descriptor describing the configuration */
    //0xC0,   /* bmAttributes: self powered */
    //0x32,   /* MaxPower 50*2 mA */
};

USBD_Class_cb_TypeDef USBCompositeCb = {
    CompositeInit,
    CompositeDeInit,
    /* Control Endpoints*/
    CompositeSetup,
    CompositeEP0_TxSent,
    CompositeEP0_RxReady,
    /* Class Specific Endpoints*/
    CompositeDataIn,
    CompositeDataOut,
    CompositeSOF,
    NULL,
    NULL,

    GetCompositeConfigDescriptor,
    USBD_Composite_GetDeviceQualifierDescriptor,
    USBD_Composite_WinUSBOSStrDescriptor,
};

static USBD_Class_cb_TypeDef* g_interfaceMap[USBD_ITF_MAX_NUM];
static USBD_Class_cb_TypeDef* g_endpointInMap[USB_OTG_MAX_EP_COUNT];
static USBD_Class_cb_TypeDef* g_endpointOutMap[USB_OTG_MAX_EP_COUNT];
static uint8_t g_interfaceCount = 0;
#define WEBUSB_ENABLE 1
#define HID_ENABLE 1

static struct {
    uint32_t total_events;
    uint32_t successful_events;
    uint32_t failed_events;
    uint32_t buffer_overflow_events;
} g_hidEventStats = {0};

void hid_StateChangedEvent(const uint8_t* reportBuffer)
{
    g_hidEventStats.total_events++;
    
    if (reportBuffer == NULL) {
        printf("HID event: NULL report buffer\n");
        g_hidEventStats.failed_events++;
        return;
    }
    
    uint8_t reports[HID_OUT_PACKET_SIZE * 4]; // 减少缓冲区大小
    uint32_t hidOutputSize = USBD_HID_GetOutputReport(reports, sizeof(reports));
    
    if (hidOutputSize == 0) {
        return;
    }
    
    if (hidOutputSize > sizeof(reports)) {
        printf("HID buffer overflow: required %u, available %u\n", 
               hidOutputSize, sizeof(reports));
        g_hidEventStats.buffer_overflow_events++;
        return;
    }
    
    PushDataToField(reports, hidOutputSize);
    
    if (PubValueMsg(SPRING_MSG_HID_GET, hidOutputSize) == 0) {
        g_hidEventStats.successful_events++;
    } else {
        printf("Failed to publish HID message\n");
        g_hidEventStats.failed_events++;
    }
}

void GetHIDEventStats(uint32_t* total, uint32_t* success, uint32_t* failed, uint32_t* overflow)
{
    if (total) *total = g_hidEventStats.total_events;
    if (success) *success = g_hidEventStats.successful_events;
    if (failed) *failed = g_hidEventStats.failed_events;
    if (overflow) *overflow = g_hidEventStats.buffer_overflow_events;
}

typedef struct {
    USBD_Class_cb_TypeDef* callback;
    uint8_t in_ep;
    uint8_t out_ep;
    bool enabled;
} USB_Class_Config_t;

static USB_Class_Config_t g_usbClasses[] = {
#ifdef USBD_ENABLE_MSC
    {&USBD_MSC_cb, MSC_IN_EP & 0X0F, MSC_OUT_EP, true},
#endif
#if WEBUSB_ENABLE
    {&USBD_CDC_cb, CDC_IN_EP & 0X0F, CDC_OUT_EP, true},
#endif
#if HID_ENABLE
    {&USBD_HID_cb, HID_IN_EP & 0X0F, 0, true},
#endif
};

#define USB_CLASS_CONFIG_COUNT (sizeof(g_usbClasses)/sizeof(g_usbClasses[0]))
#define USB_INTERFACE_MAX_COUNT 8
#define USB_ENDPOINT_MAX_COUNT 16

static bool ValidateUSBConfiguration(void)
{
    uint8_t usedEndpoints = 0;
    uint8_t endpointMap[USB_OTG_MAX_EP_COUNT] = {0};
    
    for (int i = 0; i < USB_CLASS_CONFIG_COUNT; i++) {
        if (g_usbClasses[i].enabled && g_usbClasses[i].callback) {
            if (g_usbClasses[i].in_ep > 0) {
                if (endpointMap[g_usbClasses[i].in_ep]) {
                    printf("USB endpoint conflict: IN endpoint %d used by multiple classes\n", 
                           g_usbClasses[i].in_ep);
                    return false;
                }
                endpointMap[g_usbClasses[i].in_ep] = 1;
                usedEndpoints++;
            }
            
            if (g_usbClasses[i].out_ep > 0) {
                if (endpointMap[g_usbClasses[i].out_ep]) {
                    printf("USB endpoint conflict: OUT endpoint %d used by multiple classes\n", 
                           g_usbClasses[i].out_ep);
                    return false;
                }
                endpointMap[g_usbClasses[i].out_ep] = 1;
                usedEndpoints++;
            }
        }
    }
    
    printf("USB configuration validated: %d classes, %d endpoints\n", 
           g_interfaceCount, usedEndpoints);
    return true;
}

void CompositeCbInit(void)
{
    g_interfaceCount = 0;
    
    memset(g_interfaceMap, 0, sizeof(g_interfaceMap));
    memset(g_endpointInMap, 0, sizeof(g_endpointInMap));
    memset(g_endpointOutMap, 0, sizeof(g_endpointOutMap));
    
    for (int i = 0; i < USB_CLASS_CONFIG_COUNT; i++) {
        if (g_usbClasses[i].enabled && g_usbClasses[i].callback) {
            g_interfaceMap[g_interfaceCount] = g_usbClasses[i].callback;
            g_endpointInMap[g_usbClasses[i].in_ep] = g_usbClasses[i].callback;
            if (g_usbClasses[i].out_ep > 0) {
                g_endpointOutMap[g_usbClasses[i].out_ep] = g_usbClasses[i].callback;
            }
            g_interfaceCount++;
        }
    }
    
    if (!ValidateUSBConfiguration()) {
        printf("USB configuration validation failed!\n");
    }
    
#if HID_ENABLE
    USBD_HID_OutputReport_Event = hid_StateChangedEvent;
#endif
}

static uint8_t CompositeInit(void *pdev, uint8_t cfgidx)
{
    for (int i = 0; i < g_interfaceCount; i++)
    {
        USBD_Class_cb_TypeDef *callback = g_interfaceMap[i];
        if (callback && callback->Init)
            callback->Init(pdev, cfgidx);
    }
    return USBD_OK;    
}

static uint8_t CompositeDeInit(void *pdev, uint8_t cfgidx)
{
    for (int i = 0; i < g_interfaceCount; i++)
    {
        USBD_Class_cb_TypeDef *callback = g_interfaceMap[i];
        if (callback && callback->DeInit)
            callback->DeInit(pdev, cfgidx);
    }
    return USBD_OK;
}

static uint8_t CompositeSetup(void *pdev, USB_SETUP_REQ *req)
{
    uint8_t index = LOBYTE(req->wIndex);

    if (index >= g_interfaceCount) {
        return USBD_FAIL;
    }
    
    USBD_Class_cb_TypeDef* callback = g_interfaceMap[index];
    if (callback == NULL || callback->Setup == NULL) {
        return USBD_FAIL;
    }

    return callback->Setup(pdev, req);
}

static uint8_t CompositeEP0_TxSent(void *pdev)
{
    return USBD_OK;
}

static uint8_t CompositeEP0_RxReady(void *pdev)
{
    for (int i = 0; i < g_interfaceCount; i++)
    {
        USBD_Class_cb_TypeDef* callback = g_interfaceMap[i];
        if (callback && callback->EP0_RxReady)
            callback->EP0_RxReady(pdev);
    }
    return USBD_OK;
}

static uint8_t CompositeDataIn(void *pdev, uint8_t epnum)
{
    epnum = epnum & 0x0F;
    
    if (epnum >= USB_OTG_MAX_EP_COUNT) {
        return USBD_FAIL;
    }
    
    USBD_Class_cb_TypeDef *callback = g_endpointInMap[epnum];
    if (callback == NULL || callback->DataIn == NULL) {
        return USBD_FAIL;
    }

    return callback->DataIn(pdev, epnum);
}

static uint8_t CompositeDataOut(void *pdev, uint8_t epnum)
{
    epnum = epnum & 0x0F;
    
    if (epnum >= USB_OTG_MAX_EP_COUNT) {
        return USBD_FAIL;
    }
    
    USBD_Class_cb_TypeDef *callback = g_endpointOutMap[epnum];
    if (callback == NULL || callback->DataOut == NULL) {
        return USBD_FAIL;
    }

    return callback->DataOut(pdev, epnum);
}

static uint8_t CompositeSOF(void* pdev)
{
    for (int i = 0; i < g_interfaceCount; i++)
    {
        USBD_Class_cb_TypeDef *callback = g_interfaceMap[i];
        if (callback && callback->SOF)
            callback->SOF(pdev);
    }
    return USBD_OK;
}

static uint8_t* GetCompositeConfigDescriptor(uint8_t speed, uint16_t* length)
{
    *length = 9;

    uint16_t descriptorSize = 0;
    uint8_t* descriptor;
    uint8_t  interfaceIndex = 0;

#ifdef USBD_ENABLE_MSC
    descriptor = USBD_MSC_cb.GetConfigDescriptor(speed, &descriptorSize);
    descriptorSize -= 9;
    assert_param(*length + descriptorSize <= USB_COMPOSITE_CONFIG_DESC_MAX_SIZE);
    descriptor[9 + 2] = interfaceIndex; // Printer Interface
    interfaceIndex++;
    memcpy(CompositeConfigDescriptor + *length, descriptor + 9, descriptorSize);
    *length += descriptorSize;
#endif // CONFIG_USB_DEVICE_MSC

#if WEBUSB_ENABLE
    descriptor = USBD_CDC_cb.GetConfigDescriptor(speed, &descriptorSize);
    descriptorSize -= 9;
    assert_param(*length + descriptorSize <= USB_COMPOSITE_CONFIG_DESC_MAX_SIZE);
    descriptor[9 + 2] = interfaceIndex; // Interface number
    interfaceIndex++;
    memcpy(CompositeConfigDescriptor + *length, descriptor + 9, descriptorSize);
    *length += descriptorSize;
    // WebUSB descriptor doesn't need to modify other interface numbers since it only has one interface
#endif

#if HID_ENABLE
    descriptor = USBD_HID_cb.GetConfigDescriptor(speed, &descriptorSize);
    descriptorSize -= 9;
    assert_param(*length + descriptorSize <= USB_COMPOSITE_CONFIG_DESC_MAX_SIZE);
    descriptor[9 + 2] = interfaceIndex; // HID Interface
    interfaceIndex++;
    memcpy(CompositeConfigDescriptor + *length, descriptor + 9, descriptorSize);
    *length += descriptorSize;
#endif // CONFIG_USB_DEVICE_HID_DEFAULT

    CompositeConfigDescriptor[2] = LOBYTE(*length);
    CompositeConfigDescriptor[3] = HIBYTE(*length);
    CompositeConfigDescriptor[4] = interfaceIndex;

    return CompositeConfigDescriptor;
}

__ALIGN_BEGIN static uint8_t USBD_Composite_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END = {
    USB_LEN_DEV_QUALIFIER_DESC,
    USB_DESC_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x01,
    0x00,
};

static uint8_t *USBD_Composite_GetDeviceQualifierDescriptor(uint16_t *length)
{
    *length = sizeof(USBD_Composite_DeviceQualifierDesc);
    return USBD_Composite_DeviceQualifierDesc;
}

static uint8_t *USBD_Composite_WinUSBOSStrDescriptor(uint16_t *length)
{
    return USBD_CDC_cb.GetWinUSBOSDescriptor(length);
}
