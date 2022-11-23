/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : mhscpu_dac.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 05/28/2017
 * Description          : DAC headfile.
 *****************************************************************************/


#ifndef __MHSCPU_DAC_H__
#define __MHSCPU_DAC_H__


#ifdef __cplusplus
extern "C" {
#endif

/* Include ------------------------------------------------------------------*/
#include "mhscpu.h"


#define DAC_CR1_IS_RUNNING              ((uint32_t)0x20000000)
#define DAC_CR1_POWER_DOWN              ((uint32_t)0x00000010)
#define DAC_CR1_FIFO_RESET              ((uint32_t)0x00000008)
#define DAC_CR1_DMA_ENABLE              ((uint32_t)0x00000004)


#define IS_DAC_DATA(DATA)               ((DATA) <= 0x3FF)


#define DAC_CURR_SEL_MASK               ((uint32_t)0x00000020)
#define DAC_CURR_SEL_20K                ((uint32_t)0x00000000)
#define DAC_CURR_SEL_2K                 ((uint32_t)0x00000020)
#define IS_DAC_CURR_SEL(CURR)           (((CURR) == DAC_CURR_SEL_20K) || \
                                         ((CURR) == DAC_CURR_SEL_2K))

#define IS_DAC_FIFO_THR(THR)            ((THR) <= 0xF)


typedef struct _DAC_InitTypeDef {
    uint32_t DAC_CurrSel;            /* DAC mode select */
    uint32_t DAC_TimerExp;           /* DAC timer expectation */
    uint32_t DAC_FIFOThr;            /* DAC FIFO Threshold */
} DAC_InitTypeDef;


#define DAC_FIFO_DEPTH                  (16)
#define DAC_IT_STATUS_SHIFT             (30)

#define DAC_IT_FIFO_THR                 ((uint32_t)0x0002)
#define DAC_IT_FIFO_OVERFLOW            ((uint32_t)0x0001)
#define IS_DAC_IT(IT)                   (((IT) == DAC_IT_FIFO_THR) || \
                                         ((IT) == DAC_IT_FIFO_OVERFLOW))

#define DAC_FLAG_RUNNING                (DAC_CR1_IS_RUNNING)
#define DAC_FLAG_OVERFLOW               ((uint32_t)0x40000000)
#define IS_DAC_FLAG(FLAG)               (((FLAG) == DAC_FLAG_RUNNING) || \
                                         ((FLAG) == DAC_FLAG_OVERFLOW))


/* Exported functions -------------------------------------------------------*/
void DAC_Init(DAC_InitTypeDef *DAC_InitStruct);
void DAC_StructInit(DAC_InitTypeDef *DAC_InitStruct);
void DAC_FIFOReset(void);

void DAC_Cmd(FunctionalState NewState);
void DAC_DMACmd(FunctionalState NewState);

void DAC_SetData(uint16_t Data);

FlagStatus DAC_GetFlagStatus(uint32_t DAC_Flag);
void DAC_ClearFlag(uint32_t DAC_Flag);

void DAC_ITConfig(uint32_t DAC_IT, FunctionalState NewState);
ITStatus DAC_GetITStatus(uint32_t DAC_IT);
void DAC_ClearITPendingBit(uint32_t DAC_IT);


#ifdef __cplusplus
}
#endif

#endif  /* __MHSCPU_DAC_H__ */

/************************ (C) COPYRIGHT 2015 Megahuntmicro ****END OF FILE****/
