
/* Includes ------------------------------------------------------------------*/
#include "misc.h"



#define AIRCR_VECTKEY_MASK    ((uint32_t)0x05FA0000)


/**
  * @brief  Configures the priority grouping: pre-emption priority and subpriority.
  * @param  NVIC_PriorityGroup: specifies the priority grouping bits length.
  *   This parameter can be one of the following values:
  *     @arg NVIC_PriorityGroup_0: 0 bits for pre-emption priority
  *                                4 bits for subpriority
  *     @arg NVIC_PriorityGroup_1: 1 bits for pre-emption priority
  *                                3 bits for subpriority
  *     @arg NVIC_PriorityGroup_2: 2 bits for pre-emption priority
  *                                2 bits for subpriority
  *     @arg NVIC_PriorityGroup_3: 3 bits for pre-emption priority
  *                                1 bits for subpriority
  *     @arg NVIC_PriorityGroup_4: 4 bits for pre-emption priority
  *                                0 bits for subpriority
  * @retval None
  */
void NVIC_PriorityGroupConfig(uint32_t NVIC_PriorityGroup)
{
    /* Check the parameters */
    assert_param(IS_NVIC_PRIORITY_GROUP(NVIC_PriorityGroup));

    /* Set the PRIGROUP[10:8] bits according to NVIC_PriorityGroup value */
    SCB->AIRCR = AIRCR_VECTKEY_MASK | NVIC_PriorityGroup;
}

/**
  * @brief  Initializes the NVIC peripheral according to the specified
  *   parameters in the NVIC_InitStruct.
  * @param  NVIC_InitStruct: pointer to a NVIC_InitTypeDef structure that contains
  *   the configuration information for the specified NVIC peripheral.
  * @retval None
  */
void NVIC_Init(NVIC_InitTypeDef* NVIC_InitStruct)
{
    uint32_t tmpPriority, tmpPriorityGrouping;

    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NVIC_InitStruct->NVIC_IRQChannelCmd));
    assert_param(IS_NVIC_PREEMPTION_PRIORITY(NVIC_InitStruct->NVIC_IRQChannelPreemptionPriority));
    assert_param(IS_NVIC_SUB_PRIORITY(NVIC_InitStruct->NVIC_IRQChannelSubPriority));

    if (NVIC_InitStruct->NVIC_IRQChannelCmd != DISABLE) {
        /* Compute the Corresponding IRQ Priority --------------------------------*/
        tmpPriorityGrouping = NVIC_GetPriorityGrouping();
        tmpPriority = NVIC_EncodePriority(tmpPriorityGrouping, NVIC_InitStruct->NVIC_IRQChannelPreemptionPriority, \
                                          NVIC_InitStruct->NVIC_IRQChannelSubPriority);
        NVIC_SetPriority((IRQn_Type)NVIC_InitStruct->NVIC_IRQChannel, tmpPriority);

        /* Enable the Selected IRQ Channels --------------------------------------*/
        NVIC_EnableIRQ((IRQn_Type)NVIC_InitStruct->NVIC_IRQChannel);
    } else {
        /* Disable the Selected IRQ Channels -------------------------------------*/
        NVIC_DisableIRQ((IRQn_Type)NVIC_InitStruct->NVIC_IRQChannel);
    }
}

/**
  * @brief  Sets the vector table location and Offset.
  * @param  NVIC_VectTab: specifies if the vector table is in RAM or FLASH memory.
  *   This parameter can be one of the following values:
  *     @arg NVIC_VectTab_RAM
  *     @arg NVIC_VectTab_FLASH
  * @param  Offset: Vector Table base offset field. This value must be a multiple of 0x100.
  * @retval None
  */
void NVIC_SetVectorTable(uint32_t NVIC_VectTab, uint32_t Offset)
{
    /* Check the parameters */
    assert_param(IS_NVIC_VECTTAB(NVIC_VectTab));
    assert_param(IS_NVIC_OFFSET(Offset));

    SCB->VTOR = NVIC_VectTab | (Offset & (uint32_t)0x1FFFFF80);
}

/**
  * @brief  Configures the SysTick clock source.
  * @param  SysTick_CLKSource: specifies the SysTick clock source.
  *   This parameter can be one of the following values:
  *     @arg SysTick_CLKSource_HCLK_Div8: AHB clock divided by 8 selected as SysTick clock source.
  *     @arg SysTick_CLKSource_HCLK: AHB clock selected as SysTick clock source.
  * @retval None
  */
void SysTick_CLKSourceConfig(uint32_t SysTick_CLKSource)
{
    /* Check the parameters */
    assert_param(IS_SYSTICK_CLK_SOURCE(SysTick_CLKSource));
    if (SysTick_CLKSource == SysTick_CLKSource_HCLK) {
        SysTick->CTRL |= SysTick_CLKSource_HCLK;
    } else {
        SysTick->CTRL &= SysTick_CLKSource_HCLK_Div8;
    }
}

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
