#ifndef _DRV_OTP_H
#define _DRV_OTP_H

#include "stdint.h"
#include "stdbool.h"
#include "mhscpu.h"

#define OTA_ADDR_FACTORY_BASE       0x40009400

#define OTP_ADDR_BASE               0x40009500
#define OTP_ADDR_SALT               OTP_ADDR_BASE
#define OTP_ADDR_AES_KEY            OTP_ADDR_SALT + 32
#define OTP_ADDR_ATECC608B          OTP_ADDR_SALT + 64
#define OTP_ADDR_DS28S60            OTP_ADDR_ATECC608B + 96
#define OTP_ADDR_TAMPER             OTP_ADDR_DS28S60 + 96

#define OTP_ADDR_SN                 0x40009700

#define OTP_ADDR_WEB_AUTH_RSA_KEY   0x40009800

#define OTP_ADDR_UPDATE_PUB_KEY     0x40009C00

#if (OTP_ADDR_BASE % 256 != 0)
#error "OTP_ADDR_BASE must be 256 byte aligned."
#endif


#if (OTP_ADDR_TAMPER + 4 - OTP_ADDR_BASE > 512)
#error "OTP overlap"
#endif


int32_t WriteOtpData(uint32_t addr, const uint8_t *data, uint32_t len);
bool ReadTamperFlag(void);
int32_t WriteTamperFlag(void);

#endif

