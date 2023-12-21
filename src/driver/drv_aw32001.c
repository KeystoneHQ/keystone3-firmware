#include "drv_aw32001.h"
#include "drv_i2c_io.h"
#include "stdio.h"
#include "mhscpu.h"
#include "err_code.h"
#include "log_print.h"
#include "drv_battery.h"
#include "hardware_version.h"


#define AW32001_SCL_PORT                            GPIOE
#define AW32001_SDA_PORT                            GPIOF
#define AW32001_SCL_PIN                             GPIO_Pin_15
#define AW32001_SDA_PIN                             GPIO_Pin_0

#define USB_DET_INT_PORT                            GPIOF
#define USB_DET_INT_PIN                             GPIO_Pin_15

#define AW32001_ADDR                                0x49


#define AW320XX_REG0_SCR                            (0x00)
#define AW320XX_REG1_POCR                           (0x01)
#define AW320XX_REG2_CCR                            (0x02)
#define AW320XX_REG3_CCR2                           (0x03)
#define AW320XX_REG4_CVR                            (0x04)
#define AW320XX_REG5_TIMCR                          (0x05)
#define AW320XX_REG6_MCR                            (0x06)
#define AW320XX_REG7_SVCR                           (0x07)
#define AW320XX_REG8_STATR                          (0x08)
#define AW320XX_REG9_FLTR                           (0x09)
#define AW320XX_REGA_ID                             (0x0a)
#define AW320XX_REGB_CCR3                           (0x0b)
#define AW320XX_REGC_RCR                            (0x0c)

#define AW320XX_I2C_RW_RETRIES                      (5)
#define AW320XX_REGADD_SIZE_8BIT                    (1)
#define AW320XX_REGADD_SIZE_16BIT                   (2)
#define AW32001_REV_ID                              2
#define AW32012_REV_ID                              3

#define AW320XX_STATUS_IS_OK                        1

/********************AW320XX Set config start*******************/
#define PARM_VBAT_UVLO                              (0x04 << 0)
#define PARM_IDSCHG                                 (0x09 << 4)
#define PARM_IIN_LMT                                (0x0F << 0)
#define PARM_VIN_MIN                                (0x06 << 4)
#define PARM_VSYS_REG                               (0x08 << 0)
#define PARM_VBAT_REG                               (0x28 << 2)
#define PARM_VBAT_PRE                               (0x01 << 1)
#define PARM_VRECH                                  (0x01 << 0)
#define PARM_EN_ICC_DIVD                            (0x00 << 7)
#define PARM_ICC                                    (0x38 << 0)
#define PARM_ITERM                                  (0x01 << 0)
#define PARM_ITERM_31MA                             (0x0f << 0)
#define PARM_TJ_REG                                 (0x03 << 4)

/********************AW320XX Set config end*******************/


/* reg:0x00 */
#define AW320XX_BIT_SCR_VIN_MIN_MASK                 (0x0F)
#define AW320XX_BIT_SCR_IIN_LMT_MASK                 (0xF0)
#define AW320XX_BIT_SCR_IIN_LMT_500MA                (0X0F)

/* reg:0x01 */
#define AW320XX_BIT_POCR_RST_DEG_MASK                (0x3F)
#define AW320XX_BIT_POCR_RST_DEG_8S                  (0 << 6)
#define AW320XX_BIT_POCR_RST_DEG_12S                 (1 << 6)
#define AW320XX_BIT_POCR_RST_DEG_16S                 (2 << 6)
#define AW320XX_BIT_POCR_RST_DEG_20S                 (3 << 6)
#define AW320XX_BIT_POCR_RST_DUR_MASK                (0xDF)
#define AW320XX_BIT_POCR_RST_DUR_2S                  (0 << 5)
#define AW320XX_BIT_POCR_RST_DUR_4S                  (1 << 5)
#define AW320XX_BIT_POCR_ENHIZ_MASK                  (0xEF)
#define AW320XX_BIT_POCR_ENHIZ_ENABLE                (1 << 4)
#define AW320XX_BIT_POCR_ENHIZ_DISABLE               (0 << 4)
#define AW320XX_BIT_POCR_CEB_MASK                    (0xF7)
#define AW320XX_BIT_POCR_CEB_ENABLE                  (0 << 3)
#define AW320XX_BIT_POCR_CEB_DISABLE                 (1 << 3)
#define AW320XX_BIT_POCR_VBAT_UVLO_MASK              (0xF8)
/* reg:0x02 */
#define AW320XX_BIT_CCR_SOFT_RST_MASK                (0x7F)
#define AW320XX_BIT_CCR_SOFT_RST_NORMAL              (0 << 7)
#define AW320XX_BIT_CCR_SOFT_RST_RESET               (1 << 7)
#define AW320XX_BIT_CCR_WD_RST_MASK                  (0xBF)
#define AW320XX_BIT_CCR_WD_RST_NORMAL                (0 << 6)
#define AW320XX_BIT_CCR_WD_RST_RESET                 (1 << 6)
#define AW320XX_BIT_CCR_ICC_MASK                     (0xC0)
/* reg:0x03 */
#define AW320XX_BIT_CCR2_IDSCHG_MASK                 (0x0F)
#define AW320XX_BIT_CCR2_ITERM_MASK                  (0xF0)
/* reg:0x04 */
#define AW320XX_BIT_CVR_VBAT_REG_MASK                (0x03)
#define AW320XX_BIT_CVR_VBAT_PRE_MASK                (0xFD)
#define AW320XX_BIT_CVR_VBAT_PRE_2P8V                (0 << 1)
#define AW320XX_BIT_CVR_VBAT_PRE_3P0V                (1 << 1)
#define AW320XX_BIT_CVR_VRECH_MASK                   (0xFE)
#define AW320XX_BIT_CVR_VRECH_100MV                  (0 << 0)
#define AW320XX_BIT_CVR_VRECH_200MV                  (1 << 0)
/* reg:0x05 */
#define AW320XX_BIT_TIMCR_EN_WD_DSCHG_MASK           (0x7F)
#define AW320XX_BIT_TIMCR_EN_WD_DSCHG_DISABLE        (0 << 7)
#define AW320XX_BIT_TIMCR_EN_WD_DSCHG_ENABLE         (1 << 7)
#define AW320XX_BIT_TIMCR_WD_CFG_MASK                (0x9F)
#define AW320XX_BIT_TIMCR_WD_CFG_DISABLE             (0 << 5)
#define AW320XX_BIT_TIMCR_WD_CFG_40S                 (1 << 5)
#define AW320XX_BIT_TIMCR_WD_CFG_80S                 (2 << 5)
#define AW320XX_BIT_TIMCR_WD_CFG_160S                (3 << 5)
#define AW320XX_BIT_TIMCR_EN_TERM_MASK               (0xEF)
#define AW320XX_BIT_TIMCR_EN_TERM_DISABLE            (0 << 4)
#define AW320XX_BIT_TIMCR_EN_TERM_ENABLE             (1 << 4)
#define AW320XX_BIT_TIMCR_EN_TMR_MASK                (0xF7)
#define AW320XX_BIT_TIMCR_EN_TMR_DISABLE             (0 << 3)
#define AW320XX_BIT_TIMCR_EN_TMR_ENABLE              (1 << 3)
#define AW320XX_BIT_TIMCR_CHG_TMR_MASK               (0xF9)
#define AW320XX_BIT_TIMCR_CHG_TMR_3HRS               (0 << 1)
#define AW320XX_BIT_TIMCR_CHG_TMR_5HRS               (1 << 1)
#define AW320XX_BIT_TIMCR_CHG_TMR_8HRS               (2 << 1)
#define AW320XX_BIT_TIMCR_CHG_TMR_12HRS              (3 << 1)
#define AW320XX_BIT_TIMCR_TERM_TMR_MASK              (0xFE)
#define AW320XX_BIT_TIMCR_TERM_TMR_DISABLE           (0 << 0)
#define AW320XX_BIT_TIMCR_TERM_TMR_ENABLE            (1 << 0)
/* reg:0x06 */
#define AW320XX_BIT_MCR_EN_NTC_MASK                  (0x7F)
#define AW320XX_BIT_MCR_EN_NTC_DISABLE               (0 << 7)
#define AW320XX_BIT_MCR_EN_NTC_ENABLE                (1 << 7)
#define AW320XX_BIT_MCR_TMR2X_EN_MASK                (0xBF)
#define AW320XX_BIT_MCR_TMR2X_EN_DISABLE             (0 << 6)
#define AW320XX_BIT_MCR_TMR2X_EN_ENABLE              (1 << 6)
#define AW320XX_BIT_MCR_FET_DIS_MASK                 (0xDF)
#define AW320XX_BIT_MCR_FET_DIS_ON                   (0 << 5)
#define AW320XX_BIT_MCR_FET_DIS_OFF                  (1 << 5)
#define AW320XX_BIT_MCR_PG_INT_CTR_MASK              (0xEF)
#define AW320XX_BIT_MCR_PG_INT_CTR_ON                (0 << 4)
#define AW320XX_BIT_MCR_PG_INT_CTR_OFF               (1 << 4)
#define AW320XX_BIT_MCR_EOC_INT_CTR_MASK             (0xF7)
#define AW320XX_BIT_MCR_EOC_INT_CTR_ON               (0 << 3)
#define AW320XX_BIT_MCR_EOC_INT_CTR_OFF              (1 << 3)
#define AW320XX_BIT_MCR_CS_INT_CTR_MASK              (0xFB)
#define AW320XX_BIT_MCR_CS_INT_CTR_ON                (0 << 2)
#define AW320XX_BIT_MCR_CS_INT_CTR_OFF               (1 << 2)
#define AW320XX_BIT_MCR_NTC_INT_CTR_MASK             (0xFD)
#define AW320XX_BIT_MCR_NTC_INT_CTR_ON               (0 << 1)
#define AW320XX_BIT_MCR_NTC_INT_CTR_OFF              (1 << 1)
#define AW320XX_BIT_MCR_BOVP_INT_CTR_MASK            (0xFE)
#define AW320XX_BIT_MCR_BOVP_INT_CTR_ON              (0 << 0)
#define AW320XX_BIT_MCR_BOVP_INT_CTR_OFF             (1 << 0)
/* reg:0x07 */
#define AW320XX_BIT_SVCR_EN_PCB_OTP_MASK             (0x7F)
#define AW320XX_BIT_SVCR_EN_PCB_OTP_DISABLE          (1 << 7)
#define AW320XX_BIT_SVCR_EN_PCB_OTP_ENABLE           (0 << 7)
#define AW320XX_BIT_SVCR_EN_VINLOOP_MASK             (0xBF)
#define AW320XX_BIT_SVCR_EN_VINLOOP_DISABLE          (1 << 6)
#define AW320XX_BIT_SVCR_EN_VINLOOP_ENABLE           (0 << 6)
#define AW320XX_BIT_SVCR_TJ_REG_MASK                 (0xCF)
#define AW320XX_BIT_SVCR_TJ_REG_60                   (0 << 4)
#define AW320XX_BIT_SVCR_TJ_REG_80                   (1 << 4)
#define AW320XX_BIT_SVCR_TJ_REG_100                  (2 << 4)
#define AW320XX_BIT_SVCR_TJ_REG_120                  (3 << 4)
#define AW320XX_BIT_SVCR_VSYS_REG_MASK               (0xF0)
/* reg:0x08 */
#define AW320XX_BIT_REV_ID_MASK                      (3 << 5)
#define AW320XX_BIT_REV_ID_SHIFT                     (5)
/* reg:0x09 */
#define AW320XX_BIT_FLTR_EN_SHIP_DEG_MASK            (0x3F)
#define AW320XX_BIT_FLTR_TJ_REG_1S                   (0 << 6)
#define AW320XX_BIT_FLTR_TJ_REG_2S                   (1 << 6)
#define AW320XX_BIT_FLTR_TJ_REG_4S                   (2 << 6)
#define AW320XX_BIT_FLTR_TJ_REG_8S                   (3 << 6)
/* reg:0x0a is DEV_ID*/
/* reg:0x0b */
#define AW320XX_BIT_CCR3_EN_ICC_DIVD_MASK            (0x7F)
#define AW320XX_BIT_CCR3_EN_ICC_DIVD_DISABLE         (0 << 7)
#define AW320XX_BIT_CCR3_EN_ICC_DIVD_ENABLE          (1 << 7)
#define AW320XX_BIT_CCR3_EN_IPRE_SET_MASK            (0xDF)
#define AW320XX_BIT_CCR3_EN_IPRE_SET_DISABLE         (0 << 5)
#define AW320XX_BIT_CCR3_EN_IPRE_SET_ENABLE          (1 << 5)
#define AW320XX_BIT_CCR3_IPRE_MASK                   (0xE1)
#define AW320XX_BIT_CCR3_EXSHIP_DEG_MASK             (0xFE)
#define AW320XX_BIT_CCR3_EXSHIP_DEG_2S               (0 << 0)
#define AW320XX_BIT_CCR3_EXSHIP_DEG_100MS            (1 << 0)
/* reg:0x0c */
#define AW320XX_BIT_RCR_EN0P55_MASK                  (0x7F)
#define AW320XX_BIT_RCR_EN0P55_DISABLE               (0 << 7)
#define AW320XX_BIT_RCR_EN0P55_ENABLE                (1 << 7)
#define AW320XX_BIT_RCR_ITERMDEG_MASK                (0xBF)
#define AW320XX_BIT_RCR_ITERMDEG_3S                  (0 << 6)
#define AW320XX_BIT_RCR_ITERMDEG_1S                  (1 << 6)
#define AW320XX_BIT_RCR_PRETO_MASK                   (0xF7)
#define AW320XX_BIT_RCR_PRETO_1H                     (0 << 3)
#define AW320XX_BIT_RCR_PRETO_2H                     (1 << 3)
#define AW320XX_BIT_RCR_EN10KNTC_MASK                (0xFD)
#define AW320XX_BIT_RCR_EN10KNTC_DISABLE             (0 << 1)
#define AW320XX_BIT_RCR_EN10KNTC_ENABLE              (1 << 1)
#define AW320XX_BIT_RCR_RSTDLY_MASK                  (0xFE)
#define AW320XX_BIT_RCR_RSTDLY_0S                    (0 << 0)
#define AW320XX_BIT_RCR_RSTDLY_2S                    (1 << 0)


typedef struct {
    ChargeState chargeState;
    UsbPowerState userPowerState;
} PowerChangeState_t;



static I2CIO_Cfg_t g_i2cIoCfg;
static PowerChangeState_t g_powerChangeState;
static ChangerInsertIntCallbackFunc_t g_changerInsertIntCallback;


static void Aw32001RegValueInit(void);
static void Aw32001ChargingEnable(void);
static void Aw32001ChargingDisable(void);
static int32_t Aw32001ReadReg(uint8_t regAddr, uint8_t *pData, uint8_t len);
static int32_t Aw32001WriteReg(uint8_t regAddr, const uint8_t *pData, uint8_t len);
static int32_t Aw32001WriteRegBits(uint8_t regAddr, uint8_t mask, uint8_t data);

void Aw32001Init(void)
{
    uint32_t milliVolt;
    I2CIO_Init(&g_i2cIoCfg, AW32001_SCL_PORT, AW32001_SCL_PIN, AW32001_SDA_PORT, AW32001_SDA_PIN);
    Aw32001RegValueInit();
    milliVolt = GetBatteryMilliVolt();
    printf("milliVolt=%d\n", milliVolt);
    Aw32001ChargingEnable();
}


void RegisterChangerInsertCallback(ChangerInsertIntCallbackFunc_t func)
{
    g_changerInsertIntCallback = func;
}


int32_t Aw32001PowerOff(void)
{
    uint8_t byte;
    int32_t ret = SUCCESS_CODE;
    do {
        ret = Aw32001WriteRegBits(AW320XX_REG6_MCR, AW320XX_BIT_MCR_FET_DIS_MASK, AW320XX_BIT_MCR_FET_DIS_OFF);
        if (ret != SUCCESS_CODE) {
            continue;
        }
        ret = Aw32001ReadReg(AW320XX_REG6_MCR, &byte, 1);
        printf("AW320XX_REG6_MCR byte=0x%X\n", byte);
        printf("AW320XX_REG6_MCR ship power = %#x\n", byte & AW320XX_BIT_MCR_FET_DIS_OFF);
        if ((ret == SUCCESS_CODE) && ((byte & AW320XX_BIT_MCR_FET_DIS_OFF) == AW320XX_BIT_MCR_FET_DIS_OFF)) {
            printf("power off success\n");
            return SUCCESS_CODE;
        }
    } while (1);
    return ret;
}


int32_t Aw32001RefreshState(void)
{
    uint8_t byte;
    int32_t ret;
    do {
        ret = Aw32001ReadReg(AW320XX_REG8_STATR, &byte, 1);
        CHECK_ERRCODE_BREAK("read aw32001 reg", ret);
        printf("byte=0x%X\n", byte);
        g_powerChangeState.chargeState = (ChargeState)((byte >> 3) & 0x03);
        g_powerChangeState.userPowerState = (UsbPowerState)((byte >> 1) & 0x01);
    } while (0);
    return ret;
}


ChargeState GetChargeState(void)
{
    return g_powerChangeState.chargeState;
}


UsbPowerState GetUsbPowerState(void)
{
    return g_powerChangeState.userPowerState;
}


bool GetUsbDetectState(void)
{
    BitAction state = Bit_SET;
    if (GetHardwareVersion() >= VERSION_V3_1) {
        state = Bit_RESET;
    }
    return GPIO_ReadInputDataBit(USB_DET_INT_PORT, USB_DET_INT_PIN) == state;
}


void ChangerInsertHandler(void)
{
    if (g_changerInsertIntCallback != NULL) {
        g_changerInsertIntCallback();
    }
}


void Aw32001Test(int argc, char *argv[])
{
    uint8_t data[0x0C], data22;

    if (strcmp(argv[0], "read") == 0) {
        Aw32001ReadReg(0, data, 0x0C);
        PrintArray("reg data", data, 0x0C);
        Aw32001ReadReg(0x22, &data22, 1);
        printf("data22=0x%02X\r\n", (uint32_t)data22);
    } else if (strcmp(argv[0], "poweroff") == 0) {
        printf("poweroff\r\n");
        Aw32001PowerOff();
    } else if (strcmp(argv[0], "reset") == 0) {
        printf("reset\r\n");
        NVIC_SystemReset();
    } else if (strcmp(argv[0], "enable_charge") == 0) {
        printf("enable charge\r\n");
        Aw32001ChargingEnable();
    } else if (strcmp(argv[0], "disable_charge") == 0) {
        printf("disable charge\r\n");
        Aw32001ChargingDisable();
    } else if (strcmp(argv[0], "state") == 0) {
        printf("GetChargeState=%d\r\n", GetChargeState());
        printf("GetUsbPowerState=%d\r\n", GetUsbPowerState());
    }
}


static void Aw32001RegValueInit(void)
{
    Aw32001WriteRegBits(AW320XX_REG2_CCR, AW320XX_BIT_CCR_SOFT_RST_MASK, AW320XX_BIT_CCR_SOFT_RST_RESET);     // Reg Reset

    Aw32001WriteRegBits(AW320XX_REG1_POCR, AW320XX_BIT_POCR_VBAT_UVLO_MASK, PARM_VBAT_UVLO);        //UVLO threshold 2.76V
    Aw32001WriteRegBits(AW320XX_REG1_POCR, AW320XX_BIT_POCR_RST_DEG_MASK, AW320XX_BIT_POCR_RST_DEG_12S);        //RST_DGL 12s
    Aw32001WriteRegBits(AW320XX_REG3_CCR2, AW320XX_BIT_CCR2_IDSCHG_MASK, PARM_IDSCHG);              //IDSCHG 2000mA
    Aw32001WriteRegBits(AW320XX_REG0_SCR, AW320XX_BIT_SCR_IIN_LMT_MASK, PARM_IIN_LMT);              //IIN_LMT 500mA
    Aw32001WriteRegBits(AW320XX_REG0_SCR, AW320XX_BIT_SCR_VIN_MIN_MASK, PARM_VIN_MIN);              //VIN_MIN 4.36V
    Aw32001WriteRegBits(AW320XX_REG7_SVCR, AW320XX_BIT_SVCR_VSYS_REG_MASK, PARM_VSYS_REG);          //VSYS_REG 4.60V
    Aw32001WriteRegBits(AW320XX_REG4_CVR, AW320XX_BIT_CVR_VBAT_REG_MASK, PARM_VBAT_REG);            //VBAT_REG 4.20V
    Aw32001WriteRegBits(AW320XX_REG4_CVR, AW320XX_BIT_CVR_VBAT_PRE_MASK, PARM_VBAT_PRE);            //VBAT_PRE 3.0V
    Aw32001WriteRegBits(AW320XX_REG4_CVR, AW320XX_BIT_CVR_VRECH_MASK, PARM_VRECH);                  //PARM_VRECH 200mV
    Aw32001WriteRegBits(AW320XX_REGB_CCR3, AW320XX_BIT_CCR3_EN_ICC_DIVD_MASK, PARM_EN_ICC_DIVD);    //EN_ICC_DIVD Keep the current value of REG02[5:0] configuration.
    Aw32001WriteRegBits(AW320XX_REG2_CCR, AW320XX_BIT_CCR_ICC_MASK, PARM_ICC);                      //ICC 456mA
    Aw32001WriteRegBits(AW320XX_REG3_CCR2, AW320XX_BIT_CCR2_ITERM_MASK, PARM_ITERM_31MA);                //ITERM 3mA
    Aw32001WriteRegBits(AW320XX_REG7_SVCR, AW320XX_BIT_SVCR_TJ_REG_MASK, PARM_TJ_REG);              //Thermal regulation threshold 120℃
    //Disable watchdog timer
    Aw32001WriteRegBits(AW320XX_REG5_TIMCR, AW320XX_BIT_TIMCR_WD_CFG_MASK, AW320XX_BIT_TIMCR_WD_CFG_DISABLE);
    //Disable NTC
    Aw32001WriteRegBits(AW320XX_REG6_MCR, AW320XX_BIT_MCR_EN_NTC_MASK, AW320XX_BIT_MCR_EN_NTC_DISABLE);
    Aw32001WriteRegBits(AW320XX_REG7_SVCR, AW320XX_BIT_SVCR_EN_PCB_OTP_MASK, AW320XX_BIT_SVCR_EN_PCB_OTP_DISABLE);
}


static void Aw32001ChargingEnable(void)
{
    Aw32001WriteRegBits(AW320XX_REG1_POCR, AW320XX_BIT_POCR_CEB_MASK, AW320XX_BIT_POCR_CEB_ENABLE);
}


static void Aw32001ChargingDisable(void)
{
    Aw32001WriteRegBits(AW320XX_REG1_POCR, AW320XX_BIT_POCR_CEB_MASK, AW320XX_BIT_POCR_CEB_DISABLE);
}


static int32_t Aw32001ReadReg(uint8_t regAddr, uint8_t *pData, uint8_t len)
{
    int32_t ret;
    do {
        ret = I2CIO_SendData(&g_i2cIoCfg, AW32001_ADDR, &regAddr, 1);
        CHECK_ERRCODE_BREAK("i2cio send", ret);
        ret = I2CIO_ReceiveData(&g_i2cIoCfg, AW32001_ADDR, pData, len);
        CHECK_ERRCODE_BREAK("i2cio rcv", ret);
    } while (0);

    return ret;
}


static int32_t Aw32001WriteReg(uint8_t regAddr, const uint8_t *pData, uint8_t len)
{
    int32_t ret;
    uint8_t sendData[len + 1];

    sendData[0] = regAddr;
    memcpy(&sendData[1], pData, len);
    do {
        ret = I2CIO_SendData(&g_i2cIoCfg, AW32001_ADDR, sendData, len + 1);
        CHECK_ERRCODE_BREAK("i2cio send", ret);
    } while (0);

    return ret;
}


static int32_t Aw32001WriteRegBits(uint8_t regAddr, uint8_t mask, uint8_t data)
{
    int32_t ret;
    uint8_t readData;

    do {
        ret = Aw32001ReadReg(regAddr, &readData, 1);
        CHECK_ERRCODE_BREAK("read aw32001 reg", ret);
        readData &= mask;
        readData |= (data & (~mask));
        ret = Aw32001WriteReg(regAddr, &readData, 1);
        CHECK_ERRCODE_BREAK("write aw32001 reg", ret);
    } while (0);

    return ret;
}
