/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_sensor.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the SENSOR firmware library
 *****************************************************************************/

#ifndef __MHSCPU_SENSOR_H
#define __MHSCPU_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhscpu.h"

typedef enum {
    SENSOR_EN_EXT0 = 0,
    SENSOR_EN_EXT1,
    SENSOR_EN_EXT2,
    SENSOR_EN_EXT3,
    SENSOR_EN_EXT4,
    SENSOR_EN_EXT5,
    SENSOR_EN_EXT6,
    SENSOR_EN_EXT7,
    //SENSOR_EN_EXT8,
    //SENSOR_EN_EXT9,
    //SENSOR_EN_EXT10,
    //SENSOR_EN_EXT11,
    SENSOR_EN_VH = 12,
    SENSOR_EN_VL,
    SENSOR_EN_TH,
    SENSOR_EN_TL,
    SENSOR_EN_XTAL32,
    SENSOR_EN_MESH,
    SENSOR_EN_VOLGLITCH,
} SENSOR_EN_Def;

typedef struct {
    uint32_t SENSOR_Port_Pull;
    uint32_t SENSOR_Port_Static;
    uint32_t SENSOR_Port_Dynamic;
    FunctionalState SENSOR_Port_Enable;
    uint32_t SENSOR_DynamicFrequency;
    FunctionalState SENSOR_GlitchEnable;
    uint32_t SENSOR_PUPU_Frequency;
    uint32_t SENSOR_PUPU_DetectTime;
    uint32_t SENSOR_PUPU_HoldTime;
    FunctionalState SENSOR_PUPU_Enable;
    FunctionalState SENSOR_Trig_Hold_Enable;
    uint32_t SENSOR_Dynamic_Sample;
    uint32_t SENSOR_Static_Sample;

} SENSOR_EXTInitTypeDef;

#define SENSOR_Port_S0                              ((uint32_t)0x0001)
#define SENSOR_Port_S1                              ((uint32_t)0x0002)
#define SENSOR_Port_S2                              ((uint32_t)0x0004)
#define SENSOR_Port_S3                              ((uint32_t)0x0008)
#define SENSOR_Port_S4                              ((uint32_t)0x0010)
#define SENSOR_Port_S5                              ((uint32_t)0x0020)
#define SENSOR_Port_S6                              ((uint32_t)0x0040)
#define SENSOR_Port_S7                              ((uint32_t)0x0080)
#define SENSOR_Port_All                             ((uint32_t)0x00FF)

#define SENSOR_Port_S01                             (SENSOR_Port_S0 | SENSOR_Port_S1)
#define SENSOR_Port_S23                             (SENSOR_Port_S2 | SENSOR_Port_S3)
#define SENSOR_Port_S45                             (SENSOR_Port_S4 | SENSOR_Port_S5)
#define SENSOR_Port_S67                             (SENSOR_Port_S6 | SENSOR_Port_S7)
#define IS_SENSOR_PORT(PORT)                        ((((PORT) & (uint32_t)0xFFFFFF00) == 0x00) && ((PORT) != (uint32_t)0x00))

/* 外部静态传感器脉冲上拉频率 */
#define SENSOR_PUPU_Frequency_Default               ((uint32_t)0x0000)
#define SENSOR_PUPU_Frequency_500ms                 ((uint32_t)0x0000)
#define SENSOR_PUPU_Frequency_1s                    ((uint32_t)0x0001)
#define IS_SENSOR_EXT_PUPU_FREQ(FREQ)               ((FREQ) == SENSOR_PUPU_Frequency_500ms ||\
                                                     (FREQ) == SENSOR_PUPU_Frequency_1s)

/* 外部静态传感器采样次数 */
#define SENSOR_STATIC_SAMPLE_Default                ((uint32_t)0x0001)
#define SENSOR_STATIC_SAMPLE_1                      ((uint32_t)0x0000)
#define SENSOR_STATIC_SAMPLE_2                      ((uint32_t)0x0001)
#define SENSOR_STATIC_SAMPLE_3                      ((uint32_t)0x0002)
#define SENSOR_STATIC_SAMPLE_4                      ((uint32_t)0x0003)
#define IS_SENSOR_EXT_STATIC_SAMPLE(COUNT)          ((COUNT) >= SENSOR_STATIC_SAMPLE_1 &&\
                                                     (COUNT) <= SENSOR_STATIC_SAMPLE_4)

/* 外部动态传感器采样次数 */
#define SENSOR_DYNAMIC_SAMPLE_Default                ((uint32_t)0x0002)
#define SENSOR_DYNAMIC_SAMPLE_1                      ((uint32_t)0x0000)
#define SENSOR_DYNAMIC_SAMPLE_2                      ((uint32_t)0x0001)
#define SENSOR_DYNAMIC_SAMPLE_3                      ((uint32_t)0x0002)
//#define SENSOR_DYNAMIC_SAMPLE_4                      ((uint32_t)0x0003)
#define IS_SENSOR_EXT_DYNAMIC_SAMPLE(COUNT)          ((COUNT) >= SENSOR_DYNAMIC_SAMPLE_1 &&\
                                                      (COUNT) <= SENSOR_DYNAMIC_SAMPLE_3)


/* 外部静态传感器脉冲上拉电阻，上拉有效时间 */
#define SENSOR_PUPU_HoldTime_Default                ((uint32_t)0x0002)
#define SENSOR_PUPU_HoldTime_1_2                    ((uint32_t)0x0000)
#define SENSOR_PUPU_HoldTime_1_4                    ((uint32_t)0x0001)
#define SENSOR_PUPU_HoldTime_1_8                    ((uint32_t)0x0002)
#define SENSOR_PUPU_HoldTime_1_16                   ((uint32_t)0x0003)
#define IS_SENSOR_EXT_PUPU_HOLD_TIME(TIME)          ((TIME) == SENSOR_PUPU_HoldTime_1_2 ||\
                                                     (TIME) == SENSOR_PUPU_HoldTime_1_4 ||\
                                                     (TIME) == SENSOR_PUPU_HoldTime_1_8 ||\
                                                     (TIME) == SENSOR_PUPU_HoldTime_1_16)

/* 动态传感器输出频率 */
#define SENSOR_DynamicFrequency_Default             ((uint32_t)0x0003)
#define SENSOR_DynamicFrequency_31_25ms             ((uint32_t)0x0000)
#define SENSOR_DynamicFrequency_125ms               ((uint32_t)0x0001)
#define SENSOR_DynamicFrequency_500ms               ((uint32_t)0x0002)
#define SENSOR_DynamicFrequency_1s                  ((uint32_t)0x0003)
#define IS_SENSOR_EXT_DYNAMIC_FREQ(FREQ)            ((FREQ) == SENSOR_DynamicFrequency_Default  ||\
                                                     (FREQ) == SENSOR_DynamicFrequency_31_25ms ||\
                                                     (FREQ) == SENSOR_DynamicFrequency_125ms ||\
                                                     (FREQ) == SENSOR_DynamicFrequency_500ms ||\
                                                     (FREQ) == SENSOR_DynamicFrequency_1s)


#define SENSOR_ANA_VOL_HIGH                         ((uint32_t)0x00000001)
#define SENSOR_ANA_VOL_LOW                          ((uint32_t)0x00000002)
#define SENSOR_ANA_TEMPER_HIGH                      ((uint32_t)0x00000004)
#define SENSOR_ANA_TEMPER_LOW                       ((uint32_t)0x00000008)
#define SENSOR_ANA_XTAL32K                          ((uint32_t)0x00000010)
#define SENSOR_ANA_MESH                             ((uint32_t)0x00000020)
#define SENSOR_ANA_VOLGLITCH                        ((uint32_t)0x00000040)
#define IS_SENSOR_ANA(ANA)                          ((((ANA) & (uint32_t)0xFFFFFF80) == 0x00) && ((ANA) != (uint32_t)0x00))

#define SENSOR_LOCK_EXTS                            ((uint32_t)0x00000001)
#define SENSOR_LOCK_VOL_HIGH                        ((uint32_t)0x00000002)
#define SENSOR_LOCK_VOL_LOW                         ((uint32_t)0x00000004)
#define SENSOR_LOCK_TEMPER_HIGH                     ((uint32_t)0x00000008)
#define SENSOR_LOCK_TEMPER_LOW                      ((uint32_t)0x00000010)
#define SENSOR_LOCK_VOLGLITCH                       ((uint32_t)0x00000020)
#define SENSOR_LOCK_XTAL32K                         ((uint32_t)0x00000040)
#define SENSOR_LOCK_MESH                            ((uint32_t)0x00000080)
#define IS_SENSOR_LOCK(LOCK)                        ((((LOCK) & (uint32_t)0xFFFFFF00) == 0x00) && ((LOCK) != (uint32_t)0x00))


#define SENSOR_IT_EXTS                              ((uint32_t)0x00000FFF)
#define SENSOR_IT_VOL_HIGH                          ((uint32_t)0x00001000)
#define SENSOR_IT_VOL_LOW                           ((uint32_t)0x00002000)
#define SENSOR_IT_TEMPER_HIGH                       ((uint32_t)0x00004000)
#define SENSOR_IT_TEMPER_LOW                        ((uint32_t)0x00008000)
#define SENSOR_IT_GLITCH                            ((uint32_t)0x00010000)
#define SENSOR_IT_XTAL32K                           ((uint32_t)0x00020000)
#define SENSOR_IT_MESH                              ((uint32_t)0x00040000)
#define SENSOR_IT_SSC                               ((uint32_t)0x00100000)
#define IS_SENSOR_IT(IT)                            ((IT) == SENSOR_IT_EXTS || \
                                                    (IT) == SENSOR_IT_VOL_HIGH || \
                                                    (IT) == SENSOR_IT_VOL_LOW || \
                                                    (IT) == SENSOR_IT_TEMPER_HIGH || \
                                                    (IT) == SENSOR_IT_TEMPER_LOW || \
                                                    (IT) == SENSOR_IT_GLITCH || \
                                                    (IT) == SENSOR_IT_XTAL32K || \
                                                    (IT) == SENSOR_IT_MESH || \
                                                    (IT) == SENSOR_IT_SSC)

typedef enum {
    SENSOR_CPUReset  = 0,
    SENSOR_Interrupt = 1
} SENSOR_RespModeTypeDef;
#define IS_SENSOR_RESP_MODE(Mode)                   ((Mode) == SENSOR_CPUReset ||\
                                                    (Mode) == SENSOR_Interrupt)

uint32_t SENSOR_SetRand(uint32_t Rand);
uint32_t SENSOR_EXTInit(SENSOR_EXTInitTypeDef *SENSOR_EXTInitStruct);
uint32_t SENSOR_EXTPortCmd(uint32_t SENSOR_Port, FunctionalState NewState);
uint32_t SENSOR_EXTCmd(FunctionalState NewState);
FlagStatus SENSOR_EXTIsRuning(void);
uint32_t SENSOR_ANACmd(uint32_t SENSOR_ANA, FunctionalState NewState);
void SENSOR_Lock(uint32_t SENSOR_LOCK);
void SENSOR_AttackRespMode(SENSOR_RespModeTypeDef SENSOR_RespMode);

int32_t SENSOR_GetITStatus(uint32_t ITState);
int32_t SENSOR_GetITStatusReg(void);
void SENSOR_ClearITPendingBit(void);


FunctionalState SENSOR_GetEXTTrigPull(void);
FunctionalState SENSOR_SetEXTTrigPull(FunctionalState NewState);

FunctionalState SENSOR_GetEXTFilter(void);
FunctionalState SENSOR_SetEXTFilter(FunctionalState NewState);


#define SENSOR_STATIC_PULSE_PERIOD_500MS     (0)
#define SENSOR_STATIC_PULSE_PERIOD_1S        (1)

uint32_t SENSOR_GetEXTPulsePeriod(void);
uint32_t SENSOR_SetEXTPulsePeriod(uint32_t u32Period);



#define SENSOR_STATIC_PULSE_HOLD_1_2        (0)
#define SENSOR_STATIC_PULSE_HOLD_1_4        (1)
#define SENSOR_STATIC_PULSE_HOLD_1_8        (2)
#define SENSOR_STATIC_PULSE_HOLD_1_16       (3)

uint32_t SENSOR_GetEXTPulseHold(void);
uint32_t SENSOR_SetEXTPulseHold(uint32_t u32Hold);


FunctionalState SENSOR_GetEXTPulsePull(void);
FunctionalState SENSOR_SetEXTPulsePull(FunctionalState NewState);


#define SENSOR_SAMPLE_COUNT_1        (0)
#define SENSOR_SAMPLE_COUNT_2        (1)
#define SENSOR_SAMPLE_COUNT_3        (2)
#define SENSOR_SAMPLE_COUNT_4        (3)


uint32_t SENSOR_GetEXTStaticSample(void);
uint32_t SENSOR_SetEXTStaticSample(uint32_t u32Count);
uint32_t SENSOR_GetEXTDynamicSample(void);
uint32_t SENSOR_SetEXTDynamicSample(uint32_t u32Count);


uint32_t SENSOR_GetEXTDynamicFreq(void);
uint32_t SENSOR_SetEXTDynamicFreq(uint32_t u32Freq);

#ifdef __cplusplus
}
#endif

#endif

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
