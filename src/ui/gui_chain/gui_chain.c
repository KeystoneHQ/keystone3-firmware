#include "gui_chain.h"

PtrT_TransactionCheckResult CheckScanResult(ViewType viewType)
{
    switch (ViewTypeReMap(viewType)) {
        case REMAPVIEW_BTC:
            return GuiGetPsbtCheckResult();
        case REMAPVIEW_ETH:
        case REMAPVIEW_ETH_PERSONAL_MESSAGE:
        case REMAPVIEW_ETH_TYPEDDATA:
            return GuiGetEthCheckResult();
        case REMAPVIEW_TRX:
            return GuiGetTrxCheckResult();
        case REMAPVIEW_COSMOS:
            return GuiGetCosmosCheckResult();
        case REMAPVIEW_SUI:
            return GuiGetSuiCheckResult();
        case REMAPVIEW_SOL:
        case REMAPVIEW_SOL_MESSAGE:
            return GuiGetSolCheckResult();
        case REMAPVIEW_APT:
            return GuiGetAptosCheckResult();
        case REMAPVIEW_ADA:
            return GuiGetAdaCheckResult();
        default:
            return NULL;
    }
}