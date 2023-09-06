/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_i2c.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the I2C firmware functions
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "mhscpu_i2c.h"

#define I2C_SPEED_STANDARD_MAX_FREQ                         (100000)
#define I2C_SPEED_FAST_MAX_FREQ                             (400000)
#define I2C_SPEED_HIGH_MAX_FREQ                             (3400000)

#define I2C_IC_CON_SPEED_Mask                               (I2C_IC_CON_SPEED_0 | I2C_IC_CON_SPEED_1)
#define I2C_IC_CON_SPEED_STANDARD                           (I2C_IC_CON_SPEED_0)
#define I2C_IC_CON_SPEED_FAST                               (I2C_IC_CON_SPEED_1)
#define I2C_IC_CON_SPEED_HIGH                               (I2C_IC_CON_SPEED_0 | I2C_IC_CON_SPEED_1)

#define I2C_SPEED_FAST_SPIKE_MAX                            (50)        //in fast mode, the max of spikes pulse is 50ns
#define I2C_SPEED_HIGH_SPIKE_MAX                            (10)        //in high mode, the max of spikes pulse is 10ns

#define I2C_SPEED_FAST_SPIKE_Hz_MAX                         (20000000)      //in fast mode, the max of spikes pulse is 20MHz
#define I2C_SPEED_HIGH_SPIKE_Hz_MAX                         (100000000)     //in high mode, the max of spikes pulse is 100MHz

/* Exported functions -------------------------------------------------------*/

/**
  * @brief  Deinitializes the I2Cx peripheral registers to their default reset values.
  * @param  I2Cx: Select the I2C peripheral.
  *         This parameter can be one of the following values:
  *         I2C0
  * @retval None
  */
void I2C_DeInit(I2C_TypeDef *I2Cx)
{
    if (I2C0 == I2Cx) {
        SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_I2C0, ENABLE);
    }
}

/**
  * @brief  Initializes the I2Cx peripheral according to the specified parameters.
  * @param  I2Cx: Select the I2C peripheral.
  *         This parameter can be one of the following values:
  *         I2C0
  * @param  I2C_InitStruct: pointer to a I2C_InitTypeDef structure that contains the configuration information.
  * @retval None
  */
void I2C_Init(I2C_TypeDef *I2Cx, I2C_InitTypeDef *I2C_InitStruct)
{
    uint32_t tmpReg = 0;
    uint32_t tmpSpeed = 0;
    uint32_t tmpClockCount = 0;
    uint32_t tmpDutyCycle = 0;
    uint32_t tmpDutyCycleTlow = 0;
    uint32_t tmpDutyCycleThigh = 0;
    uint32_t tmpSpikeCountSpeedFast = 0;
    uint32_t tmpSpikeCountSpeedHigh = 0;
    SYSCTRL_ClocksTypeDef  clocks;

    assert_param(IS_I2C_MODE(I2C_InitStruct->I2C_Mode));
    assert_param(IS_I2C_SDA_SETUP_TIME(I2C_InitStruct->I2C_SDASetupTime));
    assert_param(IS_I2C_SDA_HOLD_TIME(I2C_InitStruct->I2C_SDAHoldTime));
    assert_param(IS_I2C_RX_FIFO_FULL_THRESHOLD(I2C_InitStruct->I2C_RXFIFOFullThreshold));
    assert_param(IS_I2C_TX_FIFO_EMPTY_THRESHOLD(I2C_InitStruct->I2C_TXFIFOEmptyThreshold));

    SYSCTRL_GetClocksFreq(&clocks);

    I2Cx->IC_ENABLE = 0;
    /*---------------------------- I2Cx Clock Control Configuration -----------------------*/
    /*                             I2Cx IC_*_SCL_HCNT Configuration                        */
    /*                             I2Cx IC_*_SCL_LCNT Configuration                        */
    /*                             I2Cx IC_*_SPKLEN Configuration                          */
    /*-------------------------------------------------------------------------------------*/
    tmpDutyCycleTlow = (I2C_InitStruct->I2C_DutyCycle & 0xFF00) >> 8;
    tmpDutyCycleThigh = I2C_InitStruct->I2C_DutyCycle & 0x00FF;
    tmpDutyCycle = tmpDutyCycleTlow + tmpDutyCycleThigh;

    tmpClockCount = ((clocks.PCLK_Frequency - 1) + I2C_InitStruct->I2C_ClockSpeed) / I2C_InitStruct->I2C_ClockSpeed;
    tmpSpikeCountSpeedFast = ((clocks.PCLK_Frequency - 1) + I2C_SPEED_FAST_SPIKE_Hz_MAX) / I2C_SPEED_FAST_SPIKE_Hz_MAX;
    tmpSpikeCountSpeedHigh = ((clocks.PCLK_Frequency - 1) + I2C_SPEED_HIGH_SPIKE_Hz_MAX) / I2C_SPEED_HIGH_SPIKE_Hz_MAX;

    if (I2C_InitStruct->I2C_ClockSpeed <= I2C_SPEED_STANDARD_MAX_FREQ) {
        tmpSpeed = I2C_IC_CON_SPEED_STANDARD;
    } else if (I2C_InitStruct->I2C_ClockSpeed <= I2C_SPEED_FAST_MAX_FREQ) {
        tmpSpeed = I2C_IC_CON_SPEED_FAST;
    } else if (I2C_InitStruct->I2C_ClockSpeed <= I2C_SPEED_HIGH_MAX_FREQ) {
        tmpSpeed = I2C_IC_CON_SPEED_HIGH;
    } else {
        tmpSpeed = I2C_IC_CON_SPEED_STANDARD;
    }

    switch (tmpSpeed) {
    case I2C_IC_CON_SPEED_STANDARD:
        // Standard Speed Spike in I2C Bus Specification is N/A (Not Applicable)
        // The following code of Spike setting depand on SCPU I2C IP Specification
        if (tmpSpikeCountSpeedFast) {
            I2Cx->IC_FS_SPKLEN = tmpSpikeCountSpeedFast;
        } else {
            I2Cx->IC_FS_SPKLEN = 1;
        }
        I2Cx->IC_SS_SCL_HCNT = (tmpClockCount + 1) / 2 - I2Cx->IC_FS_SPKLEN;
        I2Cx->IC_SS_SCL_LCNT = (tmpClockCount + 1) / 2;
        break;

    case I2C_IC_CON_SPEED_FAST:
        if (tmpSpikeCountSpeedFast) {
            I2Cx->IC_FS_SPKLEN = tmpSpikeCountSpeedFast;
        } else {
            I2Cx->IC_FS_SPKLEN = 1;
        }
        I2Cx->IC_FS_SCL_HCNT = ((tmpClockCount + tmpDutyCycle) / tmpDutyCycle) * tmpDutyCycleThigh - I2Cx->IC_FS_SPKLEN;
        I2Cx->IC_FS_SCL_LCNT = ((tmpClockCount + tmpDutyCycle) / tmpDutyCycle) * tmpDutyCycleTlow;
        break;

    case I2C_IC_CON_SPEED_HIGH:
        if (tmpSpikeCountSpeedHigh) {
            I2Cx->IC_HS_SPKLEN = tmpSpikeCountSpeedHigh;
        } else {
            I2Cx->IC_HS_SPKLEN = 1;
        }
        I2Cx->IC_HS_SCL_HCNT = ((tmpClockCount + tmpDutyCycle) / tmpDutyCycle) * tmpDutyCycleThigh - I2Cx->IC_HS_SPKLEN;
        I2Cx->IC_HS_SCL_LCNT = ((tmpClockCount + tmpDutyCycle) / tmpDutyCycle) * tmpDutyCycleTlow;
        break;
    }

    /*---------------------------- I2Cx IC_CON Configuration -----------------------*/
    tmpReg = I2Cx->IC_CON;

    tmpReg &= ~I2C_IC_CON_SPEED_Mask;
    tmpReg |= tmpSpeed;

    tmpReg &= ~(I2C_IC_CON_MASTER_MODE | I2C_IC_CON_SLAVE_DISABLE);
    if (I2C_InitStruct->I2C_Mode == I2C_Mode_Master) {
        tmpReg |= (I2C_IC_CON_MASTER_MODE | I2C_IC_CON_SLAVE_DISABLE);
    } else if (I2C_InitStruct->I2C_Mode == I2C_Mode_Slave) {
        tmpReg &= ~(I2C_IC_CON_MASTER_MODE | I2C_IC_CON_SLAVE_DISABLE);
    }

    if (I2C_AcknowledgedAddress_10bit == I2C_InitStruct->I2C_AcknowledgedAddress) {
        tmpReg |= I2C_IC_CON_10BITADDR_SLAVE;
    } else {
        tmpReg &= ~I2C_IC_CON_10BITADDR_SLAVE;
    }

    if (DISABLE != I2C_InitStruct->I2C_GenerateRestartEnable) {
        tmpReg |= I2C_IC_CON_RESTART_EN;
    } else {
        tmpReg &= ~I2C_IC_CON_RESTART_EN;
    }

    I2Cx->IC_CON = tmpReg;


    /*---------------------------- I2Cx IC_SDA_SETUP Configuration -----------------------*/
    I2Cx->IC_SDA_SETUP = (I2C_InitStruct->I2C_SDASetupTime & I2C_IC_SDA_SETUP);

    /*---------------------------- I2Cx IC_SDA_HOLD Configuration -----------------------*/
    I2Cx->IC_SDA_HOLD = (I2C_InitStruct->I2C_SDAHoldTime & I2C_IC_SDA_HOLD);


    /*---------------------------- I2Cx IC_TAR Configuration -----------------------*/
    tmpReg = I2Cx->IC_TAR;

    if (I2C_TargetAddressMode_10bit == I2C_InitStruct->I2C_TargetAddressMode) {
        tmpReg |= I2C_IC_TAR_10BITADDR_MASTER;
        tmpReg &= ~I2C_IC_TAR_TAR;
        tmpReg |= (I2C_InitStruct->I2C_TargetAddress & 0x03FF);
    } else {
        tmpReg &= ~I2C_IC_TAR_10BITADDR_MASTER;
        tmpReg &= ~I2C_IC_TAR_TAR;
        tmpReg |= (I2C_InitStruct->I2C_TargetAddress & 0x007F);
    }

    I2Cx->IC_TAR = tmpReg;

    /*---------------------------- I2Cx IC_SAR Configuration -----------------------*/
    I2Cx->IC_SAR = I2C_InitStruct->I2C_OwnAddress;

    /*---------------------------- I2Cx IC_RX_TL Configuration -----------------------*/
    /*---------------------------- I2Cx IC_TX_TL Configuration -----------------------*/
    I2Cx->IC_RX_TL = I2C_InitStruct->I2C_RXFIFOFullThreshold;
    I2Cx->IC_TX_TL = I2C_InitStruct->I2C_TXFIFOEmptyThreshold;

    /*---------------------------- I2Cx IC_INTR_MASK Configuration -----------------------*/
    I2Cx->IC_INTR_MASK = 0;
}

/**
  * @brief  Initializes each I2C_InitStruct member with its default value.
  * @param  I2C_InitStruct: pointer to a I2C_InitTypeDef structure which will be initialized.
  * @retval None
  */
void I2C_StructInit(I2C_InitTypeDef *I2C_InitStruct)
{
    I2C_InitStruct->I2C_ClockSpeed = I2C_ClockSpeed_100KHz;
    I2C_InitStruct->I2C_Mode = I2C_Mode_Master;
    I2C_InitStruct->I2C_DutyCycle = I2C_DutyCycle_1;
    /* I2C_SDASetupTime = 0x64 is reset value*/
    I2C_InitStruct->I2C_SDASetupTime = 0x64;
    /* I2C_SDAHoldTime = 0x0001 is reset value*/
    I2C_InitStruct->I2C_SDAHoldTime = 0x0001;
    /* I2C_TargetAddress = 0x55 is reset value*/
    I2C_InitStruct->I2C_TargetAddress = 0x55;
    /* I2C_TargetAddressMode = I2C_TargetAddressMode_10bit is reset value*/
    I2C_InitStruct->I2C_TargetAddressMode = I2C_TargetAddressMode_10bit;
    /* I2C_OwnAddress = 0x55 is reset value*/
    I2C_InitStruct->I2C_OwnAddress = 0x55;
    /* I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_10bit is reset value*/
    I2C_InitStruct->I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_10bit;
    I2C_InitStruct->I2C_RXFIFOFullThreshold = I2C_RXFIFOFullThreshold_1;
    I2C_InitStruct->I2C_TXFIFOEmptyThreshold = I2C_TXFIFOEmptyThreshold_0;
    I2C_InitStruct->I2C_GenerateRestartEnable = ENABLE;
}

/**
  * @brief  Configures the specified I2C Target address
  * @param  I2Cx: Select the I2C peripheral.
  *         This parameter can be one of the following values:
  *         I2C0
  * @param  TargetAddress: specifies target address
  * @param  TargetAddressMode: specifies target address mode
  *         This parameter can be one of the following values:
  *         I2C_TargetAddressMode_7bit or I2C_TargetAddressMode_10bit
  * @retval None
  */
void I2C_SetTargetAddress(I2C_TypeDef *I2Cx, uint32_t TargetAddress, uint32_t TargetAddressMode)
{
    uint32_t tmpReg = 0;
    assert_param(IS_I2C_TARGET_ADDRESS_MODE(TargetAddressMode));

    tmpReg = I2Cx->IC_TAR;

    if (I2C_TargetAddressMode_10bit == TargetAddressMode) {
        tmpReg |= I2C_IC_TAR_10BITADDR_MASTER;
        tmpReg &= ~I2C_IC_TAR_TAR;
        tmpReg |= (TargetAddress & 0x03FF);
    } else {
        tmpReg &= ~I2C_IC_TAR_10BITADDR_MASTER;
        tmpReg &= ~I2C_IC_TAR_TAR;
        tmpReg |= (TargetAddress & 0x007F);
    }

    I2Cx->IC_TAR = tmpReg;
}

/**
  * @brief  Enable or disable the specified I2C peripheral.
  * @param  I2Cx: Select the I2C peripheral.
  *         This parameter can be one of the following values:
  *         I2C0
  * @param  NewState: new state of the I2Cx peripheral
  *         This parameter can be ENABLE or DISABLE
  * @retval None
  */
void I2C_Cmd(I2C_TypeDef *I2Cx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE) {
        I2Cx->IC_ENABLE = 1;
    } else {
        I2Cx->IC_ENABLE = 0;
    }
}

/**
  * @brief  Initializes the i2cX DMA interface according to the specified parameters.
  * @param  I2Cx: Select the I2C peripheral.
  *         This parameter can be one of the following values:
  *         I2C0
  * @param  I2C_DMAInitStruct: pointer to a I2C_DMAInitTypeDef structure that contains the configuration information.
  * @retval None
  */
void I2C_DMAInit(I2C_TypeDef *I2Cx, I2C_DMAInitTypeDef *I2C_DMAInitStruct)
{
    /* Check the parameters */
    assert_param(IS_I2C_DMAREQ(I2C_DMAInitStruct->I2C_DMAReq));
    assert_param(IS_I2C_DMA_RECEIVE_LEVEL(I2C_DMAInitStruct->I2C_DMAReceiveLevel));
    assert_param(IS_I2C_DMA_TRANSMIT_LEVEL(I2C_DMAInitStruct->I2C_DMATransmitLevel));
    assert_param(IS_FUNCTIONAL_STATE(I2C_DMAInitStruct->I2C_DMAEnCmd));

    I2Cx->IC_DMA_RDLR = I2C_DMAInitStruct->I2C_DMAReceiveLevel;
    I2Cx->IC_DMA_TDLR = I2C_DMAInitStruct->I2C_DMATransmitLevel;

    if (DISABLE != I2C_DMAInitStruct->I2C_DMAEnCmd) {
        I2Cx->IC_DMA_CR |= I2C_DMAInitStruct->I2C_DMAReq;
    } else {
        I2Cx->IC_DMA_CR &= ~I2C_DMAInitStruct->I2C_DMAReq;
    }
}

/**
  * @brief  Initializes each I2C_DMAInitStruct member with its default value.
  * @param  I2C_DMAInitStruct: pointer to a I2C_DMAInitTypeDef structure which will be initialized.
  * @retval None
  */
void I2C_DMAStructInit(I2C_DMAInitTypeDef *I2C_DMAInitStruct)
{
    I2C_DMAInitStruct->I2C_DMAReq = I2C_DMAReq_Rx | I2C_DMAReq_Tx;
    /* Initialize the I2C_DMAReceiveLevel member */
    I2C_DMAInitStruct->I2C_DMAReceiveLevel = I2C_DMAReceiveLevel_4;
    /* Initialize the I2C_DMATransmitLevel member */
    I2C_DMAInitStruct->I2C_DMATransmitLevel = I2C_DMATransmitLevel_4;

    I2C_DMAInitStruct->I2C_DMAEnCmd = DISABLE;
}


/**
  * @brief  Enable or disable the i2c DMA interface
  * @param  I2Cx: Select the i2c peripheral.
  *         This parameter can be one of the following values:
  *         I2C0
  * @param  I2C_DMAReq: I2C DMA transfer requests
  *         This parameter can be I2C_DMAReq_Rx or I2C_DMAReq_Tx
  * @param  NewState: new state of the DMA interface
  *         This parameter can be ENABLE or DISABLE
  * @retval None
  */
void I2C_DMACmd(I2C_TypeDef *I2Cx, uint32_t I2C_DMAReq, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_I2C_DMAREQ(I2C_DMAReq));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (DISABLE != NewState) {
        I2Cx->IC_DMA_CR |= I2C_DMAReq;
    } else {
        I2Cx->IC_DMA_CR &= ~I2C_DMAReq;
    }
}

void I2C_SetSDASetupTime(I2C_TypeDef *I2Cx, uint32_t PCLKCycles)
{
    /* Check the parameters */
    assert_param(IS_I2C_SDA_SETUP_TIME(PCLKCycles));

    if (RESET == I2C_IsEnable(I2Cx)) {
        I2Cx->IC_SDA_SETUP = (PCLKCycles & I2C_IC_SDA_SETUP);
    }
}

void I2C_SetSDAHoldTime(I2C_TypeDef *I2Cx, uint32_t PCLKCycles)
{
    /* Check the parameters */
    assert_param(IS_I2C_SDA_HOLD_TIME(PCLKCycles));

    if (RESET == I2C_IsEnable(I2Cx)) {
        I2Cx->IC_SDA_HOLD = (PCLKCycles & I2C_IC_SDA_HOLD);
    }
}

void I2C_ITConfig(I2C_TypeDef *I2Cx, uint32_t I2C_IT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));
    assert_param(IS_I2C_CONFIG_IT(I2C_IT));

    if (NewState != DISABLE) {
        I2Cx->IC_INTR_MASK |= I2C_IT;
    } else {
        I2Cx->IC_INTR_MASK &= ~I2C_IT;
    }
}


ITStatus I2C_GetITStatus(I2C_TypeDef *I2Cx, uint32_t I2C_IT)
{
    /* Check the parameters */
    assert_param(IS_I2C_GET_IT(I2C_IT));

    if ((I2Cx->IC_INTR_STAT & I2C_IT) != RESET) {
        return SET;
    }

    return RESET;
}

ITStatus I2C_GetRawITStatus(I2C_TypeDef *I2Cx, uint32_t I2C_IT)
{
    /* Check the parameters */
    assert_param(IS_I2C_GET_IT(I2C_IT));

    if ((I2Cx->IC_RAW_INTR_STAT & I2C_IT) != RESET) {
        return SET;
    }

    return RESET;
}

void I2C_ClearITPendingBit(I2C_TypeDef *I2Cx, uint32_t I2C_IT)
{
    /* Check the parameters */
    assert_param(IS_I2C_CLEAR_IT(I2C_IT));

    if (I2C_IT_RXUDF | I2C_IT) {
        I2Cx->IC_CLR_RX_UNDER;
    }
    if (I2C_IT_RXOVF | I2C_IT) {
        I2Cx->IC_CLR_RX_OVER;
    }
    if (I2C_IT_TXOVF | I2C_IT) {
        I2Cx->IC_CLR_TX_OVER;
    }
    if (I2C_IT_RD_REQ | I2C_IT) {
        I2Cx->IC_CLR_RD_REQ;
    }
    if (I2C_IT_TX_ABRT | I2C_IT) {
        I2Cx->IC_CLR_TX_ABRT;
    }
    if (I2C_IT_RX_DONE | I2C_IT) {
        I2Cx->IC_CLR_RX_DONE;
    }
    if (I2C_IT_ACTIVITY | I2C_IT) {
        I2Cx->IC_CLR_ACTIVITY;
    }
    if (I2C_IT_STOP_DET | I2C_IT) {
        I2Cx->IC_CLR_STOP_DET;
    }
    if (I2C_IT_START_DET | I2C_IT) {
        I2Cx->IC_CLR_START_DET;
    }
    if (I2C_IT_GEN_CALL | I2C_IT) {
        I2Cx->IC_CLR_GEN_CALL;
    }
}

FlagStatus I2C_IsEnable(I2C_TypeDef *I2Cx)
{
    if ((I2Cx->IC_ENABLE & I2C_IC_ENABLE_ENABLE) != RESET) {
        return SET;
    }

    return RESET;
}

FlagStatus I2C_GetFlagStatus(I2C_TypeDef *I2Cx, uint32_t I2C_FLAG)
{
    if ((I2Cx->IC_STATUS & I2C_FLAG) != RESET) {
        return SET;
    }

    return RESET;
}

uint32_t I2C_GetFlagStatusReg(I2C_TypeDef *I2Cx)
{
    return I2Cx->IC_STATUS;
}

FlagStatus I2C_GetTXAbortSource(I2C_TypeDef *I2Cx, uint32_t I2C_TX_ABRT)
{
    if ((I2Cx->IC_TX_ABRT_SOURCE & I2C_TX_ABRT) != RESET) {
        return SET;
    }

    return RESET;
}

uint32_t I2C_GetTXAbortSourceReg(I2C_TypeDef *I2Cx)
{
    return I2Cx->IC_TX_ABRT_SOURCE;
}


I2CMode_TypeDef I2C_GetI2CMode(I2C_TypeDef *I2Cx)
{
    if ((I2Cx->IC_CON & I2C_IC_CON_MASTER_MODE) && (I2Cx->IC_CON & I2C_IC_CON_SLAVE_DISABLE)) {
        return I2C_Mode_Master;
    }

    return I2C_Mode_Slave;
}

void I2C_MasterGenerateReceiveSCL(I2C_TypeDef *I2Cx, I2CDataEndCondition_TypeDef DataCondition)
{
    uint16_t tmpReg = 0;
    assert_param(IS_I2C_DATA_END_CONDITION(DataCondition));

    tmpReg = I2C_ExtendData(tmpReg, I2C_DataCMD_Read, DataCondition);
    I2C_WriteDataToDR(I2Cx, tmpReg);
}

void I2C_WriteDataToDR(I2C_TypeDef *I2Cx, uint16_t ExtendData)
{
    I2Cx->IC_DATA_CMD = (ExtendData & 0x07FF);
}

uint8_t I2C_ReadDataFromDR(I2C_TypeDef *I2Cx)
{
    return (uint8_t)(I2Cx->IC_DATA_CMD & 0x00FF);
}

uint16_t I2C_ExtendData(uint8_t Data, I2CDataCMD_TypeDef DataCMD, I2CDataEndCondition_TypeDef DataCondition)
{
    uint16_t tmpRet = 0;
    assert_param(IS_I2C_DATA_CMD(DataCMD));
    assert_param(IS_I2C_DATA_END_CONDITION(DataCondition));

    tmpRet = (uint16_t)Data;

    /* Extend Data With Read/Write Operation */
    switch ((uint32_t)DataCMD) {
    case I2C_DataCMD_Read: {
        tmpRet |= I2C_IC_DATA_CMD_CMD;
        break;
    }
    case I2C_DataCMD_Write: {
        tmpRet &= ~I2C_IC_DATA_CMD_CMD;
        break;
    }
    }

    /* Extend Data With End Condition */
    switch ((uint32_t)DataCondition) {
    case I2C_DataEndCondition_Stop: {
        tmpRet |= I2C_IC_DATA_CMD_STOP;
        break;
    }
    case I2C_DataEndCondition_Restart: {
        tmpRet |= I2C_IC_DATA_CMD_RESTART;
        break;
    }
    case I2C_DataEndCondition_None: {
        tmpRet &= ~(I2C_IC_DATA_CMD_STOP | I2C_IC_DATA_CMD_RESTART);
        break;
    }
    }

    return tmpRet;
}

void I2C_SendData(I2C_TypeDef *I2Cx, uint8_t Data, I2CDataEndCondition_TypeDef DataCondition)
{
    uint32_t tmpReg = 0;
    assert_param(IS_I2C_DATA_END_CONDITION(DataCondition));

    if (I2C_Mode_Master == I2C_GetI2CMode(I2Cx)) {
        tmpReg = I2C_ExtendData(Data, I2C_DataCMD_Write, DataCondition);
    } else {
        tmpReg = I2C_ExtendData(Data, I2C_DataCMD_Write, I2C_DataEndCondition_None);
    }

    I2C_WriteDataToDR(I2Cx,  tmpReg);
}

uint8_t I2C_ReceiveData(I2C_TypeDef *I2Cx, I2CDataEndCondition_TypeDef DataCondition)
{
    uint8_t tmpRet = 0;
    assert_param(IS_I2C_DATA_END_CONDITION(DataCondition));

    if (I2C_Mode_Master == I2C_GetI2CMode(I2Cx)) {
        while (RESET == (I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXNF)));
        I2C_MasterGenerateReceiveSCL(I2Cx, DataCondition);
        while (RESET == (I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE)));
        tmpRet = (uint8_t)I2C_ReadDataFromDR(I2Cx);
    }

    return tmpRet;
}

void I2C_SendBytes(I2C_TypeDef *I2Cx, uint8_t *Data, uint32_t DataLen, I2CDataEndCondition_TypeDef DataCondition)
{
    assert_param(IS_I2C_DATA_END_CONDITION(DataCondition));

    while (DataLen) {
        while (RESET == (I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXNF)));

        if (DataLen == 1) {
            I2C_SendData(I2Cx, *Data, DataCondition);
        } else {
            /* it is same as I2C_SendData_Slave */
            I2C_SendData(I2Cx, *Data, I2C_DataEndCondition_None);
        }
        Data++;
        DataLen--;
    }
}

void I2C_ReceiveBytes(I2C_TypeDef *I2Cx, uint8_t *Data, uint32_t DataLen, I2CDataEndCondition_TypeDef DataCondition)
{
    assert_param(IS_I2C_DATA_END_CONDITION(DataCondition));
    uint32_t count = 0;

    while (RESET != I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE)) {
        I2Cx->IC_DATA_CMD;
    }

    while (DataLen) {
        if (RESET == (I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXNF))) {
            continue;
        }

        if (I2C_Mode_Master == I2C_GetI2CMode(I2Cx)) {
            if (DataLen == 1) {
                I2C_MasterGenerateReceiveSCL(I2Cx, DataCondition);
            } else {
                I2C_MasterGenerateReceiveSCL(I2Cx, I2C_DataEndCondition_None);
            }
        }

        while (RESET == (I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE))) {
            if (count++ > 100000) {
                return;
            }
        }
        *Data = (uint8_t)I2C_ReadDataFromDR(I2Cx);
        Data++;
        DataLen--;
    }
}

void I2C_SlaveGeneralNACKOnlyCmd(I2C_TypeDef *I2Cx, FunctionalState NewState)
{
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (DISABLE != NewState) {
        I2Cx->IC_SLV_DATA_NACK_ONLY |= I2C_IC_SLV_DATA_NACK_ONLY;
    } else {
        I2Cx->IC_SLV_DATA_NACK_ONLY &= ~I2C_IC_SLV_DATA_NACK_ONLY;
    }
}

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
