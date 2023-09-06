/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: mh1903 gpio spi driver.
 * Author: leon sun
 * Create: 2023-3-1
 ************************************************************************************************/


#ifndef _DRV_SPI_IO_H
#define _DRV_SPI_IO_H


#include "stdint.h"
#include "stdbool.h"
#include "mhscpu.h"

typedef struct {
    GPIO_TypeDef *MISO_PORT;
    uint16_t MISO_PIN;
    GPIO_TypeDef *MOSI_PORT;
    uint16_t MOSI_PIN;
    GPIO_TypeDef *CLK_PORT;
    uint16_t CLK_PIN;
} SPIIO_Cfg_t;


void SpiIoInit(const SPIIO_Cfg_t *cfg);
void SpiIoSendRecv(const SPIIO_Cfg_t *cfg, const uint8_t *sendData, uint8_t *recvData, uint32_t len);
void SpiIoSendData(const SPIIO_Cfg_t *cfg, const uint8_t *data, uint32_t len);
void SpiIoRecvData(const SPIIO_Cfg_t *cfg, uint8_t *data, uint32_t len);


#endif
