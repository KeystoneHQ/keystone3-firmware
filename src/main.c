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
#include "cmd_task.h"
#include "ui_display_task.h"
#include "qrdecode_task.h"
#include "touchpad_task.h"
#include "background_task.h"
#include "fetch_sensitive_data_task.h"
#include "data_parser_task.h"
#include "log_task.h"
#include "usb_task.h"
#include "user_fatfs.h"
#include "user_sqlite3.h"
#include "screen_manager.h"
#include "keystore.h"
#include "log.h"
#include "fingerprint_process.h"
#include "fingerprint_task.h"
#include "low_power.h"
#include "draw_on_lcd.h"
#include "device_setting.h"
#include "anti_tamper.h"
#include "power_on_self_check.h"
#include "account_manager.h"
#include "version.h"
#include "hardware_version.h"
#include "librust_c.h"

bool verify_signature(char *pubkey_bytes)
{
    char content_hash[32] = {0xb3, 0x91, 0x98, 0xb0, 0xa2, 0x05, 0xc1, 0x0a, 0x6e, 0x2d, 0x91, 0x15, 0xa1, 0x29, 0x4c, 0x58, 0x0e, 0x43, 0x66, 0x69, 0x89, 0x61, 0x0e, 0xd6, 0xfb, 0x7e, 0x24, 0x55, 0x74, 0x8c, 0xeb, 0xba};
    char *signature = "85b7d4688bf2f5a48288508bb92ffeefba0148ae60ade347aa122fb46646b8d06a733754bb1dcc708f3b0e00acde861574f9e3fc5cb617ac24f13b172de15b4a";
    char signature_bytes[64] = {0};
    StrToHex(signature_bytes, signature);
    PrintArray("signature_bytes", signature_bytes, 64);
    
    if (k1_verify_signature(signature_bytes, content_hash, pubkey_bytes) == true) {
        printf("signature check ok\n");
        return true;
    } else {
        printf("signature check failed\n");
        return false;
    }
}

void check_and_update_pub_key(void)
{
    #define UPDATE_PUB_KEY_LEN 65
    char *get_pubkey_bytes = SRAM_MALLOC(UPDATE_PUB_KEY_LEN);
    int ret = GetUpdatePubKey(get_pubkey_bytes);
    printf("ret=%d\n", ret);
    PrintArray("update pub key", get_pubkey_bytes, UPDATE_PUB_KEY_LEN);
    
    if (verify_signature(get_pubkey_bytes) == true) {
        printf("signature check ok, no need to set update pub key\n");
        return;
    } else {
        printf("signature check failed, need to set update pub key\n");
    }

    char *pubkey_str = "042a070aa36d918dd83d1646e9499c9f7a8c85737b65906130daabe9f93bf216d401de66ea787cf14087156e29291d95e9b3089158a576cc7d8fe7f388c1cd1fbc";
    char *pubkey_bytes = SRAM_MALLOC(UPDATE_PUB_KEY_LEN);
    int len = StrToHex(pubkey_bytes, pubkey_str);
    if (len != UPDATE_PUB_KEY_LEN) {
        printf("set_update_pub_key err hex,len=%d\n", len);
        SRAM_FREE(pubkey_bytes);
        SRAM_FREE(get_pubkey_bytes);
        return;
    }

    if (verify_signature(pubkey_bytes) == true) {
        printf("signature check ok, updating pub key\n");
        ret = SetUpdatePubKey(pubkey_bytes);
        printf("SetUpdatePubKey ret=%d\n", ret);
    } else {
        printf("signature check failed, not updating\n");
    }
    SRAM_FREE(pubkey_bytes);
    SRAM_FREE(get_pubkey_bytes);
}

int main(void)
{
    __enable_irq();
    SetAllGpioLow();
    SystemClockInit();
    SensorInit();
    Uart0Init(CmdIsrRcvByte);
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
    LogInit();
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
    UserSqlite3Init();
    ScreenManagerInit();
    AccountManagerInit();
    PowerOnSelfCheck();

    check_and_update_pub_key();

    PrintSystemInfo();
    osKernelInitialize();
    CreateFingerprintTask();
#ifndef BUILD_PRODUCTION
    CreateCmdTask();
#endif
    CreateFetchSensitiveDataTask();
    CreateDataParserTask();
    CreateUiDisplayTask();
    CreateQrDecodeTask();
    CreateTouchPadTask();
    CreateBackgroundTask();
    CreateLogTask();
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
