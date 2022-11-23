#include "drv_sdio.h"
#include "mhscpu_sdio.h"


#define SDIO_DEBUG          0

#if SDIO_DEBUG == 1
#define SDIO_PRINTF(fmt, args...)                printf(fmt, ##args)
#else
#define SDIO_PRINTF(fmt, args...)
#endif

static void SDIODMAConfig(DMA_TypeDef* dmaChannel, bool isWrite, uint32_t* buffer, uint32_t size)
{
    SDIO_DMACmd(ENABLE);
    DMA_Cmd(ENABLE);

    DMA_InitTypeDef DMA_InitStruct;

    DMA_InitStruct.DMA_DIR = isWrite ? DMA_DIR_Memory_To_Peripheral : DMA_DIR_Peripheral_To_Memory;

    DMA_InitStruct.DMA_Peripheral          = (uint32_t)SDIO;
    DMA_InitStruct.DMA_PeripheralBaseAddr  = (uint32_t) & (SDIO->FIFO);
    DMA_InitStruct.DMA_PeripheralInc       = DMA_Inc_Nochange;
    DMA_InitStruct.DMA_PeripheralDataSize  = DMA_DataSize_Word;
    DMA_InitStruct.DMA_PeripheralBurstSize = DMA_BurstSize_8;

    DMA_InitStruct.DMA_MemoryBaseAddr      = (uint32_t)buffer;
    DMA_InitStruct.DMA_MemoryInc           = DMA_Inc_Increment;
    DMA_InitStruct.DMA_MemoryDataSize      = DMA_DataSize_Word;
    DMA_InitStruct.DMA_MemoryBurstSize     = DMA_BurstSize_8;
    DMA_InitStruct.DMA_BlockSize           = size / 4;
    DMA_InitStruct.DMA_PeripheralHandShake = DMA_PeripheralHandShake_Hardware;

    DMA_Init(dmaChannel, &DMA_InitStruct);
    DMA_ChannelCmd(dmaChannel, ENABLE);
}

void SDIOClockConfig(uint32_t clockInKHZ, bool isQuadBus, bool isClockLowPower)
{
    bool isLowSpeed = clockInKHZ <= 400;

    uint32_t sdioClockInput = SYSCTRL->HCLK_1MS_VAL / (isLowSpeed ? 8 : 4);

    SDIO->UHS_REG_EXT = (isLowSpeed << 30) | (2 << 23) | (0 << 16);

    SDIO->CLKDIV = (sdioClockInput / clockInKHZ + isLowSpeed) / 2;
    SDIO->CTYPE  = isQuadBus ? SDIO_BusWide_4b : SDIO_BusWide_1b;
    SDIO->CLKENA = isClockLowPower << 16 | BIT0;

    SDIO->CMD = 0x80200000;
    while (SDIO->CMD & BIT31) {}
}

bool SDIOExecuteCommand(uint8_t cmdIndex, uint32_t argument, SDIOResponseTypeEnum responseType, uint32_t* response)
{
    SDIOCommandStruct command = {0};

    command.CmdIndex = cmdIndex;
    command.Response = responseType == SDIOResponseR2 ? 3 : (responseType ? 1 : 0);

    command.StartCmd            = 1;
    command.WaitPrvDataComplete = 1;
    command.ResponseCRCCheck    = responseType != SDIOResponseNone && responseType != SDIOResponseR3;
    command.UseHoldReg          = 1;

    // Start Command
    SDIO->CMDARG = argument;
    SDIO->CMD    = command.Parameter;

    // Wait Command Done & Check Command Error
    while (!(SDIO->RINTSTS & SDIO_IT_CD)) {}
    uint16_t error = SDIO->RINTSTS;

    SDIO->RINTSTS = SDIO_IT_ALL;
    if (error & SDIO_IT_CRE) {
        SDIO_PRINTF("CMD%d error %04X\n", cmdIndex, error);
        return false;
    }

    // Get Response
    if (responseType && response) {
        response[0] = SDIO->RESP0;
        if (responseType == SDIOResponseR2) {
            response[1] = SDIO->RESP1;
            response[2] = SDIO->RESP2;
            response[3] = SDIO->RESP3;
        }
    }

    // Wait Busy
    if (responseType == SDIOResponseR1b) {
        while (SDIO->STATUS & (BIT9)) {}
    }

    return true;
}

bool SDIOTransferBlock(uint8_t cmdIndex, uint32_t argument, bool isWrite, uint8_t* buffer, uint32_t blockLength, uint32_t blockCount)
{
    SDIOCommandStruct command = {0};

    command.CmdIndex = cmdIndex;
    command.Response = SDIOResponseR1;

    command.StartCmd            = 1;
    command.WaitPrvDataComplete = 1;
    command.ResponseCRCCheck    = 1;
    command.UseHoldReg          = 1;

    // Data Transfer Config
    command.DataExpected = 1;
    command.TransferMode = 0;
    command.TransferDir  = isWrite;
    command.SendAutoStop = 0;

    SDIO->BLKSIZ = blockLength;
    SDIO->BYTCNT = blockLength * blockCount;
    SDIODMAConfig(CONFIG_SDIO_DMA_CHANNEL, isWrite, (uint32_t*)buffer, blockLength * blockCount);

    // Start Command
    SDIO->CMDARG = argument;
    SDIO->CMD    = command.Parameter;

    // Wait Command Done & Check Command Error
    while (!(SDIO->RINTSTS & SDIO_IT_CD)) {}
    uint16_t error = SDIO->RINTSTS;
    if (error & SDIO_IT_CRE) {
        SDIO->RINTSTS = SDIO_IT_ALL;
        SDIO_PRINTF("CMD%d error %04X\n", cmdIndex, error);
        return error;
    }

    // Wait Data End & Check Data Error
    while (!(SDIO->RINTSTS & SDIO_IT_DTO)) {}
    GPIO_ResetBits(GPIOD, GPIO_Pin_6);

    error = SDIO->RINTSTS;

    SDIO->RINTSTS = SDIO_IT_ALL;
    if (error & SDIO_IT_DTE) {
        SDIO_PRINTF("Data error %04X\n", error);
        return false;
    }

    // Wait Busy (Write)
    if (isWrite) {
        while (SDIO->STATUS & (BIT9 | BIT10)) {}
    }

    return true;
}
