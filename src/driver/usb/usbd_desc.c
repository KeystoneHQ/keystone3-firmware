/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_desc.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : This file provides the USBD descriptors and string formating method.
 *****************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usbd_desc.h"
#include "usb_regs.h"
#include "usbd_conf.h"
#include "usbd_core.h"
#include "usbd_req.h"

/** @defgroup USBD_DESC_Private_Defines
 * @{
 */
#define USBD_VID 0x1209
#define USBD_PID 0x3001

#define USBD_LANGID_STRING          0x0409
#define USBD_MANUFACTURER_STRING    "Keystone"

#if CONFIG_USB_DEVICE_MSC
#undef USBD_PRODUCT_STRING
#define USBD_PRODUCT_STRING         "Keystone 3 Pro"
#endif // CONFIG_USB_DEVICE_MSC

#define USBD_CONFIGURATION_STRING   "USB Config"
#define USBD_INTERFACE_STRING       "USB Interface"

/**
 * @}
 */

/** @defgroup USBD_DESC_Private_Variables
 * @{
 */
USBD_DEVICE USR_desc = {
    USBD_USR_DeviceDescriptor,          /* DeviceDescriptor */
    USBD_USR_LangIDStrDescriptor,       /* LangID:0409 */
    USBD_USR_ManufacturerStrDescriptor, /* Manufacturer:Megahunt */
    USBD_USR_ProductStrDescriptor,      /* Product:Megahunt Composite Device */
    USBD_USR_SerialStrDescriptor,       /* SerialNumber */
    USBD_USR_ConfigStrDescriptor,       /* Config */
    USBD_USR_InterfaceStrDescriptor,    /* Interface */
    USBD_USR_QualifierDescriptor,       /* Qualifier */

    USBD_WinUSBOSStrDescriptor,
    USBD_WinUSBOSFeatureDescriptor,
    USBD_WinUSBOSPropertyDescriptor,
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN uint8_t USBD_DeviceDesc[USB_SIZ_DEVICE_DESC] __ALIGN_END = {
    0x12,                       /*bLength */
    USB_DEVICE_DESCRIPTOR_TYPE, /*bDescriptorType*/
    0x00,                       /*bcdUSB */
    0x02,
    0x00,                 /*bDeviceClass*/
    0x00,                 /*bDeviceSubClass*/
    0x00,                 /*bDeviceProtocol*/
    USB_OTG_MAX_EP0_SIZE, /*bMaxPacketSize*/
    LOBYTE(USBD_VID),     /*idVendor*/
    HIBYTE(USBD_VID),     /*idVendor*/
    LOBYTE(USBD_PID),     /*idVendor*/
    HIBYTE(USBD_PID),     /*idVendor*/
    0x00,                 /*bcdDevice rel. 2.00*/
    0x02,
    USBD_IDX_MFC_STR,     /*Index of manufacturer  string*/
    USBD_IDX_PRODUCT_STR, /*Index of product string*/
    USBD_IDX_SERIAL_STR,  /*Index of serial number string*/
    USBD_CFG_MAX_NUM,     /*bNumConfigurations*/
};                        /* USB_DeviceDescriptor */

/* USB Standard Device Descriptor */
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_SIZ_STRING_LANGID] __ALIGN_END = {
    USB_SIZ_STRING_LANGID,
    USB_DESC_TYPE_STRING,
    LOBYTE(USBD_LANGID_STRING),
    HIBYTE(USBD_LANGID_STRING),
};
/**
 * @}
 */

uint8_t USBD_QualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] = {
    USB_LEN_DEV_QUALIFIER_DESC,     /* bLength */
    USB_DESC_TYPE_DEVICE_QUALIFIER, /* bDescriptorType */
    0x00,                           /* bcdUSB (LSB) */
    0x02,                           /* bcdUSB (MSB) */
    0x00,                           /* bDeviceClass */
    0x00,                           /* bDeviceSubClass */
    0x00,                           /* bDeviceprotocol */
    USB_OTG_MAX_EP0_SIZE,           /* bMaxPacketSize0 */
    0x00,                           /* bNumConfigurations */
    0x00,                           /* bReserved */
};

/** @defgroup USBD_DESC_Private_Functions
 * @{
 */
/**
 * @brief  USBD_USR_DeviceDescriptor
 *         return the device descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_USR_DeviceDescriptor(uint8_t speed, uint16_t* length)
{
    *length = sizeof(USBD_DeviceDesc);
    return USBD_DeviceDesc;
}

uint8_t* USBD_USR_QualifierDescriptor(uint8_t speed, uint16_t* length)
{
    USBD_QualifierDesc[4] = USBD_DeviceDesc[4];
    USBD_QualifierDesc[5] = USBD_DeviceDesc[5];
    USBD_QualifierDesc[6] = USBD_DeviceDesc[6];

    *length = sizeof(USBD_QualifierDesc);
    return USBD_QualifierDesc;
}

/**
 * @brief  USBD_USR_LangIDStrDescriptor
 *         return the LangID string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_USR_LangIDStrDescriptor(uint8_t speed, uint16_t* length)
{
    *length = sizeof(USBD_LangIDDesc);
    return USBD_LangIDDesc;
}

/**
 * @brief  USBD_USR_ProductStrDescriptor
 *         return the product string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_USR_ProductStrDescriptor(uint8_t speed, uint16_t* length)
{
    USBD_GetString((uint8_t*)USBD_PRODUCT_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

/**
 * @brief  USBD_USR_ManufacturerStrDescriptor
 *         return the manufacturer string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_USR_ManufacturerStrDescriptor(uint8_t speed, uint16_t* length)
{
    USBD_GetString((uint8_t*)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

/**
 * @brief  USBD_USR_SerialStrDescriptor
 *         return the serial number string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_USR_SerialStrDescriptor(uint8_t speed, uint16_t* length)
{
    // Dynamic USB Device SN format:
    //   [interface functions]-[ChipSN sum]
    uint8_t usbSN[32] = {0};

    uint8_t maxLength = sizeof(usbSN);

    uint8_t snIndex = 0;

#if CONFIG_USB_DEVICE_MSC
    usbSN[snIndex++] = 'M';
#endif // CONFIG_USB_DEVICE_MSC
    usbSN[snIndex++] = '-';

    uint8_t chipSN[SYSCTRL_CHIP_SN_LEN];
    SYSCTRL_GetChipSN(chipSN);

    uint8_t usnLength = MIN((maxLength - snIndex), 8);
    uint8_t shift = 0;
    for (int i = 0; i < sizeof(chipSN) * 2; i++) {
        int usnIndex = i % usnLength + snIndex;
        shift += usbSN[usnIndex] + chipSN[i % sizeof(chipSN)];
        usbSN[usnIndex] = shift % 36;
    }

    *length = snIndex + usnLength;
    for (int i = snIndex; i < *length; i++) {
        usbSN[i] += usbSN[i] < 0x0A ? '0' : '7';
    }

    USBD_GetString(usbSN, USBD_StrDesc, length);
    return USBD_StrDesc;
}

/**
 * @brief  USBD_USR_ConfigStrDescriptor
 *         return the configuration string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_USR_ConfigStrDescriptor(uint8_t speed, uint16_t* length)
{
    USBD_GetString((uint8_t*)USBD_CONFIGURATION_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

/**
 * @brief  USBD_USR_InterfaceStrDescriptor
 *         return the interface string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_USR_InterfaceStrDescriptor(uint8_t speed, uint16_t* length)
{
    USBD_GetString((uint8_t*)USBD_INTERFACE_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

const uint8_t USBD_OS_STRING[8] = {
    'M',
    'S',
    'F',
    'T',
    '1',
    '0',
    '0',
    USB_REQ_MS_VENDOR_CODE,
};

#define USB_LEN_OS_FEATURE_DESC 0x28
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */

__ALIGN_BEGIN uint8_t USBD_WINUSB_OSFeatureDesc[USB_LEN_OS_FEATURE_DESC] __ALIGN_END = {
    0x28, 0, 0, 0,                          // length
    0, 1,                                   // bcd version 1.0
    4, 0,                                   // windex: extended compat ID descritor
    1,                                      // no of function
    0, 0, 0, 0, 0, 0, 0,                    // reserve 7 bytes
// function
    1,                                      // interface no
    0,                                      // reserved
    'W', 'I', 'N', 'U', 'S', 'B', 0, 0,     // first ID
    0, 0, 0, 0, 0, 0, 0, 0,                 // second ID
    0, 0, 0, 0, 0, 0                        // reserved 6 bytes
};

#define USB_LEN_OS_PROPERTY_DESC 0x8E
__ALIGN_BEGIN uint8_t USBD_WINUSB_OSPropertyDesc[USB_LEN_OS_PROPERTY_DESC] __ALIGN_END = {
    0x8E, 0, 0, 0,  // length 246 byte
    0x00, 0x01,   // BCD version 1.0
    0x05, 0x00,   // Extended Property Descriptor Index(5)
    0x01, 0x00,   // number of section (1)
    0x84, 0x00, 0x00, 0x00,   // size of property section
    0x1, 0, 0, 0,   //; property data type (1)
    0x28, 0,        //; property name length (42)
    'D', 0,
    'e', 0,
    'v', 0,
    'i', 0,
    'c', 0,
    'e', 0,
    'I', 0,
    'n', 0,
    't', 0,
    'e', 0,
    'r', 0,
    'f', 0,
    'a', 0,
    'c', 0,
    'e', 0,
    'G', 0,
    'U', 0,
    'I', 0,
    'D', 0,
    0, 0,
    // Ledger:
    // CE809264-4B24-4E81-A8B2-57ED01D580E1
    // ce809264-4b24-4e81-a8b2-57ed01d580e1
    0x4E, 0, 0, 0,  // ; property data length
    '{', 0,
    'C', 0,
    'E', 0,
    '8', 0,
    '0', 0,
    '9', 0,
    '2', 0,
    '6', 0,
    '4', 0,
    '-', 0,
    '4', 0,
    'B', 0,
    '2', 0,
    '4', 0,
    '-', 0,
    '4', 0,
    'E', 0,
    '8', 0,
    '1', 0,
    '-', 0,
    'A', 0,
    '8', 0,
    'B', 0,
    '2', 0,
    '-', 0,   // CE809264-4B24-4E81-A8B2-57ED01D580E1
    '5', 0,
    '7', 0,
    'E', 0,
    'D', 0,
    '0', 0,
    '1', 0,
    'D', 0,
    '5', 0,
    '8', 0,
    '0', 0,
    'E', 0,
    '1', 0,
    '}', 0,
    0, 0,
};

//#include "stdio.h"

uint8_t *USBD_WinUSBOSStrDescriptor(uint16_t *length)
{
    //printf("USBD_WinUSBOSStrDescriptor!\r\n");
    USBD_GetString((uint8_t *)USBD_OS_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

uint8_t *USBD_WinUSBOSFeatureDescriptor(uint16_t *length)
{
    //printf("USBD_WinUSBOSFeatureDescriptor!\r\n");
    *length = USB_LEN_OS_FEATURE_DESC;
    return USBD_WINUSB_OSFeatureDesc;
}

uint8_t *USBD_WinUSBOSPropertyDescriptor(uint16_t *length)
{
    //printf("USBD_WinUSBOSPropertyDescriptor!\r\n");
    *length = USB_LEN_OS_PROPERTY_DESC;
    return USBD_WINUSB_OSPropertyDesc;
}

/**
 * @}
 */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
