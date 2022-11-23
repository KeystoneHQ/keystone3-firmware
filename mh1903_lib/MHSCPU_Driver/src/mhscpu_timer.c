/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_timer.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the TIMER firmware functions
 *****************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "mhscpu_timer.h"
#include "define.h"

/* Exported functions ---------------------------------------------------------*/
/**
  * @brief  Deinitializes the TIMx peripheral registers to their default reset values.
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @retval None
  */
void TIM_DeInit(TIM_Module_TypeDef *TIMMx)
{
    if (TIMM0 == TIMMx) {
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_TIMM0, ENABLE);
    }
}

/**
  * @brief  Initializes the TIMx Unit peripheral according to the specified parameters.
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @param  TIM_InitStruct: pointer to a TIM_InitTypeDef structor that contains the configuration information
  * @retval None
  */
void TIM_Init(TIM_Module_TypeDef *TIMMx, TIM_InitTypeDef *TIM_InitStruct)
{
    TIM_Cmd(TIMMx, TIM_InitStruct->TIMx, DISABLE);

    TIMMx->TIM[TIM_InitStruct->TIMx].ControlReg = 0;
    TIMMx->TIM[TIM_InitStruct->TIMx].ControlReg |= TIMER_CONTROL_REG_TIMER_MODE;
    TIMMx->TIM[TIM_InitStruct->TIMx].ControlReg &= ~TIMER_CONTROL_REG_TIMER_PWM;

    TIMMx->TIM[TIM_InitStruct->TIMx].LoadCount = TIM_InitStruct->TIM_Period;
}

/**
  * @brief  Initializes the TIMx PWM Unit peripheral according to the specified parameters.
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @param  TIM_PWMInitStruct: pointer to a TIM_PWMInitTypeDef structor that contains the configuration information
  * @retval None
  */
void TIM_PWMInit(TIM_Module_TypeDef *TIMMx, TIM_PWMInitTypeDef *TIM_PWMInitStruct)
{
    TIM_Cmd(TIMMx, TIM_PWMInitStruct->TIMx, DISABLE);

    TIMMx->TIM[TIM_PWMInitStruct->TIMx].ControlReg = 0;
    TIMMx->TIM[TIM_PWMInitStruct->TIMx].ControlReg |= TIMER_CONTROL_REG_TIMER_MODE;
    TIMMx->TIM[TIM_PWMInitStruct->TIMx].ControlReg |= TIMER_CONTROL_REG_TIMER_PWM;
    TIMMx->TIM[TIM_PWMInitStruct->TIMx].ControlReg |= TIMER_CONTROL_REG_TIMER_INTERRUPT;
    TIMMx->TIM[TIM_PWMInitStruct->TIMx].LoadCount = TIM_PWMInitStruct->TIM_LowLevelPeriod;
    TIMMx->TIM_ReloadCount[TIM_PWMInitStruct->TIMx] = TIM_PWMInitStruct->TIM_HighLevelPeriod;
}

/**
  * @brief  Gets the TIMx CurrentValue.
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @param  TIMx: x can be 0 to 7 to select the TIM Number
  * @retval Counter Current Value
  */
uint32_t TIM_GetCounter(TIM_Module_TypeDef *TIMMx, TIM_NumTypeDef TIMx)
{
    /* Get the Counter Register value */
    return TIMMx->TIM[TIMx].CurrentValue;
}

/**
  * @brief  Enable or disables the specified TIM peripheral
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @param  TIMx: x can be 0 to 7 to select the TIM Number
  * @param  NewState: new state of the TIMx
  *         This parameter can be: ENABLE or DISABLE
  * @retval None
  */
void TIM_Cmd(TIM_Module_TypeDef *TIMMx, TIM_NumTypeDef TIMx, FunctionalState NewState)
{
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE) {
        TIMMx->TIM[TIMx].ControlReg |= TIMER_CONTROL_REG_TIMER_ENABLE;
    } else {
        TIMMx->TIM[TIMx].ControlReg &= ~TIMER_CONTROL_REG_TIMER_ENABLE;
    }
}

/**
  * @brief  Config TIM unit mode
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @param  TIMx: x can be 0 to 7 to select the TIM Number
  * @param  TIM_Mode: TIM peripheral mode
  *         This parameter can be: TIM_Mode_General or TIM_Mode_PWM
  * @retval None
  */
void TIM_ModeConfig(TIM_Module_TypeDef *TIMMx, TIM_NumTypeDef TIMx, TIM_ModeTypeDef TIM_Mode)
{
    assert_param(IS_TIM_MODE(TIM_Mode));

    if (TIM_Mode_General == TIM_Mode) {
        TIMMx->TIM[TIMx].ControlReg &= ~TIMER_CONTROL_REG_TIMER_PWM;
    } else if (TIM_Mode_PWM == TIM_Mode) {
        TIMMx->TIM[TIMx].ControlReg |= TIMER_CONTROL_REG_TIMER_PWM;
    }
}

/**
  * @brief  Set general TIM period
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @param  TIMx: x can be 0 to 7 to select the TIM Number
  * @param  Period: TIM period value
  * @retval None
  */
void TIM_SetPeriod(TIM_Module_TypeDef *TIMMx, TIM_NumTypeDef TIMx, uint32_t Period)
{
    TIMMx->TIM[TIMx].LoadCount = Period;
}

/**
  * @brief  Set TIM PWM period
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @param  TIMx: x can be 0 to 7 to select the TIM Number
  * @param  PWM_LowLevelPeriod: low level period value
  * @param  PWM_HighLevelPeriod: high level period value
  * @retval None
  */
void TIM_SetPWMPeriod(TIM_Module_TypeDef *TIMMx, TIM_NumTypeDef TIMx, uint32_t PWM_LowLevelPeriod, uint32_t PWM_HighLevelPeriod)
{
    TIMMx->TIM[TIMx].LoadCount = PWM_LowLevelPeriod;
    TIMMx->TIM_ReloadCount[TIMx] = PWM_HighLevelPeriod;
}

/**
  * @brief  Enable or disable the TIM interrupt
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @param  TIMx: x can be 0 to 7 to select the TIM Number
  * @param  NewState: new state of the TIMx
  *         This parameter can be: ENABLE or DISABLE
  * @retval None
  */
void TIM_ITConfig(TIM_Module_TypeDef *TIMMx, TIM_NumTypeDef TIMx, FunctionalState NewState)
{
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE) {
        TIMMx->TIM[TIMx].ControlReg &= ~TIMER_CONTROL_REG_TIMER_INTERRUPT;
    } else {
        TIMMx->TIM[TIMx].ControlReg |= TIMER_CONTROL_REG_TIMER_INTERRUPT;
    }
}

/**
  * @brief  Clear the TIM interrupt
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @param  TIMx: x can be 0 to 7 to select the TIM Number
  * @retval None
  */
void TIM_ClearITPendingBit(TIM_Module_TypeDef *TIMMx, TIM_NumTypeDef TIMx)
{
    volatile uint32_t clr;
    clr = TIMMx->TIM[TIMx].EOI;
    UNUSED(clr);
}

/**
  * @brief  Get the specific TIM interrupt status
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @param  TIMx: x can be 0 to 7 to select the TIM Number
  * @retval The new state of specific TIM IT_Flag(SET or RESET)
  */
ITStatus TIM_GetITStatus(TIM_Module_TypeDef *TIMMx, TIM_NumTypeDef TIMx)
{
    if ((TIMMx->TIM[TIMx].IntStatus & TIMER_INT_STATUS_INTERRUPT) != RESET) {
        return SET;
    }

    return RESET;
}

/**
  * @brief  Get all TIM interrupt status
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @retval The new state of TIM IT_Flag
  */
uint32_t TIM_GetAllITStatus(TIM_Module_TypeDef *TIMMx)
{
    return TIMMx->TIM_IntStatus;
}

/**
  * @brief  Get the specific TIM raw interrupt status
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @param  TIMx: x can be 0 to 7 to select the TIM Number
  * @retval The new state of specific TIM IT_Flag(SET or RESET)
  */
ITStatus TIM_GetRawITStatus(TIM_Module_TypeDef *TIMMx, TIM_NumTypeDef TIMx)
{
    if (((TIMMx->TIM_RawIntStatus) & (0x01 << TIMx)) != RESET) {
        return SET;
    }

    return RESET;
}

/**
  * @brief  Get all TIM raw interrupt status
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @retval The new state of TIM raw IT_Flag
  */
uint32_t TIM_GetAllRawITStatus(TIM_Module_TypeDef *TIMMx)
{
    return TIMMx->TIM_RawIntStatus;
}

/**
  * @brief  Enable or disable the TIM PWM single pulse mode
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @param  TIMx: x can be 0 to 7 to select the TIM Number
  * @param  NewState: new state of the TIM PWM single pulse mode
  *         This parameter can be: ENABLE or DISABLE
  * @retval None
  */
void TIM_PWMSinglePulseConfig(TIM_Module_TypeDef *TIMMx, TIM_NumTypeDef TIMx, FunctionalState NewState)
{
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (DISABLE != NewState) {
        TIMMx->TIM[TIMx].ControlReg |= TIMER_CONTROL_REG_PWM_SINGLE_PULSE;
    } else {
        TIMMx->TIM[TIMx].ControlReg &= ~TIMER_CONTROL_REG_PWM_SINGLE_PULSE;
    }
}

/**
  * @brief  Reload the TIM PWM single pulse
  * @param  TIMMx: x can be 0 to select the TIM peripheral
  * @param  TIMx: x can be 0 to 5 to select the TIM Number
  * @retval None
  */
void TIM_PWMReloadSinglePulse(TIM_Module_TypeDef *TIMMx, TIM_NumTypeDef TIMx)
{
    TIMMx->TIM[TIMx].ControlReg |= TIMER_CONTROL_REG_PWM_RELOAD_SINGLE_PULSE;
}


/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
