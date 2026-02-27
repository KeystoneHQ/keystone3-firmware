/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_desc.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : header file for the usbd_desc.c file
 *****************************************************************************/

#ifndef __USB_DESC_H
#define __USB_DESC_H

/* Includes ------------------------------------------------------------------*/
#include "usb_core.h"
#include "usbd_def.h"

/** @defgroup USB_DESC_Exported_Defines
 * @{
 */
#define USB_DEVICE_DESCRIPTOR_TYPE        0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE 0x02
#define USB_STRING_DESCRIPTOR_TYPE        0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE     0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE      0x05
#define USB_SIZ_DEVICE_DESC               18
#define USB_SIZ_STRING_LANGID             4

#define  USB_REQ_MS_VENDOR_CODE                         0xA0U

/**
 * @}
 */

/** @defgroup USBD_DESC_Exported_Variables
 * @{
 */
extern uint8_t USBD_DeviceDesc[USB_SIZ_DEVICE_DESC];
extern uint8_t USBD_StrDesc[USB_MAX_STR_DESC_SIZ];
extern uint8_t USBD_OtherSpeedCfgDesc[USB_LEN_CFG_DESC];
extern uint8_t USBD_QualifierDesc[USB_LEN_DEV_QUALIFIER_DESC];
extern uint8_t USBD_LangIDDesc[USB_SIZ_STRING_LANGID];

extern USBD_DEVICE USR_desc;
/**
 * @}
 */

/** @defgroup USBD_DESC_Exported_FunctionsPrototype
 * @{
 */
uint8_t* USBD_USR_DeviceDescriptor(uint8_t speed, uint16_t* length);
uint8_t* USBD_USR_LangIDStrDescriptor(uint8_t speed, uint16_t* length);
uint8_t* USBD_USR_ManufacturerStrDescriptor(uint8_t speed, uint16_t* length);
uint8_t* USBD_USR_ProductStrDescriptor(uint8_t speed, uint16_t* length);
uint8_t* USBD_USR_SerialStrDescriptor(uint8_t speed, uint16_t* length);
uint8_t* USBD_USR_ConfigStrDescriptor(uint8_t speed, uint16_t* length);
uint8_t* USBD_USR_InterfaceStrDescriptor(uint8_t speed, uint16_t* length);
uint8_t* USBD_USR_QualifierDescriptor(uint8_t speed, uint16_t* length);

uint8_t* USBD_WinUSBOSStrDescriptor(uint16_t *length);
uint8_t *USBD_WinUSBOSFeatureDescriptor(uint16_t *length);
uint8_t *USBD_WinUSBOSPropertyDescriptor(uint16_t *length);

/**
 * @}
 */

#endif /* __USBD_DESC_H */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
