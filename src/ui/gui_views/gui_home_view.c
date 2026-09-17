#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_home_widgets.h"
#include "gui_pending_hintbox.h"
#include "gui_lock_widgets.h"
#include "gui_scan_widgets.h"

#ifdef WEB3_VERSION
#include "secret_cache.h"
#include "gui_attention_hintbox.h"
#include "rsa.h"
static bool g_arReceiveAfterSetup;
static bool g_arAllowGenerate;
static uint8_t g_arSetupAccount = 0xFF;

void GuiHomeCancelArSetup(void)
{
    if (g_arSetupAccount != 0xFF) {
        g_arSetupAccount = 0xFF;
        g_arAllowGenerate = false;
        g_arReceiveAfterSetup = false;
        ClearSecretCache();
        GuiCloseAttentionHintbox();
    }
}
#endif

static int32_t GuiHomeViewInit(void)
{
    GuiHomeAreaInit();
    GuiModeGetWalletDesc();
    return SUCCESS_CODE;
}

static int32_t GuiHomeViewDeInit(void)
{
#ifdef WEB3_VERSION
    GuiHomeCancelArSetup();
    ClearSecretCache();
    g_arAllowGenerate = false;
    g_arReceiveAfterSetup = false;
#endif
    GuiHomeDeInit();
    GuiPendingHintBoxMoveToTargetParent(lv_scr_act());
    return SUCCESS_CODE;
}

int32_t GuiHomeViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiHomeViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiHomeViewDeInit();
    case GUI_EVENT_DISACTIVE:
#ifdef WEB3_VERSION
        GuiHomeCancelArSetup();
#endif
        GuiHomeDisActive();
        break;
    case GUI_EVENT_RESTART:
        GuiHomeRestart();
        break;
    case GUI_EVENT_CHANGE_LANGUAGE:
        GuiHomeRestart();
        return ERR_GUI_UNHANDLED;
    case GUI_EVENT_REFRESH:
        GuiHomeRefresh();
        if (param != NULL) {
            GuiModeGetWalletDesc();
        }
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiHomeRefresh();
        break;
    case SIG_INIT_GET_CURRENT_WALLET_DESC:
        GuiHomeSetWalletDesc((WalletDesc_t *)param);
        break;
#ifdef WEB3_VERSION
    case SIG_SETUP_RSA_PRIVATE_KEY_PARSER_CONFIRM:
    case SIG_SETUP_RSA_PRIVATE_KEY_RECEIVE_CONFIRM:
        if (param != NULL) {
            GuiHomeCancelArSetup();
        }
        g_arReceiveAfterSetup = usEvent == SIG_SETUP_RSA_PRIVATE_KEY_RECEIVE_CONFIRM;
        g_arAllowGenerate = g_arReceiveAfterSetup &&
                            (param == NULL || (usLen == sizeof(bool) && *(bool *)param));
        if (g_arAllowGenerate && g_arSetupAccount != 0xFF) {
            if (g_arSetupAccount != GetCurrentAccountIndex()) {
                GuiHomeCancelArSetup();
                break;
            }
            g_arSetupAccount = 0xFF;
            if (SecretCacheGetPassword() != NULL) {
                GuiRemoveKeyboardWidget(true);
                g_arAllowGenerate = false;
                break;
            }
        }
        GuiShowRsaSetupasswordHintbox();
        break;
    case SIG_SETUP_RSA_PRIVATE_KEY_RSA_VERIFY_PASSWORD_FAIL:
        if (param != NULL) {
            PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
            uint16_t sig = passwordVerifyResult->signal;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenPassCode(false);
                GuiHomePasswordErrorCount(param);
                return SUCCESS_CODE;
            }
        }
        GuiHomePasswordErrorCount(param);
        break;
    case SIG_SETUP_RSA_PRIVATE_KEY_RSA_VERIFY_PASSWORD_PASS:
        printf("SIG_VERIFY_PASSWORD_PASS\n");
        if (param != NULL) {
            uint16_t sig = *(uint16_t *)param;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenToHome();
                return SUCCESS_CODE;
            }
        }
        GuiRemoveKeyboardWidget(g_arAllowGenerate);
        g_arAllowGenerate = false;
        break;
    case SIG_SETUP_RSA_PRIVATE_KEY_WITH_PASSWORD_START:
        GuiPendingHintBoxRemove();
        if (param != NULL && usLen == sizeof(bool) && *(bool *)param) {
            GuiPendingHintBoxOpen(_("PreparingArAddress"), NULL);
        } else {
            GuiPendingHintBoxOpen(_("InitializingRsaTitle"), _("FindingRsaPrimes"));
        }
        break;
    case SIG_SETUP_RSA_PRIVATE_KEY_GENERATE_ADDRESS:
        GuiUpdatePendingHintBoxSubtitle(_("GeneratingRsaAddress"));
        break;
    case SIG_SETUP_RSA_PRIVATE_KEY_WITH_PASSWORD_PASS:
        if (param == NULL || usLen != 44 || strnlen_s(param, usLen) != 43) {
            GuiPendingHintBoxRemove();
            ClearSecretCache();
            GuiCreateErrorCodeWindow(ERR_GENERAL_FAIL, NULL, NULL);
            g_arReceiveAfterSetup = false;
            break;
        }
        if (g_arReceiveAfterSetup) {
            GuiShowRsaInitializatioCompleteHintbox(true, param);
        } else {
            GuiPendingHintBoxRemove();
            ClearSecretCache();
            GuiArTransactionReady(param);
        }
        g_arReceiveAfterSetup = false;
        break;
    case SIG_SETUP_RSA_PRIVATE_KEY_WRITE_FAIL:
        GuiPendingHintBoxRemove();
        if (param != NULL && usLen == sizeof(int32_t)) {
            if (ArKeyNeedsSetup(*(int32_t *)param)) {
                g_arSetupAccount = GetCurrentAccountIndex();
                if (g_arReceiveAfterSetup) {
                    GuiHomeShowArSetupNotice();
                } else {
                    GuiHomeCancelArSetup();
                    ClearSecretCache();
                    GuiCreateErrorCodeWindow(*(int32_t *)param, NULL, NULL);
                }
            } else {
                ClearSecretCache();
                GuiCreateErrorCodeWindow(*(int32_t *)param, NULL, NULL);
            }
        } else {
            ClearSecretCache();
        }
        g_arReceiveAfterSetup = false;
        break;
#endif

#ifndef BTC_ONLY
    case SIG_CLEAR_HOME_PAGE_INDEX:
        ClearHomePageCurrentIndex();
        break;
#endif
    case SIG_QRCODE_VIEW_SCAN_FAIL:
        GuiScanResult(false, param);
        break;
    case SIG_QRCODE_VIEW_SCAN_PASS:
        GuiScanResult(true, param);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_homeView = {
    .id = SCREEN_HOME,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiHomeViewEventProcess,
};
