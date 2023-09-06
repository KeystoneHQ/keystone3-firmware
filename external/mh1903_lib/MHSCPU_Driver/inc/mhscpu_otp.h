/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_otp.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the OTP firmware library
 *****************************************************************************/

#ifndef __MHSCPU_OTP_H
#define __MHSCPU_OTP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhscpu.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
    OTP_Complete = 0,
    OTP_ReadOnProgramOrSleep,               //在编程、休眠状态下对OTP进行读操作
    OTP_ProgramIn_HiddenOrRO_Block,         //对只读区进行编程
    OTP_ProgramOutOfAddr,                   //编程范围超过OTP范围
    OTP_ProgramOnSleep,                     //在休眠状态进行编程操作
    OTP_WakeUpOnNoSleep,                    //在非休眠状态下进行唤醒操作
    OTP_TimeOut,                            //OTP完成标志位超时没有置位
    OTP_DataWrong,
} OTP_StatusTypeDef;

#define IS_OTP_ADDRESS(ADDRESS) (((ADDRESS) > OTP_BASE - 1) && ((ADDRESS) < OTP_BASE + OTP_SIZE))


void OTP_WakeUp(void);
void OTP_Unlock(void);
void OTP_Lock(void);
void OTP_ClearStatus(void);
void OTP_SetLatency(uint8_t u8_1UsClk, uint8_t u8_10NsCLK);
void OTP_TimCmd(FunctionalState NewState);

void OTP_SetProtect(uint32_t u32Addr);
void OTP_SetProtectLock(uint32_t u32Addr);
void OTP_UnProtect(uint32_t u32Addr);
uint32_t OTP_GetProtect(void);
uint32_t OTP_GetProtectLock(void);

void OTP_PowerOn(void);

Boolean OTP_IsReadReady(void);
Boolean OTP_IsWriteDone(void);

OTP_StatusTypeDef OTP_GetFlag(void);

Boolean OTP_IsProtect(uint32_t u32Addr);
Boolean OTP_IsProtectLock(uint32_t u32Addr);

OTP_StatusTypeDef OTP_WriteWord(uint32_t addr, uint32_t w);


#ifdef __cplusplus
}
#endif

#endif

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
