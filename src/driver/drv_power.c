#include "drv_power.h"
#include "mhscpu.h"
#include "stdio.h"

#define POWER_CTRL_VCC33_PORT               GPIOE
#define POWER_CTRL_VCC33_PIN                GPIO_Pin_13
#define POWER_CTRL_BLUETOOTH_PORT           GPIOF
#define POWER_CTRL_BLUETOOTH_PIN            GPIO_Pin_13


/// @brief Power ctrl init.
/// @param
void PowerInit(void)
{
    GPIO_InitTypeDef gpioInit = {0};

    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = POWER_CTRL_VCC33_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(POWER_CTRL_VCC33_PORT, &gpioInit);
    GPIO_ResetBits(POWER_CTRL_VCC33_PORT, POWER_CTRL_VCC33_PIN);

    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = POWER_CTRL_BLUETOOTH_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(POWER_CTRL_BLUETOOTH_PORT, &gpioInit);
    GPIO_ResetBits(POWER_CTRL_BLUETOOTH_PORT, POWER_CTRL_BLUETOOTH_PIN);

    OpenPower(POWER_TYPE_VCC33);
    //OpenPower(POWER_TYPE_BLUETOOTH);

    ////BT RSTN HIGH
    //gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    //gpioInit.GPIO_Pin = GPIO_Pin_10;
    //gpioInit.GPIO_Remap = GPIO_Remap_1;
    //GPIO_Init(GPIOE, &gpioInit);
    //GPIO_SetBits(GPIOE, GPIO_Pin_10);
}


/// @brief Open power.
/// @param powerType Type of power.
void OpenPower(PowerType powerType)
{
    switch (powerType) {
    case POWER_TYPE_VCC33: {
        GPIO_SetBits(POWER_CTRL_VCC33_PORT, POWER_CTRL_VCC33_PIN);
    }
    break;
    case POWER_TYPE_BLUETOOTH: {
        GPIO_SetBits(POWER_CTRL_BLUETOOTH_PORT, POWER_CTRL_BLUETOOTH_PIN);
    }
    break;
    default:
        break;
    }
}


/// @brief Close power.
/// @param powerType Type of power.
void ClosePower(PowerType powerType)
{
    switch (powerType) {
    case POWER_TYPE_VCC33: {
        GPIO_ResetBits(POWER_CTRL_VCC33_PORT, POWER_CTRL_VCC33_PIN);
    }
    break;
    case POWER_TYPE_BLUETOOTH: {
        GPIO_ResetBits(POWER_CTRL_BLUETOOTH_PORT, POWER_CTRL_BLUETOOTH_PIN);
    }
    break;
    default:
        break;
    }
}


/// @brief Power test.
/// @param
void PowerTest(int argc, char *argv[])
{
    int32_t line = 0;

    if (strcmp(argv[0], "vcc33") == 0) {
        if (strcmp(argv[1], "open") == 0) {
            OpenPower(POWER_TYPE_VCC33);
            line = __LINE__;
        }
        if (strcmp(argv[1], "close") == 0) {
            ClosePower(POWER_TYPE_VCC33);
            line = __LINE__;
        }
    } else if (strcmp(argv[0], "bluetooth") == 0) {
        if (strcmp(argv[1], "open") == 0) {
            OpenPower(POWER_TYPE_BLUETOOTH);
            line = __LINE__;
        }
        if (strcmp(argv[1], "close") == 0) {
            ClosePower(POWER_TYPE_BLUETOOTH);
            line = __LINE__;
        }
    }
    printf("power test,execute line=%d\r\n", line);
}

