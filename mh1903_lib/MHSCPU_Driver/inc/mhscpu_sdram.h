/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_sdram.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 24-February-2018
 * @brief               : This file contains all the functions prototypes for the SDRAM firmware library
 *****************************************************************************/

#ifndef __MHSCPU_SDRAM_H
#define __MHSCPU_SDRAM_H


#ifdef __cplusplus
extern "C" {
#endif

#include "mhscpu.h"


#define SDRAM_FBCLK_SEL_Pos              (28)
#define SDRAM_FBCLK_SEL_Mask             (0xF0000000U)

#define SDRAM_CONTROL_ENABLE             0x1

#define SDRAM_CONFIG_LITTLE_ENDIAN       0x0
#define SDRAM_CONFIG_BIG_ENDIAN          0x1


typedef struct {
    uint32_t ModelConfig;           // SDRAM Config
    uint32_t Endian;                // Little-Endian = 0, Big-Endian = 1
    uint32_t ClockDelay;            //
    uint32_t SysFreq;               // Sys Clock Frequence
    uint32_t CAS;                   // CAS
    uint32_t RAS;                   // RAS
    uint32_t tRefresh;              // Refresh Clk
    uint32_t tRP;
    uint32_t tRAS;
    uint32_t tSREX;
    uint32_t tWR;
    uint32_t tRC;
    uint32_t tRFC;
    uint32_t tXSR;
    uint32_t tRRD;
    uint32_t tMRD;
    uint32_t tCDLR;

} SDRAM_InitTypeDef;


void SDRAM_Init(SDRAM_InitTypeDef *SDRAM_InitStruct);
void SDRAM_InitSequence(uint32_t ModeSet, FunctionalState NewState);


#ifdef __cplusplus
}
#endif

#endif

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
