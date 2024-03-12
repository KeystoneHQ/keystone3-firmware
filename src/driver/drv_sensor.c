#include <stdio.h>
#include <string.h>
#include "drv_sensor.h"
#include "drv_bpk.h"

#define EXT_SENSOR_STATIC           (0x1)
#define EXT_SENSOR_DYNAMIC          (0x2)

#define EXT_SENSOR_PORT_MODE        EXT_SENSOR_DYNAMIC

#if (EXT_SENSOR_PORT_MODE == EXT_SENSOR_DYNAMIC)
#define EXT_SENSOR_PORT             SENSOR_Port_S01
#else
#include "mhscpu_gpio.h"
#define EXT_SENSOR_PORT             SENSOR_Port_S0
#define EXT_GPIO_PORT               GPIOD
#define EXT_GPIO_PIN                GPIO_Pin_15
#endif

static void SensorNvicConfiguration(void);
static void SensorRegPrint(char *regName, uint32_t *regAddr);
void SENSOR_Soft_Enable(void);

void SensorInit(void)
{
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_BPU, ENABLE);

    uint32_t SENSOR_ANA;
    SENSOR_EXTInitTypeDef SENSOR_EXTInitStruct;
    memset(&SENSOR_EXTInitStruct, 0, sizeof(SENSOR_EXTInitStruct));
    SENSOR_EXTInitStruct.SENSOR_Port_Dynamic = SENSOR_Port_S01 | SENSOR_Port_S23 | SENSOR_Port_S45 | SENSOR_Port_S67;
    SENSOR_EXTInitStruct.SENSOR_DynamicFrequency = SENSOR_DynamicFrequency_1s;//SENSOR_DynamicFrequency_31_25ms;̫��Ӱ���ͷ
    SENSOR_EXTInitStruct.SENSOR_Dynamic_Sample = SENSOR_DYNAMIC_SAMPLE_2;

    SENSOR_EXTInitStruct.SENSOR_GlitchEnable = ENABLE;
    SENSOR_EXTInitStruct.SENSOR_Port_Enable = ENABLE;

    SENSOR_ClearITPendingBit();

    SENSOR_ANA = SENSOR_ANA_VOL_HIGH | SENSOR_ANA_VOL_LOW | SENSOR_ANA_TEMPER_HIGH | SENSOR_ANA_TEMPER_LOW | SENSOR_ANA_MESH | SENSOR_ANA_XTAL32K | SENSOR_ANA_VOLGLITCH;

    SENSOR_EXTCmd(DISABLE);
    SENSOR_Soft_Enable();
    SENSOR->SEN_VG_DETECT &= ~0X04;

    while (SET == SENSOR_EXTIsRuning());

    SENSOR_EXTInit(&SENSOR_EXTInitStruct);
    SENSOR_AttackRespMode(SENSOR_Interrupt);

    NVIC_ClearPendingIRQ(SENSOR_IRQn);
    SENSOR_EXTCmd(DISABLE);
    SENSOR_ANACmd(SENSOR_ANA, DISABLE);

    SENSOR->SEN_ANA0 |= 0x7 << 3;
    SENSOR->SEN_ANA0 |= 0xF << 6;
    SENSOR_ANACmd(SENSOR_ANA_TEMPER_HIGH | SENSOR_ANA_TEMPER_LOW, ENABLE);
    SensorNvicConfiguration();
}

static void SensorRegPrint(char *regName, uint32_t *regAddr)
{
    printf("%s addr = %#x, val = %08X\r\n", regName, regAddr, *regAddr);
}

static void SensorNvicConfiguration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);

    NVIC_InitStructure.NVIC_IRQChannel = SENSOR_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

bool SensorTamperStatus(void)
{
    uint32_t data[BPK_KEY_LENGTH];
    uint32_t dataBak[BPK_KEY_LENGTH];
    for (int i = 0; i < BPK_KEY_LENGTH; i++) {
        data[i] = i;
    }
    ErrorStatus ret = GetBpkValue(dataBak, BPK_KEY_LENGTH, 0);
    if (ret == ERROR) {
        return false;
    }
    if (0 == memcmp(data, dataBak, BPK_KEY_LENGTH)) {
        return false;
    } else {
        return true;
    }
}

void SENSOR_IRQHandler(void)
{
    uint32_t data[BPK_KEY_LENGTH];
    SensorRegPrint("SEN_STATE", (uint32_t *)&SENSOR->SEN_STATE);
    if (((SENSOR->SEN_STATE) & 0x4000) == 0x4000) {
        for (int i = 0; i < BPK_KEY_LENGTH; i++) {
            data[i] = i;
        }
        ErrorStatus ret = SetBpkValue(data, BPK_KEY_LENGTH, 0);
        if (ret != SUCCESS) {
            SetBpkValue(data, BPK_KEY_LENGTH, 0);
        }
    }
    SENSOR->SEN_STATE = 0;
    NVIC_ClearPendingIRQ(SENSOR_IRQn);
    // todo prevent demolition
}

