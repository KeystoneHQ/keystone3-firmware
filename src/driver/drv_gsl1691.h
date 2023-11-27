#ifndef _DRV_GSL1691_H
#define _DRV_GSL1691_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"
#include "cmsis_os.h"
#include "hal_touch.h"

#define GSL1691_I2C_ADDR                 0x40


/// @brief GSL1691 touch pad init.
void Gsl1691Init(void);


/// @brief GSL1691 open.
void Gsl1691Open(void);


/// @brief Get touch status, including touch state, X/Y coordinate.
/// @param status TouchStatus struct addr.
int32_t Gsl1691GetStatus(TouchStatus_t *status);


#endif
