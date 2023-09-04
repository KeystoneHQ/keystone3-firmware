/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : drv_sensor.c
 * Description:
 * author     : stone wang
 * data       : 2022-12-23 10:02
**********************************************************************/

#include <stdio.h>
#include "drv_sensor.h"

#define EXT_SENSOR_STATIC           (0x1)
#define EXT_SENSOR_DYNAMIC          (0x2)

#define EXT_SENSOR_PORT_MODE        EXT_SENSOR_DYNAMIC

#if (EXT_SENSOR_PORT_MODE == EXT_SENSOR_DYNAMIC)
#define EXT_SENSOR_PORT             SENSOR_Port_S01
#else
#include "mhscpu_gpio.h"
#define EXT_SENSOR_PORT             SENSOR_Port_S0
#define EXT_GPIO_PORT               GPIOD
#define EXT_GPIO_PIN                GPIO_Pin_15
#endif

//static void SensorNvicConfiguration(void);
static void SensorRegPrint(char *regName, uint32_t *regAddr);

void SensorInit(void)
{
#if 1
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_BPU, ENABLE);

    //===========================================================
    SENSOR_EXTInitTypeDef SENSOR_EXTInitStruct;
    uint32_t SENSOR_ANA;
    memset(&SENSOR_EXTInitStruct, 0, sizeof(SENSOR_EXTInitStruct));
    SENSOR_EXTInitStruct.SENSOR_Port_Dynamic = SENSOR_Port_S01 | SENSOR_Port_S23 | SENSOR_Port_S45 | SENSOR_Port_S67;
    SENSOR_EXTInitStruct.SENSOR_DynamicFrequency = SENSOR_DynamicFrequency_1s;//SENSOR_DynamicFrequency_31_25ms;̫��Ӱ���ͷ
    SENSOR_EXTInitStruct.SENSOR_Dynamic_Sample = SENSOR_DYNAMIC_SAMPLE_2;

    SENSOR_EXTInitStruct.SENSOR_GlitchEnable = ENABLE;
    SENSOR_EXTInitStruct.SENSOR_Port_Enable = ENABLE;

    SENSOR_ClearITPendingBit();

    SENSOR_ANA = SENSOR_ANA_VOL_HIGH | SENSOR_ANA_VOL_LOW | SENSOR_ANA_TEMPER_HIGH | SENSOR_ANA_TEMPER_LOW | SENSOR_ANA_MESH | SENSOR_ANA_XTAL32K | SENSOR_ANA_VOLGLITCH;

    SENSOR_EXTCmd(DISABLE);
    SENSOR_Soft_Enable();
    SENSOR->SEN_VG_DETECT &= ~0X04;

    while (SET == SENSOR_EXTIsRuning());

    SENSOR_EXTInit(&SENSOR_EXTInitStruct);
    SENSOR_AttackRespMode(SENSOR_Interrupt);

    NVIC_ClearPendingIRQ(SENSOR_IRQn);
    SENSOR_EXTCmd(DISABLE);
    SENSOR_ANACmd(SENSOR_ANA, DISABLE);
#else
    SENSOR_EXTInitTypeDef Sensor_InitStruct;
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_BPU, ENABLE);

    SENSOR_EXTCmd(DISABLE);
    SENSOR_ClearITPendingBit();

    //SensorNvicConfiguration();
    SSC_SENSORAttackRespMode(SSC_SENSOR_Interrupt);

    Sensor_InitStruct.SENSOR_Port_Pull = EXT_SENSOR_PORT;
#if (EXT_SENSOR_PORT_MODE == EXT_SENSOR_DYNAMIC)
    Sensor_InitStruct.SENSOR_Port_Static = 0;
    Sensor_InitStruct.SENSOR_Port_Dynamic = EXT_SENSOR_PORT;
#else
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = EXT_GPIO_PIN;
    GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(EXT_GPIO_PORT, &GPIO_InitStruct);
    GPIO_ResetBits(EXT_GPIO_PORT, EXT_GPIO_PIN);

    Sensor_InitStruct.SENSOR_Port_Static = EXT_SENSOR_PORT;
    Sensor_InitStruct.SENSOR_Port_Dynamic = 0;
#endif
    Sensor_InitStruct.SENSOR_Port_Enable = ENABLE;
    Sensor_InitStruct.SENSOR_GlitchEnable = ENABLE;
    Sensor_InitStruct.SENSOR_PUPU_Frequency = SENSOR_PUPU_Frequency_Default;
    Sensor_InitStruct.SENSOR_PUPU_DetectTime = 0;
    Sensor_InitStruct.SENSOR_PUPU_HoldTime = SENSOR_PUPU_HoldTime_Default;
    Sensor_InitStruct.SENSOR_PUPU_Enable = DISABLE;
    Sensor_InitStruct.SENSOR_Trig_Hold_Enable = DISABLE;
    Sensor_InitStruct.SENSOR_Dynamic_Sample = SENSOR_DynamicFrequency_Default;
    Sensor_InitStruct.SENSOR_Static_Sample = SENSOR_STATIC_SAMPLE_Default;
    SENSOR_EXTInit(&Sensor_InitStruct);
    // SENSOR_EXTCmd(ENABLE);
#endif
}

void SensorRegDebug(void)
{
    char tempBuf[16] = {0};
    SensorRegPrint("SEN_EXT_TYPE", (uint32_t *)&SENSOR->SEN_EXT_TYPE);
    SensorRegPrint("SEN_EXT_CFG", (uint32_t *)&SENSOR->SEN_EXT_CFG);
    SensorRegPrint("SEN_SOFT_EN", (uint32_t *)&SENSOR->SEN_SOFT_EN);
    SensorRegPrint("SEN_STATE", (uint32_t *)&SENSOR->SEN_STATE);
    SensorRegPrint("SEN_BRIDGE", (uint32_t *)&SENSOR->SEN_BRIDGE);
    SensorRegPrint("SEN_SOFT_ATTACK", (uint32_t *)&SENSOR->SEN_SOFT_ATTACK);
    SensorRegPrint("SEN_SOFT_LOCK", (uint32_t *)&SENSOR->SEN_SOFT_LOCK);
    SensorRegPrint("SEN_ATTACK_CNT", (uint32_t *)&SENSOR->SEN_ATTACK_CNT);
    SensorRegPrint("SEN_ATTACK_TYP", (uint32_t *)&SENSOR->SEN_ATTACK_TYP);
    SensorRegPrint("SEN_VG_DETECT", (uint32_t *)&SENSOR->SEN_VG_DETECT);
    SensorRegPrint("SEN_RNG_INI", (uint32_t *)&SENSOR->SEN_RNG_INI);
    for (int i = 0; i < 8; i++) {
        sprintf(tempBuf, "SEN_EN[%d]", i);
        SensorRegPrint(tempBuf, (uint32_t *) & (SENSOR->SEN_EN[i]));
    }
    SensorRegPrint("SEN_VH_EN", (uint32_t *) & (SENSOR->SEN_EN[SENSOR_EN_VH]));
    SensorRegPrint("SEN_VL_EN", (uint32_t *) & (SENSOR->SEN_EN[SENSOR_EN_VL]));
    SensorRegPrint("SEN_TH_EN", (uint32_t *) & (SENSOR->SEN_EN[SENSOR_EN_TH]));
    SensorRegPrint("SEN_TL_EN", (uint32_t *) & (SENSOR->SEN_EN[SENSOR_EN_TL]));
    SensorRegPrint("SEN_XTAL32_EN", (uint32_t *) & (SENSOR->SEN_EN[SENSOR_EN_XTAL32]));
    SensorRegPrint("SEN_MESH_EN", (uint32_t *) & (SENSOR->SEN_EN[SENSOR_EN_MESH]));
    SensorRegPrint("SEN_VOLGLTCH_EN", (uint32_t *) & (SENSOR->SEN_EN[SENSOR_EN_VOLGLITCH]));
    SensorRegPrint("SEN_EXTS_START", (uint32_t *)&SENSOR->SEN_EXTS_START);
    SensorRegPrint("SEN_LOCK", (uint32_t *)&SENSOR->SEN_LOCK);
    SensorRegPrint("SEN_ANA0", (uint32_t *)&SENSOR->SEN_ANA0);
    SensorRegPrint("SEN_ANA1", (uint32_t *)&SENSOR->SEN_ANA1);
    SensorRegPrint("SEN_ATTCLR", (uint32_t *)&SENSOR->SEN_ATTCLR);
}

static void SensorRegPrint(char *regName, uint32_t *regAddr)
{
    printf("%s addr = %#x, val = %08X\r\n", regName, regAddr, *regAddr);
}

//static void SensorNvicConfiguration(void)
//{
//    NVIC_InitTypeDef NVIC_InitStructure;
//
//    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);
//
//    NVIC_InitStructure.NVIC_IRQChannel = SENSOR_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
//    NVIC_Init(&NVIC_InitStructure);
//
//    NVIC_InitStructure.NVIC_IRQChannel = SSC_IRQn;
//    NVIC_Init(&NVIC_InitStructure);
//}

void SENSOR_IRQHandler(void)
{
    SensorRegPrint("SEN_STATE", (uint32_t *)&SENSOR->SEN_STATE);
    SENSOR->SEN_STATE = 0;
    NVIC_ClearPendingIRQ(SENSOR_IRQn);

    // todo prevent demolition
}

void SSC_IRQHandler(void)
{
    printf("SSC_IRQHandler in %08X\n", SSC->SSC_SR);

    SSC_ClearITPendingBit(SSC_ITSysXTAL12M | SSC_ITSysGlitch | SSC_ITSysVolHigh | SSC_ITSysVolLow);
    NVIC_ClearPendingIRQ(SSC_IRQn);
}

