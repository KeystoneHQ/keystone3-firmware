/**************************************************************************//**
 * @file     <Device>.h
 * @brief    CMSIS Cortex-M# Core Peripheral Access Layer Header File for
 *           Device <Device>
 * @version  V3.10
 * @date     23. November 2012
 *
 * @note
 *
 ******************************************************************************/
/* Copyright (c) 2012 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list ofC conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/


#ifndef MHSCPU_H
#define MHSCPU_H

#ifdef __cplusplus
extern "C" {
#endif

/* ToDo: replace '<Device>' with your device name; add your doxyGen comment   */
/** @addtogroup <Device>_Definitions <Device> Definitions
  This file defines all structures and symbols for <Device>:
    - registers and bitfields
    - peripheral base address
    - peripheral ID
    - Peripheral definitions
  @{
*/


/******************************************************************************/
/*                Processor and Core Peripherals                              */
/******************************************************************************/
/** @addtogroup <Device>_CMSIS Device CMSIS Definitions
  Configuration of the Cortex-M# Processor and Core Peripherals
  @{
*/
/*
 * ==========================================================================
 * ---------- Interrupt Number Definition -----------------------------------
 * ==========================================================================
 */
typedef enum IRQn {
    /******  Cortex-M# Processor Exceptions Numbers ***************************************************/



    /* ToDo: use this Cortex interrupt numbers if your device is a CORTEX-M3 / Cortex-M4 device       */
    NonMaskableInt_IRQn           = -14,      /*!<  2 Non Maskable Interrupt                        */
    MemoryManagement_IRQn         = -12,      /*!<  4 Memory Management Interrupt                   */
    BusFault_IRQn                 = -11,      /*!<  5 Bus Fault Interrupt                           */
    UsageFault_IRQn               = -10,      /*!<  6 Usage Fault Interrupt                         */
    SVCall_IRQn                   = -5,       /*!< 11 SV Call Interrupt                             */
    DebugMonitor_IRQn             = -4,       /*!< 12 Debug Monitor Interrupt                       */
    PendSV_IRQn                   = -2,       /*!< 14 Pend SV Interrupt                             */
    SysTick_IRQn                  = -1,       /*!< 15 System Tick Interrupt                         */

    /******  Device Specific Interrupt Numbers ********************************************************/
    /* ToDo: add here your device specific external interrupt numbers
             according the interrupt handlers defined in startup_Device.s
             eg.: Interrupt for Timer#1       TIM1_IRQHandler   ->   TIM1_IRQn                        */
    DMA_IRQn                                        = 0,
    USB_IRQn                                        = 1,
    USBDMA_IRQn                                     = 2,
    LCD_IRQn                                        = 3,
    SCI0_IRQn                                       = 4,
    UART0_IRQn                                      = 5,
    UART1_IRQn                                      = 6,
    SPI0_IRQn                                       = 7,
    CRYPT0_IRQn                                     = 8,
    TIM0_0_IRQn                                     = 9,
    TIM0_1_IRQn                                     = 10,
    TIM0_2_IRQn                                     = 11,
    TIM0_3_IRQn                                     = 12,
    EXTI0_IRQn                                      = 13,
    EXTI1_IRQn                                      = 14,
    EXTI2_IRQn                                      = 15,
    RTC_IRQn                                        = 16,
    SENSOR_IRQn                                     = 17,
    TRNG_IRQn                                       = 18,
    ADC0_IRQn                                       = 19,
    SSC_IRQn                                        = 20,
    TIM0_4_IRQn                                     = 21,
    TIM0_5_IRQn                                     = 22,
    KBD_IRQn                                        = 23,
    MSR_IRQn                                        = 24,
    EXTI3_IRQn                                      = 25,
    SPI1_IRQn                                       = 26,
    SPI2_IRQn                                       = 27,
    SCI1_IRQn                                       = 28,
    SCI2_IRQn                                       = 29,
    SPI3_IRQn                                       = 30,
    SPI4_IRQn                                       = 31,
    UART2_IRQn                                      = 32,
    UART3_IRQn                                      = 33,
    QSPI_IRQn                                       = 35,
    I2C0_IRQn                                       = 36,
    EXTI4_IRQn                                      = 37,
    EXTI5_IRQn                                      = 38,
    TIM0_6_IRQn                                     = 39,
    TIM0_7_IRQn                                     = 40,
    CSI2_IRQn                                       = 41,
    DCMI_IRQn                                       = 42,
    EXTI6_IRQn                                      = 43,
    EXTI7_IRQn                                      = 44,
    SDIOM_IRQn                                      = 45,
    QR_IRQn                                         = 46,
    GPU_IRQn                                        = 47,
    AWD_IRQn                                        = 49,
    DAC_IRQn                                        = 50
} IRQn_Type;


/*
 * ==========================================================================
 * ----------- Processor and Core Peripheral Section ------------------------
 * ==========================================================================
 */

/* Configuration of the Cortex-M# Processor and Core Peripherals */
/* ToDo: set the defines according your Device                                                    */
/* ToDo: define the correct core revision
         __CM0_REV if your device is a CORTEX-M0 device
         __CM3_REV if your device is a CORTEX-M3 device
         __CM4_REV if your device is a CORTEX-M4 device                                           */
//#define __CM3_REV                 0x0201    /*!< Core Revision r2p1                               */
//#define __CM3_REV                 0x0200    /*!< Core Revision r2p0                               */
#define __CM4_REV                   0x0001    /*!< Core Revision r2p0                               */
#define __NVIC_PRIO_BITS          3         /*!< Number of Bits used for Priority Levels          */
#define __Vendor_SysTickConfig    0         /*!< Set to 1 if different SysTick Config is used     */
#define __MPU_PRESENT             1         /*!< MPU present or not                               */
/* ToDo: define __FPU_PRESENT if your devise is a CORTEX-M4                                       */
#define __FPU_PRESENT             1        /*!< FPU present or not                                */

/*@}*/ /* end of group <Device>_CMSIS */


/* ToDo: include the correct core_cm#.h file
         core_cm0.h if your device is a CORTEX-M0 device
         core_cm3.h if your device is a CORTEX-M3 device
         core_cm4.h if your device is a CORTEX-M4 device                                          */
#include "core_cm4.h"                       /* Cortex-M# processor and core peripherals           */
/* ToDo: include your system_<Device>.h file
         replace '<Device>' with your device name                                                 */
#include "system_mhscpu.h"                /* <Device> System  include file                      */


/******************************************************************************/
/*                Device Specific Peripheral registers structures             */
/******************************************************************************/
/** @addtogroup <Device>_Peripherals <Device> Peripherals
  <Device> Device Specific Peripheral registers structures
  @{
*/

#if defined ( __CC_ARM   )
#pragma anon_unions
#endif

#include <stdint.h>

/** @addtogroup Exported_types
  * @{
  */

typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

typedef enum {FALSE = 0, TRUE = !FALSE} Boolean;

typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;

/* ToDo: add here your device specific peripheral access structure typedefs
         following is an example for a timer                                  */
#define BIT0        (0x00000001U)
#define BIT1        (0x00000002U)
#define BIT2        (0x00000004U)
#define BIT3        (0x00000008U)
#define BIT4        (0x00000010U)
#define BIT5        (0x00000020U)
#define BIT6        (0x00000040U)
#define BIT7        (0x00000080U)
#define BIT8        (0x00000100U)
#define BIT9        (0x00000200U)
#define BIT10       (0x00000400U)
#define BIT11       (0x00000800U)
#define BIT12       (0x00001000U)
#define BIT13       (0x00002000U)
#define BIT14       (0x00004000U)
#define BIT15       (0x00008000U)
#define BIT16       (0x00010000U)
#define BIT17       (0x00020000U)
#define BIT18       (0x00040000U)
#define BIT19       (0x00080000U)
#define BIT20       (0x00100000U)
#define BIT21       (0x00200000U)
#define BIT22       (0x00400000U)
#define BIT23       (0x00800000U)
#define BIT24       (0x01000000U)
#define BIT25       (0x02000000U)
#define BIT26       (0x04000000U)
#define BIT27       (0x08000000U)
#define BIT28       (0x10000000U)
#define BIT29       (0x20000000U)
#define BIT30       (0x40000000U)
#define BIT31       (0x80000000U)

#define BIT(n)      (1UL << (n))

typedef struct {
    __IO uint32_t FREQ_SEL;
    __IO uint32_t CG_CTRL1;
    __IO uint32_t CG_CTRL2;
    __O uint32_t  SOFT_RST1;
    __O uint32_t  SOFT_RST2;
    __IO uint32_t LOCK_R;
    __IO uint32_t PHER_CTRL;
    __I uint32_t  SYS_RSVD[(0x2C - 0x1C) >> 2];
    __I uint32_t  HCLK_1MS_VAL;
    __I uint32_t  PCLK_1MS_VAL;
    __IO uint32_t ANA_CTRL;
    __IO uint32_t DMA_CHAN;
    __IO uint32_t SYS_RSVD1;
    __IO uint32_t SW_RSV1;
    __IO uint32_t SW_RSV2;
    __IO uint32_t CARD_RSVD;
    __IO uint32_t LDO25_CR;
    __IO uint32_t DMA_CHAN1;
    __I uint32_t  SYS_RSVD2[(0x100 - 0x54) >> 2];
    __IO uint32_t MSR_CR1;
    __IO uint32_t MSR_CR2;
    __IO uint32_t USBPHY_CR1;
    __IO uint32_t USBPHY_CR2;
    __IO uint32_t USBPHY_CR3;
    __I uint32_t  SYS_RSVD3[(0x200 - 0x114) >> 2];
    __IO uint32_t SDRAM_CTRL;
    __IO uint32_t RSVD_POR;
    __I uint32_t  SYS_RSVD4[(0x3EC - 0x208) >> 2];
    __IO uint32_t PM2_WK_FLAG;
    __IO uint32_t CALIB_CSR;
    __IO uint32_t DBG_CR;
    __IO uint32_t CHIP_ID;
} SYSCTRL_TypeDef;

typedef struct {
    union {
        __I  uint32_t RBR;
        __O  uint32_t THR;
        __IO uint32_t DLL;
    } OFFSET_0;
    union {
        __IO uint32_t DLH;
        __IO uint32_t IER;
    } OFFSET_4;
    union {
        __I uint32_t IIR;
        __O uint32_t FCR;
    } OFFSET_8;
    __IO uint32_t LCR;
    __IO uint32_t MCR;
    __I  uint32_t LSR;
    __I  uint32_t MSR;
    __IO uint32_t SCR;
    __IO uint32_t LPDLL;
    __IO uint32_t LPDLH;
    __I  uint32_t RES0[2];
    union {
        __I  uint32_t SRBR[16];
        __O  uint32_t STHR[16];
    } OFFSET_48;
    __IO uint32_t FAR;
    __I  uint32_t TFR;
    __O  uint32_t RFW;
    __I  uint32_t USR;
    __I  uint32_t TFL;
    __I  uint32_t RFL;
    __O  uint32_t SRR;
    __IO uint32_t SRTS;
    __IO uint32_t SBCR;
    __IO uint32_t SDMAM;
    __IO uint32_t SFE;
    __IO uint32_t SRT;
    __IO uint32_t STET;
    __IO uint32_t HTX;
    __O uint32_t DMASA;
    __I  uint32_t RES1[18];
    __I  uint32_t CPR;
    __I  uint32_t UCV;
    __I  uint32_t CTR;

} UART_TypeDef;

typedef struct {
    __IO uint16_t CTRLR0;
    uint16_t RESERVED0;
    __IO uint16_t CTRLR1;
    uint16_t RESERVED1;
    __IO uint32_t SSIENR;
    __IO uint32_t MWCR;
    __IO uint32_t SER;
    __IO uint32_t BAUDR;
    __IO uint32_t TXFTLR;
    __IO uint32_t RXFTLR;
    __IO uint32_t TXFLR;
    __I  uint32_t RXFLR;
    __I  uint32_t SR;
    __IO uint32_t IMR;
    __I  uint32_t ISR;
    __I  uint32_t RISR;
    __I  uint32_t TXOICR;
    __I  uint32_t RXOICR;
    __I  uint32_t RXUICR;
    __I  uint32_t MSTICR;
    __IO uint32_t ICR;
    __IO uint32_t DMACR;
    __IO uint32_t DMATDLR;
    __IO uint32_t DMARDLR;
    __I  uint32_t IDR;
    __I  uint32_t SSI_COMP_VERSION;
    __IO uint32_t DR;
    __IO uint32_t DR_Array[35];
    __IO uint32_t RX_SAMPLE_DLY;
} SPI_TypeDef;

typedef struct {
    __IO uint32_t FCU_CMD;
    __O  uint32_t ADDRES;
    __IO uint32_t BYTE_NUM;
    __O  uint32_t WR_FIFO;
    __I  uint32_t RD_FIFO;
    __IO uint32_t DEVICE_PARA;
    __IO uint32_t REG_WDATA;
    __O  uint32_t REG_RDATA;
    __IO uint32_t INT_MASK;
    __IO uint32_t INT_UMASK;
    __IO uint32_t INT_MASK_STATUS;
    __IO uint32_t INT_STATUS;
    __IO uint32_t INT_RAWSTATUS;
    __IO uint32_t INT_CLEAR;
    __IO uint32_t CACHE_INTF_CMD;
    __IO uint32_t DMA_CNTL;
    __IO uint32_t FIFO_CNTL;
} QSPI_TypeDef;

typedef struct {
    __IO uint32_t CACHE_AES_I0;
    __IO uint32_t CACHE_AES_I1;
    __IO uint32_t CACHE_AES_I2;
    __IO uint32_t CACHE_AES_I3;
    __IO uint32_t CACHE_AES_K0;
    __IO uint32_t CACHE_AES_K1;
    __IO uint32_t CACHE_AES_K2;
    __IO uint32_t CACHE_AES_K3;
    __IO uint32_t CACHE_AES_CS;
    __IO uint32_t CACHE_REF;
    __I  uint32_t CACHE_RSVD0[(0x40 - 0x28) >> 2];
    __IO uint32_t CACHE_CONFIG;
    __I  uint32_t CACHE_RSVD1[(0x74 - 0x44) >> 2];
    __IO uint32_t CACHE_SADDR;
    __IO uint32_t CACHE_EADDR;

} CACHE_TypeDef;

typedef struct {
    __IO uint32_t SMU_CTRL;
    __IO uint32_t FPM_CTRL;
    __O  uint32_t INTR_STAT;
    __IO uint32_t INTR_CTRL;
    __IO uint32_t RESERVED1[12];
    __IO uint32_t SMU_OP1;
    __IO uint32_t SMU_OP2;
    __O  uint32_t SMU_RES;
    __IO uint32_t RESERVED2[13];
    __IO float    MATRIX1_00;
    __IO float    MATRIX1_01;
    __IO float    MATRIX1_02;
    __IO float    MATRIX1_10;
    __IO float    MATRIX1_11;
    __IO float    MATRIX1_12;
    __IO float    MATRIX1_20;
    __IO float    MATRIX1_21;
    __IO float    MATRIX1_22;
    __IO uint32_t RESERVED3[7];
    __IO float    MATRIX2_00;
    __IO float    MATRIX2_01;
    __IO float    MATRIX2_02;
    __IO uint32_t RESERVED4[13];
    __IO uint32_t TABLE1_LEN;
    __IO uint32_t TABLE2_LEN;
    __IO uint32_t ACC;
    __IO uint32_t POSITION;
    __IO uint32_t VAL3;
    __IO uint32_t RESERVED5[443];
    __IO uint32_t TABLE1_RAM;
    __IO uint32_t RESERVED6[255];
    __IO uint32_t TABLE2_RAM;
    __IO uint32_t RESERVED7[63];
} QRCODE_TypeDef;

typedef struct {
    __IO uint32_t SADDR;
    __IO uint32_t DADDR;
    __IO uint32_t RADDR;
    __IO uint32_t S_W;
    __IO uint32_t S_H;
    __IO uint32_t S_ROW_SKIP;
    __IO uint32_t S_WND_SKIP;
    __IO uint32_t R_W;
    __IO uint32_t R_H;
    __IO uint32_t WND_STEP;
    __IO uint32_t WND_W;
    __IO uint32_t WND_H;
    __IO uint32_t CAL_FACTIR;
    __IO uint32_t D_ROW_SKIP;
    __IO uint32_t GPU_CR;
    __IO uint32_t RLC_BN;
} GPU_TypeDef;

typedef struct {
    __IO uint32_t WDT_CR;
    __IO uint32_t RESERVED0;
    __I  uint32_t WDT_CCVR;
    __O  uint32_t WDT_CRR;
    __I  uint32_t WDT_STAT;
    __I  uint32_t WDT_EOI;
    __I  uint32_t RESERVED1;
    __IO uint32_t WDT_RLD;
    __I  uint32_t RESERVED[53];
    __I  uint32_t WDT_COMP_PARAMS_1;
    __I  uint32_t WDT_COMP_VERSION;
    __I  uint32_t WDT_COMP_TYPE;
} WDT_TypeDef;

typedef struct {
    __IO uint32_t CRC_CSR;
    __O  uint32_t CRC_INI;
    union {
        __I uint32_t DOUT;
        __O uint8_t  DIN;
    } CRC_DATA;
} CRC_TypeDef;

typedef struct {
    __IO uint32_t LoadCount;
    __I  uint32_t CurrentValue;
    __IO uint32_t ControlReg;
    __IO  uint32_t EOI;
    __I  uint32_t IntStatus;
} TIM_TypeDef;

#define TIM_NUM      8
typedef struct {
    TIM_TypeDef TIM[TIM_NUM];
    __I  uint32_t TIM_IntStatus;
    __I  uint32_t TIM_EOI;
    __I  uint32_t TIM_RawIntStatus;
    __I  uint32_t TIM_Comp;
    __IO uint32_t TIM_ReloadCount[TIM_NUM];
} TIM_Module_TypeDef;

typedef struct {
    __IO uint32_t ADC_CR1;
    __I  uint32_t ADC_SR;
    __IO uint32_t ADC_FIFO;
    __I  uint32_t ADC_DATA;
    __I  uint32_t ADC_FIFO_FL;
    __IO uint32_t ADC_FIFO_THR;
    __IO uint32_t ADC_CR2;
} ADC_TypeDef;

typedef struct {
    __IO uint32_t DAC_CR1;
    __IO uint32_t DAC_DATA;
    __IO uint32_t DAC_TIMER;
    __I  uint32_t DAC_FIFO_FL;
    __IO uint32_t DAC_FIFO_THR;
} DAC_TypeDef;

typedef struct {
    __IO uint32_t AWD_CR1;
    __IO uint32_t AWD_CR2;
    __I  uint32_t AWD_SR;
} AWD_TypeDef;

typedef struct {
    __IO uint32_t IODR;
    __IO uint32_t BSRR;
    __IO uint32_t OEN;
    __IO uint32_t PUE;
} GPIO_TypeDef;

typedef struct {
    __IO uint32_t INTP_TYPE;
    __IO uint32_t INTP_STA;
} GPIO_INTP_TypeDef;

#define GPIO_GROUP_NUM    8
typedef struct {
    GPIO_TypeDef GPIO[GPIO_GROUP_NUM];
    __I  uint32_t RSVD0[(0x10C - 0x080) >> 2];
    __I  uint32_t INTP[GPIO_GROUP_NUM];
    __I  uint32_t RSVD1[(0x180 - 0x12C) >> 2];
    __IO uint32_t ALT[GPIO_GROUP_NUM];
    __I  uint32_t RSVD2[(0x200 - 0x1a0) >> 2];
    __IO uint32_t SYS_CR1;
    __I  uint32_t RSVD3[(0x220 - 0x204) >> 2];
    __IO uint32_t WAKE_TYPE_EN;
    __IO uint32_t WAKE_P0_EN;
    __IO uint32_t WAKE_P1_EN;
    __IO uint32_t WAKE_P2_EN;
    __IO uint32_t WAKE_P3_EN;
    __I  uint32_t RSVD5[(0x800 - 0x234) >> 2];
    GPIO_INTP_TypeDef INTP_TYPE_STA[GPIO_GROUP_NUM];
} GPIO_MODULE_TypeDef;


typedef struct {
    __IO uint32_t FLAG[(0x0174 - 0x00164) >> 2];
} FLAG_TypeDef;

#define BPK_KEY_NUM      16
typedef struct {
    __IO uint32_t KEY[BPK_KEY_NUM];
    __I  uint32_t BPK_RSVD0[(0x80 - 0x40) >> 2];
    __IO uint32_t BPK_RDY;
    __IO uint32_t BPK_CLR;
    __IO uint32_t BPK_LRA;
    __IO uint32_t BPK_LWA;
    __I  uint32_t BPK_RSVD1;
    __IO uint32_t BPK_LR;
    __I  uint32_t BPK_RSVD2[(0xA0 - 0x98) >> 2];

    __IO uint32_t RTC_CS;
    __IO uint32_t RTC_REF;
    __IO uint32_t RTC_ARM;
    __I  uint32_t RTC_TIM;
    __O  uint32_t RTC_INTCLR;
    __IO uint32_t OSC32K_CR;
    __IO uint32_t RTC_ATTA_TIM;

    __IO uint32_t BPK_RR;
    __IO uint32_t SEN_EXT_TYPE;
    __IO uint32_t SEN_EXT_CFG;
    __IO uint32_t SEN_SOFT_EN;
    __IO uint32_t SEN_STATE;
    __IO uint32_t SEN_BRIDGE;
    __IO uint32_t SEN_SOFT_ATTACK;
    __IO uint32_t SEN_SOFT_LOCK;
    __IO uint32_t SEN_ATTACK_CNT;
    __IO uint32_t SEN_ATTACK_TYP;
    __IO uint32_t SEN_VG_DETECT;
    __IO uint32_t SEN_RNG_INI;
    __IO uint32_t RESERVED3[(0x0104 - 0x00EC) >> 2];
    __IO uint32_t SEN_EN[19];
    __IO uint32_t SEN_EXTS_START;
    __IO uint32_t SEN_LOCK;
    __IO uint32_t SEN_ANA0;
    __IO uint32_t SEN_ANA1;
    __IO uint32_t SEN_ATTCLR;
    FLAG_TypeDef  SEN_FLAG;
    __IO uint32_t SEN_DEBUG;
} BPU_MODULE_TypeDef;

typedef struct {
    __IO uint32_t KEY[BPK_KEY_NUM];
    __I  uint32_t BPK_RSVD0[(0x80 - 0x40) >> 2];
    __IO uint32_t BPK_RDY;
    __IO uint32_t BPK_CLR;
    __IO uint32_t BPK_LRA;
    __IO uint32_t BPK_LWA;
    __I  uint32_t BPK_RSVD1;
    __IO uint32_t BPK_LR;
    __O  uint32_t BPK_SCR;
    __I  uint32_t BPK_RSVD2[(0xA0 - 0x98) >> 2];
} BPK_TypeDef;

typedef struct {
    __IO uint32_t RTC_CS;
    __IO uint32_t RTC_REF;
    __IO uint32_t RTC_ARM;
    __I  uint32_t RTC_TIM;
    __O  uint32_t RTC_INTCLR;
    __IO uint32_t OSC32K_CR;
    __IO uint32_t RTC_ATTA_TIM;
} RTC_TypeDef;

#define EXT_SENSOR_NUM         8
#define INNER_SENSOR_NUM       7
typedef struct {
    __IO uint32_t BPK_RR;
    __IO uint32_t SEN_EXT_TYPE;
    __IO uint32_t SEN_EXT_CFG;
    __IO uint32_t SEN_SOFT_EN;
    __IO uint32_t SEN_STATE;
    __IO uint32_t SEN_BRIDGE;
    __IO uint32_t SEN_SOFT_ATTACK;
    __IO uint32_t SEN_SOFT_LOCK;
    __IO uint32_t SEN_ATTACK_CNT;
    __IO uint32_t SEN_ATTACK_TYP;
    __IO uint32_t SEN_VG_DETECT;
    __IO uint32_t SEN_RNG_INI;
    __IO uint32_t RESERVED3[(0x0104 - 0x00EC) >> 2];
    __IO uint32_t SEN_EN[19];
    __IO uint32_t SEN_EXTS_START;
    __IO uint32_t SEN_LOCK;
    __IO uint32_t SEN_ANA0;
    __IO uint32_t SEN_ANA1;
    __IO uint32_t SEN_ATTCLR;
    FLAG_TypeDef  SEN_FLAG;
    __IO uint32_t SEN_DEBUG;
} SEN_TypeDef;


typedef struct {
    __IO uint32_t RNG_CSR;
    __IO uint32_t RNG_DATA[1];
    __I  uint32_t RES;
    __IO uint32_t RNG_AMA;
    __IO uint32_t RNG_PN;
    __IO uint32_t RNG_INDEX;
} TRNG_TypeDef;

typedef struct {
    __IO uint32_t IC_CON;
    __IO uint32_t IC_TAR;
    __IO uint32_t IC_SAR;
    __IO uint32_t IC_HS_MADDR;
    __IO uint32_t IC_DATA_CMD;
    __IO uint32_t IC_SS_SCL_HCNT;
    __IO uint32_t IC_SS_SCL_LCNT;
    __IO uint32_t IC_FS_SCL_HCNT;
    __IO uint32_t IC_FS_SCL_LCNT;
    __IO uint32_t IC_HS_SCL_HCNT;
    __IO uint32_t IC_HS_SCL_LCNT;
    __I  uint32_t IC_INTR_STAT;
    __IO uint32_t IC_INTR_MASK;
    __I  uint32_t IC_RAW_INTR_STAT;
    __IO uint32_t IC_RX_TL;
    __IO uint32_t IC_TX_TL;
    __I  uint32_t IC_CLR_INTR;
    __I  uint32_t IC_CLR_RX_UNDER;
    __I  uint32_t IC_CLR_RX_OVER;
    __I  uint32_t IC_CLR_TX_OVER;
    __I  uint32_t IC_CLR_RD_REQ;
    __I  uint32_t IC_CLR_TX_ABRT;
    __I  uint32_t IC_CLR_RX_DONE;
    __I  uint32_t IC_CLR_ACTIVITY;
    __I  uint32_t IC_CLR_STOP_DET;
    __I  uint32_t IC_CLR_START_DET;
    __I  uint32_t IC_CLR_GEN_CALL;
    __IO uint32_t IC_ENABLE;
    __I  uint32_t IC_STATUS;
    __I  uint32_t IC_TXFLR;
    __I  uint32_t IC_RXFLR;
    __IO uint32_t IC_SDA_HOLD;
    __I  uint32_t IC_TX_ABRT_SOURCE;
    __IO uint32_t IC_SLV_DATA_NACK_ONLY;
    __IO uint32_t IC_DMA_CR;
    __IO uint32_t IC_DMA_TDLR;
    __IO uint32_t IC_DMA_RDLR;
    __IO uint32_t IC_SDA_SETUP;
    __IO uint32_t IC_ACK_GENERAL_CALL;
    __I  uint32_t IC_ENABLE_STATUS;
    __IO uint32_t IC_FS_SPKLEN;
    __IO uint32_t IC_HS_SPKLEN;
} I2C_TypeDef;

typedef struct {
    __IO uint32_t KCU_CTRL0;
    __IO uint32_t KCU_CTRL1;
    __I  uint32_t KCU_STATUS;
    __I  uint32_t KCU_EVENT;
    __IO uint32_t KCU_RNG;
} KCU_TypeDef;

typedef struct {
    __IO uint32_t SAR_L;
    __IO uint32_t SAR_H;
    __IO uint32_t DAR_L;
    __IO uint32_t DAR_H;
    __IO uint32_t LLP_L;
    __IO uint32_t LLP_H;
    __IO uint32_t CTL_L;
    __IO uint32_t CTL_H;
    __IO uint32_t SSTAT_L;
    __IO uint32_t SSTAT_H;
    __IO uint32_t DSTAT_L;
    __IO uint32_t DSTAT_H;
    __IO uint32_t SSTATAR_L;
    __IO uint32_t SSTATAR_H;
    __IO uint32_t DSTATAR_L;
    __IO uint32_t DSTATAR_H;
    __IO uint32_t CFG_L;
    __IO uint32_t CFG_H;
    __IO uint32_t SGR_L;
    __IO uint32_t SGR_H;
    __IO uint32_t DSR_L;
    __IO uint32_t DSR_H;
} DMA_TypeDef;

typedef struct {
    DMA_TypeDef DMA_Channel[8];

    __I  uint32_t RawTfr_L;
    __I  uint32_t RawTfr_H;
    __I  uint32_t RawBlock_L;
    __I  uint32_t RawBlock_H;
    __I  uint32_t RawSrcTran_L;
    __I  uint32_t RawSrcTran_H;
    __I  uint32_t RawDstTran_L;
    __I  uint32_t RawDstTran_H;
    __I  uint32_t RawErr_L;
    __I  uint32_t RawErr_H;

    __I  uint32_t StatusTfr_L;
    __I  uint32_t StatusTfr_H;
    __I  uint32_t StatusBlock_L;
    __I  uint32_t StatusBlock_H;
    __I  uint32_t StatusSrcTran_L;
    __I  uint32_t StatusSrcTran_H;
    __I  uint32_t StatusDstTran_L;
    __I  uint32_t StatusDstTran_H;
    __I  uint32_t StatusErr_L;
    __I  uint32_t StatusErr_H;

    __IO uint32_t MaskTfr_L;
    __IO uint32_t MaskTfr_H;
    __IO uint32_t MaskBlock_L;
    __IO uint32_t MaskBlock_H;
    __IO uint32_t MaskSrcTran_L;
    __IO uint32_t MaskSrcTran_H;
    __IO uint32_t MaskDstTran_L;
    __IO uint32_t MaskDstTran_H;
    __IO uint32_t MaskErr_L;
    __IO uint32_t MaskErr_H;

    __O  uint32_t ClearTfr_L;
    __O  uint32_t ClearTfr_H;
    __O  uint32_t ClearBlock_L;
    __O  uint32_t ClearBlock_H;
    __O  uint32_t ClearSrcTran_L;
    __O  uint32_t ClearSrcTran_H;
    __O  uint32_t ClearDstTran_L;
    __O  uint32_t ClearDstTran_H;
    __O  uint32_t ClearErr_L;
    __O  uint32_t ClearErr_H;

    __I  uint32_t StatusInt_L;
    __I  uint32_t StatusInt_H;

    __IO uint32_t ReqSrcReg_L;
    __IO uint32_t ReqSrcReg_H;
    __IO uint32_t ReqDstReg_L;
    __IO uint32_t ReqDstReg_H;
    __IO uint32_t SglReqSrcReg_L;
    __IO uint32_t SglReqSrcReg_H;
    __IO uint32_t SglReqDstReg_L;
    __IO uint32_t SglReqDstReg_H;
    __IO uint32_t LstSrcReg_L;
    __IO uint32_t LstSrcReg_H;
    __IO uint32_t LstDstReg_L;
    __IO uint32_t LstDstReg_H;

    __IO uint32_t DmaCfgReg_L;
    __IO uint32_t DmaCfgReg_H;
    __IO uint32_t ChEnReg_L;
    __IO uint32_t ChEnReg_H;
    __I  uint32_t DmaIdReg_L;
    __I  uint32_t DmaIdReg_H;
    __IO uint32_t DmaTestReg_L;
    __IO uint32_t DmaTestReg_H;

    __IO uint32_t RESERVED2[4];

    __I  uint32_t DMA_COMP_PARAMS_6_L;
    __I  uint32_t DMA_COMP_PARAMS_6_H;
    __I  uint32_t DMA_COMP_PARAMS_5_L;
    __I  uint32_t DMA_COMP_PARAMS_5_H;
    __I  uint32_t DMA_COMP_PARAMS_4_L;
    __I  uint32_t DMA_COMP_PARAMS_4_H;
    __I  uint32_t DMA_COMP_PARAMS_3_L;
    __I  uint32_t DMA_COMP_PARAMS_3_H;
    __I  uint32_t DMA_COMP_PARAMS_2_L;
    __I  uint32_t DMA_COMP_PARAMS_2_H;
    __I  uint32_t DMA_COMP_PARAMS_1_L;
    __I  uint32_t DMA_COMP_PARAMS_1_H;
    __I  uint32_t DMA_Component_ID_Register_L;
    __I  uint32_t DMA_Component_ID_Register_H;

} DMA_MODULE_TypeDef;

typedef struct {
    __IO uint32_t lcdi_ctrl;
    __IO uint32_t lcdi_cycle;
    __IO uint32_t lcdi_status;
    __IO uint32_t lcdi_data;
    __IO uint32_t lcdi_fifolevel;
    __IO uint32_t lcdi_fifothr;
} LCD_TypeDef;

typedef struct {
    __IO uint32_t SCI_DATA;
    __IO uint32_t SCI_CR0;
    __IO uint32_t SCI_CR1;
    __IO uint32_t SCI_CR2;
    __IO uint32_t SCI_IER;
    __IO uint32_t SCI_RETRY;
    __IO uint32_t SCI_TIDE;
    __IO uint32_t SCI_TXCOUNT;
    __IO uint32_t SCI_RXCOUNT;
    __I  uint32_t SCI_FR;
    __IO uint32_t SCI_RXTIME;
    __IO uint32_t SCI_ISTAT;
    __IO uint32_t SCI_STABLE;
    __IO uint32_t SCI_ATIME;
    __IO uint32_t SCI_DTIME;

    __IO uint32_t SCI_ATRSTIME;
    __IO uint32_t SCI_ATRDTIME;
    __IO uint32_t SCI_BLKTIME;
    __IO uint32_t SCI_CHTIME;
    __IO uint32_t SCI_CLKICC;
    __IO uint32_t SCI_BAUD;
    __IO uint32_t SCI_VALUE;
    __IO uint32_t SCI_CHGUARD;
    __IO uint32_t SCI_BLKGUARD;
    __IO uint32_t SCI_SYNCCR;
    __IO uint32_t SCI_SYNCDATA;
    __IO uint32_t SCI_RAWSTAT;
    __IO uint32_t SCI_IIR;
    __I  uint32_t SCI_RES1[4];
    __I  uint32_t SCI_RES2[32];
} SCI_TypeDef;



typedef struct {
    __IO uint32_t CTRL;
    __IO uint32_t PWREN;
    __IO uint32_t CLKDIV;
    __I  uint32_t SDIO_RSVD0;
    __IO uint32_t CLKENA;
    __IO uint32_t TMOUT;
    __IO uint32_t CTYPE;
    __IO uint32_t BLKSIZ;
    __IO uint32_t BYTCNT;
    __IO uint32_t INTMASK;
    __IO uint32_t CMDARG;
    __IO uint32_t CMD;
    __I  uint32_t RESP0;
    __I  uint32_t RESP1;
    __I  uint32_t RESP2;
    __I  uint32_t RESP3;
    __I  uint32_t MINTSTS;
    __IO uint32_t RINTSTS;
    __I  uint32_t STATUS;
    __IO uint32_t FIFOTH;
    __I  uint32_t CDETECT;
    __I  uint32_t WRTPRT;
    __I  uint32_t SDIO_RSVD1;
    __I  uint32_t TCBCNT;
    __I  uint32_t TBBCNT;
    __IO uint32_t DEBNCE;
    __I  uint32_t SDIO_RSVD2[(0x100 - 0x68) >> 2];
    __IO uint32_t CARDTHRCTL;
    __I  uint32_t SDIO_RSVD3;
    __IO uint32_t UHS_REG_EXT;
    __IO uint32_t EMMC_DDR_REG;
    __I  uint32_t SDIO_RSVD4[(0x200 - 0x110) >> 2];
    __IO uint32_t FIFO;

} SDIO_TypeDef;


/**
  * @brief DCMI&IMG COP
  */

typedef struct {
    __IO uint32_t CR;       /*!< DCMI control register 1,                       Address offset: 0x00 */
    __IO uint32_t SR;       /*!< DCMI status register,                          Address offset: 0x04 */
    __IO uint32_t RISR;     /*!< DCMI raw interrupt status register,            Address offset: 0x08 */
    __IO uint32_t IER;      /*!< DCMI interrupt enable register,                Address offset: 0x0C */
    __IO uint32_t MISR;     /*!< DCMI masked interrupt status register,         Address offset: 0x10 */
    __IO uint32_t ICR;      /*!< DCMI interrupt clear register,                 Address offset: 0x14 */
    __IO uint32_t RESERVED1[2];
    __IO uint32_t CWSTRTR;  /*!< DCMI crop window start,                        Address offset: 0x20 */
    __IO uint32_t CWSIZER;  /*!< DCMI crop window size,                         Address offset: 0x24 */
    __IO uint32_t DR;       /*!< DCMI data register,                            Address offset: 0x28 */
} DCMI_TypeDef;

typedef struct {
    __IO uint32_t IMG_COP_CONFIG;
    __I uint32_t STATUS;
    __IO uint32_t INTC;
    __IO uint32_t IMG_SIZE;
    __IO uint32_t IMG_ADDR;
    __IO uint32_t INTGIMG_ADDR;
    __IO uint32_t BINIMG_ADDR;
    __IO uint32_t WINDOW_ADDR;
    __IO uint32_t SEARCH_PIXEL;
    __IO uint32_t SEARCH_MAX;
    __I  uint32_t SUM_X;
    __I  uint32_t SUM_Y;
    __I  uint32_t SUM_TOTAL;
    __I  uint32_t SEARCH_COUNT;
} IMG_COP_TypeDef;

typedef struct {
    __O  uint32_t   RESERVED[(0x2000 - 0x0000) / 4];
    __IO uint32_t   CFG;
    __IO uint32_t   CS;
    __IO uint32_t   PROT;
    __IO uint32_t   ADDR;
    __IO uint32_t   PDATA;
    __IO uint32_t   RO;
    __IO uint32_t   ROL;
    __IO uint32_t   RSVD;
    __IO uint32_t   TIM;
    __IO uint32_t   TIM_EN;
} OTP_TypeDef;

typedef struct {
    __IO uint32_t  RESERVED0[(0x0008 - 0x0000) >> 2];
    __IO uint32_t  SSC_CR3;
    __O  uint32_t  RESERVED1[(0x0104 - 0x000C) >> 2];
    __IO uint32_t  SSC_SR;
    __IO uint32_t  SSC_SR_CLR;
    __IO uint32_t  SSC_ACK;
    __O  uint32_t  RESERVED2[(0x0184 - 0x0110) >> 2];
    __IO uint32_t  DATARAM_SCR;
    __O  uint32_t  RESERVED3[(0x01FC - 0x0188) >> 2];
    __IO uint32_t  BPU_RWC;
    __O  uint32_t  RESERVED4[(0x03EC - 0x0200) >> 2];
    __IO uint32_t  MAIN_SEN_LOCK;
    __IO uint32_t  MAIN_SEN_EN;
} SSC_TypeDef;

typedef struct {
    __IO uint32_t CONTROL;              // 0x0000
    __IO uint32_t STATUS;               // 0x0004
    __IO uint32_t CONFIG;               // 0x0008
    __IO uint32_t RSVD0[(0x0020 - 0x000C) >> 2];
    __IO uint32_t DYN_CONTROL;          // 0x0020
    __IO uint32_t DYN_REFRESH;          // 0x0024
    __IO uint32_t DYN_READCONFIG;       // 0x0028
    __IO uint32_t RSVD1[(0x0030 - 0x002C) >> 2];
    __IO uint32_t DYN_RP;               // 0x0030
    __IO uint32_t DYN_RAS;              // 0x0034
    __IO uint32_t DYN_SREX;             // 0x0038
    __IO uint32_t RSVD2[(0x0044 - 0x003C) >> 2];
    __IO uint32_t DYN_WR;               // 0x0044
    __IO uint32_t DYN_RC;               // 0x0048
    __IO uint32_t DYN_RFC;              // 0x004C
    __IO uint32_t DYN_XSR;              // 0x0050
    __IO uint32_t DYN_RRD;              // 0x0054
    __IO uint32_t DYN_MRD;              // 0x0058
    __IO uint32_t DYN_CDLR;             // 0x005C
    __IO uint32_t RSVD4[(0x0120 - 0x0060) >> 2];
    __IO uint32_t DYN_CONFIG;           // 0x0120
    __IO uint32_t DYN_RASCAS;           // 0x0124
} SDRAM_TypeDef;

typedef struct {
    __IO uint32_t  TST_JTAG;
    __IO uint32_t  TST_ROM;
    __IO uint32_t  TST_FLASH;
} MH_SMCU_TST_TypeDef;


typedef struct {
    __IO uint32_t MST_CR;
    __IO uint32_t MST_CONFIG;
    __IO uint32_t MST_ADDR;
    __I  uint32_t MST_RIS;
    __IO uint32_t MST_IER;
    __I  uint32_t MST_MIS;
    __O  uint32_t MST_ICR;
} DCMI_MST_TypeDef;

typedef struct {
    __IO uint32_t PSRAM_CMD;
    __IO uint32_t DEVICE_PARA;
    __O uint32_t  DEVICE_ID_H;
    __O uint32_t  DEVICE_ID_L;
} PSRAM_Typedef;

typedef struct {
    __I  uint32_t RSVD0;
    __IO uint32_t N_LANES;
    __IO uint32_t RESETN;
    __I  uint32_t INT_ST_MAIN;
    __IO uint32_t DATA_IDS_1;
    __I  uint32_t RSVD1[(0x40 - 0x14) >> 2];
    __IO uint32_t PHY_SHUTDOWNZ;
    __IO uint32_t DPHY_RSTZ;
    __I  uint32_t PHY_RX;
    __I  uint32_t PHY_STOPSTATE;
    __IO uint32_t PHY_TEST_CTRL0;
    __IO uint32_t PHY_TEST_CTRL1;
    __I  uint32_t RSVD2[(0x80 - 0x58) >> 2];
    __IO uint32_t IPI_MODE;
    __IO uint32_t IPI_VCID;
    __IO uint32_t IPI_DATA_TYPE;
    __IO uint32_t IPI_MEM_FLUSH;
    __IO uint32_t IPI_HSA_TIME;
    __IO uint32_t IPI_HBP_TIME;
    __IO uint32_t IPI_HSD_TIME;
    __IO uint32_t IPI_HLINE_TIME;
    __I  uint32_t RSVD3[(0xb0 - 0xa0) >> 2];
    __IO uint32_t IPI_VSA_LINES;
    __IO uint32_t IPI_VBP_LINES;
    __IO uint32_t IPI_VFP_LINES;
    __IO uint32_t IPI_VACTIVE_LINES;
    __I  uint32_t RSVD4[(0xe0 - 0xc0) >> 2];
    __I  uint32_t INT_ST_PHY_FATAL;
    __IO uint32_t INT_MSK_PHY_FATAL;
    __IO uint32_t INT_FORCE_PHY_FATAL;
    __I  uint32_t RSVD5;
    __I  uint32_t INT_ST_PKT_FATAL;
    __IO uint32_t INT_MSK_PKT_FATAL;
    __IO uint32_t INT_FORCE_PKT_FATAL;
    __I  uint32_t RSVD6;
    __I  uint32_t INT_ST_FRAME_FATAL;
    __IO uint32_t INT_MSK_FRAME_FATAL;
    __IO uint32_t INT_FORCE_FRAME_FATAL;
    __I  uint32_t RSVD7;
    __I  uint32_t INT_ST_PHY;
    __IO uint32_t INT_MSK_PHY;
    __IO uint32_t INT_FORCE_PHY;
    __I  uint32_t RSVD8;
    __I  uint32_t INT_ST_PKT;
    __IO uint32_t INT_MSK_PKT;
    __IO uint32_t INT_FORCE_PKT;
    __I  uint32_t RSVD9;
    __I  uint32_t INT_ST_LINE;
    __IO uint32_t INT_MSK_LINE;
    __IO uint32_t INT_FORCE_LINE;
    __I  uint32_t RSVD10;
    __I  uint32_t INT_ST_IPI;
    __IO uint32_t MSK_INT_IPI;
    __IO uint32_t FORCE_INT_IPI;
} CSI2_TypeDef;


#if defined ( __CC_ARM   )
#pragma no_anon_unions
#endif

/*@}*/ /* end of group <Device>_Peripherals */


/******************************************************************************/
/*                         Peripheral memory map                              */
/******************************************************************************/
/* ToDo: add here your device peripherals base addresses
         following is an example for timer                                    */
/** @addtogroup <Device>_MemoryMap <Device> Memory Mapping
  @{
*/

/* Peripheral and SRAM base address */
#define MHSCPU_FLASH_BASE                       (0x01000000UL)                /*!< (FLASH     ) Base Address */
#define MHSCPU_SRAM_BASE                        (0x20000000UL)                /*!< (SRAM      ) Base Address */
#define MHSCPU_SRAM_SIZE                        (1UL << 20)
#define MHSCPU_PERIPH_BASE                      (0x40000000UL)                /*!< (Peripheral) Base Address */

#define MHSCPU_EXT_FLASH_BASE                   (0x60000000UL)
#define MHSCPU_EXT_RAM_BASE                     (0x80000000UL)

#define MHSCPU_SDRAM_BASE                       (0x60000000UL)
#define MHSCPU_SDRAM_END                        (0x7FFFFFFFUL)
#define MHSCPU_SDRAM_SIZE                       (0x01000000UL)                /*!< 16MBytes */

#define MHSCPU_PSRAM_BASE                       (0x80000000UL)   //(0x807FC000UL)
#define MHSCPU_PSRAM_END                        (0x9FFFFFFFUL)
#define MHSCPU_PSRAM_SIZE                       (0x00800000UL)   //(0x00004000UL)                /*!< 8MBytes */

#define OTP_SIZE                                (1UL << 13)

#define SRAM_SIZE                               (1UL << 20)
#define SRAM_BASE                               (0x20000000UL)

/* Peripheral memory map */
#define MHSCPU_AHB_BASE                         (MHSCPU_PERIPH_BASE)
#define MHSCPU_APB0_BASE                        (MHSCPU_PERIPH_BASE + 0x10000)
#define MHSCPU_APB1_BASE                        (MHSCPU_PERIPH_BASE + 0x20000)
#define MHSCPU_APB2_BASE                        (MHSCPU_PERIPH_BASE + 0x30000)
#define MHSCPU_APB3_BASE                        (MHSCPU_PERIPH_BASE + 0x40000)

#define SSC_BASE                                (MHSCPU_AHB_BASE + 0x0000)
#define TST_BASE                                (MHSCPU_AHB_BASE + 0x03F4)
#define DMA_BASE                                (MHSCPU_AHB_BASE + 0x0800)
#define USB_BASE                                (MHSCPU_AHB_BASE + 0x0C00)
#define LCD_BASE                                (MHSCPU_AHB_BASE + 0x1000)
#define OTP_BASE                                (MHSCPU_AHB_BASE + 0x8000)
#define DCMI_BASE                               (MHSCPU_AHB_BASE + 0x60000) /*DCMI����ַ*/
#define SDRAM_BASE                              (MHSCPU_AHB_BASE + 0x70000)
#define CACHE_BASE                              (MHSCPU_AHB_BASE + 0x80000)
#define QRCODE_BASE                             (MHSCPU_AHB_BASE + 0x90000)
#define PSRAM_BASE                              (MHSCPU_AHB_BASE + 0xA0000)
#define GPU_BASE                                (MHSCPU_AHB_BASE + 0xA1000)
#define QSPI_BASE                               (MHSCPU_AHB_BASE + 0xA2000)
#define DCMI_MST_BASE                           (MHSCPU_AHB_BASE + 0x4A000)  /* need modify  */

#define SCI0_BASE                               (MHSCPU_APB0_BASE)
#define SCI1_BASE                               (MHSCPU_APB0_BASE + 0x1000)

#define CRC_BASE                                (MHSCPU_APB0_BASE + 0x2000)
#define TIMM0_BASE                              (MHSCPU_APB0_BASE + 0x3000)
#define ADC_BASE                                (MHSCPU_APB0_BASE + 0x4000)
#define DAC_BASE                                (MHSCPU_APB0_BASE + 0x4100)
#define AWD_BASE                                (MHSCPU_APB0_BASE + 0x4200)
#define SCI2_BASE                               (MHSCPU_APB0_BASE + 0x5000)
#define UART0_BASE                              (MHSCPU_APB0_BASE + 0x6000)
#define UART1_BASE                              (MHSCPU_APB0_BASE + 0x7000)
#define SPIM1_BASE                              (MHSCPU_APB0_BASE + 0x8000)
#define SPIM2_BASE                              (MHSCPU_APB0_BASE + 0x9000)
#define SPIM0_BASE                              (MHSCPU_APB0_BASE + 0xA000)
#define SPIS0_BASE                              (MHSCPU_APB0_BASE + 0xB000)
#define WDG_BASE                                (MHSCPU_APB0_BASE + 0xC000)
#define GPIO_BASE                               (MHSCPU_APB0_BASE + 0xD000)
#define TRNG_BASE                               (MHSCPU_APB0_BASE + 0xE000)
#define SYSCTRL_BASE                            (MHSCPU_APB0_BASE + 0xF000)

#define MSR_BASE                                (MHSCPU_APB1_BASE)

#define BPU_BASE                                (MHSCPU_APB2_BASE)

#define SPIM3_BASE                              (MHSCPU_APB3_BASE)
#define SPIM4_BASE                              (MHSCPU_APB3_BASE + 0x1000)
#define UART2_BASE                              (MHSCPU_APB3_BASE + 0x4000)
#define UART3_BASE                              (MHSCPU_APB3_BASE + 0x5000)
#define KEYBOARD_BASE                           (MHSCPU_APB3_BASE + 0x8000)
#define I2C0_BASE                               (MHSCPU_APB3_BASE + 0x9000)
#define SDIO_BASE                               (MHSCPU_APB3_BASE + 0xE000)
#define CSI2_BASE                               (MHSCPU_APB3_BASE + 0xF000)


/*@}*/ /* end of group <Device>_MemoryMap */


/******************************************************************************/
/*                         Peripheral declaration                             */
/******************************************************************************/
/* ToDo: add here your device peripherals pointer definitions
         following is an example for timer                                    */

/** @addtogroup <Device>_PeripheralDecl <Device> Peripheral Declaration
  @{
*/
#define SYSCTRL                                 ((SYSCTRL_TypeDef *) SYSCTRL_BASE)

#define UART0                                   ((UART_TypeDef *) UART0_BASE)
#define UART1                                   ((UART_TypeDef *) UART1_BASE)
#define UART2                                   ((UART_TypeDef *) UART2_BASE)
#define UART3                                   ((UART_TypeDef *) UART3_BASE)

#define SPIM0                                   ((SPI_TypeDef *) SPIM0_BASE)
#define SPIM1                                   ((SPI_TypeDef *) SPIM1_BASE)
#define SPIM2                                   ((SPI_TypeDef *) SPIM2_BASE)
#define SPIM3                                   ((SPI_TypeDef *) SPIM3_BASE)
#define SPIM4                                   ((SPI_TypeDef *) SPIM4_BASE)
#define SPIS0                                   ((SPI_TypeDef *) SPIS0_BASE)

#define PSRAM                                   ((PSRAM_Typedef *)PSRAM_BASE)
#define QSPI                                    ((QSPI_TypeDef *) QSPI_BASE)

#define MFC                                     ((MFC_TypeDef *) MFC_BASE)
#define CACHE                                   ((CACHE_TypeDef *)CACHE_BASE)
#define QRCODE                                  ((QRCODE_TypeDef *)QRCODE_BASE)
#define GPU                                     ((GPU_TypeDef *)GPU_BASE)
#define SCI0                                    ((SCI_TypeDef *) SCI0_BASE)
#define SCI1                                    ((SCI_TypeDef *) SCI1_BASE)
#define SCI2                                    ((SCI_TypeDef *) SCI2_BASE)

#define TIMM0                                   ((TIM_Module_TypeDef *)TIMM0_BASE)

#define ADC0                                    ((ADC_TypeDef *)ADC_BASE)
#define DAC                                     ((DAC_TypeDef *)DAC_BASE)
#define AWD                                     ((AWD_TypeDef *)AWD_BASE)

#define TRNG                                    ((TRNG_TypeDef *)TRNG_BASE)
#define LCD                                     ((LCD_TypeDef *)LCD_BASE)
#define KCU                                     ((KCU_TypeDef *)KEYBOARD_BASE)
#define CRC                                     ((CRC_TypeDef *)CRC_BASE)
#define OTP                                     ((OTP_TypeDef *)OTP_BASE)

#define SDIO                                    ((SDIO_TypeDef *)SDIO_BASE)
#define CSI2                                    ((CSI2_TypeDef *)CSI2_BASE)

#define I2C0                                    ((I2C_TypeDef *)I2C0_BASE)

#define DMA                                     ((DMA_MODULE_TypeDef *)DMA_BASE)
#define DMA_Channel_0                           ((DMA_TypeDef *)DMA_BASE)
#define DMA_Channel_1                           ((DMA_TypeDef *)(DMA_BASE + 0x58))
#define DMA_Channel_2                           ((DMA_TypeDef *)(DMA_BASE + 0x58*2))
#define DMA_Channel_3                           ((DMA_TypeDef *)(DMA_BASE + 0x58*3))
#define DMA_Channel_4                           ((DMA_TypeDef *)(DMA_BASE + 0x58*4))
#define DMA_Channel_5                           ((DMA_TypeDef *)(DMA_BASE + 0x58*5))
#define DMA_Channel_6                           ((DMA_TypeDef *)(DMA_BASE + 0x58*6))
#define DMA_Channel_7                           ((DMA_TypeDef *)(DMA_BASE + 0x58*7))

#define GPIO                                    ((GPIO_MODULE_TypeDef *)GPIO_BASE)
#define GPIOA                                   ((GPIO_TypeDef *)GPIO_BASE)
#define GPIOB                                   ((GPIO_TypeDef *)(GPIO_BASE + 0x0010))
#define GPIOC                                   ((GPIO_TypeDef *)(GPIO_BASE + 0x0020))
#define GPIOD                                   ((GPIO_TypeDef *)(GPIO_BASE + 0x0030))
#define GPIOE                                   ((GPIO_TypeDef *)(GPIO_BASE + 0x0040))
#define GPIOF                                   ((GPIO_TypeDef *)(GPIO_BASE + 0x0050))
#define GPIOG                                   ((GPIO_TypeDef *)(GPIO_BASE + 0x0060))
#define GPIOH                                   ((GPIO_TypeDef *)(GPIO_BASE + 0x0070))
#define GPIO_GROUP                              ((GPIO_TypeDef *)GPIO_BASE)
#define GPIO_ALT_GROUP                          ((__IO uint32_t *)(GPIO_BASE + 0x180))
#define GPIO_WKEN_TYPE_EN                       ((__IO uint32_t *)(GPIO_BASE + 0x220))
#define GPIO_WKEN_P0_EN                         ((__IO uint32_t *)(GPIO_BASE + 0x224))
#define GPIO_WKEN_P1_EN                         ((__IO uint32_t *)(GPIO_BASE + 0x228))
#define GPIO_WKEN_P2_EN                         ((__IO uint32_t *)(GPIO_BASE + 0x22C))
#define GPIO_WKEN_P3_EN                         ((__IO uint32_t *)(GPIO_BASE + 0x230))

#define WDT                                     ((WDT_TypeDef *)WDG_BASE)
#define SSC                                     ((SSC_TypeDef *)SSC_BASE)
#define TST                                     ((MH_SMCU_TST_TypeDef *)TST_BASE)

#define DCMI                                    ((DCMI_TypeDef *) DCMI_BASE)
#define IMG_COP                                 ((IMG_COP_TypeDef *) IMG_COP_BASE)
#define SDRAM                                   ((SDRAM_TypeDef *)SDRAM_BASE)
#define BPU                                     ((BPU_MODULE_TypeDef *)BPU_BASE)
#define BPK                                     ((BPK_TypeDef *)BPU_BASE)
#define RTC                                     ((RTC_TypeDef *)(BPU_BASE + 0xA0))
#define SENSOR                                  ((SEN_TypeDef *)(BPU_BASE + 0xBC))
#define SEN_FLAG                                ((FLAG_TypeDef*)(BPU_BASE + 0x164))
#define DCMI_MST                                                                ((DCMI_MST_TypeDef *) DCMI_MST_BASE)


#define GPIO0_PD_IODR                ((__IO uint32_t *) (GPIO_BASE + 0x0030))


/** @addtogroup Exported_constants
  * @{
  */

/** @addtogroup Peripheral_Registers_Bits_Definition
* @{
*/

/******************************************************************************/
/*                         Peripheral Registers_Bits_Definition               */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                             System Control Unit                            */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for FREQ_SEL register  *******************/
#define SYSCTRL_FREQ_SEL_XTAL_Pos                   (16)
#define SYSCTRL_FREQ_SEL_XTAL_Mask                  (0x1F << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_XTAL_108Mhz                (0x08 << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_XTAL_120Mhz                (0x09 << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_XTAL_132Mhz                (0x0a << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_XTAL_144Mhz                (0x0b << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_XTAL_156Mhz                (0x0c << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_XTAL_168Mhz                (0x0d << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_XTAL_180Mhz                (0x0e << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_XTAL_192Mhz                (0x0f << SYSCTRL_FREQ_SEL_XTAL_Pos)
#define SYSCTRL_FREQ_SEL_XTAL_204Mhz                (0x10 << SYSCTRL_FREQ_SEL_XTAL_Pos)

#define SYSCTRL_FREQ_SEL_CLOCK_SOURCE_Pos           (12)
#define SYSCTRL_FREQ_SEL_CLOCK_SOURCE_Mask          (0x01 << SYSCTRL_FREQ_SEL_CLOCK_SOURCE_Pos)
#define SYSCTRL_FREQ_SEL_CLOCK_SOURCE_EXT           (0x00 << SYSCTRL_FREQ_SEL_CLOCK_SOURCE_Pos)
#define SYSCTRL_FREQ_SEL_CLOCK_SOURCE_INC           (0x01 << SYSCTRL_FREQ_SEL_CLOCK_SOURCE_Pos)

#define SYSCTRL_FREQ_SEL_PLL_DIV_Pos                (8)
#define SYSCTRL_FREQ_SEL_PLL_DIV_Mask               (0x03 << SYSCTRL_FREQ_SEL_PLL_DIV_Pos)
#define SYSCTRL_FREQ_SEL_PLL_DIV_1_0                (0x00 << SYSCTRL_FREQ_SEL_PLL_DIV_Pos)
#define SYSCTRL_FREQ_SEL_PLL_DIV_1_2                (0x01 << SYSCTRL_FREQ_SEL_PLL_DIV_Pos)
#define SYSCTRL_FREQ_SEL_PLL_DIV_1_4                (0x02 << SYSCTRL_FREQ_SEL_PLL_DIV_Pos)

#define SYSCTRL_FREQ_SEL_HCLK_DIV_Pos               (4)
#define SYSCTRL_FREQ_SEL_HCLK_DIV_Mask              (0x01 << SYSCTRL_FREQ_SEL_HCLK_DIV_Pos)
#define SYSCTRL_FREQ_SEL_HCLK_DIV_1_0               (0x00 << SYSCTRL_FREQ_SEL_HCLK_DIV_Pos)
#define SYSCTRL_FREQ_SEL_HCLK_DIV_1_2               (0x01 << SYSCTRL_FREQ_SEL_HCLK_DIV_Pos)

#define SYSCTRL_FREQ_SEL_PCLK_DIV_Pos               (0)
#define SYSCTRL_FREQ_SEL_PCLK_DIV_Mask              (0x01 << SYSCTRL_FREQ_SEL_PCLK_DIV_Pos)
#define SYSCTRL_FREQ_SEL_PCLK_DIV_1_2               (0x00 << SYSCTRL_FREQ_SEL_PCLK_DIV_Pos)

/*******************  Bit definition for CG_CTRL2 register  *******************/
#define SYSCTRL_AHBPeriph_DMA                       ((uint32_t)0x20000000)
#define SYSCTRL_AHBPeriph_USB                       ((uint32_t)0x10000000)
#define SYSCTRL_AHBPeriph_QR                        ((uint32_t)0x00000020)
#define SYSCTLR_AHBPeriph_MSRFC                     ((uint32_t)0x00000010)
#define SYSCTRL_AHBPeriph_OTP                       ((uint32_t)0x00000008)
#define SYSCTRL_AHBPeriph_GPU                       ((uint32_t)0x00000004)
#define SYSCTRL_AHBPeriph_LCD                       ((uint32_t)0x00000002)
#define SYSCTRL_AHBPeriph_CRYPT                     ((uint32_t)0x00000001)
#define SYSCTRL_AHBPeriph_ALL                       ((uint32_t)0x3000003F)
#define IS_SYSCTRL_AHB_PERIPH(PERIPH)               ((((PERIPH) & ~SYSCTRL_AHBPeriph_ALL) == 0x00) && ((PERIPH) != 0x00))

/*******************  Bit definition for CG_CTRL1 register  *******************/
#define SYSCTRL_APBPeriph_TRNG                      ((uint32_t)0x80000000)
#define SYSCTRL_APBPeriph_ADC                       ((uint32_t)0x40000000)
#define SYSCTRL_APBPeriph_CRC                       ((uint32_t)0x20000000)
#define SYSCTRL_APBPeriph_SDIOM                     ((uint32_t)0x10000000)
#define SYSCTRL_APBPeriph_KBD                       ((uint32_t)0x08000000)
#define SYSCTRL_APBPeriph_BPU                       ((uint32_t)0x04000000)
#define SYSCTRL_APBPeriph_SDRAM                     ((uint32_t)0x01000000)
#define SYSCTRL_APBPeriph_DCMIS                     ((uint32_t)0x00800000)
#define SYSCTRL_APBPeriph_DCMI_MST                  ((uint32_t)0x02000000)   //need modify 
#define SYSCTRL_APBPeriph_CSI2                      ((uint32_t)0x00400000)
#define SYSCTRL_APBPeriph_TIMM0                     ((uint32_t)0x00200000)
#define SYSCTRL_APBPeriph_GPIO                      ((uint32_t)0x00100000)
#define SYSCTRL_APBPeriph_I2C0                      ((uint32_t)0x00040000)
#define SYSCTRL_APBPeriph_SCI2                      ((uint32_t)0x00010000)
#define SYSCTRL_APBPeriph_SCI1                      ((uint32_t)0x00008000)
#define SYSCTRL_APBPeriph_SCI0                      ((uint32_t)0x00004000)
#define SYSCTRL_APBPeriph_SPI4                      ((uint32_t)0x00001000)
#define SYSCTRL_APBPeriph_SPI3                      ((uint32_t)0x00000800)
#define SYSCTRL_APBPeriph_SPI2                      ((uint32_t)0x00000400)
#define SYSCTRL_APBPeriph_SPI1                      ((uint32_t)0x00000200)
#define SYSCTRL_APBPeriph_SPI0                      ((uint32_t)0x00000100)
#define SYSCTRL_APBPeriph_UART3                     ((uint32_t)0x00000008)
#define SYSCTRL_APBPeriph_UART2                     ((uint32_t)0x00000004)
#define SYSCTRL_APBPeriph_UART1                     ((uint32_t)0x00000002)
#define SYSCTRL_APBPeriph_UART0                     ((uint32_t)0x00000001)
#define SYSCTRL_APBPeriph_ALL                       ((uint32_t)0xFFF5DF0F)
#define IS_SYSCTRL_APB_PERIPH(PERIPH)               ((((PERIPH) & ~SYSCTRL_APBPeriph_ALL) == 0x00) && ((PERIPH) != 0x00))

/*******************  Bit definition for SOFT_RST2 register  *******************/
#define SYSCTRL_GLB_RESET                           ((uint32_t)0x80000000)
#define SYSCTRL_CM3_RESET                           ((uint32_t)0x40000000)
#define SYSCTRL_DMA_RESET                           ((uint32_t)0x20000000)
#define SYSCTRL_USB_RESET                           ((uint32_t)0x10000000)
#define SYSCTRL_PRAMC_RESET                         ((uint32_t)0x02000000)
#define SYSCTRL_QR_RESET                            ((uint32_t)0x00000020)
#define SYSCTRL_MSRFC_RESET                         ((uint32_t)0x00000010)
#define SYSCTRL_OTP_RESET                           ((uint32_t)0x00000008)
#define SYSCTRL_GPU_RESET                           ((uint32_t)0x00000004)
#define SYSCTRL_LCD_RESET                           ((uint32_t)0x00000002)
#define SYSCTRL_CRYPT_RESET                         ((uint32_t)0x00000001)
#define SYSCTRL_AHBPeriph_RESET_ALL                 ((uint32_t)0xF200003F)
#define IS_SYSCTRL_AHB_PERIPH_RESET(PERIPH)         ((((PERIPH) & ~SYSCTRL_AHBPeriph_RESET_ALL) == 0x00) && ((PERIPH) != 0x00))

/*******************  Bit definition for PHER_CTRL register  *******************/
#define SYSCTRL_PHER_CTRL_SPI0_SLV_EN               ((uint32_t)0x01000000)      /* 0:MASTER  1:SLAVE */
#define SYSCTRL_PHER_CTRL_SCI2_VCCEN_INV            ((uint32_t)0x00400000)
#define SYSCTRL_PHER_CTRL_SCI1_VCCEN_INV            ((uint32_t)0x00200000)
#define SYSCTRL_PHER_CTRL_SCI0_VCCEN_INV            ((uint32_t)0x00100000)
#define SYSCTRL_PHER_CTRL_SCI2_CDET_INV             ((uint32_t)0x00040000)
#define SYSCTRL_PHER_CTRL_SCI1_CDET_INV             ((uint32_t)0x00020000)
#define SYSCTRL_PHER_CTRL_SCI0_CDET_INV             ((uint32_t)0x00010000)

/*******************  Bit definition for DMA_CHAN REGISTER  ********************/
#define SYSCTRL_PHER_CTRL_DMA_CH0_IF_Pos                    (0)
#define SYSCTRL_PHER_CTRL_DMA_CH0_IF_Mask                   (0x1FU<<SYSCTRL_PHER_CTRL_DMA_CH0_IF_Pos)

#define SYSCTRL_PHER_CTRL_DMA_CH1_IF_Pos                    (8)
#define SYSCTRL_PHER_CTRL_DMA_CH1_IF_Mask                   (0x1FU<<SYSCTRL_PHER_CTRL_DMA_CH1_IF_Pos)

#define SYSCTRL_PHER_CTRL_DMA_CH2_IF_Pos                    (16)
#define SYSCTRL_PHER_CTRL_DMA_CH2_IF_Mask                   (0x1FU<<SYSCTRL_PHER_CTRL_DMA_CH2_IF_Pos)

#define SYSCTRL_PHER_CTRL_DMA_CH3_IF_Pos                    (24)
#define SYSCTRL_PHER_CTRL_DMA_CH3_IF_Mask                   (0x1FU<<SYSCTRL_PHER_CTRL_DMA_CH3_IF_Pos)

#define SYSCTRL_PHER_CTRL_DMA_CH4_IF_Pos                    (0)
#define SYSCTRL_PHER_CTRL_DMA_CH4_IF_Mask                   (0x1FU<<SYSCTRL_PHER_CTRL_DMA_CH4_IF_Pos)

#define SYSCTRL_PHER_CTRL_DMA_CH5_IF_Pos                    (8)
#define SYSCTRL_PHER_CTRL_DMA_CH5_IF_Mask                   (0x1FU<<SYSCTRL_PHER_CTRL_DMA_CH5_IF_Pos)

#define SYSCTRL_PHER_CTRL_DMA_CH6_IF_Pos                    (16)
#define SYSCTRL_PHER_CTRL_DMA_CH6_IF_Mask                   (0x1FU<<SYSCTRL_PHER_CTRL_DMA_CH6_IF_Pos)

#define SYSCTRL_PHER_CTRL_DMA_CH7_IF_Pos                    (24)
#define SYSCTRL_PHER_CTRL_DMA_CH7_IF_Mask                   (0x1FU<<SYSCTRL_PHER_CTRL_DMA_CH7_IF_Pos)


#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_DCMI_TX                (0x00)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_LCD                    (0x01)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART0_TX               (0x02)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART0_RX               (0x03)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART1_TX               (0x04)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART1_RX               (0x05)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_DAC                    (0x06)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI0_TX                (0x0A)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI0_RX                (0x0B)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI1_TX                (0x0C)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI1_RX                (0x0D)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI2_TX                (0x0E)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI2_RX                (0x0F)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI3_TX                (0x10)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI3_RX                (0x11)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI4_TX                (0x12)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_SPI4_RX                (0x13)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART2_TX               (0x14)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART2_RX               (0x15)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART3_TX               (0x16)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_UART3_RX               (0x17)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_I2C_TX                 (0x18)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_I2C_RX                 (0x19)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_QSPI_TX                (0x1A)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_SDIOM_TX               (0x1E)
#define SYSCTRL_PHER_CTRL_DMA_CHx_IF_SDIOM_RX               (0x1F)

/******************************************************************************/
/*                                                                            */
/*                Universal Asynchronous Receiver Transmitter                 */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for UART_RBR register  *******************/
#define UART_RBR_RBR                         ((uint32_t)0x01FF)            /*!< Data value */

/*******************  Bit definition for UART_THR register  *******************/
#define UART_THR_THR                         ((uint32_t)0x01FF)            /*!< Data value */

/*******************  Bit definition for UART_DLH register  *******************/
#define UART_DLH_DLH                         ((uint32_t)0x0FF)

/*******************  Bit definition for UART_DLL register  *******************/
#define UART_DLL_DLL                         ((uint32_t)0x0FF)

/*******************  Bit definition for UART_IER register  *******************/
#define UART_IER_ERBFI                          ((uint32_t)0x0001)
#define UART_IER_ETBEI                          ((uint32_t)0x0002)
#define UART_IER_ELSI                           ((uint32_t)0x0004)
#define UART_IER_EDSSI                          ((uint32_t)0x0008)
#define UART_IER_PTIME                          ((uint32_t)0x0080)

/*******************  Bit definition for UART_IIR register  *******************/
#define UART_IIR_IID                            ((uint32_t)0x0007)
#define UART_IIR_IID_0                          ((uint32_t)0x0001)
#define UART_IIR_IID_1                          ((uint32_t)0x0002)
#define UART_IIR_IID_2                          ((uint32_t)0x0004)
#define UART_IIR_IID_3                          ((uint32_t)0x0008)
#define UART_IIR_FIFOSE                         ((uint32_t)0x0060)
#define UART_IIR_FIFOSE_0                       ((uint32_t)0x0020)
#define UART_IIR_FIFOSE_1                       ((uint32_t)0x0040)

/*******************  Bit definition for UART_FCR register  *******************/
#define UART_FCR_FIFOE                          ((uint32_t)0x0001)
#define UART_FCR_RFIFOR                         ((uint32_t)0x0002)
#define UART_FCR_XFIFOR                         ((uint32_t)0x0004)
#define UART_FCR_DMAM                           ((uint32_t)0x0008)
#define UART_FCR_TET                            ((uint32_t)0x0030)
#define UART_FCR_TET_0                          ((uint32_t)0x0010)
#define UART_FCR_TET_1                          ((uint32_t)0x0020)
#define UART_FCR_RCVER                          ((uint32_t)0x00C0)
#define UART_FCR_RCVER_0                        ((uint32_t)0x0040)
#define UART_FCR_RCVER_1                        ((uint32_t)0x0080)

/*******************  Bit definition for UART_LCR register  *******************/
#define UART_LCR_DLS                            ((uint32_t)0x0003)
#define UART_LCR_DLS_0                          ((uint32_t)0x0001)
#define UART_LCR_DLS_1                          ((uint32_t)0x0002)
#define UART_LCR_STOP                           ((uint32_t)0x0004)
#define UART_LCR_PEN                            ((uint32_t)0x0008)
#define UART_LCR_EPS                            ((uint32_t)0x0010)
#define UART_LCR_SP                             ((uint32_t)0x0020)
#define UART_LCR_BC                             ((uint32_t)0x0040)
#define UART_LCR_DLAB                           ((uint32_t)0x0080)

/*******************  Bit definition for UART_MCR register  *******************/
#define UART_MCR_DTR                            ((uint32_t)0x0001)
#define UART_MCR_RTS                            ((uint32_t)0x0002)
#define UART_MCR_OUT1                           ((uint32_t)0x0004)
#define UART_MCR_OUT2                           ((uint32_t)0x0008)
#define UART_MCR_LB                             ((uint32_t)0x0010)
#define UART_MCR_AFCE                           ((uint32_t)0x0020)
#define UART_MCR_SIRE                           ((uint32_t)0x0040)

/*******************  Bit definition for UART_LSR register  *******************/
#define UART_LSR_DR                             ((uint32_t)0x0001)
#define UART_LSR_OE                             ((uint32_t)0x0002)
#define UART_LSR_PE                             ((uint32_t)0x0004)
#define UART_LSR_FE                             ((uint32_t)0x0008)
#define UART_LSR_BI                             ((uint32_t)0x0010)
#define UART_LSR_THRE                           ((uint32_t)0x0020)
#define UART_LSR_TEMT                           ((uint32_t)0x0040)
#define UART_LSR_PFE                            ((uint32_t)0x0080)

/*******************  Bit definition for UART_MSR register  *******************/
#define UART_MSR_DCTS                               ((uint32_t)0x0001)
#define UART_MSR_DDSR                               ((uint32_t)0x0002)
#define UART_MSR_TERI                               ((uint32_t)0x0004)
#define UART_MSR_DDCD                               ((uint32_t)0x0008)
#define UART_MSR_CTS                                ((uint32_t)0x0010)
#define UART_MSR_DSR                                ((uint32_t)0x0020)
#define UART_MSR_RI                                 ((uint32_t)0x0040)
#define UART_MSR_DCD                                ((uint32_t)0x0080)

/*******************  Bit definition for UART_SRBR register  *******************/
#define UART_SRBR_SRBR                          ((uint32_t)0x01FF)            /*!< Data value */

/*******************  Bit definition for UART_STHR register  *******************/
#define UART_STHR_STHR                          ((uint32_t)0x01FF)            /*!< Data value */

/*******************  Bit definition for UART_FAR register  *******************/
#define UART_FAR_FAR                            ((uint32_t)0x0001)

/*******************  Bit definition for UART_TFR register  *******************/
#define UART_TFR_TFR                            ((uint32_t)0x00FF)

/*******************  Bit definition for UART_RFW register  *******************/
#define UART_RFW_RFWD                           ((uint32_t)0x00FF)
#define UART_RFW_RFPE                           ((uint32_t)0x0100)
#define UART_RFW_RFFE                           ((uint32_t)0x0200)

/*******************  Bit definition for UART_USR register  *******************/
#define UART_USR_BUSY                               ((uint32_t)0x0001)
#define UART_USR_TFNF                               ((uint32_t)0x0002)
#define UART_USR_TFE                                ((uint32_t)0x0004)
#define UART_USR_RFNE                               ((uint32_t)0x0008)
#define UART_USR_RFF                                ((uint32_t)0x0010)

/*******************  Bit definition for UART_TFL register  *******************/
#define UART_TFL_TFL                                ((uint32_t)0x000F)

/*******************  Bit definition for UART_RFL register  *******************/
#define UART_RFL_RFL                                ((uint32_t)0x000F)

/*******************  Bit definition for UART_SRR register  *******************/
#define UART_SRR_UR                                 ((uint32_t)0x0001)
#define UART_SRR_RFR                                ((uint32_t)0x0002)
#define UART_SRR_XFR                                ((uint32_t)0x0004)

/*******************  Bit definition for UART_SRR register  *******************/
#define UART_SRR_UR                                 ((uint32_t)0x0001)

/*******************  Bit definition for UART_SRTS register  *******************/
#define UART_SRTS_SRTS                              ((uint32_t)0x0001)

/*******************  Bit definition for UART_SBCR register  *******************/
#define UART_SBCR_SBCR                              ((uint32_t)0x0001)

/*******************  Bit definition for UART_SDMAM register  *******************/
#define UART_SDMAM_SDMAM                            ((uint32_t)0x0001)

/*******************  Bit definition for UART_SFE register  *******************/
#define UART_SFE_SFE                                ((uint32_t)0x0001)

/*******************  Bit definition for UART_SRT register  *******************/
#define UART_SRT_SRT                                ((uint32_t)0x0003)
#define UART_SRT_SRT_0                              ((uint32_t)0x0001)
#define UART_SRT_SRT_1                              ((uint32_t)0x0002)

/*******************  Bit definition for UART_STET register  *******************/
#define UART_STET_STET                              ((uint32_t)0x0003)
#define UART_STET_STET_0                            ((uint32_t)0x0001)
#define UART_STET_STET_1                            ((uint32_t)0x0002)

/*******************  Bit definition for UART_HTX register  *******************/
#define UART_HTX_HTX                                ((uint32_t)0x0001)

/*******************  Bit definition for UART_DMASA register  *******************/
#define UART_DMASA_DMASA                            ((uint32_t)0x0001)

/******************************************************************************/
/*                                                                            */
/*                General Purpose and Alternate Function I/O                  */
/*                                                                            */
/******************************************************************************/

/*!<******************  Bit definition for GPIO_IODR register  *******************/
#define GPIO_IODR_ODR0                        ((uint16_t)0x00000001)            /*!< Port output data, bit 0 */
#define GPIO_IODR_ODR1                        ((uint16_t)0x00000002)            /*!< Port output data, bit 1 */
#define GPIO_IODR_ODR2                        ((uint16_t)0x00000004)            /*!< Port output data, bit 2 */
#define GPIO_IODR_ODR3                        ((uint16_t)0x00000008)            /*!< Port output data, bit 3 */
#define GPIO_IODR_ODR4                        ((uint16_t)0x00000010)            /*!< Port output data, bit 4 */
#define GPIO_IODR_ODR5                        ((uint16_t)0x00000020)            /*!< Port output data, bit 5 */
#define GPIO_IODR_ODR6                        ((uint16_t)0x00000040)            /*!< Port output data, bit 6 */
#define GPIO_IODR_ODR7                        ((uint16_t)0x00000080)            /*!< Port output data, bit 7 */
#define GPIO_IODR_ODR8                        ((uint16_t)0x00000100)            /*!< Port output data, bit 8 */
#define GPIO_IODR_ODR9                        ((uint16_t)0x00000200)            /*!< Port output data, bit 9 */
#define GPIO_IODR_ODR10                       ((uint16_t)0x00000400)            /*!< Port output data, bit 10 */
#define GPIO_IODR_ODR11                       ((uint16_t)0x00000800)            /*!< Port output data, bit 11 */
#define GPIO_IODR_ODR12                       ((uint16_t)0x00001000)            /*!< Port output data, bit 12 */
#define GPIO_IODR_ODR13                       ((uint16_t)0x00002000)            /*!< Port output data, bit 13 */
#define GPIO_IODR_ODR14                       ((uint16_t)0x00004000)            /*!< Port output data, bit 14 */
#define GPIO_IODR_ODR15                       ((uint16_t)0x00008000)            /*!< Port output data, bit 15 */

#define GPIO_IODR_IDR0                        ((uint16_t)0x00010000)            /*!< Port input data, bit 0 */
#define GPIO_IODR_IDR1                        ((uint16_t)0x00020000)            /*!< Port input data, bit 1 */
#define GPIO_IODR_IDR2                        ((uint16_t)0x00040000)            /*!< Port input data, bit 2 */
#define GPIO_IODR_IDR3                        ((uint16_t)0x00080000)            /*!< Port input data, bit 3 */
#define GPIO_IODR_IDR4                        ((uint16_t)0x00100000)            /*!< Port input data, bit 4 */
#define GPIO_IODR_IDR5                        ((uint16_t)0x00200000)            /*!< Port input data, bit 5 */
#define GPIO_IODR_IDR6                        ((uint16_t)0x00400000)            /*!< Port input data, bit 6 */
#define GPIO_IODR_IDR7                        ((uint16_t)0x00800000)            /*!< Port input data, bit 7 */
#define GPIO_IODR_IDR8                        ((uint16_t)0x01000000)            /*!< Port input data, bit 8 */
#define GPIO_IODR_IDR9                        ((uint16_t)0x02000000)            /*!< Port input data, bit 9 */
#define GPIO_IODR_IDR10                       ((uint16_t)0x04000000)            /*!< Port input data, bit 10 */
#define GPIO_IODR_IDR11                       ((uint16_t)0x08000000)            /*!< Port input data, bit 11 */
#define GPIO_IODR_IDR12                       ((uint16_t)0x10000000)            /*!< Port input data, bit 12 */
#define GPIO_IODR_IDR13                       ((uint16_t)0x20000000)            /*!< Port input data, bit 13 */
#define GPIO_IODR_IDR14                       ((uint16_t)0x40000000)            /*!< Port input data, bit 14 */
#define GPIO_IODR_IDR15                       ((uint16_t)0x80000000)            /*!< Port input data, bit 15 */

/******************  Bit definition for GPIO_BSRR register  *******************/
#define GPIO_BSRR_BS0                        ((uint32_t)0x00000001)        /*!< Port x Set bit 0 */
#define GPIO_BSRR_BS1                        ((uint32_t)0x00000002)        /*!< Port x Set bit 1 */
#define GPIO_BSRR_BS2                        ((uint32_t)0x00000004)        /*!< Port x Set bit 2 */
#define GPIO_BSRR_BS3                        ((uint32_t)0x00000008)        /*!< Port x Set bit 3 */
#define GPIO_BSRR_BS4                        ((uint32_t)0x00000010)        /*!< Port x Set bit 4 */
#define GPIO_BSRR_BS5                        ((uint32_t)0x00000020)        /*!< Port x Set bit 5 */
#define GPIO_BSRR_BS6                        ((uint32_t)0x00000040)        /*!< Port x Set bit 6 */
#define GPIO_BSRR_BS7                        ((uint32_t)0x00000080)        /*!< Port x Set bit 7 */
#define GPIO_BSRR_BS8                        ((uint32_t)0x00000100)        /*!< Port x Set bit 8 */
#define GPIO_BSRR_BS9                        ((uint32_t)0x00000200)        /*!< Port x Set bit 9 */
#define GPIO_BSRR_BS10                       ((uint32_t)0x00000400)        /*!< Port x Set bit 10 */
#define GPIO_BSRR_BS11                       ((uint32_t)0x00000800)        /*!< Port x Set bit 11 */
#define GPIO_BSRR_BS12                       ((uint32_t)0x00001000)        /*!< Port x Set bit 12 */
#define GPIO_BSRR_BS13                       ((uint32_t)0x00002000)        /*!< Port x Set bit 13 */
#define GPIO_BSRR_BS14                       ((uint32_t)0x00004000)        /*!< Port x Set bit 14 */
#define GPIO_BSRR_BS15                       ((uint32_t)0x00008000)        /*!< Port x Set bit 15 */

#define GPIO_BSRR_BR0                        ((uint32_t)0x00010000)        /*!< Port x Reset bit 0 */
#define GPIO_BSRR_BR1                        ((uint32_t)0x00020000)        /*!< Port x Reset bit 1 */
#define GPIO_BSRR_BR2                        ((uint32_t)0x00040000)        /*!< Port x Reset bit 2 */
#define GPIO_BSRR_BR3                        ((uint32_t)0x00080000)        /*!< Port x Reset bit 3 */
#define GPIO_BSRR_BR4                        ((uint32_t)0x00100000)        /*!< Port x Reset bit 4 */
#define GPIO_BSRR_BR5                        ((uint32_t)0x00200000)        /*!< Port x Reset bit 5 */
#define GPIO_BSRR_BR6                        ((uint32_t)0x00400000)        /*!< Port x Reset bit 6 */
#define GPIO_BSRR_BR7                        ((uint32_t)0x00800000)        /*!< Port x Reset bit 7 */
#define GPIO_BSRR_BR8                        ((uint32_t)0x01000000)        /*!< Port x Reset bit 8 */
#define GPIO_BSRR_BR9                        ((uint32_t)0x02000000)        /*!< Port x Reset bit 9 */
#define GPIO_BSRR_BR10                       ((uint32_t)0x04000000)        /*!< Port x Reset bit 10 */
#define GPIO_BSRR_BR11                       ((uint32_t)0x08000000)        /*!< Port x Reset bit 11 */
#define GPIO_BSRR_BR12                       ((uint32_t)0x10000000)        /*!< Port x Reset bit 12 */
#define GPIO_BSRR_BR13                       ((uint32_t)0x20000000)        /*!< Port x Reset bit 13 */
#define GPIO_BSRR_BR14                       ((uint32_t)0x40000000)        /*!< Port x Reset bit 14 */
#define GPIO_BSRR_BR15                       ((uint32_t)0x80000000)        /*!< Port x Reset bit 15 */

/******************  Bit definition for GPIO_OEN register  *******************/
#define GPIO_OEN_0                          ((uint32_t)0x00000001)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_1                          ((uint32_t)0x00000002)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_2                          ((uint32_t)0x00000004)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_3                          ((uint32_t)0x00000008)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_4                          ((uint32_t)0x00000010)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_5                          ((uint32_t)0x00000020)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_6                          ((uint32_t)0x00000040)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_7                          ((uint32_t)0x00000080)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_8                          ((uint32_t)0x00000100)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_9                          ((uint32_t)0x00000200)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_10                         ((uint32_t)0x00000400)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_11                         ((uint32_t)0x00000800)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_12                         ((uint32_t)0x00001000)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_13                         ((uint32_t)0x00002000)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_14                         ((uint32_t)0x00004000)        /*!< 1:Port Input 0:Port Output */
#define GPIO_OEN_15                         ((uint32_t)0x00008000)        /*!< 1:Port Input 0:Port Output */

/******************  Bit definition for GPIO_PUE register  *******************/
#define GPIO_PUE_0                          ((uint32_t)0x00000001)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_1                          ((uint32_t)0x00000002)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_2                          ((uint32_t)0x00000004)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_3                          ((uint32_t)0x00000008)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_4                          ((uint32_t)0x00000010)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_5                          ((uint32_t)0x00000020)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_6                          ((uint32_t)0x00000040)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_7                          ((uint32_t)0x00000080)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_8                          ((uint32_t)0x00000100)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_9                          ((uint32_t)0x00000200)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_10                         ((uint32_t)0x00000400)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_11                         ((uint32_t)0x00000800)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_12                         ((uint32_t)0x00001000)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_13                         ((uint32_t)0x00002000)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_14                         ((uint32_t)0x00004000)        /*!< 1:PullUp Enable 0:PullUp Disable */
#define GPIO_PUE_15                         ((uint32_t)0x00008000)        /*!< 1:PullUp Enable 0:PullUp Disable */

/******************  Bit definition for GPIO_ALT register  *******************/
#define GPIO_ALT_PORT0                      (BIT0 | BIT1)
#define GPIO_ALT_PORT0_0                    (BIT0)
#define GPIO_ALT_PORT0_1                    (BIT1)
#define GPIO_ALT_PORT1                      (BIT2 | BIT3)
#define GPIO_ALT_PORT1_0                    (BIT2)
#define GPIO_ALT_PORT1_1                    (BIT3)
#define GPIO_ALT_PORT2                      (BIT4 | BIT5)
#define GPIO_ALT_PORT2_0                    (BIT4)
#define GPIO_ALT_PORT2_1                    (BIT5)
#define GPIO_ALT_PORT3                      (BIT6 | BIT7)
#define GPIO_ALT_PORT3_0                    (BIT6)
#define GPIO_ALT_PORT3_1                    (BIT7)
#define GPIO_ALT_PORT4                      (BIT8 | BIT9)
#define GPIO_ALT_PORT4_0                    (BIT8)
#define GPIO_ALT_PORT4_1                    (BIT9)
#define GPIO_ALT_PORT5                      (BIT10 | BIT11)
#define GPIO_ALT_PORT5_0                    (BIT10)
#define GPIO_ALT_PORT5_1                    (BIT11)
#define GPIO_ALT_PORT6                      (BIT12 | BIT13)
#define GPIO_ALT_PORT6_0                    (BIT12)
#define GPIO_ALT_PORT6_1                    (BIT13)
#define GPIO_ALT_PORT7                      (BIT14 | BIT15)
#define GPIO_ALT_PORT7_0                    (BIT14)
#define GPIO_ALT_PORT7_1                    (BIT15)
#define GPIO_ALT_PORT8                      (BIT16 | BIT17)
#define GPIO_ALT_PORT8_0                    (BIT16)
#define GPIO_ALT_PORT8_1                    (BIT17)
#define GPIO_ALT_PORT9                      (BIT18 | BIT19)
#define GPIO_ALT_PORT9_0                    (BIT18)
#define GPIO_ALT_PORT9_1                    (BIT19)
#define GPIO_ALT_PORT10                     (BIT20 | BIT21)
#define GPIO_ALT_PORT10_0                   (BIT20)
#define GPIO_ALT_PORT10_1                   (BIT21)
#define GPIO_ALT_PORT11                     (BIT22 | BIT23)
#define GPIO_ALT_PORT11_0                   (BIT22)
#define GPIO_ALT_PORT11_1                   (BIT23)
#define GPIO_ALT_PORT12                     (BIT24 | BIT25)
#define GPIO_ALT_PORT12_0                   (BIT24)
#define GPIO_ALT_PORT12_1                   (BIT25)
#define GPIO_ALT_PORT13                     (BIT26 | BIT27)
#define GPIO_ALT_PORT13_0                   (BIT26)
#define GPIO_ALT_PORT13_1                   (BIT27)
#define GPIO_ALT_PORT14                     (BIT28 | BIT29)
#define GPIO_ALT_PORT14_0                   (BIT28)
#define GPIO_ALT_PORT14_1                   (BIT29)
#define GPIO_ALT_PORT15                     (BIT30 | BIT31)
#define GPIO_ALT_PORT15_0                   (BIT30)
#define GPIO_ALT_PORT15_1                   (BIT31)


#define DEEP_SLEEP_WKUP_EN_SENSOR           (BIT14)
#define DEEP_SLEEP_WKUP_EN_MSR              (BIT13)
#define DEEP_SLEEP_WKUP_EN_RTC              (BIT12)
#define DEEP_SLEEP_WKUP_EN_KBD              (BIT11)
#define DEEP_SLEEP_WKUP_EN_GPIO             (BIT0)


/******************************************************************************/
/*                                                                            */
/*                        Serial Peripheral Interface                         */
/*                                                                            */
/******************************************************************************/

/*****************  Bit definition for SPI_CTRLR0 register  *******************/
#define SPI_CTRLR0_DFS                      ((uint32_t)0x000F)
#define SPI_CTRLR0_DFS_0                    ((uint32_t)0x0001)
#define SPI_CTRLR0_DFS_1                    ((uint32_t)0x0002)
#define SPI_CTRLR0_DFS_2                    ((uint32_t)0x0004)
#define SPI_CTRLR0_DFS_3                    ((uint32_t)0x0008)
#define SPI_CTRLR0_FRF                      ((uint32_t)0x0030)
#define SPI_CTRLR0_FRF_0                    ((uint32_t)0x0010)
#define SPI_CTRLR0_FRF_1                    ((uint32_t)0x0020)
#define SPI_CTRLR0_SCPH                     ((uint32_t)0x0040)
#define SPI_CTRLR0_SCPOL                    ((uint32_t)0x0080)
#define SPI_CTRLR0_TMOD                     ((uint32_t)0x0300)
#define SPI_CTRLR0_TMOD_0                   ((uint32_t)0x0100)
#define SPI_CTRLR0_TMOD_1                   ((uint32_t)0x0200)
#define SPI_CTRLR0_SLV_OE                   ((uint32_t)0x0400)
#define SPI_CTRLR0_SRL                      ((uint32_t)0x0800)
#define SPI_CTRLR0_CFS                      ((uint32_t)0xF000)

/*****************  Bit definition for SPI_CTRLR1 register  *******************/
#define SPI_CTRLR0_NDF                      ((uint32_t)0xFFFF)

/*****************  Bit definition for SPI_SSIENR register  *******************/
#define SPI_SSIENR_SSIENR                   ((uint32_t)0x0001)

/*****************  Bit definition for SPI_MWCR register  *********************/
#define SPI_MWCR_MWMOD                      ((uint32_t)0x0001)
#define SPI_MWCR_MDD                        ((uint32_t)0x0002)
#define SPI_MWCR_MHS                        ((uint32_t)0x0004)

/*****************  Bit definition for SPI_SER register  **********************/
#define SPI_SER_SER                         ((uint32_t)0x000F)
#define SPI_SER_0                           ((uint32_t)0x0001)
#define SPI_SER_1                           ((uint32_t)0x0002)
#define SPI_SER_2                           ((uint32_t)0x0004)
#define SPI_SER_3                           ((uint32_t)0x0008)

/*****************  Bit definition for SPI_BAUDR register  ********************/
#define SPI_BAUDR_BAUDR                     ((uint32_t)0xFFFF)

/*****************  Bit definition for SPI_TXFTLR register  *******************/
#define SPI_TXFTLR_TFT                      ((uint32_t)0x000F)
/*****************  Bit definition for SPI_RXFTLR register  *******************/
#define SPI_RXFTLR_RFT                      ((uint32_t)0x000F)
/*****************  Bit definition for SPI_TXFLR register  ********************/
#define SPI_TXFLR_TXTFL                     ((uint32_t)0x001F)
/*****************  Bit definition for SPI_RXFLR register  ********************/
#define SPI_RXFLR_RXTFL                     ((uint32_t)0x001F)

/*****************  Bit definition for SPI_SR register  ***********************/
#define SPI_SR_BUSY                         ((uint32_t)0x0001)
#define SPI_SR_TFNF                         ((uint32_t)0x0002)
#define SPI_SR_TFE                          ((uint32_t)0x0004)
#define SPI_SR_RFNE                         ((uint32_t)0x0008)
#define SPI_SR_RFF                          ((uint32_t)0x0010)
#define SPI_SR_TXE                          ((uint32_t)0x0020)
#define SPI_SR_DCOL                         ((uint32_t)0x0040)
/*****************  Bit definition for SPI_IMR register  **********************/
#define SPI_IMR_TXEIM                       ((uint32_t)0x0001)
#define SPI_IMR_TXOIM                       ((uint32_t)0x0002)
#define SPI_IMR_RXUIM                       ((uint32_t)0x0004)
#define SPI_IMR_RXOIM                       ((uint32_t)0x0008)
#define SPI_IMR_RXFIM                       ((uint32_t)0x0010)
#define SPI_IMR_MSTIM                       ((uint32_t)0x0020)
/*****************  Bit definition for SPI_ISR register  **********************/
#define SPI_ISR_TXEIS                       ((uint32_t)0x0001)
#define SPI_ISR_TXOIS                       ((uint32_t)0x0002)
#define SPI_ISR_RXUIS                       ((uint32_t)0x0004)
#define SPI_ISR_RXOIS                       ((uint32_t)0x0008)
#define SPI_ISR_RXFIS                       ((uint32_t)0x0010)
#define SPI_ISR_MSTIS                       ((uint32_t)0x0020)
/*****************  Bit definition for SPI_RISR register  *********************/
#define SPI_RISR_TXEIR                      ((uint32_t)0x0001)
#define SPI_RISR_TXOIR                      ((uint32_t)0x0002)
#define SPI_RISR_RXUIR                      ((uint32_t)0x0004)
#define SPI_RISR_RXOIR                      ((uint32_t)0x0008)
#define SPI_RISR_RXFIR                      ((uint32_t)0x0010)
#define SPI_RISR_MSTIR                      ((uint32_t)0x0020)
/*****************  Bit definition for SPI_TXOICR register  *******************/
#define SPI_TXOICR_TXOICR                   ((uint32_t)0x0001)
/*****************  Bit definition for SPI_RXOICR register  *******************/
#define SPI_RXOICR_RXOICR                   ((uint32_t)0x0001)
/*****************  Bit definition for SPI_RXUICR register  *******************/
#define SPI_RXUICR_RXUICR                   ((uint32_t)0x0001)
/*****************  Bit definition for SPI_MSTICR register  *******************/
#define SPI_MSTICR_MSTICR                   ((uint32_t)0x0001)

/*****************  Bit definition for SPI_DMACR register  ********************/
#define SPI_DMACR_RDMAE                     ((uint32_t)0x0001)
#define SPI_DMACR_TDMAE                     ((uint32_t)0x0002)
/*****************  Bit definition for SPI_DMATDLR register  ******************/
#define SPI_DMATDLR_DMATDLR                 ((uint32_t)0x000F)
/*****************  Bit definition for SPI_DMARDLR register  ******************/
#define SPI_DMATDLR_DMARDLR                 ((uint32_t)0x000F)
/*****************  Bit definition for SPI_DMARDLR register  ******************/
#define SPI_DMATDLR_DMARDLR                 ((uint32_t)0x000F)

/*****************  Bit definition for SPI_DR register  ***********************/
#define SPI_DR_DR                           ((uint32_t)0xFFFF)
/**************  Bit definition for SPI_RX_SAMPLE_DLY register  ***************/
#define SPI_RX_SAMPLE_DLY                   ((uint32_t)0xFFFF)


/******************************************************************************/
/*                                                                            */
/*                      Inter-integrated Circuit Interface                    */
/*                                                                            */
/******************************************************************************/

/*******************  Bit definition for IC_CON register  *********************/
#define I2C_IC_CON_MASTER_MODE                  ((uint32_t)0x0001)
#define I2C_IC_CON_SPEED                        ((uint32_t)0x0006)
#define I2C_IC_CON_SPEED_0                      ((uint32_t)0x0002)
#define I2C_IC_CON_SPEED_1                      ((uint32_t)0x0004)
#define I2C_IC_CON_10BITADDR_SLAVE              ((uint32_t)0x0008)
#define I2C_IC_CON_10BITADDR_MASTER             ((uint32_t)0x0010)
#define I2C_IC_CON_RESTART_EN                   ((uint32_t)0x0020)
#define I2C_IC_CON_SLAVE_DISABLE                ((uint32_t)0x0040)

/*******************  Bit definition for IC_TAR register  *********************/
#define I2C_IC_TAR_TAR                          ((uint32_t)0x03FF)
#define I2C_IC_TAR_GC_OR_START                  ((uint32_t)0x0400)
#define I2C_IC_TAR_SPECIAL                      ((uint32_t)0x0800)
#define I2C_IC_TAR_10BITADDR_MASTER             ((uint32_t)0x1000)

/*******************  Bit definition for IC_SAR register  *********************/
#define I2C_IC_SAR_SAR                          ((uint32_t)0x03FF)

/*******************  Bit definition for IC_HS_MADDR register  ****************/
#define I2C_IC_HS_MADDR_MAR                     ((uint32_t)0x0007)
#define I2C_IC_HS_MADDR_MAR_0                   ((uint32_t)0x0001)
#define I2C_IC_HS_MADDR_MAR_1                   ((uint32_t)0x0002)
#define I2C_IC_HS_MADDR_MAR_2                   ((uint32_t)0x0004)

/*******************  Bit definition for IC_DATA_CMD register  ****************/
#define I2C_IC_DATA_CMD_DAT                     ((uint32_t)0x00FF)
#define I2C_IC_DATA_CMD_CMD                     ((uint32_t)0x0100)
#define I2C_IC_DATA_CMD_STOP                    ((uint32_t)0x0200)
#define I2C_IC_DATA_CMD_RESTART                 ((uint32_t)0x0400)

/*******************  Bit definition for IC_SS_SCL_HCNT register  *************/
#define I2C_IC_SS_SCL_HCNT_HCNT                     ((uint32_t)0xFFFF)

/*******************  Bit definition for IC_SS_SCL_LCNT register  *************/
#define I2C_IC_SS_SCL_LCNT_LCNT                     ((uint32_t)0xFFFF)

/*******************  Bit definition for IC_FS_SCL_HCNT register  *************/
#define I2C_IC_FS_SCL_HCNT_HCNT                     ((uint32_t)0xFFFF)

/*******************  Bit definition for IC_FS_SCL_LCNT register  *************/
#define I2C_IC_FS_SCL_LCNT_LCNT                     ((uint32_t)0xFFFF)

/*******************  Bit definition for IC_HS_SCL_HCNT register  *************/
#define I2C_IC_HS_SCL_HCNT_HCNT                     ((uint32_t)0xFFFF)

/*******************  Bit definition for IC_HS_SCL_LCNT register  *************/
#define I2C_IC_HS_SCL_LCNT_LCNT                     ((uint32_t)0xFFFF)

/*******************  Bit definition for IC_INTR_STAT register  ***************/
#define I2C_IC_INTR_STAT_R_RX_UNDER                 ((uint32_t)0x0001)
#define I2C_IC_INTR_STAT_R_RX_OVER                  ((uint32_t)0x0002)
#define I2C_IC_INTR_STAT_R_RX_FULL                  ((uint32_t)0x0004)
#define I2C_IC_INTR_STAT_R_TX_OVER                  ((uint32_t)0x0008)
#define I2C_IC_INTR_STAT_R_TX_EMPTY                 ((uint32_t)0x0010)
#define I2C_IC_INTR_STAT_R_RD_REQ                   ((uint32_t)0x0020)
#define I2C_IC_INTR_STAT_R_TX_ABRT                  ((uint32_t)0x0040)
#define I2C_IC_INTR_STAT_R_RX_DONE                  ((uint32_t)0x0080)
#define I2C_IC_INTR_STAT_R_ACTIVITY                 ((uint32_t)0x0100)
#define I2C_IC_INTR_STAT_R_STOP_DET                 ((uint32_t)0x0200)
#define I2C_IC_INTR_STAT_R_START_DET                ((uint32_t)0x0400)
#define I2C_IC_INTR_STAT_R_GEN_CALL                 ((uint32_t)0x0800)

/*******************  Bit definition for IC_INTR_MASK register  ***************/
#define I2C_IC_INTR_MASK_M_RX_UNDER                 ((uint32_t)0x0001)
#define I2C_IC_INTR_MASK_M_RX_OVER                  ((uint32_t)0x0002)
#define I2C_IC_INTR_MASK_M_RX_FULL                  ((uint32_t)0x0004)
#define I2C_IC_INTR_MASK_M_TX_OVER                  ((uint32_t)0x0008)
#define I2C_IC_INTR_MASK_M_TX_EMPTY                 ((uint32_t)0x0010)
#define I2C_IC_INTR_MASK_M_RD_REQ                   ((uint32_t)0x0020)
#define I2C_IC_INTR_MASK_M_TX_ABRT                  ((uint32_t)0x0040)
#define I2C_IC_INTR_MASK_M_RX_DONE                  ((uint32_t)0x0080)
#define I2C_IC_INTR_MASK_M_ACTIVITY                 ((uint32_t)0x0100)
#define I2C_IC_INTR_MASK_M_STOP_DET                 ((uint32_t)0x0200)
#define I2C_IC_INTR_MASK_M_START_DET                ((uint32_t)0x0400)
#define I2C_IC_INTR_MASK_M_GEN_CALL                 ((uint32_t)0x0800)

/*******************  Bit definition for IC_RAW_INTR_STAT register  ***********/
#define I2C_IC_RAW_INTR_STAT_RX_UNDER               ((uint32_t)0x0001)
#define I2C_IC_RAW_INTR_STAT_RX_OVER                ((uint32_t)0x0002)
#define I2C_IC_RAW_INTR_STAT_RX_FULL                ((uint32_t)0x0004)
#define I2C_IC_RAW_INTR_STAT_TX_OVER                ((uint32_t)0x0008)
#define I2C_IC_RAW_INTR_STAT_TX_EMPTY               ((uint32_t)0x0010)
#define I2C_IC_RAW_INTR_STAT_RD_REQ                 ((uint32_t)0x0020)
#define I2C_IC_RAW_INTR_STAT_TX_ABRT                ((uint32_t)0x0040)
#define I2C_IC_RAW_INTR_STAT_RX_DONE                ((uint32_t)0x0080)
#define I2C_IC_RAW_INTR_STAT_ACTIVITY               ((uint32_t)0x0100)
#define I2C_IC_RAW_INTR_STAT_STOP_DET               ((uint32_t)0x0200)
#define I2C_IC_RAW_INTR_STAT_START_DET              ((uint32_t)0x0400)
#define I2C_IC_RAW_INTR_STAT_GEN_CALL               ((uint32_t)0x0800)

/*******************  Bit definition for IC_RX_TL register  *******************/
#define I2C_IC_RX_TL_TL                             ((uint32_t)0x00FF)

/*******************  Bit definition for IC_TX_TL register  *******************/
#define I2C_IC_TX_TL_TL                             ((uint32_t)0x00FF)

/*******************  Bit definition for IC_CLR_INTR register  ****************/
#define I2C_IC_CLR_INTR                             ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_RX_UNDER register  ************/
#define I2C_IC_CLR_RX_UNDER                         ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_RX_OVER register  *************/
#define I2C_IC_CLR_RX_OVER                          ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_TX_OVER register  *************/
#define I2C_IC_CLR_TX_OVER                          ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_RD_REQ register  **************/
#define I2C_IC_CLR_RD_REQ                           ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_TX_ABRT register  *************/
#define I2C_IC_CLR_TX_ABRT                          ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_RX_DONE register  *************/
#define I2C_IC_CLR_RX_DONE                          ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_ACTIVITY register  ************/
#define I2C_IC_CLR_ACTIVITY                         ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_STOP_DET register  ************/
#define I2C_IC_CLR_STOP_DET                         ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_START_DET register  ***********/
#define I2C_IC_CLR_START_DET                        ((uint32_t)0x0001)

/*******************  Bit definition for IC_CLR_GEN_CALL register  ************/
#define I2C_IC_CLR_GEN_CALL                         ((uint32_t)0x0001)

/*******************  Bit definition for IC_ENABLE  register  *****************/
#define I2C_IC_ENABLE_ENABLE                        ((uint32_t)0x0001)
#define I2C_IC_ENABLE_ABORT                         ((uint32_t)0x0002)

/*******************  Bit definition for IC_STATUS  register  *****************/
#define I2C_IC_STATUS_ACTIVITY                      ((uint32_t)0x0001)
#define I2C_IC_STATUS_TFNF                          ((uint32_t)0x0002)
#define I2C_IC_STATUS_TFE                           ((uint32_t)0x0004)
#define I2C_IC_STATUS_RFNE                          ((uint32_t)0x0008)
#define I2C_IC_STATUS_RFF                           ((uint32_t)0x0010)
#define I2C_IC_STATUS_MST_ACTIVITY                  ((uint32_t)0x0020)
#define I2C_IC_STATUS_SLV_ACTIVITY                  ((uint32_t)0x0040)
/*******************  Bit definition for IC_TXFLR  register  ******************/
#define I2C_IC_TXFLR_TXFLR                          (8)

/*******************  Bit definition for IC_RXFLR  register  ******************/
#define I2C_IC_RXFLR_RXFLR                          (8)

/*******************  Bit definition for IC_SDA_HOLD  register  ***************/
#define I2C_IC_SDA_HOLD                             ((uint32_t)0xFFFF)

/*******************  Bit definition for IC_TX_ABRT_SOURCE  register  *********/
#define I2C_IC_TX_ABRT_SOURCE_7B_ADDR_NOACK         ((uint32_t)0x00000001)
#define I2C_IC_TX_ABRT_SOURCE_10ADDR1_NOACK         ((uint32_t)0x00000002)
#define I2C_IC_TX_ABRT_SOURCE_10ADDR2_NOACK         ((uint32_t)0x00000004)
#define I2C_IC_TX_ABRT_SOURCE_TXDATA_NOACK          ((uint32_t)0x00000008)
#define I2C_IC_TX_ABRT_SOURCE_GCALL_NOACK           ((uint32_t)0x00000010)
#define I2C_IC_TX_ABRT_SOURCE_GCALL_READ            ((uint32_t)0x00000020)
#define I2C_IC_TX_ABRT_SOURCE_HS_ACKDET             ((uint32_t)0x00000040)
#define I2C_IC_TX_ABRT_SOURCE_SBYTE_ACKDET          ((uint32_t)0x00000080)
#define I2C_IC_TX_ABRT_SOURCE_HS_NORSTRT            ((uint32_t)0x00000100)
#define I2C_IC_TX_ABRT_SOURCE_SBYTE_NORSTRT         ((uint32_t)0x00000200)
#define I2C_IC_TX_ABRT_SOURCE_10B_RD_NORSTRT        ((uint32_t)0x00000400)
#define I2C_IC_TX_ABRT_SOURCE_MASTER_DIS            ((uint32_t)0x00000800)
#define I2C_IC_TX_ABRT_SOURCE_LOST                  ((uint32_t)0x00001000)
#define I2C_IC_TX_ABRT_SOURCE_SLVFLUSH_TXFIFO       ((uint32_t)0x00002000)
#define I2C_IC_TX_ABRT_SOURCE_SLV_ARBLOST           ((uint32_t)0x00004000)
#define I2C_IC_TX_ABRT_SOURCE_SLVRD_INTX            ((uint32_t)0x00008000)
#define I2C_IC_TX_ABRT_SOURCE_USER_ABRT             ((uint32_t)0x00010000)
#define I2C_IC_TX_ABRT_SOURCE_TX_FLUSH_CNT          ((uint32_t)0xFF000000)

/*******************  Bit definition for IC_SLV_DATA_NACK_ONLY  register  *****/
#define I2C_IC_SLV_DATA_NACK_ONLY                   ((uint32_t)0x0001)

/*******************  Bit definition for IC_DMA_TDLR  register  ***************/
#define I2C_IC_DMA_TDLR_TDLR                        ((uint32_t)0x000F)

/*******************  Bit definition for IC_DMA_RDLR  register  ***************/
#define I2C_IC_DMA_TDLR_TDLR                        ((uint32_t)0x000F)

/*******************  Bit definition for IC_SDA_SETUP  register  **************/
#define I2C_IC_SDA_SETUP                            ((uint32_t)0x00FF)

/*******************  Bit definition for IC_ACK_GENERAL_CALL  register  *******/
#define I2C_IC_ACK_GENERAL_CALL                     ((uint32_t)0x0001)

/*******************  Bit definition for IC_ENABLE_STATUS  register  **********/
#define I2C_IC_ENABLE_STATUS_IC_EN                  ((uint32_t)0x0001)
#define I2C_IC_ENABLE_STATUS_SLV_RX_ABORTED         ((uint32_t)0x0001)
#define I2C_IC_ENABLE_STATUS_SLV_FIFO_FILLED_AND_FLUSHED        ((uint32_t)0x0001)

/*******************  Bit definition for IC_FS_SPKLEN  register  **************/
#define I2C_IC_FS_SPKLEN_SPKLEN                     ((uint32_t)0x00FF)

/*******************  Bit definition for IC_HS_SPKLEN  register  **************/
#define I2C_IC_HS_SPKLEN_SPKLEN                     ((uint32_t)0x00FF)

/*******************  Bit definition for IC_COMP_PARAM_1  register  ***********/
#define I2C_IC_COMP_PARAM_1_APB_DATA_WIDTH          ((uint32_t)0x0003)
#define I2C_IC_COMP_PARAM_1_APB_DATA_WIDTH_0        ((uint32_t)0x0001)
#define I2C_IC_COMP_PARAM_1_APB_DATA_WIDTH_1        ((uint32_t)0x0002)
#define I2C_IC_COMP_PARAM_1_MAX_SPEED_MODE          ((uint32_t)0x000C)
#define I2C_IC_COMP_PARAM_1_MAX_SPEED_MODE_0        ((uint32_t)0x0004)
#define I2C_IC_COMP_PARAM_1_MAX_SPEED_MODE_1        ((uint32_t)0x0008)
#define I2C_IC_COMP_PARAM_1_HC_COUNT_VALUES         ((uint32_t)0x0010)
#define I2C_IC_COMP_PARAM_1_INTR_IO                 ((uint32_t)0x0020)
#define I2C_IC_COMP_PARAM_1_HAS_DMA                 ((uint32_t)0x0040)
#define I2C_IC_COMP_PARAM_1_ADD_ENCODED_PARAMS      ((uint32_t)0x0080)
#define I2C_IC_COMP_PARAM_1_RX_BUFFER_DEPTH         ((uint32_t)0x0000FF00)
#define I2C_IC_COMP_PARAM_1_TX_BUFFER_DEPTH         ((uint32_t)0x00FF0000)

/*******************  Bit definition for IC_COMP_VERSION  register  ***********/
#define I2C_IC_COMP_VERSION                         ((uint32_t)0xFFFFFFFF)

/*******************  Bit definition for IC_COMP_TYPE  register  **************/
#define I2C_IC_COMP_TYPE                            ((uint32_t)0xFFFFFFFF)


/******************************************************************************/
/*                                                                            */
/*                      Backup Register Unit Block                            */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for BPK_RDY register  ********************/
#define BPK_RDY_POR                                 ((uint32_t)0x0002)
#define BPK_RDY_READY                               ((uint32_t)0x0001)

/*******************  Bit definition for BPK_RR register  *********************/
#define BPK_RR_RESET                                ((uint32_t)0x0001)

/*******************  Bit definition for BPK_LR register  *********************/
#define BPK_LR_LOCK_SELF                            ((uint32_t)0x0001)
#define BPK_LR_LOCK_RESET                           ((uint32_t)0x0002)
#define BPK_LR_LOCK_KEYWRITE                        ((uint32_t)0x0004)
#define BPK_LR_LOCK_KEYREAD                         ((uint32_t)0x0008)
#define BPK_LR_LOCK_KEYCLEAR                        ((uint32_t)0x0010)
#define BPK_LR_LOCK_ALL                             ((uint32_t)0x001F)

/******************************************************************************/
/*                                                                            */
/*                               RTC Unit Block                               */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for RTC_CS register  *********************/
#define RTC_CS_ALARM_IT                            ((uint32_t)0x01)
#define RTC_CS_LOCK_TIM                            ((uint32_t)0x02)
#define RTC_CS_ALARM_EN                            ((uint32_t)0x04)
#define RTC_CS_READY                               ((uint32_t)0x08)
#define RTC_CS_CLR                                 ((uint32_t)0x10)

/******************************************************************************/
/*                                                                            */
/*                      Keyboard Control Unit Block                           */
/*                                                                            */
/******************************************************************************/
/*****************  Bit definition for KCU_CTRL0 register  ********************/
#define KCU_PORT_0                                  ((uint32_t)0x0001)
#define KCU_PORT_1                                  ((uint32_t)0x0002)
#define KCU_PORT_2                                  ((uint32_t)0x0004)
#define KCU_PORT_3                                  ((uint32_t)0x0008)
#define KCU_PORT_4                                  ((uint32_t)0x0010)
#define KCU_PORT_5                                  ((uint32_t)0x0020)
#define KCU_PORT_6                                  ((uint32_t)0x0040)
#define KCU_PORT_7                                  ((uint32_t)0x0080)
#define KCU_PORT_8                                  ((uint32_t)0x0100)
#define KCU_PORT_ALL_Mask                           ((uint32_t)0x001F)

#define KCU_DEBOUNCETIMELEVEL_0                     ((uint32_t)0x0000)
#define KCU_DEBOUNCETIMELEVEL_1                     ((uint32_t)0x0001)
#define KCU_DEBOUNCETIMELEVEL_2                     ((uint32_t)0x0002)
#define KCU_DEBOUNCETIMELEVEL_3                     ((uint32_t)0x0003)
#define KCU_DEBOUNCETIMELEVEL_4                     ((uint32_t)0x0004)
#define KCU_DEBOUNCETIMELEVEL_5                     ((uint32_t)0x0005)
#define KCU_DEBOUNCETIMELEVEL_6                     ((uint32_t)0x0006)
#define KCU_DEBOUNCETIMELEVEL_7                     ((uint32_t)0x0007)
#define KCU_DEBOUNCETIMELEVEL_POS                   (9)

/*****************  Bit definition for KCU_CTRL1 register  ********************/
#define KCU_CTRL1_KBD_EN                            ((uint32_t)0x0001)
#define KCU_CTRL1_PUSH_IT                           ((uint32_t)0x0002)
#define KCU_CTRL1_RELEASE_IT                        ((uint32_t)0x0004)
#define KCU_CTRL1_OVERRUN_IT                        ((uint32_t)0x0008)
#define KCU_CTRL1_KCU_RUNING                        ((uint32_t)0x80000000)

/*****************  Bit definition for KCU_STATUS register  *******************/
#define KCU_STATUS_IT                               ((uint32_t)0x0001)
#define KCU_STATUS_OVERRUN_IT                       ((uint32_t)0x0002)
#define KCU_STATUS_PUSH_IT                          ((uint32_t)0x0004)
#define KCU_STATUS_RELEASE_IT                       ((uint32_t)0x0008)
#define KCU_STATUS_EVENT_0_PUSH                     ((uint32_t)0x0010)
#define KCU_STATUS_EVENT_0_NEW                      ((uint32_t)0x0020)
#define KCU_STATUS_EVENT_1_PUSH                     ((uint32_t)0x0040)
#define KCU_STATUS_EVENT_1_NEW                      ((uint32_t)0x0080)
#define KCU_STATUS_EVENT_2_PUSH                     ((uint32_t)0x0100)
#define KCU_STATUS_EVENT_2_NEW                      ((uint32_t)0x0200)
#define KCU_STATUS_EVENT_3_PUSH                     ((uint32_t)0x0400)
#define KCU_STATUS_EVENT_3_NEW                      ((uint32_t)0x0800)

/*****************  Bit definition for KCU_EVENT register  ********************/
#define KCU_EVENT_EVENT_0_INPUT_NUM                 ((uint32_t)0x0000000F)
#define KCU_EVENT_EVENT_0_OUTPUT_NUM                ((uint32_t)0x000000F0)
#define KCU_EVENT_EVENT_1_INPUT_NUM                 ((uint32_t)0x00000F00)
#define KCU_EVENT_EVENT_1_OUTPUT_NUM                ((uint32_t)0x0000F000)
#define KCU_EVENT_EVENT_2_INPUT_NUM                 ((uint32_t)0x000F0000)
#define KCU_EVENT_EVENT_2_OUTPUT_NUM                ((uint32_t)0x00F00000)
#define KCU_EVENT_EVENT_3_INPUT_NUM                 ((uint32_t)0x0F000000)
#define KCU_EVENT_EVENT_3_OUTPUT_NUM                ((uint32_t)0xF0000000)


/******************************************************************************/
/*                                                                            */
/*                      Timer Control Unit Block                              */
/*                                                                            */
/******************************************************************************/
/**********  Bit definition for TIMER_CONTROL_REG register  *******************/
#define TIMER_CONTROL_REG_TIMER_ENABLE              (0x0001U)
#define TIMER_CONTROL_REG_TIMER_MODE                (0x0002U)
#define TIMER_CONTROL_REG_TIMER_INTERRUPT           (0x0004U)
#define TIMER_CONTROL_REG_TIMER_PWM                 (0x0008U)
#define TIMER_CONTROL_REG_PWM_SINGLE_PULSE          (0x0010U)
#define TIMER_CONTROL_REG_PWM_RELOAD_SINGLE_PULSE   (0x0020U)
/*****************  Bit definition for IntStatus register  ********************/
#define TIMER_INT_STATUS_INTERRUPT                  (0x0001U)


/******************************************************************************/
/*                                                                            */
/*                          WDT Control Unit Block                            */
/*                                                                            */
/******************************************************************************/
/*****************  Bit definition for WDT_CR register  ***********************/
#define WDT_CR_WDT_EN                               ((uint32_t)0x0001)
#define WDT_CR_RMOD                                 ((uint32_t)0x0002)
/*****************  Bit definition for WDT_CCVR register  *********************/
#define WDT_CCVR_CCVR                               ((uint32_t)0xFFFFFFFF)
/*****************  Bit definition for WDT_CRR register  **********************/
#define WDT_CRR_CRR                                 ((uint32_t)0x00FF)
/*****************  Bit definition for WDT_STAT register  *********************/
#define WDT_STAT_INT                                ((uint32_t)0x0001)
/*****************  Bit definition for WDT_EOI register  **********************/
#define WDT_EOI_EOI                                 ((uint32_t)0x0001)
/*****************  Bit definition for WDT_RLD register  **********************/
#define WDT_RLD_RLD                                 ((uint32_t)0xFFFFFFFF)


/******************************************************************************/
/*                                                                            */
/*                          RNG Control Unit Block                            */
/*                                                                            */
/******************************************************************************/

/************ bit definition for TRNG RNG_CSR REGISTER ************/
#define TRNG_RNG_CSR_INTP_EN_Mask                   ((uint32_t)0x0010)
#define TRNG_RNG_CSR_ATTACK_TRNG0_Mask              ((uint32_t)0x0004)
#define TRNG_RNG_CSR_S128_TRNG0_Mask                ((uint32_t)0x0001)

/************ bit definition for TRNG RNG_AMA REGISTER ************/
#define TRNG_RNG_AMA_ANA_OUT_TRNG0_Mask             ((uint32_t)0x10000000)
#define TRNG_RNG_AMA_PD_TRNG0_Mask                  ((uint32_t)0x00001000)
#define TRNG_RNG_AMA_PD_TRNG1_Mask                  ((uint32_t)0x00002000)
#define TRNG_RNG_AMA_PD_TRNG2_Mask                  ((uint32_t)0x00004000)
#define TRNG_RNG_AMA_PD_TRNG3_Mask                  ((uint32_t)0x00008000)
#define TRNG_RNG_AMA_PD_TRNG4_Mask                  ((uint32_t)0x00010000)
#define TRNG_RNG_AMA_PD_TRNG5_Mask                  ((uint32_t)0x00020000)
#define TRNG_RNG_AMA_PD_TRNG6_Mask                  ((uint32_t)0x00040000)
#define TRNG_RNG_AMA_PD_TRNG7_Mask                  ((uint32_t)0x00080000)
#define TRNG_RNG_AMA_PD_ALL_Mask                    ((uint32_t)0x000FF000)


/******************************************************************************/
/*                                                                            */
/*                                    DCMI                                    */
/*                                                                            */
/******************************************************************************/
/********************  Bits definition for DCMI_CR register  ******************/
#define DCMI_CR_CAPTURE                      ((uint32_t)0x00000001)
#define DCMI_CR_CM                           ((uint32_t)0x00000002)
#define DCMI_CR_CROP                         ((uint32_t)0x00000004)
#define DCMI_CR_JPEG                         ((uint32_t)0x00000008)
#define DCMI_CR_ESS                          ((uint32_t)0x00000010)
#define DCMI_CR_PCKPOL                       ((uint32_t)0x00000020)
#define DCMI_CR_HSPOL                        ((uint32_t)0x00000040)
#define DCMI_CR_VSPOL                        ((uint32_t)0x00000080)
#define DCMI_CR_FCRC_0                       ((uint32_t)0x00000100)
#define DCMI_CR_FCRC_1                       ((uint32_t)0x00000200)
#define DCMI_CR_EDM_0                        ((uint32_t)0x00000400)
#define DCMI_CR_EDM_1                        ((uint32_t)0x00000800)
#define DCMI_CR_CRE                          ((uint32_t)0x00001000)
#define DCMI_CR_ENABLE                       ((uint32_t)0x00004000)
#define DCMI_CR_BSM_0                        ((uint32_t)0x00010000)
#define DCMI_CR_BSM_1                        ((uint32_t)0x00020000)
#define DCMI_CR_OEBS                         ((uint32_t)0x00040000)
#define DCMI_CR_LSM                          ((uint32_t)0x00080000)
#define DCMI_CR_OELS                         ((uint32_t)0x00100000)

/********************  Bits definition for DCMI_SR register  ******************/
#define DCMI_SR_HSYNC                        ((uint32_t)0x00000001)
#define DCMI_SR_VSYNC                        ((uint32_t)0x00000002)
#define DCMI_SR_FNE                          ((uint32_t)0x00000004)

/********************  Bits definition for DCMI_RISR register  ****************/
#define DCMI_RISR_FRAME_RIS                  ((uint32_t)0x00000001)
#define DCMI_RISR_OVF_RIS                    ((uint32_t)0x00000002)
#define DCMI_RISR_ERR_RIS                    ((uint32_t)0x00000004)
#define DCMI_RISR_VSYNC_RIS                  ((uint32_t)0x00000008)
#define DCMI_RISR_LINE_RIS                   ((uint32_t)0x00000010)

/********************  Bits definition for DCMI_IER register  *****************/
#define DCMI_IER_FRAME_IE                    ((uint32_t)0x00000001)
#define DCMI_IER_OVF_IE                      ((uint32_t)0x00000002)
#define DCMI_IER_ERR_IE                      ((uint32_t)0x00000004)
#define DCMI_IER_VSYNC_IE                    ((uint32_t)0x00000008)
#define DCMI_IER_LINE_IE                     ((uint32_t)0x00000010)

/********************  Bits definition for DCMI_MISR register  ****************/
#define DCMI_MISR_FRAME_MIS                  ((uint32_t)0x00000001)
#define DCMI_MISR_OVF_MIS                    ((uint32_t)0x00000002)
#define DCMI_MISR_ERR_MIS                    ((uint32_t)0x00000004)
#define DCMI_MISR_VSYNC_MIS                  ((uint32_t)0x00000008)
#define DCMI_MISR_LINE_MIS                   ((uint32_t)0x00000010)

/********************  Bits definition for DCMI_ICR register  *****************/
#define DCMI_ICR_FRAME_ISC                   ((uint32_t)0x00000001)
#define DCMI_ICR_OVF_ISC                     ((uint32_t)0x00000002)
#define DCMI_ICR_ERR_ISC                     ((uint32_t)0x00000004)
#define DCMI_ICR_VSYNC_ISC                   ((uint32_t)0x00000008)
#define DCMI_ICR_LINE_ISC                    ((uint32_t)0x00000010)

/******************************************************************************/
/*                                                                            */
/*                                    PSRAM                                   */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for PSRAM PSRAM_CMD register  *******************/
#define PSRAM_PSRAM_CMD_COMMAND_CODE                    ((uint32_t)0xFF000000)
#define PSRAM_PSRAM_CMD_READ_BUS_MODE                   ((uint32_t)0x00000300)
#define PSRAM_PSRAM_CMD_WRITE_BUS_MODE                  ((uint32_t)0x000000C0)
#define PSRAM_PSRAM_CMD_QUAL_MODE                       ((uint32_t)0x00000020)
#define PSRAM_PSRAM_CMD_READ_ID                         ((uint32_t)0x00000010)
#define PSRAM_PSRAM_CMD_CMD_DONE                        ((uint32_t)0x00000008)
#define PSRAM_PSRAM_CMD_CMD_BUSY                        ((uint32_t)0x00000004)
#define PSRAM_PSRAM_CMD_CONFIG_CMD                      ((uint32_t)0x00000001)
/*******************  Bit definition for PSRAM_DEVICE_PARA register  *******************/
#define PSRAM_DEVICE_PARA_SAMPLE_EDGE_SEL               ((uint32_t)0x00080000)
#define PSRAM_DEVICE_PARA_CSN_SYCLE                     ((uint32_t)0x00030000)
#define PSRAM_DEVICE_PARA_BRUST_WORD                    ((uint32_t)0x0000FF00)
#define PSRAM_DEVICE_PARA_DUMMY_CYCLES                  ((uint32_t)0x000000F0)
#define PSRAM_DEVICE_PARA_FREQ_SEL                      ((uint32_t)0x00000003)


/******************************************************************************/
/*                                                                            */
/*                                    CSI2                                    */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for CSI2 CSI2_RESETN register  *****************/
#define CSI2_RESETN_RELEASE                             ((uint32_t)0x00000001)

/******************  Bit definition for CSI2 PHY_SHUTDOWNZ register  ****************/
#define CSI2_PHY_SHUTDOWNZ_RELEASE                      ((uint32_t)0x00000001)

/********************  Bit definition for CSI2 DPHY_RSTZ register  ******************/
#define CSI2_DPHY_RSTZ_RELEASE                          ((uint32_t)0x00000001)

/******************  Bit definition for CSI2 PHY_STOPSTATE register  ****************/
#define CSI2_PHY_STOPSTATE_DATA0                        ((uint32_t)0x00000001)
#define CSI2_PHY_STOPSTATE_DATA1                        ((uint32_t)0x00000002)
#define CSI2_PHY_STOPSTATE_CLOCK                        ((uint32_t)0x00010000)


/******************  Bit definition for QUADSPI_FCU_CMD register  ******************/
#define QUADSPI_FCU_CMD_CODE                            ((uint32_t)0xFF000000U)
#define QUADSPI_FCU_CMD_BUS_MODE                        ((uint32_t)0x00000300U)
#define QUADSPI_FCU_CMD_CMD_FORMAT                      ((uint32_t)0x000000F0U)
#define QUADSPI_FCU_CMD_DONE                            ((uint32_t)0x00000008U)
#define QUADSPI_FCU_CMD_BUSY                            ((uint32_t)0x00000004U)
#define QUADSPI_FCU_CMD_ACCESS_ACK                      ((uint32_t)0x00000002U)
#define QUADSPI_FCU_CMD_ACCESS_REQ                      ((uint32_t)0x00000001U)

#define QUADSPI_ADDRESS_ADR                             ((uint32_t)0xFFFFFF00U)
#define QUADSPI_ADDRESS_M8                              ((uint32_t)0x000000FFU)

#define QUADSPI_BYTE_NUM_WR_BYTE                        ((uint32_t)0x00001FFFU)
#define QUADSPI_BYTE_NUM_RD_BYTE                        ((uint32_t)0x1FFF0000U)

#define QUADSPI_WR_FIFO_WR_DATA                         ((uint32_t)0xFFFFFFFFU)
#define QUADSPI_RD_FIFO_RD_DATA                         ((uint32_t)0xFFFFFFFFU)

#define QUADSPI_DEVICE_PARA_PROTOCOL                    ((uint32_t)0x00000300U)
#define QUADSPI_DEVICE_PARA_DUMMY_CYCLE                 ((uint32_t)0x000000F0U)
#define QUADSPI_DEVICE_PARA_FLASH_READY                 ((uint32_t)0x00000008U)
#define QUADSPI_DEVICE_PARA_FREQ_SEL                    ((uint32_t)0x00000003U)

#define QUADSPI_REG_WDATA                               ((uint32_t)0xFFFFFFFFU)
#define QUADSPI_REG_RDATA                               ((uint32_t)0xFFFFFFFFU)

#define QUADSPI_INT_MASK_TFDM                           ((uint32_t)0x00000040U)
#define QUADSPI_INT_MASK_RFDM                           ((uint32_t)0x00000020U)
#define QUADSPI_INT_MASK_TFOM                           ((uint32_t)0x00000010U)
#define QUADSPI_INT_MASK_TFUM                           ((uint32_t)0x00000008U)
#define QUADSPI_INT_MASK_RFOM                           ((uint32_t)0x00000004U)
#define QUADSPI_INT_MASK_RFUM                           ((uint32_t)0x00000002U)
#define QUADSPI_INT_MASK_DONE_IM                        ((uint32_t)0x00000001U)

#define QUADSPI_INT_UMSAK_TFDU                          ((uint32_t)0x00000040U)
#define QUADSPI_INT_UMSAK_RFDU                          ((uint32_t)0x00000020U)
#define QUADSPI_INT_UMSAK_TFOU                          ((uint32_t)0x00000010U)
#define QUADSPI_INT_UMSAK_TFUU                          ((uint32_t)0x00000008U)
#define QUADSPI_INT_UMSAK_RFOU                          ((uint32_t)0x00000004U)
#define QUADSPI_INT_UMSAK_RFUU                          ((uint32_t)0x00000002U)
#define QUADSPI_INT_UMSAK_DONE_IU                       ((uint32_t)0x00000001U)

#define QUADSPI_INT_MASK_STATUS_TFDM                    ((uint32_t)0x00000040U)
#define QUADSPI_INT_MASK_STATUS_RFDM                    ((uint32_t)0x00000020U)
#define QUADSPI_INT_MASK_STATUS_TFOM                    ((uint32_t)0x00000010U)
#define QUADSPI_INT_MASK_STATUS_TFUM                    ((uint32_t)0x00000008U)
#define QUADSPI_INT_MASK_STATUS_RFOM                    ((uint32_t)0x00000004U)
#define QUADSPI_INT_MASK_STATUS_RFUM                    ((uint32_t)0x00000002U)
#define QUADSPI_INT_MASK_STATUS_DONE_IM                 ((uint32_t)0x00000001U)

#define QUADSPI_INT_STATUS_TFDS                         ((uint32_t)0x00000040U)
#define QUADSPI_INT_STATUS_RFDS                         ((uint32_t)0x00000020U)
#define QUADSPI_INT_STATUS_TFOS                         ((uint32_t)0x00000010U)
#define QUADSPI_INT_STATUS_TFUS                         ((uint32_t)0x00000008U)
#define QUADSPI_INT_STATUS_RFOS                         ((uint32_t)0x00000004U)
#define QUADSPI_INT_STATUS_RFUS                         ((uint32_t)0x00000002U)
#define QUADSPI_INT_STATUS_DONE_IS                      ((uint32_t)0x00000001U)

#define QUADSPI_INT_RAWSTATUS_TFDR                      ((uint32_t)0x00000040U)
#define QUADSPI_INT_RAWSTATUS_RFDR                      ((uint32_t)0x00000020U)
#define QUADSPI_INT_RAWSTATUS_TFOR                      ((uint32_t)0x00000010U)
#define QUADSPI_INT_RAWSTATUS_TFUR                      ((uint32_t)0x00000008U)
#define QUADSPI_INT_RAWSTATUS_RFOR                      ((uint32_t)0x00000004U)
#define QUADSPI_INT_RAWSTATUS_RFUR                      ((uint32_t)0x00000002U)
#define QUADSPI_INT_RAWSTATUS_DONE_IR                   ((uint32_t)0x00000001U)

#define QUADSPI_INT_CLEAR_CTFD                          ((uint32_t)0x00000040U)
#define QUADSPI_INT_CLEAR_CRFD                          ((uint32_t)0x00000020U)
#define QUADSPI_INT_CLEAR_CTFO                          ((uint32_t)0x00000010U)
#define QUADSPI_INT_CLEAR_CTFU                          ((uint32_t)0x00000008U)
#define QUADSPI_INT_CLEAR_CRFO                          ((uint32_t)0x00000004U)
#define QUADSPI_INT_CLEAR_CRFU                          ((uint32_t)0x00000002U)
#define QUADSPI_INT_CLEAR_DONE                          ((uint32_t)0x00000001U)

#define QUADSPI_CACHE_INTF_CMD_RELDS                    ((uint32_t)0xFF000000U)
#define QUADSPI_CACHE_INTF_CMD_DS                       ((uint32_t)0x00FF0000U)
#define QUADSPI_CACHE_INTF_CMD_RD_BUS_MODE              ((uint32_t)0x00003000U)
#define QUADSPI_CACHE_INTF_CMD_RD_FORMAT                ((uint32_t)0x00000F00U)
#define QUADSPI_CACHE_INTF_CMD_RDCMD                    ((uint32_t)0x000000FFU)

#define QUADSPI_DMA_CNTL_TX_EN                          ((uint32_t)0x00000001U)

#define QUADSPI_FIFO_CNTL_TFFH                          ((uint32_t)0x80000000U)
#define QUADSPI_FIFO_CNTL_TFE                           ((uint32_t)0x00200000U)
#define QUADSPI_FIFO_CNTL_TFFL                          ((uint32_t)0x00100000U)
#define QUADSPI_FIFO_CNTL_TFL                           ((uint32_t)0x000F0000U)
#define QUADSPI_FIFO_CNTL_RFFH                          ((uint32_t)0x00008000U)
#define QUADSPI_FIFO_CNTL_RFE                           ((uint32_t)0x00000020U)
#define QUADSPI_FIFO_CNTL_RFFL                          ((uint32_t)0x00000010U)
#define QUADSPI_FIFO_CNTL_RFL                           ((uint32_t)0x0000000FU)


#define MHSCPU_READ_REG8(reg)                       (*(__IO uint8_t *) reg)
#define MHSCPU_READ_REG16(reg)                      (*(__IO uint16_t *) reg)
#define MHSCPU_READ_REG32(reg)                      (*(__IO uint32_t *) reg)
#define MHSCPU_WRITE_REG8(reg, value)               (*(__IO uint8_t *) reg = value)
#define MHSCPU_WRITE_REG16(reg, value)              (*(__IO uint16_t *) reg = value)
#define MHSCPU_WRITE_REG32(reg, value)              (*(__IO uint32_t *) reg = value)
#define MHSCPU_MODIFY_REG8(reg, clear_mask, set_mask)   \
        MHSCPU_WRITE_REG8(reg, (((MHSCPU_READ_REG8(reg)) & ~clear_mask) | set_mask))
#define MHSCPU_MODIFY_REG16(reg, clear_mask, set_mask)   \
        MHSCPU_WRITE_REG16(reg, (((MHSCPU_READ_REG16(reg)) & ~clear_mask) | set_mask))
#define MHSCPU_MODIFY_REG32(reg, clear_mask, set_mask)   \
        MHSCPU_WRITE_REG32(reg, (((MHSCPU_READ_REG32(reg)) & ~clear_mask) | set_mask))

#ifdef USE_STDPERIPH_DRIVER
#include "mhscpu_conf.h"
#endif

#ifdef __cplusplus
}
#endif

#endif  /* MHSCPU_H */
