/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : mhscpu_adc.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 05/28/2017
 * Description          : ADC headfile.
 *****************************************************************************/


#ifndef __MHSCPU_ADC_H__
#define __MHSCPU_ADC_H__


#ifdef __cplusplus
extern "C" {
#endif

/* Include ------------------------------------------------------------------*/
#include "mhscpu.h"

#define ADC_CR1_CHANNEL_MASK           ((uint32_t)0x07)
#define ADC_CR1_SAMPLE_RATE_Pos         3
#define ADC_CR1_SAMPLE_RATE_MASK       (0x3 << ADC_CR1_SAMPLE_RATE_Pos)
#define ADC_CR1_SAMP_ENABLE             BIT(6)
#define ADC_CR1_POWER_DOWN              BIT(8)
#define ADC_CR1_IRQ_ENABLE              BIT(5)

#define ADC_SR_DONE_FLAG                BIT(0)
#define ADC_SR_FIFO_OV_FLAG             BIT(1)

#define ADC_FIFO_OV_INT_ENABLE          BIT(2)
#define ADC_FIFO_RESET                  BIT(1)

#define ADC_FIFO_ENABLE                 BIT(0)

#define ADC_CR2_BUFF_ENABLE             BIT(14)


/* ADC Channel select */
typedef enum {
    ADC_CHANNEL_0 = 0,
    ADC_CHANNEL_1,
    ADC_CHANNEL_2,
    ADC_CHANNEL_3,
    ADC_CHANNEL_4,
    ADC_CHANNEL_5,
    ADC_CHANNEL_VDD
} ADC_ChxTypeDef;

#define IS_ADC_CHANNEL(CHANNEL_NUM) (((CHANNEL_NUM) == ADC_CHANNEL_0) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_1) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_2) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_3) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_4) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_5) || \
                                     ((CHANNEL_NUM) == ADC_CHANNEL_VDD))

/* ADC Samp Select */
typedef enum {
    ADC_SpeedPrescaler_None = 0,
    ADC_SpeedPrescaler_2,
    ADC_SpeedPrescaler_4,
    ADC_SpeedPrescaler_8,
} ADC_SampTypeDef;

#define IS_ADC_SAMP(SAMP)           (((SAMP) == ADC_SpeedPrescaler_None)  || \
                                     ((SAMP) == ADC_SpeedPrescaler_2) || \
                                     ((SAMP) == ADC_SpeedPrescaler_4) || \
                                     ((SAMP) == ADC_SpeedPrescaler_8))


typedef struct _ADC_InitTypeDef {
    ADC_ChxTypeDef              ADC_Channel;            /* ADC Channel select */
    ADC_SampTypeDef             ADC_SampSpeed;          /* ADC sampspeed select */
    FunctionalState             ADC_IRQ_EN;             /* ADC IRQ/Polling Select */
    FunctionalState             ADC_FIFO_EN;            /* ADC FIFO Enable Select */
} ADC_InitTypeDef;

/* Exported constants -------------------------------------------------------*/
/* Exported macro -----------------------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
void ADC_Init(ADC_InitTypeDef *ADC_InitStruct);
void ADC_StartCmd(FunctionalState NewState);
void ADC_FIFODeepth(uint32_t FIFO_Deepth);
void ADC_FIFOReset(void);
void ADC_ITCmd(FunctionalState NewState);
void ADC_FIFOOverflowITcmd(FunctionalState NewState);
int32_t ADC_GetFIFOCount(void);
int32_t ADC_GetResult(void);
int32_t ADC_GetFIFOResult(uint16_t *ADCdata, uint32_t len);
uint32_t ADC_CalVoltage(uint32_t u32ADC_Value, uint32_t u32ADC_Ref_Value);
void ADC_BuffCmd(FunctionalState NewState);
void ADC_ChannelSwitch(ADC_ChxTypeDef Channelx);

/* Exported variables -------------------------------------------------------*/

/* Exported types -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif  /* __MHSCPU_ADC_H__ */
/************************ (C) COPYRIGHT 2015 Megahuntmicro ****END OF FILE****/
