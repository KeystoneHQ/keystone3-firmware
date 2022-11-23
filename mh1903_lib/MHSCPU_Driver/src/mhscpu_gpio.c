/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_gpio.c
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file provides all the GPIO firmware functions
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "mhscpu_gpio.h"


/* Private variables --------------------------------------------------------*/
static volatile uint16_t GPIO_OD_Mark[GPIO_GROUP_NUM] = {0};

/* Private function prototypes ----------------------------------------------*/
/**
  * @brief  Get GPIO Number.
  * @param  GPIOx: where x can be (A..H) to select the GPIO peripheral.
  * @retval GPIO Number
  */
static int32_t GPIO_GetGPIONum(GPIO_TypeDef *GPIOx)
{
    int32_t i;
    GPIO_TypeDef *GPIOArray[GPIO_GROUP_NUM] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH};

    assert_param(IS_GPIO_PERIPH(GPIOx));

    for (i = 0; i < GPIO_GROUP_NUM; i++) {
        if (GPIOArray[i] == GPIOx) {
            return i;
        }
    }

    return -1;
}

/* Exported functions -------------------------------------------------------*/
/**
  * @brief  Deinitializes the GPIOx peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void GPIO_DeInit(void)
{
    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_GPIO, ENABLE);
}

/**
  * @brief  Initializes the GPIOx peripheral according to the specified
  *         parameters in the GPIO_InitStruct.
  * @param  GPIOx: where x can be (A..H) to select the GPIO peripheral.
  * @param  GPIO_InitStruct: pointer to a GPIO_InitTypeDef structure that
  *         contains the configuration information for the specified GPIO peripheral.
  * @retval None
  */
void GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_InitStruct)
{
    int32_t GPIONum = -1;
    assert_param(IS_GPIO_PERIPH(GPIOx));
    assert_param(IS_GPIO_PIN(GPIO_InitStruct->GPIO_Pin));
    assert_param(IS_GPIO_MODE(GPIO_InitStruct->GPIO_Mode));

    GPIO_PinRemapConfig(GPIOx, GPIO_InitStruct->GPIO_Pin, GPIO_InitStruct->GPIO_Remap);

    if (GPIO_Remap_1 != GPIO_InitStruct->GPIO_Remap) {
        return;
    }
    GPIONum = GPIO_GetGPIONum(GPIOx);
    switch ((uint32_t)GPIO_InitStruct->GPIO_Mode) {
    case GPIO_Mode_IN_FLOATING: {
        GPIOx->OEN |= GPIO_InitStruct->GPIO_Pin;        //Input Mode
        GPIOx->PUE &= ~GPIO_InitStruct->GPIO_Pin;       //Pue Disable
        break;
    }
    case GPIO_Mode_IPU: {
        GPIOx->OEN |= GPIO_InitStruct->GPIO_Pin;        //Input Mode
        GPIOx->PUE |= GPIO_InitStruct->GPIO_Pin;        //PUE Enable
        break;
    }
    case GPIO_Mode_Out_OD: {
        /* In OD Mode,Using OEN Control GPIO*/
        GPIOx->OEN |= GPIO_InitStruct->GPIO_Pin;       //Set Init Status
        GPIOx->PUE &= ~GPIO_InitStruct->GPIO_Pin;       //PUE Disable
        GPIOx->IODR &= ~GPIO_InitStruct->GPIO_Pin;
        GPIO_OD_Mark[GPIONum] |= GPIO_InitStruct->GPIO_Pin;
        break;
    }
    case GPIO_Mode_Out_OD_PU: {
        /* In OD Mode,Using OEN Control GPIO*/
        GPIOx->OEN |= GPIO_InitStruct->GPIO_Pin;        //Set Init Status
        GPIOx->PUE |= GPIO_InitStruct->GPIO_Pin;        //PUE Enable
        GPIOx->IODR &= ~GPIO_InitStruct->GPIO_Pin;
        GPIO_OD_Mark[GPIONum] |= GPIO_InitStruct->GPIO_Pin;
        break;
    }
    case GPIO_Mode_Out_PP: {
        GPIOx->OEN &= ~GPIO_InitStruct->GPIO_Pin;       //OutPut Mode
        GPIOx->PUE &= ~GPIO_InitStruct->GPIO_Pin;       //Do not use PUE in push pull mode, it may cause power consumption
        GPIOx->IODR |= GPIO_InitStruct->GPIO_Pin;       //Set Init Status
        GPIO_OD_Mark[GPIONum] &= ~GPIO_InitStruct->GPIO_Pin;
        break;
    }
    }
}


/**
  * @brief  Fills each GPIO_InitStruct member with its default value.
  * @param  GPIO_InitStruct : pointer to a GPIO_InitTypeDef structure which will
  *         be initialized.
  * @retval None
  */
void GPIO_StructInit(GPIO_InitTypeDef* GPIO_InitStruct)
{
    /* Reset GPIO init structure parameters values */
    GPIO_InitStruct->GPIO_Pin  = GPIO_Pin_All;
    GPIO_InitStruct->GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct->GPIO_Remap = GPIO_Remap_0;
}

/**
  * @brief  Reads the specified input port pin.
  * @param  GPIOx: where x can be (A..H) to select the GPIO peripheral.
  * @param  GPIO_Pin:  specifies the port bit to read.
  *   This parameter can be GPIO_Pin_x where x can be (0..15).
  * @retval The input port pin value.
  */
uint8_t GPIO_ReadInputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    /* Check the parameters */
    assert_param(IS_GPIO_PERIPH(GPIOx));
    assert_param(IS_GET_GPIO_PIN(GPIO_Pin));

    if ((((GPIOx->IODR >> 16) & 0xFFFF) & GPIO_Pin) != (uint32_t)Bit_RESET) {
        return (uint8_t)Bit_SET;
    }

    return (uint8_t)Bit_RESET;
}

/**
  * @brief  Reads the specified GPIO input data port.
  * @param  GPIOx: where x can be (A..H) to select the GPIO peripheral.
  * @retval GPIO input data port value.
  */
uint16_t GPIO_ReadInputData(GPIO_TypeDef* GPIOx)
{
    return ((GPIOx->IODR >> 16) & 0xFFFF);
}

/**
  * @brief  Reads the specified output data port bit.
  * @param  GPIOx: where x can be (A..H) to select the GPIO peripheral.
  * @param  GPIO_Pin:  specifies the port bit to read.
  *   This parameter can be GPIO_Pin_x where x can be (0..15).
  * @retval The output port pin value.
  */
uint8_t GPIO_ReadOutputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    /* Check the parameters */
    assert_param(IS_GPIO_PERIPH(GPIOx));
    assert_param(IS_GET_GPIO_PIN(GPIO_Pin));

    if (((GPIOx->IODR & 0xFFFF) & GPIO_Pin) != (uint32_t)Bit_RESET) {
        return (uint8_t)Bit_SET;
    }

    return (uint8_t)Bit_RESET;
}

/**
  * @brief  Reads the specified GPIO output data port.
  * @param  GPIOx: where x can be (A..H) to select the GPIO peripheral.
  * @retval GPIO output data port value.
  */
uint16_t GPIO_ReadOutputData(GPIO_TypeDef* GPIOx)
{
    return (GPIOx->IODR & 0xFFFF);
}

/**
  * @brief  Sets the selected data port bits.
  * @param  GPIOx: where x can be (A..H) to select the GPIO peripheral.
  * @param  GPIO_Pin: specifies the port bits to be written.
  *   This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval None
  */
void GPIO_SetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    int32_t GPIONum = -1;
    uint16_t tmpOEN = 0;
    uint16_t tmpBSRR = 0;
    assert_param(IS_GPIO_PERIPH(GPIOx));
    assert_param(IS_GET_GPIO_PIN(GPIO_Pin));

    GPIONum = GPIO_GetGPIONum(GPIOx);

    tmpOEN = GPIO_Pin & GPIO_OD_Mark[GPIONum];
    tmpBSRR = GPIO_Pin & (~GPIO_OD_Mark[GPIONum]);

    if (GPIO_OD_Mark[GPIONum] & GPIO_Pin) {
        GPIOx->OEN |= ((uint32_t)tmpOEN);
    } else {
        GPIOx->BSRR = ((uint32_t)tmpBSRR);
    }
}

/**
  * @brief  Clears the selected data port bits.
  * @param  GPIOx: where x can be (A..H) to select the GPIO peripheral.
  * @param  GPIO_Pin: specifies the port bits to be written.
  *   This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval None
  */
void GPIO_ResetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    int32_t GPIONum = -1;
    uint32_t tmpOEN = 0;
    uint32_t tmpBSRR = 0;
    assert_param(IS_GPIO_PERIPH(GPIOx));
    assert_param(IS_GET_GPIO_PIN(GPIO_Pin));

    GPIONum = GPIO_GetGPIONum(GPIOx);

    tmpOEN = GPIO_Pin & GPIO_OD_Mark[GPIONum];
    tmpBSRR = GPIO_Pin & (~GPIO_OD_Mark[GPIONum]);

    if (GPIO_OD_Mark[GPIONum] & GPIO_Pin) {
        GPIOx->OEN &= ~((uint32_t)tmpOEN);
    } else {
        GPIOx->BSRR = (((uint32_t)tmpBSRR) << 16);
    }
}

/**
  * @brief  Writes data to the specified GPIO data port.
  * @param  GPIOx: where x can be (A..H) to select the GPIO peripheral.
  * @param  PortVal: specifies the value to be written to the port output data register.
  * @retval None
  */
void GPIO_Write(GPIO_TypeDef *GPIOx, uint16_t PortVal)
{
    uint32_t i = 0;
    int32_t GPIONum = -1;
    assert_param(IS_GPIO_PERIPH(GPIOx));

    GPIONum = GPIO_GetGPIONum(GPIOx);

    for (i = 0; i < 16; i++) {
        if (GPIO_OD_Mark[GPIONum] & (0x01U << i)) {
            if (PortVal & (0x01U << i)) {
                GPIOx->OEN |= (0x01U << i);
            } else {
                GPIOx->OEN &= ~(0x01U << i);
            }
        } else {
            if (PortVal & (0x01U << i)) {
                GPIOx->IODR |= (0x01U << i);
            } else {
                GPIOx->IODR &= ~(0x01U << i);
            }
        }
    }
}

/**
  * @brief  Enable or disable the selected data port pullup resistance
  * @param  GPIOx: where x can be (A..H) to select the GPIO peripheral.
  * @param  GPIO_Pin: specifies the port bit to be written.
  * @param  NewState: new state of the port pin Pull Up.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval
  */
void GPIO_PullUpCmd(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, FunctionalState NewState)
{
    assert_param(IS_GPIO_PERIPH(GPIOx));
    assert_param(IS_GPIO_PIN(GPIO_Pin));

    if (DISABLE != NewState) {
        GPIOx->PUE |= GPIO_Pin;
    } else {
        GPIOx->PUE &= ~GPIO_Pin;
    }
}

/**
  * @brief  Changes the mapping of the specified pin.
  * @param  GPIO_PortSource: selects the GPIO port to be used as source for EXTI lines.
  *         This parameter can be GPIO_PortSourceGPIOx where x can be (A..H).
  * @param  GPIO_Pin: specifies the port bit to be written.
  *         This parameter can be one of GPIO_Pin_x where x can be (0..15).
  * @param  GPIO_Remap: selects the pin to remap.
  *         This parameter can be one of the following values:
  *           @arg GPIO_Remap_0
  *           @arg GPIO_Remap_1
  *           @arg GPIO_Remap_2
  *           @arg GPIO_Remap_3
  * @retval None
  */
void GPIO_PinRemapConfig(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_RemapTypeDef GPIO_Remap)
{
    uint32_t i;
    int32_t GPIONum;
    assert_param(IS_GPIO_PERIPH(GPIOx));
    assert_param(IS_GPIO_PIN(GPIO_Pin));
    assert_param(IS_GET_GPIO_REMAP(GPIO_Remap));

    GPIONum = GPIO_GetGPIONum(GPIOx);

    for (i = 0; i < 16; i++) {
        if (GPIO_Pin & (0x01U << i)) {
            switch ((uint32_t)GPIO_Remap) {
            case GPIO_Remap_0:
                GPIO->ALT[GPIONum] &= ~(0x03U << (i * 2));
                break;
            case GPIO_Remap_1:
                GPIO->ALT[GPIONum] = (GPIO->ALT[GPIONum] & ~(0x03U << (i * 2))) | (0x01U << (i * 2));
                break;
            case GPIO_Remap_2:
                GPIO->ALT[GPIONum] = (GPIO->ALT[GPIONum] & ~(0x03U << (i * 2))) | (0x02U << (i * 2));
                break;
            case GPIO_Remap_3:
                GPIO->ALT[GPIONum] = (GPIO->ALT[GPIONum] & ~(0x03U << (i * 2))) | (0x03U << (i * 2));
                break;
            }
        }
    }
}

/**
  * @brief  Initialize the WAKE_EVEN_TYPE_EN registers to their default values.
  * @param  None
  * @retval None
  */
void GPIO_WakeEvenDeInit(void)
{
    GPIO->WAKE_TYPE_EN = 0x00007801;
}

/**
  * @brief  Selects the GPIO pin used as WAKE EVEN Line.
  * @param  GPIO_PortSource: selects the GPIO port to be used as source for WEAK EVEN lines.
  *         This parameter can be GPIO_PortSourceGPIOx where x can be (A..H).
  * @param  GPIO_Pin: specifies the port bit to be written.
  * @retval None
  */
void GPIO_WakeEvenConfig(uint32_t GPIO_PortSource, uint32_t GPIO_Pin, FunctionalState NewState)
{
    assert_param(IS_GPIO_PORT_SOURCE(GPIO_PortSource));
    assert_param(IS_GPIO_PIN(GPIO_Pin));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (DISABLE == NewState) {
        switch (GPIO_PortSource) {
        case GPIO_PortSourceGPIOA:
            GPIO->WAKE_P0_EN &= ~GPIO_Pin;
            break;

        case GPIO_PortSourceGPIOB:
            GPIO->WAKE_P0_EN &= ~(GPIO_Pin << 16);
            break;

        case GPIO_PortSourceGPIOC:
            GPIO->WAKE_P1_EN &= ~GPIO_Pin;
            break;

        case GPIO_PortSourceGPIOD:
            GPIO->WAKE_P1_EN &= ~(GPIO_Pin << 16);
            break;

        case GPIO_PortSourceGPIOE:
            GPIO->WAKE_P2_EN &= ~GPIO_Pin;
            break;

        case GPIO_PortSourceGPIOF:
            GPIO->WAKE_P2_EN &= ~(GPIO_Pin << 16);
            break;

        case GPIO_PortSourceGPIOG:
            GPIO->WAKE_P3_EN &= ~GPIO_Pin;
            break;

        case GPIO_PortSourceGPIOH:
            GPIO->WAKE_P3_EN &= ~(GPIO_Pin << 16);
            break;
        }
    } else {
        switch (GPIO_PortSource) {
        case GPIO_PortSourceGPIOA:
            GPIO->WAKE_P0_EN |= GPIO_Pin;
            break;

        case GPIO_PortSourceGPIOB:
            GPIO->WAKE_P0_EN |= (GPIO_Pin << 16);
            break;

        case GPIO_PortSourceGPIOC:
            GPIO->WAKE_P1_EN |= GPIO_Pin;
            break;

        case GPIO_PortSourceGPIOD:
            GPIO->WAKE_P1_EN |= (GPIO_Pin << 16);
            break;

        case GPIO_PortSourceGPIOE:
            GPIO->WAKE_P2_EN |= GPIO_Pin;
            break;

        case GPIO_PortSourceGPIOF:
            GPIO->WAKE_P2_EN |= (GPIO_Pin << 16);
            break;

        case GPIO_PortSourceGPIOG:
            GPIO->WAKE_P3_EN |= GPIO_Pin;
            break;

        case GPIO_PortSourceGPIOH:
            GPIO->WAKE_P3_EN |= (GPIO_Pin << 16);
            break;
        }
    }
}

/**
  * @brief  Selects the GPIO WakeUp type.
  * @param  GPIO_WakeMode: mode of the gpio wakeup source.
  *   This parameter can be: GPIO_WakeMode_Now or GPIO_WakeMode_AfterGlitch.
  * @retval None
  */
void GPIO_WakeModeConfig(GPIO_WakeModeTypeDef GPIO_WakeMode)
{
    assert_param(IS_GPIO_WAKE_MODE(GPIO_WakeMode));

    GPIO->WAKE_TYPE_EN = (GPIO->WAKE_TYPE_EN & ~0x1U) | GPIO_WakeMode;
}

/**************************      (C) COPYRIGHT Megahunt    *****END OF FILE****/
