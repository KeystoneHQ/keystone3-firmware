#include "hal_mh1903_i2c.h"
//#include "atca_hal.h"
#include "cryptoauthlib.h"
#include "drv_i2c_io.h"
#include "hardware_version.h"


#define EVT0_I2CIO_SCL_PORT             GPIOG
#define EVT0_I2CIO_SDA_PORT             GPIOG
#define EVT0_I2CIO_SCL_PIN              GPIO_Pin_11
#define EVT0_I2CIO_SDA_PIN              GPIO_Pin_12

#define EVT1_I2CIO_SCL_PORT             GPIOG
#define EVT1_I2CIO_SDA_PORT             GPIOG
#define EVT1_I2CIO_SCL_PIN              GPIO_Pin_12
#define EVT1_I2CIO_SDA_PIN              GPIO_Pin_11


I2CIO_Cfg_t g_i2cIoCfg;

/** \brief initialize an I2C interface using given config
 * \param[in] hal - opaque ptr to HAL data
 * \param[in] cfg - interface configuration
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_init(ATCAIface iface, ATCAIfaceCfg *cfg)
{
    GPIO_TypeDef *sclPort, *sdaPort;
    uint16_t sclPin, sdaPin;

    if (GetHardwareVersion() == VERSION_EVT0) {
        sclPort = EVT0_I2CIO_SCL_PORT;
        sdaPort = EVT0_I2CIO_SDA_PORT;
        sclPin = EVT0_I2CIO_SCL_PIN;
        sdaPin = EVT0_I2CIO_SDA_PIN;
    } else {
        sclPort = EVT1_I2CIO_SCL_PORT;
        sdaPort = EVT1_I2CIO_SDA_PORT;
        sclPin = EVT1_I2CIO_SCL_PIN;
        sdaPin = EVT1_I2CIO_SDA_PIN;
    }
    I2CIO_Init(&g_i2cIoCfg, sclPort, sclPin, sdaPort, sdaPin);

    return ATCA_SUCCESS;
}


/** \brief HAL implementation of I2C post init
 * \param[in] iface  instance
 * \return ATCA_SUCCESS
 */
ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
    return ATCA_SUCCESS;
}


/** \brief HAL implementation of I2C send
 * \param[in] iface         instance
 * \param[in] word_address  device transaction type
 * \param[in] txdata        pointer to space to bytes to send
 * \param[in] txlength      number of bytes to send
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t address, uint8_t *txdata, int txlength)
{
    I2CIO_SendData(&g_i2cIoCfg, address >> 1, txdata, txlength);
    return ATCA_SUCCESS;
}


/** \brief HAL implementation of I2C receive function
 * \param[in]    iface          Device to interact with.
 * \param[in]    address        Device address
 * \param[out]   rxdata         Data received will be returned here.
 * \param[in,out] rxlength      As input, the size of the rxdata buffer.
 *                              As output, the number of bytes received.
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t address, uint8_t *rxdata, uint16_t *rxlength)
{
    I2CIO_ReceiveData(&g_i2cIoCfg, address >> 1, rxdata, *rxlength);
    return ATCA_SUCCESS;
}


/** \brief manages reference count on given bus and releases resource if no more refences exist
 * \param[in] hal_data - opaque pointer to hal data structure - known only to the HAL implementation
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_release(void *hal_data)
{
    return ATCA_SUCCESS;
}


/** \brief Perform control operations for the kit protocol
 * \param[in]     iface          Interface to interact with.
 * \param[in]     option         Control parameter identifier
 * \param[in]     param          Optional pointer to parameter value
 * \param[in]     paramlen       Length of the parameter
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_control(ATCAIface iface, uint8_t option, void* param, size_t paramlen)
{
    return ATCA_SUCCESS;
}
