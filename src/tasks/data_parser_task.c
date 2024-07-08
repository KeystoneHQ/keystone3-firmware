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
#include "drv_otp.h"
#include "librust_c.h"

#define ECC_PRV_KEY_SIZE                                24
#define ECC_PUB_KEY_SIZE                                (2 * ECC_PRV_KEY_SIZE)
#define PARSER_CACHE_LEN                                1024
#define PRIV_KEY_SIZE                                   32
#define PUB_KEY_SIZE                                    33

static void DataParserTask(void *argument);
void USBD_cdc_SendBuffer_Cb(const uint8_t *data, uint32_t len);

static uint8_t g_dataParserCache[PARSER_CACHE_LEN] __attribute__((section(".data_parser_section")));
static cbuf_handle_t g_cBufHandle;
static uint8_t g_dataParserPubKey[PUB_KEY_SIZE] = {0};
static uint8_t g_dataSharedKey[PRIV_KEY_SIZE] = {0};
static uint8_t g_dataParserIv[16] = {0};
static osThreadId_t g_dataParserHandle;

void SetDeviceParserIv(uint8_t *iv)
{
    memcpy_s(g_dataParserIv, sizeof(g_dataParserIv), iv, 16);
}

uint8_t *GetDeviceParserPubKey(uint8_t *webPub, uint16_t len)
{
    assert(len == 33);
    uint8_t privKey[32] = {36, 152, 38, 220, 181, 219, 183, 145, 246, 234, 111, 76, 161, 118, 67, 239, 70, 95, 241, 130, 17, 82, 24, 232, 53, 216, 250, 63, 93, 81, 164, 129};
    uint8_t shareKey[PUB_KEY_SIZE] = {3, 161, 210, 17, 253, 221, 40, 129, 137, 80, 105, 46, 126, 230, 80, 221, 82, 51, 206, 214, 24, 144, 201, 11, 99, 132, 251, 21, 68, 169, 44, 42, 251};
    uint8_t shareKey2[PUB_KEY_SIZE] = {0};
    uint8_t webPubKey[] = {3, 161, 210, 17, 253, 221, 40, 129, 137, 80, 105, 46, 126, 230, 80, 221, 82, 51, 206, 214, 24, 144, 201, 11, 99, 132, 251, 21, 68, 169, 44, 42, 251};
    uint8_t aboutShareKey[] = {164, 17, 34, 134, 65, 88, 48, 2, 139, 207, 159, 246, 129, 149, 245, 135, 244, 19, 144, 189, 18, 200, 154, 196, 247, 24, 60, 22, 61, 161, 130, 105};
    #if 1
    TrngGet(privKey, sizeof(privKey));
    SimpleResponse_u8 *simpleResponse = k1_generate_pubkey_by_privkey(privKey, sizeof(privKey));
    memcpy_s(shareKey2, sizeof(shareKey2) + 1, simpleResponse->data, PUB_KEY_SIZE);
    free_simple_response_u8(simpleResponse);
    memcpy_s(g_dataParserPubKey, sizeof(g_dataParserPubKey) + 1, shareKey2, PUB_KEY_SIZE);
    simpleResponse = k1_generate_ecdh_sharekey(privKey, sizeof(privKey), webPub, PUB_KEY_SIZE);
    if (simpleResponse == NULL) {
        printf("get_master_fingerprint return NULL\r\n");
        return NULL;
    }
    memcpy_s(g_dataSharedKey, sizeof(g_dataSharedKey), simpleResponse->data, 32);
    free_simple_response_u8(simpleResponse);
    #else
    memcpy_s(g_dataParserPubKey, sizeof(g_dataParserPubKey) + 1, webPubKey, PUB_KEY_SIZE);
    memcpy_s(g_dataSharedKey, sizeof(g_dataSharedKey), aboutShareKey, 32);
    #endif
    printf("share key:\n");
    for (int i = 0; i < 32; i++) {
        printf("%d ", g_dataSharedKey[i]);
    }
    printf("\n");
    printf("g_dataParserPubKey:\n");
    for (int i = 0; i < sizeof(g_dataParserPubKey); i++) {
        printf("%d ", g_dataParserPubKey[i]);
    }
    printf("\n");
    memset_s(privKey, sizeof(privKey), 0, sizeof(privKey));
    // for (int i = 0; i < sizeof(g_dataSharedKey); i++) {
    //     g_dataSharedKey[i] = i + 1;
    // }
    // for (int i = 0; i < sizeof(g_dataParserIv); i++) {
    //     g_dataParserIv[i] = 2 * i;
    // }
    return g_dataParserPubKey;
}

void DataEncrypt(uint8_t *data, uint16_t len)
{
    AES256_CBC_ctx ctx;
    AES256_CBC_init(&ctx, g_dataSharedKey, g_dataParserIv);
    AES256_CBC_encrypt(&ctx, len / 16, data, data);
}

void DataDecrypt(uint8_t *data, uint8_t *plain, uint16_t len)
{
    AES256_CBC_ctx ctx;
    AES256_CBC_init(&ctx, g_dataSharedKey, g_dataParserIv);
    AES256_CBC_decrypt(&ctx, len / 16, plain, data);
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

static void DataParserCacheMpuInit(void)
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

void MemManage_Handler(void)
{
#define SCB_CFSR (*((volatile uint32_t *)0xE000ED28))
#define SCB_HFSR (*((volatile uint32_t *)0xE000ED2C))
#define SCB_MMFAR (*((volatile uint32_t *)0xE000ED34))
    uint32_t cfsr = SCB_CFSR;
    uint32_t mmfar = SCB_MMFAR;

    if (cfsr & (1 << 0)) {
        uint32_t fault_address = mmfar;
        
        printf("Memory management fault at address: 0x%08X\n", fault_address);
    }

    if (cfsr & (1 << 1)) {
        // Data Access Violation
    }
    if (cfsr & (1 << 3)) {
        // Unstacking Error
    }
    if (cfsr & (1 << 4)) {
    }
    if (cfsr & (1 << 5)) {
    }

    // system reset test
    *(uint32_t *)0 = 123;
    NVIC_SystemReset();
}