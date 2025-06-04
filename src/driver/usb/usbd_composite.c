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

void hid_StateChangedEvent(const uint8_t* reportBuffer)
{
    printf("hid_StateChangedEvent\n");
    uint8_t  reports[HID_OUT_PACKET_SIZE * 8];
    uint32_t hidOutputSize = USBD_HID_GetOutputReport(reports, sizeof(reports));
    if (hidOutputSize)
    {
        printf("hidOutputSize: %d\r\n", hidOutputSize);
        printf("reports: %s\r\n", reports);
        if (USBD_HID_PutInputReport(reports, hidOutputSize))
        {
            printf("Input reports full!\r\n");
        }
        else
        {
            printf("Put input reports %d\r\n", hidOutputSize);
        }
    }
}

void CompositeCbInit(void)
{
    g_interfaceCount = 0;
#ifdef USBD_ENABLE_MSC
    g_interfaceMap[g_interfaceCount] = &USBD_MSC_cb;
    g_endpointInMap[MSC_IN_EP & 0X0F] = &USBD_MSC_cb;
    g_endpointOutMap[MSC_OUT_EP] = &USBD_MSC_cb;
    g_interfaceCount++;
#endif
    
    g_interfaceMap[g_interfaceCount] = &USBD_CDC_cb;
    g_endpointInMap[CDC_IN_EP & 0X0F] = &USBD_CDC_cb;
    g_endpointOutMap[CDC_OUT_EP] = &USBD_CDC_cb;
    // g_endpointInMap[CDC_IN_EP & 0x0F] = &USBD_CDC_cb;
    g_interfaceCount += 2;

    g_interfaceMap[g_interfaceCount] = &USBD_HID_cb;
    g_endpointInMap[HID_IN_EP & 0X0F] = &USBD_HID_cb;
    USBD_HID_OutputReport_Event = hid_StateChangedEvent;
    g_interfaceCount++;
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

    USBD_Class_cb_TypeDef* callback;
    if (index >= g_interfaceCount || 0 == (callback = g_interfaceMap[index]) || callback->Setup == 0)
        return USBD_FAIL;

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
    USBD_Class_cb_TypeDef *callback;
    if (epnum >= USB_OTG_MAX_EP_COUNT || (callback = g_endpointInMap[epnum]) == 0 || callback->DataIn == 0)
        return USBD_FAIL;

    return callback->DataIn(pdev, epnum);
}

static uint8_t CompositeDataOut(void *pdev, uint8_t epnum)
{
    epnum = epnum & 0x0F;
    USBD_Class_cb_TypeDef *callback;
    if (epnum >= USB_OTG_MAX_EP_COUNT || (callback = g_endpointOutMap[epnum]) == 0 || callback->DataOut == 0)
        return USBD_FAIL;

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

static __ALIGN_BEGIN uint8_t USBD_Composite_ConfigDesc[] __ALIGN_END = {
    /* Configuration Descriptor */
    0x09,                       // bLength: Configuration Descriptor size
    USB_DESC_TYPE_CONFIGURATION,// bDescriptorType: Configuration
    0,   // wTotalLength
    0,
    0x02,                       // bNumInterfaces: 2
    0x01,                       // bConfigurationValue
    0x00,                       // iConfiguration
    0xC0,                       // bmAttributes: self powered
    0x32,                       // MaxPower 100 mA

    /************ Interface 0: WinUSB ************/
    0x09,                       // bLength: Interface Descriptor size
    USB_DESC_TYPE_INTERFACE,    
    0x00,                       // bInterfaceNumber: 0
    0x00,                       // bAlternateSetting
    0x02,                       // bNumEndpoints
    0xFF,                       // bInterfaceClass: Vendor Specific
    0x00,                       // bInterfaceSubClass
    0x00,                       // bInterfaceProtocol
    0x00,                       // iInterface

    // Endpoint OUT
    0x07, USB_DESC_TYPE_ENDPOINT, CDC_OUT_EP, 0x02,
    LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
    HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
    0x00,

    // Endpoint IN
    0x07, USB_DESC_TYPE_ENDPOINT, CDC_IN_EP, 0x02,
    LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
    HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
    0x00,

    /************ Interface 1: HID ************/
    0x09, USB_DESC_TYPE_INTERFACE,
    0x01,                       // bInterfaceNumber: 1
    0x00,                       // bAlternateSetting
#ifdef HID_OUT_EP
    0x02,                       // bNumEndpoints
#else
    0x01,                       // bNumEndpoints
#endif
    HID_CLASS, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE,
    0x00,

    // HID Descriptor
    HID_DESCRIPTOR_SIZE, HID_DESCRIPTOR,
    LSB(HID_VERSION), MSB(HID_VERSION),
    0x00, 0x01, REPORT_DESCRIPTOR,
    LSB(HID_REPORT_DESC_SIZE), MSB(HID_REPORT_DESC_SIZE),

    // HID IN Endpoint
    0x07, USB_DESC_TYPE_ENDPOINT,
    HID_IN_EP, 0x03,
    LOBYTE(HID_IN_PACKET_SIZE),
    HIBYTE(HID_IN_PACKET_SIZE),
    HID_INTERVAL,

#ifdef HID_OUT_EP
    // HID OUT Endpoint
    0x07, USB_DESC_TYPE_ENDPOINT,
    HID_OUT_EP, 0x03,
    LOBYTE(HID_OUT_PACKET_SIZE),
    HIBYTE(HID_OUT_PACKET_SIZE),
    HID_INTERVAL,
#endif
};


static uint8_t *GetCompositeConfigDescriptor(uint8_t speed, uint16_t *length)
{
    uint16_t descriptorSize = 0;
    uint8_t *descriptor;
    uint8_t interfaceIndex = 0;

//     *length = 9;

// #ifdef USBD_ENABLE_MSC
//     //MSC
//     descriptor = USBD_MSC_cb.GetConfigDescriptor(speed, &descriptorSize);
//     descriptorSize -= 9;
//     descriptor[9 + 2] = interfaceIndex;
//     interfaceIndex++;
//     memcpy(CompositeConfigDescriptor + *length, descriptor + 9, descriptorSize);
//     *length += descriptorSize;
//     interfaceIndex++;
// #endif

//     //CDC
//     descriptor = USBD_CDC_cb.GetConfigDescriptor(speed, &descriptorSize);
//     descriptorSize -= 9;
//     descriptor[9 + 2] = interfaceIndex;
//     memcpy(CompositeConfigDescriptor + *length, descriptor + 9, descriptorSize);
//     *length += descriptorSize;
//     interfaceIndex++;

//     //HID
//     descriptor = USBD_HID_cb.GetConfigDescriptor(speed, &descriptorSize);
//     descriptorSize -= 9;
//     descriptor[9 + 2] = interfaceIndex;          // HID接口
//     interfaceIndex++;
//     memcpy(CompositeConfigDescriptor + *length, descriptor + 9, descriptorSize);
//     *length += descriptorSize;

    *length = sizeof(USBD_Composite_ConfigDesc);
    USBD_Composite_ConfigDesc[2] = LOBYTE(*length);
    USBD_Composite_ConfigDesc[3] = HIBYTE(*length);
    memcpy(CompositeConfigDescriptor, USBD_Composite_ConfigDesc, *length);
    
    // CompositeConfigDescriptor[2] = LOBYTE(*length);
    // CompositeConfigDescriptor[3] = HIBYTE(*length);
    // CompositeConfigDescriptor[4] = interfaceIndex;  // 总接口数应该是2
    printf("length: %d\n", *length);
    printf("InterfaceCount: %d\n", interfaceIndex);
    for (int i = 0; i < *length; i++)
    {
        printf("%02X", CompositeConfigDescriptor[i]);
    }
    printf("\n");
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
