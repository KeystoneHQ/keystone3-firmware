#include "drv_spi_io.h"
#include "cmsis_os.h"

/// @brief SPI implemented by GPIO init.
/// @param[in] cfg SPI config struct.
void SpiIoInit(const SPIIO_Cfg_t *cfg)
{
    GPIO_InitTypeDef gpioInit = {0};
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO, ENABLE);

    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = cfg->CLK_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(cfg->CLK_PORT, &gpioInit);

    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = cfg->MOSI_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(cfg->MOSI_PORT, &gpioInit);

    gpioInit.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioInit.GPIO_Pin = cfg->MISO_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(cfg->MISO_PORT, &gpioInit);

    GPIO_ResetBits(cfg->CLK_PORT, cfg->CLK_PIN);
    GPIO_ResetBits(cfg->MOSI_PORT, cfg->MOSI_PIN);
}

/// @brief Send/receive data to/from SPI device.
/// @param[in] cfg SPI config struct.
/// @param[in] sendData Data to be sent.
/// @param[out] recvData Received data.
/// @param[in] len Send/receive data length.
void SpiIoSendRecv(const SPIIO_Cfg_t *cfg, const uint8_t *sendData, uint8_t *recvData, uint32_t len)
{
    osKernelLock();
    if (recvData) {
        memset(recvData, 0, len);
    }
    for (uint32_t i = 0; i < len; i ++) {
        //GPIO_ResetBits(cfg->CLK_PORT, cfg->CLK_PIN);
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (sendData == NULL || (0x80 & (sendData[i] << bit)) == 0) {
                //logic 0
                //printf("sendData[%d]=%X\r\n", i, sendData[i]);
                GPIO_ResetBits(cfg->MOSI_PORT, cfg->MOSI_PIN);
            } else {
                //logic 1
                GPIO_SetBits(cfg->MOSI_PORT, cfg->MOSI_PIN);
            }
            GPIO_SetBits(cfg->CLK_PORT, cfg->CLK_PIN);
            if (recvData) {
                if (GPIO_ReadInputDataBit(cfg->MISO_PORT, cfg->MISO_PIN)) {
                    recvData[i] |= (0x80 >> bit);
                }
            }
            GPIO_ResetBits(cfg->CLK_PORT, cfg->CLK_PIN);
        }
    }
    //GPIO_ResetBits(cfg->MOSI_PORT, cfg->MOSI_PIN);
    osKernelUnlock();
}

/// @brief Send data to SPI device.
/// @param[in] cfg SPI config struct.
/// @param[in] data Data to be sent.
/// @param[in] len Send data length.
void SpiIoSendData(const SPIIO_Cfg_t *cfg, const uint8_t *data, uint32_t len)
{
    SpiIoSendRecv(cfg, data, NULL, len);
}

/// @brief Receive data from SPI device.
/// @param[in] cfg SPI config struct.
/// @param[in] data Received data.
/// @param[in] len Received data length.
void SpiIoRecvData(const SPIIO_Cfg_t *cfg, uint8_t *data, uint32_t len)
{
    SpiIoSendRecv(cfg, NULL, data, len);
}
