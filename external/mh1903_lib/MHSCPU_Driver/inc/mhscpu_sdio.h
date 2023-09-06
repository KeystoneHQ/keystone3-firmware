/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_sdio.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the SDIO firmware library
 *****************************************************************************/

#ifndef __MHSCPU_SDIO_H
#define __MHSCPU_SDIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhscpu.h"


typedef struct {
    uint32_t SDIO_BusWide;            /*!< Specifies the SDIO bus width */

    uint32_t SDIO_ClockLowPower;      /*!< Specifies whether stop SDIO clock or not when the bus is idle */

    uint8_t SDIO_ClockDiv;            /*!< Specifies the clock frequency of the SDIO controller,
                                           value of 0 means divide by 2*0 = 0 (no division, bypass) */
} SDIO_InitTypeDef;


typedef union {
    struct {
        uint32_t CmdIndex               : 6;    /*bit0-5*/
        uint32_t Response               : 2;    /*bit6-7*/
        uint32_t ResponseCRCCheck       : 1;    /*bit8*/
        uint32_t DataExpected           : 1;    /*bit9*/
        uint32_t TransferDir            : 1;    /*bit10*/
        uint32_t TransferMode           : 1;    /*bit11*/
        uint32_t SendAutoStop           : 1;    /*bit12*/
        uint32_t WaitPrvDataComplete    : 1;    /*bit13*/
        uint32_t StopAbordCmd           : 1;    /*bit14*/
        uint32_t SendInitSequence       : 1;    /*bit15*/
        uint32_t Rsvd0                  : 5;    /*bit16-20*/
        uint32_t OnlyUpdateClock        : 1;    /*bit21*/
        uint32_t ReadCEATADevice        : 1;    /*bit22*/
        uint32_t CCSExpected            : 1;    /*bit23*/
        uint32_t BootCmd                : 3;    /*bit24-26*/
        uint32_t BootMode               : 1;    /*bit27*/
        uint32_t VolSwitch              : 1;    /*bit28*/
        uint32_t UseHoldReg             : 1;    /*bit29*/
        uint32_t Rsvd1                  : 1;    /*bit30*/
        uint32_t StartCmd               : 1;    /*bit31*/
    } b;
    uint32_t d;
} SDIO_CmdTypeDef;


typedef struct {
    uint32_t SDIO_Argument;       /*!< Specifies the SDIO command argument which is sent
                                       to a card as part of a command message. If a command
                                       contains an argument, it must be loaded into this register
                                       before writing the command to the command register */
    SDIO_CmdTypeDef SDIO_Cmd;

} SDIO_CmdInitTypeDef;


typedef struct {
    uint32_t SDIO_DataTimeOut;

    uint32_t SDIO_DataLength;

    uint32_t SDIO_DataBlockSize;

} SDIO_DataInitTypeDef;


/**
  * @brief  SDIO DMA Init structure definition
  */
typedef struct {
    uint32_t SDIO_DMABurstSize;

    uint32_t SDIO_DMAReceiveLevel;

    uint32_t SDIO_DMATransmitLevel;

    FunctionalState SDIO_DMAEnCmd;
} SDIO_DMAInitTypeDef;
/**
  * @}
  */


/** @defgroup SDIO_Exported_Constants
  * @{
  */

/** @defgroup SDIO_Bus_Wide
  * @{
  */
#define SDIO_BusWide_1b                     ((uint32_t)0x00000000)
#define SDIO_BusWide_4b                     ((uint32_t)0x00000001)
#define SDIO_BusWide_8b                     ((uint32_t)0x00010000)
#define IS_SDIO_BUS_WIDE(WIDE)              (((WIDE) == SDIO_BusWide_1b) || ((WIDE) == SDIO_BusWide_4b) || \
                                            ((WIDE) == SDIO_BusWide_8b))
/**
  * @}
  */


/** @defgroup SDIO_Clk_Divider
  * @{
  */
#define SDIO_ClkDivider_0                   ((uint32_t)0x00)
#define SDIO_ClkDivider_255                 ((uint32_t)0xFF)
#define IS_SDIO_CLOCK_DIVIDER(DIVIDER)      ((((int32_t)(DIVIDER)) >= SDIO_ClkDivider_0) && (((int32_t)(DIVIDER)) <= SDIO_ClkDivider_255))
/**
  * @}
  */


/** @defgroup SDIO_Power_State
  * @{
  */

#define SDIO_PowerState_OFF                 ((uint32_t)0x00000000)
#define SDIO_PowerState_ON                  ((uint32_t)0x00000001)
#define IS_SDIO_POWER_STATE(STATE)          (((STATE) == SDIO_PowerState_OFF) || ((STATE) == SDIO_PowerState_ON))
/**
  * @}
  */


/** @defgroup SDIO_Interrupt_sources
  * @{
  */
#define SDIO_IT_CARD                        ((uint32_t)0x00010000)

#define SDIO_IT_CARDDET                     ((uint32_t)0x00000001)      /*!< Card detect */
#define SDIO_IT_RESPERR                     ((uint32_t)0x00000002)      /*!< Response error */
#define SDIO_IT_CMDDONE                     ((uint32_t)0x00000004)      /*!< Command done */
#define SDIO_IT_DATAEND                     ((uint32_t)0x00000008)      /*!< Data transfer over */
#define SDIO_IT_TXDREQ                      ((uint32_t)0x00000010)      /*!< Transmit FIFO data request */
#define SDIO_IT_RXDREQ                      ((uint32_t)0x00000020)      /*!< Receive FIFO data request */
#define SDIO_IT_RCRCFAIL                    ((uint32_t)0x00000040)      /*!< Response CRC error */
#define SDIO_IT_DCRCFAIL                    ((uint32_t)0x00000080)      /*!< Data CRC error */
#define SDIO_IT_RTIMEOUT                    ((uint32_t)0x00000100)      /*!< Response timeout */
#define SDIO_IT_DTIMEOUT                    ((uint32_t)0x00000200)      /*!< Data read timeout */
#define SDIO_IT_DHTIMEOUT_VOLSWITCH         ((uint32_t)0x00000400)      /*!< Data starvation-by-host timeout/Volt_switch_int */
#define SDIO_IT_FRUN                        ((uint32_t)0x00000800)      /*!< FIFO underrun/overrun error */
#define SDIO_IT_HLW_ERR                     ((uint32_t)0x00001000)      /*!< Hardware locked write error */
#define SDIO_IT_STBITERR                    ((uint32_t)0x00002000)      /*!< Start-bit error */
#define SDIO_IT_AUTO_CMDSENT                ((uint32_t)0x00004000)      /*!< Auto command done */
#define SDIO_IT_EDBITERR                    ((uint32_t)0x00008000)      /*!< End-bit error (read)/Write no CRC */
#define SDIO_IT_ALL                         ((uint32_t)0x0000FFFF)

#define IS_SDIO_IT(IT)                      ((((IT) & ~SDIO_IT_ALL) == 0x00) && ((IT) != 0x00))

#define IS_SDIO_GET_IT(IT)                  (((IT) == SDIO_IT_CARDDET)  || \
                                             ((IT) == SDIO_IT_RESPERR)  || \
                                             ((IT) == SDIO_IT_CMDDONE)  || \
                                             ((IT) == SDIO_IT_DATAEND)  || \
                                             ((IT) == SDIO_IT_TXDREQ)   || \
                                             ((IT) == SDIO_IT_RXDREQ)   || \
                                             ((IT) == SDIO_IT_RCRCFAIL) || \
                                             ((IT) == SDIO_IT_DCRCFAIL) || \
                                             ((IT) == SDIO_IT_RTIMEOUT) || \
                                             ((IT) == SDIO_IT_DTIMEOUT) || \
                                             ((IT) == SDIO_IT_DHTIMEOUT_VOLSWITCH) || \
                                             ((IT) == SDIO_IT_FRUN) || \
                                             ((IT) == SDIO_IT_HLW_ERR)  || \
                                             ((IT) == SDIO_IT_STBITERR) || \
                                             ((IT) == SDIO_IT_AUTO_CMDSENT)  || \
                                             ((IT) == SDIO_IT_EDBITERR) || \
                                             ((IT) == SDIO_IT_CARD))
/**
  * @}
  */


/** @defgroup SDIO_Status
  * @{
  */
#define SDIO_FLAG_RXFIFO_WATERMARK          ((uint32_t)0x00000001)      /*!< FIFO reached Receive watermark level; not qualified with data transfer */
#define SDIO_FLAG_TXFIFO_WATERMARK          ((uint32_t)0x00000002)      /*!< FIFO reached Transmit watermark level; not qualified with data transfer */
#define SDIO_FLAG_FIFO_EMPTY                ((uint32_t)0x00000004)      /*!< FIFO is empty status */
#define SDIO_FLAG_FIFO_FULL                 ((uint32_t)0x00000008)      /*!< FIFO is full status */
#define SDIO_FLAG_CARD_PRESENT              ((uint32_t)0x00000100)      /*!< Raw selected card_data[3]; checks whether card is present */
#define SDIO_FLAG_DATA_BUSY                 ((uint32_t)0x00000200)      /*!< Inverted version of raw selected card_data[0] */
#define SDIO_FLAG_ALL                       ((uint32_t)0x0000003f)

#define IS_SDIO_FLAG(FLAG)                  ((((FLAG) & ~SDIO_FLAG_ALL) == 0x00) && ((FLAG) != 0x00))

#define IS_SDIO_GET_FLAG(FLAG)              (((FLAG) == SDIO_FLAG_RXFIFO_WATERMARK)  || \
                                             ((FLAG) == SDIO_FLAG_TXFIFO_WATERMARK)  || \
                                             ((FLAG) == SDIO_FLAG_FIFO_EMPTY)  || \
                                             ((FLAG) == SDIO_FLAG_FIFO_FULL)  || \
                                             ((FLAG) == SDIO_FLAG_CARD_PRESENT)   || \
                                             ((FLAG) == SDIO_FLAG_DATA_BUSY))
/**
  * @}
  */


/** @defgroup SDIO_Command_Index
  * @{
  */

#define IS_SDIO_CMD_INDEX(INDEX)             ((INDEX) < 0x40)
/**
  * @}
  */


/** @defgroup SDIO_Response_Type
  * @{
  */

#define SDIO_Response_No                        ((uint32_t)0x00)
#define SDIO_Response_Short                     ((uint32_t)0x01)
#define SDIO_Response_Long                      ((uint32_t)0x03)
#define IS_SDIO_RESPONSE(RESPONSE)              (((RESPONSE) == SDIO_Response_No) || \
                                                 ((RESPONSE) == SDIO_Response_Short) || \
                                                 ((RESPONSE) == SDIO_Response_Long))
/**
  * @}
  */


/** @defgroup SDIO_Response_Registers
  * @{
  */

#define SDIO_RESP0                          ((uint32_t)0x00000000)
#define SDIO_RESP1                          ((uint32_t)0x00000004)
#define SDIO_RESP2                          ((uint32_t)0x00000008)
#define SDIO_RESP3                          ((uint32_t)0x0000000C)
#define IS_SDIO_RESP(RESP)                  (((RESP) == SDIO_RESP0) || ((RESP) == SDIO_RESP1) || \
                                             ((RESP) == SDIO_RESP2) || ((RESP) == SDIO_RESP3))
/**
  * @}
  */


/** @defgroup SDIO_Data_Config
  * @{
  */

#define IS_SDIO_DATA_LENGTH(LENGTH)         ((LENGTH) <= 0x01FFFFFF)
#define SDIO_BLOCK_SIZE_MASK                ((uint32_t)0xFFFF)
#define SDIO_DATA_TIMEOUT_MASK              ((uint32_t)0xFFFFFF00)
/**
  * @}
  */


/** @defgroup SDIO_CMD_Control_Bit
  * @{
  */
#define SDIO_CMD_TransferDir_ToCard                    (1)
#define SDIO_CMD_TransferDir_FromCard                  (0)

#define SDIO_CMD_TransferMode_Block                    (0)
#define SDIO_CMD_TransferMode_Stream                   (1)
/**
  * @}
  */


/** @defgroup SDIO_Clk_ENA
  * @{
  */
#define SDIO_ClockLowPower_Enable                      ((uint32_t)0x00010000)
#define SDIO_Clock_Enable                              ((uint32_t)0x01)
/**
  * @}
  */


/** @defgroup SDIO_Ctrl_Reg
  * @{
  */
#define SDIO_CTRL_DMA_ENABLE                           ((uint32_t)0x0020)
#define SDIO_CTRL_GLOBAL_IT_ENABLE                     ((uint32_t)0x0010)
/**
  * @}
  */


/** @defgroup SDIO_Status_Reg
  * @{
  */
#define SDIO_STATUS_FIFO_COUNT_POS                      (17)
#define SDIO_STATUS_FIFO_COUNT_MASK                     ((uint32_t)0x1FFF)
#define SDIO_STATUS_RESPONSE_INDEX_POS                  (11)
#define SDIO_STATUS_RESPONSE_INDEX_MASK                 ((uint32_t)0x3F)
/**
  * @}
  */


/** @defgroup SDIO_FIFOTH_Reg
  * @{
  */
#define SDIO_FIFOTH_DMA_BURST_SIZE_POS                  (28)
#define SDIO_FIFOTH_DMA_BURST_SIZE_MASK                 (0x70000000)
#define SDIO_FIFOTH_RX_WMARK_POS                        (16)
#define SDIO_FIFOTH_RX_WMARK_MASK                       (0x0FFF0000)
#define SDIO_FIFOTH_TX_WMARK_POS                        (0)
#define SDIO_FIFOTH_TX_WMARK_MASK                       (0x00000FFF)
/**
  * @}
  */


/** @defgroup DMA_burst_size
  * @{
  */
#define SDIO_DMA_BurstSize_1                            ((uint32_t)0x00)
#define SDIO_DMA_BurstSize_4                            ((uint32_t)0x01)
#define SDIO_DMA_BurstSize_8                            ((uint32_t)0x02)
#define IS_SDIO_DMA_BURST_SIZE(SIZE)                    ((((int32_t)(SIZE)) >= SDIO_DMA_BurstSize_1) && \
                                                         (((int32_t)(SIZE)) <= SDIO_DMA_BurstSize_8))
/**
  * @}
  */

/** @defgroup SDIO_TXFIFOWMARK
  * @{
  */
#define SDIO_TXFIFOWMARK_1                              ((uint32_t)0x0001)
#define SDIO_TXFIFOWMARK_2                              ((uint32_t)0x0002)
#define SDIO_TXFIFOWMARK_3                              ((uint32_t)0x0003)
#define SDIO_TXFIFOWMARK_4                              ((uint32_t)0x0004)
#define SDIO_TXFIFOWMARK_5                              ((uint32_t)0x0005)
#define SDIO_TXFIFOWMARK_6                              ((uint32_t)0x0006)
#define SDIO_TXFIFOWMARK_7                              ((uint32_t)0x0007)
#define SDIO_TXFIFOWMARK_8                              ((uint32_t)0x0008)
#define SDIO_TXFIFOWMARK_9                              ((uint32_t)0x0009)
#define SDIO_TXFIFOWMARK_10                             ((uint32_t)0x000A)
#define SDIO_TXFIFOWMARK_11                             ((uint32_t)0x000B)
#define SDIO_TXFIFOWMARK_12                             ((uint32_t)0x000C)
#define SDIO_TXFIFOWMARK_13                             ((uint32_t)0x000D)
#define SDIO_TXFIFOWMARK_14                             ((uint32_t)0x000E)
#define SDIO_TXFIFOWMARK_15                             ((uint32_t)0x000F)

#define IS_SDIO_TX_FIFO_WMARK(WMARK)                     ((((int32_t)(WMARK)) >= SDIO_TXFIFOWMARK_1) && \
                                                         (((int32_t)(WMARK)) <= SDIO_TXFIFOWMARK_15))
/**
  * @}
  */

/** @defgroup SDIO_RXFIFOWMARK
  * @{
  */
#define SDIO_RXFIFOWMARK_0                              ((uint32_t)0x0000)
#define SDIO_RXFIFOWMARK_1                              ((uint32_t)0x0001)
#define SDIO_RXFIFOWMARK_2                              ((uint32_t)0x0002)
#define SDIO_RXFIFOWMARK_3                              ((uint32_t)0x0003)
#define SDIO_RXFIFOWMARK_4                              ((uint32_t)0x0004)
#define SDIO_RXFIFOWMARK_5                              ((uint32_t)0x0005)
#define SDIO_RXFIFOWMARK_6                              ((uint32_t)0x0006)
#define SDIO_RXFIFOWMARK_7                              ((uint32_t)0x0007)
#define SDIO_RXFIFOWMARK_8                              ((uint32_t)0x0008)
#define SDIO_RXFIFOWMARK_9                              ((uint32_t)0x0009)
#define SDIO_RXFIFOWMARK_10                             ((uint32_t)0x000A)
#define SDIO_RXFIFOWMARK_11                             ((uint32_t)0x000B)
#define SDIO_RXFIFOWMARK_12                             ((uint32_t)0x000C)
#define SDIO_RXFIFOWMARK_13                             ((uint32_t)0x000D)
#define SDIO_RXFIFOWMARK_14                             ((uint32_t)0x000E)

#define IS_SDIO_RX_FIFO_WMARK(WMARK)                    ((((int32_t)(WMARK)) >= SDIO_RXFIFOWMARK_0) && \
                                                         (((int32_t)(WMARK)) <= SDIO_RXFIFOWMARK_14))


/** @defgroup SDIO_Exported_Functions
  * @{
  */
void SDIO_DeInit(void);
/****************** Initialization and Configuration functions ******************/
void SDIO_Init(SDIO_InitTypeDef *SDIO_InitStruct);
void SDIO_StructInit(SDIO_InitTypeDef *SDIO_InitStruct);

void SDIO_SetPowerState(uint32_t SDIO_PowerState);
uint32_t SDIO_GetPowerState(void);

void SDIO_ClockCmd(FunctionalState NewState);

/************************ Command management functions ************************/
void SDIO_SendCommand(SDIO_CmdInitTypeDef *SDIO_CmdInitStruct);
void SDIO_CmdStructInit(SDIO_CmdInitTypeDef *SDIO_CmdInitStruct);
uint8_t SDIO_GetCommandResponse(void);
uint32_t SDIO_GetResponse(uint32_t SDIO_RESP);

/************************* Data management functions *************************/
void SDIO_DataConfig(SDIO_DataInitTypeDef *SDIO_DataInitStruct);
void SDIO_DataStructInit(SDIO_DataInitTypeDef *SDIO_DataInitStruct);
uint32_t SDIO_ReadData(void);
void SDIO_WriteData(uint32_t Data);
uint32_t SDIO_GetFIFOCount(void);

/********************* DMA transfers management functions ********************/
void SDIO_DMAInit(SDIO_DMAInitTypeDef *SDIO_DMAInitStruct);
void SDIO_DMAStructInit(SDIO_DMAInitTypeDef *SDIO_DMAInitStruct);
void SDIO_DMACmd(FunctionalState NewState);

/****************** Interrupts and Flags management functions ******************/
void SDIO_GlobalITCmd(FunctionalState NewState);
void SDIO_ITConfig(uint32_t SDIO_IT, FunctionalState NewState);
ITStatus SDIO_GetRawITStatus(uint32_t SDIO_IT);
ITStatus SDIO_GetITStatus(uint32_t SDIO_IT);
void SDIO_ClearITPendingBit(uint32_t SDIO_IT);
FlagStatus SDIO_GetFlagStatus(uint32_t SDIO_FLAG);


#ifdef __cplusplus
}
#endif

#endif  /* __MHSCPU_SDIO_H */

/**
  * @}
  */


/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
