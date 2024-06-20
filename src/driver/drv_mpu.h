#ifndef _MPU_H
#define _MPU_H

#include "mhscpu.h"
#include "drv_otp.h"

#define MPU_HFNMI_PRIVDEF_NONE              (0x00000000U)
#define MPU_HARDFAULT_NMI                   (MPU_CTRL_HFNMIENA_Msk)
#define MPU_PRIVILEGED_DEFAULT              (MPU_CTRL_PRIVDEFENA_Msk)
#define MPU_HFNMI_PRIVDEF                   (MPU_CTRL_HFNMIENA_Msk | MPU_CTRL_PRIVDEFENA_Msk)

#define MPU_REGION_ENABLE                   ((uint8_t)0x01)
#define MPU_REGION_DISABLE                  ((uint8_t)0x00)

#define MPU_INSTRUCTION_ACCESS_ENABLE       ((uint8_t)0x00)
#define MPU_INSTRUCTION_ACCESS_DISABLE      ((uint8_t)0x01)

#define MPU_ACCESS_SHAREABLE                ((uint8_t)0x01)
#define MPU_ACCESS_NOT_SHAREABLE            ((uint8_t)0x00)

#define MPU_ACCESS_CACHEABLE                ((uint8_t)0x01)
#define MPU_ACCESS_NOT_CACHEABLE            ((uint8_t)0x00)

#define MPU_ACCESS_BUFFERABLE               ((uint8_t)0x01)
#define MPU_ACCESS_NOT_BUFFERABLE           ((uint8_t)0x00)

#define MPU_TEX_LEVEL0                      ((uint8_t)0x00)
#define MPU_TEX_LEVEL1                      ((uint8_t)0x01)
#define MPU_TEX_LEVEL2                      ((uint8_t)0x02)

#define MPU_REGION_SIZE_32B                 ((uint8_t)0x04)
#define MPU_REGION_SIZE_64B                 ((uint8_t)0x05)
#define MPU_REGION_SIZE_128B                ((uint8_t)0x06)
#define MPU_REGION_SIZE_256B                ((uint8_t)0x07)
#define MPU_REGION_SIZE_512B                ((uint8_t)0x08)
#define MPU_REGION_SIZE_1KB                 ((uint8_t)0x09)
#define MPU_REGION_SIZE_2KB                 ((uint8_t)0x0A)
#define MPU_REGION_SIZE_4KB                 ((uint8_t)0x0B)
#define MPU_REGION_SIZE_8KB                 ((uint8_t)0x0C)
#define MPU_REGION_SIZE_16KB                ((uint8_t)0x0D)
#define MPU_REGION_SIZE_32KB                ((uint8_t)0x0E)
#define MPU_REGION_SIZE_64KB                ((uint8_t)0x0F)
#define MPU_REGION_SIZE_128KB               ((uint8_t)0x10)
#define MPU_REGION_SIZE_256KB               ((uint8_t)0x11)
#define MPU_REGION_SIZE_512KB               ((uint8_t)0x12)
#define MPU_REGION_SIZE_1M                  ((uint8_t)0x13)
#define MPU_REGION_SIZE_2M                  ((uint8_t)0x14)
#define MPU_REGION_SIZE_4M                  ((uint8_t)0x15)
#define MPU_REGION_SIZE_8M                  ((uint8_t)0x16)
#define MPU_REGION_SIZE_16M                 ((uint8_t)0x17)
#define MPU_REGION_SIZE_32M                 ((uint8_t)0x18)
#define MPU_REGION_SIZE_64M                 ((uint8_t)0x19)
#define MPU_REGION_SIZE_128M                ((uint8_t)0x1A)
#define MPU_REGION_SIZE_256M                ((uint8_t)0x1B)
#define MPU_REGION_SIZE_512M                ((uint8_t)0x1C)
#define MPU_REGION_SIZE_1G                  ((uint8_t)0x1D)
#define MPU_REGION_SIZE_2G                  ((uint8_t)0x1E)
#define MPU_REGION_SIZE_4G                  ((uint8_t)0x1F)

#define MPU_REGION_NO_ACCESS                ((uint8_t)0x00)
#define MPU_REGION_PRIV_RW                  ((uint8_t)0x01)
#define MPU_REGION_PRIV_RW_URO              ((uint8_t)0x02)
#define MPU_REGION_FULL_ACCESS              ((uint8_t)0x03)
#define MPU_REGION_PRIV_RO                  ((uint8_t)0x05)
#define MPU_REGION_PRIV_RO_URO              ((uint8_t)0x06)

#define MPU_REGION_NUMBER0                  ((uint8_t)0x00)
#define MPU_REGION_NUMBER1                  ((uint8_t)0x01)
#define MPU_REGION_NUMBER2                  ((uint8_t)0x02)
#define MPU_REGION_NUMBER3                  ((uint8_t)0x03)
#define MPU_REGION_NUMBER4                  ((uint8_t)0x04)
#define MPU_REGION_NUMBER5                  ((uint8_t)0x05)
#define MPU_REGION_NUMBER6                  ((uint8_t)0x06)
#define MPU_REGION_NUMBER7                  ((uint8_t)0x07)

typedef struct {
    uint8_t Enable;
    uint8_t Number;
    uint32_t BaseAddress;
    uint8_t Size;
    uint8_t SubRegionDisable;
    uint8_t TypeExtField;
    uint8_t AccessPermission;
    uint8_t DisableExec;
    uint8_t IsShareable;
    uint8_t IsCacheable;
    uint8_t IsBufferable;
} MPU_Region_InitTypeDef;


void MpuConfiguration(MPU_Region_InitTypeDef* MPU_Init);

void MpuSetProtection(uint32_t BaseAddress, uint32_t RegionSize, uint32_t RegionNum, uint8_t DisableExec,
                      uint8_t AccessPermission, uint8_t Shareable, uint8_t Cacheable, uint8_t Bufferable);

extern bool g_otpProtect;

#if 1
#define MpuSetOtpProtection(x)     {uint8_t accessPermission = x ? MPU_REGION_NO_ACCESS : MPU_REGION_FULL_ACCESS; g_otpProtect = x;\
    MpuSetProtection(OTP_ADDR_BASE,MPU_REGION_SIZE_1KB,MPU_REGION_NUMBER1,MPU_INSTRUCTION_ACCESS_DISABLE, accessPermission,MPU_ACCESS_SHAREABLE,MPU_ACCESS_CACHEABLE,MPU_ACCESS_BUFFERABLE); \
    printf("%s %d.\n", __func__, __LINE__);}
#else
#define MpuSetOtpProtection(x)
#endif

#endif

