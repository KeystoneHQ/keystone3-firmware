/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: PSRAM驱动.
 * Author: leon sun
 * Create: 2022-11-10
 ************************************************************************************************/

#include "drv_psram.h"
#include "mhscpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "drv_trng.h"
#include "log_print.h"

void PsramInit(void)
{
    PSRAM_InitTypeDef PSRAM_InitStruct = {0};
    GPIO_PinRemapConfig(GPIOG, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10, GPIO_Remap_0);

    PSRAM_InitStruct.BurstWord = 1;
    PSRAM_InitStruct.CsnCycle = 0;
    PSRAM_InitStruct.DummyCycle = 6;
    PSRAM_InitStruct.SampleEdgeSel = PSRAM_SAMPLE_EDGE_SEL_FALLING;
    PSRAM_InitStruct.FreqSel = PSRAM_QSPI_CLK_FCLK_2;
    PSRAM_InitStruct.ReadMode = PSRAM_CMD_READ_BUS_MODE_FAST_READ_QUAD;//PSRAM_CMD_READ_BUS_MODE_FAST_READ;
    PSRAM_InitStruct.WriteMode = PSRAM_CMD_WRITE_BUS_MODE_QUAL_WRITE;//PSRAM_CMD_WRITE_BUS_MODE_WRITE;

    PSRAM_Init(&PSRAM_InitStruct);
    PSRAM_ExitQuadMode();
    PSRAM_DeviceReset();
    PSRAM_EnterQuadMode();

    //PsramSelfCheck();
    printf("start memset\r\n");
    memset((void *)MHSCPU_PSRAM_BASE, 0, MHSCPU_PSRAM_SIZE);
    printf("memset over\r\n");
}


void PsramOpen(void)
{
    PSRAM_InitTypeDef PSRAM_InitStruct = {0};
    GPIO_PinRemapConfig(GPIOG, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10, GPIO_Remap_0);

    PSRAM_InitStruct.BurstWord = 1;
    PSRAM_InitStruct.CsnCycle = 0;
    PSRAM_InitStruct.DummyCycle = 6;
    PSRAM_InitStruct.SampleEdgeSel = PSRAM_SAMPLE_EDGE_SEL_FALLING;
    PSRAM_InitStruct.FreqSel = PSRAM_QSPI_CLK_FCLK_2;
    PSRAM_InitStruct.ReadMode = PSRAM_CMD_READ_BUS_MODE_FAST_READ_QUAD;//PSRAM_CMD_READ_BUS_MODE_FAST_READ;
    PSRAM_InitStruct.WriteMode = PSRAM_CMD_WRITE_BUS_MODE_QUAL_WRITE;//PSRAM_CMD_WRITE_BUS_MODE_WRITE;

    PSRAM_Init(&PSRAM_InitStruct);
    PSRAM_ExitQuadMode();
    PSRAM_DeviceReset();
    PSRAM_EnterQuadMode();
}



void PsramTest(void)
{
    uint32_t i;
    uint8_t *psramAddr = (uint8_t *)MHSCPU_PSRAM_BASE;
    for (i = 0; i < 256; i++) {
        printf("psramAddr[%d]=%d\r\n", i, psramAddr[i]);
    }
    for (i = 0; i < 256; i++) {
        psramAddr[i] = i;
    }
    for (i = 0; i < 256; i++) {
        printf("psramAddr[%d]=%d\r\n", i, psramAddr[i]);
    }
}


void PsramSelfCheck(void)
{
    uint32_t i, seed;

    TrngGet(&seed, sizeof(seed));
    printf("start psram test,random seed=%d\r\n", seed);
    srand(seed);
    uint8_t *psram8Addr = (uint8_t *)MHSCPU_PSRAM_BASE;
    for (i = 0; i < MHSCPU_PSRAM_SIZE; i++) {
        psram8Addr[i] = rand() % 256;
    }

    //check
    srand(seed);
    for (i = 0; i < MHSCPU_PSRAM_SIZE; i++) {
        if (psram8Addr[i] != rand() % 256) {
            printf("psram err,psramAddr[%d]=%d\r\n", i, psram8Addr[i]);
            return;
        }
    }
    printf("psram self check ok!\r\n");
}


uint32_t GetPsramMappedAddr(void)
{
    return MHSCPU_PSRAM_BASE;
}


uint32_t GetPsramTotalSize(void)
{
    return MHSCPU_PSRAM_SIZE;
}

