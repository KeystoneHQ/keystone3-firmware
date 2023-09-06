/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbh_hid_keybd.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : This file contains all the prototypes for the usbh_hid_keybd.c
 *****************************************************************************/

/* Define to prevent recursive -----------------------------------------------*/
#ifndef __USBH_HID_KEYBD_H
#define __USBH_HID_KEYBD_H

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"
#include "usbh_hid_core.h"

/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_HID_CLASS
  * @{
  */

/** @defgroup USBH_HID_KEYBD
  * @brief This file is the Header file for USBH_HID_KEYBD.c
  * @{
  */


/** @defgroup USBH_HID_KEYBD_Exported_Types
  * @{
  */


/**
  * @}
  */

/** @defgroup USBH_HID_KEYBD_Exported_Defines
  * @{
  */
//#define QWERTY_KEYBOARD
#define AZERTY_KEYBOARD

#define  KBD_LEFT_CTRL                                  0x01
#define  KBD_LEFT_SHIFT                                 0x02
#define  KBD_LEFT_ALT                                   0x04
#define  KBD_LEFT_GUI                                   0x08
#define  KBD_RIGHT_CTRL                                 0x10
#define  KBD_RIGHT_SHIFT                                0x20
#define  KBD_RIGHT_ALT                                  0x40
#define  KBD_RIGHT_GUI                                  0x80

#define  KBR_MAX_NBR_PRESSED                            6

/**
  * @}
  */

/** @defgroup USBH_HID_KEYBD_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_HID_KEYBD_Exported_Variables
  * @{
  */

extern HID_cb_TypeDef HID_KEYBRD_cb;
/**
  * @}
  */

/** @defgroup USBH_HID_KEYBD_Exported_FunctionsPrototype
  * @{
  */
void  USR_KEYBRD_Init(void);
void  USR_KEYBRD_ProcessData(uint8_t pbuf);
/**
  * @}
  */

#endif /* __USBH_HID_KEYBD_H */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT Megahuntmicro *****END OF FILE****/

