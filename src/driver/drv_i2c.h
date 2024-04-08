#ifndef _DRV_I2C_H
#define _DRV_I2C_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

/// @brief I2C hardware init
/// @param
void I2cInit(void);

/// @brief I2C send data sequence.
/// @param addr I2C addr(high 7-bit).
/// @param data send data buffer.
/// @param len send data buffer length.
void I2cSendData(uint8_t addr, const uint8_t *data, uint32_t len);

/// @brief Send u16 cmd and then send data sequence.
/// @param addr I2C addr(high 7-bit).
/// @param cmd u16 cmd, send MSB first.
/// @param data send data buffer.
/// @param len send data buffer length.
void I2cSendCmdAndData(uint8_t addr, uint16_t cmd, const uint8_t *data, uint32_t len);

/// @brief Send u16 cmd and then get i2c data sequence.
/// @param addr I2C addr(high 7-bit).
/// @param cmd u16 cmd, send MSB first.
/// @param data received data.
/// @param len received data length.
void I2cSendCmdAndReceiveData(uint8_t addr, uint16_t cmd, uint8_t *data, uint32_t len);

/// @brief Send u8 addr and then get i2c data sequence.
/// @param addr I2C addr(high 7-bit).
/// @param sendData send data.
/// @param sendLen send data len.
/// @param rcvData received data.
/// @param rcvLen received data length.
void I2cSendAndReceiveData(uint8_t addr, const uint8_t *sendData, uint32_t sendLen, uint8_t *rcvData, uint32_t rcvLen);

#endif
