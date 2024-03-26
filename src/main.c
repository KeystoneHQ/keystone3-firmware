#include <string.h>
#include <stdio.h>
#include "mhscpu.h"
#include "cm_backtrace.h"
#include "drv_sys.h"
#include "drv_uart.h"
#include "drv_spi.h"
#include "drv_psram.h"
#include "drv_trng.h"
#include "drv_ds28s60.h"
#include "drv_gd25qxx.h"
#include "drv_sensor.h"
#include "drv_rtc.h"
#include "drv_usb.h"
#include "drv_battery.h"
#include "drv_lcd_bright.h"
#include "drv_i2c_io.h"
#include "drv_atecc608b.h"
#include "drv_power.h"
#include "drv_aw32001.h"
#include "drv_button.h"
#include "drv_tamper.h"
#include "drv_exti.h"
#include "drv_motor.h"
#include "hal_lcd.h"
#include "cmsis_os.h"
#include "user_msg.h"
#include "ui_display_task.h"
#include "touchpad_task.h"
#include "background_task.h"
#include "usb_task.h"
#include "user_fatfs.h"
#include "screen_manager.h"
#include "keystore.h"

#include "fingerprint_process.h"
#include "fingerprint_task.h"
#include "low_power.h"
#include "draw_on_lcd.h"
#include "device_setting.h"
#include "anti_tamper.h"
#include "account_manager.h"
#include "version.h"
#include "hardware_version.h"


int main(void)
{
    __enable_irq();
    SetAllGpioLow();
    SystemClockInit();
    SensorInit();
    Uart0Init(NULL);
    FingerprintInit();
    cm_backtrace_init("mh1903", GetHardwareVersionString(), GetSoftwareVersionString());
    TrngInit();
    TamperInit(TamperStartup);
    PowerInit();
    LcdBrightInit();
    LcdCheck();
    LcdInit();
    DrawBootLogoOnLcd();
    Gd25FlashInit();
    NvicInit();
    PsramInit();
    DeviceSettingsInit();
    UserMsgInit();
    DS28S60_Init();
    Atecc608bInit();
    AccountsDataCheck();
    MountUsbFatfs();
    UsbInit();
    RtcInit();
    MotorInit();
    BatteryInit();
    Aw32001Init();
    ButtonInit();
    ExtInterruptInit();
    MountSdFatfs();
    ScreenManagerInit();
    AccountManagerInit();

    PrintSystemInfo();
    osKernelInitialize();

    CreateFingerprintTask();
    CreateUiDisplayTask();
    CreateTouchPadTask();
    CreateBackgroundTask();
    CreateUsbTask();

    printf("start FreeRTOS scheduler\r\n");
    osKernelStart();
    while (1);
}


int _write(int fd, char *pBuffer, int size)
{
    for (int i = 0; i < size; i++) {
        while (!UART_IsTXEmpty(UART0));
#ifdef BUILD_PRODUCTION
        // disable any log info on the production mode
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
