#include "usbd_composite.h"
#include "stdio.h"
#include "string.h"
#include "usb_core.h"
#include "usbd_msc_core.h"
#include "usbd_cdc_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "log_print.h"
#include "usb_task.h"

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
static uint8_t AppendClassDescriptor(uint8_t *outDesc, uint16_t *length, uint8_t *descriptor, uint16_t descriptorSize, uint8_t interfaceIndex);
static uint8_t CompositeSelectClass(USB_SETUP_REQ *req, uint8_t *isMsc);

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

static uint8_t g_interfaceCount = 0;
static uint8_t g_mscInterfaceNo = 0xFF;
static uint8_t g_cdcInterfaceNo = 0xFF;

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

static uint8_t CompositeInit(void *pdev, uint8_t cfgidx)
{
#ifdef USBD_ENABLE_MSC
    USBD_MSC_cb.Init(pdev, cfgidx);
#endif
    USBD_CDC_cb.Init(pdev, cfgidx);
    return USBD_OK;
}

static uint8_t CompositeDeInit(void *pdev, uint8_t cfgidx)
{
#ifdef USBD_ENABLE_MSC
    USBD_MSC_cb.DeInit(pdev, cfgidx);
#endif
    USBD_CDC_cb.DeInit(pdev, cfgidx);
    return USBD_OK;
}

static uint8_t CompositeSetup(void *pdev, USB_SETUP_REQ *req)
{
    uint8_t isMsc = 0;
    if (CompositeSelectClass(req, &isMsc) != USBD_OK) {
        USBD_CtlError(pdev, req);
        return USBD_FAIL;
    }

#ifdef USBD_ENABLE_MSC
    if (isMsc != 0U) {
        return USBD_MSC_cb.Setup(pdev, req);
    }
#endif
    return USBD_CDC_cb.Setup(pdev, req);
}

static uint8_t CompositeEP0_TxSent(void *pdev)
{
    return USBD_OK;
}

static uint8_t CompositeEP0_RxReady(void *pdev)
{
    return USBD_CDC_cb.EP0_RxReady(pdev);
}

static uint8_t CompositeDataIn(void *pdev, uint8_t epnum)
{
    epnum = epnum & 0x0F;
    if (epnum == MSC_EP_NUM) {
        return USBD_MSC_cb.DataIn(pdev, epnum);
    } else {
        return USBD_CDC_cb.DataIn(pdev, epnum);
    }
}

static uint8_t CompositeDataOut(void *pdev, uint8_t epnum)
{
    epnum = epnum & 0x0F;
    if (epnum == MSC_EP_NUM) {
        return USBD_MSC_cb.DataOut(pdev, epnum);
    } else {
        return USBD_CDC_cb.DataOut(pdev, epnum);
    }
}

static uint8_t CompositeSOF(void* pdev)
{
    return USBD_CDC_cb.SOF(pdev);
}

static uint8_t AppendClassDescriptor(uint8_t *outDesc, uint16_t *length, uint8_t *descriptor, uint16_t descriptorSize, uint8_t interfaceIndex)
{
    if (outDesc == NULL || length == NULL || descriptor == NULL || descriptorSize < 9U) {
        return 0U;
    }

    descriptorSize -= 9U;
    if ((uint32_t)(*length) + descriptorSize > USB_COMPOSITE_CONFIG_DESC_MAX_SIZE) {
        return 0U;
    }

    descriptor[9 + 2] = interfaceIndex;
    memcpy(outDesc + *length, descriptor + 9U, descriptorSize);
    *length += descriptorSize;
    return 1U;
}

static uint8_t *GetCompositeConfigDescriptor(uint8_t speed, uint16_t *length)
{
    uint16_t descriptorSize = 0;
    uint8_t *descriptor;
    uint8_t interfaceIndex = 0;
    uint8_t appendOk = 0;

    g_interfaceCount = 0;
    g_mscInterfaceNo = 0xFF;
    g_cdcInterfaceNo = 0xFF;
    *length = 9;

#ifdef USBD_ENABLE_MSC
    //MSC
    descriptor = USBD_MSC_cb.GetConfigDescriptor(speed, &descriptorSize);
    appendOk = AppendClassDescriptor(CompositeConfigDescriptor, length, descriptor, descriptorSize, interfaceIndex);
    if (!appendOk) {
        *length = 9;
        return CompositeConfigDescriptor;
    }
    g_mscInterfaceNo = interfaceIndex;
    interfaceIndex++;
    g_interfaceCount++;
#endif

    //CDC
    descriptor = USBD_CDC_cb.GetConfigDescriptor(speed, &descriptorSize);
    appendOk = AppendClassDescriptor(CompositeConfigDescriptor, length, descriptor, descriptorSize, interfaceIndex);
    if (!appendOk) {
        *length = 9;
        return CompositeConfigDescriptor;
    }
    g_cdcInterfaceNo = interfaceIndex;
    g_interfaceCount++;

    CompositeConfigDescriptor[2] = (uint8_t)(*length & 0xFFU);
    CompositeConfigDescriptor[3] = (uint8_t)((*length >> 8) & 0xFFU);
    CompositeConfigDescriptor[4] = g_interfaceCount;
    //printf("length=%d\r\n", *length);
    //PrintArray("Descriptor", CompositeConfigDescriptor, *length);
    return CompositeConfigDescriptor;
}

static uint8_t CompositeSelectClass(USB_SETUP_REQ *req, uint8_t *isMsc)
{
    uint8_t recipient;
    uint8_t index;
    uint8_t epNum;

    if (req == NULL || isMsc == NULL) {
        return USBD_FAIL;
    }

    *isMsc = 0U;
    recipient = req->bmRequest & USB_REQ_RECIPIENT_MASK;
    index = LOBYTE(req->wIndex);

    switch (recipient) {
    case USB_REQ_RECIPIENT_INTERFACE:
#ifdef USBD_ENABLE_MSC
        if (index == g_mscInterfaceNo) {
            *isMsc = 1U;
            return USBD_OK;
        }
#endif
        if (index == g_cdcInterfaceNo) {
            return USBD_OK;
        }
        return USBD_FAIL;

    case USB_REQ_RECIPIENT_ENDPOINT:
        epNum = index & 0x7FU;
#ifdef USBD_ENABLE_MSC
        if ((epNum == (MSC_IN_EP & 0x7FU)) || (epNum == (MSC_OUT_EP & 0x7FU))) {
            *isMsc = 1U;
            return USBD_OK;
        }
#endif
        if ((epNum == (CDC_IN_EP & 0x7FU)) || (epNum == (CDC_OUT_EP & 0x7FU))) {
            return USBD_OK;
        }
        return USBD_FAIL;

    default:
        return USBD_FAIL;
    }
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
