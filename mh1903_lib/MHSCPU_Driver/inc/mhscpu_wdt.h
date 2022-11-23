/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_wdt.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the WDT firmware library
 *****************************************************************************/

#ifndef __MHSCPU_WDT_H
#define __MHSCPU_WDT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhscpu.h"

typedef enum {
    WDT_Mode_CPUReset = 0,
    WDT_Mode_Interrupt = 1
} WDT_ModeTypeDef;

void WDT_SetReload(uint32_t Reload);
void WDT_ReloadCounter(void);
void WDT_Enable(void);
void WDT_ModeConfig(WDT_ModeTypeDef WDT_Mode);

ITStatus WDT_GetITStatus(void);
void WDT_ClearITPendingBit(void);

#ifdef __cplusplus
}
#endif

#endif


/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
