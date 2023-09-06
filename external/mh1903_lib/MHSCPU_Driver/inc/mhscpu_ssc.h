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


#define SSC_ITSysXTAL12M                    BIT(18) //ϵͳ��12Mʱ�ӱ�־
#define SSC_ITSysGlitch                     BIT(17) //����Դë�̱�־
#define SSC_ITSysVolHigh                    BIT(16) //����Դ��ѹ��־
#define SSC_ITSysVolLow                     BIT(15) //����ԴǷѹ��־

typedef struct {
    FunctionalState ParityCheck;                                //��żУ��ʹ��
} SSC_InitTypeDef;


/*
 *  ��BPK��Ϊ4��ÿ��256����Ϊ��λ���ö�дȨ��
 *  SSC_BPKAccessCtrBlock_0Ϊ��ʼ0��ַ��
 */
#define SSC_BPKAccessCtrBlock_0             (0x01)
#define SSC_BPKAccessCtrBlock_1             (0x02)

typedef enum {
    SSC_BPKReadOnly     = 0x01,     //BPK��ֻ��
    SSC_BPKWriteOnly    = 0x02,     //BPK��ֻд
    SSC_BPKReadWrite    = 0x03      //BPK���д
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
  * @brief  SSC��ȫ���Գ�ʼ��
  * @param  SSC_InitTypeDef SSC_InitStruct
  * @retval void
  */
void SSC_Init(SSC_InitTypeDef *SSC_InitStruct);


/**
  * @method SSC_GetITStatus
  * @brief  SSC��ȫ�ж�״̬
  * @param  uint32_t SSC_IT
  * @retval ITStatus
  */
ITStatus SSC_GetITStatus(uint32_t SSC_IT);


/**
  * @method SSC_ClearITPendingBit
  * @brief  SSC��ȫ�ж����
  * @param  uint32_t SSC_IT
  * @retval void
  */
void SSC_ClearITPendingBit(uint32_t SSC_IT);


/**
  * @method SSC_SetDataRAMScrambler
  * @brief  ��������RAM����
  * @param  uint32_t Scrambler
  * @retval void
  */
void SSC_SetDataRAMScrambler(uint32_t Scrambler);


/**
  * @method SSC_BPKAccessCtrConfig
  * @brief  ����BPK����Ȩ��
  * @param  uint32_t SSC_BPKAccessCtrBlock
  * @param  SSC_BPKAccessCtrlTypeDef SSC_BPKAccessCtr
  * @retval void
  */
void SSC_BPKAccessCtrlConfig(uint32_t SSC_BPKAccessCtrBlock, SSC_BPKAccessCtrlTypeDef SSC_BPKAccessCtr);


/**
  * @method SSC_SENSOR_Enable
  * @brief  ����ϵͳSensor
  * @param  SSC_SENSOR
  * @retval
  */
uint32_t SSC_SENSORCmd(uint32_t SSC_SENSOR, FunctionalState NewState);


/**
  * @method SSC_SENSORLock
  * @brief  ����ϵͳSensor����״̬
  * @param  SSC_SENSOR
  * @retval
  */
void SSC_SENSORLock(uint32_t SSC_SENSOR);


/**
  * @method SSC_SENSOR_AttackRespMode
  * @brief  ϵͳSensor��Ӧ��ʽ
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
