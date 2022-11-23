/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_kcu.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the KCU firmware library
 *****************************************************************************/

#ifndef __MHSCPU_KCU_H
#define __MHSCPU_KCU_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhscpu.h"

/**
  * @brief  KCU Init structure definition
  */
typedef struct {
    uint32_t KCU_DebounceTimeLevel;
    uint32_t KCU_PortInput;
    uint32_t KCU_PortOutput;
    uint32_t KCU_Rand;
} KCU_InitTypeDef;


#define KCU_IT_PUSH                         (KCU_CTRL1_PUSH_IT)
#define KCU_IT_RELEASE                      (KCU_CTRL1_RELEASE_IT)
#define KCU_IT_OVERRUN                      (KCU_CTRL1_OVERRUN_IT)

void KCU_DeInit(void);

void KCU_Init(KCU_InitTypeDef *KCU_InitStruct);
void KCU_StructInit(KCU_InitTypeDef *KCU_InitStruct);

void KCU_Cmd(FunctionalState NewState);

uint32_t KCU_SetRand(uint32_t rand);

void KCU_ITConfig(uint32_t KCU_IT, FunctionalState NewState);
FlagStatus KCU_GetITStatus(uint32_t KCU_IT);
void KCU_ClearITPending(void);

uint32_t KCU_GetEvenReg(void);


#ifdef __cplusplus
}
#endif

#endif


/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
