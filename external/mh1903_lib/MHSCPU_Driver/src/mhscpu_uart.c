/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_uart.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the UART firmware functions
 *****************************************************************************/

/* Includes ----------------------------------------------------------------*/
#include "mhscpu_uart.h"


/* Exported functions -------------------------------------------------------*/
/**
  * @brief  Deinitializes the UARTx peripheral registers to their default reset values.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the following values:
  *         UART0, UART1, UART2 or UART3.
  * @retval None
  */
void UART_DeInit(UART_TypeDef *UARTx)
{
    /* Check the parameters */
    assert_param(IS_UART_PERIPH(UARTx));

    if (UARTx == UART0) {
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_UART0, ENABLE);
    } else if (UARTx == UART1) {
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_UART1, ENABLE);
    } else if (UARTx == UART2) {
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_UART2, ENABLE);
    } else if (UARTx == UART3) {
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_UART3, ENABLE);
    }
}

/**
  * @brief  Initializes the UARTx peripheral according to the specified parameters.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the following values:
  *         UART0, UART1, UART2 or UART3.
  * @param  UART_InitStruct: pointer to a UART_InitTypeDef structure that contains the configuration information.
  * @retval None
  */
void UART_Init(UART_TypeDef *UARTx, UART_InitTypeDef *UART_InitStruct)
{
    uint32_t tmpBaudRateDiv;
    SYSCTRL_ClocksTypeDef clocks;

    /* Check the parameters */
    assert_param(IS_UART_PERIPH(UARTx));
    assert_param(IS_UART_BAUDRATE(UART_InitStruct->UART_BaudRate));
    assert_param(IS_UART_WORD_LENGTH(UART_InitStruct->UART_WordLength));
    assert_param(IS_UART_STOPBITS(UART_InitStruct->UART_StopBits, UART_InitStruct->UART_WordLength));
    assert_param(IS_UART_PARITY(UART_InitStruct->UART_Parity));

    /* get clock frequence */
    SYSCTRL_GetClocksFreq(&clocks);

    /* LCR = 1 */
    UARTx->LCR |= UART_LCR_DLAB;

    // baud rate = (serial clock freq) / (16 * divisor).
    tmpBaudRateDiv = ((clocks.PCLK_Frequency / 16) + UART_InitStruct->UART_BaudRate / 2) / UART_InitStruct->UART_BaudRate;
    UARTx->OFFSET_0.DLL = (tmpBaudRateDiv & 0x00FF);
    UARTx->OFFSET_4.DLH = ((tmpBaudRateDiv >> 8) & 0x00FF);

    /* LCR = 0 */
    UARTx->LCR &= ~UART_LCR_DLAB;
    UARTx->LCR = UART_InitStruct->UART_WordLength | UART_InitStruct->UART_StopBits | UART_InitStruct->UART_Parity;
}

/**
  * @brief  Initializes each UART_InitStruct member with its default value.
  * @param  UART_InitStruct: pointer to a UART_InitTypeDef structure which will be initialized.
  * @retval None
  */
void UART_StructInit(UART_InitTypeDef *UART_InitStruct)
{
    UART_InitStruct->UART_BaudRate = 9600;
    UART_InitStruct->UART_WordLength = UART_WordLength_8b;
    UART_InitStruct->UART_StopBits = UART_StopBits_1;
    UART_InitStruct->UART_Parity = UART_Parity_No ;
}

/**
  * @brief  Enable or disable the specified UART interrupt.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the following values:
  *         UART0, UART1, UART2 or UART3.
  * @param  UART_IT: specifies the UART interrupt sources
  *         This parameter can be one of the following values:
  *     @arg UART_IT_RX_RECVD: Receive data available interrupt
  *     @arg UART_IT_TX_EMPTY: Transmitter holding register Empty at/below threshold
  *     @arg UART_IT_LINE_STATUS: Error interrupt(Overrun/parity/framing error)
  *     @arg UART_IT_MODEM_STATUS: Modem status
  * @param  NewState: new state of the specified UART interrupt
  *         This parameter can be ENABLE or DISABLE
  * @retval None
  */
void UART_ITConfig(UART_TypeDef *UARTx, uint32_t UART_IT, FunctionalState NewState)
{
    if (DISABLE != NewState) {
        UARTx->OFFSET_4.IER |= UART_IT;
    } else {
        UARTx->OFFSET_4.IER &= ~UART_IT;
    }
}

/**
  * @brief  Transmits single data through the UARTx peripheral.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the following values:
  *         UART0, UART1, UART2 or UART3.
  * @param  data: the data to be transmitted
  * @retval None
  */
void UART_SendData(UART_TypeDef *UARTx, uint8_t Data)
{
    UARTx->OFFSET_0.THR = (Data & 0xFF);
}

/**
  * @brief  Receive single data through the UARTx peripheral.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the following values:
  *         UART0, UART1, UART2 or UART3.
  * @retval The received data
  */
uint8_t UART_ReceiveData(UART_TypeDef *UARTx)
{
    return (uint8_t)(UARTx->OFFSET_0.RBR & 0xFF);
}

/**
  * @brief  Enable or disable the specified UART FlowCtrl
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the following values:
  *         UART0, UART1, UART2 or UART3.
  * @param  NewState: new state of the specified UART flowctrl
  *         This parameter can be ENABLE or DISABLE
  * @retval None
  */
void UART_AutoFlowCtrlCmd(UART_TypeDef *UARTx, FunctionalState NewState)
{
    if (DISABLE != NewState) {
        UARTx->MCR |= UART_MCR_AFCE;
    } else {
        UARTx->MCR &= ~UART_MCR_AFCE;
    }
}

void UART_SetDTR(UART_TypeDef *UARTx)
{
    UARTx->MCR |= UART_MCR_DTR;
}

void UART_ResetDTR(UART_TypeDef *UARTx)
{
    UARTx->MCR &= ~UART_MCR_DTR;
}

void UART_SetRTS(UART_TypeDef *UARTx)
{
    UARTx->MCR |= UART_MCR_RTS;
}

void UART_ResetRTS(UART_TypeDef *UARTx)
{
    UARTx->MCR &= ~UART_MCR_RTS;
}

/**
  * @brief  Initializes the UARTx peripheral FIFO function according to the specified parameters.
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the following values:
  *         UART0, UART1, UART2 or UART3.
  * @param  UART_FIFOInitStruct: pointer to a UART_FIFOInitTypeDef structure that contains the configuration information.
  * @retval None
  */
void UART_FIFOInit(UART_TypeDef *UARTx, UART_FIFOInitTypeDef *UART_FIFOInitStruct)
{
    /**************************  FIFO Tx Interrupt Config ******************************/
    if (DISABLE != UART_FIFOInitStruct->FIFO_TX_TriggerIntEnable) {
        UARTx->OFFSET_4.IER |= UART_IER_PTIME;
    } else {
        UARTx->OFFSET_4.IER &= ~UART_IER_PTIME;
    }

    /**************************  FIFO Config ******************************/
    /* FCR Write Only So Here we Use FCR Shadow Register SDMAM(WR) */
    if (UARTx->SFE | UART_SFE_SFE) {
        UARTx->SFE &= ~UART_SFE_SFE;
    }

    if (UART_FIFO_DMA_Mode_0 == UART_FIFOInitStruct->FIFO_DMA_Mode) {
        UARTx->SDMAM &= ~UART_SDMAM_SDMAM;
    } else if (UART_FIFO_DMA_Mode_1 == UART_FIFOInitStruct->FIFO_DMA_Mode) {
        UARTx->SDMAM |= UART_SDMAM_SDMAM;
    }

    /* FCR Write Only So Here we Use FCR Shadow Register SRT and STET(WR) */
    UARTx->SRT = UART_FIFOInitStruct->FIFO_RX_Trigger;
    UARTx->STET = UART_FIFOInitStruct->FIFO_TX_Trigger;

    if (DISABLE != UART_FIFOInitStruct->FIFO_Enable) {
        UARTx->SFE |= UART_SFE_SFE;
    } else {
        UARTx->SFE &= ~UART_SFE_SFE;
    }
}

/**
  * @brief  Initializes each UART_FIFOInitStruct member with its default value.
  * @param  UART_FIFOInitStruct: pointer to a UART_FIFOInitTypeDef structure which will be initialized.
  * @retval None
  */
void UART_FIFOStructInit(UART_FIFOInitTypeDef *UART_FIFOInitStruct)
{
    UART_FIFOInitStruct->FIFO_Enable = DISABLE;
    UART_FIFOInitStruct->FIFO_DMA_Mode = UART_FIFO_DMA_Mode_0;
    UART_FIFOInitStruct->FIFO_RX_Trigger = UART_FIFO_RX_Trigger_1_2_Full;
    UART_FIFOInitStruct->FIFO_TX_Trigger = UART_FIFO_TX_Trigger_1_2_Full;
    UART_FIFOInitStruct->FIFO_TX_TriggerIntEnable = DISABLE;
}

/**
  * @brief  Reset UARTx peripheral TX or RX FIFO
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the following values:
  *         UART0, UART1, UART2 or UART3.
  * @param  UART_FIFO: specifies the FIFO to be reset.
  *         This parameter can be one of the following values:
  *         UART_FIFO_TX or UART_FIFO_RX
  * @retval None
  */
void UART_FIFOReset(UART_TypeDef *UARTx, uint32_t UART_FIFO)
{
    uint32_t u32tmp = 0;

    if (0 != (UART_FIFO & UART_FIFO_TX)) {
        u32tmp |= UART_SRR_XFR;
    }

    if (0 != (UART_FIFO & UART_FIFO_RX)) {
        u32tmp |= UART_SRR_RFR;
    }

    /* FCR Write Only So Here we Use FCR Shadow Register SRR to protect FCR's value*/
    UARTx->SRR = u32tmp;
}

/**
  * @brief  Enable or disable the specified UART FIFO
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the following values:
  *         UART0, UART1, UART2 or UART3.
  * @param  NewState: new state of the UART FIFO State
  *         This parameter can be ENABLE or DISABLE
  * @retval None
  */
void UART_FIFOCmd(UART_TypeDef *UARTx, FunctionalState NewState)
{
    /* FCR Write Only So Here we Use FCR Shadow Register SFE(WR) */
    if (DISABLE != NewState) {
        UARTx->SFE |= UART_SFE_SFE;
    } else {
        UARTx->SFE &= ~UART_SFE_SFE;
    }
}

uint32_t UART_GetLineStatus(UART_TypeDef *UARTx)
{
    return (UARTx->LSR);
}

uint32_t UART_GetModemStatus(UART_TypeDef *UARTx)
{
    return (UARTx->MSR);
}

uint32_t UART_GetITIdentity(UART_TypeDef *UARTx)
{
    return (UARTx->OFFSET_8.IIR);
}

Boolean UART_IsRXFIFOError(UART_TypeDef *UARTx)
{
    if (0 != (UARTx->LSR & UART_LINE_STATUS_RX_FIFO_ERROR)) {
        return TRUE;
    }

    return FALSE;
}

Boolean UART_IsRXFramingError(UART_TypeDef *UARTx)
{
    if (0 != (UARTx->LSR & UART_LINE_STATUS_RX_FRAMING_ERROR)) {
        return TRUE;
    }

    return FALSE;
}

Boolean UART_IsRXParityError(UART_TypeDef *UARTx)
{
    if (0 != (UARTx->LSR & UART_LINE_STATUS_RX_PARITY_ERROR))
        return TRUE;
    else
        return FALSE;
}

Boolean UART_IsRXOverrunError(UART_TypeDef *UARTx)
{
    if (0 != (UARTx->LSR & UART_LINE_STATUS_RX_OVERRUN_ERROR)) {
        return TRUE;
    }

    return FALSE;
}

Boolean UART_IsRXReceived(UART_TypeDef *UARTx)
{
    if (0 != (UARTx->LSR & UART_LINE_STATUS_RX_RECVD)) {
        return TRUE;
    }

    return FALSE;
}

Boolean UART_IsTXEmpty(UART_TypeDef *UARTx)
{
    if (0 != (UARTx->LSR & UART_LINE_STATUS_TX_EMPTY)) {
        return TRUE;
    }

    return FALSE;
}

Boolean UART_IsTXHoldingRegisterEmpty(UART_TypeDef *UARTx)
{
    if (0 != (UARTx->LSR & UART_LINE_STATUS_TX_HOLDING_REGISTER_EMPTY)) {
        return TRUE;
    }

    return FALSE;
}

Boolean UART_IsTXFIFOTrigger(UART_TypeDef *UARTx)
{
    if (0 != (UARTx->LSR & UART_LINE_STATUS_TX_HOLDING_REGISTER_EMPTY)) {
        return TRUE;
    }

    return FALSE;
}


Boolean UART_IsRXFIFOFull(UART_TypeDef *UARTx)
{
    if (0 != (UARTx->USR & UART_STATUS_RX_FIFO_FULL)) {
        return TRUE;
    }

    return FALSE;
}

Boolean UART_IsRXFIFONotEmpty(UART_TypeDef *UARTx)
{
    if (0 != (UARTx->USR & UART_STATUS_RX_FIFO_NOT_EMPTY)) {
        return TRUE;
    }

    return FALSE;
}

Boolean UART_IsTXFIFOEmpty(UART_TypeDef *UARTx)
{
    if (0 != (UARTx->USR & UART_STATUS_TX_FIFO_EMPTY)) {
        return TRUE;
    }

    return FALSE;
}

Boolean UART_IsTXFIFONotFull(UART_TypeDef *UARTx)
{
    if (0 != (UARTx->USR & UART_STATUS_TX_FIFO_NOT_FULL)) {
        return TRUE;
    }

    return FALSE;
}

Boolean UART_IsBusy(UART_TypeDef *UARTx)
{
    if (0 != (UARTx->USR & UART_STATUS_BUSY)) {
        return TRUE;
    }

    return FALSE;
}

void UART_DMAGenerateSoftAck(UART_TypeDef *UARTx)
{
    UARTx->DMASA |= UART_DMASA_DMASA;
}

void UART_TXHaltCmd(UART_TypeDef *UARTx, FunctionalState NewStatus)
{
    if (DISABLE != NewStatus) {
        UARTx->HTX |= UART_HTX_HTX;
    } else {
        UARTx->HTX &= ~UART_HTX_HTX;
    }
}

void UART_FIFOAccessModeCmd(UART_TypeDef *UARTx, FunctionalState NewStatus)
{
    if (DISABLE != NewStatus) {
        UARTx->FAR |= UART_FAR_FAR;
    } else {
        UARTx->FAR &= ~UART_FAR_FAR;
    }
}

uint8_t UART_FIFOTxRead(UART_TypeDef *UARTx)
{
    return (uint8_t)(UARTx->TFR & 0xFF);
}

/**
  * @brief  Enable or disable the UART's IrDA interface
  * @param  UARTx: Select the UART peripheral.
  *         This parameter can be one of the following values:
  *         UART0, UART1, UART2 or UART3.
  * @param  NewState: new state of the IrDA interface
  *         This parameter can be ENABLE or DISABLE
  * @retval None
  */
void UART_IrDACmd(UART_TypeDef *UARTx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE) {
        /* Enable the IrDA mode by setting the SIRE bit in the MCR register */
        UARTx->MCR |= UART_MCR_SIRE;
    } else {
        /* Disable the IrDA mode by clearing the SIRE bit in the MCR register */
        UARTx->MCR &= UART_MCR_SIRE;
    }
}

void UART_SendBreak(UART_TypeDef *UARTx)
{
    /* Send break characters */
    UARTx->SBCR |= UART_SBCR_SBCR;
}

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
