/**************************************************************************************************
 * Copyright (c) keyst.one 2020-2025. All rights reserved.
 * Description: uart 用户驱动.
 * Author: leon sun
 * Create: 2022-11-8
 ************************************************************************************************/

#include "drv_uart.h"
#include "mhscpu.h"
#include "define.h"
#include "drv_sys.h"

//#define UART1_PORT_IN_FLOAT
static UartRcvByteCallbackFunc_t g_uart0RcvByteCallback;
static UartRcvByteCallbackFunc_t g_uart1RcvByteCallback;
static UartRcvByteCallbackFunc_t g_uart2RcvByteCallback;

void Uart0Init(UartRcvByteCallbackFunc_t func)
{
    UART_InitTypeDef UART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_UART0 | SYSCTRL_APBPeriph_GPIO, ENABLE);
    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_UART0, ENABLE);

    if (func != NULL) {
        g_uart0RcvByteCallback = func;
    }
    GPIO_PinRemapConfig(GPIOA, GPIO_Pin_0 | GPIO_Pin_1, GPIO_Remap_0);

    UART_InitStructure.UART_BaudRate = 512000;
    UART_InitStructure.UART_WordLength = UART_WordLength_8b;
    UART_InitStructure.UART_StopBits = UART_StopBits_1;
    UART_InitStructure.UART_Parity = UART_Parity_No;

    UART_Init(UART0, &UART_InitStructure);

    UART_ITConfig(UART0, UART_IT_RX_RECVD, ENABLE);

    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_0);

    NVIC_InitStructure.NVIC_IRQChannel = UART0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


void Uart0OpenPort(void)
{
    UART_InitTypeDef UART_InitStructure;

    GPIO_PinRemapConfig(GPIOA, GPIO_Pin_0 | GPIO_Pin_1, GPIO_Remap_0);
    UART_InitStructure.UART_BaudRate = 512000;
    UART_InitStructure.UART_WordLength = UART_WordLength_8b;
    UART_InitStructure.UART_StopBits = UART_StopBits_1;
    UART_InitStructure.UART_Parity = UART_Parity_No;
    UART_Init(UART0, &UART_InitStructure);
}


void Uart1Init(UartRcvByteCallbackFunc_t func)
{
#ifdef UART1_PORT_IN_FLOAT
    GPIO_InitTypeDef gpioInit = {0};
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO, ENABLE);
    gpioInit.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioInit.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOB, &gpioInit);
#else
    UART_InitTypeDef UART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_UART1 | SYSCTRL_APBPeriph_GPIO, ENABLE);
    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_UART1, ENABLE);

    if (func != NULL) {
        g_uart1RcvByteCallback = func;
    }
    GPIO_PinRemapConfig(GPIOB, GPIO_Pin_12 | GPIO_Pin_13, GPIO_Remap_3);

    UART_InitStructure.UART_BaudRate = 115200;
    UART_InitStructure.UART_WordLength = UART_WordLength_8b;
    UART_InitStructure.UART_StopBits = UART_StopBits_1;
    UART_InitStructure.UART_Parity = UART_Parity_No;

    UART_Init(UART1, &UART_InitStructure);

    UART_ITConfig(UART1, UART_IT_RX_RECVD, ENABLE);

    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_0);

    NVIC_InitStructure.NVIC_IRQChannel = UART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
}


void Uart1OpenPort(void)
{
    UART_InitTypeDef UART_InitStructure;

    GPIO_PinRemapConfig(GPIOB, GPIO_Pin_12 | GPIO_Pin_13, GPIO_Remap_3);
    UART_InitStructure.UART_BaudRate = 115200;
    UART_InitStructure.UART_WordLength = UART_WordLength_8b;
    UART_InitStructure.UART_StopBits = UART_StopBits_1;
    UART_InitStructure.UART_Parity = UART_Parity_No;
    UART_Init(UART1, &UART_InitStructure);
}

void Uart2DeInit(void)
{
    UART_DeInit(UART2);
    UART_ITConfig(UART2, UART_IT_RX_RECVD, DISABLE);
    GPIO_InitTypeDef gpioInit = {0};
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO, ENABLE);
    gpioInit.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioInit.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOD, &gpioInit);

    gpioInit.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOE, &gpioInit);
}

void Uart2Init(UartRcvByteCallbackFunc_t func)
{
    UART_InitTypeDef UART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_UART2 | SYSCTRL_APBPeriph_GPIO, ENABLE);
    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_UART2, ENABLE);

    if (func != NULL) {
        g_uart2RcvByteCallback = func;
    }
    GPIO_PinRemapConfig(GPIOD, GPIO_Pin_12 | GPIO_Pin_13, GPIO_Remap_0);

    UART_InitStructure.UART_BaudRate = 115200;
    UART_InitStructure.UART_WordLength = UART_WordLength_8b;
    UART_InitStructure.UART_StopBits = UART_StopBits_1;
    UART_InitStructure.UART_Parity = UART_Parity_No;

    UART_Init(UART2, &UART_InitStructure);

    UART_ITConfig(UART2, UART_IT_RX_RECVD, ENABLE);

    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_0);

    NVIC_InitStructure.NVIC_IRQChannel = UART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void Uart2OpenPort(void)
{
    UART_InitTypeDef UART_InitStructure;
    GPIO_PinRemapConfig(GPIOD, GPIO_Pin_12 | GPIO_Pin_13, GPIO_Remap_0);
    UART_InitStructure.UART_BaudRate = 115200;
    UART_InitStructure.UART_WordLength = UART_WordLength_8b;
    UART_InitStructure.UART_StopBits = UART_StopBits_1;
    UART_InitStructure.UART_Parity = UART_Parity_No;
    UART_Init(UART2, &UART_InitStructure);
}

void UART0_IRQHandler(void)
{
    volatile uint32_t iir;
    uint8_t byte;

    UART_TypeDef * UARTx = UART0;

    iir = UART_GetITIdentity(UARTx);

    switch (iir & 0x0f) {
    case UART_IT_ID_RX_RECVD: {
        byte = UART_ReceiveData(UARTx);
        g_uart0RcvByteCallback(byte);
    }
    break;
    case UART_IT_ID_TX_EMPTY: {
    }
    break;
    case UART_IT_ID_MODEM_STATUS: {
        volatile uint32_t msr = UARTx->MSR;
        UNUSED(msr);
    }
    break;
    case UART_IT_ID_LINE_STATUS: {
        volatile uint32_t lsr = UARTx->LSR;
        UNUSED(lsr);
    }
    break;
    case UART_IT_ID_BUSY_DETECT: {
        volatile uint32_t usr = UARTx->USR;
        UNUSED(usr);
    }
    break;
    case UART_IT_ID_CHAR_TIMEOUT: {
        volatile uint32_t rbr = UART_ReceiveData(UARTx);
        UNUSED(rbr);
    }
    break;
    default:
        break;
    }
}



void UART1_IRQHandler(void)
{
    volatile uint32_t iir;
    uint8_t byte;

    UART_TypeDef * UARTx = UART1;

    iir = UART_GetITIdentity(UARTx);

    switch (iir & 0x0f) {
    case UART_IT_ID_RX_RECVD: {
        byte = UART_ReceiveData(UARTx);
        g_uart1RcvByteCallback(byte);
    }
    break;
    case UART_IT_ID_TX_EMPTY: {
    }
    break;
    case UART_IT_ID_MODEM_STATUS: {
        volatile uint32_t msr = UARTx->MSR;
        UNUSED(msr);
    }
    break;
    case UART_IT_ID_LINE_STATUS: {
        volatile uint32_t lsr = UARTx->LSR;
        UNUSED(lsr);
    }
    break;
    case UART_IT_ID_BUSY_DETECT: {
        volatile uint32_t usr = UARTx->USR;
        UNUSED(usr);
    }
    break;
    case UART_IT_ID_CHAR_TIMEOUT: {
        volatile uint32_t rbr = UART_ReceiveData(UARTx);
        UNUSED(rbr);
    }
    break;
    default:
        break;
    }
}

void UART2_IRQHandler(void)
{
    volatile uint32_t iir;
    uint8_t byte;

    UART_TypeDef * UARTx = UART2;

    iir = UART_GetITIdentity(UARTx);

    switch (iir & 0x0f) {
    case UART_IT_ID_RX_RECVD: {
        byte = UART_ReceiveData(UARTx);
        g_uart2RcvByteCallback(byte);
    }
    break;
    case UART_IT_ID_TX_EMPTY: {
    }
    break;
    case UART_IT_ID_MODEM_STATUS: {
        volatile uint32_t msr = UARTx->MSR;
        UNUSED(msr);
    }
    break;
    case UART_IT_ID_LINE_STATUS: {
        volatile uint32_t lsr = UARTx->LSR;
        UNUSED(lsr);
    }
    break;
    case UART_IT_ID_BUSY_DETECT: {
        volatile uint32_t usr = UARTx->USR;
        UNUSED(usr);
    }
    break;
    case UART_IT_ID_CHAR_TIMEOUT: {
        volatile uint32_t rbr = UART_ReceiveData(UARTx);
        UNUSED(rbr);
    }
    break;
    default:
        break;
    }
}
