#include "hardware_version.h"
#include "mhscpu.h"
#include "user_delay.h"
#include "stdio.h"

#define VER_DET_ADC_TIMES           100


static uint32_t GetVerDetAdcValue(void);


HardwareVersion GetHardwareVersion(void)
{
    static HardwareVersion version = VERSION_NONE;
    uint32_t adcValue;

    if (version == VERSION_NONE) {
        //Get the current hardware version by measuring the ver_det pin voltage.
        adcValue = GetVerDetAdcValue();
        printf("ver_det adc=%d\r\n", adcValue);
        if (adcValue < 1500) {
            version = VERSION_V3_1;//   adc : 1112
            printf("hardware version:DVT1\r\n");
        } else if (adcValue < 2000) {
            version = VERSION_V3_0;//   adc : 1874
            printf("hardware version:DVT1\r\n");
        } else if (adcValue < 2300) {
            version = VERSION_DVT2;
            printf("hardware version:DVT2\r\n");
        } else if (adcValue < 2500) {
            version = VERSION_DVT1;
            printf("hardware version:DVT1\r\n");
        } else if (adcValue < 4000) {
            version = VERSION_EVT0;
            printf("hardware version:EVT0\r\n");
        } else {
            version = VERSION_EVT1;
            printf("hardware version:EVT1\r\n");
        }
    }
    return version;
}


/// @brief Get hardware version string.
/// @return char pointer to a const zone.
char *GetHardwareVersionString(void)
{
    HardwareVersion version;

    version = GetHardwareVersion();
    if (version == VERSION_EVT0) {
        return "EVT0";
    } else if (version == VERSION_EVT1) {
        return "EVT0";
    } else if (version == VERSION_DVT1) {
        return "DVT1";
    } else if (version == VERSION_DVT2) {
        return "DVT2";
    } else if (version == VERSION_V3_0) {
        return "Version 3.0";
    } else if (version == VERSION_V3_1) {
        return "Version 3.1";
    } else {
        return "NONE";
    }
}


static uint32_t GetVerDetAdcValue(void)
{
    int32_t i, adcAver, temp, max, min;
    uint64_t adcSum = 0;
    ADC_InitTypeDef ADC_InitStruct;

    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO | SYSCTRL_APBPeriph_ADC, ENABLE);
    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_ADC, ENABLE);

    GPIO_PinRemapConfig(GPIOC, GPIO_Pin_2, GPIO_Remap_2);
    GPIO_PullUpCmd(GPIOC, GPIO_Pin_2, DISABLE);

    ADC_InitStruct.ADC_Channel = ADC_CHANNEL_2;
    ADC_InitStruct.ADC_SampSpeed = ADC_SpeedPrescaler_2;
    ADC_InitStruct.ADC_IRQ_EN = DISABLE;
    ADC_InitStruct.ADC_FIFO_EN = DISABLE;

    ADC_Init(&ADC_InitStruct);
    max = 0;
    min = 0xFFF;
    ADC_StartCmd(ENABLE);
    UserDelay(100);
    for (i = 0; i < VER_DET_ADC_TIMES; i++) {
        UserDelay(1);
        temp = ADC_GetResult();
        //printf("adc=%d\r\n", temp);
        adcSum += temp;
        if (temp > max) {
            max = temp;
        }
        if (temp < min) {
            min = temp;
        }
    }
    ADC_StartCmd(DISABLE);
    adcSum -= max;
    adcSum -= min;
    adcAver = adcSum / (VER_DET_ADC_TIMES - 2);
    return (uint32_t)adcAver;
}

