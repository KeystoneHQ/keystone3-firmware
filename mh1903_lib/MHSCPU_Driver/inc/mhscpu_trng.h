/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_trng.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the TRNG firmware library
 *****************************************************************************/

#ifndef __MHSCPU_TRNG_H
#define __MHSCPU_TRNG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhscpu.h"

/** @defgroup RNG_Exported_Types
  * @{
  */
typedef enum {
    TRNG0
} TRNG_ChannelTypeDef;
#define IS_TRNG_CHANNEL(CHANNEL)    (((CHANNEL) == TRNG0))


#define TRNG_IT_RNG0_S128           ((uint32_t)0x00000001)
#define TRNG_IT_RNG0_ATTACK         ((uint32_t)0x00000004)
#define IS_TRNG_GET_IT(IT)          (((IT) == TRNG_IT_RNG0_S128) || \
                                    ((IT) == TRNG_IT_RNG0_ATTACK))

/** @defgroup RNG_Exported_Functions
  * @{
  */
void TRNG_Start(TRNG_ChannelTypeDef TRNGx);
void TRNG_Stop(TRNG_ChannelTypeDef TRNGx);
uint32_t TRNG_Get(uint32_t rand[4], TRNG_ChannelTypeDef TRNGx);
void TRNG_SetPseudoRandom(uint32_t PseudoRandom);
void TRNG_DirectOutANA(TRNG_ChannelTypeDef TRNGx, FunctionalState NewState);

void TRNG_ITConfig(FunctionalState NewState);
ITStatus TRNG_GetITStatus(uint32_t TRNG_IT);
void TRNG_ClearITPendingBit(uint32_t TRNG_IT);


#ifdef __cplusplus
}
#endif

#endif

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
