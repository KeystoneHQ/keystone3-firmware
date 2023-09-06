
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MHSCPU_CONF_H
#define __MHSCPU_CONF_H

/* Includes ------------------------------------------------------------------*/
/* Uncomment the line below to enable peripheral header file inclusion */
#include "mhscpu_uart.h"
#include "mhscpu_exti.h"
#include "mhscpu_sysctrl.h"
#include "mhscpu_spi.h"
#include "mhscpu_dma.h"
#include "mhscpu_wdt.h"
#include "mhscpu_crc.h"
#include "mhscpu_timer.h"
#include "mhscpu_gpio.h"
#include "mhscpu_rtc.h"
#include "mhscpu_sensor.h"
#include "mhscpu_bpk.h"
#include "mhscpu_trng.h"
#include "mhscpu_psram.h"
#include "mhscpu_lcdi.h"
#include "mhscpu_i2c.h"
#include "mhscpu_dcmi.h"
#include "mhscpu_sensor.h"
#include "mhscpu_ssc.h"
#include "mhscpu_adc.h"
#include "mhscpu_otp.h"
#include "misc.h" /* High level functions for NVIC and SysTick (add-on to CMSIS functions) */

#include "string.h"
#include "assert.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line below to expanse the "assert_param" macro in the
   Standard Peripheral Library drivers code */
/* #define USE_FULL_ASSERT    1 */

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls ASSERT function
  *   which reports the name of the source file and the source
  *   line number of the call that failed.
  *   If expr is true, it returns no value.
  * @retval None
  */
#define assert_param        ASSERT
/* Exported functions ------------------------------------------------------- */

#else
#define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */

#endif

