#ifndef BTC_ONLY
#include "gui_key_derivation_request_widgets.h"
#include "gui.h"
#include "gui_page.h"
#include "librust_c.h"
#include "keystore.h"
#include "user_memory.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_global_resources.h"
#include "gui_obj.h"

#include "secret_cache.h"
typedef struct KeyDerivationWidget {
    uint8_t currentTile;
    PageWidget_t *pageWidget;
    lv_obj_t *tileView;
    lv_obj_t *cont;
    lv_obj_t *qrCode;
} KeyDerivationWidget_t;

typedef enum {
    TILE_APPROVE,
    TILE_QRCODE,

    TILE_BUTT,
} PAGE_TILE;

static void *g_data;
static URParseResult *g_urResult;
static URParseMultiResult *g_urMultiResult;
static bool g_isMulti;
static KeyDerivationWidget_t g_keyDerivationTileView;
static QRHardwareCallData *g_callData;
static Response_QRHardwareCallData *g_response;
static WALLET_LIST_INDEX_ENUM g_walletIndex;
static lv_obj_t *g_openMoreHintBox;
static lv_obj_t *g_derivationPathCont = NULL;
static lv_obj_t *g_derivationPathConfirmBtn = NULL;
static lv_obj_t *g_derivationTypeCheck[2] = {0, 0};
static AdaXPubType g_currentCardanoPathIndex[3] = {STANDARD_ADA, STANDARD_ADA, STANDARD_ADA};
static uint32_t g_currentSelectedPathIndex[3] = {0};
static lv_obj_t *g_egCont = NULL;
static lv_obj_t *g_egAddressIndex[2];
static lv_obj_t *g_egAddress[2];
static char g_derivationPathAddr[2][2][BUFFER_SIZE_128];

static void RecalcCurrentWalletIndex(char *origin);

static void GuiCreateApproveWidget(lv_obj_t *parent);
static void GuiCreateHardwareCallApproveWidget(lv_obj_t *parent);
static void GuiCreateQRCodeWidget(lv_obj_t *parent);
static void GuiCreateCommonHardWareCallQRCodeWidget(lv_obj_t *parent);
static void OnApproveHandler(lv_event_t *e);
static void OnReturnHandler(lv_event_t *e);
static void ModelParseQRHardwareCall();
static UREncodeResult *ModelGenerateSyncUR(void);
static uint8_t GetXPubIndexByPath(char *path);
static void OpenTutorialHandler(lv_event_t *e);
static void OpenMoreHandler(lv_event_t *e);

static void QRCodePause(bool pause);
static const char *GetChangeDerivationAccountType(int i);
static void SetCurrentSelectedIndex(uint8_t index);
static uint32_t GetCurrentSelectedIndex();
static void SelectDerivationHandler(lv_event_t *e);
static void CloseDerivationHandler(lv_event_t *e);
static void ConfirmDerivationHandler(lv_event_t *e);
static char *GetChangeDerivationPathDesc(void);
static void GetCardanoEgAddress(void);
static void UpdateCardanoEgAddress(uint8_t index);
static void ShowEgAddressCont(lv_obj_t *egCont);
static void OpenDerivationPath();
static void ChangeDerivationPathHandler(lv_event_t *e);
static void UpdateConfirmBtn(void);
static bool IsSelectChanged(void);
static void SetAccountType(uint8_t index);
static bool IsCardano();

static KeyboardWidget_t *g_keyboardWidget = NULL;
static void GuiShowKeyBoardDialog(lv_obj_t *parent);
void GuiSetKeyDerivationRequestData(void *urResult, void *multiResult, bool is_multi)
{
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


static void RecalcCurrentWalletIndex(char *origin)
{
    if (strcmp("eternl", origin) == 0) {
        g_walletIndex = WALLET_LIST_ETERNL;
    } else if (strcmp("Typhon Extension", origin) == 0) {
        g_walletIndex = WALLET_LIST_TYPHON;
    } else if (strcmp("Leap Wallet", origin) == 0) {
        g_walletIndex = WALLET_LIST_LEAP;
    } else if (strcmp("Begin", origin) == 0) {
        g_walletIndex = WALLET_LIST_BEGIN;
    } else {
        g_walletIndex = WALLET_LIST_ETERNL;
    }
}

void GuiKeyDerivationRequestInit()
{
    GUI_PAGE_DEL(g_keyDerivationTileView.pageWidget);
    g_keyDerivationTileView.pageWidget = CreatePageWidget();
    g_keyDerivationTileView.cont = g_keyDerivationTileView.pageWidget->contentZone;
    SetNavBarLeftBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    lv_obj_t *tileView = lv_tileview_create(g_keyDerivationTileView.cont);
    lv_obj_clear_flag(tileView, LV_OBJ_FLAG_SCROLLABLE);
    if (GuiDarkMode()) {
        lv_obj_set_style_bg_color(tileView, BLACK_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(tileView, WHITE_COLOR, LV_PART_MAIN);
    }
    lv_obj_set_style_bg_opa(tileView, LV_OPA_0, LV_PART_SCROLLBAR & LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
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
    g_keyDerivationTileView.currentTile = TILE_APPROVE;
    g_keyDerivationTileView.tileView = tileView;

    lv_obj_set_tile_id(g_keyDerivationTileView.tileView, g_keyDerivationTileView.currentTile, 0, LV_ANIM_OFF);
}
void GuiKeyDerivationRequestDeInit()
{
    GUI_PAGE_DEL(g_keyDerivationTileView.pageWidget);
    GUI_DEL_OBJ(g_derivationPathCont);
    GuiAnimatingQRCodeDestroyTimer();
    FreeKeyDerivationRequestMemory();
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
    UpdateConfirmBtn();
}

static void OpenDerivationPath()
{
    if (IsCardano()) {
        SetAccountType(GetKeyDerivationAdaXPubType());
        SetCurrentSelectedIndex(GetKeyDerivationAdaXPubType());
    }

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

    lv_obj_t *label =
        GuiCreateNoticeLabel(scrollCont, _("derivation_path_select_ada"));
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
        lv_obj_t *button =
            GuiCreateButton(cont, 408, 102, table, NUMBER_OF_ARRAYS(table),
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
    lv_obj_add_event_cb(btn, ConfirmDerivationHandler, LV_EVENT_CLICKED, NULL);
    g_derivationPathConfirmBtn = btn;
    UpdateConfirmBtn();

    g_derivationPathCont = bgCont;
}

void GuiKeyDerivationRequestRefresh()
{
    GuiAnimatingQRCodeControl(false);
}
void GuiKeyDerivationRequestNextTile()
{
    g_keyDerivationTileView.currentTile++;
    switch (g_keyDerivationTileView.currentTile) {
    case TILE_QRCODE:
        SetNavBarLeftBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_RETURN, OnReturnHandler, NULL);
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
        GuiAnimatingQRCodeDestroyTimer();
        break;
    default:
        break;
    }
    lv_obj_set_tile_id(g_keyDerivationTileView.tileView, g_keyDerivationTileView.currentTile, 0, LV_ANIM_OFF);
}

static void ModelParseQRHardwareCall()
{
    Response_QRHardwareCallData *data = parse_qr_hardware_call(g_data);
    g_callData = data->data;
    g_response = data;
}
#else
static void ModelParseQRHardwareCall()
{
    g_response = SRAM_MALLOC(sizeof(Response_QRHardwareCallData));
    g_response->error_code = 0;
    g_response->error_message = "";
    g_response->data = SRAM_MALLOC(sizeof(QRHardwareCallData));
    g_response->data->call_type = "key_derivation";
    g_response->data->origin = "Leap Wallet";
    g_response->data->key_derivation = SRAM_MALLOC(sizeof(KeyDerivationRequestData));
    g_response->data->key_derivation->schemas = SRAM_MALLOC(sizeof(VecFFI_KeyDerivationSchema));
    g_response->data->key_derivation->schemas->cap = 4;
    g_response->data->key_derivation->schemas->size = 4;
    g_response->data->key_derivation->schemas->data = SRAM_MALLOC(4 * sizeof(KeyDerivationSchema));
    // sol
    g_response->data->key_derivation->schemas->data[0].algo = "ED25519";
    // ed25519
    g_response->data->key_derivation->schemas->data[0].curve = "1";
    g_response->data->key_derivation->schemas->data[0].key_path = "44'/501'/0'";
    g_response->data->key_derivation->schemas->data[0].chain_type = "SOL";
    // cosmos
    g_response->data->key_derivation->schemas->data[1].algo = "BIP32";
    // secp256k1
    g_response->data->key_derivation->schemas->data[1].curve = "0";
    g_response->data->key_derivation->schemas->data[1].key_path = "44'/118'/0'";
    g_response->data->key_derivation->schemas->data[1].chain_type = "ATOM";

    // cosmos
    g_response->data->key_derivation->schemas->data[2].algo = "BIP32";
    // ed25519
    g_response->data->key_derivation->schemas->data[2].curve = "1";
    g_response->data->key_derivation->schemas->data[2].key_path = "44'/501'/0'/0/0";
    g_response->data->key_derivation->schemas->data[2].chain_type = "SOL";

    // cosmos
    g_response->data->key_derivation->schemas->data[3].algo = "BIP32";
    // secp256k1
    g_response->data->key_derivation->schemas->data[3].curve = "0";
    g_response->data->key_derivation->schemas->data[3].key_path = "44'/118'/0'/0/0";
    g_response->data->key_derivation->schemas->data[3].chain_type = "ATOM";

    g_response->data->version = "1";


    g_callData = g_response->data;
}
#endif

typedef enum {
    SECP256K1,
    ED25519,
} PublicInfoType_t;


static uint8_t GetPublicKeyTypeByCurveString(char *curve)
{
    if (strcmp(curve, "SECP256K1") == 0) return SECP256K1;
    if (strcmp(curve, "ED25519") == 0) return ED25519;
}

static UREncodeResult *ModelGenerateSyncUR(void)
{
    CSliceFFI_ExtendedPublicKey keys;
    if (strcmp("1", g_callData->version) == 0) {
        // hardware call version 1
        ExtendedPublicKey xpubs[24];
        for (size_t i = 0; i < g_callData->key_derivation->schemas->size; i++) {
            uint8_t seed[64];
            char *password = SecretCacheGetPassword();
            int32_t ret = GetAccountSeed(GetCurrentAccountIndex(), seed, password);
            if (ret != 0) {
                printf("Failed to get account seed\n");
                return NULL;
            }
            uint8_t algo = GetPublicKeyTypeByCurveString(g_callData->key_derivation->schemas->data[i].curve);
            char *path = g_callData->key_derivation->schemas->data[i].key_path;
            SimpleResponse_c_char *pubkey;
            // todo need add bip32-ed25519
            if (algo == SECP256K1) {
                pubkey = get_extended_pubkey_bytes_by_seed(seed, sizeof(seed), path);
                xpubs[i].path = path;
                xpubs[i].xpub = pubkey->data;
            } else if (algo == ED25519) {
                pubkey = get_ed25519_pubkey_by_seed(seed, sizeof(seed), path);
                xpubs[i].path = path;
                xpubs[i].xpub = pubkey->data;
            }
        }
        keys.data = xpubs;
        keys.size = g_callData->key_derivation->schemas->size;
        uint8_t mfp[4] = {0};
        GetMasterFingerPrint(mfp);
        // clean the cache after use
        ClearSecretCache();
        return generate_key_derivation_ur(mfp, 4, &keys);
    }
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
    return generate_key_derivation_ur(mfp, 4, &keys);
}

static uint8_t GetXPubIndexByPath(char *path)
{
    if (strcmp("1852'/1815'/0'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(0);
    if (strcmp("1852'/1815'/1'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(1);
    if (strcmp("1852'/1815'/2'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(2);
    if (strcmp("1852'/1815'/3'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(3);
    if (strcmp("1852'/1815'/4'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(4);
    if (strcmp("1852'/1815'/5'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(5);
    if (strcmp("1852'/1815'/6'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(6);
    if (strcmp("1852'/1815'/7'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(7);
    if (strcmp("1852'/1815'/8'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(8);
    if (strcmp("1852'/1815'/9'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(9);
    if (strcmp("1852'/1815'/10'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(10);
    if (strcmp("1852'/1815'/11'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(11);
    if (strcmp("1852'/1815'/12'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(12);
    if (strcmp("1852'/1815'/13'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(13);
    if (strcmp("1852'/1815'/14'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(14);
    if (strcmp("1852'/1815'/15'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(15);
    if (strcmp("1852'/1815'/16'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(16);
    if (strcmp("1852'/1815'/17'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(17);
    if (strcmp("1852'/1815'/18'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(18);
    if (strcmp("1852'/1815'/19'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(19);
    if (strcmp("1852'/1815'/20'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(20);
    if (strcmp("1852'/1815'/21'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(21);
    if (strcmp("1852'/1815'/22'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(22);
    if (strcmp("1852'/1815'/23'", path) == 0) return GetKeyDerivationAdaXPubTypeByIndex(23);
    if (strcmp("M/44'/148'/0'", path) == 0) return XPUB_TYPE_STELLAR_0;
    if (strcmp("M/44'/148'/1'", path) == 0) return XPUB_TYPE_STELLAR_1;
    if (strcmp("M/44'/148'/2'", path) == 0) return XPUB_TYPE_STELLAR_2;
    if (strcmp("M/44'/148'/3'", path) == 0) return XPUB_TYPE_STELLAR_3;
    if (strcmp("M/44'/148'/4'", path) == 0) return XPUB_TYPE_STELLAR_4;
    return GetKeyDerivationAdaXPubTypeByIndex(0);
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
    ModelParseQRHardwareCall();

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




static bool CheckHardWareCallParaIsValied()
{
    if (strcmp("1", g_callData->version) == 0) {
        // only hard ware call version 1 need check
        for (size_t i = 0; i < g_response->data->key_derivation->schemas->size; i++) {
            Response_bool *response = check_hardware_call_path(g_response->data->key_derivation->schemas->data[i].key_path, g_response->data->key_derivation->schemas->data[i].chain_type);
            if (*response->data == false) {
                printf("path is invalid\n");
                return false;
            }
        }
    }
    return true;
}

static void OnApproveHandler(lv_event_t *e)
{
    if (!CheckHardWareCallParaIsValied()) {
        GuiCreateHardwareCallInvaildPathHintbox();
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

void HiddenKeyboardAndShowAnimateQR()
{
    // close password keyboard
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    // show dynamic qr code
    GuiAnimatingQRCodeInit(g_keyDerivationTileView.qrCode, ModelGenerateSyncUR, true);
    // jump to next page to show qr code
    GuiKeyDerivationRequestNextTile();
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
}

static void SetCurrentSelectedIndex(uint8_t index)
{
    g_currentSelectedPathIndex[GetCurrentAccountIndex()] = index;
}

static uint32_t GetCurrentSelectedIndex()
{
    return g_currentSelectedPathIndex[GetCurrentAccountIndex()];
}

static void SetAccountType(uint8_t index)
{
    g_currentCardanoPathIndex[GetCurrentAccountIndex()] = index;
}

static AdaXPubType GetAccountType()
{
    return g_currentCardanoPathIndex[GetCurrentAccountIndex()];
}

static bool IsSelectChanged(void)
{
    return GetCurrentSelectedIndex() != GetAccountType();
}

static void UpdateConfirmBtn(void)
{
    if (IsSelectChanged()) {
        lv_obj_set_style_bg_opa(g_derivationPathConfirmBtn, LV_OPA_COVER,
                                LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_derivationPathConfirmBtn, 0),
                                  LV_OPA_COVER, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_opa(g_derivationPathConfirmBtn, LV_OPA_30,
                                LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_derivationPathConfirmBtn, 0),
                                  LV_OPA_30, LV_PART_MAIN);
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

static bool IsCardano()
{
    return g_walletIndex == WALLET_LIST_ETERNL || g_walletIndex == WALLET_LIST_TYPHON;
}

static void ConfirmDerivationHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED && IsSelectChanged()) {
        if (IsCardano()) {
            SetKeyDerivationAdaXPubType(GetCurrentSelectedIndex());
            SetAccountType(GetKeyDerivationAdaXPubType());
        }
        GuiAnimatingQRCodeDestroyTimer();
        GuiAnimatingQRCodeInit(g_keyDerivationTileView.qrCode, ModelGenerateSyncUR, true);
        GuiKeyDerivationRequestNextTile();
        GUI_DEL_OBJ(g_derivationPathCont);
        SetNavBarLeftBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler,
                         NULL);
        SetWallet(g_keyDerivationTileView.pageWidget->navBarWidget, g_walletIndex,
                  NULL);
        SetNavBarRightBtn(g_keyDerivationTileView.pageWidget->navBarWidget, NVS_BAR_MORE_INFO,
                          OpenMoreHandler, &g_walletIndex);
    }
}

static char *GetChangeDerivationPathDesc(void)
{
    return GetDerivationPathDescs(ADA_DERIVATION_PATH_DESC)[GetCurrentSelectedIndex()];
}

static void GetCardanoEgAddress(void)
{
    char *xPub = NULL;
    xPub = GetCurrentAccountPublicKey(GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_0 : XPUB_TYPE_LEDGER_ADA_0);
    SimpleResponse_c_char *result = cardano_get_base_address(xPub, 0, 1);
    CutAndFormatString(g_derivationPathAddr[STANDARD_ADA][0], BUFFER_SIZE_128,
                       result->data, 24);
    free_simple_response_c_char(result);

    xPub = GetCurrentAccountPublicKey(GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_1 : XPUB_TYPE_LEDGER_ADA_1);
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
    int hintboxHeight = 228;
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
    WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);
    lv_obj_t *derivationBtn = GuiCreateSelectButton(g_openMoreHintBox, _("derivation_path_change"),
                              &imgPath, ChangeDerivationPathHandler, wallet,
                              true);
    lv_obj_align(derivationBtn, LV_ALIGN_BOTTOM_MID, 0, -120);
}

static void OpenTutorialHandler(lv_event_t *e)
{
    WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);
    GuiFrameOpenViewWithParam(&g_walletTutorialView, wallet, sizeof(WALLET_LIST_INDEX_ENUM));
    GUI_DEL_OBJ(g_openMoreHintBox);
}

void GuiResetCurrentKeyDerivationCache(uint8_t index)
{
    SetKeyDerivationAdaXpubTypesByAccountIndex(STANDARD_ADA, index);
}
