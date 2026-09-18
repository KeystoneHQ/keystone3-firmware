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
#include "circular_buffer.h"
#include "protocol_parse.h"
#include "ctaes.h"
#include "drv_otp.h"
#include "drv_trng.h"
#include "librust_c.h"

#define ECC_PRV_KEY_SIZE                                24
#define ECC_PUB_KEY_SIZE                                (2 * ECC_PRV_KEY_SIZE)
#define PARSER_CACHE_LEN                                2048
#define PRIV_KEY_SIZE                                   32
#define PUB_KEY_SIZE                                    33

static void DataParserTask(void *argument);
void USBD_cdc_SendBuffer_Cb(const uint8_t *data, uint32_t len);

static uint8_t g_dataParserCache[PARSER_CACHE_LEN];
static cbuf_handle_t g_cBufHandle;
static uint8_t g_dataParserPubKey[PUB_KEY_SIZE] = {0};
static uint8_t g_dataSharedKey[PRIV_KEY_SIZE] = {0};
static uint8_t g_dataParserIv[16] = {0};
static osThreadId_t g_dataParserHandle;

uint8_t *GetDataParserCache(void)
{
    return g_dataParserCache;
}

void SetDeviceParserIv(uint8_t *iv)
{
    memcpy_s(g_dataParserIv, sizeof(g_dataParserIv), iv, 16);
}

uint8_t *GetDeviceParserPubKey(uint8_t *webPub, uint16_t len)
{
    assert(len == 33);
    uint8_t privKey[32] = {};
    uint8_t shareKey[PUB_KEY_SIZE] = {0};
    TrngGet(privKey, sizeof(privKey));
    SimpleResponse_u8 *simpleResponse = k1_generate_pubkey_by_privkey(privKey, sizeof(privKey));
    memcpy_s(shareKey, sizeof(shareKey), simpleResponse->data, PUB_KEY_SIZE);
    free_simple_response_u8(simpleResponse);
    memcpy_s(g_dataParserPubKey, sizeof(g_dataParserPubKey), shareKey, PUB_KEY_SIZE);
    simpleResponse = k1_generate_ecdh_sharekey(privKey, sizeof(privKey), webPub, PUB_KEY_SIZE);
    if (simpleResponse == NULL) {
        printf("get_master_fingerprint return NULL\r\n");
        return NULL;
    }
    memcpy_s(g_dataSharedKey, sizeof(g_dataSharedKey), simpleResponse->data, 32);
    free_simple_response_u8(simpleResponse);
    memset_s(privKey, sizeof(privKey), 0, sizeof(privKey));
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
        .stack_size = 1024 * 28,
        .priority = (osPriority_t)osPriorityHigh,
    };
    g_dataParserHandle = osThreadNew(DataParserTask, NULL, &dataParserTask_attributes);
}

bool CanPushDataToField(uint16_t len)
{
    if (g_cBufHandle == NULL) {
        return false;
    }

    size_t capacity = circular_buf_capacity(g_cBufHandle);
    size_t used = circular_buf_size(g_cBufHandle);
    return (capacity >= used) && ((capacity - used) >= len);
}

uint16_t PushDataToField(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0 || !CanPushDataToField(len)) {
        return 0;
    }

    uint16_t pushed = 0;
    for (uint16_t i = 0; i < len; i++) {
        if (circular_buf_try_put(g_cBufHandle, data[i]) != 0) {
            break;
        }
        pushed++;
    }

    return pushed;
}

void ResetDataField(void)
{
    if (g_cBufHandle != NULL) {
        circular_buf_reset(g_cBufHandle);
    }
}

static void DataParserTask(void *argument)
{
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
        case SPRING_MSG_GET: {
            uint32_t targetLen = rcvMsg.value;
            if (targetLen > sizeof(USB_Rx_Buffer) - 1U) {
                targetLen = sizeof(USB_Rx_Buffer) - 1U;
            }
            uint32_t actualLen = 0;
            for (uint32_t i = 0; i < targetLen; i++) {
                if (circular_buf_get(g_cBufHandle, &USB_Rx_Buffer[i]) != 0) {
                    break;
                }
                actualLen++;
            }
            if (actualLen > 0U) {
                ProtocolReceivedData(USB_Rx_Buffer, actualLen, USBD_cdc_SendBuffer_Cb);
            }
            break;
        }
        default:
            break;
        }
        if (rcvMsg.buffer != NULL) {
            SRAM_FREE(rcvMsg.buffer);
        }
    }
}

static void MemManageFaultHandler(uint32_t *stackFrame, uint32_t excReturn)
    __attribute__((noreturn, noinline, used));

__attribute__((naked)) void MemManage_Handler(void)
{
    __asm volatile (
        "tst lr, #4\n"
        "ite eq\n"
        "mrseq r0, msp\n"
        "mrsne r0, psp\n"
        "mov r1, lr\n"
        "b MemManageFaultHandler\n"
    );
}

static void MemManageFaultHandler(uint32_t *stackFrame, uint32_t excReturn)
{
    extern volatile uint32_t ulSystemCallRejectReason;
    extern volatile uint32_t ulSystemCallRejectLocation;
    extern volatile uint32_t ulSystemCallRejectNumber;
    extern volatile uint32_t ulSystemCallRejectTaskFrame;
    extern volatile uint32_t ulSystemCallRejectActiveTaskStack;
    extern volatile uint32_t ulSystemCallRejectImplementation;
    extern volatile uint32_t ulSystemCallRejectSavedLinkRegister;
    extern volatile uint32_t ulSystemCallRejectExceptionReturn;
    uint32_t cfsr = SCB->CFSR;
    uint32_t faultAddress = (cfsr & SCB_CFSR_MMARVALID_Msk) != 0U ? SCB->MMFAR : UINT32_MAX;
    uint32_t stackedLr = UINT32_MAX;
    uint32_t stackedPc = UINT32_MAX;
    uint32_t stackedPsr = UINT32_MAX;

    if ((stackFrame != NULL) &&
            ((cfsr & (SCB_CFSR_MUNSTKERR_Msk | SCB_CFSR_MSTKERR_Msk)) == 0U)) {
        stackedLr = stackFrame[5];
        stackedPc = stackFrame[6];
        stackedPsr = stackFrame[7];
    }

    printf("MemManage cfsr=0x%08lX mmfar=0x%08lX exc_return=0x%08lX "
           "lr=0x%08lX pc=0x%08lX psr=0x%08lX\r\n",
           (unsigned long)cfsr, (unsigned long)faultAddress, (unsigned long)excReturn,
           (unsigned long)stackedLr, (unsigned long)stackedPc, (unsigned long)stackedPsr);
    printf("SyscallReject reason=0x%02lX location=0x%08lX svc=%lu frame=0x%08lX "
           "active=0x%08lX impl=0x%08lX saved_lr=0x%08lX exc_return=0x%08lX\r\n",
           (unsigned long)ulSystemCallRejectReason,
           (unsigned long)ulSystemCallRejectLocation,
           (unsigned long)ulSystemCallRejectNumber,
           (unsigned long)ulSystemCallRejectTaskFrame,
           (unsigned long)ulSystemCallRejectActiveTaskStack,
           (unsigned long)ulSystemCallRejectImplementation,
           (unsigned long)ulSystemCallRejectSavedLinkRegister,
           (unsigned long)ulSystemCallRejectExceptionReturn);
    __DSB();
    NVIC_SystemReset();
    for (;;) {
    }
}
