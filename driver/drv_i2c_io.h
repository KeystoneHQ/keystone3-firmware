/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: mh1903 gpio i2c driver.
 * Author: leon sun
 * Create: 2023-1-31
 ************************************************************************************************/


#ifndef _DRV_I2C_IO_H
#define _DRV_I2C_IO_H


#include "stdint.h"
#include "stdbool.h"
#include "mhscpu.h"

typedef struct {
    GPIO_TypeDef *SCL_PORT;
    uint16_t SCL_PIN;
    GPIO_TypeDef *SDA_PORT;
    uint16_t SDA_PIN;
} I2CIO_Cfg_t;


/// @brief I2C implemented by GPIO, Init.
/// @param[out] cfg I2C config struct, will be used later.
/// @param[in] SCL_Port
/// @param[in] SCL_Pin
/// @param[in] SDA_Port
/// @param[in] SDA_Pin
void I2CIO_Init(I2CIO_Cfg_t *cfg, GPIO_TypeDef *SCL_Port, uint16_t SCL_Pin, GPIO_TypeDef *SDA_Port, uint16_t SDA_Pin);


/// @brief Send data to I2C device.
/// @param[in] cfg I2C config struct.
/// @param[in] addr Device I2C address.
/// @param[in] data Data to be send.
/// @param[in] len Data length.
/// @return Err code.
int32_t I2CIO_SendData(const I2CIO_Cfg_t *cfg, uint8_t addr, const uint8_t *data, uint32_t len);


/// @brief Receive data from I2C device.
/// @param[in] cfg I2C config struct.
/// @param[in] addr Device I2C address.
/// @param[out] data Received data.
/// @param[in] len Expected length.
/// @return Err code.
int32_t I2CIO_ReceiveData(const I2CIO_Cfg_t *cfg, uint8_t addr, uint8_t *data, uint32_t len);


/// @brief Search devices from I2C.
/// @param[in] cfg I2C config struct.
/// @return Device address that be found first.
uint8_t I2CIO_SearchDevices(const I2CIO_Cfg_t *cfg);

#endif
