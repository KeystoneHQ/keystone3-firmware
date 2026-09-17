#include "mhscpu_wdt.h"

#define COUNTER_RELOAD_KEY ((uint32_t) 0x76)

void WDT_SetReload(uint32_t Reload)
{
    WDT->WDT_RLD = Reload;
}

void WDT_ReloadCounter(void)
{
    WDT->WDT_CRR = COUNTER_RELOAD_KEY;
}

ITStatus WDT_GetITStatus(void)
{
    ITStatus bitstatus = RESET;

    if ((WDT->WDT_STAT & WDT_STAT_INT) != 0U) {
        bitstatus = SET;
    }

    return bitstatus;
}

void WDT_ClearITPendingBit(void)
{
    (void) WDT->WDT_EOI;
}

void WDT_Enable(void)
{
    WDT->WDT_CR |= WDT_CR_WDT_EN;
}

void WDT_ModeConfig(WDT_ModeTypeDef WDT_Mode)
{
    if (WDT_Mode == WDT_Mode_Interrupt) {
        WDT->WDT_CR |= WDT_CR_RMOD;
    } else if (WDT_Mode == WDT_Mode_CPUReset) {
        WDT->WDT_CR &= ~WDT_CR_RMOD;
    }
}
