#include "gui.h"
#include "gui_page.h"
#include "librust_c.h"
#include "keystore.h"
#include "user_memory.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_global_resources.h"
#include "gui_obj.h"
#include "general/eapdu_services/service_resolve_ur.h"
#include "secret_cache.h"
#include "fingerprint_process.h"
#include "version.h"
#include "gui_keyboard_hintbox.h"
#include "gui_lock_widgets.h"
#include "account_public_info.h"
#include "gui_key_derivation_request_widgets.h"
#ifdef COMPILE_SIMULATOR
#include "simulator_model.h"
#endif

typedef struct KeyDerivationWidget {
    uint8_t currentTile;
    PageWidget_t *pageWidget;
    lv_obj_t *tileView;
    lv_obj_t *qrCode;
    lv_obj_t *usbCont;
} KeyDerivationWidget_t;

typedef enum {
    TRANSACTION_MODE_QR_CODE = 0,
    TRANSACTION_MODE_USB,
} TransactionMode;

typedef enum {
    TILE_APPROVE,
    TILE_QRCODE,
    TILE_USB_CONNECT,

    TILE_BUTT,
} PAGE_TILE;

typedef struct HardwareCallResult {
    bool isLegal;
    const char *title;
    const char *message;
} HardwareCallResult_t;

typedef enum HardwareCallV1AdaDerivationAlgo {
    HD_STANDARD_ADA = 0,
    HD_LEDGER_BITBOX_ADA,
} ADA_DERIVATION_ALGO;

// global variable to save the selected derivation path
static ADA_DERIVATION_ALGO selected_ada_derivation_algo = HD_STANDARD_ADA;

static void *g_data;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static bool g_isMulti;
static KeyDerivationWidget_t g_keyDerivationTileView;
static QRHardwareCallData *g_callData = NULL;
static Response_QRHardwareCallData *g_response = NULL;
static WALLET_LIST_INDEX_ENUM g_walletIndex;
static lv_obj_t *g_openMoreHintBox;
static lv_obj_t *g_derivationPathCont = NULL;
static lv_obj_t *g_derivationPathConfirmBtn = NULL;
static lv_obj_t *g_derivationTypeCheck[2] = {0, 0};
static uint32_t g_currentSelectedPathIndex = 0;;
static lv_obj_t *g_egCont = NULL;
static lv_obj_t *g_egAddressIndex[2];
static lv_obj_t *g_egAddress[2];
static char g_derivationPathAddr[2][2][BUFFER_SIZE_128];
static bool g_isUsb = false;
static bool g_isUsbPassWordCheck = false;
static bool g_hasAda = false;

static void RecalcCurrentWalletIndex(char *origin);

static void GuiCreateApproveWidget(lv_obj_t *parent);
static void GuiCreateHardwareCallApproveWidget(lv_obj_t *parent);
static void GuiCreateQRCodeWidget(lv_obj_t *parent);
static void GuiCreateCommonHardWareCallQRCodeWidget(lv_obj_t *parent);
static void OnApproveHandler(lv_event_t *e);
static void OnReturnHandler(lv_event_t *e);
static void ModelParseQRHardwareCall();
static UREncodeResult *ModelGenerateSyncUR(void);
static void OpenTutorialHandler(lv_event_t *e);
static void OpenMoreHandler(lv_event_t *e);
static void OpenMoreHandlerWithOutDerivationPath(lv_event_t *e);
static void QRCodePause(bool pause);
static const char *GetChangeDerivationAccountType(int i);
static void SetCurrentSelectedIndex(uint8_t index);
static uint32_t GetCurrentSelectedIndex();
static void SelectDerivationHandler(lv_event_t *e);
static void CloseDerivationHandler(lv_event_t *e);
static char *GetChangeDerivationPathDesc(void);
static void GetCardanoEgAddress(void);
static void UpdateCardanoEgAddress(uint8_t index);
static void ShowEgAddressCont(lv_obj_t *egCont);
static void OpenDerivationPath();
static void ChangeDerivationPathHandler(lv_event_t *e);
static void UpdateConfirmBtn(bool update);
static void GuiConnectUsbEntranceWidget(lv_obj_t *parent);
static void GuiConnectUsbCreateImg(lv_obj_t *parent);
static void RejectButtonHandler(lv_event_t *e);
static void ApproveButtonHandler(lv_event_t *e);
static void GuiConnectUsbPasswordPass(void);
static void FreeKeyDerivationRequestMemory(void);
static KeyboardWidget_t *g_keyboardWidget = NULL;
static void GuiShowKeyBoardDialog(lv_obj_t *parent);
static HardwareCallResult_t CheckHardwareCallRequestIsLegal(void);
#ifdef WEB3_VERSION
static void SaveHardwareCallVersion1AdaDerivationAlgo(lv_event_t *e);
static AdaXPubType GetAccountType(void);
static void SetAccountType(uint8_t index);
static bool IsCardano();
static uint8_t GetXPubIndexByPath(char *path);
#endif

void GuiSetKeyDerivationRequestData(void *urResult, void *multiResult, bool is_multi)
{
    FreeKeyDerivationRequestMemory();
    g_urResult = urResult;
    g_urMultiResult = multiResult;
    g_isMulti = is_multi;
    g_data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
}

void FreeKeyDerivationRequestMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    if (g_response != NULL) {
        free_Response_QRHardwareCallData(g_response);
        g_response = NULL;
    }
}

static char *GetChangeDerivationPathDesc(void)
{
    return GetDerivationPathDescs(ADA_DERIVATION_PATH_DESC)[GetCurrentSelectedIndex()];
}

static void RecalcCurrentWalletIndex(char *origin)
{
    if (strcmp("eternl", origin) == 0) {
        g_walletIndex = WALLET_LIST_ETERNL;
    } else if (strcmp("Typhon Extension", origin) == 0) {
        g_walletIndex = WALLET_LIST_TYPHON;
    } else if (strcmp("Leap Mobile", origin) == 0) {
        g_walletIndex = WALLET_LIST_LEAP;
    } else if (strcmp("Begin", origin) == 0) {
        g_walletIndex = WALLET_LIST_BEGIN;
    } else if (strcmp("Medusa", origin) == 0) {
        g_walletIndex = WALLET_LIST_MEDUSA;
    } else {
        g_walletIndex = WALLET_LIST_ETERNL;
    }
}

void GuiKeyDerivationRequestInit(bool isUsb)
{
    g_isUsb = isUsb;
    g_isUsbPassWordCheck = false;
    g_hasAda = false;
    GUI_PAGE_DEL(g_keyDerivationTileView.pageWidget);
    g_keyDerivationTileView.pageWidget = CreatePageWidget();
    SetNavBarLeftBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    lv_obj_t *tileView = GuiCreateTileView(g_keyDerivationTileView.pageWidget->contentZone);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, TILE_APPROVE, 0, LV_DIR_HOR);
    ModelParseQRHardwareCall();
    // choose different animate qr widget by hardware call  version
    if (strcmp("1", g_callData->version) == 0) {
        GuiCreateHardwareCallApproveWidget(tile);
    } else {
        GuiCreateApproveWidget(tile);
    }
    RecalcCurrentWalletIndex(g_response->data->origin);
    SetWallet(g_keyDerivationTileView.pageWidget->navBarWidget, g_walletIndex, NULL);
    SetNavBarRightBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_MORE_INFO, OpenMoreHandler, NULL);
    tile = lv_tileview_add_tile(tileView, TILE_QRCODE, 0, LV_DIR_HOR);
    // choose different animate qr widget by hardware call  version
    if (strcmp("1", g_callData->version) == 0) {
        GuiCreateCommonHardWareCallQRCodeWidget(tile);
    } else {
        GuiCreateQRCodeWidget(tile);
    }

    tile = lv_tileview_add_tile(tileView, TILE_USB_CONNECT, 0, LV_DIR_HOR);
    GuiConnectUsbEntranceWidget(tile);
    g_keyDerivationTileView.usbCont = tile;

    if (isUsb) {
        g_keyDerivationTileView.currentTile = TILE_USB_CONNECT;
        SetPageLockScreen(false);
    } else {
        g_keyDerivationTileView.currentTile = TILE_APPROVE;
    }
    g_keyDerivationTileView.tileView = tileView;

    lv_obj_set_tile_id(g_keyDerivationTileView.tileView, g_keyDerivationTileView.currentTile, 0, LV_ANIM_OFF);
}
void GuiKeyDerivationRequestDeInit()
{
    g_isUsb = false;
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    GUI_PAGE_DEL(g_keyDerivationTileView.pageWidget);
    GUI_DEL_OBJ(g_derivationPathCont);
    GuiAnimatingQRCodeDestroyTimer();
    FreeKeyDerivationRequestMemory();
    SetPageLockScreen(true);
}

static void SelectDerivationHandler(lv_event_t *e)
{
    lv_obj_t *newCheckBox = lv_event_get_user_data(e);
    uint8_t index = 0;
    if (newCheckBox == g_derivationTypeCheck[1]) {
        index = 1;
    }
    lv_obj_add_state(g_derivationTypeCheck[index], LV_STATE_CHECKED);
    lv_obj_clear_state(g_derivationTypeCheck[!index], LV_STATE_CHECKED);
    SetCurrentSelectedIndex(index);
    ShowEgAddressCont(g_egCont);
#ifdef WEB3_VERSION
    UpdateConfirmBtn(index != GetAccountType());
#endif
}

static void OpenDerivationPath()
{
#ifdef WEB3_VERSION
    if (IsCardano()) {
        SetCurrentSelectedIndex(GetAccountType());
    }
#endif

    lv_obj_t *bgCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()),
                                          lv_obj_get_height(lv_scr_act()) -
                                          GUI_MAIN_AREA_OFFSET);

    lv_obj_align(bgCont, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET);

    lv_obj_t *scrollCont = GuiCreateContainerWithParent(
                               bgCont, lv_obj_get_width(lv_scr_act()),
                               lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET - 114);
    lv_obj_align(scrollCont, LV_ALIGN_DEFAULT, 0, 0);
    lv_obj_add_flag(scrollCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scrollCont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label = GuiCreateNoticeLabel(scrollCont, _("derivation_path_select_ada"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *cont = GuiCreateContainerWithParent(scrollCont, 408, 205);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 84);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    for (int i = 0; i < 2; i++) {
        lv_obj_t *accountType =
            GuiCreateTextLabel(cont, GetChangeDerivationAccountType(i));
        lv_obj_t *checkBox = GuiCreateSingleCheckBox(cont, _(""));
        g_derivationTypeCheck[i] = checkBox;
        lv_obj_set_size(g_derivationTypeCheck[i], 45, 45);
        if (i == GetCurrentSelectedIndex()) {
            lv_obj_add_state(g_derivationTypeCheck[i], LV_STATE_CHECKED);
        }
        GuiButton_t table[] = {
            {
                .obj = accountType,
                .align = LV_ALIGN_DEFAULT,
                .position = {24, 32},
            },
            {
                .obj = g_derivationTypeCheck[i],
                .align = LV_ALIGN_RIGHT_MID,
                .position = {-24, 0},
            },
        };
        lv_obj_t *button = GuiCreateButton(cont, 408, 102, table, NUMBER_OF_ARRAYS(table),
                                           SelectDerivationHandler, g_derivationTypeCheck[i]);
        lv_obj_align(button, LV_ALIGN_TOP_MID, 0, i * 102);
        if (i != 0) {
            static lv_point_t points[2] = {{0, 0}, {360, 0}};
            lv_obj_t *line = (lv_obj_t *)GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, i * 102);
        }
    }

    lv_obj_t *egCont = GuiCreateContainerWithParent(scrollCont, 408, 186);
    lv_obj_align_to(egCont, cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
    lv_obj_set_style_bg_color(egCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(egCont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(egCont, 24, LV_PART_MAIN);
    g_egCont = egCont;
    ShowEgAddressCont(g_egCont);
    SetMidBtnLabel(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_MID_LABEL,
                   _("derivation_path_change"));
    SetNavBarLeftBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_RETURN,
                     CloseDerivationHandler, NULL);
    SetNavBarRightBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL,
                      NULL);
    GUI_DEL_OBJ(g_openMoreHintBox);

    lv_obj_t *tmCont = GuiCreateContainerWithParent(bgCont, 480, 114);
    lv_obj_align(tmCont, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_color(tmCont, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_t *btn = GuiCreateBtn(tmCont, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -36, 0);
#ifdef WEB3_VERSION
    lv_obj_add_event_cb(btn, SaveHardwareCallVersion1AdaDerivationAlgo, LV_EVENT_CLICKED, NULL);
#else
#endif

    g_derivationPathConfirmBtn = btn;
    UpdateConfirmBtn(false);

    g_derivationPathCont = bgCont;
}

void GuiKeyDerivationRequestRefresh()
{
    if (g_isUsb) {
        SetNavBarLeftBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_LEFT_BUTTON_BUTT, NULL, NULL);
        SetNavBarRightBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        SetNavBarMidBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
    } else {
        GuiAnimatingQRCodeControl(false);
    }
}

void GuiKeyDerivationRequestNextTile()
{
    g_keyDerivationTileView.currentTile++;
    switch (g_keyDerivationTileView.currentTile) {
    case TILE_QRCODE:
        SetNavBarLeftBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_RETURN, OnReturnHandler, NULL);
        SetNavBarRightBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_MORE_INFO, OpenMoreHandlerWithOutDerivationPath, NULL);
        break;
    case TILE_USB_CONNECT:
        g_keyDerivationTileView.currentTile--;
        break;
    default:
        break;
    }
    lv_obj_set_tile_id(g_keyDerivationTileView.tileView, g_keyDerivationTileView.currentTile, 0, LV_ANIM_OFF);
}
void GuiKeyDerivationRequestPrevTile()
{
    g_keyDerivationTileView.currentTile--;
    switch (g_keyDerivationTileView.currentTile) {
    case TILE_APPROVE:
        SetNavBarLeftBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
        SetNavBarRightBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_MORE_INFO, OpenMoreHandler, NULL);
        GuiAnimatingQRCodeDestroyTimer();
        break;
    default:
        break;
    }
    lv_obj_set_tile_id(g_keyDerivationTileView.tileView, g_keyDerivationTileView.currentTile, 0, LV_ANIM_OFF);
}

void UpdateAndParseHardwareCall(void)
{
    GuiModelURClear();
    if (strnlen_s(SecretCacheGetPassword(), PASSWORD_MAX_LEN) != 0 && g_isUsbPassWordCheck) {
        if (g_response != NULL) {
            free_Response_QRHardwareCallData(g_response);
            g_response = NULL;
        }
        ModelParseQRHardwareCall();
        HiddenKeyboardAndShowAnimateQR();
    }
}

static void ModelParseQRHardwareCall()
{
    Response_QRHardwareCallData *data = parse_qr_hardware_call(g_data);
    g_callData = data->data;
    g_response = data;
    for (size_t i = 0; i < g_callData->key_derivation->schemas->size; i++) {
        g_hasAda = g_hasAda || g_callData->key_derivation->schemas->data[i].is_ada;
    }
    if (strcmp("0", g_callData->version) == 0) {
        g_hasAda = true;
    }
    CheckHardwareCallRequestIsLegal();
}

typedef enum {
    SECP256K1,
    SLIP10_ED25519,
    BIP32_ED25519,
    UNSUPPORT_DERIVATION_TYPE,
} DerivationType_t;

static uint8_t GetDerivationTypeByCurveAndDeriveAlgo(char *curve, char *algo)
{
    if (strcmp("Secp256k1", curve) == 0) {
        return SECP256K1;
    }
    if (strcmp("ED25519", curve) == 0) {
        if (strcmp("BIP32-ED25519", algo) == 0) {
            return BIP32_ED25519;
        }
        if (strcmp("SLIP10", algo) == 0) {
            return SLIP10_ED25519;
        }
    }
    printf("unsupport derivation type\n");
    // other case
    return UNSUPPORT_DERIVATION_TYPE;
}


static HardwareCallResult_t g_hardwareCallParamsCheckResult = {
    .isLegal = false,
    .title = "Invaild Params",
    .message = "hardware call params check failed"
};

static void SetHardwareCallParamsCheckResult(HardwareCallResult_t result)
{
    g_hardwareCallParamsCheckResult = result;
}

static HardwareCallResult_t CheckHardWareCallV0AdaPathIsLegal(char *path)
{
    char *token;
    char *last_token = NULL;
    int result;
    token = strtok(path, "/");
    while (token != NULL) {
        last_token = token;
        token = strtok(NULL, "/");
    }
    if (last_token != NULL) {
        char *end = strchr(last_token, '\'');
        if (end != NULL) {
            *end = '\0';
        }
        result = atoi(last_token);
    } else {
        SetHardwareCallParamsCheckResult((HardwareCallResult_t) {
            false, _("invaild_path_index_title"), _("invaild_path_index_con")
        });
        return g_hardwareCallParamsCheckResult;
    }
    // hardware call version 0, the derivation path index must be less than 24
    if (result > 23) {
        SetHardwareCallParamsCheckResult((HardwareCallResult_t) {
            false, _("invaild_path_index_title"), _("invaild_path_index_con")
        });
        return g_hardwareCallParamsCheckResult;
    } else {
        SetHardwareCallParamsCheckResult((HardwareCallResult_t) {
            true, "Check Pass", "hardware call params check pass"
        });
        return g_hardwareCallParamsCheckResult;
    }
}

static HardwareCallResult_t CheckHardwareCallRequestIsLegal(void)
{
    if (g_callData->key_derivation->schemas->size > 24) {
        SetHardwareCallParamsCheckResult((HardwareCallResult_t) {
            false, _("invaild_schemas_size"), _("invaild_schemas_size_big")
        });
        return g_hardwareCallParamsCheckResult;
    }
    if (strcmp("0", g_callData->version) == 0) {
        for (size_t i = 0; i < g_callData->key_derivation->schemas->size; i++) {
            // does not contain the ada prefix
            if (strstr(g_callData->key_derivation->schemas->data[i].key_path, "1852'/1815'") == NULL) {
                SetHardwareCallParamsCheckResult((HardwareCallResult_t) {
                    false, _("invaild_ada_path"), _("invaild_ada_path_con")
                });
                return g_hardwareCallParamsCheckResult;
            }
            char path_copy[256];
            strncpy(path_copy, g_callData->key_derivation->schemas->data[i].key_path, sizeof(path_copy) - 1);
            path_copy[sizeof(path_copy) - 1] = '\0';
            HardwareCallResult_t result = CheckHardWareCallV0AdaPathIsLegal(path_copy);
            if (!result.isLegal) {
                SetHardwareCallParamsCheckResult(result);
                return g_hardwareCallParamsCheckResult;
            }
        }
    }
    if (strcmp("1", g_callData->version) == 0) {
        for (size_t i = 0; i < g_callData->key_derivation->schemas->size; i++) {
            uint8_t derivationType = GetDerivationTypeByCurveAndDeriveAlgo(g_callData->key_derivation->schemas->data[i].curve, g_callData->key_derivation->schemas->data[i].algo);
            if (derivationType == UNSUPPORT_DERIVATION_TYPE) {
                SetHardwareCallParamsCheckResult((HardwareCallResult_t) {
                    false, _("invaild_derive_type"), _("invaild_derive_type_con")
                });
                return g_hardwareCallParamsCheckResult;
            }
            // check path match the chainType
            Response_bool *response = check_hardware_call_path(g_response->data->key_derivation->schemas->data[i].key_path, g_response->data->key_derivation->schemas->data[i].chain_type);
            if (*response->data == false) {
                SetHardwareCallParamsCheckResult((HardwareCallResult_t) {
                    false, _("invaild_account_path"),  _("invaild_account_path_notice")
                });
                return g_hardwareCallParamsCheckResult;
            }
        }
    }
    if (g_hasAda) {
        MnemonicType mnemonicType = GetMnemonicType();
        if (mnemonicType == MNEMONIC_TYPE_SLIP39) {
            SetHardwareCallParamsCheckResult((HardwareCallResult_t) {
                false, _("invaild_derive_type"), _("invalid_slip39_ada_con")
            });
            return g_hardwareCallParamsCheckResult;
        }
    }

    SetHardwareCallParamsCheckResult((HardwareCallResult_t) {
        true, "Check Pass", "hardware call params check pass"
    });
    return g_hardwareCallParamsCheckResult;
}

static UREncodeResult *ModelGenerateSyncUR(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    CSliceFFI_ExtendedPublicKey keys;
    char firmwareVersion[BUFFER_SIZE_32];
    GetSoftWareVersionNumber(firmwareVersion);
    if (strcmp("1", g_callData->version) == 0) {
        uint8_t seed[64];
        char *password = SecretCacheGetPassword();
        MnemonicType mnemonicType = GetMnemonicType();
        bool isSlip39 = mnemonicType == MNEMONIC_TYPE_SLIP39;
        int seedLen = isSlip39 ? GetCurrentAccountEntropyLen() : sizeof(seed) ;

        GetAccountSeed(GetCurrentAccountIndex(), seed, password);
        ExtendedPublicKey xpubs[24];
        SimpleResponse_c_char *pubkey[24];
        for (size_t i = 0; i < g_callData->key_derivation->schemas->size; i++) {
            uint8_t derivationType = GetDerivationTypeByCurveAndDeriveAlgo(g_callData->key_derivation->schemas->data[i].curve, g_callData->key_derivation->schemas->data[i].algo);
            char *path = g_callData->key_derivation->schemas->data[i].key_path;
            switch (derivationType) {
            case SECP256K1:
                pubkey[i] = get_extended_pubkey_bytes_by_seed(seed, seedLen, path);
                break;
            case SLIP10_ED25519:
                pubkey[i] = get_ed25519_pubkey_by_seed(seed, seedLen, path);
                break;
            case BIP32_ED25519:
                if (selected_ada_derivation_algo == HD_STANDARD_ADA && !g_isUsb) {
                    uint8_t entropyLen = 0;
                    uint8_t entropy[64];
                    GetAccountEntropy(GetCurrentAccountIndex(), entropy, &entropyLen, password);
                    SimpleResponse_c_char* cip3_response = get_icarus_master_key(entropy, entropyLen, GetPassphrase(GetCurrentAccountIndex()));
                    char* icarusMasterKey = cip3_response->data;
                    pubkey[i] = derive_bip32_ed25519_extended_pubkey(icarusMasterKey, path);
                } else if (selected_ada_derivation_algo == HD_LEDGER_BITBOX_ADA || g_isUsb) {
                    // seed -> mnemonic --> master key(m) -> derive key
                    uint8_t entropyLen = 0;
                    uint8_t entropy[64];
                    GetAccountEntropy(GetCurrentAccountIndex(), entropy, &entropyLen, password);
                    char *mnemonic = NULL;
                    bip39_mnemonic_from_bytes(NULL, entropy, entropyLen, &mnemonic);
                    SimpleResponse_c_char *ledger_bitbox02_response  = get_ledger_bitbox02_master_key(mnemonic, GetPassphrase(GetCurrentAccountIndex()));
                    char* ledgerBitbox02Key = ledger_bitbox02_response->data;
                    pubkey[i] = derive_bip32_ed25519_extended_pubkey(ledgerBitbox02Key, path);
                }
                break;
            default:
                break;
            }
            xpubs[i].path = path;
            xpubs[i].xpub = pubkey[i]->data;
        }
        keys.data = xpubs;
        keys.size = g_callData->key_derivation->schemas->size;
        uint8_t mfp[4] = {0};
        GetMasterFingerPrint(mfp);
        // clean the cache after use
        if (!g_isUsb) {
            ClearSecretCache();
        }
        Ptr_UREncodeResult urResult = generate_key_derivation_ur(mfp, 4, &keys, firmwareVersion);
        for (size_t i = 0; i < g_callData->key_derivation->schemas->size; i++) {
            if (pubkey[i] != NULL) {
                free_simple_response_c_char(pubkey[i]);
            }
        }
        SetLockScreen(enable);
        return urResult;
    }
#ifdef WEB3_VERSION
    ExtendedPublicKey xpubs[24];
    for (size_t i = 0; i < g_callData->key_derivation->schemas->size; i++) {
        KeyDerivationSchema schema = g_callData->key_derivation->schemas->data[i];
        char* xpub = GetCurrentAccountPublicKey(GetXPubIndexByPath(schema.key_path));
        xpubs[i].path = schema.key_path;
        xpubs[i].xpub = xpub;
    }
    keys.data = xpubs;
    keys.size = g_callData->key_derivation->schemas->size;
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    // print keys
    for (size_t i = 0; i < keys.size; i++) {
        printf("v0 path: %s, xpub: %s\n", keys.data[i].path, keys.data[i].xpub);
    }
    SetLockScreen(enable);
    return generate_key_derivation_ur(mfp, 4, &keys, firmwareVersion);
#endif
    SetLockScreen(enable);
    return NULL;
}


static void GuiCreateHardwareCallApproveWidget(lv_obj_t *parent)
{
    lv_obj_t *label, *cont, *btn, *pathCont, *noticeCont;
    cont = GuiCreateContainerWithParent(parent, 408, 534);
    lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 36, 8);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);

    label = GuiCreateIllustrateLabel(cont, _("connect_wallet_key_request_fmt"));
    lv_label_set_text_fmt(label, _("connect_wallet_key_request_fmt"), g_response->data->origin);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

    // pathContainer height should be 102 * size
    pathCont = GuiCreateContainerWithParent(cont, 408, 102 * g_response->data->key_derivation->schemas->size);
    if (g_response->data->key_derivation->schemas->size > 3) {
        // for better view, set the height to 3 rows height
        lv_obj_set_height(pathCont, 102 * 3);
        // show the scroll bar
        lv_obj_set_style_bg_color(pathCont, WHITE_COLOR, LV_PART_SCROLLBAR);
        lv_obj_set_style_bg_opa(pathCont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
        lv_obj_set_style_bg_opa(pathCont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    }
    lv_obj_align(pathCont, LV_ALIGN_TOP_LEFT, 0, 92);
    lv_obj_set_style_radius(pathCont, 24, 0);
    lv_obj_set_style_bg_color(pathCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pathCont, LV_OPA_12, LV_PART_MAIN);
    lv_obj_add_flag(pathCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(pathCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *line;
    static lv_point_t points[24] = {{0, 0}, {360, 0}};

    for (size_t i = 0; i < g_response->data->key_derivation->schemas->size; i++) {
        cont = GuiCreateContainerWithParent(pathCont, 408, 102);
        lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 0, 102 * i);
        lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_MAIN);
        char title[BUFFER_SIZE_32] = {0};
        sprintf(title, "%s-%d", _("account_head"), i);
        label = GuiCreateIllustrateLabel(cont, title);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
        char path[BUFFER_SIZE_64] = {0};
        snprintf_s(path, BUFFER_SIZE_64, "M/%s", g_response->data->key_derivation->schemas->data[i].key_path);
        label = GuiCreateIllustrateLabel(cont, path);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 56);
        line = GuiCreateLine(cont, points, 2);
        lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 101);
    }

    // create notice container
    noticeCont = GuiCreateContainerWithParent(parent, 408, 202);
    if (g_response->data->key_derivation->schemas->size > 3) {
        // for better view, set the height to 3 rows height
        lv_obj_align(noticeCont, LV_ALIGN_TOP_LEFT, 36, 92 + 102 * 3 + 30);
    } else {
        lv_obj_align(noticeCont, LV_ALIGN_TOP_LEFT, 36, 92 + 102 * g_response->data->key_derivation->schemas->size + 30);
    }
    label = GuiCreateIllustrateLabel(noticeCont, _("hardware_call_approve_notice"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

    // bottom container
    cont = GuiCreateContainerWithParent(parent, 480, 114);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    btn = GuiCreateTextBtn(cont, _("Cancel"));
    lv_obj_set_size(btn, 192, 66);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_20, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 36, 24);
    lv_obj_add_event_cb(btn, CloseCurrentViewHandler, LV_EVENT_CLICKED, NULL);

    btn = GuiCreateTextBtn(cont, _("Approve"));
    lv_obj_set_size(btn, 192, 66);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 252, 24);
    lv_obj_add_event_cb(btn, OnApproveHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiCreateApproveWidget(lv_obj_t *parent)
{
    lv_obj_t *label, *cont, *btn, *pathCont;

    cont = GuiCreateContainerWithParent(parent, 408, 534);
    lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 36, 8);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);

    label = GuiCreateIllustrateLabel(cont, _("connect_wallet_key_request_fmt"));
    lv_label_set_text_fmt(label, _("connect_wallet_key_request_fmt"), g_response->data->origin);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

    pathCont = GuiCreateContainerWithParent(cont, 408, 450);
    lv_obj_align(pathCont, LV_ALIGN_TOP_LEFT, 0, 92);
    lv_obj_set_style_radius(pathCont, 24, 0);
    lv_obj_set_style_bg_color(pathCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pathCont, LV_OPA_12, LV_PART_MAIN);
    lv_obj_add_flag(pathCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(pathCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(pathCont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_t *line;
    static lv_point_t points[2] = {{0, 0}, {360, 0}};

    for (size_t i = 0; i < g_response->data->key_derivation->schemas->size; i++) {
        cont = GuiCreateContainerWithParent(pathCont, 408, 102);
        lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 0, 102 * i);
        lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_MAIN);
        char title[BUFFER_SIZE_32] = {0};
        sprintf(title, "%s-%d", _("account_head"), i);
        label = GuiCreateIllustrateLabel(cont, title);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
        char path[BUFFER_SIZE_64] = {0};
        snprintf_s(path, BUFFER_SIZE_64, "M/%s", g_response->data->key_derivation->schemas->data[i].key_path);
        label = GuiCreateIllustrateLabel(cont, path);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 56);
        if (i > 0) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 101);
        }
    }

    cont = GuiCreateContainerWithParent(parent, 480, 114);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    btn = GuiCreateTextBtn(cont, _("Cancel"));
    lv_obj_set_size(btn, 192, 66);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_20, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 36, 24);
    lv_obj_add_event_cb(btn, CloseCurrentViewHandler, LV_EVENT_CLICKED, NULL);

    btn = GuiCreateTextBtn(cont, _("Approve"));
    lv_obj_set_size(btn, 192, 66);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 252, 24);
    lv_obj_add_event_cb(btn, OnApproveHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiCreateCommonHardWareCallQRCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, _("connect_wallet_scan"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 152 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);

    lv_obj_t *qrCont = GuiCreateContainerWithParent(parent, 408, 408);
    lv_obj_add_flag(qrCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(qrCont, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_set_style_bg_color(qrCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(qrCont, 24, LV_PART_MAIN);

    lv_obj_t *qrBgCont = GuiCreateContainerWithParent(qrCont, 336, 336);
    lv_obj_align(qrBgCont, LV_ALIGN_TOP_MID, 0, 36);
    lv_obj_set_style_bg_color(qrBgCont, WHITE_COLOR, LV_PART_MAIN);

    lv_obj_t *qrcode = GuiCreateContainerWithParent(qrBgCont, 294, 294);
    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 21);

    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 21);
    g_keyDerivationTileView.qrCode = qrcode;

    lv_obj_t *bottomCont = GuiCreateContainerWithParent(qrCont, 408, 104);
    lv_obj_align(bottomCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bottomCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bottomCont, LV_OPA_0, LV_STATE_DEFAULT | LV_PART_MAIN);
}

static void GuiCreateQRCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, _("connect_wallet_scan"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 152 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);

    lv_obj_t *qrCont = GuiCreateContainerWithParent(parent, 408, 482);
    lv_obj_add_flag(qrCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(qrCont, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_set_style_bg_color(qrCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(qrCont, 24, LV_PART_MAIN);

    lv_obj_t *qrBgCont = GuiCreateContainerWithParent(qrCont, 336, 336);
    lv_obj_align(qrBgCont, LV_ALIGN_TOP_MID, 0, 36);
    lv_obj_set_style_bg_color(qrBgCont, WHITE_COLOR, LV_PART_MAIN);

    lv_obj_t *qrcode = GuiCreateContainerWithParent(qrBgCont, 294, 294);
    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 21);

    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 21);
    g_keyDerivationTileView.qrCode = qrcode;

    lv_obj_t *bottomCont = GuiCreateContainerWithParent(qrCont, 408, 104);
    lv_obj_align(bottomCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bottomCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bottomCont, LV_OPA_0, LV_STATE_DEFAULT | LV_PART_MAIN);

    label = GuiCreateNoticeLabel(bottomCont, _("Network"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 12);

    lv_obj_t *coinCont = GuiCreateContainerWithParent(bottomCont, 280, 30);
    lv_obj_align(coinCont, LV_ALIGN_TOP_LEFT, 36, 50);
    lv_obj_set_style_bg_color(coinCont, DARK_BG_COLOR, LV_PART_MAIN);

    lv_obj_t *img = GuiCreateImg(coinCont, &coinAda);
    lv_img_set_zoom(img, 110);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 0, 0);
}

static void GuiShowKeyBoardDialog(lv_obj_t *parent)
{
    g_keyboardWidget = GuiCreateKeyboardWidget(parent);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    // set sig for global cache logic
    static uint16_t sig = SIG_HARDWARE_CALL_DERIVE_PUBKEY;
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}

static void OnApproveHandler(lv_event_t *e)
{
    // click approve button and check the hardware call params
    HardwareCallResult_t res =  g_hardwareCallParamsCheckResult;
    if (!res.isLegal) {
        GuiCreateHardwareCallInvaildParamHintbox(res.title, res.message);
        return;
    }
    if (strcmp("1", g_callData->version) == 0) {
        // pop up password keyboard dialog when hardware call version is 1
        lv_obj_t *parent = lv_obj_get_parent(lv_event_get_target(e));
        GuiShowKeyBoardDialog(parent);
    } else {
        GuiAnimatingQRCodeInit(g_keyDerivationTileView.qrCode, ModelGenerateSyncUR, true);
        GuiKeyDerivationRequestNextTile();
    }
}

void GuiKeyDerivePasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    if (passwordVerifyResult->errorCount == MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX) {
        // if (GetCurrentTransactionMode() == TRANSACTION_MODE_USB) {
        //     const char *data = "Please try again after unlocking";
        //     HandleURResultViaUSBFunc(data, strlen(data), GetCurrentUSParsingRequestID(), PRS_PARSING_VERIFY_PASSWORD_ERROR);
        // }
    }
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}

void GuiKeyDeriveUsbPullout(void)
{
    if (g_isUsb) {
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        g_keyboardWidget = NULL;
        ClearUSBRequestId();
        static uint16_t signal = SIG_LOCK_VIEW_VERIFY_PIN;
        GuiCloseToTargetView(&g_homeView);
        GuiLockScreenUpdatePurpose(LOCK_SCREEN_PURPOSE_UNLOCK);
        GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_VERIFY, &signal, sizeof(signal));
        if (GuiNeedFpRecognize()) {
            FpRecognize(RECOGNIZE_UNLOCK);
        }
    }
}

void HiddenKeyboardAndShowAnimateQR()
{
    // close password keyboard
    if (g_isUsb) {
        if (g_keyboardWidget != NULL) {
            GuiConnectUsbPasswordPass();
        }
        g_isUsbPassWordCheck = true;
        UREncodeResult *urResult = ModelGenerateSyncUR();
        HandleURResultViaUSBFunc(urResult->data, strlen(urResult->data), GetCurrentUSParsingRequestID(), PRS_EXPORT_HARDWARE_CALL_SUCCESS);
        free_ur_encode_result(urResult);
    } else {
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        // show dynamic qr code
        GuiAnimatingQRCodeInit(g_keyDerivationTileView.qrCode, ModelGenerateSyncUR, true);
        // jump to next page to show qr code
        GuiKeyDerivationRequestNextTile();
    }
}

static void OnReturnHandler(lv_event_t *e)
{
    GuiKeyDerivationRequestPrevTile();
}

void GuiKeyDerivationWidgetHandleURGenerate(char *data, uint16_t len)
{
    GuiAnimantingQRCodeFirstUpdate(data, len);
}

void GuiKeyDerivationWidgetHandleURUpdate(char *data, uint16_t len)
{
    GuiAnimatingQRCodeUpdate(data, len);
}

static void QRCodePause(bool pause)
{
    GuiAnimatingQRCodeControl(pause);
}

static const char *GetChangeDerivationAccountType(int i)
{
    if (i == 0) {
        return _("receive_ada_more_t_standard");
    } else if (i == 1) {
        return _("receive_ada_more_t_ledger");
    }

    return NULL;
}

static void SetCurrentSelectedIndex(uint8_t index)
{
    g_currentSelectedPathIndex = index;
}

static uint32_t GetCurrentSelectedIndex()
{
    return g_currentSelectedPathIndex;
}


static void UpdateConfirmBtn(bool update)
{
    if (update) {
        lv_obj_set_style_bg_opa(g_derivationPathConfirmBtn, LV_OPA_COVER,
                                LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_derivationPathConfirmBtn, 0),
                                  LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_add_flag(g_derivationPathConfirmBtn, LV_OBJ_FLAG_CLICKABLE);
    } else {
        lv_obj_set_style_bg_opa(g_derivationPathConfirmBtn, LV_OPA_30,
                                LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_derivationPathConfirmBtn, 0),
                                  LV_OPA_30, LV_PART_MAIN);
        lv_obj_clear_flag(g_derivationPathConfirmBtn, LV_OBJ_FLAG_CLICKABLE);
    }
}

static void CloseDerivationHandler(lv_event_t *e)
{
    QRCodePause(false);
    GUI_DEL_OBJ(g_derivationPathCont);
    SetNavBarLeftBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetWallet(g_keyDerivationTileView.pageWidget->navBarWidget, g_walletIndex,
              NULL);
    SetNavBarRightBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_MORE_INFO,
                      OpenMoreHandler, &g_walletIndex);
}

// hardware call version 1 need another CompareDerivationHandler


static void GetCardanoEgAddress(void)
{
#ifdef WEB3_VERSION
    char *xPub = NULL;
    xPub = GetCurrentAccountPublicKey(XPUB_TYPE_ADA_0);
    SimpleResponse_c_char *result = cardano_get_base_address(xPub, 0, 1);
    CutAndFormatString(g_derivationPathAddr[STANDARD_ADA][0], BUFFER_SIZE_128,
                       result->data, 24);
    free_simple_response_c_char(result);

    xPub = GetCurrentAccountPublicKey(XPUB_TYPE_ADA_1);
    result = cardano_get_base_address(xPub, 0, 1);
    CutAndFormatString(g_derivationPathAddr[STANDARD_ADA][1], BUFFER_SIZE_128,
                       result->data, 24);
    free_simple_response_c_char(result);

    xPub = GetCurrentAccountPublicKey(XPUB_TYPE_LEDGER_ADA_0);
    result = cardano_get_base_address(xPub, 0, 1);
    CutAndFormatString(g_derivationPathAddr[LEDGER_ADA][0], BUFFER_SIZE_128,
                       result->data, 24);
    free_simple_response_c_char(result);

    xPub = GetCurrentAccountPublicKey(XPUB_TYPE_LEDGER_ADA_1);
    result = cardano_get_base_address(xPub, 0, 1);
    CutAndFormatString(g_derivationPathAddr[LEDGER_ADA][1], BUFFER_SIZE_128,
                       result->data, 24);
    free_simple_response_c_char(result);
#endif
}

static void UpdateCardanoEgAddress(uint8_t index)
{
    lv_label_set_text(g_egAddress[0],
                      (const char *)g_derivationPathAddr[index][0]);
    lv_label_set_text(g_egAddress[1],
                      (const char *)g_derivationPathAddr[index][1]);
}

static void ShowEgAddressCont(lv_obj_t *egCont)
{
    if (egCont == NULL) {
        printf("egCont is NULL, cannot show eg address\n");
        return;
    }
    lv_obj_clean(egCont);

    lv_obj_t *prevLabel = NULL, *label;
    int egContHeight = 12;
    char *desc = GetChangeDerivationPathDesc();
    if (desc != NULL && strnlen_s(desc, BUFFER_SIZE_128) > 0) {
        label = GuiCreateNoticeLabel(egCont, desc);
        lv_obj_set_width(label, 360);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 12);
        lv_obj_update_layout(label);
        egContHeight += lv_obj_get_height(label);
        prevLabel = label;
    }
    label = GuiCreateNoticeLabel(egCont, _("derivation_path_address_eg"));
    if (prevLabel != NULL) {
        lv_obj_align_to(label, prevLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    } else {
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 12);
    }
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_update_layout(label);
    egContHeight = egContHeight + 4 + lv_obj_get_height(label);
    prevLabel = label;
    lv_obj_t *index = GuiCreateNoticeLabel(egCont, _("0"));
    lv_obj_align_to(index, prevLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_label_set_long_mode(index, LV_LABEL_LONG_WRAP);
    lv_obj_update_layout(index);
    egContHeight = egContHeight + 4 + lv_obj_get_height(index);
    g_egAddressIndex[0] = index;
    prevLabel = index;
    label = GuiCreateIllustrateLabel(egCont, "");
    lv_obj_align_to(label, prevLabel, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    g_egAddress[0] = label;
    index = GuiCreateNoticeLabel(egCont, _("1"));
    lv_obj_align_to(index, prevLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_label_set_long_mode(index, LV_LABEL_LONG_WRAP);
    lv_obj_update_layout(index);
    egContHeight = egContHeight + 4 + lv_obj_get_height(index);
    g_egAddressIndex[1] = index;
    prevLabel = index;
    label = GuiCreateIllustrateLabel(egCont, "");
    lv_obj_align_to(label, prevLabel, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    g_egAddress[1] = label;
    egContHeight += 12;
    lv_obj_set_height(egCont, egContHeight);
    GetCardanoEgAddress();
    UpdateCardanoEgAddress(GetCurrentSelectedIndex());
}

static void ChangeDerivationPathHandler(lv_event_t *e)
{
    if (g_derivationPathCont != NULL) {
        GUI_DEL_OBJ(g_derivationPathCont);
    }
    OpenDerivationPath();
    QRCodePause(true);
}

static void OpenMoreHandler(lv_event_t *e)
{
    int height = 84;
    int hintboxHeight = 144;
    bool hasChangePath = g_hasAda && g_hardwareCallParamsCheckResult.isLegal;

    if (hasChangePath) {
        hintboxHeight += height;
    }
    g_openMoreHintBox = GuiCreateHintBox(hintboxHeight);
    lv_obj_add_event_cb(lv_obj_get_child(g_openMoreHintBox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_openMoreHintBox);
    lv_obj_t *label = GuiCreateTextLabel(g_openMoreHintBox, _("Tutorial"));
    lv_obj_t *img = GuiCreateImg(g_openMoreHintBox, &imgTutorial);

    GuiButton_t table[] = {
        {
            .obj = img,
            .align = LV_ALIGN_LEFT_MID,
            .position = {24, 0},
        },
        {
            .obj = label,
            .align = LV_ALIGN_LEFT_MID,
            .position = {76, 0},
        }
    };
    lv_obj_t *btn = GuiCreateButton(g_openMoreHintBox, 456, 84, table, NUMBER_OF_ARRAYS(table),
                                    OpenTutorialHandler, &g_walletIndex);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    if (hasChangePath) {
        WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);
        lv_obj_t *derivationBtn = GuiCreateSelectButton(g_openMoreHintBox, _("derivation_path_change"),
                                  &imgPath, ChangeDerivationPathHandler, wallet,
                                  true);
        lv_obj_align(derivationBtn, LV_ALIGN_BOTTOM_MID, 0, -120);
    }
}

static void OpenMoreHandlerWithOutDerivationPath(lv_event_t *e)
{
    int height = 24;
    int hintboxHeight = 88;
    bool hasChangePath = g_hasAda && g_hardwareCallParamsCheckResult.isLegal;

    if (hasChangePath) {
        hintboxHeight += height;
    }
    g_openMoreHintBox = GuiCreateHintBox(hintboxHeight);
    lv_obj_add_event_cb(lv_obj_get_child(g_openMoreHintBox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_openMoreHintBox);
    lv_obj_t *label = GuiCreateTextLabel(g_openMoreHintBox, _("Tutorial"));
    lv_obj_t *img = GuiCreateImg(g_openMoreHintBox, &imgTutorial);

    GuiButton_t table[] = {
        {
            .obj = img,
            .align = LV_ALIGN_LEFT_MID,
            .position = {24, 0},
        },
        {
            .obj = label,
            .align = LV_ALIGN_LEFT_MID,
            .position = {76, 0},
        }
    };
    lv_obj_t *btn = GuiCreateButton(g_openMoreHintBox, 456, 84, table, NUMBER_OF_ARRAYS(table),
                                    OpenTutorialHandler, &g_walletIndex);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
}

static void OpenTutorialHandler(lv_event_t *e)
{
    WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);
    GuiFrameOpenViewWithParam(&g_walletTutorialView, wallet, sizeof(WALLET_LIST_INDEX_ENUM));
    GUI_DEL_OBJ(g_openMoreHintBox);
}

static void GuiConnectUsbPasswordPass(void)
{
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    g_keyboardWidget = NULL;
    lv_obj_clean(g_keyDerivationTileView.usbCont);
    lv_obj_t *parent = g_keyDerivationTileView.usbCont;
    GuiConnectUsbCreateImg(parent);

    lv_obj_t *titlelabel = GuiCreateLittleTitleLabel(parent, _("usb_connectioning_wallet_title"));
    lv_obj_align(titlelabel, LV_ALIGN_TOP_MID, 0, 184);
    lv_obj_t *contentLabel = GuiCreateNoticeLabel(parent, _("usb_connectioning_wallet_desc"));
    lv_obj_set_style_text_align(contentLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(contentLabel, LV_ALIGN_TOP_MID, 0, 234);

    lv_obj_t *button = GuiCreateTextBtn(parent, _("Close"));
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_size(button, 408, 66);
    lv_obj_add_event_cb(button, CloseCurrentViewHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiConnectUsbEntranceWidget(lv_obj_t *parent)
{
    GuiConnectUsbCreateImg(parent);
    lv_obj_t *titlelabel = GuiCreateLittleTitleLabel(parent, _("usb_transport_connection_request"));
    lv_obj_align(titlelabel, LV_ALIGN_TOP_MID, 0, 184);
    lv_obj_t *contentLabel = GuiCreateNoticeLabel(parent, _("usb_transport_connect_desc"));
    lv_obj_set_style_text_align(contentLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(contentLabel, LV_ALIGN_TOP_MID, 0, 234);

    lv_obj_t *button = GuiCreateTextBtn(parent, _("Reject"));
    lv_obj_align(button, LV_ALIGN_BOTTOM_LEFT, 36, -24);
    lv_obj_set_size(button, 192, 66);
    lv_obj_add_event_cb(button, RejectButtonHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(button, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button, LV_OPA_20, LV_PART_MAIN);

    button = GuiCreateTextBtn(parent, _("Approve"));
    lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_set_size(button, 192, 66);
    lv_obj_add_event_cb(button, ApproveButtonHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiConnectUsbCreateImg(lv_obj_t *parent)
{
    lv_obj_t *img = GuiCreateImg(parent, &imgSoftwareWallet);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 110, 40);

    for (int i = 0; i < 3; i++) {
        lv_obj_t *led = lv_led_create(parent);
        lv_obj_align(led, LV_ALIGN_DEFAULT, 236 + i * 12, 70);
        lv_led_set_brightness(led, 150);
        lv_obj_set_style_shadow_width(led, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
        lv_obj_set_style_radius(led, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
        lv_led_set_color(led, ORANGE_COLOR);
        lv_led_on(led);
        lv_obj_set_size(led, 4, 4);
    }

    img = GuiCreateImg(parent, &imgLogoGraph);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 298, 40);
}

static void ApproveButtonHandler(lv_event_t *e)
{
    static uint16_t sig = SIG_INIT_CONNECT_USB;
    g_keyboardWidget = GuiCreateKeyboardWidget(g_keyDerivationTileView.usbCont);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}

static void RejectButtonHandler(lv_event_t *e)
{
    const char *data = "UR parsing rejected";
    HandleURResultViaUSBFunc(data, strlen(data), GetCurrentUSParsingRequestID(), PRS_PARSING_REJECTED);
    GuiCloseCurrentWorkingView();
}

#ifdef WEB3_VERSION
static void SetAccountType(uint8_t index)
{
    SetConnectWalletPathIndex(g_response->data->origin, index);
}

static AdaXPubType GetAccountType(void)
{
    return GetConnectWalletPathIndex(g_response->data->origin);
}

static void SaveHardwareCallVersion1AdaDerivationAlgo(lv_event_t *e)
{
    selected_ada_derivation_algo = GetCurrentSelectedIndex();
    // save the derivation path type to the json file that be saved in flash
    SetAccountType(selected_ada_derivation_algo);
    CloseDerivationHandler(e);
}

static uint8_t GetXPubIndexByPath(char *path)
{
    if (strcmp("1852'/1815'/0'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 0);
    if (strcmp("1852'/1815'/1'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 1);
    if (strcmp("1852'/1815'/2'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 2);
    if (strcmp("1852'/1815'/3'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 3);
    if (strcmp("1852'/1815'/4'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 4);
    if (strcmp("1852'/1815'/5'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 5);
    if (strcmp("1852'/1815'/6'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 6);
    if (strcmp("1852'/1815'/7'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 7);
    if (strcmp("1852'/1815'/8'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 8);
    if (strcmp("1852'/1815'/9'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 9);
    if (strcmp("1852'/1815'/10'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 10);
    if (strcmp("1852'/1815'/11'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 11);
    if (strcmp("1852'/1815'/12'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 12);
    if (strcmp("1852'/1815'/13'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 13);
    if (strcmp("1852'/1815'/14'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 14);
    if (strcmp("1852'/1815'/15'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 15);
    if (strcmp("1852'/1815'/16'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 16);
    if (strcmp("1852'/1815'/17'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 17);
    if (strcmp("1852'/1815'/18'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 18);
    if (strcmp("1852'/1815'/19'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 19);
    if (strcmp("1852'/1815'/20'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 20);
    if (strcmp("1852'/1815'/21'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 21);
    if (strcmp("1852'/1815'/22'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 22);
    if (strcmp("1852'/1815'/23'", path) == 0) return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 23);
    if (strcmp("M/44'/148'/0'", path) == 0) return XPUB_TYPE_STELLAR_0;
    if (strcmp("M/44'/148'/1'", path) == 0) return XPUB_TYPE_STELLAR_1;
    if (strcmp("M/44'/148'/2'", path) == 0) return XPUB_TYPE_STELLAR_2;
    if (strcmp("M/44'/148'/3'", path) == 0) return XPUB_TYPE_STELLAR_3;
    if (strcmp("M/44'/148'/4'", path) == 0) return XPUB_TYPE_STELLAR_4;
    return GetAdaXPubTypeByIndexAndDerivationType(GetConnectWalletPathIndex(g_response->data->origin), 0);
}

static bool IsCardano()
{
    return g_walletIndex == WALLET_LIST_ETERNL || g_walletIndex == WALLET_LIST_TYPHON || g_walletIndex == WALLET_LIST_BEGIN || g_walletIndex == WALLET_LIST_MEDUSA;
}
#endif
