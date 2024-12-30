#include "gui_chain.h"
#include "gui_analyze.h"

static GetLabelDataFunc GuiMoneroTextFuncGet(char *type);

GetCustomContainerFunc GetOtherChainCustomFunc(char *funcName)
{
    if (!strcmp(funcName, "GuiZcashOverview")) {
        return GuiZcashOverview;
    } else if (!strcmp(funcName, "GuiShowXmrOutputsDetails")) {
        return GuiShowXmrOutputsDetails;
    } else if (!strcmp(funcName, "GuiShowXmrTransactionDetails")) {
        return GuiShowXmrTransactionDetails;
    } else if (!strcmp(funcName, "GuiShowXmrTransactionOverview")) {
        return GuiShowXmrTransactionOverview;
    }
    return NULL;
}

GetLabelDataFunc GuiOtherChainTextFuncGet(char *type, GuiRemapViewType remapIndex)
{
    switch (remapIndex) {
    case REMAPVIEW_XMR_OUTPUT:
    case REMAPVIEW_XMR_UNSIGNED:
        return GuiMoneroTextFuncGet(type);
    default:
        break;
    }

    return NULL;
}

static GetLabelDataFunc GuiMoneroTextFuncGet(char *type)
{
    if (!strcmp(type, "GetXmrTxoCount")) {
        return GetXmrTxoCount;
    } else if (!strcmp(type, "GetXmrTotalAmount")) {
        return GetXmrTotalAmount;
    }
    return NULL;
}