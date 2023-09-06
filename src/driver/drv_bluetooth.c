/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Bluetooth driver.
 * Author: leon sun
 * Create: 2023-5-18
 ************************************************************************************************/


#include "drv_bluetooth.h"
#include "stdio.h"
#include "string.h"
#include "mhscpu.h"
#include "drv_uart.h"
#include "user_utils.h"
#include "cmsis_os.h"

#define BLUETOOTH_MAX_LENGTH     512

static void BluetoothByteIsrHandler(uint8_t byte);


uint8_t g_bluetoothRcvBuffer[BLUETOOTH_MAX_LENGTH];
uint32_t g_bluetoothRcvCount = 0;


void BluetoothInit(void)
{
    //Uart1Init(BluetoothByteIsrHandler);
}


void BluetoothSend(const char *str)
{
    uint32_t len;
    uint32_t i;
    char end[] = "\r\n";

    len = strlen(str);
    for (i = 0; i < len; i++) {
        while (!UART_IsTXEmpty(UART1));
        UART_SendData(UART1, (uint8_t) str[i]);
    }
    len = strlen(end);
    for (i = 0; i < len; i++) {
        while (!UART_IsTXEmpty(UART1));
        UART_SendData(UART1, (uint8_t) end[i]);
    }
}


void BluetoothTest(int argc, char *argv[])
{
    if (strcmp(argv[0], "send_string") == 0) {
        VALUE_CHECK(argc, 2);
        BluetoothSend(argv[1]);
        printf("bluetooth send:%s\r\n", argv[1]);
    } else {
        printf("bluetooth test input err\r\n");
    }
}



static void BluetoothByteIsrHandler(uint8_t byte)
{
    static uint32_t lastTick = 0;
    uint32_t tick;

    if (osKernelGetState() < osKernelRunning) {
        return;
    }
    tick = osKernelGetTickCount();
    if (g_bluetoothRcvCount != 0) {
        if (tick - lastTick > 200) {
            g_bluetoothRcvCount = 0;
        }
    }
    lastTick = tick;

    if (g_bluetoothRcvCount == 0) {
        if (byte != '\r' && byte != '\n') {
            g_bluetoothRcvBuffer[g_bluetoothRcvCount] = byte;
            g_bluetoothRcvCount++;
        }
    } else if (byte == '\n') {
        g_bluetoothRcvBuffer[g_bluetoothRcvCount] = byte;
        g_bluetoothRcvBuffer[g_bluetoothRcvCount + 1] = 0;
        //PubValueMsg(MSG_BLUETOOTH_FRAME, g_bluetoothRcvCount + 2);
        g_bluetoothRcvCount = 0;
        printf("bt rcv:%s\r\n", g_bluetoothRcvBuffer);
    } else if (g_bluetoothRcvCount >= BLUETOOTH_MAX_LENGTH - 1) {
        g_bluetoothRcvCount = 0;
    } else {
        g_bluetoothRcvBuffer[g_bluetoothRcvCount] = byte;
        g_bluetoothRcvCount++;
    }
}




