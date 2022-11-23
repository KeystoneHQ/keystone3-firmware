/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_psram.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the PSRAM firmware functions
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/

#include "mhscpu_psram.h"


void PSRAM_SendCmd(uint8_t cmd)
{
    PSRAM->PSRAM_CMD = (PSRAM->PSRAM_CMD & (~PSRAM_PSRAM_CMD_COMMAND_CODE)) | (cmd << PSRAM_CMD_COMMAND_CODE_Pos) | PSRAM_PSRAM_CMD_CONFIG_CMD;

    while (!(PSRAM->PSRAM_CMD & PSRAM_PSRAM_CMD_CMD_DONE));
}

void PSRAM_DeviceReset(void)
{
    PSRAM_SendCmd(PSRAM_CODE_RESET_ENABLE);
    PSRAM_SendCmd(PSRAM_CODE_RESET);
}

void PSRAM_EnterQuadMode(void)
{
    PSRAM_SendCmd(PSRAM_CODE_ENTER_QUAD_MODE);
    PSRAM->PSRAM_CMD |= PSRAM_PSRAM_CMD_QUAL_MODE;
}

void PSRAM_ExitQuadMode(void)
{
    PSRAM_SendCmd(PSRAM_CODE_EXIT_QUAD_MODE);
    PSRAM->PSRAM_CMD &= ~(PSRAM_PSRAM_CMD_QUAL_MODE);
}

void PSRAM_Init(PSRAM_InitTypeDef *PSRAM_InitStruct)
{
    uint32_t tmpPsramCmd = 0;
    uint32_t tmpDevicePara = 0;

    assert_param(IS_PSRAM_SAMPLE_EDGE_SEL(PSRAM_InitStruct->SampleEdgeSel));
    assert_param(IS_PSRAM_FREQ_SEL(PSRAM_InitStruct->FreqSel));
    assert_param(IS_PSRAM_READ_BUS_MODE(PSRAM_InitStruct->ReadMode));
    assert_param(IS_PSRAM_WRITE_BUS_MODE(PSRAM_InitStruct->WriteMode));

    tmpPsramCmd = PSRAM->PSRAM_CMD;
    tmpPsramCmd = (tmpPsramCmd & ~PSRAM_PSRAM_CMD_READ_BUS_MODE) | PSRAM_InitStruct->ReadMode;
    tmpPsramCmd = (tmpPsramCmd & ~PSRAM_PSRAM_CMD_WRITE_BUS_MODE) | PSRAM_InitStruct->WriteMode;
    PSRAM->PSRAM_CMD = tmpPsramCmd;

    tmpDevicePara = PSRAM->DEVICE_PARA;
    tmpDevicePara = (tmpDevicePara & ~PSRAM_DEVICE_PARA_SAMPLE_EDGE_SEL) | PSRAM_InitStruct->SampleEdgeSel;
    tmpDevicePara = (tmpDevicePara & ~PSRAM_DEVICE_PARA_FREQ_SEL) | PSRAM_InitStruct->FreqSel;
    tmpDevicePara = (tmpDevicePara & ~PSRAM_DEVICE_PARA_DUMMY_CYCLES) | (PSRAM_InitStruct->DummyCycle << PSRAM_DUMMY_CYCLE_Pos);
    tmpDevicePara = (tmpDevicePara & ~PSRAM_DEVICE_PARA_BRUST_WORD) | (PSRAM_InitStruct->BurstWord << PSRAM_BURST_WORD_Pos);
    tmpDevicePara = (tmpDevicePara & ~PSRAM_DEVICE_PARA_CSN_SYCLE) | (PSRAM_InitStruct->CsnCycle << PSRAM_CSN_CYCLE_Pos);
    PSRAM->DEVICE_PARA = tmpDevicePara;
}

PSRAM_DEVIDTypeDef PSRAM_ReadID(void)
{
    PSRAM_DEVIDTypeDef id;

    PSRAM->PSRAM_CMD |= PSRAM_PSRAM_CMD_READ_ID;
    PSRAM_SendCmd(PSRAM_CODE_READ_ID);

    id.MF_ID    = (PSRAM->DEVICE_ID_H >> 24) & 0xFF;
    id.KGD      = (PSRAM->DEVICE_ID_H >> 16) & 0xFF;
    id.EID_H16  = (PSRAM->DEVICE_ID_H) & 0xFFFF;
    id.EID_L32  = PSRAM->DEVICE_ID_L;

    return id;
}

/**
  * @brief  Set Read Bus Mode
  * @param  ReadMode: specifies the read mode.
  *   This parameter can be PSRAM_CMD_READ_BUS_MODE_READ,PSRAM_CMD_READ_BUS_MODE_FAST_READ,PSRAM_CMD_READ_BUS_MODE_FAST_READ_QUAD
  * @retval None
  */
void PSRAM_SetReadMode(uint32_t ReadMode)
{
    assert_param(IS_PSRAM_READ_BUS_MODE(ReadMode));

    PSRAM->PSRAM_CMD = (PSRAM->PSRAM_CMD & ~(PSRAM_PSRAM_CMD_READ_BUS_MODE)) | ReadMode;
}

/**
  * @brief  Set Write Bus Mode
  * @param  WriteMode: specifies the write mode.
  *   This parameter can be PSRAM_CMD_WRITE_BUS_MODE_WRITE,PSRAM_CMD_WRITE_BUS_MODE_QUAL_WRITE
  * @retval None
  */
void PSRAM_SetWriteMode(uint32_t WriteMode)
{
    assert_param(IS_PSRAM_WRITE_BUS_MODE(WriteMode));

    PSRAM->PSRAM_CMD = (PSRAM->PSRAM_CMD & ~(PSRAM_PSRAM_CMD_WRITE_BUS_MODE)) | WriteMode;
}

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
