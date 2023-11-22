#ifndef _DRV_SPI_H
#define _DRV_SPI_H


#include "stdint.h"
#include "stdbool.h"
#include "mhscpu.h"

typedef struct {
    SPI_TypeDef *SPIx;
    uint32_t SYSCTRL_APB;

    GPIO_TypeDef *SPI_MISO_PORT;
    uint16_t SPI_MISO_PIN;
    GPIO_RemapTypeDef SPI_MISO_REMAP;

    GPIO_TypeDef *SPI_MOSI_PORT;
    uint16_t SPI_MOSI_PIN;
    GPIO_RemapTypeDef SPI_MOSI_REMAP;

    GPIO_TypeDef *SPI_CLK_PORT;
    uint16_t SPI_CLK_PIN;
    GPIO_RemapTypeDef SPI_CLK_REMAP;

    uint8_t dummyByte;
} SPI_Cfg_t;

void SpiInit(const SPI_Cfg_t *cfg);
void SpiSendRecv(const SPI_Cfg_t *cfg, const uint8_t *sendData, uint8_t *recvData, uint32_t len);
void SpiSendData(const SPI_Cfg_t *cfg, uint8_t *data, uint32_t len);
void SpiRecvData(const SPI_Cfg_t *cfg, uint8_t *data, uint32_t len);
void SpiTest(const SPI_Cfg_t *cfg, uint32_t num);
uint32_t GetSpiDmaRemain(void);

#endif
