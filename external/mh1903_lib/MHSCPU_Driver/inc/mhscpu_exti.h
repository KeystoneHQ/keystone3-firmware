/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_exti.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the EXTI firmware library
 *****************************************************************************/

#ifndef __MHSCPU_EXTI_H
#define __MHSCPU_EXTI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes -----------------------------------------------------------------*/
#include "mhscpu.h"


/**
  * @brief  EXTI Trigger enumeration
  */
typedef enum {
    EXTI_Trigger_Off            = (uint32_t)0x00,
    EXTI_Trigger_Rising         = (uint32_t)0x01,
    EXTI_Trigger_Falling        = (uint32_t)0x02,
    EXTI_Trigger_Rising_Falling = (uint32_t)0x03
} EXTI_TriggerTypeDef;

#define IS_EXTI_TRIGGER(TRIGGER) (((TRIGGER) == EXTI_Trigger_Off) || \
                                  ((TRIGGER) == EXTI_Trigger_Rising) || \
                                  ((TRIGGER) == EXTI_Trigger_Falling) || \
                                  ((TRIGGER) == EXTI_Trigger_Rising_Falling))
/**
  * @}
  */


/** @defgroup EXTI_Lines
  * @{
  */
#define EXTI_Line0                      ((uint32_t)0x0000)  /*!< External interrupt line 0 */
#define EXTI_Line1                      ((uint32_t)0x0001)  /*!< External interrupt line 1 */
#define EXTI_Line2                      ((uint32_t)0x0002)  /*!< External interrupt line 2 */
#define EXTI_Line3                      ((uint32_t)0x0003)  /*!< External interrupt line 3 */
#define EXTI_Line4                      ((uint32_t)0x0004)  /*!< External interrupt line 4 */
#define EXTI_Line5                      ((uint32_t)0x0005)  /*!< External interrupt line 5 */
#define EXTI_Line6                      ((uint32_t)0x0006)  /*!< External interrupt line 6 */
#define EXTI_Line7                      ((uint32_t)0x0007)  /*!< External interrupt line 7 */

#define IS_EXTI_LINE(LINE)              (((LINE) == EXTI_Line0) || ((LINE) == EXTI_Line1) || \
                                         ((LINE) == EXTI_Line2) || ((LINE) == EXTI_Line3) || \
                                         ((LINE) == EXTI_Line4) || ((LINE) == EXTI_Line5) || \
                                         ((LINE) == EXTI_Line6) || ((LINE) == EXTI_Line7))
/**
  * @}
  */

/** @defgroup EXTI_PinSource
  * @{
  */
#define EXTI_PinSource0                 ((uint32_t)0x0001)  /*!< Pin 0 selected */
#define EXTI_PinSource1                 ((uint32_t)0x0002)  /*!< Pin 1 selected */
#define EXTI_PinSource2                 ((uint32_t)0x0004)  /*!< Pin 2 selected */
#define EXTI_PinSource3                 ((uint32_t)0x0008)  /*!< Pin 3 selected */
#define EXTI_PinSource4                 ((uint32_t)0x0010)  /*!< Pin 4 selected */
#define EXTI_PinSource5                 ((uint32_t)0x0020)  /*!< Pin 5 selected */
#define EXTI_PinSource6                 ((uint32_t)0x0040)  /*!< Pin 6 selected */
#define EXTI_PinSource7                 ((uint32_t)0x0080)  /*!< Pin 7 selected */
#define EXTI_PinSource8                 ((uint32_t)0x0100)  /*!< Pin 8 selected */
#define EXTI_PinSource9                 ((uint32_t)0x0200)  /*!< Pin 9 selected */
#define EXTI_PinSource10                ((uint32_t)0x0400)  /*!< Pin 10 selected */
#define EXTI_PinSource11                ((uint32_t)0x0800)  /*!< Pin 11 selected */
#define EXTI_PinSource12                ((uint32_t)0x1000)  /*!< Pin 12 selected */
#define EXTI_PinSource13                ((uint32_t)0x2000)  /*!< Pin 13 selected */
#define EXTI_PinSource14                ((uint32_t)0x4000)  /*!< Pin 14 selected */
#define EXTI_PinSource15                ((uint32_t)0x8000)  /*!< Pin 15 selected */
#define EXTI_PinSourceAll               ((uint32_t)0xffff)  /*!< Pin All selected */

#define IS_EXTI_PIN_SOURCE(PIN)         (((((PIN) & ~(uint32_t)0xFFFF)) == 0x00) && ((PIN) != (uint32_t)0x00))
/**
  * @}
  */


/** @defgroup EXTI_Exported_Functions
  * @{
  */

void EXTI_DeInit(void);
void EXTI_LineConfig(uint32_t EXTI_Line, uint32_t EXTI_PinSource,  EXTI_TriggerTypeDef EXTI_Trigger);
uint32_t EXTI_GetITStatus(void);
uint32_t EXTI_GetITLineStatus(uint32_t EXTI_Line);
void EXTI_ClearITPendingBit(uint32_t EXTI_Line);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif // __MHSCPU_EXTI_H

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
