#include "drv_parallel8080.h"
#include "stdio.h"
#include "mhscpu.h"
#include "cmsis_os.h"
#include "user_memory.h"
#include "user_delay.h"
#include "user_utils.h"

static void LcdInterfaceInit(void);
static void LcdDmaInit(void);

static LCD_InitTypeDef LCD_InitStructure;
static DMA_InitTypeDef DMA_InitStruct;
static uint8_t *g_dmaBuff;
static uint32_t g_dmaIndex, g_dmaTotal;
static volatile bool g_dmaBusy = false;

void Parallel8080Init(void)
{
    LcdInterfaceInit();
    LcdDmaInit();
}

bool Parallel8080Busy(void)
{
    return g_dmaBusy;
}

void Parallel8080Reset(void)
{
    UserDelay(50);
    PARALLEL_8080_RST_CLR;
    UserDelay(50);
    PARALLEL_8080_RST_SET;
    UserDelay(50);
}

void Parallel8080SendDmaData(uint8_t *data, uint32_t len)
{
    PARALLEL_8080_CS_CLR;
    LCD->lcdi_ctrl |= LCDI_CTRL_CD;

    if (len > PARALLEL_8080_DMA_MAX_BYTE) {
        DMA_SetBlockSize(PARALLEL_8080_DMA_CHANNEL, PARALLEL_8080_DMA_MAX_BYTE);
        g_dmaIndex = PARALLEL_8080_DMA_MAX_BYTE;
    } else {
        DMA_SetBlockSize(PARALLEL_8080_DMA_CHANNEL, len);
        g_dmaIndex = len;
    }
    g_dmaBuff = data;
    g_dmaTotal = len;
    g_dmaBusy = true;
    //DMA_ClearITPendingBit(PARALLEL_8080_DMA_CHANNEL, DMA_IT_DMATransferComplete);
    PARALLEL_8080_DMA_CHANNEL->SAR_L = (uint32_t)g_dmaBuff;
    DMA_ChannelCmd(PARALLEL_8080_DMA_CHANNEL, ENABLE);
}

//void Ili9806ReadData(uint8_t cmd, uint8_t len)
//{
//    uint8_t data = 0x55;
//    printf("read 0x%02X,len=%d\r\n", cmd, len);
//    PARALLEL_8080_CS_CLR;
//    LCDI_Write(LCD, LCDI_CMD, cmd);
//    PARALLEL_8080_CS_SET;
//    UserDelay(1);
//    PARALLEL_8080_CS_CLR;
//    for (uint8_t i = 0; i < len; i++) {
//        LCDI_Read(LCD, LCDI_DAT, &data);
//        printf("%02X ", data);
//    }
//    PARALLEL_8080_CS_SET;
//    printf("\r\n");
//}

static void LcdInterfaceInit(void)
{
    GPIO_InitTypeDef gpioInit = {0};
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO, ENABLE);
    SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_LCD | SYSCTRL_AHBPeriph_DMA, ENABLE);
    SYSCTRL_AHBPeriphResetCmd(SYSCTRL_AHBPeriph_LCD | SYSCTRL_AHBPeriph_DMA, ENABLE);

    GPIO_PinRemapConfig(GPIOC, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | \
                        GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | \
                        GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15, GPIO_Remap_0);

    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = PARALLEL_8080_CS_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(PARALLEL_8080_CS_PORT, &gpioInit);
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = PARALLEL_8080_RST_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(PARALLEL_8080_RST_PORT, &gpioInit);

    LCD_InitStructure.LCD_BusMode = LCDI_MODE_8080;
    LCD_InitStructure.LCD_IntRead = 0;

    LCD_InitStructure.LCD_MaxQTR = 0;

    LCD_InitStructure.fifo_level = 16;
    LCD_InitStructure.fifo_rst_enable = ENABLE;
    LCD_InitStructure.wr_dma_enable = ENABLE;
    LCD_InitStructure.wr_fifo_enable = ENABLE;

    LCDI_Init(LCD, &LCD_InitStructure);
}

static void LcdDmaInit(void)
{
    //memory to peripheral
    DMA_InitStruct.DMA_DIR = DMA_DIR_Memory_To_Peripheral;

    DMA_InitStruct.DMA_MemoryBaseAddr = 0;
    DMA_InitStruct.DMA_MemoryInc = DMA_Inc_Increment;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_DataSize_Byte;
    DMA_InitStruct.DMA_MemoryBurstSize = DMA_BurstSize_8;
    DMA_InitStruct.DMA_BlockSize = 0;

    DMA_InitStruct.DMA_Peripheral = (uint32_t)LCD;
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & (LCD->lcdi_data);
    DMA_InitStruct.DMA_PeripheralInc = DMA_Inc_Nochange;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_DataSize_Byte;
    DMA_InitStruct.DMA_PeripheralBurstSize = DMA_BurstSize_8;
    DMA_InitStruct.DMA_PeripheralHandShake = DMA_PeripheralHandShake_Hardware;
    DMA_Init(PARALLEL_8080_DMA_CHANNEL, &DMA_InitStruct);
    LCDI_FIFODMAconfig(LCD, &LCD_InitStructure);
    DMA_Cmd(ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);
    NVIC_InitStructure.NVIC_IRQChannel = DMA_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DMA_ClearITPendingBit(PARALLEL_8080_DMA_CHANNEL, DMA_IT_DMATransferComplete);
    DMA_ITConfig(PARALLEL_8080_DMA_CHANNEL, DMA_IT_DMATransferComplete, ENABLE);
}

void DMA0_IRQHandler(void)
{
    uint32_t transNum;
    int32_t remaining;

    if (DMA_GetITStatus(PARALLEL_8080_DMA_CHANNEL, DMA_IT_DMATransferComplete) == SET) {
        DMA_ClearITPendingBit(PARALLEL_8080_DMA_CHANNEL, DMA_IT_DMATransferComplete);
        //printf("g_dmaIndex=%d\r\n", g_dmaIndex);
        remaining = g_dmaTotal - g_dmaIndex;
        //printf("remaining=%d\r\n", remaining);
        if (remaining <= 0) {
            g_dmaBusy = false;
            PARALLEL_8080_CS_SET;
        } else {
            if (remaining > PARALLEL_8080_DMA_MAX_BYTE) {
                transNum = PARALLEL_8080_DMA_MAX_BYTE;
            } else {
                transNum = remaining;
                DMA_SetBlockSize(PARALLEL_8080_DMA_CHANNEL, transNum);
            }
            PARALLEL_8080_DMA_CHANNEL->SAR_L = (uint32_t)g_dmaBuff + g_dmaIndex;
            DMA_ChannelCmd(PARALLEL_8080_DMA_CHANNEL, ENABLE);
            g_dmaIndex += transNum;
        }
    }
    NVIC_ClearPendingIRQ(DMA_IRQn);
}
