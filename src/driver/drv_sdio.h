#ifndef __PERIPH_SDIO_H__
#define __PERIPH_SDIO_H__

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "mhscpu.h"

#if defined(__CC_ARM)
#pragma anon_unions
#pragma diag_suppress 94
#endif

#define SDIO_IT_CDT  BIT0         // [0]    0001 : Card detect (CDT)
#define SDIO_IT_RE   BIT1         // [1]    0002 : Response error (RE)
#define SDIO_IT_CD   BIT2         // [2]    0004 : Command done (CD)
#define SDIO_IT_DTO  BIT3         // [3]    0008 : Data transfer over (DTO)
#define SDIO_IT_TXDR BIT4         // [4]    0010 : Transmit FIFO data request (TXDR)
#define SDIO_IT_RXDR BIT5         // [5]    0020 : Receive FIFO data request (RXDR)
#define SDIO_IT_RCRC BIT6         // [6]    0040 : Response CRC error (RCRC)
#define SDIO_IT_DCRC BIT7         // [7]    0080 : Data CRC error (DCRC)
#define SDIO_IT_RTO  BIT8         // [8]    0100 : Response timeout (RTO)
#define SDIO_IT_BAR  SDIO_IT_RTO  // [8]    0100 : Boot Ack Received (BAR)
#define SDIO_IT_DRTO BIT9         // [9]    0200 : Data read timeout (DRTO)
#define SDIO_IT_BDS  SDIO_IT_DRTO // [9]    0200 : Boot Data Start (BDS)
#define SDIO_IT_HTO  BIT10        // [10]   0400 : Data starvation-by-host timeout (HTO) /Volt_switch_int
//#ifndef SDIO_IT_FRUN
//#define SDIO_IT_FRUN BIT11 // [11]   0800 : FIFO underrun/overrun error (FRUN)
//#endif
#define SDIO_IT_HLE BIT12 // [12]   1000 : Hardware locked write error (HLE)
#define SDIO_IT_SBE BIT13 // [13]   2000 : Start-bit error (SBE)
#define SDIO_IT_ACD BIT14 // [14]   4000 : Auto command done (ACD)
#define SDIO_IT_EBE BIT15 // [15]   8000 : End-bit error (read)/write no CRC (EBE)

#define SDIO_IT_CRE (SDIO_IT_RTO | SDIO_IT_RCRC | SDIO_IT_RE)   // Command Response Error (CRE)
#define SDIO_IT_DTE (SDIO_IT_DRTO | SDIO_IT_DCRC | SDIO_IT_SBE) // Data Transfer Error (DTE) // SDIO_IT_EBE
//#ifndef SDIO_IT_ALL
//#define SDIO_IT_ALL (0xFFFFU) // All interrupts (ALL)
//#endif

#ifndef CONFIG_SDIO_DMA_CHANNEL
#define CONFIG_SDIO_DMA_CHANNEL DMA_Channel_0
#endif

typedef union {
    uint32_t Parameter;
    struct {
        uint32_t CmdIndex : 6;            // [0:5]
        uint32_t Response : 2;            // [6:7]
        uint32_t ResponseCRCCheck : 1;    // [8]
        uint32_t DataExpected : 1;        // [9]
        uint32_t TransferDir : 1;         // [10]
        uint32_t TransferMode : 1;        // [11]
        uint32_t SendAutoStop : 1;        // [12]
        uint32_t WaitPrvDataComplete : 1; // [13]
        uint32_t StopAbordCmd : 1;        // [14]
        uint32_t SendInitSequence : 1;    // [15]
        uint32_t Rsvd0 : 5;               // [16:20]
        uint32_t OnlyUpdateClock : 1;     // [21]
        uint32_t ReadCEATADevice : 1;     // [22]
        uint32_t CCSExpected : 1;         // [23]
        uint32_t BootCmd : 3;             // [24:26]
        uint32_t BootMode : 1;            // [27]
        uint32_t VolSwitch : 1;           // [28]
        uint32_t UseHoldReg : 1;          // [29]
        uint32_t Rsvd1 : 1;               // [30]
        uint32_t StartCmd : 1;            // [31]
    };
} SDIOCommandStruct;

typedef enum {
    SDIOResponseNone,
    SDIOResponseR1,
    SDIOResponseR1b,
    SDIOResponseR2,
    SDIOResponseR3,
    SDIOResponseR6,
    SDIOResponseR7,
} SDIOResponseTypeEnum;

extern void SDIOClockConfig(uint32_t clockInKHZ, bool isQuadBus, bool isClockLowPower);

extern bool SDIOExecuteCommand(uint8_t cmdIndex, uint32_t argument, SDIOResponseTypeEnum responseType, uint32_t* response);

extern bool SDIOTransferBlock(uint8_t cmdIndex, uint32_t argument, bool isWrite, uint8_t* buffer, uint32_t blockLength, uint32_t blockCount);

#endif // __PERIPH_SDIO_H__
