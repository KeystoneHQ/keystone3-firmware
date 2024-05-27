#include "gui_analyze.h"
#include "gui_web_auth_result_widgets.h"
#include "assert.h"
#include "ui_display_task.h"
#include "librust_c.h"
#include "qrdecode_task.h"
#include "gui_resolve_ur.h"
#include "user_delay.h"
#include "gui_api.h"

#ifndef BTC_ONLY
#include "gui_key_derivation_request_widgets.h"
#else
#include "gui_import_multisig_wallet_info_widgets.h"
#include "gui_create_multisig_wallet_widgets.h"
#endif
#include <gui_views.h>

// The order of the enumeration must be guaranteed
static SetChainData_t g_chainViewArray[] = {
    {REMAPVIEW_BTC, (SetChainDataFunc)GuiSetPsbtUrData},
    {REMAPVIEW_BTC_MESSAGE, (SetChainDataFunc)GuiSetPsbtUrData},
#ifndef BTC_ONLY
    {REMAPVIEW_ETH, (SetChainDataFunc)GuiSetEthUrData},
    {REMAPVIEW_ETH_PERSONAL_MESSAGE, (SetChainDataFunc)GuiSetEthUrData},
    {REMAPVIEW_ETH_TYPEDDATA, (SetChainDataFunc)GuiSetEthUrData},
    {REMAPVIEW_TRX, (SetChainDataFunc)GuiSetTrxUrData},
    {REMAPVIEW_COSMOS, (SetChainDataFunc)GuiSetCosmosUrData},
    {REMAPVIEW_SUI, (SetChainDataFunc)GuiSetSuiUrData},
    {REMAPVIEW_SOL, (SetChainDataFunc)GuiSetSolUrData},
    {REMAPVIEW_SOL_MESSAGE, (SetChainDataFunc)GuiSetSolUrData},
    {REMAPVIEW_APT, (SetChainDataFunc)GuiSetAptosUrData},
    {REMAPVIEW_ADA, (SetChainDataFunc)GuiSetupAdaUrData},
    {REMAPVIEW_XRP, (SetChainDataFunc)GuiSetXrpUrData},
    {REMAPVIEW_AR, (SetChainDataFunc)GuiSetArUrData},
    {REMAPVIEW_AR_MESSAGE, (SetChainDataFunc)GuiSetArUrData},
    {REMAPVIEW_TON, (SetChainDataFunc)GuiSetTonUrData},
    {REMAPVIEW_TON_SIGNPROOF, (SetChainDataFunc)GuiSetTonUrData},
#endif
};

void HandleDefaultViewType(URParseResult *urResult, URParseMultiResult *urMultiResult, UrViewType_t urViewType, bool is_multi)
{
    GuiRemapViewType viewType = ViewTypeReMap(urViewType.viewType);
    if (viewType != REMAPVIEW_BUTT) {
        g_chainViewArray[viewType].func(urResult, urMultiResult, is_multi);
    }
}

void handleURResult(URParseResult *urResult, URParseMultiResult *urMultiResult, UrViewType_t urViewType, bool is_multi)
{
    GuiRemapViewType viewType = ViewTypeReMap(urViewType.viewType);
    switch (urViewType.viewType) {
    case WebAuthResult:
        GuiSetWebAuthResultData(urResult, urMultiResult, is_multi);
        break;
#ifndef BTC_ONLY
    case KeyDerivationRequest:
        GuiSetKeyDerivationRequestData(urResult, urMultiResult, is_multi);
        break;
#else
    case MultisigWalletImport:
        GuiSetMultisigImportWalletDataByQRCode(urResult, urMultiResult, is_multi);
        break;
    case MultisigCryptoImportXpub:
    case MultisigBytesImportXpub:
        GuiSetMultisigImportXpubByQRCode(urResult);
        break;
#endif
    default:
        HandleDefaultViewType(urResult, urMultiResult, urViewType, is_multi);
        break;
    }

    if (urViewType.viewType == WebAuthResult
#ifndef BTC_ONLY
            || urViewType.viewType == KeyDerivationRequest
#else
            || urViewType.viewType == MultisigWalletImport
            || urViewType.viewType == MultisigBytesImportXpub
            || urViewType.viewType == MultisigCryptoImportXpub
#endif
            || viewType != REMAPVIEW_BUTT) {
#ifndef COMPILE_SIMULATOR
        StopQrDecode();
        UserDelay(500);
        GuiApiEmitSignal(SIG_QRCODE_VIEW_SCAN_PASS, &urViewType, sizeof(urViewType));
#else
        printf("urViewType: %d\n", urViewType.viewType);
        GuiEmitSignal(SIG_QRCODE_VIEW_SCAN_PASS, &urViewType, sizeof(urViewType));
#endif
    } else {
        printf("unhandled viewType=%d\r\n", urViewType.viewType);
    }
}