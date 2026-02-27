/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_dma.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the DMA firmware functions
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "mhscpu_dma.h"

#if defined(__CC_ARM)
#pragma diag_suppress 1296
#elif defined(__ICCARM__)
#elif defined(__GNUC__)
#endif

#define DMA_CHANNEL_0_BIT                               ((uint32_t)0x0001)
#define DMA_CHANNEL_1_BIT                               ((uint32_t)0x0002)
#define DMA_CHANNEL_2_BIT                               ((uint32_t)0x0004)
#define DMA_CHANNEL_3_BIT                               ((uint32_t)0x0008)
#define DMA_CHANNEL_4_BIT                               ((uint32_t)0x0010)
#define DMA_CHANNEL_5_BIT                               ((uint32_t)0x0020)
#define DMA_CHANNEL_6_BIT                               ((uint32_t)0x0040)
#define DMA_CHANNEL_7_BIT                               ((uint32_t)0x0080)

/************ operation definition for DMA  DMA_CTL_H REGISTER ************/
#define DMA_CTL_BLOCK_TS_Pos                                (0)
#define DMA_CTL_BLOCK_TS_Mask                               (0x0fffU<<DMA_CTL_BLOCK_TS_Pos)

/************ operation definition for DMA  DMA_CTL_L REGISTER ************/
#define DMA_CTL_TT_FC_Pos                                   (20)
#define DMA_CTL_TT_FC_Mask                                  (0x07U<<DMA_CTL_TT_FC_Pos)

#define DMA_CTL_TT_FC_Memory_to_Memory_Set                  (0x00U<<DMA_CTL_TT_FC_Pos)
#define DMA_CTL_TT_FC_Memory_to_Peripheral_Set              (0x01U<<DMA_CTL_TT_FC_Pos)
#define DMA_CTL_TT_FC_Peripheral_to_Memory_Set              (0x02U<<DMA_CTL_TT_FC_Pos)
#define DMA_CTL_TT_FC_Peripheral_to_Memory_P_Set            (0x04U<<DMA_CTL_TT_FC_Pos)
#define DMA_CTL_TT_FC_Memory_to_Peripheral_P_Set            (0x06U<<DMA_CTL_TT_FC_Pos)

#define DMA_CTL_SRC_MSIZE_Pos                               (14)
#define DMA_CTL_SRC_MSIZE_Mask                              (0x07U<<DMA_CTL_SRC_MSIZE_Pos)

#define DMA_CTL_DEST_MSIZE_Pos                              (11)
#define DMA_CTL_DEST_MSIZE_Mask                             (0x07U<<DMA_CTL_DEST_MSIZE_Pos)

#define DMA_CTL_SINC_Pos                                    (9)
#define DMA_CTL_SINC_Mask                                   (0x03U<<DMA_CTL_SINC_Pos)

#define DMA_CTL_DINC_Pos                                    (7)
#define DMA_CTL_DINC_Mask                                   (0x03U<<DMA_CTL_DINC_Pos)

#define DMA_CTL_SRC_TR_WIDTH_Pos                            (4)
#define DMA_CTL_SRC_TR_WIDTH_Mask                           (0x07U<<DMA_CTL_SRC_TR_WIDTH_Pos)

#define DMA_CTL_DST_TR_WIDTH_Pos                            (1)
#define DMA_CTL_DST_TR_WIDTH_Mask                           (0x07U<<DMA_CTL_DST_TR_WIDTH_Pos)

#define DMA_CTL_INT_EN_Set                                  ((uint32_t)0x01)

/************ operation definition for DMA  DMA_CFG_L REGISTER ************/
#define DMA_CFG_HS_SEL_SRC_Pos                              (11)
#define DMA_CFG_HS_SEL_SRC_Mask                             (0x01U<<DMA_CFG_HS_SEL_SRC_Pos)//0 HARD 1 SOFT

#define DMA_CFG_HS_SEL_DST_Pos                              (10)
#define DMA_CFG_HS_SEL_DST_Mask                             (0x01U<<DMA_CFG_HS_SEL_DST_Pos)

/************ operation definition for DMA  DMA_CFG_H REGISTER ************/
#define DMA_CFG_DEST_PER_Pos                                (11)
#define DMA_CFG_DEST_PER_Mask                               (0x07U<<DMA_CFG_DEST_PER_Pos)//need write current channel num

#define DMA_CFG_SRC_PER_Pos                                 (7)
#define DMA_CFG_SRC_PER_Mask                                (0x07U<<DMA_CFG_SRC_PER_Pos)//need write current channel num

/************ operation definition for DMA  DMA_LLP_L REGISTER ************/
#define DMAC_LLP_NEXT_LLI_MSK                                   (0x3)

void DMA_ChannelConfig(DMA_TypeDef* DMA_Channelx, uint32_t DMA_Peripheral, uint32_t DMA_DIR);

typedef struct {
    uint32_t DMA_Addr;

    uint32_t DMA_TR_Width;

    uint32_t DMA_Inc;

    uint32_t DMA_HandShake;

    uint32_t DMA_MSize;
} DMA_PeripheralInfo_Def;

typedef enum {
    DMA_Peripheral_Type_Peripheral = ((uint32_t)0x0001),
    DMA_Peripheral_Type_Memory
} DMA_PeripheralType_Def;


static void DMA_GetPeripheralConfig(DMA_PeripheralInfo_Def *DMAPeripheralInfoStruct,
                                    DMA_InitTypeDef* DMA_InitStruct,
                                    DMA_PeripheralType_Def DMAPeripheralType)
{
    if (DMA_Peripheral_Type_Peripheral == DMAPeripheralType) {
        DMAPeripheralInfoStruct->DMA_Addr = DMA_InitStruct->DMA_PeripheralBaseAddr;
        DMAPeripheralInfoStruct->DMA_Inc = DMA_InitStruct->DMA_PeripheralInc;
        DMAPeripheralInfoStruct->DMA_TR_Width =  DMA_InitStruct->DMA_PeripheralDataSize;
        DMAPeripheralInfoStruct->DMA_MSize =  DMA_InitStruct->DMA_PeripheralBurstSize;
        DMAPeripheralInfoStruct->DMA_HandShake =  DMA_InitStruct->DMA_PeripheralHandShake;
    } else if (DMA_Peripheral_Type_Memory == DMAPeripheralType) {
        DMAPeripheralInfoStruct->DMA_Addr = DMA_InitStruct->DMA_MemoryBaseAddr;
        DMAPeripheralInfoStruct->DMA_Inc = DMA_InitStruct->DMA_MemoryInc;
        DMAPeripheralInfoStruct->DMA_TR_Width =  DMA_InitStruct->DMA_MemoryDataSize;
        DMAPeripheralInfoStruct->DMA_MSize =  DMA_InitStruct->DMA_MemoryBurstSize;
        DMAPeripheralInfoStruct->DMA_HandShake =  0;
    }
}

static uint32_t DMA_GetChannelxBit(DMA_TypeDef* DMA_Channelx)
{
    return DMA_CHANNEL_0_BIT << (DMA_Channelx - DMA_Channel_0);
}

FunctionalState DMA_IsChannelEnabled(DMA_TypeDef* DMA_Channelx)
{
    if (DMA->ChEnReg_L & DMA_GetChannelxBit(DMA_Channelx)) {
        return ENABLE;
    }

    return DISABLE;
}


void DMA_Init(DMA_TypeDef* DMA_Channelx, DMA_InitTypeDef* DMA_InitStruct)
{
    uint32_t tmpCtlxReg = 0;
    uint32_t tmpCfgxReg = 0;
    uint32_t tmpChannelxBit = 0;

    DMA_PeripheralInfo_Def DMA_SRC = {0}, DMA_DST = {0};

    switch (DMA_InitStruct->DMA_DIR) {
    case DMA_DIR_Memory_To_Memory:
    case DMA_DIR_Peripheral_To_Memory: {
        DMA_GetPeripheralConfig(&DMA_SRC, DMA_InitStruct, DMA_Peripheral_Type_Peripheral);
        DMA_GetPeripheralConfig(&DMA_DST, DMA_InitStruct, DMA_Peripheral_Type_Memory);
        break;
    }
    case DMA_DIR_Memory_To_Peripheral: {
        DMA_GetPeripheralConfig(&DMA_SRC, DMA_InitStruct, DMA_Peripheral_Type_Memory);
        DMA_GetPeripheralConfig(&DMA_DST, DMA_InitStruct, DMA_Peripheral_Type_Peripheral);
        break;
    }
    }

    DMA_ChannelCmd(DMA_Channelx, DISABLE);

    tmpChannelxBit = DMA_GetChannelxBit(DMA_Channelx);

    if (DMA->ChEnReg_L & tmpChannelxBit)
        return;
    //  Clear any pending interrupts on the channel from the previous DMA transfer by writing to the
    // Interrupt Clear registers: ClearTfr, ClearBlock, ClearSrcTran, ClearDstTran, and ClearErr (page 182).
    // Reading the Interrupt Raw Status and Interrupt Status registers confirms that all interrupts have been
    // cleared.
    DMA->ClearTfr_L = tmpChannelxBit;
    DMA->ClearBlock_L = tmpChannelxBit;
    DMA->ClearSrcTran_L = tmpChannelxBit;
    DMA->ClearDstTran_L = tmpChannelxBit;
    DMA->ClearErr_L = tmpChannelxBit;
    if ((DMA->RawBlock_L & tmpChannelxBit) | (DMA->RawDstTran_L & tmpChannelxBit) | (DMA->RawErr_L & tmpChannelxBit) \
            | (DMA->RawSrcTran_L & tmpChannelxBit) | (DMA->RawTfr_L & tmpChannelxBit)  | (DMA->StatusBlock_L & tmpChannelxBit) \
            | (DMA->StatusDstTran_L & tmpChannelxBit) | (DMA->StatusErr_L & tmpChannelxBit) | (DMA->StatusSrcTran_L & tmpChannelxBit) \
            | (DMA->StatusTfr_L & tmpChannelxBit))
        return;

    //  Program the following channel registers:
    //  a. Write the starting source address in the SARx register for channel x
    DMA_Channelx->SAR_L = DMA_SRC.DMA_Addr;
    //  b. Write the starting destination address in the DARx register for channel x
    DMA_Channelx->DAR_L = DMA_DST.DMA_Addr;

    //  c. Program CTLx and CFGx according to Row 1, as shown in Table 7-1 on page 233. Program the LLPx register with 0 (see page 153).

    //  d. Write the control information for the DMA transfer in the CTLx register for channel x (see
    // page 155). For example, in the register, you can program the following:
    //  i. Set up the transfer type (memory or non-memory peripheral for source and destination) and
    // flow control device by programming the TT_FC of the CTLx register. Table 6-4 on page 161
    // lists the decoding for this field.

    //SET block size
    DMA_Channelx->CTL_H &= ~DMA_CTL_BLOCK_TS_Mask;
    DMA_Channelx->CTL_H |= DMA_InitStruct->DMA_BlockSize;

    tmpCtlxReg &= ~DMA_CTL_SRC_MSIZE_Mask;
    tmpCtlxReg |= DMA_SRC.DMA_MSize << DMA_CTL_SRC_MSIZE_Pos;

    tmpCtlxReg &= ~DMA_CTL_DEST_MSIZE_Mask;
    tmpCtlxReg |= DMA_DST.DMA_MSize << DMA_CTL_DEST_MSIZE_Pos;

    //only use DMAC as Flow Control
    tmpCtlxReg &= ~DMA_CTL_TT_FC_Mask;

    switch (DMA_InitStruct->DMA_DIR) {
    case DMA_DIR_Memory_To_Memory: {
        tmpCtlxReg |= DMA_CTL_TT_FC_Memory_to_Memory_Set;
        break;
    }
    case DMA_DIR_Memory_To_Peripheral: {
        tmpCtlxReg |= DMA_CTL_TT_FC_Memory_to_Peripheral_Set;
        break;
    }
    case DMA_DIR_Peripheral_To_Memory: {
        tmpCtlxReg |= DMA_CTL_TT_FC_Peripheral_to_Memory_Set;
        break;
    }
    default: {
        tmpCtlxReg |= DMA_CTL_TT_FC_Memory_to_Memory_Set;
        break;
    }
    }

    //  ii. Set up the transfer characteristics, such as:
    // Transfer width for the source in the SRC_TR_WIDTH field. Table 6-3 on page 161 lists the
    // decoding for this field.
    tmpCtlxReg &= ~DMA_CTL_SRC_TR_WIDTH_Mask;
    tmpCtlxReg |= (DMA_SRC.DMA_TR_Width << DMA_CTL_SRC_TR_WIDTH_Pos);

    // Transfer width for the destination in the DST_TR_WIDTH field. Table 6-3 on page 161
    // lists the decoding for this field.
    tmpCtlxReg &= ~DMA_CTL_DST_TR_WIDTH_Mask;
    tmpCtlxReg |= (DMA_DST.DMA_TR_Width << DMA_CTL_DST_TR_WIDTH_Pos);

    //Source master layer in the SMS field where the source resides.
    //default is 0;scpu just one ahb master
    //Destination master layer in the DMS field where the destination resides
    //default is 0;scpu just one ahb master

    //  Incrementing/decrementing or fixed address for the source in the SINC field.
    tmpCtlxReg &= ~DMA_CTL_SINC_Mask;
    tmpCtlxReg |= (DMA_SRC.DMA_Inc << DMA_CTL_SINC_Pos);
    //  Incrementing/decrementing or fixed address for the destination in the DINC field.
    tmpCtlxReg &= ~DMA_CTL_DINC_Mask;
    tmpCtlxReg |= (DMA_DST.DMA_Inc << DMA_CTL_DINC_Pos);

    DMA_Channelx->CTL_L = tmpCtlxReg;


    // e. Write the channel configuration information into the CFGx register for channel x (see page 166).
    // i. Designate the handshaking interface type (hardware or software) for the source and
    // destination peripherals; this is not required for memory.

    tmpCfgxReg &= ~(DMA_CFG_HS_SEL_SRC_Mask | DMA_CFG_HS_SEL_DST_Mask);
    //Configure for Default(SoftWare) HankShake
    tmpCfgxReg |= (1 << DMA_CFG_HS_SEL_SRC_Pos) | (1 << DMA_CFG_HS_SEL_DST_Pos);

    if (DMA_DIR_Memory_To_Memory == DMA_InitStruct->DMA_DIR) {
        // memory to memory do not need set the SYSCTRL HardShake bits
    } else {
        if (DMA_DIR_Peripheral_To_Memory == DMA_InitStruct->DMA_DIR) {
            tmpCfgxReg &= ~DMA_CFG_HS_SEL_SRC_Mask;
            tmpCfgxReg |= DMA_SRC.DMA_HandShake << DMA_CFG_HS_SEL_SRC_Pos; //SRC HandShake Config
        }
        if (DMA_DIR_Memory_To_Peripheral == DMA_InitStruct->DMA_DIR) {
            tmpCfgxReg &= ~DMA_CFG_HS_SEL_DST_Mask;
            tmpCfgxReg |= DMA_DST.DMA_HandShake << DMA_CFG_HS_SEL_DST_Pos; //DTS HandShake Config
        }
    }
    DMA_Channelx->CFG_L = tmpCfgxReg;


    //  ii. If the hardware handshaking interface is activated for the source or destination peripheral,
    // assign a handshaking interface to the source and destination peripheral; this requires
    // programming the SRC_PER and DEST_PER bits, respectively
    DMA_Channel_0->CFG_H |= (0 << DMA_CFG_DEST_PER_Pos) | (0 << DMA_CFG_SRC_PER_Pos);
    DMA_Channel_1->CFG_H |= (1 << DMA_CFG_DEST_PER_Pos) | (1 << DMA_CFG_SRC_PER_Pos);
    DMA_Channel_2->CFG_H |= (2 << DMA_CFG_DEST_PER_Pos) | (2 << DMA_CFG_SRC_PER_Pos);
    DMA_Channel_3->CFG_H |= (3 << DMA_CFG_DEST_PER_Pos) | (3 << DMA_CFG_SRC_PER_Pos);
    DMA_Channel_4->CFG_H |= (4 << DMA_CFG_DEST_PER_Pos) | (4 << DMA_CFG_SRC_PER_Pos);
    DMA_Channel_5->CFG_H |= (5 << DMA_CFG_DEST_PER_Pos) | (5 << DMA_CFG_SRC_PER_Pos);
    DMA_Channel_6->CFG_H |= (6 << DMA_CFG_DEST_PER_Pos) | (6 << DMA_CFG_SRC_PER_Pos);
    DMA_Channel_7->CFG_H |= (7 << DMA_CFG_DEST_PER_Pos) | (7 << DMA_CFG_SRC_PER_Pos);

    if (DMA_DIR_Memory_To_Memory != DMA_InitStruct->DMA_DIR)
        DMA_ChannelConfig(DMA_Channelx, DMA_InitStruct->DMA_Peripheral, DMA_InitStruct->DMA_DIR);

    //  4. Ensure that bit 0 of the DmaCfgReg register is enabled before writing to ChEnReg.
    DMA_Cmd(ENABLE);
}

void DMA_ChannelCmd(DMA_TypeDef* DMA_Channelx, FunctionalState NewState)
{
    uint32_t tmpChannelxBit = 0;
    tmpChannelxBit = DMA_GetChannelxBit(DMA_Channelx);

    if (NewState != DISABLE) {
        DMA->ChEnReg_L = (tmpChannelxBit << 8) | tmpChannelxBit;
    } else {
        DMA->ChEnReg_L = tmpChannelxBit << 8 | 0;
    }
}

void DMA_Cmd(FunctionalState NewState)
{
    if (NewState != DISABLE) {
        DMA->DmaCfgReg_L = 1;
    } else {
        DMA->DmaCfgReg_L = 0;
    }
}


void DMA_SetSrcSingleReq(DMA_TypeDef* DMA_Channelx)
{
    uint32_t tmpChannelxBit = 0;
    tmpChannelxBit = DMA_GetChannelxBit(DMA_Channelx);
    DMA->SglReqSrcReg_L = tmpChannelxBit;
    DMA->ReqSrcReg_L = tmpChannelxBit;
}

void DMA_SetSrcBlockReq(DMA_TypeDef* DMA_Channelx)
{
    uint32_t tmpChannelxBit = 0;
    tmpChannelxBit = DMA_GetChannelxBit(DMA_Channelx);
    DMA->ReqSrcReg_L = tmpChannelxBit;
    DMA->SglReqSrcReg_L = tmpChannelxBit;
}


void DMA_SetSRCAddress(DMA_TypeDef* DMA_Channelx, uint32_t Address)
{
    //  a. Write the starting source address in the SARx register for channel x
    DMA_Channelx->SAR_L = Address;
}

void DMA_SetDSRAddress(DMA_TypeDef* DMA_Channelx, uint32_t Address)
{
    //  b. Write the starting destination address in the DARx register for channel x
    DMA_Channelx->DAR_L = Address;
}

void DMA_SetBlockSize(DMA_TypeDef* DMA_Channelx, uint32_t BlockSize)
{
    DMA_Channelx->CTL_H = (DMA_Channelx->CTL_H & ~DMA_CTL_BLOCK_TS_Mask) | BlockSize;
}

void DMA_ITConfig(DMA_TypeDef* DMA_Channelx, uint32_t DMA_IT, FunctionalState NewState)
{
    uint32_t  i = 0;
    uint32_t tmpChannelxBit = 0;
    tmpChannelxBit = DMA_GetChannelxBit(DMA_Channelx);

    if (NewState != DISABLE) {
        DMA_Channelx->CTL_L |= DMA_CTL_INT_EN_Set;
    } else {
        DMA_Channelx->CTL_L &= ~DMA_CTL_INT_EN_Set;
        tmpChannelxBit &= ~((uint32_t)0x1f);
    }

    for (i = 0; i < 5; i++) {
        uint32_t tmp_DMA_IT = DMA_IT & (((uint32_t)0x01) << i);

        switch (tmp_DMA_IT) {
        case DMA_IT_BlockTransferComplete:
            DMA->MaskBlock_L = (tmpChannelxBit << 8 | tmpChannelxBit);
            break;
        case DMA_IT_DestinationTransactionComplete:
            DMA->MaskDstTran_L = (tmpChannelxBit << 8 | tmpChannelxBit);
            break;
        case DMA_IT_Error:
            DMA->MaskErr_L = (tmpChannelxBit << 8 | tmpChannelxBit);
            break;
        case DMA_IT_SourceTransactionComplete:
            DMA->MaskSrcTran_L = (tmpChannelxBit << 8 | tmpChannelxBit);
            break;
        case DMA_IT_DMATransferComplete:
            DMA->MaskTfr_L = (tmpChannelxBit << 8 | tmpChannelxBit);
            break;
        }
    }
}

void DMA_ClearITPendingBit(DMA_TypeDef* DMA_Channelx, uint32_t DMA_IT)
{
    uint32_t tmpChannelxBit = 0;
    tmpChannelxBit = DMA_GetChannelxBit(DMA_Channelx);

    switch (DMA_IT) {
    case DMA_IT_BlockTransferComplete:
        DMA->ClearBlock_L = tmpChannelxBit;
        break;
    case DMA_IT_DestinationTransactionComplete:
        DMA->ClearDstTran_L = tmpChannelxBit;
        break;
    case DMA_IT_Error:
        DMA->ClearErr_L = tmpChannelxBit;
        break;
    case DMA_IT_SourceTransactionComplete:
        DMA->ClearSrcTran_L = tmpChannelxBit;
        break;
    case DMA_IT_DMATransferComplete:
        DMA->ClearTfr_L = tmpChannelxBit;
        break;
    }
}

ITStatus DMA_GetITStatus(DMA_TypeDef* DMA_Channelx, uint32_t DMA_IT)
{
    uint32_t DMA_IT_Status = 0;

    switch (DMA_IT) {
    case DMA_IT_BlockTransferComplete:
        DMA_IT_Status = DMA->StatusBlock_L;
        break;
    case DMA_IT_DestinationTransactionComplete:
        DMA_IT_Status = DMA->StatusDstTran_L;
        break;
    case DMA_IT_Error:
        DMA_IT_Status = DMA->StatusErr_L;
        break;
    case DMA_IT_SourceTransactionComplete:
        DMA_IT_Status = DMA->StatusSrcTran_L;
        break;
    case DMA_IT_DMATransferComplete:
        DMA_IT_Status = DMA->StatusTfr_L;
        break;
    }

    if ((DMA_IT_Status & DMA_GetChannelxBit(DMA_Channelx)) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}


FlagStatus DMA_GetRawStatus(DMA_TypeDef* DMA_Channelx, uint32_t DMA_IT)
{
    uint32_t DMA_Raw_Status = 0;

    switch (DMA_IT) {
    case DMA_IT_BlockTransferComplete:
        DMA_Raw_Status = DMA->RawBlock_L;
        break;
    case DMA_IT_DestinationTransactionComplete:
        DMA_Raw_Status = DMA->RawDstTran_L;
        break;
    case DMA_IT_Error:
        DMA_Raw_Status = DMA->RawErr_L;
        break;
    case DMA_IT_SourceTransactionComplete:
        DMA_Raw_Status = DMA->RawSrcTran_L;
        break;
    case DMA_IT_DMATransferComplete:
        DMA_Raw_Status = DMA->RawTfr_L;
        break;
    }

    if (DMA_Raw_Status & DMA_GetChannelxBit(DMA_Channelx)) {
        return SET;
    } else {
        return RESET;
    }
}

void DMA_ChannelConfig(DMA_TypeDef* DMA_Channelx, uint32_t DMA_Peripheral, uint32_t DMA_DIR)
{
    uint32_t tmpDMA_CHx_IFx = 0;

    if (DMA_DIR_Peripheral_To_Memory == DMA_DIR) {
        switch (DMA_Peripheral) {
        case (uint32_t)(UART0):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART0_RX;
            break;
        case (uint32_t)(UART1):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART1_RX;
            break;
        case (uint32_t)(UART2):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART2_RX;
            break;
        case (uint32_t)(UART3):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART3_RX;
            break;
        case (uint32_t)(SPIS0):
        case (uint32_t)(SPIM0):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI0_RX;
            break;
        case (uint32_t)(SPIM1):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI1_RX;
            break;
        case (uint32_t)(SPIM2):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI2_RX;
            break;
        case (uint32_t)(SPIM3):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI3_RX;
            break;
        case (uint32_t)(SPIM4):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI4_RX;
            break;
        case (uint32_t)(I2C0):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_I2C_RX;
            break;
        case (uint32_t)(SDIO):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_SDIOM_RX;
            break;
        }
    } else if (DMA_DIR_Memory_To_Peripheral == DMA_DIR) {
        switch (DMA_Peripheral) {
        case (uint32_t)(DCMI):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_DCMI_TX;
            break;
        case (uint32_t)(UART0):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART0_TX;
            break;
        case (uint32_t)(UART1):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART1_TX;
            break;
        case (uint32_t)(UART2):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART2_TX;
            break;
        case (uint32_t)(UART3):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART3_TX;
            break;
        case (uint32_t)(SPIS0):
        case (uint32_t)(SPIM0):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI0_TX;
            break;
        case (uint32_t)(SPIM1):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI1_TX;
            break;
        case (uint32_t)(SPIM2):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI2_TX;
            break;
        case (uint32_t)(SPIM3):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI3_TX;
            break;
        case (uint32_t)(SPIM4):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI4_TX;
            break;
        case (uint32_t)(I2C0):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_I2C_TX;
            break;
        case (uint32_t)(SDIO):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_SDIOM_TX;
            break;
        case (uint32_t)(QSPI):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_QSPI_TX;
            break;
        case (uint32_t)(LCD):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_LCD;
            break;
        case (uint32_t)(DAC):
            tmpDMA_CHx_IFx = SYSCTRL_PHER_CTRL_DMA_CHx_IF_DAC;
            break;
        }
    }

    if (DMA_Channelx == DMA_Channel_0) {
        SYSCTRL->DMA_CHAN = (SYSCTRL->DMA_CHAN & ~SYSCTRL_PHER_CTRL_DMA_CH0_IF_Mask) | (tmpDMA_CHx_IFx << SYSCTRL_PHER_CTRL_DMA_CH0_IF_Pos);
    } else if (DMA_Channelx == DMA_Channel_1) {
        SYSCTRL->DMA_CHAN = (SYSCTRL->DMA_CHAN & ~SYSCTRL_PHER_CTRL_DMA_CH1_IF_Mask) | (tmpDMA_CHx_IFx << SYSCTRL_PHER_CTRL_DMA_CH1_IF_Pos);
    } else if (DMA_Channelx == DMA_Channel_2) {
        SYSCTRL->DMA_CHAN = (SYSCTRL->DMA_CHAN & ~SYSCTRL_PHER_CTRL_DMA_CH2_IF_Mask) | (tmpDMA_CHx_IFx << SYSCTRL_PHER_CTRL_DMA_CH2_IF_Pos);
    } else if (DMA_Channelx == DMA_Channel_3) {
        SYSCTRL->DMA_CHAN = (SYSCTRL->DMA_CHAN & ~SYSCTRL_PHER_CTRL_DMA_CH3_IF_Mask) | (tmpDMA_CHx_IFx << SYSCTRL_PHER_CTRL_DMA_CH3_IF_Pos);
    } else if (DMA_Channelx == DMA_Channel_4) {
        SYSCTRL->DMA_CHAN1 = (SYSCTRL->DMA_CHAN1 & ~SYSCTRL_PHER_CTRL_DMA_CH4_IF_Mask) | (tmpDMA_CHx_IFx << SYSCTRL_PHER_CTRL_DMA_CH4_IF_Pos);
    } else if (DMA_Channelx == DMA_Channel_5) {
        SYSCTRL->DMA_CHAN1 = (SYSCTRL->DMA_CHAN1 & ~SYSCTRL_PHER_CTRL_DMA_CH5_IF_Mask) | (tmpDMA_CHx_IFx << SYSCTRL_PHER_CTRL_DMA_CH5_IF_Pos);
    } else if (DMA_Channelx == DMA_Channel_6) {
        SYSCTRL->DMA_CHAN1 = (SYSCTRL->DMA_CHAN1 & ~SYSCTRL_PHER_CTRL_DMA_CH6_IF_Mask) | (tmpDMA_CHx_IFx << SYSCTRL_PHER_CTRL_DMA_CH6_IF_Pos);
    } else if (DMA_Channelx == DMA_Channel_7) {
        SYSCTRL->DMA_CHAN1 = (SYSCTRL->DMA_CHAN1 & ~SYSCTRL_PHER_CTRL_DMA_CH7_IF_Mask) | (tmpDMA_CHx_IFx << SYSCTRL_PHER_CTRL_DMA_CH7_IF_Pos);
    }
}

//Init DMA
void DMA_MultiBlockInit(DMA_TypeDef* DMA_Channelx, DMA_InitTypeDef* DMA_InitStruct, \
                        LLI *first_lli, uint8_t Multi_Block_Mode)
{
    uint32_t tmpCtlxReg = 0;
    uint32_t tmpCfgxReg = 0;
    uint32_t tmpChannelxBit = 0;

    DMA_PeripheralInfo_Def DMA_SRC = {0}, DMA_DST = {0};

    switch (DMA_InitStruct->DMA_DIR) {
    case DMA_DIR_Memory_To_Memory:
    case DMA_DIR_Peripheral_To_Memory: {
        DMA_GetPeripheralConfig(&DMA_SRC, DMA_InitStruct, DMA_Peripheral_Type_Peripheral);
        DMA_GetPeripheralConfig(&DMA_DST, DMA_InitStruct, DMA_Peripheral_Type_Memory);
        break;
    }
    case DMA_DIR_Memory_To_Peripheral: {
        DMA_GetPeripheralConfig(&DMA_SRC, DMA_InitStruct, DMA_Peripheral_Type_Memory);
        DMA_GetPeripheralConfig(&DMA_DST, DMA_InitStruct, DMA_Peripheral_Type_Peripheral);
        break;
    }
    }

    DMA_ChannelCmd(DMA_Channelx, DISABLE);

    tmpChannelxBit = DMA_GetChannelxBit(DMA_Channelx);

    if (DMA->ChEnReg_L & tmpChannelxBit)
        return;
    //  Clear any pending interrupts on the channel from the previous DMA transfer by writing to the
    // Interrupt Clear registers: ClearTfr, ClearBlock, ClearSrcTran, ClearDstTran, and ClearErr (page 182).
    // Reading the Interrupt Raw Status and Interrupt Status registers confirms that all interrupts have been
    // cleared.
    DMA->ClearTfr_L = tmpChannelxBit;
    DMA->ClearBlock_L = tmpChannelxBit;
    DMA->ClearSrcTran_L = tmpChannelxBit;
    DMA->ClearDstTran_L = tmpChannelxBit;
    DMA->ClearErr_L = tmpChannelxBit;
    if ((DMA->RawBlock_L & tmpChannelxBit) | (DMA->RawDstTran_L & tmpChannelxBit) | (DMA->RawErr_L & tmpChannelxBit) \
            | (DMA->RawSrcTran_L & tmpChannelxBit) | (DMA->RawTfr_L & tmpChannelxBit)  | (DMA->StatusBlock_L & tmpChannelxBit) \
            | (DMA->StatusDstTran_L & tmpChannelxBit) | (DMA->StatusErr_L & tmpChannelxBit) | (DMA->StatusSrcTran_L & tmpChannelxBit) \
            | (DMA->StatusTfr_L & tmpChannelxBit))
        return;

    //  Program the following channel registers:
    //  a. Write the starting source address in the SARx register for channel x
    DMA_Channelx->SAR_L = DMA_SRC.DMA_Addr;
    //  b. Write the starting destination address in the DARx register for channel x
    DMA_Channelx->DAR_L = DMA_DST.DMA_Addr;

    //  c. Program CTLx and CFGx according to Row 1, as shown in Table 7-1 on page 233. Program the LLPx register with 0 (see page 153).

    //  d. Write the control information for the DMA transfer in the CTLx register for channel x (see
    // page 155). For example, in the register, you can program the following:
    //  i. Set up the transfer type (memory or non-memory peripheral for source and destination) and
    // flow control device by programming the TT_FC of the CTLx register. Table 6-4 on page 161
    // lists the decoding for this field.

    //SET block size
    DMA_Channelx->CTL_H &= ~DMA_CTL_BLOCK_TS_Mask;
    DMA_Channelx->CTL_H |= DMA_InitStruct->DMA_BlockSize;

    tmpCtlxReg &= ~DMA_CTL_SRC_MSIZE_Mask;
    tmpCtlxReg |= DMA_SRC.DMA_MSize << DMA_CTL_SRC_MSIZE_Pos;

    tmpCtlxReg &= ~DMA_CTL_DEST_MSIZE_Mask;
    tmpCtlxReg |= DMA_DST.DMA_MSize << DMA_CTL_DEST_MSIZE_Pos;

    //only use DMAC as Flow Control
    tmpCtlxReg &= ~DMA_CTL_TT_FC_Mask;

    switch (DMA_InitStruct->DMA_DIR) {
    case DMA_DIR_Memory_To_Memory: {
        tmpCtlxReg |= DMA_CTL_TT_FC_Memory_to_Memory_Set;
        break;
    }
    case DMA_DIR_Memory_To_Peripheral: {
        tmpCtlxReg |= DMA_CTL_TT_FC_Memory_to_Peripheral_Set;
        break;
    }
    case DMA_DIR_Peripheral_To_Memory: {
        tmpCtlxReg |= DMA_CTL_TT_FC_Peripheral_to_Memory_Set;
        break;
    }
    default: {
        tmpCtlxReg |= DMA_CTL_TT_FC_Memory_to_Memory_Set;
        break;
    }
    }

    //  ii. Set up the transfer characteristics, such as:
    // Transfer width for the source in the SRC_TR_WIDTH field. Table 6-3 on page 161 lists the
    // decoding for this field.
    tmpCtlxReg &= ~DMA_CTL_SRC_TR_WIDTH_Mask;
    tmpCtlxReg |= (DMA_SRC.DMA_TR_Width << DMA_CTL_SRC_TR_WIDTH_Pos);

    // Transfer width for the destination in the DST_TR_WIDTH field. Table 6-3 on page 161
    // lists the decoding for this field.
    tmpCtlxReg &= ~DMA_CTL_DST_TR_WIDTH_Mask;
    tmpCtlxReg |= (DMA_DST.DMA_TR_Width << DMA_CTL_DST_TR_WIDTH_Pos);

    //Source master layer in the SMS field where the source resides.
    //default is 0;scpu just one ahb master
    //Destination master layer in the DMS field where the destination resides
    //default is 0;scpu just one ahb master

    //  Incrementing/decrementing or fixed address for the source in the SINC field.
    tmpCtlxReg &= ~DMA_CTL_SINC_Mask;
    tmpCtlxReg |= (DMA_SRC.DMA_Inc << DMA_CTL_SINC_Pos);
    //  Incrementing/decrementing or fixed address for the destination in the DINC field.
    tmpCtlxReg &= ~DMA_CTL_DINC_Mask;
    tmpCtlxReg |= (DMA_DST.DMA_Inc << DMA_CTL_DINC_Pos);

    DMA_Channelx->CTL_L = tmpCtlxReg;


    // e. Write the channel configuration information into the CFGx register for channel x (see page 166).
    // i. Designate the handshaking interface type (hardware or software) for the source and
    // destination peripherals; this is not required for memory.

    tmpCfgxReg &= ~(DMA_CFG_HS_SEL_SRC_Mask | DMA_CFG_HS_SEL_DST_Mask);
    //Configure for Default(SoftWare) HankShake
    tmpCfgxReg |= (1 << DMA_CFG_HS_SEL_SRC_Pos) | (1 << DMA_CFG_HS_SEL_DST_Pos);

    if (DMA_DIR_Memory_To_Memory == DMA_InitStruct->DMA_DIR) {
        // memory to memory do not need set the SYSCTRL HardShake bits
    } else {
        if (DMA_DIR_Peripheral_To_Memory == DMA_InitStruct->DMA_DIR) {
            tmpCfgxReg &= ~DMA_CFG_HS_SEL_SRC_Mask;
            tmpCfgxReg |= DMA_SRC.DMA_HandShake << DMA_CFG_HS_SEL_SRC_Pos; //SRC HandShake Config
        }
        if (DMA_DIR_Memory_To_Peripheral == DMA_InitStruct->DMA_DIR) {
            tmpCfgxReg &= ~DMA_CFG_HS_SEL_DST_Mask;
            tmpCfgxReg |= DMA_DST.DMA_HandShake << DMA_CFG_HS_SEL_DST_Pos; //DTS HandShake Config
        }
    }
    DMA_Channelx->CFG_L = tmpCfgxReg;


    //  ii. If the hardware handshaking interface is activated for the source or destination peripheral,
    // assign a handshaking interface to the source and destination peripheral; this requires
    // programming the SRC_PER and DEST_PER bits, respectively
    DMA_Channel_0->CFG_H |= (0 << DMA_CFG_DEST_PER_Pos) | (0 << DMA_CFG_SRC_PER_Pos);
    DMA_Channel_1->CFG_H |= (1 << DMA_CFG_DEST_PER_Pos) | (1 << DMA_CFG_SRC_PER_Pos);
    DMA_Channel_2->CFG_H |= (2 << DMA_CFG_DEST_PER_Pos) | (2 << DMA_CFG_SRC_PER_Pos);
    DMA_Channel_3->CFG_H |= (3 << DMA_CFG_DEST_PER_Pos) | (3 << DMA_CFG_SRC_PER_Pos);
    DMA_Channel_4->CFG_H |= (4 << DMA_CFG_DEST_PER_Pos) | (4 << DMA_CFG_SRC_PER_Pos);
    DMA_Channel_5->CFG_H |= (5 << DMA_CFG_DEST_PER_Pos) | (5 << DMA_CFG_SRC_PER_Pos);
    DMA_Channel_6->CFG_H |= (6 << DMA_CFG_DEST_PER_Pos) | (6 << DMA_CFG_SRC_PER_Pos);
    DMA_Channel_7->CFG_H |= (7 << DMA_CFG_DEST_PER_Pos) | (7 << DMA_CFG_SRC_PER_Pos);

    if (DMA_DIR_Memory_To_Memory != DMA_InitStruct->DMA_DIR)
        DMA_ChannelConfig(DMA_Channelx, DMA_InitStruct->DMA_Peripheral, DMA_InitStruct->DMA_DIR);

    DMA_Channelx->LLP_L &= DMAC_LLP_NEXT_LLI_MSK  ;

    switch (Multi_Block_Mode) {
    case Multi_Block_MODE02 :
        DMA_Channelx->LLP_L &= DMAC_LLP_NEXT_LLI_MSK;
        DMA_Channelx->CFG_L &= ~(1 << 30);
//          DMA_Channelx->CFG_L |= 1 << 31;
        DMA_Channelx->CFG_L |= 0x80000000;
        DMA_Channelx->CTL_L &= ~(1 << 28);
        DMA_Channelx->CTL_L &= ~(1 << 27);
        DMA_Channelx->CTL_H |= 1 << 12;
        break;
    case Multi_Block_MODE03 :
        DMA_Channelx->LLP_L &= DMAC_LLP_NEXT_LLI_MSK;
        DMA_Channelx->CFG_L |= 1 << 30;
        DMA_Channelx->CFG_L &= ~(0x80000000);
        DMA_Channelx->CTL_L &= ~(1 << 28);
        DMA_Channelx->CTL_L &= ~(1 << 27);
        DMA_Channelx->CTL_H |= 1 << 12;
        break;
    case Multi_Block_MODE04 :
        DMA_Channelx->LLP_L &= DMAC_LLP_NEXT_LLI_MSK;
        DMA_Channelx->CFG_L |= 1 << 30;
        DMA_Channelx->CFG_L |= 0x80000000;
        DMA_Channelx->CTL_L &= ~(1 << 28);
        DMA_Channelx->CTL_L &= ~(1 << 27);
        DMA_Channelx->CTL_H |= 1 << 12;
        break;
    case Multi_Block_MODE05 :
        DMA_Channelx->LLP_L |= ((uint32_t)first_lli) & ~DMAC_LLP_NEXT_LLI_MSK  ;
        DMA_Channelx->CFG_L &= ~(1 << 30);
        DMA_Channelx->CFG_L &= ~(0x80000000);
        DMA_Channelx->CTL_L &= ~(1 << 28);
        DMA_Channelx->CTL_L &= ~(1 << 27);
        DMA_Channelx->CTL_H |= 1 << 12;
        break;
    case Multi_Block_MODE06 :
        DMA_Channelx->LLP_L |= (((uint32_t)first_lli) & ~DMAC_LLP_NEXT_LLI_MSK) ;
        DMA_Channelx->CFG_L &= ~(1 << 30);
        DMA_Channelx->CFG_L &= ~(0x80000000);
        DMA_Channelx->CTL_L &= ~(1 << 28);
        DMA_Channelx->CTL_L |= 1 << 27;
        DMA_Channelx->CTL_H |= 1 << 12;
        break;
    case Multi_Block_MODE07 :
        DMA_Channelx->LLP_L |= (((uint32_t)first_lli) & ~DMAC_LLP_NEXT_LLI_MSK) ;
        DMA_Channelx->CFG_L |= 1 << 30;
        DMA_Channelx->CFG_L &= ~(0x80000000);
        DMA_Channelx->CTL_L &= ~(1 << 28);
        DMA_Channelx->CTL_L |= 1 << 27;
        DMA_Channelx->CTL_H |= 1 << 12;
        break;
    case Multi_Block_MODE08 :
        DMA_Channelx->LLP_L |= ((uint32_t)first_lli) & ~DMAC_LLP_NEXT_LLI_MSK  ;
        DMA_Channelx->CFG_L &= ~(1 << 30);
        DMA_Channelx->CFG_L &= ~(0x80000000);
        DMA_Channelx->CTL_L |= 1 << 28;
        DMA_Channelx->CTL_L &= ~(1 << 27);
        DMA_Channelx->CTL_H |= 1 << 12;
        break;
    case Multi_Block_MODE09 :
        DMA_Channelx->LLP_L |= ((uint32_t)first_lli) & ~DMAC_LLP_NEXT_LLI_MSK  ;
        DMA_Channelx->CFG_L &= ~(1 << 30);
        DMA_Channelx->CFG_L |= 0x80000000;
        DMA_Channelx->CTL_L |= 1 << 28;
        DMA_Channelx->CTL_L &= ~(1 << 27);
        DMA_Channelx->CTL_H |= 1 << 12;
        break;
    case Multi_Block_MODE10 :
        DMA_Channelx->LLP_L |= ((uint32_t)first_lli) & ~DMAC_LLP_NEXT_LLI_MSK  ;
        DMA_Channelx->CFG_L &= ~(1 << 30);
        DMA_Channelx->CFG_L &= ~(0x80000000);
        DMA_Channelx->CTL_L |= 1 << 28;
        DMA_Channelx->CTL_L |= 1 << 27;
        DMA_Channelx->CTL_H |= (1 << 12);
        break;
    case Multi_Block_MODE01 :
    default:
        DMA_Channelx->LLP_L &= DMAC_LLP_NEXT_LLI_MSK;
        DMA_Channelx->CFG_L &= ~(1 << 30);
        DMA_Channelx->CFG_L &= ~(0x80000000);
        DMA_Channelx->CTL_L &= ~(1 << 28);
        DMA_Channelx->CTL_L &= ~(1 << 27);
        DMA_Channelx->CTL_H |= 1 << 12;
        break;
    }

    //  4. Ensure that bit 0 of the DmaCfgReg register is enabled before writing to ChEnReg.
    DMA_Cmd(ENABLE);
}

void DMA_InitLLI(DMA_TypeDef *DMA_Channelx, LLI *lli, LLI *next_lli, void *src_addr, void *dest_addr, uint16_t btsize)
{
    lli->SAR = (uint32_t)src_addr;
    lli->DAR = (uint32_t)dest_addr;

    lli->LLP = DMA_Channelx->LLP_L & DMAC_LLP_NEXT_LLI_MSK;
    lli->LLP |= ((uint32_t)next_lli) & ~DMAC_LLP_NEXT_LLI_MSK;
    lli->CTL_L = DMA_Channelx->CTL_L;
//  lli->CTL_H &= ~DMA_CTL_BLOCK_TS_Mask;
    lli->CTL_H = btsize;
    lli->CTL_H |= 1 << 12;
    lli->DSTAT = 0;
}

uint32_t DMA_GetTransferNum(DMA_TypeDef* DMA_Channelx, uint32_t *first_adr)
{
    return (DMA_Channelx->DAR_L - (uint32_t)(first_adr));
}

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
