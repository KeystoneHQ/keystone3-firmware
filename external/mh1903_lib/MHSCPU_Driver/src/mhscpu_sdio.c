/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_sdio.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the SDIO firmware functions
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "mhscpu_sdio.h"


/********************* SDIO RESP Registers Address *********************/
#define SDIO_RESP_ADDR                  ((uint32_t)(SDIO_BASE + 0x30))


/**
  * @brief  Deinitializes the SDIO Peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void SDIO_DeInit(void)
{
    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_SDIOM, ENABLE);
}

/**
  * @brief  Initializes the SDIO Peripheral according to the specified parameters
  *         in the SDIO_InitStruct.
  * @param  SDIO_InitStruct: pointer to a SDIO_InitTypeDef structure that contains
  *         the configuration information for the SDIO peripheral.
  * @retval None
  */
void SDIO_Init(SDIO_InitTypeDef *SDIO_InitStruct)
{
    /* Check the parameters */
    assert_param(IS_SDIO_BUS_WIDE(SDIO_InitStruct->SDIO_BusWide));
    assert_param(IS_FUNCTIONAL_STATE(SDIO_InitStruct->SDIO_ClockLowPower));
    assert_param(IS_SDIO_CLOCK_DIVIDER(SDIO_InitStruct->SDIO_ClockDiv));

    /* Set CLKDIV */
    SDIO->CLKDIV = SDIO_InitStruct->SDIO_ClockDiv;

    /* Set BusWide */
    SDIO->CTYPE = SDIO_InitStruct->SDIO_BusWide;

    /* Set Clock Low Power */
    if (ENABLE == SDIO_InitStruct->SDIO_ClockLowPower) {
        SDIO->CLKENA |= SDIO_ClockLowPower_Enable;
    } else {
        SDIO->CLKENA &= ~SDIO_ClockLowPower_Enable;
    }
}

/**
  * @brief  Fills each SDIO_InitStruct member with its default value.
  * @param  SDIO_InitStruct: pointer to a SDIO_InitTypeDef structure which will be initialized.
  * @retval None
  */
void SDIO_StructInit(SDIO_InitTypeDef *SDIO_InitStruct)
{
    SDIO_InitStruct->SDIO_ClockDiv = 0x00;
    SDIO_InitStruct->SDIO_BusWide = SDIO_BusWide_1b;
    SDIO_InitStruct->SDIO_ClockLowPower = DISABLE;
}

/**
  * @brief  Set the power status of the controller.
  * @param  SDIO_PowerState: new state of the power state.
  *         This parameter can be one of the following values:
  *         @arg SDIO_PowerState_OFF: SDIO powe OFF
  *         @arg SDIO_PowerState_ON: SDIO powe ON
  * @retval None
  */
void SDIO_SetPowerState(uint32_t SDIO_PowerState)
{
    /* Check the parameters */
    assert_param(IS_SDIO_POWER_STATE(SDIO_PowerState));

    SDIO->PWREN = SDIO_PowerState;
}

/**
  * @brief  Get the power status of the controller.
  * @param  None
  * @retval Power status of the controller.The returned value can be one of the following values:
  *         0x0: Power OFF
  *         0x1: Power ON
  */
uint32_t SDIO_GetPowerState(void)
{
    return SDIO->PWREN;
}

/**
  * @brief  Enable or disable the SDIO clock
  * @param  NewState: new state of the selected SDIO clock state.
            This parameter can be ENABLE or DISABLE.
  * @retval None
  */
void SDIO_ClockCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (ENABLE == NewState) {
        SDIO->CLKENA |= SDIO_Clock_Enable;
    } else {
        SDIO->CLKENA &= ~SDIO_Clock_Enable;
    }
}

/**
  * @brief  Initializes the each SDIO Command according to the specified parameters
  *         in the SDIO_CmdInitStruct and send the command.
  * @param  SDIO_CmdInitStruct: pointer to a SDIO_CmdInitTypeDef structure that contains
  *         the configuration information for the SDIO command.
  * @retval None
  */
void SDIO_SendCommand(SDIO_CmdInitTypeDef *SDIO_CmdInitStruct)
{
    /* Check the parameters */
    assert_param(IS_SDIO_CMD_INDEX(SDIO_CmdInitStruct->SDIO_Cmd.b.CmdIndex));
    assert_param(IS_SDIO_RESPONSE(SDIO_CmdInitStruct->SDIO_Cmd.b.Response));

    SDIO->CMDARG = SDIO_CmdInitStruct->SDIO_Argument;
    SDIO->CMD = SDIO_CmdInitStruct->SDIO_Cmd.d;
}

/**
  * @brief  Fills each SDIO_CmdInitStruct member with its default value.
  * @param  SDIO_CmdInitStruct: pointer to a SDIO_CmdInitTypeDef structure which will be initialized.
  * @retval None
  */
void SDIO_CmdStructInit(SDIO_CmdInitTypeDef *SDIO_CmdInitStruct)
{
    SDIO_CmdInitStruct->SDIO_Argument = 0;
    SDIO_CmdInitStruct->SDIO_Cmd.d = 0;
}

/**
  * @brief  Returns commond index of last command for which response received.
  * @param  None
  * @retval The command index of the last command response received.
  */
uint8_t SDIO_GetCommandResponse(void)
{
    return (SDIO->STATUS >> SDIO_STATUS_RESPONSE_INDEX_POS) & SDIO_STATUS_RESPONSE_INDEX_MASK;
}

/**
  * @brief  Returns response received from the card for the last command.
  * @param  SDIO_RESP: Specifies the SDIO response register.
  *         This parameter can be one of the following values:
  *         @arg SDIO_RESP0: Response Register 0
  *         @arg SDIO_RESP1: Response Register 1
  *         @arg SDIO_RESP2: Response Register 2
  *         @arg SDIO_RESP3: Response Register 3
  * @retval The Corresponding response register value
  */
uint32_t SDIO_GetResponse(uint32_t SDIO_RESP)
{
    __IO uint32_t tmp = 0;

    /* Check the parameters */
    assert_param(IS_SDIO_RESP(SDIO_RESP));

    tmp = SDIO_RESP_ADDR + SDIO_RESP;

    return (*(__IO uint32_t *)tmp);
}

/**
  * @brief  Initializes the SDIO data path according to the specified parameters
  *         in the SDIO_DataInitStruct.
  * @param  SDIO_DataInitStruct: pointer to a SDIO_DataInitTypeDef structure that contains
  *         the configuration information for the SDIO command.
  * @retval None
  */
void SDIO_DataConfig(SDIO_DataInitTypeDef *SDIO_DataInitStruct)
{
    /* Check the parameters */
    assert_param(IS_SDIO_DATA_LENGTH(SDIO_DataInitStruct->SDIO_DataLength));

    /* Set SDIO TMOUT */
    SDIO->TMOUT = (SDIO->TMOUT & ~SDIO_DATA_TIMEOUT_MASK) | (SDIO_DataInitStruct->SDIO_DataTimeOut << 8);

    /* Set SDIO DataLength */
    SDIO->BYTCNT = SDIO_DataInitStruct->SDIO_DataLength;

    /* Set SDIO Block Size */
    SDIO->BLKSIZ = SDIO_DataInitStruct->SDIO_DataBlockSize & SDIO_BLOCK_SIZE_MASK;
}

/**
  * @brief  Fills each SDIO_DataInitStruct member with its default value.
  * @param  SDIO_DataInitStruct: pointer to a SDIO_DataInitTypeDef structure which will be initialized.
  * @retval None
  */
void SDIO_DataStructInit(SDIO_DataInitTypeDef *SDIO_DataInitStruct)
{
    SDIO_DataInitStruct->SDIO_DataTimeOut = 0xFFFFFF;
    SDIO_DataInitStruct->SDIO_DataLength = 0x00;
    SDIO_DataInitStruct->SDIO_DataBlockSize = 0x00;
}

/**
  * @brief  Read one data word from Rx FIFO
  * @param  None
  * @retval Data Received
  */
uint32_t SDIO_ReadData(void)
{
    return SDIO->FIFO;
}

/**
  * @brief  Write one data word to Tx FIFO
  * @param  Data: 32-bit data word to write
  * @retval None
  */
void SDIO_WriteData(uint32_t Data)
{
    SDIO->FIFO = Data;
}

/**
  * @brief  Returns the number of words left to be written to or read from FIFO
  * @param  None
  * @retval Remaining number of words
  */
uint32_t SDIO_GetFIFOCount(void)
{
    return (SDIO->STATUS >> SDIO_STATUS_FIFO_COUNT_POS) & SDIO_STATUS_FIFO_COUNT_MASK;
}

/**
  * @brief  Initializes the SDIO DMA interface according to the specified parameters.
  * @param  SDIO_DMAInitStruct: pointer to a SDIO_DMAInitTypeDef structure that contains the configuration information.
  * @retval None
  */
void SDIO_DMAInit(SDIO_DMAInitTypeDef *SDIO_DMAInitStruct)
{
    uint32_t tmpreg = 0;

    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(SDIO_DMAInitStruct->SDIO_DMAEnCmd));
    assert_param(IS_SDIO_DMA_BURST_SIZE(SDIO_DMAInitStruct->SDIO_DMABurstSize));
    assert_param(IS_SDIO_TX_FIFO_WMARK(SDIO_DMAInitStruct->SDIO_DMATransmitLevel));
    assert_param(IS_SDIO_RX_FIFO_WMARK(SDIO_DMAInitStruct->SDIO_DMAReceiveLevel));

    tmpreg = SDIO->FIFOTH;
    tmpreg &= ~(SDIO_FIFOTH_DMA_BURST_SIZE_MASK | SDIO_FIFOTH_RX_WMARK_MASK | SDIO_FIFOTH_TX_WMARK_MASK);
    tmpreg |= (uint32_t)SDIO_DMAInitStruct->SDIO_DMABurstSize << SDIO_FIFOTH_DMA_BURST_SIZE_POS
              | SDIO_DMAInitStruct->SDIO_DMAReceiveLevel << SDIO_FIFOTH_RX_WMARK_POS
              | SDIO_DMAInitStruct->SDIO_DMATransmitLevel << SDIO_FIFOTH_TX_WMARK_POS;

    SDIO->FIFOTH = tmpreg;

    if (ENABLE == SDIO_DMAInitStruct->SDIO_DMAEnCmd) {
        SDIO->CTRL |= SDIO_CTRL_DMA_ENABLE;
    } else {
        SDIO->CTRL &= ~SDIO_CTRL_DMA_ENABLE;
    }
}

/**
  * @brief  Initializes each SDIO_DMAInitStruct member with its default value.
  * @param  SDIO_DMAInitStruct: pointer to a SDIO_DMAInitTypeDef structure which will be initialized.
  * @retval None
  */
void SDIO_DMAStructInit(SDIO_DMAInitTypeDef *SDIO_DMAInitStruct)
{
    SDIO_DMAInitStruct->SDIO_DMABurstSize = 0;
    SDIO_DMAInitStruct->SDIO_DMAEnCmd = DISABLE;
    SDIO_DMAInitStruct->SDIO_DMAReceiveLevel = SDIO_RXFIFOWMARK_8;
    SDIO_DMAInitStruct->SDIO_DMATransmitLevel = SDIO_TXFIFOWMARK_8;
}

/**
  * @brief  Enable or disable the SDIO DMA request
  * @param  NewState: new state of the selected SDIO DMA request.
            This parameter can be ENABLE or DISABLE.
  * @retval None
  */
void SDIO_DMACmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (ENABLE == NewState) {
        SDIO->CTRL |= SDIO_CTRL_DMA_ENABLE;
    } else {
        SDIO->CTRL &= ~SDIO_CTRL_DMA_ENABLE;
    }
}

/**
  * @brief  Enable or disable the SDIO global interrupt.
  * @param  NewState: new state of the selected SDIO global interrupt.
            This parameter can be ENABLE or DISABLE.
  * @retval None
  */
void SDIO_GlobalITCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (ENABLE == NewState) {
        SDIO->CTRL |= SDIO_CTRL_GLOBAL_IT_ENABLE;
    } else {
        SDIO->CTRL &= ~SDIO_CTRL_GLOBAL_IT_ENABLE;
    }
}

/**
  * @brief  Enable or disable the SDIO interrupt.
  * @param  SDIO_IT: specifies the SDIO interrupt sources to be enabled or disabled.
  *         This parameter can be one of the following values:
  *         @arg SDIO_IT_CARDDET:              Card detect
  *         @arg SDIO_IT_RESPERR:              Response error
  *         @arg SDIO_IT_CMDDONE:              Command done
  *         @arg SDIO_IT_DATAEND:              Data transfer over
  *         @arg SDIO_IT_TXDREQ:               Transmit FIFO data request
  *         @arg SDIO_IT_RXDREQ:               Receive FIFO data request
  *         @arg SDIO_IT_RCRCFAIL:             Response CRC error
  *         @arg SDIO_IT_DCRCFAIL:             Data CRC error
  *         @arg SDIO_IT_RTIMEOUT:             Response timeout
  *         @arg SDIO_IT_DTIMEOUT:             Data read timeout
  *         @arg SDIO_IT_DHTIMEOUT_VOLSWITCH:  Data starvation-by-host timeout (HTO) /Volt_switch_int
  *         @arg SDIO_IT_FIFO_OVERFLOW:        FIFO underrun/overrun error
  *         @arg SDIO_IT_HLW_ERR:              Hardware locked write error
  *         @arg SDIO_IT_STBITERR:             Start-bit error
  *         @arg SDIO_IT_AUTO_CMDSENT:         Auto command done
  *         @arg SDIO_IT_EDBITERR:             End-bit error (read)/Write no CRC
  * @param  NewState: new state of the selected SDIO global interrupt.
            This parameter can be ENABLE or DISABLE.
  * @retval None
  */
void SDIO_ITConfig(uint32_t SDIO_IT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_SDIO_IT(SDIO_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (DISABLE != NewState) {
        SDIO->INTMASK |= (SDIO_IT_CARD | SDIO_IT);
    } else {
        SDIO->INTMASK &= ~SDIO_IT;
    }
}

/**
  * @brief  Get raw status of the specified SDIO interrupt.
  * @param  SDIO_IT: specifies the SDIO interrupt sources.
  *         This parameter can be one of the following values:
  *         @arg SDIO_IT_CARDDET:              Card detect
  *         @arg SDIO_IT_RESPERR:              Response error
  *         @arg SDIO_IT_CMDDONE:              Command done
  *         @arg SDIO_IT_DATAEND:              Data transfer over
  *         @arg SDIO_IT_TXDREQ:               Transmit FIFO data request
  *         @arg SDIO_IT_RXDREQ:               Receive FIFO data request
  *         @arg SDIO_IT_RCRCFAIL:             Response CRC error
  *         @arg SDIO_IT_DCRCFAIL:             Data CRC error
  *         @arg SDIO_IT_RTIMEOUT:             Response timeout
  *         @arg SDIO_IT_DTIMEOUT:             Data read timeout
  *         @arg SDIO_IT_DHTIMEOUT_VOLSWITCH:  Data starvation-by-host timeout (HTO) /Volt_switch_int
  *         @arg SDIO_IT_FIFO_OVERFLOW:        FIFO underrun/overrun error
  *         @arg SDIO_IT_HLW_ERR:              Hardware locked write error
  *         @arg SDIO_IT_STBITERR:             Start-bit error
  *         @arg SDIO_IT_AUTO_CMDSENT:         Auto command done
  *         @arg SDIO_IT_EDBITERR:             End-bit error (read)/Write no CRC
  * @retval The raw state of SDIO_IT (SET or RESET)
  */
ITStatus SDIO_GetRawITStatus(uint32_t SDIO_IT)
{
    /* Check the parameters */
    assert_param(IS_SDIO_GET_IT(SDIO_IT));

    if (SDIO->RINTSTS & SDIO_IT) {
        return SET;
    } else {
        return RESET;
    }
}

/**
  * @brief  Check whether the specified SDIO interrupt has occurred or not.
  * @param  SDIO_IT: specifies the SDIO interrupt sources to check.
  *         This parameter can be one of the following values:
  *         @arg SDIO_IT_CARDDET:              Card detect
  *         @arg SDIO_IT_RESPERR:              Response error
  *         @arg SDIO_IT_CMDDONE:              Command done
  *         @arg SDIO_IT_DATAEND:              Data transfer over
  *         @arg SDIO_IT_TXDREQ:               Transmit FIFO data request
  *         @arg SDIO_IT_RXDREQ:               Receive FIFO data request
  *         @arg SDIO_IT_RCRCFAIL:             Response CRC error
  *         @arg SDIO_IT_DCRCFAIL:             Data CRC error
  *         @arg SDIO_IT_RTIMEOUT:             Response timeout
  *         @arg SDIO_IT_DTIMEOUT:             Data read timeout
  *         @arg SDIO_IT_DHTIMEOUT_VOLSWITCH:  Data starvation-by-host timeout (HTO) /Volt_switch_int
  *         @arg SDIO_IT_FIFO_OVERFLOW:        FIFO underrun/overrun error
  *         @arg SDIO_IT_HLW_ERR:              Hardware locked write error
  *         @arg SDIO_IT_STBITERR:             Start-bit error
  *         @arg SDIO_IT_AUTO_CMDSENT:         Auto command done
  *         @arg SDIO_IT_EDBITERR:             End-bit error (read)/Write no CRC
  * @retval The new state of SDIO_IT (SET or RESET)
  */
ITStatus SDIO_GetITStatus(uint32_t SDIO_IT)
{
    /* Check the parameters */
    assert_param(IS_SDIO_GET_IT(SDIO_IT));

    if ((SDIO->MINTSTS & SDIO_IT)) {
        return SET;
    } else {
        return RESET;
    }
}

/**
  * @brief  Clears the SDIO's interrupt pending bits.
  * @param  SDIO_IT: specifies the SDIO interrupt pending bit to clear.
  *         This parameter can be one of the following values:
  *         @arg SDIO_IT_CARDDET:              Card detect
  *         @arg SDIO_IT_RESPERR:              Response error
  *         @arg SDIO_IT_CMDDONE:              Command done
  *         @arg SDIO_IT_DATAEND:              Data transfer over
  *         @arg SDIO_IT_TXDREQ:               Transmit FIFO data request
  *         @arg SDIO_IT_RXDREQ:               Receive FIFO data request
  *         @arg SDIO_IT_RCRCFAIL:             Response CRC error
  *         @arg SDIO_IT_DCRCFAIL:             Data CRC error
  *         @arg SDIO_IT_RTIMEOUT:             Response timeout
  *         @arg SDIO_IT_DTIMEOUT:             Data read timeout
  *         @arg SDIO_IT_DHTIMEOUT_VOLSWITCH:  Data starvation-by-host timeout (HTO) /Volt_switch_int
  *         @arg SDIO_IT_FRUN:                 FIFO underrun/overrun error
  *         @arg SDIO_IT_HLW_ERR:              Hardware locked write error
  *         @arg SDIO_IT_STBITERR:             Start-bit error
  *         @arg SDIO_IT_AUTO_CMDSENT:         Auto command done
  *         @arg SDIO_IT_EDBITERR:             End-bit error (read)/Write no CRC
  * @retval None
  */
void SDIO_ClearITPendingBit(uint32_t SDIO_IT)
{
    /* Check the parameters */
    assert_param(IS_SDIO_IT(SDIO_IT));

    SDIO->RINTSTS = SDIO_IT;
}

/**
  * @brief  Check whether the specified SDIO FLAG has occurred or not.
  * @param  SDIO_FLAG: specifies the flag to check.
  *         This parameter can be one of the following values:
  *         @arg SDIO_FLAG_RXFIFO_WATERMARK:       FIFO reached Receive watermark level; not qualified with data transfer
  *         @arg SDIO_FLAG_TXFIFO_WATERMARK:       FIFO reached Transmit watermark level; not qualified with data transfer
  *         @arg SDIO_FLAG_FIFO_EMPTY:             FIFO is empty status
  *         @arg SDIO_FLAG_FIFO_FULL:              FIFO is full status
  *         @arg SDIO_FLAG_CARD_PRESENT:           Raw selected card_data[3]; checks whether card is present
  *         @arg SDIO_FLAG_DATA_BUSY:              Inverted version of raw selected card_data[0]
  * @retval The new state of SDIO_FLAG (SET or RESET)
  */
FlagStatus SDIO_GetFlagStatus(uint32_t SDIO_FLAG)
{
    /* Check the parameters */
    assert_param(IS_SDIO_GET_FLAG(SDIO_FLAG));

    if (RESET != (SDIO->STATUS & SDIO_FLAG)) {
        return SET;
    } else {
        return RESET;
    }
}


/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
