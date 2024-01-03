#include <string.h>
#include <stdio.h>
#include "mhscpu.h"
#include "drv_sdcard.h"
#include "drv_sdio.h"
#include "drv_sys.h"
#include "mhscpu_gpio.h"
#include "low_power.h"
#include "user_msg.h"

SDCardInfoStruct SDCardInfo;

bool SDCardSetBusWidth(uint8_t busWidth)
{
    if (!SDIOExecuteCommand(SD_CMD_APP_CMD, SDCardInfo.RCA << 16, SDIOResponseR1, NULL))
        return false;

    if (!SDIOExecuteCommand(SD_CMD_APP_SD_SET_BUSWIDTH, busWidth ? 2 : 0, SDIOResponseR1, NULL))
        return false;

    return true;
}

bool SDCardPowerUp(void)
{
    memset(&SDCardInfo, 0, sizeof(SDCardInfoStruct));

    // Step 1: CMD0 - GO_IDLE_STATE without error
    SDIOExecuteCommand(SD_CMD_GO_IDLE_STATE, 0, SDIOResponseNone, NULL);

    // Step 2: CMD8 - SEND_IF_COND
    if (SDIOExecuteCommand(SD_CMD_HS_SEND_EXT_CSD, SD_CHECK_PATTERN, SDIOResponseR7, NULL)) {
        SDCardInfo.SpecVersion = 2;
    }

    // Step 3: CMD55 - SD_CMD_APP_CMD
    //         ACMD41 - SD_APP_OP_COND
    uint32_t ocr;
    for (int tryCount = 0; tryCount < 500; tryCount++) {
        if (!SDIOExecuteCommand(SD_CMD_APP_CMD, 0, SDIOResponseR1, NULL))
            return false;

        if (!SDIOExecuteCommand(SD_CMD_SD_APP_OP_COND, SD_VOLTAGE_WINDOW_SD | SD_HIGH_CAPACITY, SDIOResponseR3, &ocr))
            return false;

        // Check OCR power up status
        if (ocr & BIT31) {
            if (ocr & BIT30) {
                SDCardInfo.Capacity = SDCardCapacityHigh;
            }
            return true;
        }
    }

    printf("Power up failed, OCR: %08X.\n", ocr);
    return false;
}

void PrintSdCardInfo(SDCardInfoStruct info)
{
    printf("SDCardInfo:\n");
    printf("  ManufacturerID: %02X\n", info.ManufacturerID);
    printf("  ApplicationID: %04X\n", info.ApplicationID);
    printf("  ProductName: %.5s\n", info.ProductName);
    printf("  ProductRevision: %02X\n", info.ProductRevision);
    printf("  ProductSN: %08X\n", info.ProductSN);
    printf("  ManufacturingDate: %06X\n", info.ManufacturingDate);
    printf("  Class: %d\n", info.Class);
    printf("  BlockSize: %u\n", info.BlockSize);
    printf("  DeviceSize: %u MB\n", info.DeviceSize);
    printf("  TransferRate: %d Kbps\n", info.TransferRate);
}

void GPIO_RemapConfiguration(void)
{
    GPIO_InitTypeDef ioConfig;
    ioConfig.GPIO_Mode  = GPIO_Mode_Out_PP;
    ioConfig.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    ioConfig.GPIO_Remap = GPIO_Remap_2;
    GPIO_Init(GPIOD, &ioConfig);

    ioConfig.GPIO_Mode  = GPIO_Mode_Out_PP;
    ioConfig.GPIO_Pin   = GPIO_Pin_6;
    ioConfig.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOD, &ioConfig);
    GPIO_ResetBits(GPIOD, GPIO_Pin_6);
}

bool SDCardSetup(void)
{
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_SDIOM, ENABLE);
    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_SDIOM, ENABLE);
    GPIO_RemapConfiguration();

    SDIO_GlobalITCmd(ENABLE);
    SDIO_SetPowerState(SDIO_PowerState_ON);

    SDIOClockConfig(400, false, true);

    SDIO->INTMASK = 0;

    SDIO_DMAInitTypeDef sdioDMAConfig;
    sdioDMAConfig.SDIO_DMABurstSize     = SDIO_DMA_BurstSize_8;
    sdioDMAConfig.SDIO_DMAReceiveLevel  = SDIO_RXFIFOWMARK_7;
    sdioDMAConfig.SDIO_DMATransmitLevel = SDIO_TXFIFOWMARK_8;
    sdioDMAConfig.SDIO_DMAEnCmd         = ENABLE;
    SDIO_DMAInit(&sdioDMAConfig);

    if (!SDCardPowerUp())
        return false;

    // Get CID
    if (!SDIOExecuteCommand(SD_CMD_ALL_SEND_CID, 0, SDIOResponseR2, SDCardInfo.CID))
        return false;

    // PrintSdCardInfo(SDCardInfo);

    // CMD3: Get RCA
    if (!SDIOExecuteCommand(SD_CMD_SET_REL_ADDR, 0, SDIOResponseR6, (uint32_t *)&SDCardInfo.CardStatus))
        return false;

    // CMD9: Get CSD
    if (!SDIOExecuteCommand(SD_CMD_SEND_CSD, SDCardInfo.RCA << 16, SDIOResponseR2, SDCardInfo.CSD))
        return false;

    static uint8_t  rateValues[] = {0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80};
    static uint16_t rateUnits[]  = {10, 100, 1000, 10000};

    SDCardInfo.Class     = 31 - __CLZ(SDCardInfo.CardCommandClasses);
    SDCardInfo.BlockSize = BIT(SDCardInfo.MaxReadLength);

    SDCardInfo.TransferRate = rateValues[(SDCardInfo.MaxDataRate >> 3) & 0xF] * rateUnits[SDCardInfo.MaxDataRate & 0x7];

    SDCardInfo.DeviceSize = SDCardInfo.CSDStruct ? ((SDCardInfo.V2.DeviceSize + 1) >> 1)
                            : ((SDCardInfo.V1.DeviceSize + 1) << (SDCardInfo.V1.DeviceSizeMultiplier + SDCardInfo.MaxReadLength - 8));

    // CMD7: Select Card
    if (!SDIOExecuteCommand(SD_CMD_SEL_DESEL_CARD, SDCardInfo.RCA << 16, SDIOResponseR1b, NULL))
        return false;

    if (!SDCardSetBusWidth(4))
        return false;

    SDIOClockConfig(SDCardInfo.TransferRate, true, true);

    printf("%s SDCard Setup Done!\n\n", SDCardInfo.Capacity < SDCardCapacityHigh ? "SDSC" : "SDHC");

    return true;
}

bool SDCardErase(uint32_t startAddress, uint32_t endAddress)
{
    if (SDCardInfo.Capacity > SDCardCapacityStandard) {
        startAddress /= 512;
        endAddress /= 512;
    }

    if (!SDIOExecuteCommand(SD_CMD_SD_ERASE_GRP_START, startAddress, SDIOResponseR1, NULL))
        return false;

    if (!SDIOExecuteCommand(SD_CMD_SD_ERASE_GRP_END, endAddress, SDIOResponseR1, NULL))
        return false;

    if (!SDIOExecuteCommand(SD_CMD_ERASE, 0, SDIOResponseR1b, NULL))
        return false;

    return true;
}

bool SDCardSetBlockLength(uint32_t blockLength)
{
    return SDIOExecuteCommand(SD_CMD_SET_BLOCKLEN, blockLength, SDIOResponseR1, NULL);
}

bool SDCardSetBlockCount(uint32_t blockCount)
{
    return SDIOExecuteCommand(SD_CMD_SET_BLOCK_COUNT, blockCount, SDIOResponseR1, NULL);
}

bool SDCardTransferBlock(bool isWrite, uint32_t address, uint8_t* buffer, uint32_t blockCount)
{
    uint32_t addressUnit = 512;
    if (SDCardInfo.Capacity > SDCardCapacityStandard) {
        address /= 512;
        addressUnit = 1;
        osDelay(1);
    }

    if (SDCardInfo.Capacity <= SDCardCapacityStandard && !SDCardSetBlockLength(512))
        return false;

    //    if (!SDCardSetBlockCount(blockCount))
    //        return false;

    uint8_t commandIndex = (isWrite ? SD_CMD_WRITE_SINGLE_BLOCK : SD_CMD_READ_SINGLE_BLOCK);

    for (int i = 0; i < blockCount; i++) {
        if (!SDIOTransferBlock(commandIndex, address + i * addressUnit, isWrite, buffer + i * 512, 512, 1))
            return false;
    }

    return true;
}

SD_Error SD_Init(void)
{
    if (!SDCardSetup()) {
        printf("SDCard Init failed!\n");
        return 1;
    }
    // printf("SDCard Init is Successful!\n");
    return 0;
}

SD_Error SD_ReadBlock(uint8_t* readbuff, uint32_t ReadAddr, uint16_t BlockSize)
{
    if (!SDCardTransferBlock(false, ReadAddr, readbuff, 1)) {
        printf("SDCard read failed!\n");
        return 1;
    }
    return 0;
}

SD_Error SD_WaitReadOperation(void)
{
    SD_Error errorstatus = SD_OK;

    return errorstatus;
}

SD_Error SD_WaitWriteOperation(void)
{
    SD_Error errorstatus = SD_OK;
    return errorstatus;
}

SD_Error SD_WriteBlock(uint8_t* writebuff, uint32_t WriteAddr, uint16_t BlockSize)
{
    if (!SDCardTransferBlock(true, WriteAddr, writebuff, 1)) {
        printf("SDCard write failed!\n");
        return 1;
    }
    return 0;
}

bool SdCardInsert(void)
{
    return GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7) == Bit_RESET;
}

void SdCardIntHandler(void)
{
    if (GetLowPowerState() != LOW_POWER_STATE_WORKING) {
        return;
    }

    PubValueMsg(BACKGROUND_MSG_SD_CARD_CHANGE, 0);
}