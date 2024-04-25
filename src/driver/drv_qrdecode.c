#include "drv_qrdecode.h"
#include "mhscpu.h"
#include "decodelib.h"
#include "user_memory.h"
#include "cmsis_os.h"
#include "hal_lcd.h"
#include "draw_on_lcd.h"

#define VIEW_IMAGE_ENABLE

/*camera XCK set*/
#define CAM_XCK_GPIO                GPIOA
#define CAM_XCK_GPIO_PIN            GPIO_Pin_5
#define CAM_XCK_TIM                 TIM_5

/*camera I2C set*/
#define SI2C_PORT                   GPIOB
#define SI2C_SCL_PIN                GPIO_Pin_0
#define SI2C_SDA_PIN                GPIO_Pin_1
#define SI2C_GPIO_REMAP             GPIO_Remap_0

/*camera PWDN set*/
#define CAM_PWDN_GPIO               GPIOH
#define CAM_PWDN_GOIO_PIN           GPIO_Pin_9

/*camera RST set*/
#define CAM_RST_GPIO                GPIOH
#define CAM_RST_GOIO_PIN            GPIO_Pin_8

static void DCMI_NVICConfig(void);
static void CameraI2CGPIOConfig(void);
static void Cameraclk_Configuration(void);
#ifdef VIEW_IMAGE_ENABLE
static uint8_t *GetQrDecodeImageAddr(void);
static void ViewImageOnLcd(void);
#endif

static uint32_t g_camTick = 0;
static uint32_t g_viewTick = 0;
static uint32_t g_decodeTick = 0;

static uint8_t *g_memPool = NULL;
DecodeConfigTypeDef g_decodeCfg = {0};

LV_FONT_DECLARE(openSans_20);
LV_FONT_DECLARE(openSans_24);

/**
 * @brief       QR decode init, malloc QRDECODE_BUFF_SIZE byte mem.
 * @retval      none.
 */
int32_t QrDecodeInit(uint8_t *pool)
{
    DecodeInitTypeDef DecodeInitStruct = {0};
    DecodeFlagTypeDef ret;

    SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_OTP, ENABLE);
    SYSCTRL_AHBPeriphResetCmd(SYSCTRL_AHBPeriph_OTP, ENABLE);

    g_memPool = pool;
    CameraI2CGPIOConfig();
    Cameraclk_Configuration();
    DecodeInitStruct.pool = g_memPool;
    DecodeInitStruct.size = QRDECODE_BUFF_SIZE;
    DecodeInitStruct.CAM_PWDN_GPIOx = CAM_PWDN_GPIO;
    DecodeInitStruct.CAM_PWDN_GPIO_Pin = CAM_PWDN_GOIO_PIN;
    DecodeInitStruct.CAM_RST_GPIOx = CAM_RST_GPIO;
    DecodeInitStruct.CAM_RST_GPIO_Pin = CAM_RST_GOIO_PIN;
    DecodeInitStruct.CAM_I2Cx = I2C0;
    DecodeInitStruct.CAM_I2CClockSpeed = I2C_ClockSpeed_400KHz;
    DecodeInitStruct.SensorConfig = NULL;
    DecodeInitStruct.SensorCfgSize = 0;
    ret = DecodeInit(&DecodeInitStruct);
    DecodeConfigInit(&g_decodeCfg);
    DCMI_NVICConfig();

    return ret;
}

/**
 * @brief       QR decode deinit, release hardware/software source.
 * @retval      none.
 */
void QrDecodeDeinit(void)
{
    //SRAM_FREE(g_memPool);
    CloseDecode();
    //SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_OTP, DISABLE);
}

uint32_t QrDecodeGetCamTick(void)
{
    uint32_t tick = g_camTick;
    g_camTick = 0;
    return tick;
}

uint32_t QrDecodeGetViewTick(void)
{
    uint32_t tick = g_viewTick;
    g_viewTick = 0;
    return tick;
}

uint32_t QrDecodeGetDecodeTick(void)
{
    uint32_t tick = g_decodeTick;
    g_decodeTick = 0;
    return tick;
}

/// @brief QR decode process, called in the decoding thread loop.
/// @param[out] result store qrdecode result here if success.
/// @param[in] maxLen max length of result.
/// @param[in] progress 0-100, show progress bar on lcd. Do not show progress bar if progress value is 0.
/// @return err code, int32_t
///             return QR decode char length.
///             0 represents unrecognized image.
///             A negative number returned represents an error.
int32_t QrDecodeProcess(char *result, uint32_t maxLen, uint8_t progress)
{
    int32_t resnum;
    DecodeResultTypeDef res = {.result = (uint8_t *)result, .maxn = maxLen};
    uint32_t tick;
    static uint8_t progressNum = 100;
    char progressStr[16];

    if (progressNum != progress) {
        if (progress > 0) {
            snprintf_s(progressStr, sizeof(progressStr), "%d%%  ", progress);
            DrawStringOnLcd(215, 638, progressStr, 0xFFFF, &openSans_24);
            DrawProgressBarOnLcd(80, 594, 320, 9, progress, 0x21F4);
        }
        progressNum = progress;
    }
    tick = osKernelGetTickCount();
    DecodeDcmiStart();
    while (!DecodeDcmiFinish()) {           //Finish waiting by DCMI_CallBackFrame()
#ifdef VIEW_IMAGE_ENABLE
        if (GetQrDecodeImageAddr() != NULL) {
            //break;            //break before cam finish.
        }
#endif
        osDelay(1);
    }
    g_camTick += osKernelGetTickCount() - tick;
    tick = osKernelGetTickCount();
#ifdef VIEW_IMAGE_ENABLE
    ViewImageOnLcd();
#endif
    while (!DecodeDcmiFinish()) {
        osDelay(1);
    }
    g_viewTick += osKernelGetTickCount() - tick;
    tick = osKernelGetTickCount();
    resnum = DecodeStart(&g_decodeCfg, &res);
    if (resnum > 0) {
        // printf("ID:%d\tAIMID:%s\n", res.id, res.AIM);
        CleanDecodeBuffFlag();
    }
    g_decodeTick += osKernelGetTickCount() - tick;

    return resnum;
    //return 0;
}

#ifdef VIEW_IMAGE_ENABLE

#define VIEW_IMAGE_LINE             20

static uint8_t *staticImgAddr = NULL;

static uint8_t *GetQrDecodeImageAddr(void)
{
    return staticImgAddr;
}

static void ViewImageOnLcd(void)
{
    uint8_t *imgAddr;

    static uint16_t *buffer1 = NULL;
    uint8_t *u8Addr;
    uint16_t R, G, B;

    if (buffer1 == NULL) {
        buffer1 = SRAM_MALLOC(320 * VIEW_IMAGE_LINE * 2);
    }

    imgAddr = (uint8_t *)GetImageBuffAddr();
    if (imgAddr == NULL) {
        imgAddr = staticImgAddr;
    }
    if (imgAddr == NULL) {
        return;
    }
    staticImgAddr = imgAddr;

    uint32_t i, camPixelIndex = 0, line;
    uint32_t x = 0, y = 0;
#define START_SCAN_LINE 225
#define START_SCAN_COL  82
    for (line = START_SCAN_LINE; line < START_SCAN_LINE + 320; line += VIEW_IMAGE_LINE) {
        x = 0;
        while (LcdBusy()) {
            osDelay(1);
        }
        for (i = 0; i < 320 * VIEW_IMAGE_LINE; i++) {
            camPixelIndex = ((320 - y) * 3 / 2 + 80) + (x * 3 / 2) * 640;
            u8Addr = (uint8_t *)&buffer1[i];
            // *u8Addr = (imgAddr[camPixelIndex] & 0xF1) | (imgAddr[camPixelIndex] >> 5);
            // *(u8Addr + 1) = ((imgAddr[camPixelIndex] << 3) & 0xE0) | (imgAddr[camPixelIndex] >> 3);
            G = imgAddr[camPixelIndex] >> 2;
            R = G >> 1;
            B = R;
            *(uint16_t*)u8Addr = ((R << 3 | B << 8 | G << 13 | G >> 3));
            x++;
            if (x >= 320) {
                x = 0;
                y++;
            }
        }
        LcdDraw(START_SCAN_COL, line, START_SCAN_COL + 320 - 1, line + VIEW_IMAGE_LINE - 1, (uint16_t *)buffer1);
    }
}

#endif

/* DCMI Interrupt Config */
static void DCMI_NVICConfig(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);

    NVIC_InitStructure.NVIC_IRQChannel = DCMI_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

//  DCMI_ITConfig(DCMI_IT_VSYNC, ENABLE);
    DCMI_ITConfig(DCMI_IT_OVF, ENABLE);
//  DCMI_ITConfig(DCMI_IT_LINE, ENABLE);
    DCMI_ITConfig(DCMI_IT_FRAME, ENABLE);
    DCMI_ITConfig(DCMI_IT_ERR, ENABLE);

    DCMI_ClearITPendingBit(DCMI_IT_VSYNC);
    DCMI_ClearITPendingBit(DCMI_IT_OVF);
    DCMI_ClearITPendingBit(DCMI_IT_LINE);
    DCMI_ClearITPendingBit(DCMI_IT_FRAME);
    DCMI_ClearITPendingBit(DCMI_IT_ERR);
}

/* I2C Pin Config */
static void CameraI2CGPIOConfig(void)
{
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_I2C0, ENABLE);
    I2C_DeInit(I2C0);

    GPIO_PinRemapConfig(SI2C_PORT, SI2C_SCL_PIN, SI2C_GPIO_REMAP);
    GPIO_PinRemapConfig(SI2C_PORT, SI2C_SDA_PIN, SI2C_GPIO_REMAP);
}

/* Camera Clock Config */
static void Cameraclk_Configuration(void)
{
    uint32_t Period = 0;
    uint32_t PWM_HZ = 24000000;
    SYSCTRL_ClocksTypeDef clocks;
    TIM_PWMInitTypeDef TIM_PWMSetStruct;

    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_TIMM0, ENABLE);

    SYSCTRL_GetClocksFreq(&clocks);

    /* Check PCLK, need >= 48MHz */
    //if (clocks.PCLK_Frequency / 2 < PWM_HZ) {
    PWM_HZ = clocks.PCLK_Frequency / 2;
    //}

    Period = clocks.PCLK_Frequency / PWM_HZ;

    TIM_PWMSetStruct.TIM_LowLevelPeriod = (Period / 2 - 1);
    TIM_PWMSetStruct.TIM_HighLevelPeriod = (Period - TIM_PWMSetStruct.TIM_LowLevelPeriod - 2);

    TIM_PWMSetStruct.TIMx = CAM_XCK_TIM;
    TIM_PWMInit(TIMM0, &TIM_PWMSetStruct);

    GPIO_PinRemapConfig(CAM_XCK_GPIO, CAM_XCK_GPIO_PIN, GPIO_Remap_2);

    TIM_Cmd(TIMM0, CAM_XCK_TIM, ENABLE);
}

void DCMI_IRQHandler(void)
{
    if (DCMI_GetITStatus(DCMI_IT_LINE) != RESET) {
        DCMI_ClearITPendingBit(DCMI_IT_LINE);
    }

    if (DCMI_GetITStatus(DCMI_IT_VSYNC) != RESET) {
        DCMI_ClearITPendingBit(DCMI_IT_VSYNC);
    }

    if (DCMI_GetITStatus(DCMI_IT_FRAME) != RESET) {
        //callback
        DCMI_CallBackFrame();
        DCMI_ClearITPendingBit(DCMI_IT_FRAME);
    }

    if (DCMI_GetITStatus(DCMI_IT_OVF) != RESET) {
        DCMI_ClearITPendingBit(DCMI_IT_OVF);
    }

    if (DCMI_GetITStatus(DCMI_IT_ERR) != RESET) {
        DCMI_ClearITPendingBit(DCMI_IT_ERR);
    }
}
