#ifndef _DRV_CST726_H
#define _DRV_CST726_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"
#include "cmsis_os.h"
#include "hal_touch.h"

#define CST726_I2C_ADDR                 0x15


/// @brief CST726 touch pad init.
void Cst726Init(void);


/// @brief CST726 open.
void Cst726Open(void);


/// @brief Get touch status, including touch state, X/Y coordinate.
/// @param status TouchStatus struct addr.
int32_t Cst726GetStatus(TouchStatus_t *status);



#endif
