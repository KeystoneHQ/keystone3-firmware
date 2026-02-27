/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_spi.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the SPI firmware functions
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "mhscpu_spi.h"


/* Exported functions -------------------------------------------------------*/
static void SPI_MasterSlaveModeSet(SPI_TypeDef *SPIx);

/**
  * @brief  Deinitializes the SPIx peripheral registers to their default reset values.
  * @param  SPIx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0, SPIM1, SPIM2, SPIM3 or SPIM4
  * @retval None
  */
void SPI_DeInit(SPI_TypeDef *SPIx)
{
    if (SPIM0 == SPIx || SPIS0 == SPIx) {
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_SPI0, ENABLE);
    } else if (SPIM1 == SPIx) {
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_SPI1, ENABLE);
    } else if (SPIM2 == SPIx) {
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_SPI2, ENABLE);
    } else if (SPIM3 == SPIx) {
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_SPI3, ENABLE);
    } else if (SPIM4 == SPIx) {
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_SPI4, ENABLE);
    }
}

/**
  * @brief  Initializes the SPIx peripheral according to the specified parameters.
  * @param  SPIx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0, SPIM1, SPIM2, SPIM3 or SPIM4
  * @param  SPI_InitStruct: pointer to a SPI_InitTypeDef structure that contains the configuration information.
  * @retval None
  */
void SPI_Init(SPI_TypeDef *SPIx, SPI_InitTypeDef *SPI_InitStruct)
{
    assert_param(IS_SPI_DIRECTION_MODE(SPI_InitStruct->SPI_Direction));
    assert_param(IS_SPI_DATASIZE(SPI_InitStruct->SPI_DataSize));
    assert_param(IS_SPI_NSS(SPI_InitStruct->SPI_NSS));
    assert_param(IS_SPI_CPOL(SPI_InitStruct->SPI_CPOL));
    assert_param(IS_SPI_CPHA(SPI_InitStruct->SPI_CPHA));
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_InitStruct->SPI_BaudRatePrescaler));
    assert_param(IS_SPI_RX_FIFO_FULL_THRESHOLD(SPI_InitStruct->SPI_RXFIFOFullThreshold));
    assert_param(IS_SPI_TX_FIFO_EMPTY_THRESHOLD(SPI_InitStruct->SPI_TXFIFOEmptyThreshold));

    /* DISABLE current SPI before configure CONTROL registers */
    SPIx->SSIENR = 0;

    SPIx->IMR = 0;

    SPIx->CTRLR0 = (uint16_t)(SPI_InitStruct->SPI_Direction | SPI_InitStruct->SPI_CPOL | SPI_InitStruct->SPI_CPHA | SPI_InitStruct->SPI_DataSize);

    SPIx->BAUDR = SPI_InitStruct->SPI_BaudRatePrescaler;

    SPIx->SER = SPI_InitStruct->SPI_NSS;

    SPIx->RXFTLR = SPI_InitStruct->SPI_RXFIFOFullThreshold;
    SPIx->TXFTLR = SPI_InitStruct->SPI_TXFIFOEmptyThreshold;

    SPI_MasterSlaveModeSet(SPIx);
}

/**
  * @brief  Initializes each SPI_InitStruct member with its default value.
  * @param  SPI_InitStruct: pointer to a SPI_InitTypeDef structure which will be initialized.
  * @retval None
  */
void SPI_StructInit(SPI_InitTypeDef *SPI_InitStruct)
{
    /* Initialize the SPI_Direction member */
    SPI_InitStruct->SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    /* initialize the SPI_DataSize member */
    SPI_InitStruct->SPI_DataSize = SPI_DataSize_8b;
    /* Initialize the SPI_CPOL member */
    SPI_InitStruct->SPI_CPOL = SPI_CPOL_Low;
    /* Initialize the SPI_CPHA member */
    SPI_InitStruct->SPI_CPHA = SPI_CPHA_1Edge;
    /* Initialize the SPI_NSS member */
    SPI_InitStruct->SPI_NSS = SPI_NSS_0;
    /* Initialize the SPI_BaudRatePrescaler member */
    SPI_InitStruct->SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    /* Initialize the SPI_RXFIFOFullThreshold member */
    SPI_InitStruct->SPI_RXFIFOFullThreshold = SPI_RXFIFOFullThreshold_1;
    /* Initialize the SPI_TXFIFOEmptyThreshold member */
    SPI_InitStruct->SPI_TXFIFOEmptyThreshold = SPI_TXFIFOEmptyThreshold_0;
}

/**
  * @brief  Enable or disable the SPI peripheral
  * @param  UARTx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0, SPIM1, SPIM2, SPIM3 or SPIM4
  * @param  NewState: new state of the SPIx peripheral
  *         This parameter can be ENABLE or DISABLE
  * @retval None
  */
void SPI_Cmd(SPI_TypeDef *SPIx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE) {
        /* Enable the selected SPI peripheral */
        SPIx->SSIENR = 1;
    } else {
        /* Disable the selected SPI peripheral */
        SPIx->SSIENR = 0;
    }
}

/**
  * @brief  Initializes the SPIx DMA interface according to the specified parameters.
  * @param  SPIx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0, SPIM1, SPIM2, SPIM3 or SPIM4
  * @param  SPI_DMAInitStruct: pointer to a SPI_DMAInitStruct structure that contains the configuration information.
  * @retval None
  */
void SPI_DMAInit(SPI_TypeDef *SPIx, SPI_DMAInitTypeDef *SPI_DMAInitStruct)
{
    /* Check the parameters */
    assert_param(IS_SPI_DMAREQ(SPI_DMAInitStruct->SPI_DMAReq));
    assert_param(IS_SPI_DMA_RECEIVE_LEVEL(SPI_DMAInitStruct->SPI_DMAReceiveLevel));
    assert_param(IS_SPI_DMA_TRANSMIT_LEVEL(SPI_DMAInitStruct->SPI_DMATransmitLevel));
    assert_param(IS_FUNCTIONAL_STATE(SPI_DMAInitStruct->SPI_DMAEnCmd));

    SPIx->DMARDLR = SPI_DMAInitStruct->SPI_DMAReceiveLevel;
    SPIx->DMATDLR = SPI_DMAInitStruct->SPI_DMATransmitLevel;
    if (DISABLE != SPI_DMAInitStruct->SPI_DMAEnCmd) {
        SPIx->DMACR |= SPI_DMAInitStruct->SPI_DMAReq;
    } else {
        SPIx->DMACR &= ~SPI_DMAInitStruct->SPI_DMAReq;
    }
}

/**
  * @brief  Initializes each SPI_DMAInitStruct member with its default value.
  * @param  SPI_DMAInitStruct: pointer to a SPI_DMAInitTypeDef structure which will be initialized.
  * @retval None
  */
void SPI_DMAStructInit(SPI_DMAInitTypeDef *SPI_DMAInitStruct)
{
    SPI_DMAInitStruct->SPI_DMAReq = SPI_DMAReq_Rx | SPI_DMAReq_Tx;
    /* Initialize the SPI_DMAReceiveLevel member */
    SPI_DMAInitStruct->SPI_DMAReceiveLevel = SPI_DMAReceiveLevel_8;
    /* Initialize the SPI_DMATransmitLevel member */
    SPI_DMAInitStruct->SPI_DMATransmitLevel = SPI_DMATransmitLevel_8;

    SPI_DMAInitStruct->SPI_DMAEnCmd = DISABLE;
}

/**
  * @brief  Enable or disable the SPI DMA interface
  * @param  UARTx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0, SPIM1, SPIM2, SPIM3 or SPIM4
  * @param  SPI_DMAReq: SPI DMA transfer requests
  *         This parameter can be SPI_DMAReq_Rx or SPI_DMAReq_Tx
  * @param  NewState: new state of the DMA interface
  *         This parameter can be ENABLE or DISABLE
  * @retval None
  */
void SPI_DMACmd(SPI_TypeDef *SPIx, uint32_t SPI_DMAReq, FunctionalState NewState)
{
    assert_param(IS_SPI_DMAREQ(SPI_DMAReq));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (DISABLE != NewState) {
        SPIx->DMACR |= SPI_DMAReq;
    } else {
        SPIx->DMACR &= ~SPI_DMAReq;
    }
}

/**
  * @brief  Enable or disable the specified SPI interrupt.
  * @param  SPIx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0, SPIM1, SPIM2, SPIM3 or SPIM4
  * @param  SPI_IT: specifies the SPI interrupt sources
  *         This parameter can be one of the following values:
  *     @arg SPI_IT_TXE:
  *     @arg SPI_IT_TXOVF:
  *     @arg SPI_IT_RXF:
  *     @arg SPI_IT_RXOVF:
  *     @arg SPI_IT_RXUDF:
  *     @arg SPI_IT_MMC:
  * @param  NewState: new state of the specified SPI interrupt
  *         This parameter can be ENABLE or DISABLE
  * @retval None
  */
void SPI_ITConfig(SPI_TypeDef *SPIx, uint32_t SPI_IT, FunctionalState NewState)
{
    assert_param(IS_SPI_GET_IT(SPI_IT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE) {
        SPIx->IMR |= SPI_IT;
    } else {
        SPIx->IMR &= ~SPI_IT;
    }
}

/**
  * @brief  Transmits single data through the SPIx peripheral.
  * @param  SPIx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0, SPIM1, SPIM2, SPIM3 or SPIM4
  * @param  data: the data to be transmitted
  * @retval None
  */
void SPI_SendData(SPI_TypeDef *SPIx, uint16_t Data)
{
    /* Write in the DR register the data to be sent */
    SPIx->DR = Data;
}

/**
  * @brief  Receive single data through the SPIx peripheral.
  * @param  SPIx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0, SPIM1, SPIM2, SPIM3 or SPIM4
  * @retval The received data
  */
uint16_t SPI_ReceiveData(SPI_TypeDef *SPIx)
{
    /* Return the data in the DR register */
    return (uint16_t)SPIx->DR;
}

/**
  * @brief  Configure the data size for selected SPI
  * @param  SPIx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0, SPIM1, SPIM2, SPIM3 or SPIM4
  * @param  SPI_DataSize: specifies the SPI data size
  *         This parameter can be one of the following values:
  *     @arg SPI_DataSize_8b: set data frame format to 8bit
  *     @arg SPI_DataSize_16b: set data frame format to 16bit
  * @retval None
  */
void SPI_DataSizeConfig(SPI_TypeDef *SPIx, uint32_t SPI_DataSize)
{
    assert_param(IS_SPI_DATASIZE(SPI_DataSize));

    SPIx->CTRLR0 &= ~SPI_CTRLR0_DFS;
    SPIx->CTRLR0 |= SPI_DataSize;
}

/**
  * @brief  Configure the data direction for selected SPI
  * @param  SPIx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0, SPIM1, SPIM2, SPIM3 or SPIM4
  * @param  SPI_Direction: specifies the SPI data direction
  *         This parameter can be one of the following values:
  *     @arg SPI_Direction_2Lines_FullDuplex
  *     @arg SPI_Direction_1Line_Tx
  *     @arg SPI_Direction_1Line_Rx
  *     @arg SPI_Direction_EEPROM_Read
  * @retval None
  */
void SPI_BiDirectionalLineConfig(SPI_TypeDef *SPIx, uint32_t SPI_Direction)
{
    assert_param(IS_SPI_DIRECTION_MODE(SPI_Direction));

    SPIx->CTRLR0 &= ~SPI_CTRLR0_TMOD;
    SPIx->CTRLR0 |= SPI_Direction;
}

/**
  * @brief  Check whether the specified SPI interrupt has occurred or not
  * @param  SPIx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0, SPIM1, SPIM2, SPIM3 or SPIM4
  * @param  SPI_IT: specifies the SPI interrupt sources
  *         This parameter can be one of the following values:
  *     @arg SPI_IT_TXE
  *     @arg SPI_IT_TXOVF
  *     @arg SPI_IT_RXF
  *     @arg SPI_IT_RXOVF
  *     @arg SPI_IT_RXUDF
  *     @arg SPI_IT_MMC
  * @retval The new state of SPI_IT (SET or RESET)
  */
ITStatus SPI_GetITStatus(SPI_TypeDef *SPIx, uint32_t SPI_IT)
{
    assert_param(IS_SPI_GET_IT(SPI_IT));

    if ((SPIx->ISR & SPI_IT) != RESET) {
        return SET;
    }

    return RESET;
}

/**
  * @brief  Clear the specified SPI interrupt pending bit
  * @param  SPIx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0, SPIM1, SPIM2, SPIM3 or SPIM4
  * @param  SPI_IT: specifies the SPI interrupt pending bit to clear
  *         This parameter can be one of the following values:
  *     @arg SPI_IT_TXE
  *     @arg SPI_IT_TXOVF
  *     @arg SPI_IT_RXF
  *     @arg SPI_IT_RXOVF
  *     @arg SPI_IT_RXUDF
  *     @arg SPI_IT_MMC
  * @retval None
  */
void SPI_ClearITPendingBit(SPI_TypeDef *SPIx, uint32_t SPI_IT)
{
    assert_param(IS_SPI_GET_IT(SPI_IT));

    if (SPI_IT_MMC | SPI_IT) {
        SPIx->MSTICR;
    }
    if (SPI_IT_RXUDF | SPI_IT) {
        SPIx->RXUICR;
    }
    if (SPI_IT_RXOVF | SPI_IT) {
        SPIx->RXOICR;
    }
    if (SPI_IT_TXOVF | SPI_IT) {
        SPIx->TXOICR;
    }
}

FlagStatus SPI_GetFlagStatus(SPI_TypeDef *SPIx, uint32_t SPI_FLAG)
{
    assert_param(IS_SPI_GET_FLAG(SPI_FLAG));

    if ((SPIx->SR & SPI_FLAG) != RESET) {
        return SET;
    }

    return RESET;
}

uint32_t SPI_GetFlagStatusReg(SPI_TypeDef *SPIx)
{
    return ((uint32_t)SPIx->SR);
}

FlagStatus SPI_IsBusy(SPI_TypeDef *SPIx)
{
    if ((SPIx->SR & SPI_SR_BUSY) != RESET) {
        return SET;
    }

    return RESET;
}

FlagStatus SPI_IsTXErr(SPI_TypeDef *SPIx)
{
    if ((SPIx->SR & SPI_SR_TXE) != RESET) {
        return SET;
    }

    return RESET;
}

FlagStatus SPI_IsDataCollisionErr(SPI_TypeDef *SPIx)
{
    if ((SPIx->SR & SPI_SR_DCOL) != RESET) {
        return SET;
    }

    return RESET;
}

/**
  * @brief  Initializes the SPIx SSP interface according to the specified parameters.
  * @param  SPIx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0, SPIM1, SPIM2, SPIM3 or SPIM4
  * @param  SSP_InitStruct: pointer to a SSP_InitTypeDef structure that contains the configuration information.
  * @retval None
  */
void SSP_Init(SPI_TypeDef *SPIx, SSP_InitTypeDef *SSP_InitStruct)
{
    assert_param(IS_SPI_DIRECTION_MODE(SSP_InitStruct->SSP_Direction));
    assert_param(IS_SPI_DATASIZE(SSP_InitStruct->SSP_DataSize));
    assert_param(IS_SPI_NSS(SSP_InitStruct->SSP_NSS));
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SSP_InitStruct->SSP_BaudRatePrescaler));
    assert_param(IS_SPI_RX_FIFO_FULL_THRESHOLD(SSP_InitStruct->SSP_RXFIFOFullThreshold));
    assert_param(IS_SPI_TX_FIFO_EMPTY_THRESHOLD(SSP_InitStruct->SSP_TXFIFOEmptyThreshold));

    /* DISABLE current SPI before configure CONTROL registers */
    SPIx->SSIENR = 0;

    SPIx->RXFTLR = SSP_InitStruct->SSP_RXFIFOFullThreshold;
    SPIx->TXFTLR = SSP_InitStruct->SSP_TXFIFOEmptyThreshold;

    SPIx->CTRLR0 = (uint16_t)(SSP_InitStruct->SSP_Direction | SSP_InitStruct->SSP_DataSize);

    SPIx->BAUDR = SSP_InitStruct->SSP_BaudRatePrescaler;

    SPIx->SER = SSP_InitStruct->SSP_NSS;

    /* SSI set to SSP mode */
    SPIx->CTRLR0 |= SPI_CTRLR0_FRF_0;

    SPI_MasterSlaveModeSet(SPIx);
}

/**
  * @brief  Initializes each SSP_InitStruct member with its default value.
  * @param  SSP_InitStruct: pointer to a SSP_InitTypeDef structure which will be initialized.
  * @retval None
  */
void SSP_StructInit(SSP_InitTypeDef *SSP_InitStruct)
{
    /* Initialize the SSP_Direction member */
    SSP_InitStruct->SSP_Direction = SPI_Direction_2Lines_FullDuplex;
    /* initialize the SSP_DataSize member */
    SSP_InitStruct->SSP_DataSize = SPI_DataSize_8b;
    /* Initialize the SSP_NSS member */
    SSP_InitStruct->SSP_NSS = SPI_NSS_0;
    /* Initialize the SSP_BaudRatePrescaler member */
    SSP_InitStruct->SSP_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    /* Initialize the SSP_RXFIFOFullThreshold member */
    SSP_InitStruct->SSP_RXFIFOFullThreshold = SPI_RXFIFOFullThreshold_1;
    /* Initialize the SSP_TXFIFOEmptyThreshold member */
    SSP_InitStruct->SSP_TXFIFOEmptyThreshold = SPI_TXFIFOEmptyThreshold_0;
}

/**
  * @brief  Initializes the SPIx NSM interface according to the specified parameters.
  * @param  SPIx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0, SPIM1, SPIM2, SPIM3 or SPIM4
  * @param  NSM_InitStruct: pointer to a NSM_InitTypeDef structure that contains the configuration information.
  * @retval None
  */
void NSM_Init(SPI_TypeDef *SPIx, NSM_InitTypeDef *NSM_InitStruct)
{
    assert_param(IS_NSM_DIRECTION_MODE(NSM_InitStruct->NSM_Direction));
    assert_param(IS_NSM_TRANSFER_MODE(NSM_InitStruct->NSM_TransferMode));
    assert_param(IS_NSM_DATASIZE(NSM_InitStruct->NSM_DataSize));
    assert_param(IS_NSM_CONTROL_DATASIZE(NSM_InitStruct->NSM_ControlDataSize));
    assert_param(IS_SPI_NSS(NSM_InitStruct->NSM_NSS));
    assert_param(IS_SPI_BAUDRATE_PRESCALER(NSM_InitStruct->NSM_BaudRatePrescaler));
    assert_param(IS_FUNCTIONAL_STATE(NSM_InitStruct->NSM_HandShakingCmd));
    assert_param(IS_SPI_RX_FIFO_FULL_THRESHOLD(NSM_InitStruct->NSM_RXFIFOFullThreshold));
    assert_param(IS_SPI_TX_FIFO_EMPTY_THRESHOLD(NSM_InitStruct->NSM_TXFIFOEmptyThreshold));

    /* DISABLE current SPI before configure CONTROL registers */
    SPIx->SSIENR = 0;

    SPIx->RXFTLR = NSM_InitStruct->NSM_RXFIFOFullThreshold;
    SPIx->TXFTLR = NSM_InitStruct->NSM_TXFIFOEmptyThreshold;

    SPIx->CTRLR0 = (uint16_t)((NSM_InitStruct->NSM_ControlDataSize) << 12 | NSM_InitStruct->NSM_DataSize);

    SPIx->BAUDR = NSM_InitStruct->NSM_BaudRatePrescaler;

    SPIx->SER = NSM_InitStruct->NSM_NSS;

    if (DISABLE != NSM_InitStruct->NSM_HandShakingCmd) {
        SPIx->MWCR |= SPI_MWCR_MHS;
    } else {
        SPIx->MWCR &= ~SPI_MWCR_MHS;
    }

    if (NSM_Direction_Data_Transmit == NSM_InitStruct->NSM_Direction) {
        SPIx->MWCR |= SPI_MWCR_MDD;
    } else if (NSM_Direction_Data_Receive == NSM_InitStruct->NSM_Direction) {
        SPIx->MWCR &= ~SPI_MWCR_MDD;
    }

    if (NSM_TransferMode_Sequential == NSM_InitStruct->NSM_TransferMode) {
        SPIx->MWCR |= SPI_MWCR_MWMOD;
    } else if (NSM_TransferMode_Non_Sequential == NSM_InitStruct->NSM_TransferMode) {
        SPIx->MWCR &= ~SPI_MWCR_MWMOD;
    }

    /* SSI set to NSM mode */
    SPIx->CTRLR0 |= SPI_CTRLR0_FRF_1;

    SPI_MasterSlaveModeSet(SPIx);
}

/**
  * @brief  Initializes each NSM_InitStruct member with its default value.
  * @param  NSM_InitStruct: pointer to a NSM_InitTypeDef structure which will be initialized.
  * @retval None
  */
void NSM_StructInit(NSM_InitTypeDef *NSM_InitStruct)
{
    /* Initialize the NSM_Direction member */
    NSM_InitStruct->NSM_Direction = NSM_Direction_Data_Receive;
    /* Initialize the NSM_Direction member */
    NSM_InitStruct->NSM_TransferMode = NSM_TransferMode_Non_Sequential;
    /* initialize the NSM_DataSize member */
    NSM_InitStruct->NSM_DataSize = NSM_DataSize_8b;
    /* initialize the NSM_DataSize member */
    NSM_InitStruct->NSM_ControlDataSize = NSM_ControlDataSize_8b;
    /* Initialize the NSM_NSS member */
    NSM_InitStruct->NSM_NSS = SPI_NSS_0;
    /* Initialize the NSM_BaudRatePrescaler member */
    NSM_InitStruct->NSM_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    /* Initialize the NSM_HandShakingCmd */
    NSM_InitStruct->NSM_HandShakingCmd = DISABLE;
    /* Initialize the NSM_RXFIFOFullThreshold member */
    NSM_InitStruct->NSM_RXFIFOFullThreshold = SPI_RXFIFOFullThreshold_1;
    /* Initialize the NSM_TXFIFOEmptyThreshold member */
    NSM_InitStruct->NSM_TXFIFOEmptyThreshold = SPI_TXFIFOEmptyThreshold_0;
}

/**
  * @brief  Configure SPI Mode
  * @param  SPIx: Select the SPI peripheral.
  *         This parameter can be one of the following values:
  *         SPIS0, SPIM0
  * @retval None
  */
static void SPI_MasterSlaveModeSet(SPI_TypeDef *SPIx)
{
    if (SPIM0 == SPIx) {
        SYSCTRL->PHER_CTRL &= ~SYSCTRL_PHER_CTRL_SPI0_SLV_EN;
    } else if (SPIS0 == SPIx) {
        SYSCTRL->PHER_CTRL |= SYSCTRL_PHER_CTRL_SPI0_SLV_EN;
    }
}

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
