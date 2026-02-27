#include "drv_sys.h"
#include "mhscpu.h"
#include "stdio.h"
#include "version.h"

void SystemClockInit(void)
{
    SYSCTRL_SYSCLKSourceSelect(SELECT_EXT12M);
    //SYSCTRL_SYSCLKSourceSelect(SELECT_INC12M);
    SYSCTRL_PLLConfig(SYSCTRL_PLL_204MHz);
    SYSCTRL_PLLDivConfig(SYSCTRL_PLL_Div_None);
    SYSCTRL_HCLKConfig(SYSCTRL_HCLK_Div_None);
    SYSCTRL_PCLKConfig(SYSCTRL_PCLK_Div2);
}

void NvicInit(void)
{
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);
    NVIC_SetPriority(MemoryManagement_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_3, 0, 0));
    NVIC_SetPriority(BusFault_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_3, 0, 0));
    NVIC_SetPriority(UsageFault_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_3, 0, 0));
    NVIC_SetPriority(SVCall_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_3, 0, 0));
    NVIC_SetPriority(DebugMonitor_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_3, 0, 0));
    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_3, 7, 0));
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_PriorityGroup_3, 7, 0));
}

void PrintSystemInfo(void)
{
    SYSCTRL_ClocksTypeDef ClocksStructure;

    printf("keystone on mh1903\r\n");
    printf("compiler:");
#ifdef __ARMCC_VERSION
    printf("ARMCC\r\n");
#elif defined(__ICCARM__)
    printf("ICCARM\r\n");
#elif defined(__GNUC__)
    printf("GNUC\r\n");
#endif
    SYSCTRL_GetClocksFreq(&ClocksStructure);
    printf("PLL_Frequency :%dHz\r\n", ClocksStructure.PLL_Frequency);
    printf("CPU_Frequency :%dHz\r\n", ClocksStructure.CPU_Frequency);
    printf("HCLK_Frequency:%dHz\r\n", ClocksStructure.HCLK_Frequency);
    printf("PCLK_Frequency:%dHz\r\n", ClocksStructure.PCLK_Frequency);
    printf("SYSCTRL->FREQ_SEL=0x%08X\r\n", SYSCTRL->FREQ_SEL);
    printf("SYSCTRL->FREQ_SEL=0x%08X\r\n", SYSCTRL->HCLK_1MS_VAL);
    printf("%s\n", GetSoftwareVersionString());
}
