/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: 8080 driver.
 * Author: leon sun
 * Create: 2023-4-6
 ************************************************************************************************/


#ifndef _DRV_PARALLEL_8080_H
#define _DRV_PARALLEL_8080_H

#include "stdint.h"
#include "stdbool.h"


#define PARALLEL_8080_CS_PORT               GPIOE
#define PARALLEL_8080_CS_PIN                GPIO_Pin_9
#define PARALLEL_8080_RST_PORT              GPIOE
#define PARALLEL_8080_RST_PIN               GPIO_Pin_7

#define PARALLEL_8080_CS_SET                GPIO_SetBits(PARALLEL_8080_CS_PORT, PARALLEL_8080_CS_PIN)
#define PARALLEL_8080_CS_CLR                GPIO_ResetBits(PARALLEL_8080_CS_PORT, PARALLEL_8080_CS_PIN)

#define PARALLEL_8080_RST_SET               GPIO_SetBits(PARALLEL_8080_RST_PORT, PARALLEL_8080_RST_PIN)
#define PARALLEL_8080_RST_CLR               GPIO_ResetBits(PARALLEL_8080_RST_PORT, PARALLEL_8080_RST_PIN)

#define PARALLEL_8080_DMA_MAX_BYTE          4095

#define PARALLEL_8080_DMA_CHANNEL           DMA_Channel_7


void Parallel8080Init(void);
bool Parallel8080Busy(void);
void Parallel8080Reset(void);
void Parallel8080SendDmaData(uint8_t *data, uint32_t len);

//void Parallel8080ReadData(uint8_t cmd, uint8_t len)

#endif
