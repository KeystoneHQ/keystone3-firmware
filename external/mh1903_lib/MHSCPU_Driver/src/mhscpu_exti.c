/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_exti.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the EXTI firmware functions
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "mhscpu_exti.h"

/** @defgroup EXTI_Private_Macros
  * @{
  */
#define EXTI_Num                          8
#define EXTI_Pin_Num                      16
/**
  * @}
  */


/* Exported functions -------------------------------------------------------*/
/**
  * @brief  Deinitialize the EXTI peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void EXTI_DeInit(void)
{
    uint32_t i;

    for (i = 0; i < EXTI_Num; i++) {
        GPIO->INTP_TYPE_STA[i].INTP_TYPE = 0;
        GPIO->INTP_TYPE_STA[i].INTP_STA = 0xFFFF;
    }
}

/**
  * @brief  Configure the EXTI peripheral
  * @param  EXTI_Line: specify the EXTI line num
  *      This parameter can be EXTI_Linex where x can be (0..7).
  * @param  EXTI_PinSource: specify the EXTI pin num
  *      This parameter can be EXTI_PinSourcex where x can be (0..15).
  * @param  EXTI_Trigger: specify the EXTI trigger mode
  *         This parameter can be one of the following values:
  *      @arg EXTI_Trigger_Off
  *      @arg EXTI_Trigger_Rising
  *      @arg EXTI_Trigger_Falling
  *      @arg EXTI_Trigger_Rising_Falling
  * @retval None
  */
void EXTI_LineConfig(uint32_t EXTI_Line, uint32_t EXTI_PinSource,  EXTI_TriggerTypeDef EXTI_Trigger)
{
    uint32_t i;

    assert_param(IS_EXTI_LINE(EXTI_Line));
    assert_param(IS_EXTI_PIN_SOURCE(EXTI_PinSource));
    assert_param(IS_EXTI_TRIGGER(EXTI_Trigger));

    for (i = 0; i < EXTI_Pin_Num; i++) {
        if (EXTI_PinSource & BIT(i)) {
            GPIO->INTP_TYPE_STA[EXTI_Line].INTP_TYPE &= ~(0x03 << (i * 2));
            GPIO->INTP_TYPE_STA[EXTI_Line].INTP_TYPE |= (EXTI_Trigger << (i * 2));
        }
    }
}

/**
  * @brief  Get all the EXTI line interrupt flag
  * @param  None
  * @retval The new state of all EXTI line
  */
uint32_t EXTI_GetITStatus(void)
{
    uint32_t i;
    uint32_t u32ret = 0;

    for (i = 0; i < EXTI_Num; i++) {
        u32ret |= GPIO->INTP[i] << i;
    }

    return u32ret;
}

/**
  * @brief  Check whether the specified EXTI line is asserted or not
  * @param  EXTI_Line: specifies the EXTI lines to clear
  *      This parameter can be EXTI_Linex where x can be (0..7).
  * @retval the new state of EXTI_Line
  */
uint32_t EXTI_GetITLineStatus(uint32_t EXTI_Line)
{
    return (GPIO->INTP_TYPE_STA[EXTI_Line].INTP_STA);
}

/**
  * @brief  Clear the EXTI's line pending bits
  * @param  EXTI_Line: specifies the EXTI lines to clear
  *      This parameter can be EXTI_Linex where x can be (0..7).
  * @retval None
  */
void EXTI_ClearITPendingBit(uint32_t EXTI_Line)
{
    GPIO->INTP_TYPE_STA[EXTI_Line].INTP_STA = 0xFFFF;
}

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
