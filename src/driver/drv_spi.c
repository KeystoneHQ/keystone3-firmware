#include "drv_spi.h"
#include "stdio.h"
#include "mhscpu.h"
#include "user_memory.h"
#include "log_print.h"

#define SPI_MODE_POLL               0
#define SPI_MODE_INTERRUPT          1
#define SPI_MODE_DMA                2

void SpiInit(const SPI_Cfg_t *cfg)
{
    SPI_InitTypeDef SPI_InitStructure;
    //GPIO_InitTypeDef gpioInit = {0};
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO | cfg->SYSCTRL_APB, ENABLE);
    SYSCTRL_APBPeriphResetCmd(cfg->SYSCTRL_APB, ENABLE);

    GPIO_PullUpCmd(cfg->SPI_MISO_PORT, cfg->SPI_MISO_PIN, DISABLE);
    //GPIO_PinRemapConfig(GPIOB, GPIO_Pin_2/* | GPIO_Pin_3*/ | GPIO_Pin_4 | GPIO_Pin_5, GPIO_Remap_0);
    GPIO_PinRemapConfig(cfg->SPI_MISO_PORT, cfg->SPI_MISO_PIN, cfg->SPI_MISO_REMAP);
    GPIO_PinRemapConfig(cfg->SPI_MOSI_PORT, cfg->SPI_MOSI_PIN, cfg->SPI_MOSI_REMAP);
    GPIO_PinRemapConfig(cfg->SPI_CLK_PORT, cfg->SPI_CLK_PIN, cfg->SPI_CLK_REMAP);
    GPIO_PullUpCmd(cfg->SPI_MISO_PORT, cfg->SPI_MISO_PIN, DISABLE);
    //gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    //gpioInit.GPIO_Pin = GPIO_Pin_3;
    //gpioInit.GPIO_Remap = GPIO_Remap_1;
    //GPIO_Init(GPIOB, &gpioInit);
    //GPIO_SetBits(GPIOB, GPIO_Pin_3);                //mh1903 EVB, NFC Chip needs to be disable

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_0;//SPI_NSS_Null;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_InitStructure.SPI_RXFIFOFullThreshold = SPI_RXFIFOFullThreshold_8;
    SPI_InitStructure.SPI_TXFIFOEmptyThreshold = SPI_TXFIFOEmptyThreshold_7;

    SPI_Init(cfg->SPIx, &SPI_InitStructure);
    SPI_Cmd(cfg->SPIx, ENABLE);

    //DMA_Cmd(ENABLE);
}


//sendData or recvData could be NULL when do not need send or receive.
void SpiSendRecv(const SPI_Cfg_t *cfg, const uint8_t *sendData, uint8_t *recvData, uint32_t len)
{
    uint32_t i;
    uint8_t dataTemp;
    for (i = 0; i < len; i++) {
        dataTemp = sendData != NULL ? sendData[i] : cfg->dummyByte;
        SPI_SendData(cfg->SPIx, dataTemp);
        while (SPI_GetFlagStatus(cfg->SPIx, SPI_FLAG_RXNE) == RESET);
        dataTemp = SPI_ReceiveData(cfg->SPIx);
        if (recvData !=  NULL) {
            recvData[i] = dataTemp;
        }
    }
}


void SpiSendData(const SPI_Cfg_t *cfg, uint8_t *data, uint32_t len)
{
    SpiSendRecv(cfg, data, NULL, len);
}


void SpiRecvData(const SPI_Cfg_t *cfg, uint8_t *data, uint32_t len)
{
    SpiSendRecv(cfg, NULL, data, len);
}


void Spi2Test(uint32_t num)
{
    //uint8_t *sendData, *recvData;
    //printf("send %d bytes\r\n", num);
    //sendData = SRAM_MALLOC(num);
    //recvData = SRAM_MALLOC(num);
    //if (sendData == NULL || recvData == NULL) {
    //    SRAM_FREE(sendData);
    //    SRAM_FREE(recvData);
    //    printf("malloc err\r\n");
    //    return;
    //}
    //for (uint32_t i = 0; i < num; i++) {
    //    sendData[i] = i;
    //}
    //printf("start\r\n");
    //Spi2SendRecv(sendData, recvData, num);
    //printf("send over\r\n");
    //PrintArray("recv", recvData, num);
}


void SPI2_IRQHandler(void)
{
    if (SPI_GetITStatus(SPIM2, SPI_IT_TXE) == SET) {
        SPI_ITConfig(SPIM2, SPI_IT_TXE, DISABLE);
        SPI_ClearITPendingBit(SPIM2, SPI_IT_TXE);
    }
    NVIC_ClearPendingIRQ(SPI2_IRQn);
}

