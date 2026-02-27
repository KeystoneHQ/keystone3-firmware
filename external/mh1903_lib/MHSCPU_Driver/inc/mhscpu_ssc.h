/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_ssc.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the SSC firmware library
 *****************************************************************************/

#ifndef __MHSCPU_SSC_H
#define __MHSCPU_SSC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhscpu.h"


#define SSC_ITSysXTAL12M                    BIT(18) //系统主12M时钟标志
#define SSC_ITSysGlitch                     BIT(17) //主电源毛刺标志
#define SSC_ITSysVolHigh                    BIT(16) //主电源过压标志
#define SSC_ITSysVolLow                     BIT(15) //主电源欠压标志

typedef struct {
    FunctionalState ParityCheck;                                //奇偶校验使能
} SSC_InitTypeDef;


/*
 *  将BPK分为4块每块256比特为单位设置读写权限
 *  SSC_BPKAccessCtrBlock_0为起始0地址块
 */
#define SSC_BPKAccessCtrBlock_0             (0x01)
#define SSC_BPKAccessCtrBlock_1             (0x02)

typedef enum {
    SSC_BPKReadOnly     = 0x01,     //BPK块只读
    SSC_BPKWriteOnly    = 0x02,     //BPK块只写
    SSC_BPKReadWrite    = 0x03      //BPK块读写
} SSC_BPKAccessCtrlTypeDef;
#define IS_BPK_ACCESS_CTRL(CTRL) (((CTRL) == SSC_BPKReadOnly) || ((CTRL) == SSC_BPKWriteOnly) || \
                                ((CTRL) == SSC_BPKReadWrite))

#define SSC_SENSOR_XTAL12M                          ((uint32_t)0x00000001)
#define SSC_SENSOR_VOL_LOW                          ((uint32_t)0x00000002)
#define SSC_SENSOR_VOL_HIGH                         ((uint32_t)0x00000004)
#define SSC_SENSOR_VOLGLITCH                        ((uint32_t)0x00000008)
#define IS_SSC_SENSOR(SENSOR)                       ((((SENSOR) & (uint32_t)0xFFFFFFF0) == 0x00) && ((SENSOR) != (uint32_t)0x00))

typedef enum {
    SSC_SENSOR_CPUReset  = 0,
    SSC_SENSOR_Interrupt = 1
} SSC_SENSOR_RespModeTypeDef;
#define IS_SSC_SENSOR_RESP_MODE(Mode)               ((Mode) == SSC_SENSOR_CPUReset ||\
                                                    (Mode) == SSC_SENSOR_Interrupt)

/**
  * @method SSC_Init
  * @brief  SSC安全特性初始化
  * @param  SSC_InitTypeDef SSC_InitStruct
  * @retval void
  */
void SSC_Init(SSC_InitTypeDef *SSC_InitStruct);


/**
  * @method SSC_GetITStatus
  * @brief  SSC安全中断状态
  * @param  uint32_t SSC_IT
  * @retval ITStatus
  */
ITStatus SSC_GetITStatus(uint32_t SSC_IT);


/**
  * @method SSC_ClearITPendingBit
  * @brief  SSC安全中断清楚
  * @param  uint32_t SSC_IT
  * @retval void
  */
void SSC_ClearITPendingBit(uint32_t SSC_IT);


/**
  * @method SSC_SetDataRAMScrambler
  * @brief  设置数据RAM扰码
  * @param  uint32_t Scrambler
  * @retval void
  */
void SSC_SetDataRAMScrambler(uint32_t Scrambler);


/**
  * @method SSC_BPKAccessCtrConfig
  * @brief  设置BPK访问权限
  * @param  uint32_t SSC_BPKAccessCtrBlock
  * @param  SSC_BPKAccessCtrlTypeDef SSC_BPKAccessCtr
  * @retval void
  */
void SSC_BPKAccessCtrlConfig(uint32_t SSC_BPKAccessCtrBlock, SSC_BPKAccessCtrlTypeDef SSC_BPKAccessCtr);


/**
  * @method SSC_SENSOR_Enable
  * @brief  开启系统Sensor
  * @param  SSC_SENSOR
  * @retval
  */
uint32_t SSC_SENSORCmd(uint32_t SSC_SENSOR, FunctionalState NewState);


/**
  * @method SSC_SENSORLock
  * @brief  锁定系统Sensor开启状态
  * @param  SSC_SENSOR
  * @retval
  */
void SSC_SENSORLock(uint32_t SSC_SENSOR);


/**
  * @method SSC_SENSOR_AttackRespMode
  * @brief  系统Sensor响应方式
  * @param  SSC_SENSOR_RespMode
  * @retval
  */
void SSC_SENSORAttackRespMode(SSC_SENSOR_RespModeTypeDef SSC_SENSOR_RespMode);


void SSC_SENSORKeyClearCmd(uint32_t SSC_SENSOR, FunctionalState KeyClearEn);


#ifdef __cplusplus
}
#endif

#endif

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
