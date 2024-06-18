#include "background_task.h"
#include "background_app.h"
#include "drv_aw32001.h"
#include "drv_battery.h"
#include "drv_tamper.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#include "user_memory.h"
#include "user_msg.h"
#include "user_fatfs.h"
#include "user_delay.h"
#include "err_code.h"
#include "gui_api.h"
#include "gui_views.h"
#include "drv_usb.h"
#include "usb_task.h"
#include "anti_tamper.h"
#include "device_setting.h"
#include "drv_mpu.h"
#include "circular_buffer.h"
#include "protocol_parse.h"
#include "ecdh.h"
#include "ctaes.h"
#include "sha256.h"
#include "drv_otp.h"

#define ECC_PRV_KEY_SIZE                                24
#define ECC_PUB_KEY_SIZE                                (2 * ECC_PRV_KEY_SIZE)
#define PARSER_CACHE_LEN                                1024

static void DataParserTask(void *argument);
void USBD_cdc_SendBuffer_Cb(const uint8_t *data, uint32_t len);

static uint8_t g_dataParserCache[PARSER_CACHE_LEN] __attribute__((section(".data_parser_section")));
uint8_t g_dataParserCache[PARSER_CACHE_LEN];
osThreadId_t g_dataParserHandle;
static cbuf_handle_t g_cBufHandle;
static uint8_t g_dataParserPubKey[] = {
    0xF0, 0xBE, 0xAC, 0x34, 0x0D, 0x58, 0x2C, 0xE0,
    0xA4, 0x17, 0xB6, 0xEF, 0xC6, 0xBD, 0x1C, 0x21,
    0x43, 0x2D, 0x4D, 0xF1, 0x06, 0x00, 0x00, 0x00,
    0x9F, 0x1E, 0x84, 0xDD, 0xC4, 0x80, 0x2A, 0x37,
    0xA2, 0x6E, 0x1C, 0x6D, 0xE7, 0x96, 0xDB, 0xE0,
    0xAE, 0xEA, 0x89, 0x67, 0x03, 0x00, 0x00, 0x00
};
static uint8_t g_dataSharedKey[32] = {0};
static uint8_t g_dataShareIv[16] = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf
};

uint8_t *GetDataParserPubKey(void)
{
    return g_dataParserPubKey;
}

uint8_t *GetDeviceParserPubKey(uint8_t *webPub, uint16_t len)
{
    assert(len == 48);
    struct sha256_ctx ctx;
    uint8_t devPrv[ECC_PRV_KEY_SIZE] = {0};
    uint8_t shareKey[ECC_PUB_KEY_SIZE] = {0};
    TrngGet(devPrv, sizeof(devPrv));
    assert(ecdh_generate_keys(g_dataParserPubKey, devPrv));
    assert(ecdh_shared_secret(devPrv, webPub, shareKey));
    sha256_init(&ctx);
    sha256_update(&ctx, shareKey, sizeof(shareKey));
    sha256_done(&ctx, (struct sha256 *)g_dataSharedKey);
    memset_s(devPrv, sizeof(devPrv), 0, sizeof(devPrv));
    return g_dataParserPubKey;
}

void DataEncrypt(uint8_t *data, uint16_t len)
{
    AES256_CBC_ctx ctx;
    AES256_CBC_init(&ctx, g_dataSharedKey, g_dataShareIv);
    AES256_CBC_encrypt(&ctx, sizeof(data) / 16, data, data);
}

void CreateDataParserTask(void)
{
    const osThreadAttr_t dataParserTask_attributes = {
        .name = "data_parser_task",
        .stack_size = 1024 * 8,
        .priority = (osPriority_t)osPriorityHigh,
    };
    g_dataParserHandle = osThreadNew(DataParserTask, NULL, &dataParserTask_attributes);
}

void PushDataToField(uint8_t *data, uint16_t len)
{
    for (int i = 0; i < len; i++) {
        circular_buf_put(g_cBufHandle, data[i]);
    }
}

static uint8_t IsPrivileged(void)
{
    uint32_t control = 0;
    __asm volatile("MRS %0, control" : "=r"(control) : : "memory");
    printf("control = %d\n", control);
    return (control & 1) == 0;
}

void DataParserCacheMpuInit(void)
{
    MpuSetProtection(g_dataParserCache,
                     MPU_REGION_SIZE_1KB,
                     MPU_REGION_NUMBER0,
                     MPU_INSTRUCTION_ACCESS_DISABLE,
                     MPU_REGION_PRIV_RW,
                     MPU_ACCESS_SHAREABLE,
                     MPU_ACCESS_CACHEABLE,
                     MPU_ACCESS_BUFFERABLE);
}

static void DataParserTask(void *argument)
{
    DataParserCacheMpuInit();
    printf("g_dataParserCache = %p\n", g_dataParserCache);
    g_cBufHandle = circular_buf_init(g_dataParserCache, sizeof(g_dataParserCache));
    memset_s(g_dataParserCache, sizeof(g_dataParserCache), 0, sizeof(g_dataParserCache));
    Message_t rcvMsg;
    osStatus_t ret;
    uint8_t USB_Rx_Buffer[64 + 1] = {0};
    while (1) {
        ret = osMessageQueueGet(g_springQueue, &rcvMsg, NULL, 10000);
        if (ret != osOK) {
            continue;
        }
        switch (rcvMsg.id) {
        case SPRING_MSG_GET:
            for (int i = 0; i < rcvMsg.value; i++) {
                circular_buf_get(g_cBufHandle, &USB_Rx_Buffer[i]);
            }
            ProtocolReceivedData(USB_Rx_Buffer, rcvMsg.value, USBD_cdc_SendBuffer_Cb);
            break;
        default:
            break;
        }
        if (rcvMsg.buffer != NULL) {
            SRAM_FREE(rcvMsg.buffer);
        }
    }
}

#define SCB_CFSR (*((volatile uint32_t *)0xE000ED28))
#define SCB_HFSR (*((volatile uint32_t *)0xE000ED2C))
#define SCB_MMFAR (*((volatile uint32_t *)0xE000ED34))

void MemManage_Handler(void)
{
    uint32_t cfsr = SCB_CFSR;  // 读取CFSR寄存器
    uint32_t mmfar = SCB_MMFAR; // 读取MMFAR寄存器

    // 检查CFSR中的内存管理错误位
    if (cfsr & (1 << 0)) {
        // MMARVALID 位被设置，表示 MMFAR 包含有效的错误地址
        uint32_t fault_address = mmfar;
        
        // 在此处处理错误地址，例如记录日志、重启系统等
        printf("Memory management fault at address: 0x%08X\n", fault_address);
    }

    // 检查其他内存管理错误位，并进行相应的处理
    if (cfsr & (1 << 1)) {
        // Data Access Violation
    }
    if (cfsr & (1 << 3)) {
        // Unstacking Error
    }
    if (cfsr & (1 << 4)) {
        // Stacking Error
    }
    if (cfsr & (1 << 5)) {
        // Lazy FP State Preservation Error
    }

    *(uint32_t *)0 = 123;
    // NVIC_SystemReset();
}