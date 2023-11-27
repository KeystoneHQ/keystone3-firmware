#include "drv_i2c_io.h"
#include "stdio.h"
#include "err_code.h"
#include "log_print.h"
#include "user_delay.h"
#include "cmsis_os.h"


static void I2CIO_Start(const I2CIO_Cfg_t *cfg);
static void I2CIO_Stop(const I2CIO_Cfg_t *cfg);
static bool I2CIO_WaitAck(const I2CIO_Cfg_t *cfg);
static void I2CIO_SendNoAck(const I2CIO_Cfg_t *cfg);
static void I2CIO_SendAck(const I2CIO_Cfg_t *cfg);

static void I2CIO_WriteByte(const I2CIO_Cfg_t *cfg, uint8_t byte);
static uint8_t I2CIO_ReadByte(const I2CIO_Cfg_t *cfg);
static void I2CIO_Delay(uint32_t tick);



static void I2CIO_Start(const I2CIO_Cfg_t *cfg)
{
    GPIO_SetBits(cfg->SDA_PORT, cfg->SDA_PIN);
    GPIO_SetBits(cfg->SCL_PORT, cfg->SCL_PIN);
    I2CIO_Delay(100);
    GPIO_ResetBits(cfg->SDA_PORT, cfg->SDA_PIN);
    I2CIO_Delay(100);
    GPIO_ResetBits(cfg->SCL_PORT, cfg->SCL_PIN);
}


static void I2CIO_Stop(const I2CIO_Cfg_t *cfg)
{
    GPIO_ResetBits(cfg->SDA_PORT, cfg->SDA_PIN);
    GPIO_ResetBits(cfg->SCL_PORT, cfg->SCL_PIN);
    I2CIO_Delay(100);
    GPIO_SetBits(cfg->SCL_PORT, cfg->SCL_PIN);
    I2CIO_Delay(100);
    GPIO_SetBits(cfg->SDA_PORT, cfg->SDA_PIN);
    I2CIO_Delay(100);
}


static bool I2CIO_WaitAck(const I2CIO_Cfg_t *cfg)
{
    uint16_t tick = 0x1FF;

    I2CIO_Delay(100);
    GPIO_SetBits(cfg->SDA_PORT, cfg->SDA_PIN);
    I2CIO_Delay(100);
    GPIO_SetBits(cfg->SCL_PORT, cfg->SCL_PIN);

    while (GPIO_ReadInputDataBit(cfg->SDA_PORT, cfg->SDA_PIN)) {
        tick--;
        if (!tick) {
            return false;
        }
    }
    I2CIO_Delay(100);
    GPIO_ResetBits(cfg->SCL_PORT, cfg->SCL_PIN);
    I2CIO_Delay(100);

    return true;
}


static void I2CIO_SendNoAck(const I2CIO_Cfg_t *cfg)
{
    GPIO_SetBits(cfg->SDA_PORT, cfg->SDA_PIN);
    I2CIO_Delay(100);
    GPIO_SetBits(cfg->SCL_PORT, cfg->SCL_PIN);
    I2CIO_Delay(100);
    GPIO_ResetBits(cfg->SCL_PORT, cfg->SCL_PIN);
}


static void I2CIO_SendAck(const I2CIO_Cfg_t *cfg)
{
    GPIO_ResetBits(cfg->SDA_PORT, cfg->SDA_PIN);
    I2CIO_Delay(100);
    GPIO_SetBits(cfg->SCL_PORT, cfg->SCL_PIN);
    I2CIO_Delay(100);
    GPIO_ResetBits(cfg->SCL_PORT, cfg->SCL_PIN);
}


static void I2CIO_WriteByte(const I2CIO_Cfg_t *cfg, uint8_t byte)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        GPIO_ResetBits(cfg->SCL_PORT, cfg->SCL_PIN);
        I2CIO_Delay(100);
        if (0x80 & byte) {
            GPIO_SetBits(cfg->SDA_PORT, cfg->SDA_PIN);
        } else {
            GPIO_ResetBits(cfg->SDA_PORT, cfg->SDA_PIN);
        }
        byte <<= 1;
        I2CIO_Delay(100);
        GPIO_SetBits(cfg->SCL_PORT, cfg->SCL_PIN);
        I2CIO_Delay(100);
    }
    GPIO_ResetBits(cfg->SCL_PORT, cfg->SCL_PIN);
}


static uint8_t I2CIO_ReadByte(const I2CIO_Cfg_t *cfg)
{
    uint8_t i;
    uint8_t readValue = 0;

    GPIO_SetBits(cfg->SDA_PORT, cfg->SDA_PIN);
    for (i = 0; i < 8; i++) {
        readValue <<= 1;
        GPIO_ResetBits(cfg->SCL_PORT, cfg->SCL_PIN);
        I2CIO_Delay(100);
        GPIO_SetBits(cfg->SCL_PORT, cfg->SCL_PIN);
        I2CIO_Delay(100);
        if (GPIO_ReadInputDataBit(cfg->SDA_PORT, cfg->SDA_PIN)) {
            readValue |= 0x01;
        }
    }
    GPIO_ResetBits(cfg->SCL_PORT, cfg->SCL_PIN);

    return readValue;
}


static void I2CIO_Delay(uint32_t tick)
{
    tick *= 2;
    while (tick) {
        tick--;
        PretendDoingSomething(tick);
    }
}


/// @brief I2C implemented by GPIO, Init.
/// @param[out] cfg I2C config struct, will be used later.
/// @param[in] SCL_Port
/// @param[in] SCL_Pin
/// @param[in] SDA_Port
/// @param[in] SDA_Pin
void I2CIO_Init(I2CIO_Cfg_t *cfg, GPIO_TypeDef *SCL_Port, uint16_t SCL_Pin, GPIO_TypeDef *SDA_Port, uint16_t SDA_Pin)
{
    GPIO_InitTypeDef gpioInit = {0};
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO, ENABLE);
    cfg->SCL_PORT = SCL_Port;
    cfg->SCL_PIN = SCL_Pin;
    cfg->SDA_PORT = SDA_Port;
    cfg->SDA_PIN = SDA_Pin;

    gpioInit.GPIO_Mode = GPIO_Mode_Out_OD;
    gpioInit.GPIO_Pin = cfg->SCL_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(cfg->SCL_PORT, &gpioInit);

    gpioInit.GPIO_Mode = GPIO_Mode_Out_OD;
    gpioInit.GPIO_Pin = cfg->SDA_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(cfg->SDA_PORT, &gpioInit);

    GPIO_SetBits(cfg->SDA_PORT, cfg->SDA_PIN);
    GPIO_SetBits(cfg->SCL_PORT, cfg->SCL_PIN);
}


/// @brief Send data to I2C device.
/// @param[in] cfg I2C config struct.
/// @param[in] addr Device I2C address.
/// @param[in] data Data to be send.
/// @param[in] len Data length.
/// @return Err code.
int32_t I2CIO_SendData(const I2CIO_Cfg_t *cfg, uint8_t addr, const uint8_t *data, uint32_t len)
{
    //printf("addr=0x%02X\r\n", addr);
    //PrintArray("i2c io send data", data, len);
    osKernelLock();
    I2CIO_Start(cfg);
    I2CIO_WriteByte(cfg, addr << 1);
    if (!I2CIO_WaitAck(cfg)) {
        osKernelUnlock();
        return ERR_I2CIO_NOACK;
    }
    for (uint32_t i = 0; i < len; i++) {
        I2CIO_WriteByte(cfg, data[i]);
        if (!I2CIO_WaitAck(cfg)) {
            osKernelUnlock();
            return ERR_I2CIO_NOACK;
        }
    }
    I2CIO_Stop(cfg);
    osKernelUnlock();
    return SUCCESS_CODE;
}


/// @brief Receive data from I2C device.
/// @param[in] cfg I2C config struct.
/// @param[in] addr Device I2C address.
/// @param[out] data Received data.
/// @param[in] len Expected length.
/// @return Err code.
int32_t I2CIO_ReceiveData(const I2CIO_Cfg_t *cfg, uint8_t addr, uint8_t *data, uint32_t len)
{
    //printf("addr=0x%02X\r\n", addr);
    osKernelLock();
    I2CIO_Start(cfg);
    I2CIO_WriteByte(cfg, addr << 1 | 0x01);
    if (!I2CIO_WaitAck(cfg)) {
        //printf("addr=%d,I2CIO rcv no ack\r\n", addr);
        osKernelUnlock();
        return ERR_I2CIO_NOACK;
    }
    for (uint32_t i = 0; i < len; i++) {
        data[i] = I2CIO_ReadByte(cfg);
        if (i != len - 1) {
            I2CIO_SendAck(cfg);
        } else {
            I2CIO_SendNoAck(cfg);
        }
    }
    I2CIO_Stop(cfg);
    osKernelUnlock();
    //PrintArray("i2c io rcv data", data, len);
    return SUCCESS_CODE;
}


/// @brief Search devices from I2C.
/// @param[in] cfg I2C config struct.
/// @return Device address that be found first.
uint8_t I2CIO_SearchDevices(const I2CIO_Cfg_t *cfg)
{
    uint8_t addr;
    uint32_t firstAddr = 0;

    printf("start search\r\n");
    for (addr = 0; addr <= 127; addr++) {
        UserDelay(5);
        I2CIO_Start(cfg);
        I2CIO_WriteByte(cfg, addr << 1);
        if (I2CIO_WaitAck(cfg)) {
            printf("addr=0x%X\r\n", addr);
            firstAddr = addr;
            I2CIO_Stop(cfg);
            break;
        }
        I2CIO_Stop(cfg);
    }
    printf("search over\r\n");
    return firstAddr;
}
