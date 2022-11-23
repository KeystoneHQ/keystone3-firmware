/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : drv_sdcard.h
 * Description: driver for sdcard
 * author     : stone wang
 * data       : 2022-12-16 11:05
 **********************************************************************/

#ifndef _DRV_SDCARD_H
#define _DRV_SDCARD_H

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "mhscpu.h"
#include "mhscpu_sdio.h"


#define DEBUG 1
#define DEBUG_SDIOcsrd printf

#define BLOCK_SIZE               512  /* Block Size in Bytes */
//#define NUMBER_OF_BLOCKS      10   /* For Multi Blocks operation (Read/Write)
//*/
#define NUMBER_OF_BLOCKS 1 /* For Multi Blocks operation (Read/Write) */
#define MULTI_BUFFER_SIZE (BLOCK_SIZE * NUMBER_OF_BLOCKS)
typedef enum { FAILED = 0, PASSED = !FAILED } TestStatus;

/**********************************************************/
#define SD_DMA_MODE ((uint32_t)0x00000000)
//#define SD_POLLING_MODE                            ((uint32_t)0x00000002)

#define SD_4BIT_TRANSFER 1
//#define SD_1BIT_TRANSFER                            1

#define SDIO_CLK_1M 1000
#define SDIO_CLK_300K 300
#define SDIO_CLK_100K 100

typedef enum {
    /**
     * @brief  SDIO specific error defines
     */
    SD_CMD_CRC_FAIL     = (1),  /*!< Command response received (but CRC check failed) */
    SD_DATA_CRC_FAIL    = (2),  /*!< Data bock sent/received (CRC check Failed) */
    SD_CMD_RSP_TIMEOUT  = (3),  /*!< Command response timeout */
    SD_DATA_TIMEOUT     = (4),  /*!< Data time out */
    SD_TX_UNDERRUN      = (5),  /*!< Transmit FIFO under-run */
    SD_RX_OVERRUN       = (6),  /*!< Receive FIFO over-run */
    SD_START_BIT_ERR    = (7),  /*!< Start bit not detected on all data signals in widE bus mode */
    SD_CMD_OUT_OF_RANGE = (8),  /*!< CMD's argument was out of range.*/
    SD_ADDR_MISALIGNED  = (9),  /*!< Misaligned address */
    SD_BLOCK_LEN_ERR    = (10), /*!< Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length */
    SD_ERASE_SEQ_ERR    = (11), /*!< An error in the sequence of erase command occurs.*/
    SD_BAD_ERASE_PARAM  = (12), /*!< An Invalid selection for erase groups */
    SD_WRITE_PROT_VIOLATION  = (13), /*!< Attempt to program a write protect block */
    SD_LOCK_UNLOCK_FAILED    = (14), /*!< Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card */
    SD_COM_CRC_FAILED        = (15), /*!< CRC check of the previous command failed */
    SD_ILLEGAL_CMD           = (16), /*!< Command is not legal for the card state */
    SD_CARD_ECC_FAILED       = (17), /*!< Card internal ECC was applied but failed to correct the data */
    SD_CC_ERROR              = (18), /*!< Internal card controller error */
    SD_GENERAL_UNKNOWN_ERROR = (19), /*!< General or Unknown error */
    SD_STREAM_READ_UNDERRUN  = (20), /*!< The card could not sustain data transfer in stream read operation. */
    SD_STREAM_WRITE_OVERRUN  = (21), /*!< The card could not sustain data programming in stream mode */
    SD_CID_CSD_OVERWRITE     = (22), /*!< CID/CSD overwrite error */
    SD_WP_ERASE_SKIP         = (23), /*!< only partial address space was erased */
    SD_CARD_ECC_DISABLED     = (24), /*!< Command has been executed without using internal ECC */
    SD_ERASE_RESET           = (25), /*!< Erase sequence was cleared before executing because an out of erase sequence command was received */
    SD_AKE_SEQ_ERROR         = (26), /*!< Error in sequence of authentication. */
    SD_INVALID_VOLTRANGE     = (27),
    SD_ADDR_OUT_OF_RANGE     = (28),
    SD_SWITCH_ERROR          = (29),
    SD_SDIO_DISABLED         = (30),
    SD_SDIO_FUNCTION_BUSY    = (31),
    SD_SDIO_FUNCTION_FAILED  = (32),
    SD_SDIO_UNKNOWN_FUNCTION = (33),

    /**
     * @brief  Standard error defines
     */
    SD_INTERNAL_ERROR,
    SD_NOT_CONFIGURED,
    SD_REQUEST_PENDING,
    SD_REQUEST_NOT_APPLICABLE,
    SD_INVALID_PARAMETER,
    SD_UNSUPPORTED_FEATURE,
    SD_UNSUPPORTED_HW,
    SD_ERROR,
    SD_OK = 0
} SD_Error;

typedef enum {
    SDCardCapacityStandard, // SDSC:     (0, 2GB]
    SDCardCapacityHigh,     // SDHC:  (2GB, 32GB]
    SDCardCapacityExtended, // SDXC:  (32GB, 2TB]
    SCCardCapacityUltra,    // SDUC: (2TB, 128TB]
} SDCardCapacityEnum;

typedef struct {
    SDCardCapacityEnum Capacity : 8;

    uint8_t  SpecVersion;
    uint8_t  Class;
    uint16_t BlockSize;

    uint32_t TransferRate;
    uint32_t DeviceSize;

    uint16_t CardStatus;
    uint16_t RCA;

    union {
        struct {
            __IO          uint8_t : 8;            /*!< Reserved always 1 */
            __IO uint16_t ManufacturingDate : 12; /*!< Manufacturing Date */

            __IO uint8_t : 4; /*!< Reserved */

            __IO uint32_t ProductSN : 32;      /*!< Product Serial Number */
            __IO uint8_t  ProductRevision : 8; /*!< Product Revision */
            __IO uint8_t  ProductName[5];      /*!< Product Name  */
            __IO uint16_t ApplicationID : 16;  /*!< OEM/Application ID */
            __IO uint8_t  ManufacturerID : 8;  /*!< ManufacturerID */
        } __PACKED;
        uint32_t CID[4];
    };

    union {
        struct {
            uint8_t : 8;                                  // [7 : 0]
            uint8_t : 1;                                  // [8]
            bool    IsWriteProtectionUntilPowerCycle : 1; // [9]
            uint8_t FileFormat : 2;                       // [11 : 10]
            uint8_t TemporaryWriteProtection : 1;         // [12]
            uint8_t PermanentWriteProtection : 1;         // [13]
            uint8_t Copy : 1;                             // [14]
            uint8_t FileFormatGroup : 1;                  // [15]

            uint8_t : 5;                         // [20 : 16]
            bool     IsPartWriteAllowed : 1;     // [21]
            uint16_t MaxWriteLength : 4;         // [25 : 22]
            uint8_t  WriteSpeedFactor : 3;       // [28 : 26]
            uint8_t : 2;                         // [30 : 29]
            bool IsWriteProtectGroupEnabled : 1; // [31]

            union {
                struct {
                    uint16_t WriteProtectGroupSize : 7;     // [38 : 32]
                    uint16_t EraseSectorSize : 7;           // [45 : 39]
                    bool     IsSingleBlockEraseEnabled : 1; // [46]
                    uint16_t DeviceSizeMultiplier : 3;      // [49 : 47] Device size multiplier */
                    uint16_t MaxWriteCurrentVDDMax : 3;     // [52 : 50] Max. write current @ VDD max */
                    uint16_t MaxWriteCurrentVDDMin : 3;     // [55 : 53] Max. write current @ VDD min */
                    uint16_t MaxReadCurrentVDDMax : 3;      // [58 : 56] Max. read current @ VDD max */
                    uint16_t MaxReadCurrentVDDMin : 3;      // [59 : 61] Max. read current @ VDD min */

                    uint32_t DeviceSize : 12;          // [73 : 60]
                    uint16_t : 2;                      // [75 : 74]
                    bool IsDSRImplemented : 1;         // [76]
                    bool IsMisalignedReadAllowed : 1;  // [77]
                    bool IsMisalignedWriteAllowed : 1; // [78]
                    bool IsPartReadAllowed : 1;        // [79] Partial blocks for read allowed
                } __PACKED V1;

                struct {
                    uint16_t WriteProtectGroupSize : 7;     // [38 : 32]
                    uint16_t EraseSectorSize : 7;           // [45 : 39]
                    bool     IsSingleBlockEraseEnabled : 1; // [46]
                    uint16_t : 1;                           // [47]
                    uint32_t DeviceSize : 22;               // [69 : 48]
                    uint16_t : 6;                           // [75 : 70]
                    bool IsDSRImplemented : 1;              // [76]
                    bool IsMisalignedReadAllowed : 1;       // [77]
                    bool IsMisalignedWriteAllowed : 1;      // [78]
                    bool IsPartReadAllowed : 1;             // [79] Partial blocks for read allowed
                } __PACKED V2;
            } __PACKED;

            uint8_t  MaxReadLength : 4;       // [83 : 80] Max. read data block length
            uint16_t CardCommandClasses : 12; // [95 : 94] Card command classes

            uint8_t MaxDataRate : 8; // [103 :  96] Max. bus clock frequency
            uint8_t NSAC : 8;        // [111 : 104] Data read access-time 2 in CLK cycles
            uint8_t TAAC : 8;        // [119 : 112] Data read access-time 1
            uint8_t : 6;             // [125 : 120] Reserved
            uint8_t CSDStruct : 2;   // [127 : 126] CSD structure
        } __PACKED;
        uint32_t CSD[4];
    };
} SDCardInfoStruct;

//=================================================
/**
 * @brief  SDIO Transfer state
 */
typedef enum {
    SD_TRANSFER_OK   = 0,
    SD_TRANSFER_BUSY = 1,
    SD_TRANSFER_ERROR
} SDTransferState;

/**
 * @brief  SD Card States
 */
typedef enum {
    SD_CARD_READY          = ((uint32_t)0x00000001),
    SD_CARD_IDENTIFICATION = ((uint32_t)0x00000002),
    SD_CARD_STANDBY        = ((uint32_t)0x00000003),
    SD_CARD_TRANSFER       = ((uint32_t)0x00000004),
    SD_CARD_SENDING        = ((uint32_t)0x00000005),
    SD_CARD_RECEIVING      = ((uint32_t)0x00000006),
    SD_CARD_PROGRAMMING    = ((uint32_t)0x00000007),
    SD_CARD_DISCONNECTED   = ((uint32_t)0x00000008),
    SD_CARD_ERROR          = ((uint32_t)0x000000FF)
} SDCardState;

/**
 * @brief Supported SD Memory Cards
 */
#define SDIO_STD_CAPACITY_SD_CARD_V1_1    ((uint32_t)0x00000000)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0    ((uint32_t)0x00000001)
#define SDIO_HIGH_CAPACITY_SD_CARD        ((uint32_t)0x00000002)
#define SDIO_MULTIMEDIA_CARD              ((uint32_t)0x00000003)
#define SDIO_SECURE_DIGITAL_IO_CARD       ((uint32_t)0x00000004)
#define SDIO_HIGH_SPEED_MULTIMEDIA_CARD   ((uint32_t)0x00000005)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD ((uint32_t)0x00000006)
#define SDIO_HIGH_CAPACITY_MMC_CARD       ((uint32_t)0x00000007)

/**
 * @brief SDIO Commands  Index
 */
#define SD_CMD_GO_IDLE_STATE        ((uint8_t)0)
#define SD_CMD_SEND_OP_COND         ((uint8_t)1)
#define SD_CMD_ALL_SEND_CID         ((uint8_t)2)
#define SD_CMD_SET_REL_ADDR         ((uint8_t)3) /*!< SDIO_SEND_REL_ADDR for SD Card */
#define SD_CMD_SET_DSR              ((uint8_t)4)
#define SD_CMD_SDIO_SEN_OP_COND     ((uint8_t)5)
#define SD_CMD_SWITCH_FUNCTION      ((uint8_t)6)
#define SD_CMD_SEL_DESEL_CARD       ((uint8_t)7)
#define SD_CMD_HS_SEND_EXT_CSD      ((uint8_t)8)
#define SD_CMD_SEND_CSD             ((uint8_t)9)
#define SD_CMD_SEND_CID             ((uint8_t)10)
#define SD_CMD_READ_DAT_UNTIL_STOP  ((uint8_t)11) /*!< SD Card doesn't support it */
#define SD_CMD_STOP_TRANSMISSION    ((uint8_t)12)
#define SD_CMD_SEND_STATUS          ((uint8_t)13)
#define SD_CMD_HS_BUSTEST_READ      ((uint8_t)14)
#define SD_CMD_GO_INACTIVE_STATE    ((uint8_t)15)
#define SD_CMD_SET_BLOCKLEN         ((uint8_t)16)
#define SD_CMD_READ_SINGLE_BLOCK    ((uint8_t)17)
#define SD_CMD_READ_MULT_BLOCK      ((uint8_t)18)
#define SD_CMD_HS_BUSTEST_WRITE     ((uint8_t)19)
#define SD_CMD_WRITE_DAT_UNTIL_STOP ((uint8_t)20) /*!< SD Card doesn't support it */
#define SD_CMD_SET_BLOCK_COUNT      ((uint8_t)23) /*!< SD Card doesn't support it */
#define SD_CMD_WRITE_SINGLE_BLOCK   ((uint8_t)24)
#define SD_CMD_WRITE_MULT_BLOCK     ((uint8_t)25)
#define SD_CMD_PROG_CID             ((uint8_t)26) /*!< reserved for manufacturers */
#define SD_CMD_PROG_CSD             ((uint8_t)27)
#define SD_CMD_SET_WRITE_PROT       ((uint8_t)28)
#define SD_CMD_CLR_WRITE_PROT       ((uint8_t)29)
#define SD_CMD_SEND_WRITE_PROT      ((uint8_t)30)
#define SD_CMD_SD_ERASE_GRP_START   ((uint8_t)32) /*!< To set the address of the first write block to be erased. (For SD card only) */
#define SD_CMD_SD_ERASE_GRP_END     ((uint8_t)33) /*!< To set the address of the last write block of the continuous range to be erased. (For SD card only) */
#define SD_CMD_ERASE_GRP_START      ((uint8_t)35) /*!< To set the address of the first write block to be erased. (For MMC card only spec 3.31) */
#define SD_CMD_ERASE_GRP_END \
    ((uint8_t)36) /*!< To set the address of the last write block of the continuous range to be erased. (For MMC card only spec 3.31) */
#define SD_CMD_ERASE        ((uint8_t)38)
#define SD_CMD_FAST_IO      ((uint8_t)39) /*!< SD Card doesn't support it */
#define SD_CMD_GO_IRQ_STATE ((uint8_t)40) /*!< SD Card doesn't support it */
#define SD_CMD_LOCK_UNLOCK  ((uint8_t)42)
#define SD_CMD_APP_CMD      ((uint8_t)55)
#define SD_CMD_GEN_CMD      ((uint8_t)56)
#define SD_CMD_NO_CMD       ((uint8_t)64)

/**
 * @brief Following commands are SD Card Specific commands.
 *        SDIO_APP_CMD should be sent before sending these commands.
 */
#define SD_CMD_APP_SD_SET_BUSWIDTH          ((uint8_t)6)  /*!< For SD Card only */
#define SD_CMD_SD_APP_STAUS                 ((uint8_t)13) /*!< For SD Card only */
#define SD_CMD_SD_APP_SEND_NUM_WRITE_BLOCKS ((uint8_t)22) /*!< For SD Card only */
#define SD_CMD_SD_APP_OP_COND               ((uint8_t)41) /*!< For SD Card only */
#define SD_CMD_SD_APP_SET_CLR_CARD_DETECT   ((uint8_t)42) /*!< For SD Card only */
#define SD_CMD_SD_APP_SEND_SCR              ((uint8_t)51) /*!< For SD Card only */
#define SD_CMD_SDIO_RW_DIRECT               ((uint8_t)52) /*!< For SD I/O Card only */
#define SD_CMD_SDIO_RW_EXTENDED             ((uint8_t)53) /*!< For SD I/O Card only */

/**
 * @brief Following commands are SD Card Specific security commands.
 *        SDIO_APP_CMD should be sent before sending these commands.
 */
#define SD_CMD_SD_APP_GET_MKB                     ((uint8_t)43) /*!< For SD Card only */
#define SD_CMD_SD_APP_GET_MID                     ((uint8_t)44) /*!< For SD Card only */
#define SD_CMD_SD_APP_SET_CER_RN1                 ((uint8_t)45) /*!< For SD Card only */
#define SD_CMD_SD_APP_GET_CER_RN2                 ((uint8_t)46) /*!< For SD Card only */
#define SD_CMD_SD_APP_SET_CER_RES2                ((uint8_t)47) /*!< For SD Card only */
#define SD_CMD_SD_APP_GET_CER_RES1                ((uint8_t)48) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_READ_MULTIPLE_BLOCK  ((uint8_t)18) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK ((uint8_t)25) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_ERASE                ((uint8_t)38) /*!< For SD Card only */
#define SD_CMD_SD_APP_CHANGE_SECURE_AREA          ((uint8_t)49) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_WRITE_MKB            ((uint8_t)48) /*!< For SD Card only */

/** @defgroup SDIO_Flags
 * @{
 */
#define SDIO_FLAG_RXFIFOHE         ((uint32_t)0x00000001) //fifo_rx_watermark
#define SDIO_FLAG_TXFIFOHE         ((uint32_t)0x00000002) //fifo_tx_watermark
#define SDIO_FLAG_FIFOEMPTY        ((uint32_t)0x00000004) //fifo_empty
#define SDIO_FLAG_FIFOFULL         ((uint32_t)0x00000008) //fifo_full
#define SDIO_FLAG_IDLE             ((uint32_t)0x00000000) //command fsm states. Idle
#define SDIO_FLAG_SENDINITSEQUENCE ((uint32_t)0x00000010) //Send init sequence
#define SDIO_FLAG_TXSTART          ((uint32_t)0x00000020) //Tx cmd start bit
#define SDIO_FLAG_TXTX             ((uint32_t)0x00000030) //Tx cmd tx bit
#define SDIO_FLAG_TXINDEXARG       ((uint32_t)0x00000040) //Tx cmd index + arg
#define SDIO_FLAG_TXCRC            ((uint32_t)0x00000050) //Tx cmd crc7
#define SDIO_FLAG_TXEND            ((uint32_t)0x00000060) //Tx cmd end bit
#define SDIO_FLAG_RXSTART          ((uint32_t)0x00000070) //Rx resp start bit
#define SDIO_FLAG_RXIRQRESPONSE    ((uint32_t)0x00000080) //Rx resp IRQ response
#define SDIO_FLAG_RXTX             ((uint32_t)0x00000090) //Rx resp tx bit
#define SDIO_FLAG_RXCMDIDX         ((uint32_t)0x000000A0) //Rx resp cmd idx
#define SDIO_FLAG_RXDATA           ((uint32_t)0x000000B0) //Rx resp data
#define SDIO_FLAG_RXCRC            ((uint32_t)0x000000C0) //Rx resp crc7
#define SDIO_FLAG_RXEND            ((uint32_t)0x000000D0) //Rx resp end bit
#define SDIO_FLAG_CMDPATHWITNCC    ((uint32_t)0x000000E0) //Cmd path wait NCC
#define SDIO_FLAG_WAITTURNAROUND   ((uint32_t)0x000000F0) //Wait; CMD-to-response turnaround


#if 1
/** @defgroup SDIO_Data_Block_Size
 * @{
 */

#define SDIO_DataBlockSize_1b     ((uint32_t)0x00000001)
#define SDIO_DataBlockSize_2b     ((uint32_t)0x00000002)
#define SDIO_DataBlockSize_4b     ((uint32_t)0x00000004)
#define SDIO_DataBlockSize_8b     ((uint32_t)0x00000008)
#define SDIO_DataBlockSize_16b    ((uint32_t)0x00000010)
#define SDIO_DataBlockSize_32b    ((uint32_t)0x00000020)
#define SDIO_DataBlockSize_64b    ((uint32_t)0x00000040)
#define SDIO_DataBlockSize_128b   ((uint32_t)0x00000080)
#define SDIO_DataBlockSize_256b   ((uint32_t)0x00000100)
#define SDIO_DataBlockSize_512b   ((uint32_t)0x00000200)
#define SDIO_DataBlockSize_1024b  ((uint32_t)0x00000400)
#define SDIO_DataBlockSize_2048b  ((uint32_t)0x00000800)
#define SDIO_DataBlockSize_4096b  ((uint32_t)0x00001000)
#define SDIO_DataBlockSize_8192b  ((uint32_t)0x00002000)
#define SDIO_DataBlockSize_16384b ((uint32_t)0x00004000)
#define IS_SDIO_BLOCK_SIZE(SIZE)                                                                                                                              \
    (((SIZE) == SDIO_DataBlockSize_1b) || ((SIZE) == SDIO_DataBlockSize_2b) || ((SIZE) == SDIO_DataBlockSize_4b) || ((SIZE) == SDIO_DataBlockSize_8b) ||      \
     ((SIZE) == SDIO_DataBlockSize_16b) || ((SIZE) == SDIO_DataBlockSize_32b) || ((SIZE) == SDIO_DataBlockSize_64b) || ((SIZE) == SDIO_DataBlockSize_128b) || \
     ((SIZE) == SDIO_DataBlockSize_256b) || ((SIZE) == SDIO_DataBlockSize_512b) || ((SIZE) == SDIO_DataBlockSize_1024b) ||                                    \
     ((SIZE) == SDIO_DataBlockSize_2048b) || ((SIZE) == SDIO_DataBlockSize_4096b) || ((SIZE) == SDIO_DataBlockSize_8192b) ||                                  \
     ((SIZE) == SDIO_DataBlockSize_16384b))

#endif
/**
 * @brief  Mask for errors Card Status R1 (OCR Register)
 */
#define SD_OCR_ADDR_OUT_OF_RANGE     ((uint32_t)0x80000000)
#define SD_OCR_ADDR_MISALIGNED       ((uint32_t)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR         ((uint32_t)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR         ((uint32_t)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM       ((uint32_t)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION  ((uint32_t)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED    ((uint32_t)0x01000000)
#define SD_OCR_COM_CRC_FAILED        ((uint32_t)0x00800000)
#define SD_OCR_ILLEGAL_CMD           ((uint32_t)0x00400000)
#define SD_OCR_CARD_ECC_FAILED       ((uint32_t)0x00200000)
#define SD_OCR_CC_ERROR              ((uint32_t)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR ((uint32_t)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN  ((uint32_t)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN  ((uint32_t)0x00020000)
#define SD_OCR_CID_CSD_OVERWRIETE    ((uint32_t)0x00010000)
#define SD_OCR_WP_ERASE_SKIP         ((uint32_t)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED     ((uint32_t)0x00004000)
#define SD_OCR_ERASE_RESET           ((uint32_t)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR         ((uint32_t)0x00000008)
#define SD_OCR_ERRORBITS             ((uint32_t)0xFDFFE008)

/**
 * @brief  Command Class Supported
 */
#define SD_CCCC_LOCK_UNLOCK ((uint32_t)0x00000080)
#define SD_CCCC_WRITE_PROT  ((uint32_t)0x00000040)
#define SD_CCCC_ERASE       ((uint32_t)0x00000020)

/**
 * @brief  Masks for R6 Response
 */
#define SD_R6_GENERAL_UNKNOWN_ERROR ((uint32_t)0x00002000)
#define SD_R6_ILLEGAL_CMD           ((uint32_t)0x00004000)
#define SD_R6_COM_CRC_FAILED        ((uint32_t)0x00008000)

#define SD_VOLTAGE_WINDOW_SD ((uint32_t)0x80100000)
#define SD_HIGH_CAPACITY     ((uint32_t)0x40000000)
#define SD_STD_CAPACITY      ((uint32_t)0x00000000)
#define SD_CHECK_PATTERN     ((uint32_t)0x000001AA)

#define SD_MAX_VOLT_TRIAL ((uint32_t)0x0000FFFF)
#define SD_ALLZERO        ((uint32_t)0x00000000)

#define SD_WIDE_BUS_SUPPORT   ((uint32_t)0x00040000)
#define SD_SINGLE_BUS_SUPPORT ((uint32_t)0x00010000)
#define SD_CARD_LOCKED        ((uint32_t)0x02000000)

#define SD_DATATIMEOUT     ((uint32_t)0xFFFFFFFF)
#define SD_0TO7BITS        ((uint32_t)0x000000FF)
#define SD_8TO15BITS       ((uint32_t)0x0000FF00)
#define SD_16TO23BITS      ((uint32_t)0x00FF0000)
#define SD_24TO31BITS      ((uint32_t)0xFF000000)
#define SD_MAX_DATA_LENGTH ((uint32_t)0x01FFFFFF)

#define SD_HALFFIFO      ((uint32_t)0x00000008)
#define SD_HALFFIFOBYTES ((uint32_t)0x00000020)

/**
 * @brief  SDIO Static flags
 */
#define SDIO_STATIC_FLAGS ((uint32_t)0x0000FFFF) //16ä¸ªé™æ€æ ‡å¿?
#define SDIO_CMD0TIMEOUT  ((uint32_t)0x000FFFFF)

SD_Error SD_Init(void);                                                             // 初始化SD卡，使卡处于就绪状�?准备传输数据)
SD_Error SD_PowerON(void);                                                          // 确保SD卡的工作电压和配置控制时�?
SD_Error SD_PowerOFF(void);                                                         // 关掉SDIO的输出信�?
SD_Error SD_InitializeCards(void);                                                  // 初始化所有的卡或者单个卡进入就绪状�?
SD_Error SD_StopTransfer(void);                                                     // 停止数据传输
SD_Error SD_GetCardInfo(SDCardInfoStruct* cardinfo);                                     // 获取SD卡的具体信息
SD_Error SD_SelectDeselect(uint32_t addr);                                          // 利用cmd7，选择卡相对地址为addr的卡，取消选择其它�?
SD_Error SD_EnableWideBusOperation(uint32_t WideMode);                              // 配置卡的数据宽度(但得看卡是否支持)
SD_Error SD_ReadBlock(uint8_t* readbuff, uint32_t ReadAddr, uint16_t BlockSize);    // 单块�?
SD_Error SD_WaitReadOperation(void);
SD_Error SD_WaitWriteOperation(void);
SD_Error SD_WriteBlock(uint8_t* writebuff, uint32_t WriteAddr, uint16_t BlockSize); // 单块�?

SDTransferState SD_GetStatus(void);

SD_Error SD_WriteMultiBlocks(uint8_t* writebuff, uint32_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks);
SD_Error SD_ReadMultiBlocks(uint8_t* readbuff, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks); //多块�?DMA操作
SD_Error SD_ProcessIRQSrc(void);

extern bool SDCardSetBusWidth(uint8_t busWidth);

extern bool SDCardSetup(void);

extern bool SDCardErase(uint32_t startAddress, uint32_t endAddress);

extern bool SDCardTransferBlock(bool isWrite, uint32_t address, uint8_t* buffer, uint32_t blockCount);

bool SdCardInsert(void);
void SdCardIntHandler(void);

#endif /* _DRV_SDCARD_H */
