#include "drv_exti.h"
#include "hal_touch.h"
#include "drv_aw32001.h"
#include "drv_button.h"
#include "anti_tamper.h"
#include "drv_sdcard.h"

void ExtInterruptInit(void)
{
    GPIO_InitTypeDef gpioInit = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    //PA:
    //PA2:touch pad interrupt.
    gpioInit.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioInit.GPIO_Pin = GPIO_Pin_2;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOA, &gpioInit);
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    EXTI_LineConfig(EXTI_Line0, EXTI_PinSource2, EXTI_Trigger_Rising_Falling);

    //PD7:SD card interrupt.
    gpioInit.GPIO_Mode = GPIO_Mode_IPU;
    gpioInit.GPIO_Pin = GPIO_Pin_7;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOD, &gpioInit);
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    EXTI_LineConfig(EXTI_Line3, EXTI_PinSource7, EXTI_Trigger_Rising_Falling);

    //PE:
    //PE14:button interrupt.
    gpioInit.GPIO_Mode = GPIO_Mode_IPU;
    gpioInit.GPIO_Pin = GPIO_Pin_14;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOE, &gpioInit);
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    EXTI_LineConfig(EXTI_Line4, EXTI_PinSource14, EXTI_Trigger_Falling);

    //PF:
    //PF1:touch pad interrupt / PF14:fingerprint interrupt / PF15:USB wakeup interrupt.
    gpioInit.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioInit.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_15;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOF, &gpioInit);
    gpioInit.GPIO_Mode = GPIO_Mode_IPU;
    // gpioInit.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioInit.GPIO_Pin = GPIO_Pin_14;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOF, &gpioInit);
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    EXTI_LineConfig(EXTI_Line5, EXTI_PinSource1, EXTI_Trigger_Falling);
    EXTI_LineConfig(EXTI_Line5, EXTI_PinSource14, EXTI_Trigger_Falling);
    EXTI_LineConfig(EXTI_Line5, EXTI_PinSource15, EXTI_Trigger_Rising_Falling);
}


//PA ext interrupt handler
void EXTI0_IRQHandler(void)
{
    uint32_t pinSource = EXTI_GetITLineStatus(EXTI_Line0);
    switch (pinSource) {
    case EXTI_PinSource2: {
        TamperIntHandler();
    }
    break;
    default:
        break;
    }
    EXTI_ClearITPendingBit(EXTI_Line0);
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
}


//PD ext interrupt handler
void EXTI3_IRQHandler(void)
{
    uint32_t pinSource = EXTI_GetITLineStatus(EXTI_Line3);
    switch (pinSource) {
    case EXTI_PinSource7: {
        SdCardIntHandler();
    }
    break;
    default:
        break;
    }
    EXTI_ClearITPendingBit(EXTI_Line3);
    NVIC_ClearPendingIRQ(EXTI3_IRQn);
}


//PE ext interrupt handler
void EXTI4_IRQHandler(void)
{
    uint32_t pinSource = EXTI_GetITLineStatus(EXTI_Line4);
    switch (pinSource) {
    case EXTI_PinSource14: {
        ButtonIntHandler();
    }
    break;
    default:
        break;
    }
    EXTI_ClearITPendingBit(EXTI_Line4);
    NVIC_ClearPendingIRQ(EXTI4_IRQn);
}


//PF ext interrupt handler
void EXTI5_IRQHandler(void)
{
    uint32_t pinSource = EXTI_GetITLineStatus(EXTI_Line5);
    switch (pinSource) {
    case EXTI_PinSource1: {
        TouchPadIntHandler();
    }
    break;
    case EXTI_PinSource15: {
        ChangerInsertHandler();
    }
    break;
    default:
        break;
    }
    EXTI_ClearITPendingBit(EXTI_Line5);
    NVIC_ClearPendingIRQ(EXTI5_IRQn);
}




