/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbh_hid_mouse.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : This file is the application layer for USB Host HID Mouse Handling.
 *****************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid_mouse.h"


/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_HID_CLASS
  * @{
  */

/** @defgroup USBH_HID_MOUSE
  * @brief    This file includes HID Layer Handlers for USB Host HID class.
  * @{
  */

/** @defgroup USBH_HID_MOUSE_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_HID_MOUSE_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_HID_MOUSE_Private_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_HID_MOUSE_Private_FunctionPrototypes
  * @{
  */
static void  MOUSE_Init(void);
static void  MOUSE_Decode(uint8_t *data);
/**
  * @}
  */


/** @defgroup USBH_HID_MOUSE_Private_Variables
  * @{
  */


HID_MOUSE_Data_TypeDef HID_MOUSE_Data;
HID_cb_TypeDef HID_MOUSE_cb = {
    MOUSE_Init,
    MOUSE_Decode,
};
/**
  * @}
  */


/** @defgroup USBH_HID_MOUSE_Private_Functions
  * @{
  */

/**
* @brief  MOUSE_Init
*         Init Mouse State.
* @param  None
* @retval None
*/
static void  MOUSE_Init(void)
{
    /* Call User Init*/
    USR_MOUSE_Init();
}

/**
* @brief  MOUSE_Decode
*         Decode Mouse data
* @param  data : Pointer to Mouse HID data buffer
* @retval None
*/
static void  MOUSE_Decode(uint8_t *data)
{
    HID_MOUSE_Data.button = data[0];

    HID_MOUSE_Data.x      = data[1];
    HID_MOUSE_Data.y      = data[2];

    USR_MOUSE_ProcessData(&HID_MOUSE_Data);

}
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


/**
  * @}
  */
/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
