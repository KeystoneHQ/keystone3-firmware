#include <string.h>
#include <stdio.h>
#include "mhscpu.h"
#include "drv_sys.h"
#include "drv_uart.h"
#include "drv_psram.h"
#include "drv_lcd_bright.h"
#include "hal_lcd.h"
#include "cmsis_os.h"
#include "helloworld_task.h"
#include "cmsis_os.h"

#define TEST_CMD_MAX_LENGTH     3072
uint8_t g_testCmdRcvBuffer[TEST_CMD_MAX_LENGTH];
uint32_t g_testCmdRcvCount = 0;

void CmdIsrRcvByte(uint8_t byte)
{
    static uint32_t lastTick = 0;
    uint32_t tick;
    static uint32_t rxF8Count = 0;

    if (osKernelGetState() < osKernelRunning) {
        return;
    }
    tick = osKernelGetTickCount();
    if (g_testCmdRcvCount != 0) {
        if (tick - lastTick > 200) {
            g_testCmdRcvCount = 0;
            rxF8Count = 0;
        }
    }
    lastTick = tick;
    if (byte == 0xF8) {
        if (rxF8Count++ > 10) {
            NVIC_SystemReset();
        }
    } else {
        rxF8Count = 0;
    }
}


int main(void)
{
    __enable_irq();
    SystemClockInit();
    Uart0Init(CmdIsrRcvByte);
    PowerInit();
    LcdBrightInit();
    LcdCheck();
    SetLcdBright(100);
    LcdInit();
    NvicInit();
    PsramInit();

    printf("Starting Hello World Application\r\n");
    
    osKernelInitialize();
    CreateHelloWorldTask();

    printf("start FreeRTOS scheduler\r\n");
    osKernelStart();
    while (1);
}

int _write(int fd, char *pBuffer, int size)
{
    for (int i = 0; i < size; i++) {
        while (!UART_IsTXEmpty(UART0));
#ifdef BUILD_PRODUCTION
        UART_SendData(UART0, '-');
#else
        UART_SendData(UART0, (uint8_t) pBuffer[i]);
#endif
    }
    return size;
}

int fputc(int ch, FILE *f)
{
    (void)(f);                              //unused arg
    while (!UART_IsTXEmpty(UART0));
    UART_SendData(UART0, (uint8_t) ch);

    return ch;
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    printf("err,file=%s,line=%d\r\n", (char *)file, line);
    while (1);
}
#endif
