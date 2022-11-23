/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_ssc.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the SSC firmware functions
 *****************************************************************************/
#include "mhscpu_ssc.h"


#define SSC_SEN_ENABLE                                  ((uint32_t)0xAA)
#define SSC_SEN_DISABLE                                 ((uint32_t)0x55)

void SSC_Init(SSC_InitTypeDef *SSC_InitStruct)
{
    assert_param(IS_FUNCTIONAL_STATE(SSC_InitStruct->ParityCheck));

    SSC->SSC_CR3 = (SSC->SSC_CR3 & ~0x3FUL) | (SSC_InitStruct->ParityCheck == ENABLE ? 0x00 : 0x10);

    SSC->SSC_SR_CLR = 0x7FFF;
}

ITStatus SSC_GetITStatus(uint32_t SSC_IT)
{

    if (SSC->SSC_SR && SSC_IT) {
        return SET;
    } else {
        return RESET;
    }
}

void SSC_ClearITPendingBit(uint32_t SSC_IT)
{
    if (SSC_IT & SSC_ITSysXTAL12M) {
        SSC->SSC_SR |= SSC_ITSysXTAL12M;
    }

    if (SSC_IT & SSC_ITSysGlitch) {
        SSC->SSC_SR |= SSC_ITSysGlitch;
    }

    if (SSC_IT & SSC_ITSysVolHigh) {
        SSC->SSC_SR |= SSC_ITSysVolHigh;
    }

    if (SSC_IT & SSC_ITSysVolLow) {
        SSC->SSC_SR |= SSC_ITSysVolLow;
    }

    SSC_IT &= ~(SSC_ITSysXTAL12M | SSC_ITSysGlitch | SSC_ITSysVolHigh | SSC_ITSysVolLow);
    SSC->SSC_SR_CLR = SSC_IT;
}

void SSC_SetDataRAMScrambler(uint32_t Scrambler)
{
    SSC->DATARAM_SCR = Scrambler;
}

void SSC_BPKAccessCtrlConfig(uint32_t SSC_BPKAccessCtrBlock, SSC_BPKAccessCtrlTypeDef SSC_BPKAccessCtr)
{
    assert_param(IS_BPK_ACCESS_CTRL(SSC_BPKAccessCtr));

    if (SSC_BPKAccessCtr == SSC_BPKReadOnly) {
        SSC->BPU_RWC |= SSC_BPKAccessCtrBlock << 4;
        SSC->BPU_RWC &= ~(SSC_BPKAccessCtrBlock);
    } else if (SSC_BPKAccessCtr == SSC_BPKWriteOnly) {
        SSC->BPU_RWC &= ~(SSC_BPKAccessCtrBlock << 4);
        SSC->BPU_RWC |= SSC_BPKAccessCtrBlock;
    } else if (SSC_BPKAccessCtr == SSC_BPKReadWrite) {
        SSC->BPU_RWC |= (SSC_BPKAccessCtrBlock << 4 | SSC_BPKAccessCtrBlock);
    }
}


uint32_t SSC_SENSORCmd(uint32_t SSC_SENSOR, FunctionalState NewState)
{
    uint32_t sensor_en = SSC_SEN_ENABLE;

    assert_param(IS_SSC_SENSOR(SSC_SENSOR));

    if (DISABLE != NewState) {
        sensor_en = SSC_SEN_ENABLE;
    } else {
        sensor_en = SSC_SEN_DISABLE;
    }

    if (SSC_SENSOR & SSC_SENSOR_XTAL12M) {
        SSC->MAIN_SEN_EN = (SSC->MAIN_SEN_EN & ~(0xFFUL << 0)) | (sensor_en << 0);
    }
    if (SSC_SENSOR & SSC_SENSOR_VOL_LOW) {
        SSC->MAIN_SEN_EN = (SSC->MAIN_SEN_EN & ~(0xFFUL << 8)) | (sensor_en << 8);
    }
    if (SSC_SENSOR & SSC_SENSOR_VOL_HIGH) {
        SSC->MAIN_SEN_EN = (SSC->MAIN_SEN_EN & ~(0xFFUL << 16)) | (sensor_en << 16);
    }
    if (SSC_SENSOR & SSC_SENSOR_VOLGLITCH) {
        SSC->MAIN_SEN_EN = (SSC->MAIN_SEN_EN & ~(0xFFUL << 24)) | (sensor_en << 24);
    }

    return 0;
}

void SSC_SENSORLock(uint32_t SSC_SENSOR)
{
    assert_param(IS_SSC_SENSOR(SSC_SENSOR));

    SSC->MAIN_SEN_LOCK = SSC_SENSOR;
}

void SSC_SENSORAttackRespMode(SSC_SENSOR_RespModeTypeDef SSC_SENSOR_RespMode)
{
    assert_param(IS_SSC_SENSOR_RESP_MODE(SSC_SENSOR_RespMode));

    if (SSC_SENSOR_CPUReset == SSC_SENSOR_RespMode) {
        SSC->SSC_CR3 &= ~BIT(28);
    } else if (SSC_SENSOR_Interrupt == SSC_SENSOR_RespMode) {
        SSC->SSC_CR3 |= BIT(28);
    }
}

void SSC_SENSORKeyClearCmd(uint32_t SSC_SENSOR, FunctionalState KeyClearEn)
{
    assert_param(IS_SSC_SENSOR(SSC_SENSOR));
    assert_param(IS_FUNCTIONAL_STATE(KeyClearEn));

    if (SSC_SENSOR & SSC_SENSOR_XTAL12M) {
        SSC->SSC_ACK = (SSC->SSC_ACK & ~BIT(12)) | (KeyClearEn << 12);
    }
    if (SSC_SENSOR & SSC_SENSOR_VOL_LOW) {
        SSC->SSC_ACK = (SSC->SSC_ACK & ~BIT(13)) | (KeyClearEn << 13);
    }
    if (SSC_SENSOR & SSC_SENSOR_VOL_HIGH) {
        SSC->SSC_ACK = (SSC->SSC_ACK & ~BIT(14)) | (KeyClearEn << 14);
    }
    if (SSC_SENSOR & SSC_SENSOR_VOLGLITCH) {
        SSC->SSC_ACK = (SSC->SSC_ACK & ~BIT(15)) | (KeyClearEn << 15);
    }
}

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
