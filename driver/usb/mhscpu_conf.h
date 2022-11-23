/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MHSCPU_CONF_H
#define __MHSCPU_CONF_H

/* Includes ------------------------------------------------------------------*/
/* Uncomment the line below to enable peripheral header file inclusion */
#include "mhscpu_gpio.h"
#include "mhscpu_sysctrl.h"
#include "mhscpu_uart.h"

#include "misc.h" /* High level functions for NVIC and SysTick (add-on to CMSIS functions) */

#include "string.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line below to expanse the "assert_param" macro in the
   Standard Peripheral Library drivers code */
/* #define USE_FULL_ASSERT    1 */

/* Exported macro ------------------------------------------------------------*/
#ifdef USE_FULL_ASSERT

/**
 * @brief  The assert_param macro is used for function's parameters check.
 * @param  expr: If expr is false, it calls ASSERT function
 *   which reports the name of the source file and the source
 *   line number of the call that failed.
 *   If expr is true, it returns no value.
 * @retval None
 */
#define assert_param          ASSERT
/* Exported functions ------------------------------------------------------- */
#else
#define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */

#endif
