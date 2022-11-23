/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_lcdi.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the LCDI firmware functions
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "mhscpu_lcdi.h"

#define LCDI_FIFO_DEPTH         (16)

void LCDI_BusRead(LCD_TypeDef *LCDx, uint8_t u8CD)
{
    if (LCDI_CTRL_TYPE_8080 != (LCDI_CTRL_TYPE_8080 & LCD->lcdi_ctrl)) {
        //In 6800 mode we should set /WR signal manually.
        LCDx->lcdi_ctrl |= LCDI_CTRL_RW_WR;
    }

    // Set CD(A0) signal.0:Instrcution; 1:Data
    if (LCDI_CMD == u8CD) {
        LCDx->lcdi_ctrl &= ~LCDI_CTRL_CD;
    } else {
        LCDx->lcdi_ctrl |= LCDI_CTRL_CD;
    }

    LCDx->lcdi_ctrl |= LCDI_CTRL_AUTO_READ;
}

void LCDI_BusWrite(LCD_TypeDef *LCDx, uint8_t u8CD, uint8_t value)
{
    if (LCDI_CTRL_TYPE_8080 != (LCDI_CTRL_TYPE_8080 & LCD->lcdi_ctrl)) {
        //In 6800 mode we should set /WR signal manually.
        LCDx->lcdi_ctrl &= ~LCDI_CTRL_RW_WR;
    }

    //Set CD(A0) signal.0:Instrcution; 1:Data
    if (LCDI_CMD == u8CD) {
        LCDx->lcdi_ctrl &= ~LCDI_CTRL_CD;
    } else {
        LCDx->lcdi_ctrl |= LCDI_CTRL_CD;
    }

    //Write to lcdi_data will trigger write timing.
    LCDx->lcdi_data = value;
}

void LCDI_FIFODMAconfig(LCD_TypeDef *LCDx, LCD_InitTypeDef *LCD_InitType)
{
    if (LCD_InitType->fifo_rst_enable) {
        LCDx->lcdi_ctrl |= LCDI_FIFO_RST;
    }

    if (LCD_InitType->wr_dma_enable) {
        LCDx->lcdi_ctrl |= LCDI_WR_DMA_EN;
    } else {
        LCDx->lcdi_ctrl &= ~LCDI_WR_DMA_EN;
    }

    if (LCD_InitType->wr_fifo_enable) {
        LCDx->lcdi_ctrl |= LCDI_WR_FIFO_EN;
    } else {
        LCDx->lcdi_ctrl &= ~LCDI_WR_FIFO_EN;
    }

    LCDx->lcdi_fifothr = LCD_InitType->fifo_level % LCDI_FIFO_DEPTH;
}

static void LCD_SetInt(LCD_TypeDef *LCDx, uint8_t u8RW, uint8_t u8Enable)
{
    uint32_t u32Bit;

    if (u8RW) {
        //Set read
        u32Bit = LCDI_CTRL_RD_IE;
    } else {
        //Set write
        u32Bit = LCDI_CTRL_WR_IE;
    }

    if (u8Enable) {
        while (0 == (LCDx->lcdi_ctrl & u32Bit)) {
            LCDx->lcdi_ctrl |= u32Bit;
        }
    } else {
        while (LCDx->lcdi_ctrl & u32Bit) {
            LCDx->lcdi_ctrl &= ~u32Bit;
        }
    }
}

//Write with no pendding.
int32_t LCDI_WriteBuff(LCD_TypeDef *LCDx, LCD_InitTypeDef *pLcdInit, uint8_t *pu8Buff, uint32_t u32BuffLen)
{
    uint32_t i, u32Cmd, u32Len = RNG_BUF_LEN(pLcdInit->ring_buf.prbWrite);

    if (u32BuffLen > BIT(30) || RNG_BUF_IS_FULL(pLcdInit->ring_buf.prbCmd) || pLcdInit->ring_buf.prbWrite->u32BuffSize < u32Len + u32BuffLen + 1) {
        return -1;
    }
    for (i = 0; i < u32BuffLen; i++) {
        pLcdInit->ring_buf.prbWrite->pu8Buff[pLcdInit->ring_buf.prbWrite->u32Tail] = pu8Buff[i];
        pLcdInit->ring_buf.prbWrite->u32Tail = RNG_BUF_NEXT_TAIL(pLcdInit->ring_buf.prbWrite);
    }

    u32Cmd = u32BuffLen;
    if (LCDI_CMD != pLcdInit->opt) {
        u32Cmd |= BIT(30);
    }
    //Put cmd to queue.
    pLcdInit->ring_buf.prbCmd->pu32Buff[pLcdInit->ring_buf.prbCmd->u32Tail] = u32Cmd;
    pLcdInit->ring_buf.prbCmd->u32Tail = RNG_BUF_NEXT_TAIL(pLcdInit->ring_buf.prbCmd);

    //LCD->lcdi_ctrl |= CTRL_WR_IE;
    LCD_SetInt(LCDx, 0, 1);
    //LCD can't generate int at first time so we pend write interrupt manually.
    NVIC_SetPendingIRQ(LCD_IRQn);

    return u32BuffLen;
}

//Read with pendding.
int32_t LCDI_ReadBuff(LCD_TypeDef *LCDx, LCD_InitTypeDef *pLcdInit, uint8_t *pu8Buff, uint32_t u32BuffLen)
{
    uint32_t i, u32Cmd, u32Len = RNG_BUF_LEN(pLcdInit->ring_buf.prbRead);

    if (u32BuffLen > BIT(30) || RNG_BUF_IS_FULL(pLcdInit->ring_buf.prbCmd) || pLcdInit->ring_buf.prbRead->u32BuffSize < u32Len + u32BuffLen + 1) {
        return -1;
    }

    u32Cmd = u32BuffLen | BIT(31);
    if (LCDI_CMD != pLcdInit->opt) {
        u32Cmd |= BIT(30);
    }

    //Must wait for other operation complate or maybe lost interrupt.
    //while (0 != RNG_BUF_LEN(prbCmd));
    //Put cmd to queue.
    //posi = RNG_BUF_PERI_TAIL(prbRead);
    //i = posi < prbCmd->u32Head;
    pLcdInit->ring_buf.prbCmd->pu32Buff[pLcdInit->ring_buf.prbCmd->u32Tail] = u32Cmd;
    pLcdInit->ring_buf.prbCmd->u32Tail = RNG_BUF_NEXT_TAIL(pLcdInit->ring_buf.prbCmd);

    LCD_SetInt(LCDx, 1, 1);
    //LCD->lcdi_ctrl |= CTRL_RD_IE;
    //LCD can't generate int at first time so we pend write interrupt manually.
    NVIC_SetPendingIRQ(LCD_IRQn);

    for (i = 0; i < u32BuffLen; i++) {
        while (0 == RNG_BUF_LEN(pLcdInit->ring_buf.prbRead));
        pLcdInit->ring_buf.prbRead->u32Head = RNG_BUF_NEXT_HEAD(pLcdInit->ring_buf.prbRead);
        pu8Buff[i] = pLcdInit->ring_buf.prbRead->pu8Buff[pLcdInit->ring_buf.prbRead->u32Head];
    }

    return u32BuffLen;
}


void LCDI_Init(LCD_TypeDef *LCDx, LCD_InitTypeDef *pLcdInit)
{
    //Set mode and auto read
    if (LCDI_MODE_8080 == pLcdInit->LCD_BusMode) {
        LCDx->lcdi_ctrl = LCDI_CTRL_TYPE_8080 | LCDI_CTRL_AUTO;
    } else {
        //In 6800 mode we should set /WR signal manually.
        LCDx->lcdi_ctrl = LCDI_CTRL_EN_RD | LCDI_CTRL_AUTO;
    }

    LCDx->lcdi_cycle = pLcdInit->LCD_MaxQTR;
    if (pLcdInit->LCD_IntRead) {
        LCDx->lcdi_ctrl |= LCDI_CTRL_RD_IE;
    }
    if (pLcdInit->LCD_IntWrite) {
        LCDx->lcdi_ctrl |= LCDI_CTRL_WR_IE;
    }
}


void LCDI_Write(LCD_TypeDef *LCDx, uint8_t u8CD, uint8_t u8Value)
{
    while (LCDI_STATUS_READY != (LCDx->lcdi_status & LCDI_STATUS_READY));
    LCDI_BusWrite(LCDx, u8CD, u8Value);
}


void LCDI_Read(LCD_TypeDef *LCDx, uint8_t u8CD, uint8_t *dat)
{
    LCDI_BusRead(LCDx, u8CD);
    while (LCDI_STATUS_READY != (LCDx->lcdi_status & LCDI_STATUS_READY));

    *dat = LCDx->lcdi_data;
}


/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
