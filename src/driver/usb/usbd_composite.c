/**************************************************************************************************
 * Copyright (c) keyst.one 2020-2025. All rights reserved.
 * Description: usbd composite
 * Author: leon sun
 * Create: 2023-6-16
 ************************************************************************************************/

#include "usbd_composite.h"
#include "stdio.h"
#include "usb_core.h"
#include "usbd_msc_core.h"
#include "usbd_cdc_core.h"
#include "usbd_desc.h"
#include "log_print.h"


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
    0x00,                              /*iConfiguration: Index of string descriptor describing the configuration*/
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
    USBD_MSC_cb.Init(pdev, cfgidx);
    USBD_CDC_cb.Init(pdev, cfgidx);
    return USBD_OK;
}


static uint8_t CompositeDeInit(void *pdev, uint8_t cfgidx)
{
    USBD_MSC_cb.DeInit(pdev, cfgidx);
    USBD_CDC_cb.DeInit(pdev, cfgidx);
    return USBD_OK;
}


static uint8_t CompositeSetup(void *pdev, USB_SETUP_REQ *req)
{
    uint8_t index = LOBYTE(req->wIndex);

    if (index == 0) {
        return USBD_MSC_cb.Setup(pdev, req);
    } else {
        return USBD_CDC_cb.Setup(pdev, req);
    }
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


static uint8_t *GetCompositeConfigDescriptor(uint8_t speed, uint16_t *length)
{
    uint16_t descriptorSize = 0;
    uint8_t *descriptor;
    uint8_t interfaceIndex = 0;

    g_interfaceCount = 0;
    *length = 9;

    //MSC
    descriptor = USBD_MSC_cb.GetConfigDescriptor(speed, &descriptorSize);
    descriptorSize -= 9;
    descriptor[9 + 2] = interfaceIndex;
    interfaceIndex++;
    memcpy(CompositeConfigDescriptor + *length, descriptor + 9, descriptorSize);
    *length += descriptorSize;
    g_interfaceCount++;

    //CDC
    descriptor = USBD_CDC_cb.GetConfigDescriptor(speed, &descriptorSize);
    descriptorSize -= 9;
    descriptor[9 + 2] = interfaceIndex;
    memcpy(CompositeConfigDescriptor + *length, descriptor + 9, descriptorSize);
    *length += descriptorSize;
    g_interfaceCount += 1;

    CompositeConfigDescriptor[2] = LOBYTE(*length);
    CompositeConfigDescriptor[3] = HIBYTE(*length);
    CompositeConfigDescriptor[4] = g_interfaceCount;
    //printf("length=%d\r\n", *length);
    //PrintArray("Descriptor", CompositeConfigDescriptor, *length);
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



