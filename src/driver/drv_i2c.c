#include "drv_i2c.h"
#include "stdio.h"
#include "mhscpu.h"


/// @brief I2C hardware init
/// @param
void I2cInit(void)
{
    I2C_InitTypeDef I2C_InitStruct = {0};

    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_I2C0, ENABLE);
    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_I2C0, ENABLE);
    //I2C_DeInit(I2C0);
    GPIO_PinRemapConfig(GPIOB, GPIO_Pin_0, GPIO_Remap_0);
    GPIO_PinRemapConfig(GPIOB, GPIO_Pin_1, GPIO_Remap_0);
    I2C_StructInit(&I2C_InitStruct);
    I2C_InitStruct.I2C_ClockSpeed = I2C_ClockSpeed_400KHz;
    I2C_InitStruct.I2C_Mode = I2C_Mode_Master;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_1;
    I2C_InitStruct.I2C_TargetAddress = 0x50;
    I2C_InitStruct.I2C_TargetAddressMode = I2C_TargetAddressMode_7bit;
    I2C_InitStruct.I2C_GenerateRestartEnable = ENABLE;
    I2C_Init(I2C0, &I2C_InitStruct);
    I2C_Cmd(I2C0, ENABLE);
}


/// @brief I2C send data sequence.
/// @param addr I2C addr(high 7-bit).
/// @param data send data buffer.
/// @param len send data buffer length.
void I2cSendData(uint8_t addr, const uint8_t *data, uint32_t len)
{
    uint32_t count = 0;
    I2C_SetTargetAddress(I2C0, addr, I2C_TargetAddressMode_7bit);
    I2C_SendBytes(I2C0, (uint8_t *)data, len, I2C_DataEndCondition_Stop);
    while (RESET != I2C_GetFlagStatus(I2C0, I2C_FLAG_ACTIVITY)) {
        if (count++ > 100000) {
            printf("i2c overtime\n");
            break;
        }
    }
}


/// @brief Send u16 cmd and then send data sequence.
/// @param addr I2C addr(high 7-bit).
/// @param cmd u16 cmd, send MSB first.
/// @param data send data buffer.
/// @param len send data buffer length.
void I2cSendCmdAndData(uint8_t addr, uint16_t cmd, const uint8_t *data, uint32_t len)
{
    uint8_t cmdSeq[2];

    I2C_SetTargetAddress(I2C0, addr, I2C_TargetAddressMode_7bit);
    cmdSeq[0] = cmd >> 8;
    cmdSeq[1] = cmd;
    I2C_SendBytes(I2C0, cmdSeq, 2, I2C_DataEndCondition_None);
    I2C_SendBytes(I2C0, (uint8_t *)data, len, I2C_DataEndCondition_None);
    printf("IC_TX_ABRT_SOURCE=0x%08X\r\n", I2C0->IC_TX_ABRT_SOURCE);
}


/// @brief Send u16 cmd and then get i2c data sequence.
/// @param addr I2C addr(high 7-bit).
/// @param cmd u16 cmd, send MSB first.
/// @param data received data.
/// @param len received data length.
void I2cSendCmdAndReceiveData(uint8_t addr, uint16_t cmd, uint8_t *data, uint32_t len)
{
    uint8_t cmdSeq[2];

    I2C_SetTargetAddress(I2C0, addr, I2C_TargetAddressMode_7bit);
    cmdSeq[0] = cmd >> 8;
    cmdSeq[1] = cmd;
    I2C_SendBytes(I2C0, cmdSeq, 2, I2C_DataEndCondition_Stop);
    I2C_ReceiveBytes(I2C0, data, len, I2C_DataEndCondition_Stop);
}



/// @brief Send u8 addr and then get i2c data sequence.
/// @param addr I2C addr(high 7-bit).
/// @param sendData send data.
/// @param sendLen send data len.
/// @param rcvData received data.
/// @param rcvLen received data length.
void I2cSendAndReceiveData(uint8_t addr, const uint8_t *sendData, uint32_t sendLen, uint8_t *rcvData, uint32_t rcvLen)
{
    I2C_SetTargetAddress(I2C0, addr, I2C_TargetAddressMode_7bit);
    I2C_SendBytes(I2C0, (uint8_t *)sendData, sendLen, I2C_DataEndCondition_Stop);
    I2C_ReceiveBytes(I2C0, rcvData, rcvLen, I2C_DataEndCondition_Stop);
}


