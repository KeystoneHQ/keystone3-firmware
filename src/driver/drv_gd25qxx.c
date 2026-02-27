#include "drv_spi.h"
#include "stdio.h"
#include "mhscpu.h"
#include "drv_gd25qxx.h"
#include "err_code.h"

#define FLASH_USE_MUTEX             1

#define FLASH_CS_PORT               GPIOB
#define FLASH_CS_PIN                GPIO_Pin_3

#define FLASH_WP_PORT               GPIOE
#define FLASH_WP_PIN                GPIO_Pin_1

#define FLASH_HOLD_PORT             GPIOA
#define FLASH_HOLD_PIN              GPIO_Pin_3

#define FLASH_MISO_PORT             GPIOB
#define FLASH_MISO_PIN              GPIO_Pin_5

#define FLASH_MOSI_PORT             GPIOB
#define FLASH_MOSI_PIN              GPIO_Pin_4

#define FLASH_CLK_PORT              GPIOB
#define FLASH_CLK_PIN               GPIO_Pin_2

#define FLASH_CS_HIGH()             GPIO_SetBits(FLASH_CS_PORT, FLASH_CS_PIN)
#define FLASH_CS_LOW()              GPIO_ResetBits(FLASH_CS_PORT, FLASH_CS_PIN)

#define GD25_FLASH_ID               (0xC84018)
#define PY25_FLASH_ID               (0x852018)

static const SPI_Cfg_t FLASH_SPI_CONFIG = {
    .SPIx = SPIM2,
    .SYSCTRL_APB = SYSCTRL_APBPeriph_SPI2,
    .SPI_MISO_PORT = FLASH_MISO_PORT,
    .SPI_MISO_PIN = FLASH_MISO_PIN,
    .SPI_MISO_REMAP = GPIO_Remap_0,
    .SPI_MOSI_PORT = FLASH_MOSI_PORT,
    .SPI_MOSI_PIN = FLASH_MOSI_PIN,
    .SPI_MOSI_REMAP = GPIO_Remap_0,
    .SPI_CLK_PORT = FLASH_CLK_PORT,
    .SPI_CLK_PIN = FLASH_CLK_PIN,
    .SPI_CLK_REMAP = GPIO_Remap_0,
    .dummyByte = Dummy_Byte
};

#if (FLASH_USE_MUTEX)
#include "cmsis_os.h"
static osMutexId_t g_flashMutex;
#endif

static uint8_t Gd25FlashSendByte(uint8_t byte);
static void Gd25FlashSendData(uint32_t length, const uint8_t *sendData, uint8_t *rcvData);
static void Gd25FlashWriteEnable(void);
static uint32_t Gd25FlashReadStatus(void);
static uint32_t Gd25FlashReadData(uint32_t addr, uint8_t *buffer, uint32_t size);
static uint32_t Gd25FlashPageProgram(uint32_t addr, const uint8_t *buffer, uint32_t size);

void Gd25FlashInit(void)
{
    GPIO_InitTypeDef gpioInit = {0};
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO, ENABLE);

    SpiInit(&FLASH_SPI_CONFIG);
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = FLASH_CS_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(FLASH_CS_PORT, &gpioInit);

    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = FLASH_WP_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(FLASH_WP_PORT, &gpioInit);
    GPIO_SetBits(FLASH_WP_PORT, FLASH_WP_PIN);

    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = FLASH_HOLD_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(FLASH_HOLD_PORT, &gpioInit);
    GPIO_SetBits(FLASH_HOLD_PORT, FLASH_HOLD_PIN);

    FLASH_CS_HIGH();
#if (FLASH_USE_MUTEX)
    g_flashMutex = osMutexNew(NULL);
#endif
}

void Gd25FlashOpen(void)
{
    GPIO_InitTypeDef gpioInit = {0};

    SpiInit(&FLASH_SPI_CONFIG);
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = FLASH_CS_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(FLASH_CS_PORT, &gpioInit);

    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = FLASH_WP_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(FLASH_WP_PORT, &gpioInit);
    GPIO_SetBits(FLASH_WP_PORT, FLASH_WP_PIN);

    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = FLASH_HOLD_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(FLASH_HOLD_PORT, &gpioInit);
    GPIO_SetBits(FLASH_HOLD_PORT, FLASH_HOLD_PIN);

    FLASH_CS_HIGH();
}

uint32_t Gd25FlashReadID(void)
{
    FLASH_CS_LOW();

    uint8_t id[3] = {0};
    Gd25FlashSendByte(GD25QXX_CMD_READ_IDENTIFICATION);

    Gd25FlashSendData(3, NULL, id);

    FLASH_CS_HIGH();
    return (id[0] << 16) | (id[1] << 8) | id[2];
}

static void Gd25FlashWriteEnable(void)
{
    FLASH_CS_LOW();
    Gd25FlashSendByte(GD25QXX_CMD_WRITE_ENABLE);
    FLASH_CS_HIGH();
}

static uint32_t Gd25FlashReadStatus(void)
{
    uint8_t status = 0;

    FLASH_CS_LOW();

    Gd25FlashSendByte(GD25QXX_CMD_READ_STATUS);
    status = Gd25FlashSendByte(Dummy_Byte);

    FLASH_CS_HIGH();

    return status;
}

static uint32_t Gd25FlashReadData(uint32_t addr, uint8_t *buffer, uint32_t size)
{
    if (addr >= GD25QXX_FLASH_SIZE || NULL == buffer || size < 1) {
        return ERR_GD25_BAD_PARAM;
    }

    FLASH_CS_LOW();

    Gd25FlashSendByte(GD25QXX_CMD_READ_DATA);
    Gd25FlashSendByte((addr >> 16) & 0xFF);
    Gd25FlashSendByte((addr >> 8) & 0xFF);
    Gd25FlashSendByte(addr & 0xFF);

    Gd25FlashSendData(size, NULL, buffer);

    FLASH_CS_HIGH();

    return size;
}

static uint32_t Gd25FlashPageProgram(uint32_t addr, const uint8_t *buffer, uint32_t size)
{
    if (addr >= GD25QXX_FLASH_SIZE || NULL == buffer || size < 1) {
        return ERR_GD25_BAD_PARAM;
    }

    Gd25FlashWriteEnable();
    if (!(GD25QXX_FLASH_STATUS_WEL & Gd25FlashReadStatus())) {
        return ERR_GD25_WEL_FAILED;
    }

    FLASH_CS_LOW();

    Gd25FlashSendByte(GD25QXX_CMD_PAGE_PROGRAM);
    Gd25FlashSendByte((addr >> 16) & 0xFF);
    Gd25FlashSendByte((addr >> 8) & 0xFF);
    Gd25FlashSendByte(addr & 0xFF);

    Gd25FlashSendData(size, buffer, NULL);

    FLASH_CS_HIGH();

    while (Gd25FlashReadStatus() & GD25QXX_FLASH_STATUS_WIP);

    return size;
}

/***********************************************************************
 flash supports fatfs, use with caution
***********************************************************************/
int32_t Gd25FlashSectorErase(uint32_t addr)
{
#if (FLASH_USE_MUTEX)
    osMutexAcquire(g_flashMutex, osWaitForever);
#endif

    if (addr >= GD25QXX_FLASH_SIZE) {
        return ERR_GD25_BAD_PARAM;
    }

    Gd25FlashWriteEnable();
    if (GD25QXX_FLASH_STATUS_WEL != Gd25FlashReadStatus()) {
        return ERR_GD25_WEL_FAILED;
    }

    FLASH_CS_LOW();

    Gd25FlashSendByte(GD25QXX_CMD_SECTOR_ERASE);
    Gd25FlashSendByte((addr >> 16) & 0xFF);
    Gd25FlashSendByte((addr >> 8) & 0xFF);
    Gd25FlashSendByte(addr & 0xFF);

    FLASH_CS_HIGH();

    while (Gd25FlashReadStatus() & GD25QXX_FLASH_STATUS_WIP);
#if (FLASH_USE_MUTEX)
    osMutexRelease(g_flashMutex);
#endif
    return SUCCESS_CODE;
}

int32_t Gd25FlashBlockErase(uint32_t addr)
{
#if (FLASH_USE_MUTEX)
    osMutexAcquire(g_flashMutex, osWaitForever);
#endif

    if (addr >= GD25QXX_FLASH_SIZE) {
        return ERR_GD25_BAD_PARAM;
    }

    Gd25FlashWriteEnable();
    if (GD25QXX_FLASH_STATUS_WEL != Gd25FlashReadStatus()) {
        return ERR_GD25_WEL_FAILED;
    }

    FLASH_CS_LOW();

    Gd25FlashSendByte(GD25QXX_CMD_BLOCK_ERASE);
    Gd25FlashSendByte((addr >> 16) & 0xFF);
    Gd25FlashSendByte((addr >> 8) & 0xFF);
    Gd25FlashSendByte(addr & 0xFF);

    FLASH_CS_HIGH();

    while (Gd25FlashReadStatus() & GD25QXX_FLASH_STATUS_WIP);
#if (FLASH_USE_MUTEX)
    osMutexRelease(g_flashMutex);
#endif
    return SUCCESS_CODE;
}

/***********************************************************************
 flash supports fatfs, use with caution
***********************************************************************/
int32_t Gd25FlashChipErase(void)
{
#if (FLASH_USE_MUTEX)
    osMutexAcquire(g_flashMutex, osWaitForever);
#endif
    Gd25FlashWriteEnable();
    if (!(GD25QXX_FLASH_STATUS_WEL & Gd25FlashReadStatus())) {
        return ERR_GD25_WEL_FAILED;
    }

    FLASH_CS_LOW();
    Gd25FlashSendByte(GD25QXX_CMD_CHIP_ERASE);
    FLASH_CS_HIGH();

    while (Gd25FlashReadStatus() & GD25QXX_FLASH_STATUS_WIP) {
        ;
    }
#if (FLASH_USE_MUTEX)
    osMutexRelease(g_flashMutex);
#endif
    return SUCCESS_CODE;
}

int32_t Gd25FlashReadBuffer(uint32_t addr, uint8_t *buffer, uint32_t size)
{
    int32_t lastSize = size;
    int32_t len = 0;
    int32_t readBytes = 0;
#if (FLASH_USE_MUTEX)
    osMutexAcquire(g_flashMutex, osWaitForever);
#endif
    while (lastSize) {
        len = lastSize > GD25QXX_PAGE_SIZE ? GD25QXX_PAGE_SIZE : lastSize;
        readBytes += Gd25FlashReadData(addr + readBytes, buffer + readBytes, len);
        lastSize -= len;
    }
#if (FLASH_USE_MUTEX)
    osMutexRelease(g_flashMutex);
#endif
    return readBytes;
}

int32_t Gd25FlashWriteBuffer(uint32_t addr, const uint8_t *buffer, uint32_t size)
{
    int32_t writeBytes, writeCount = 0;

#if (FLASH_USE_MUTEX)
    osMutexAcquire(g_flashMutex, osWaitForever);
#endif
    if (addr % GD25QXX_PAGE_SIZE + size > GD25QXX_PAGE_SIZE) {
        //across page
        writeBytes = GD25QXX_PAGE_SIZE - addr % GD25QXX_PAGE_SIZE;
        Gd25FlashPageProgram(addr, buffer, writeBytes);
        writeCount += writeBytes;
    }
    while (writeCount < size) {
        writeBytes = size - writeCount > GD25QXX_PAGE_SIZE ? GD25QXX_PAGE_SIZE : size - writeCount;
        Gd25FlashPageProgram(addr + writeCount, buffer + writeCount, writeBytes);
        writeCount += writeBytes;
    }
#if (FLASH_USE_MUTEX)
    osMutexRelease(g_flashMutex);
#endif
    return writeCount;
}

int32_t Gd25FlashWriteBufferNoMutex(uint32_t addr, const uint8_t *buffer, uint32_t size)
{
    int32_t writeBytes, writeCount = 0;

    if (addr % GD25QXX_PAGE_SIZE + size > GD25QXX_PAGE_SIZE) {
        //across page
        writeBytes = GD25QXX_PAGE_SIZE - addr % GD25QXX_PAGE_SIZE;
        Gd25FlashPageProgram(addr, buffer, writeBytes);
        writeCount += writeBytes;
    }
    while (writeCount < size) {
        writeBytes = size - writeCount > GD25QXX_PAGE_SIZE ? GD25QXX_PAGE_SIZE : size - writeCount;
        Gd25FlashPageProgram(addr + writeCount, buffer + writeCount, writeBytes);
        writeCount += writeBytes;
    }
    return writeCount;
}

static uint8_t Gd25FlashSendByte(uint8_t byte)
{
    uint8_t sendData = byte;
    uint8_t recvData = 0;
    SpiSendRecv(&FLASH_SPI_CONFIG, &sendData, &recvData, 1);

    return recvData;
}

static void Gd25FlashSendData(uint32_t length, const uint8_t *sendData, uint8_t *rcvData)
{
    SpiSendRecv(&FLASH_SPI_CONFIG, sendData, rcvData, length);
}
