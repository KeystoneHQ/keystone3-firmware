/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_dcmi.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the DCMI firmware functions
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "mhscpu_dcmi.h"


/**
  * @brief  Deinitializes the DCMI registers to their default reset values.
  * @param  None
  * @retval None
  */
void DCMI_DeInit(void)
{
    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_DCMIS, ENABLE);
}

/**
  * @brief  Initializes the DCMI according to the specified parameters in the DCMI_InitStruct.
  * @param  DCMI_InitStruct: pointer to a DCMI_InitTypeDef structure that contains
  *         the configuration information for the DCMI.
  * @retval None
  */
void DCMI_Init(DCMI_InitTypeDef* DCMI_InitStruct)
{
    uint32_t temp = 0x0;

    /* Check the parameters */
    assert_param(IS_DCMI_CAPTURE_MODE(DCMI_InitStruct->DCMI_CaptureMode));
    assert_param(IS_DCMI_SYNCHRO(DCMI_InitStruct->DCMI_SynchroMode));
    assert_param(IS_DCMI_PCKPOLARITY(DCMI_InitStruct->DCMI_PCKPolarity));
    assert_param(IS_DCMI_VSPOLARITY(DCMI_InitStruct->DCMI_VSPolarity));
    assert_param(IS_DCMI_HSPOLARITY(DCMI_InitStruct->DCMI_HSPolarity));
    assert_param(IS_DCMI_CAPTURE_RATE(DCMI_InitStruct->DCMI_CaptureRate));
    assert_param(IS_DCMI_EXTENDED_DATA(DCMI_InitStruct->DCMI_ExtendedDataMode));
    assert_param(IS_DCMI_CLOCK_DIV(DCMI_InitStruct->DCMI_ClockDiv));
    assert_param(IS_DCMI_BYTESELECT_MODE(DCMI_InitStruct->DCMI_ByteSelectMode));
    assert_param(IS_DCMI_LINESELECT_MODE(DCMI_InitStruct->DCMI_LineSelectMode));

    temp = SYSCTRL->FREQ_SEL;
    temp &= ~(DCMI_Clock_Div_MSK);
    temp |= DCMI_InitStruct->DCMI_ClockDiv;

    SYSCTRL->FREQ_SEL = temp;

    /* The DCMI configuration registers should be programmed correctly before
    enabling the CR_ENABLE Bit and the CR_CAPTURE Bit */
    DCMI->CR &= ~(DCMI_CR_ENABLE | DCMI_CR_CAPTURE);

    /* Reset the old DCMI configuration */
    temp = DCMI->CR;

    temp &= ~((uint32_t)DCMI_CR_CM   | DCMI_CR_ESS   | DCMI_CR_PCKPOL |
              DCMI_CR_HSPOL  | DCMI_CR_VSPOL | DCMI_CR_FCRC_0 |
              DCMI_CR_FCRC_1 | DCMI_CR_EDM_0 | DCMI_CR_EDM_1  |
              DCMI_CR_BSM_0  | DCMI_CR_BSM_1 | DCMI_CR_OEBS   |
              DCMI_CR_LSM    | DCMI_CR_OELS);

    /* Sets the new configuration of the DCMI peripheral */
    temp |= ((uint32_t)DCMI_InitStruct->DCMI_CaptureMode |
             DCMI_InitStruct->DCMI_SynchroMode |
             DCMI_InitStruct->DCMI_PCKPolarity |
             DCMI_InitStruct->DCMI_VSPolarity |
             DCMI_InitStruct->DCMI_HSPolarity |
             DCMI_InitStruct->DCMI_CaptureRate |
             DCMI_InitStruct->DCMI_ExtendedDataMode |
             DCMI_InitStruct->DCMI_ByteSelectMode |
             DCMI_InitStruct->DCMI_LineSelectMode);

    DCMI->CR = temp;
}

/**
  * @brief  Config DCMI DMA FIFO Full Size
  * @param  NewFIFOSize: DCMI DMA Request FIFO FULL SIZE:
                DCMI_DMARequestFIFOSize_1Word,
                DCMI_DMARequestFIFOSize_2Word,
                DCMI_DMARequestFIFOSize_4Word;
  * @retval None
  */
void DCMI_SetDMARequsetFIFOSize(uint32_t NewFIFOSize)
{
    uint32_t temp;
    assert_param(IS_DCMI_DMAREQUESTFIFO_SIZE(NewFIFOSize));

    temp = DCMI->CR;
    if (temp & DCMI_CR_CAPTURE) {
        return;
    } else {
        temp &= ~(DCMI_DMARequestFIFOSize_MSK);
        temp |= NewFIFOSize;
        DCMI->CR = temp;
    }
}

/**
  * @brief  Fills each DCMI_InitStruct member with its default value.
  * @param  DCMI_InitStruct : pointer to a DCMI_InitTypeDef structure which will
  *         be initialized.
  * @retval None
  */
void DCMI_StructInit(DCMI_InitTypeDef* DCMI_InitStruct)
{
    /* Set the default configuration */
    DCMI_InitStruct->DCMI_CaptureMode = DCMI_CaptureMode_Continuous;
    DCMI_InitStruct->DCMI_SynchroMode = DCMI_SynchroMode_Hardware;
    DCMI_InitStruct->DCMI_PCKPolarity = DCMI_PCKPolarity_Falling;
    DCMI_InitStruct->DCMI_VSPolarity = DCMI_VSPolarity_Low;
    DCMI_InitStruct->DCMI_HSPolarity = DCMI_HSPolarity_Low;
    DCMI_InitStruct->DCMI_CaptureRate = DCMI_CaptureRate_All_Frame;
    DCMI_InitStruct->DCMI_ClockDiv = DCMI_Clock_Div2;
    DCMI_InitStruct->DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;
    DCMI_InitStruct->DCMI_ByteSelectMode = DCMI_ByteSelect_Mode_AllByte;
    DCMI_InitStruct->DCMI_LineSelectMode = DCMI_LineSelect_Mode_AllLine;
}

/**
  * @brief  Initializes the DCMI peripheral CROP mode according to the specified
  *         parameters in the DCMI_CROPInitStruct.
  * @note   This function should be called before to enable and start the DCMI interface.
  * @param  DCMI_CROPInitStruct:  pointer to a DCMI_CROPInitTypeDef structure that
  *         contains the configuration information for the DCMI peripheral CROP mode.
  * @retval None
  */
void DCMI_CROPConfig(DCMI_CROPInitTypeDef* DCMI_CROPInitStruct)
{
    /* Sets the CROP window coordinates */
    DCMI->CWSTRTR = (uint32_t)((uint32_t)DCMI_CROPInitStruct->DCMI_HorizontalOffsetCount |
                               ((uint32_t)DCMI_CROPInitStruct->DCMI_VerticalStartLine << 16));

    /* Sets the CROP window size */
    DCMI->CWSIZER = (uint32_t)(DCMI_CROPInitStruct->DCMI_CaptureCount |
                               ((uint32_t)DCMI_CROPInitStruct->DCMI_VerticalLineCount << 16));
}

/**
  * @brief  Enables or disables the DCMI Crop feature.
  * @note   This function should be called before to enable and start the DCMI interface.
  * @param  NewState: new state of the DCMI Crop feature.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void DCMI_CROPCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE) {
        /* Enable the DCMI Crop feature */
        DCMI->CR |= (uint32_t)DCMI_CR_CROP;
    } else {
        /* Disable the DCMI Crop feature */
        DCMI->CR &= ~(uint32_t)DCMI_CR_CROP;
    }
}

/**
  * @brief  Enables or disables the DCMI JPEG format.
  * @note   The Crop and Embedded Synchronization features cannot be used in this mode.
  * @param  NewState: new state of the DCMI JPEG format.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void DCMI_JPEGCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE) {
        /* Enable the DCMI JPEG format */
        DCMI->CR |= (uint32_t)DCMI_CR_JPEG;
    } else {
        /* Disable the DCMI JPEG format */
        DCMI->CR &= ~(uint32_t)DCMI_CR_JPEG;
    }
}

/**
  * @brief  Enables or disables the DCMI interface.
  * @param  NewState: new state of the DCMI interface.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void DCMI_Cmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE) {
        /* Enable the DCMI by setting ENABLE bit */
        DCMI->CR |= (uint32_t)DCMI_CR_ENABLE;
    } else {
        /* Disable the DCMI by clearing ENABLE bit */
        DCMI->CR &= ~(uint32_t)DCMI_CR_ENABLE;
    }
}

/**
  * @brief  Enables or disables the DCMI Capture.
  * @param  NewState: new state of the DCMI capture.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void DCMI_CaptureCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE) {
        /* Enable the DCMI Capture */
        DCMI->CR |= (uint32_t)DCMI_CR_CAPTURE;
    } else {
        /* Disable the DCMI Capture */
        DCMI->CR &= ~(uint32_t)DCMI_CR_CAPTURE;
    }
}

/**
  * @brief  Reads the data stored in the DR register.
  * @param  None
  * @retval Data register value
  */
uint32_t DCMI_ReadData(void)
{
    return DCMI->DR;
}

/**
  * @brief  Get Fifo Used Size
  * @param
  * @retval return Fifo Used Words Size
  */
uint32_t DCMI_GetFIFODataWordNum(void)
{
    return (0x3 & (DCMI->SR >> 3));
}


/**
  * @brief  Enables or disables the DCMI interface interrupts.
  * @param  DCMI_IT: specifies the DCMI interrupt sources to be enabled or disabled.
  *          This parameter can be any combination of the following values:
  *            @arg DCMI_IT_FRAME: Frame capture complete interrupt mask
  *            @arg DCMI_IT_OVF: Overflow interrupt mask
  *            @arg DCMI_IT_ERR: Synchronization error interrupt mask
  *            @arg DCMI_IT_VSYNC: VSYNC interrupt mask
  *            @arg DCMI_IT_LINE: Line interrupt mask
  * @param  NewState: new state of the specified DCMI interrupts.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void DCMI_ITConfig(uint16_t DCMI_IT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_DCMI_CONFIG_IT(DCMI_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE) {
        /* Enable the Interrupt sources */
        DCMI->IER |= DCMI_IT;
    } else {
        /* Disable the Interrupt sources */
        DCMI->IER &= (uint16_t)(~DCMI_IT);
    }
}

/**
  * @brief  Checks whether the  DCMI interface flag is set or not.
  * @param  DCMI_FLAG: specifies the flag to check.
  *          This parameter can be one of the following values:
  *            @arg DCMI_FLAG_FRAMERI: Frame capture complete Raw flag mask
  *            @arg DCMI_FLAG_OVFRI: Overflow Raw flag mask
  *            @arg DCMI_FLAG_ERRRI: Synchronization error Raw flag mask
  *            @arg DCMI_FLAG_VSYNCRI: VSYNC Raw flag mask
  *            @arg DCMI_FLAG_LINERI: Line Raw flag mask
  *            @arg DCMI_FLAG_FRAMEMI: Frame capture complete Masked flag mask
  *            @arg DCMI_FLAG_OVFMI: Overflow Masked flag mask
  *            @arg DCMI_FLAG_ERRMI: Synchronization error Masked flag mask
  *            @arg DCMI_FLAG_VSYNCMI: VSYNC Masked flag mask
  *            @arg DCMI_FLAG_LINEMI: Line Masked flag mask
  *            @arg DCMI_FLAG_HSYNC: HSYNC flag mask
  *            @arg DCMI_FLAG_VSYNC: VSYNC flag mask
  *            @arg DCMI_FLAG_FNE: Fifo not empty flag mask
  * @retval The new state of DCMI_FLAG (SET or RESET).
  */
FlagStatus DCMI_GetFlagStatus(uint16_t DCMI_FLAG)
{
    FlagStatus bitstatus = RESET;
    uint32_t dcmireg, tempreg = 0;

    /* Check the parameters */
    assert_param(IS_DCMI_GET_FLAG(DCMI_FLAG));

    /* Get the DCMI register index */
    dcmireg = (((uint16_t)DCMI_FLAG) >> 12);

    if (dcmireg == 0x00) { /* The FLAG is in RISR register */
        tempreg = DCMI->RISR;
    } else if (dcmireg == 0x02) { /* The FLAG is in SR register */
        tempreg = DCMI->SR;
    } else { /* The FLAG is in MISR register */
        tempreg = DCMI->MISR;
    }

    if ((tempreg & DCMI_FLAG) != (uint16_t)RESET) {
        bitstatus = SET;
    } else {
        bitstatus = RESET;
    }

    /* Return the DCMI_FLAG status */
    return  bitstatus;
}

/**
  * @brief  Clears the DCMI's pending flags.
  * @param  DCMI_FLAG: specifies the flag to clear.
  *          This parameter can be any combination of the following values:
  *            @arg DCMI_FLAG_FRAMERI: Frame capture complete Raw flag mask
  *            @arg DCMI_FLAG_OVFRI: Overflow Raw flag mask
  *            @arg DCMI_FLAG_ERRRI: Synchronization error Raw flag mask
  *            @arg DCMI_FLAG_VSYNCRI: VSYNC Raw flag mask
  *            @arg DCMI_FLAG_LINERI: Line Raw flag mask
  * @retval None
  */
void DCMI_ClearFlag(uint16_t DCMI_FLAG)
{
    /* Check the parameters */
    assert_param(IS_DCMI_CLEAR_FLAG(DCMI_FLAG));

    /* Clear the flag by writing in the ICR register 1 in the corresponding
    Flag position*/
    DCMI->ICR = DCMI_FLAG;
}

/**
  * @brief  Checks whether the DCMI interrupt has occurred or not.
  * @param  DCMI_IT: specifies the DCMI interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg DCMI_IT_FRAME: Frame capture complete interrupt mask
  *            @arg DCMI_IT_OVF: Overflow interrupt mask
  *            @arg DCMI_IT_ERR: Synchronization error interrupt mask
  *            @arg DCMI_IT_VSYNC: VSYNC interrupt mask
  *            @arg DCMI_IT_LINE: Line interrupt mask
  * @retval The new state of DCMI_IT (SET or RESET).
  */
ITStatus DCMI_GetITStatus(uint16_t DCMI_IT)
{
    ITStatus bitstatus = RESET;
    uint32_t itstatus = 0;

    /* Check the parameters */
    assert_param(IS_DCMI_GET_IT(DCMI_IT));

    itstatus = DCMI->MISR & DCMI_IT; /* Only masked interrupts are checked */
    if ((itstatus != (uint16_t)RESET)) {
        bitstatus = SET;
    } else {
        bitstatus = RESET;
    }

    return bitstatus;
}

/**
  * @brief  Clears the DCMI's interrupt pending bits.
  * @param  DCMI_IT: specifies the DCMI interrupt pending bit to clear.
  *          This parameter can be any combination of the following values:
  *            @arg DCMI_IT_FRAME: Frame capture complete interrupt mask
  *            @arg DCMI_IT_OVF: Overflow interrupt mask
  *            @arg DCMI_IT_ERR: Synchronization error interrupt mask
  *            @arg DCMI_IT_VSYNC: VSYNC interrupt mask
  *            @arg DCMI_IT_LINE: Line interrupt mask
  * @retval None
  */
void DCMI_ClearITPendingBit(uint16_t DCMI_IT)
{
    /* Clear the interrupt pending Bit by writing in the ICR register 1 in the
    corresponding pending Bit position*/

    DCMI->ICR = DCMI_IT;
}


/**
  * @brief  set dcmi input from out signal
  */
void DCMI_SetExtInput()
{
    SYSCTRL->DBG_CR |= DCMI_FROM_OUT;
    return ;
}
/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
