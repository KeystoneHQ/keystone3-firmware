/**
 ******************************************************************************
 * @file    usbd_usr.c
 * @author  Megahuntmicro
 * @version V1.0.0
 * @date    21-October-2014
 * @brief   This file includes the user application layer
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

#include "mhscpu.h"
#include "usb_conf.h"
#include "usbd_ioreq.h"
#include "usbd_usr.h"

/** @defgroup USBD_USR_Private_Variables
 * @{
 */

USBD_Usr_cb_TypeDef USRD_cb = {
    USBD_USR_Init,
    USBD_USR_DeviceReset,
    USBD_USR_DeviceConfigured,
    USBD_USR_DeviceSuspended,
    USBD_USR_DeviceResumed,

    USBD_USR_DeviceConnected,
    USBD_USR_DeviceDisconnected,
};

/**
 * @}
 */

#define USER_INFORMATION1 "[Key]:RemoteWakeup"
#define USER_INFORMATION2 "[Joystick]:Mouse emulation"

/**
 * @brief  USBD_USR_Init
 *         Displays the message on LCD for host lib initialization
 * @param  None
 * @retval None
 */
void USBD_USR_Init(void)
{
    printf("> USB Composite Device initialized.\r\n");
}

/**
 * @brief  USBD_USR_DeviceReset
 *         Displays the message on LCD on device Reset Event
 * @param  speed : device speed
 * @retval None
 */
void USBD_USR_DeviceReset(uint8_t speed)
{
//    printf("> USB Composite Device reset.\r\n");
}

/**
 * @brief  USBD_USR_DeviceConfigured
 *         Displays the message on LCD on device configuration Event
 * @param  None
 * @retval Staus
 */
void USBD_USR_DeviceConfigured(void)
{
//    printf("> USB Composite Device started.\r\n");
}

/**
 * @brief  USBD_USR_DeviceConnected
 *         Displays the message on LCD on device connection Event
 * @param  None
 * @retval Staus
 */
void USBD_USR_DeviceConnected(void)
{
//    printf("> USB Device Connected.\r\n");
}

/**
 * @brief  USBD_USR_DeviceDisonnected
 *         Displays the message on LCD on device disconnection Event
 * @param  None
 * @retval Staus
 */
void USBD_USR_DeviceDisconnected(void)
{
//    printf("> USB Device Disconnected.\r\n");
}

/**
 * @brief  USBD_USR_DeviceSuspended
 *         Displays the message on LCD on device suspend Event
 * @param  None
 * @retval None
 */
void USBD_USR_DeviceSuspended(void)
{
//    printf("> USB Device in Suspend Mode.\r\n");
    /* Users can do their application actions here for the USB-Reset */
}

/**
 * @brief  USBD_USR_DeviceResumed
 *         Displays the message on LCD on device resume Event
 * @param  None
 * @retval None
 */
void USBD_USR_DeviceResumed(void)
{
//    printf("> USB Device in Idle Mode.\r\n");
    /* Users can do their application actions here for the USB-Reset */
}

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
