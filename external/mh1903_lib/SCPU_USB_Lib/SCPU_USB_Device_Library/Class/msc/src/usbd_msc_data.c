/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_req.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : This file provides the standard USB requests following chapter 9.
 *****************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usbd_msc_data.h"


/** @defgroup MSC_DATA
  * @brief Mass storage info/data module
  * @{
  */

/** @defgroup MSC_DATA_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup MSC_DATA_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup MSC_DATA_Private_Macros
  * @{
  */
/**
  * @}
  */


/** @defgroup MSC_DATA_Private_Variables
  * @{
  */


/* USB Mass storage Page 0 Inquiry Data */
const uint8_t  MSC_Page00_Inquiry_Data[] = {//7
    0x00,
    0x00,
    0x00,
    (LENGTH_INQUIRY_PAGE00 - 4),
    0x00,
    0x80,
    0x83
};
/* USB Mass storage sense 6  Data */
const uint8_t  MSC_Mode_Sense6_data[] = {
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
};
/* USB Mass storage sense 10  Data */
const uint8_t  MSC_Mode_Sense10_data[] = {
    0x00,
    0x06,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
};
/**
  * @}
  */


/** @defgroup MSC_DATA_Private_FunctionPrototypes
  * @{
  */
/**
  * @}
  */


/** @defgroup MSC_DATA_Private_Functions
  * @{
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
