#include "drv_mpu.h"
#include "drv_otp.h"

bool g_otpProtect = false;
extern uint32_t _sbss;    // .bss段的起始地址
extern uint32_t _ebss;      // .bss段的结束地址

void MpuDisable(void)
{
    __DMB();
    SCB->SHCSR &= SCB_SHCSR_MEMFAULTENA_Msk;
    MPU->CTRL = 0;
}

void MpuEnable(uint32_t MPU_Control)
{
    MPU->CTRL = MPU_Control | MPU_CTRL_ENABLE_Msk;
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
    __DSB();
    __ISB();
}


void MpuConfiguration(MPU_Region_InitTypeDef* mpuConfig)
{
    MPU->RNR = mpuConfig->Number;

    if ((mpuConfig->Enable) != RESET) {
        MPU->RBAR = mpuConfig->BaseAddress;
        MPU->RASR = ((uint32_t)mpuConfig->DisableExec << MPU_RASR_XN_Pos) |
                    ((uint32_t)mpuConfig->AccessPermission << MPU_RASR_AP_Pos) |
                    ((uint32_t)mpuConfig->TypeExtField << MPU_RASR_TEX_Pos) |
                    ((uint32_t)mpuConfig->IsShareable << MPU_RASR_S_Pos) |
                    ((uint32_t)mpuConfig->IsCacheable << MPU_RASR_C_Pos) |
                    ((uint32_t)mpuConfig->IsBufferable << MPU_RASR_B_Pos) |
                    ((uint32_t)mpuConfig->SubRegionDisable << MPU_RASR_SRD_Pos) |
                    ((uint32_t)mpuConfig->Size << MPU_RASR_SIZE_Pos) |
                    ((uint32_t)mpuConfig->Enable << MPU_RASR_ENABLE_Pos);
    } else {
        MPU->RBAR = 0x0U;
        MPU->RASR = 0x0U;
    }
}


void MpuSetProtection(uint32_t baseAddress, uint32_t regionSize, uint32_t regionNum, uint8_t disableExec,
                      uint8_t accessPermission, uint8_t shareable, uint8_t cacheable, uint8_t bufferable)
{
    MPU_Region_InitTypeDef  mpu;

    mpu.Enable = MPU_REGION_ENABLE;
    mpu.Number = regionNum;
    mpu.BaseAddress = baseAddress;
    mpu.Size = regionSize;
    mpu.SubRegionDisable = 0x00;
    mpu.TypeExtField = MPU_TEX_LEVEL0;
    mpu.AccessPermission = accessPermission;
    mpu.DisableExec = disableExec;
    mpu.IsShareable = shareable;
    mpu.IsCacheable = cacheable;
    mpu.IsBufferable = bufferable;

    MpuDisable();
    MpuConfiguration(&mpu);
    MpuEnable(MPU_PRIVILEGED_DEFAULT);
}


void ConfigureMPUForBSS(void)
{
    uint32_t bss_start = (uint32_t)&_sbss;
    uint32_t bss_size = (uint32_t)&_ebss - (uint32_t)&_sbss;
    
    uint32_t region_size = 32;
    while (region_size < bss_size && region_size < 0x00100000) {
        region_size *= 2;
    }
    
    MpuSetProtection(
        bss_start,
        region_size,
        MPU_REGION_NUMBER2,
        MPU_INSTRUCTION_ACCESS_DISABLE,
        MPU_REGION_PRIV_RW,
        MPU_ACCESS_NOT_SHAREABLE,
        MPU_ACCESS_CACHEABLE,
        MPU_ACCESS_BUFFERABLE
    );
}

void MpuInit(void)
{
    ConfigureMPUForBSS();
    MpuSetOtpProtection(true);
}

bool GetOtpProtection(void)
{
    return g_otpProtect;
}

// void MpuSetOtpProtection(bool noAccess)
// {
//     uint8_t accessPermission = noAccess ? MPU_REGION_NO_ACCESS : MPU_REGION_FULL_ACCESS;
//     MpuSetProtection(OTP_ADDR_BASE,
//                      MPU_REGION_SIZE_1KB,
//                      MPU_REGION_NUMBER1,
//                      MPU_INSTRUCTION_ACCESS_DISABLE,
//                      accessPermission,
//                      MPU_ACCESS_SHAREABLE,
//                      MPU_ACCESS_CACHEABLE,
//                      MPU_ACCESS_BUFFERABLE);
// }

