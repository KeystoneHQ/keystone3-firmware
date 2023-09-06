/************************ (C) COPYRIGHT Megahuntmicro *************************
 * @file                : mhscpu_crc.h
 * @author              : Megahuntmicro
 * @version             : V1.0.0
 * @date                : 21-October-2014
 * @brief               : This file contains all the functions prototypes for the CRC firmware library
 *****************************************************************************/

#ifndef __MHSCPU_CRC_H
#define __MHSCPU_CRC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mhscpu.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
    CRC_16 = 0x01,
    CRC_16_Modbus = 0x02,
    CRC_CCITT_0xffff = 0x03,
    CRC_CCITT_XModem = 0x04,
    CRC_32 = 0x05
} CRC_Param_TypeDef;

typedef enum {
    CRC_Poly_16_1021 = 0x01,
    CRC_Poly_16_8005 = 0x02,
    CRC_Poly_32_04C11DB7 = 0x03
} CRC_Poly_TypeDef;

typedef enum {
    CRC_PolyMode_Normal = 0x01,
    CRC_PolyMode_Reversed = 0x02,
} CRC_PolyMode_TypeDef;

typedef struct {
    CRC_Poly_TypeDef CRC_Poly;
    CRC_PolyMode_TypeDef CRC_PolyMode;
    uint32_t CRC_Init_Value;
    uint32_t CRC_Xor_Value;
} CRC_ConfigTypeDef;


uint32_t CRC_CalcBlockCRC(uint32_t CRC_type, uint8_t *pData, uint32_t len);
uint32_t CRC_Calc(CRC_ConfigTypeDef *CRC_Config, uint8_t *pData, uint32_t len);


#ifdef __cplusplus
}
#endif

#endif

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
