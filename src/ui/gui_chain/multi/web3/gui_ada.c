#include "rust.h"
#include "keystore.h"
#include "gui_chain.h"
#include "gui_chain_components.h"
#include "screen_manager.h"
#include "keystore.h"
#include "account_manager.h"
#include "secret_cache.h"
#include "assert.h"
#include "gui_ada.h"
#include "gui_hintbox.h"
#include "gui_qr_hintbox.h"
#include "gui.h"
#include "user_memory.h"
#include "drv_mpu.h"
#define ADA_ADD_MAX_LEN             (150)
#define ADA_TAB_WIDTH               408
#define ADA_TAB_CONTENT_WIDTH       (408 - 16 * 2)
#define ADA_TAB_CONTENT_TOP_GAP     16
#define ADA_TX_VIEW_HEIGHT          542
#define ADA_TX_TAB_BAR_HEIGHT       64
#define ADA_TX_CONTENT_HEIGHT       (ADA_TX_VIEW_HEIGHT - ADA_TX_TAB_BAR_HEIGHT)
#define ADA_TX_PAGER_HEIGHT         (ADA_TX_CONTENT_HEIGHT - ADA_TAB_CONTENT_TOP_GAP - 64)
#define ADA_ASSETS_PER_PAGE         8
#define ADA_POLICY_ID_HEX_LEN       56
#define ADA_ASSET_NAME_HEX_MAX_LEN  64
#define ADA_ASSET_VERIFY_URL_PREFIX "https://cardanoscan.io/token/"

static bool g_isMulti = false;
static struct URParseResult *g_urResult = NULL;
static struct URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static char g_adaBaseAddr[ADA_ADD_MAX_LEN];
static char *g_xpub = NULL;
extern uint8_t g_viewTypeIndex;
static void Try2FixAdaPathType();
static bool IsLocalAdaPath(char *path);

AdaXPubType GetAdaXPubType(void)
{
    return GetAccountReceivePath("ADA");
}

void SetReceivePageAdaXPubType(AdaXPubType type)
{
    SetAccountReceivePath("ADA", type);
}

AdaXPubType GetReceivePageAdaXPubType(void)
{
    return GetAccountReceivePath("ADA");
}

AdaXPubType GetKeyDerivationAdaXPubType(void)
{
    return GetConnectWalletAccountIndex("adaXpub");
}

static uint8_t GetXPubIndexByPath(char *path);

void GuiSetupAdaUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

#define CHECK_FREE_PARSE_RESULT(result)                                                                         \
    if (result != NULL)                                                                                         \
    {                                                                                                           \
        free_TransactionParseResult_DisplayCardanoTx((PtrT_TransactionParseResult_DisplayCardanoTx)result);     \
        result = NULL;                                                                                          \
    }

#define CHECK_FREE_PARSE_SIGN_DATA_RESULT(result)                                                               \
    if (result != NULL)                                                                                         \
    {                                                                                                           \
        free_TransactionParseResult_DisplayCardanoSignData((PtrT_TransactionParseResult_DisplayCardanoSignData)result);     \
        result = NULL;                                                                                          \
    }

#define CHECK_FREE_PARSE_CATALYST_RESULT(result)                                                               \
    if (result != NULL)                                                                                         \
    {                                                                                                           \
        free_TransactionParseResult_DisplayCardanoCatalyst((PtrT_TransactionParseResult_DisplayCardanoCatalyst)result);     \
        result = NULL;                                                                                          \
    }

#define CHECK_FREE_PARSE_SIGN_TX_HASH_RESULT(result)                                                               \
    if (result != NULL)                                                                                         \
    {                                                                                                           \
        free_TransactionParseResult_DisplayCardanoSignTxHash((PtrT_TransactionParseResult_DisplayCardanoSignTxHash)result);     \
        result = NULL;                                                                                          \
    }

static int32_t GetAccountAdaEntropy(uint8_t accountIndex, uint8_t *entropy, uint8_t *entropyLen, const char *password, bool isSlip39)
{
    if (isSlip39) {
        *entropyLen = GetCurrentAccountEntropyLen();
        return GetAccountSeed(accountIndex, entropy, password);
    } else {
        return GetAccountEntropy(accountIndex, entropy, entropyLen, password);
    }
}

void *GuiGetAdaData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    SimpleResponse_c_char *path = NULL;
    do {
        path = cardano_get_path(data);
        CHECK_CHAIN_BREAK(path);
        char *adaPath = path->data;
        if (!IsLocalAdaPath(adaPath)) {
            g_xpub = NULL;
        } else {
            uint8_t xpubIndex = GetXPubIndexByPath(adaPath);
            g_xpub = GetCurrentAccountPublicKey(xpubIndex);
        }
        TransactionParseResult_DisplayCardanoTx *parseResult = cardano_parse_tx(data, mfp, g_xpub);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    free_simple_response_c_char(path);
    return g_parseResult;
}

void *GuiGetAdaCatalyst(void)
{
    CHECK_FREE_PARSE_CATALYST_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        TransactionParseResult_DisplayCardanoCatalyst *parseResult = cardano_parse_catalyst(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

void *GuiGetAdaSignTxHashData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplayCardanoSignTxHash parseResult = cardano_parse_sign_tx_hash(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

void FreeAdaCatalystMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_CATALYST_RESULT(g_parseResult);
}

void *GuiGetAdaSignDataData(void)
{
    CHECK_FREE_PARSE_SIGN_DATA_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        TransactionParseResult_DisplayCardanoSignData *parseResult =
            g_viewTypeIndex == CardanoSignCip8Data
                ? cardano_parse_sign_cip8_data(data)
                : cardano_parse_sign_data(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

void GetAdaSignDataDerviationPathText(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoSignData *data = (DisplayCardanoSignData *)param;
    if (data->derivation_path == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, data->derivation_path);
}

void GetAdaSignDataPayloadText(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoSignData *data = (DisplayCardanoSignData *)param;
    if (data->payload == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, data->payload);
}

int GetAdaSignDataPayloadLength(void *param)
{
    DisplayCardanoSignData *data = (DisplayCardanoSignData *)param;
    if (data->payload == NULL) {
        return 0;
    }
    return strlen(data->payload) + 1;
}

void GetAdaSignDataMessageHashText(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoSignData *data = (DisplayCardanoSignData *)param;
    if (data->message_hash == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, data->message_hash);
}

int GetAdaSignDataMessageHashLength(void *param)
{
    DisplayCardanoSignData *data = (DisplayCardanoSignData *)param;
    if (data->message_hash == NULL) {
        return 0;
    }
    return strlen(data->message_hash) + 1;
}

void GetAdaSignDataXPubText(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoSignData *data = (DisplayCardanoSignData *)param;
    if (data->xpub == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, data->xpub);
}

int GetAdaSignDataXPubLength(void *param)
{
    DisplayCardanoSignData *data = (DisplayCardanoSignData *)param;
    if (data->xpub == NULL) {
        return 0;
    }
    return strlen(data->xpub) + 1;
}

PtrT_TransactionCheckResult GuiGetAdaCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    Ptr_SimpleResponse_c_char path = cardano_get_path(data);
    if (path->error_code != 0) {
        return NULL;
    }
    char *adaPath = path->data;
    if (!IsLocalAdaPath(adaPath)) {
        g_xpub = NULL;
    } else {
        g_xpub = GetCurrentAccountPublicKey(GetXPubIndexByPath(adaPath));
    }
    PtrT_TransactionCheckResult result = cardano_check_tx(data, mfp, g_xpub);
    if (result->error_code != 0) {
        free_TransactionCheckResult(result);
        Try2FixAdaPathType();
        if (!IsLocalAdaPath(adaPath)) {
            g_xpub = NULL;
        } else {
            g_xpub = GetCurrentAccountPublicKey(GetXPubIndexByPath(adaPath));
        }
        result = cardano_check_tx(data, mfp, g_xpub);
    }
    free_simple_response_c_char(path);
    return result;
}

PtrT_TransactionCheckResult GuiGetAdaSignTxHashCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    return cardano_check_tx_hash(data, mfp);
}

static AdaXPubType GetXPubTypeByPathAndXPub(char *xpub, char *path)
{
    AdaXPubType type = STANDARD_ADA;
    for (int i = XPUB_TYPE_ADA_0; i <= XPUB_TYPE_LEDGER_ADA_23; i++) {
        char *tempXpub = GetCurrentAccountPublicKey(i);
        if (strcmp(xpub, tempXpub) == 0) {
            type = i <= XPUB_TYPE_ADA_23 ? STANDARD_ADA : LEDGER_ADA;
            break;
        }
    }
    return type;
}

PtrT_TransactionCheckResult GuiGetAdaCatalystCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    Ptr_SimpleResponse_c_char master_key_index = cardano_get_catalyst_root_index(data);
    if (master_key_index == NULL) {
        return NULL;
    }
    if (master_key_index->error_code != 0 || master_key_index->data == NULL) {
        free_simple_response_c_char(master_key_index);
        return NULL;
    }
    uint16_t index = atoi(master_key_index->data);
    free_simple_response_c_char(master_key_index);
    char *xpub = GetCurrentAccountPublicKey(GetAdaXPubTypeByIndex(index));
    PtrT_TransactionCheckResult precheckResult;
    precheckResult = cardano_check_catalyst_path_type(data, xpub);
    if (precheckResult->error_code != 0) {
        Try2FixAdaPathType();
        xpub = GetCurrentAccountPublicKey(GetAdaXPubTypeByIndex(index));
        precheckResult = cardano_check_catalyst_path_type(data, xpub);
        if (precheckResult->error_code != 0) {
            return precheckResult;
        }
    }
    free_TransactionCheckResult(precheckResult);

    PtrT_TransactionCheckResult result = cardano_check_catalyst(data, mfp);
    return result;
}

static void Try2FixAdaPathType()
{
    if (GetMnemonicType() == MNEMONIC_TYPE_SLIP39) {
        return;
    }
    if (GetAdaXPubType() == LEDGER_ADA) {
        SetReceivePageAdaXPubType(STANDARD_ADA);
    } else {
        SetReceivePageAdaXPubType(LEDGER_ADA);
    }
}

PtrT_TransactionCheckResult GuiGetAdaSignDataCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    // first check is sign opcert and then check sign cip8 or cip36 data
    PtrT_TransactionCheckResult checkCardanoCip1853SignOpcertResult = cardano_check_sign_data_is_sign_opcert(data);
    if (checkCardanoCip1853SignOpcertResult->error_code != 0) {
        Try2FixAdaPathType();
        checkCardanoCip1853SignOpcertResult = cardano_check_sign_data_is_sign_opcert(data);
        if (checkCardanoCip1853SignOpcertResult->error_code == 0) {
            free_TransactionCheckResult(checkCardanoCip1853SignOpcertResult);
            PtrT_TransactionCheckResult result = cardano_check_sign_data(data, mfp);
            return result;
        }
    } else {
        free_TransactionCheckResult(checkCardanoCip1853SignOpcertResult);
        PtrT_TransactionCheckResult result = cardano_check_sign_data(data, mfp);
        return result;
    }
    Ptr_SimpleResponse_c_char master_key_index = cardano_get_sign_data_root_index(data);
    if (master_key_index->error_code != 0) {
        return NULL;
    }
    uint16_t index = atoi(master_key_index->data);
    char *xpub = GetCurrentAccountPublicKey(GetAdaXPubTypeByIndex(index));
    PtrT_TransactionCheckResult precheckResult;
    precheckResult = cardano_check_sign_data_path_type(data, xpub);
    if (precheckResult->error_code != 0) {
        Try2FixAdaPathType();
        xpub = GetCurrentAccountPublicKey(GetAdaXPubTypeByIndex(index));
        precheckResult = cardano_check_sign_data_path_type(data, xpub);
        if (precheckResult->error_code != 0) {
            return precheckResult;
        }
    }
    free_TransactionCheckResult(precheckResult);

    PtrT_TransactionCheckResult result = cardano_check_sign_data(data, mfp);
    return result;
}

PtrT_TransactionCheckResult GuiGetAdaSignCip8DataCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);

    Ptr_SimpleResponse_c_char rootIndex = cardano_get_sign_cip8_data_root_index(data);
    if (rootIndex == NULL || rootIndex->error_code != 0 || rootIndex->data == NULL) {
        if (rootIndex != NULL) {
            free_simple_response_c_char(rootIndex);
        }
        return NULL;
    }
    uint16_t index = atoi(rootIndex->data);
    free_simple_response_c_char(rootIndex);

    char *xpub = GetCurrentAccountPublicKey(GetAdaXPubTypeByIndex(index));
    PtrT_TransactionCheckResult result = cardano_check_sign_cip8_data_path_type(data, xpub);
    if (result->error_code != 0) {
        free_TransactionCheckResult(result);
        Try2FixAdaPathType();
        xpub = GetCurrentAccountPublicKey(GetAdaXPubTypeByIndex(index));
        result = cardano_check_sign_cip8_data_path_type(data, xpub);
        if (result->error_code != 0) {
            return result;
        }
    }
    free_TransactionCheckResult(result);
    return cardano_check_sign_cip8_data(data, mfp);
}

void FreeAdaMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
}

void FreeAdaSignDataMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_SIGN_DATA_RESULT(g_parseResult);
}

void FreeAdaSignTxHashMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_SIGN_TX_HASH_RESULT(g_parseResult);
}

bool GetAdaExtraDataExist(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    if (tx->auxiliary_data == NULL) {
        return false;
    }
    return strnlen_s(tx->auxiliary_data, SIMPLERESPONSE_C_CHAR_MAX_LEN) > 0;
}

int GetAdaExtraDataLen(void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    return strnlen_s(tx->auxiliary_data, SIMPLERESPONSE_C_CHAR_MAX_LEN);
}

void GetAdaExtraData(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    strcpy_s((char *)indata, maxLen, tx->auxiliary_data);
}

void GetAdaNetwork(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    strcpy_s((char *)indata,  maxLen, tx->network);
}

void GetAdaWithdrawalsLabel(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    snprintf_s((char *)indata,  maxLen, "%d Withdraw(s)", tx->withdrawals->size);
}

void GetAdaTotalInput(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    strcpy_s((char *)indata,  maxLen, tx->total_input);
}

void GetAdaTotalOutput(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    strcpy_s((char *)indata,  maxLen, tx->total_output);
}

void GetAdaFee(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    strcpy_s((char *)indata,  maxLen, tx->fee);
}

bool GetAdaMultiAssetsExist(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    return tx->has_multi_assets;
}

void GuiShowAdaMultiAssetsWarning(lv_obj_t *parent, void *totalData)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)totalData;
    if (!tx->has_multi_assets) {
        return;
    }

    lv_obj_t *card = GuiCreateContainerWithParent(parent, ADA_TAB_CONTENT_WIDTH, LV_SIZE_CONTENT);
    lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 16);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(card, YELLOW_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_radius(card, 24, LV_PART_MAIN);

    lv_obj_t *warning_icon = GuiCreateImg(card, &imgWarning);
    lv_obj_align(warning_icon, LV_ALIGN_TOP_LEFT, 24, 20);

    lv_obj_t *title = GuiCreateTextLabel(card, _("Warning"));
    lv_obj_set_style_text_color(title, YELLOW_COLOR, LV_PART_MAIN);
    lv_obj_align_to(title, warning_icon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    lv_obj_t *content = GuiCreateIllustrateLabel(
        card, "This transaction contains native assets. Review them in Outputs before signing.");
    lv_obj_set_width(content, ADA_TAB_CONTENT_WIDTH - 48);
    lv_label_set_long_mode(content, LV_LABEL_LONG_WRAP);
    lv_obj_align(content, LV_ALIGN_TOP_LEFT, 24, 64);
    lv_obj_update_layout(content);
    lv_obj_set_height(card, 64 + lv_obj_get_height(content) + 24);
}

static lv_obj_t *GuiShowAdaReviewWarning(lv_obj_t *parent,
                                         DisplayCardanoTx *tx)
{
    bool hasAdvanced = tx->advanced_data != NULL && tx->advanced_data[0] != '\0';
    const char *message = NULL;
    if (!tx->transaction_is_valid) {
        message = "This transaction is marked invalid. Review all transaction details before signing.";
    } else if (tx->has_unknown_inputs && tx->has_multi_assets && hasAdvanced) {
        message = "Some input details are unavailable, and this transaction contains native assets and advanced data. Review Inputs, Outputs, and Advanced before signing.";
    } else if (tx->has_unknown_inputs && tx->has_multi_assets) {
        message = "Some input details are unavailable, and this transaction contains native assets. Review Inputs and Outputs before signing.";
    } else if (tx->has_unknown_inputs && hasAdvanced) {
        message = "Some input details are unavailable, and this transaction contains advanced data. Review Inputs and Advanced before signing.";
    } else if (tx->has_multi_assets && hasAdvanced) {
        message = "This transaction contains native assets and advanced data. Review Outputs and Advanced before signing.";
    } else if (tx->has_unknown_inputs) {
        message = "Some input details are unavailable. Review Inputs before signing.";
    } else if (tx->has_multi_assets) {
        message = "This transaction contains native assets. Review Outputs before signing.";
    } else if (hasAdvanced) {
        message = "This transaction contains advanced data. Review Advanced before signing.";
    }
    if (message == NULL) {
        return NULL;
    }
    lv_obj_t *card = GuiCreateContainerWithParent(
        parent, ADA_TAB_CONTENT_WIDTH, LV_SIZE_CONTENT);
    lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 16);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(card, YELLOW_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_radius(card, 24, LV_PART_MAIN);
    lv_obj_t *icon = GuiCreateImg(card, &imgWarning);
    lv_obj_align(icon, LV_ALIGN_TOP_LEFT, 24, 20);
    lv_obj_t *title = GuiCreateTextLabel(card, _("Warning"));
    lv_obj_set_style_text_color(title, YELLOW_COLOR, LV_PART_MAIN);
    lv_obj_align_to(title, icon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
    lv_obj_t *content = GuiCreateIllustrateLabel(card, message);
    lv_obj_set_width(content, ADA_TAB_CONTENT_WIDTH - 48);
    lv_label_set_long_mode(content, LV_LABEL_LONG_WRAP);
    lv_obj_align(content, LV_ALIGN_TOP_LEFT, 24, 64);
    lv_obj_update_layout(content);
    lv_obj_set_height(card, 64 + lv_obj_get_height(content) + 24);
    return card;
}

static void GuiAdaAppendDetailValue(lv_obj_t *card, uint16_t *height,
                                    const char *labelText, const char *value,
                                    bool highlight)
{
    if (value == NULL) {
        return;
    }
    lv_obj_t *label = GuiCreateIllustrateLabel(card, labelText);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, *height);
    *height += 34;

    label = GuiCreateIllustrateLabel(card, value[0] == '\0' ? "(empty)" : value);
    lv_obj_set_width(label, ADA_TAB_CONTENT_WIDTH - 48);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    if (highlight) {
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    }
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, *height);
    lv_obj_update_layout(label);
    *height += lv_obj_get_height(label) + 12;
}

static void GuiAdaAppendAssetDetailValue(lv_obj_t *card, uint16_t *height,
                                         const char *labelText,
                                         const char *value, bool highlight)
{
    if (value == NULL) {
        return;
    }
    lv_obj_t *label = GuiCreateIllustrateLabel(card, labelText);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, *height);
    *height += 34;

    label = GuiCreateIllustrateLabel(card, value[0] == '\0' ? "(empty)" : value);
    lv_obj_set_width(label, ADA_TAB_CONTENT_WIDTH - 96);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    if (highlight) {
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    }
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, *height);
    lv_obj_update_layout(label);
    *height += lv_obj_get_height(label) + 12;
}

static void GuiAdaOpenAssetVerifyQrCode(lv_event_t *e)
{
    DisplayCardanoAsset *asset = (DisplayCardanoAsset *)lv_event_get_user_data(e);
    if (asset == NULL || asset->policy_id == NULL || asset->name_hex == NULL) {
        return;
    }

    size_t policyIdLen = strnlen_s(asset->policy_id, ADA_POLICY_ID_HEX_LEN + 1);
    size_t assetNameLen = strnlen_s(asset->name_hex, ADA_ASSET_NAME_HEX_MAX_LEN + 1);
    if (policyIdLen != ADA_POLICY_ID_HEX_LEN || assetNameLen > ADA_ASSET_NAME_HEX_MAX_LEN) {
        return;
    }

    char url[sizeof(ADA_ASSET_VERIFY_URL_PREFIX) + ADA_POLICY_ID_HEX_LEN +
             ADA_ASSET_NAME_HEX_MAX_LEN] = {0};
    snprintf_s(url, sizeof(url), "%s%s%s", ADA_ASSET_VERIFY_URL_PREFIX,
               asset->policy_id, asset->name_hex);
    GuiQRCodeHintBoxOpenCompact(url, "Verify asset", "cardanoscan.io");
}

static void GuiAdaAppendAssetVerifyLink(lv_obj_t *card, uint16_t *height,
                                        DisplayCardanoAsset *asset)
{
    if (asset == NULL) {
        return;
    }

    lv_obj_t *container = GuiCreateContainerWithParent(card, 180, 36);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(container, LV_ALIGN_TOP_LEFT, 24, *height);
    lv_obj_add_event_cb(container, GuiAdaOpenAssetVerifyQrCode,
                        LV_EVENT_CLICKED, asset);

    lv_obj_t *label = GuiCreateIllustrateLabel(container, "Verify asset");
    lv_obj_set_style_text_color(label, BLUE_GREEN_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *icon = GuiCreateImg(container, &imgQrcodeTurquoise);
    lv_obj_align_to(icon, label, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    lv_obj_clear_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    *height += 48;
}

static void GuiAdaAppendAssets(lv_obj_t *card, uint16_t *height,
                               Ptr_VecFFI_DisplayCardanoAsset assets,
                               size_t start, size_t count)
{
    if (assets == NULL || start >= assets->size) {
        return;
    }
    size_t end = start + count;
    if (end > assets->size || end < start) {
        end = assets->size;
    }
    for (size_t i = start; i < end; i++) {
        lv_obj_t *assetCard = GuiCreateContainerWithParent(
            card, ADA_TAB_CONTENT_WIDTH - 48, 0);
        lv_obj_align(assetCard, LV_ALIGN_TOP_LEFT, 24, *height);
        lv_obj_clear_flag(assetCard, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(assetCard, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_radius(assetCard, 16, LV_PART_MAIN);
        lv_obj_set_style_bg_color(assetCard, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(assetCard, LV_OPA_12, LV_PART_MAIN);

        uint16_t assetHeight = 16;
        char title[BUFFER_SIZE_32] = {0};
        snprintf_s(title, sizeof(title), "Native asset #%u", (unsigned)(i + 1));
        lv_obj_t *label = GuiCreateIllustrateLabel(assetCard, title);
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, assetHeight);
        assetHeight += 42;

        DisplayCardanoAsset *asset = &assets->data[i];
        GuiAdaAppendAssetDetailValue(
            assetCard, &assetHeight, "Asset name",
            asset->name == NULL ? "(not readable)" : asset->name,
            asset->name == NULL);
        GuiAdaAppendAssetDetailValue(
            assetCard, &assetHeight, "Raw amount", asset->amount, false);
        GuiAdaAppendAssetDetailValue(
            assetCard, &assetHeight, "Asset name (hex)", asset->name_hex, false);
        GuiAdaAppendAssetDetailValue(
            assetCard, &assetHeight, "Policy ID", asset->policy_id, false);
        GuiAdaAppendAssetVerifyLink(assetCard, &assetHeight, asset);
        lv_obj_set_height(assetCard, assetHeight + 4);
        *height += assetHeight + 20;
    }
}

static const char *GuiAdaMintAction(const char *amount)
{
    if (amount != NULL && (strcmp(amount, "0") == 0 || strcmp(amount, "-0") == 0)) {
        return "No change";
    }
    return amount != NULL && amount[0] == '-' ? "Burn" : "Mint";
}

static void GuiAdaAppendOverviewAsset(lv_obj_t *card, uint16_t *height,
                                      DisplayCardanoAsset *asset, size_t index,
                                      bool showMintAction)
{
    if (!showMintAction) {
        char title[BUFFER_SIZE_32] = {0};
        snprintf_s(title, sizeof(title), "Asset #%u", (unsigned)(index + 1));
        lv_obj_t *label = GuiCreateIllustrateLabel(card, title);
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, *height);
        *height += 42;
    }

    const char *amount = asset->amount;
    if (showMintAction) {
        if (amount != NULL && amount[0] == '-') {
            amount++;
        }
    }
    GuiAdaAppendAssetDetailValue(
        card, height, "Asset name",
        asset->name == NULL ? "(not readable)" : asset->name,
        asset->name == NULL);
    GuiAdaAppendAssetDetailValue(
        card, height, showMintAction ? "Amount" : "Raw amount", amount, false);
    GuiAdaAppendAssetDetailValue(
        card, height, "Asset name (hex)", asset->name_hex, false);
    GuiAdaAppendAssetDetailValue(
        card, height, "Policy ID", asset->policy_id, false);
    GuiAdaAppendAssetVerifyLink(card, height, asset);
}

static void GuiAdaAddOwnershipBadge(lv_obj_t *card, lv_obj_t *anchor,
                                    const char *text)
{
    lv_obj_t *badge = GuiCreateContainerWithParent(card, 87, 30);
    lv_obj_set_style_radius(badge, 16, LV_PART_MAIN);
    lv_obj_set_style_bg_color(badge, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(badge, LV_OPA_12, LV_PART_MAIN);
    lv_obj_t *label = GuiCreateIllustrateLabel(badge, text);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);
    lv_obj_center(label);
    lv_obj_align_to(badge, anchor, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
}

static bool GuiAdaOutputIsMine(const DisplayCardanoTx *tx,
                               const DisplayCardanoTo *output)
{
    if (tx->from == NULL || output->address == NULL) {
        return false;
    }
    for (size_t i = 0; i < tx->from->size; i++) {
        const DisplayCardanoFrom *input = &tx->from->data[i];
        if (input->has_path && input->address != NULL &&
            strcmp(input->address, output->address) == 0) {
            return true;
        }
    }
    return false;
}

static lv_obj_t *GuiAdaCreateOutputCard(lv_obj_t *parent, lv_obj_t *lastView,
                                        DisplayCardanoTx *tx,
                                        DisplayCardanoTo *output, size_t index,
                                        size_t assetStart, size_t assetCount)
{
    lv_obj_t *card = CreateRelativeTransactionContentContainer(
        parent, ADA_TAB_CONTENT_WIDTH, 0, lastView);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
    uint16_t height = 16;
    char title[BUFFER_SIZE_32] = {0};
    snprintf_s(title, sizeof(title), "Output #%u", (unsigned)(index + 1));
    lv_obj_t *label = GuiCreateIllustrateLabel(card, title);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    if (GuiAdaOutputIsMine(tx, output)) {
        GuiAdaAddOwnershipBadge(card, label, "Change");
    }
    height += 42;

    GuiAdaAppendDetailValue(card, &height, "ADA amount", output->amount, true);
    GuiAdaAppendDetailValue(card, &height, "Address", output->address, false);
    if (output->assets != NULL && output->assets->size > ADA_ASSETS_PER_PAGE) {
        char range[BUFFER_SIZE_64] = {0};
        size_t assetEnd = assetStart + assetCount;
        if (assetEnd > output->assets->size) {
            assetEnd = output->assets->size;
        }
        snprintf_s(range, sizeof(range), "%u-%u of %u",
                   (unsigned)(assetStart + 1), (unsigned)assetEnd,
                   (unsigned)output->assets->size);
        GuiAdaAppendDetailValue(card, &height, "Native assets", range, false);
    }
    GuiAdaAppendAssets(card, &height, output->assets, assetStart, assetCount);
    lv_obj_set_height(card, height + 4);
    return card;
}

static lv_obj_t *GuiAdaCreateInputCard(lv_obj_t *parent, lv_obj_t *lastView,
                                       DisplayCardanoFrom *input, size_t index)
{
    lv_obj_t *card = CreateRelativeTransactionContentContainer(
        parent, ADA_TAB_CONTENT_WIDTH, 0, lastView);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
    uint16_t height = 16;
    char title[BUFFER_SIZE_32] = {0};
    snprintf_s(title, sizeof(title), "Input #%u", (unsigned)(index + 1));
    lv_obj_t *label = GuiCreateIllustrateLabel(card, title);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    if (input->known && input->has_path) {
        GuiAdaAddOwnershipBadge(card, label, "Mine");
    }
    height += 42;

    if (input->known) {
        GuiAdaAppendDetailValue(card, &height, "ADA amount", input->amount, true);
        GuiAdaAppendDetailValue(card, &height, "Address", input->address, false);
        if (input->has_path) {
            GuiAdaAppendDetailValue(card, &height, "Path", input->path, false);
        }
    } else {
        GuiAdaAppendDetailValue(card, &height, "Details", "Unavailable", true);
    }
    char inputIndex[BUFFER_SIZE_32] = {0};
    snprintf_s(inputIndex, sizeof(inputIndex), "%u", (unsigned)input->index);
    GuiAdaAppendDetailValue(card, &height, "Transaction ID",
                            input->transaction_id, false);
    GuiAdaAppendDetailValue(card, &height, "Output index", inputIndex, false);
    lv_obj_set_height(card, height + 4);
    return card;
}

typedef struct {
    DisplayCardanoTx *tx;
    bool outputs;
    size_t page;
    size_t page_count;
    lv_obj_t *viewport;
    lv_obj_t *prev;
    lv_obj_t *next;
    lv_obj_t *page_label;
} GuiAdaUtxoPager_t;

static size_t GuiAdaOutputPageCount(const DisplayCardanoTo *output)
{
    size_t assetCount = output->assets == NULL ? 0 : output->assets->size;
    return assetCount == 0 ? 1
                           : (assetCount + ADA_ASSETS_PER_PAGE - 1) /
                                 ADA_ASSETS_PER_PAGE;
}

static bool GuiAdaResolveOutputPage(const DisplayCardanoTx *tx, size_t page,
                                    size_t *outputIndex, size_t *assetPage,
                                    size_t *assetStart)
{
    if (tx->to == NULL) {
        return false;
    }
    for (size_t i = 0; i < tx->to->size; i++) {
        size_t pages = GuiAdaOutputPageCount(&tx->to->data[i]);
        if (page < pages) {
            *outputIndex = i;
            *assetPage = page;
            *assetStart = page * ADA_ASSETS_PER_PAGE;
            return true;
        }
        page -= pages;
    }
    return false;
}

static lv_obj_t *GuiAdaPagerButton(lv_obj_t *parent, const char *text)
{
    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_set_size(button, 72, 48);
    lv_obj_set_style_radius(button, 12, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button, LV_OPA_12, LV_PART_MAIN);
    lv_obj_t *label = GuiCreateIllustrateLabel(button, text);
    lv_obj_center(label);
    return button;
}

typedef struct {
    Ptr_VecFFI_DisplayCardanoAsset assets;
    bool show_mint_action;
    size_t page;
    lv_obj_t *section;
    lv_obj_t *title_label;
    lv_obj_t *viewport;
    lv_obj_t *prev;
    lv_obj_t *next;
    lv_obj_t *page_label;
} GuiAdaAssetPager_t;

static void GuiAdaAssetPagerRefresh(GuiAdaAssetPager_t *state)
{
    lv_obj_clean(state->viewport);
    uint16_t height = 0;
    GuiAdaAppendOverviewAsset(state->viewport, &height,
                              &state->assets->data[state->page], state->page,
                              state->show_mint_action);
    lv_obj_set_height(state->viewport, height);
    lv_coord_t pagerY = 58 + height + 12;
    lv_obj_align(state->prev, LV_ALIGN_TOP_LEFT, 24, pagerY);
    lv_obj_align(state->next, LV_ALIGN_TOP_RIGHT, -24, pagerY);
    lv_obj_align(state->page_label, LV_ALIGN_TOP_MID, 0, pagerY + 10);
    lv_coord_t oldSectionHeight = lv_obj_get_height(state->section);
    lv_coord_t newSectionHeight = pagerY + 48 + 16;
    lv_obj_set_height(state->section, newSectionHeight);
    if (oldSectionHeight > 1 && oldSectionHeight != newSectionHeight) {
        lv_obj_t *parent = lv_obj_get_parent(state->section);
        bool afterSection = false;
        uint32_t childCount = lv_obj_get_child_cnt(parent);
        for (uint32_t i = 0; i < childCount; i++) {
            lv_obj_t *child = lv_obj_get_child(parent, i);
            if (afterSection) {
                lv_obj_set_y(child, lv_obj_get_y(child) +
                                        newSectionHeight - oldSectionHeight);
            } else if (child == state->section) {
                afterSection = true;
            }
        }
    }
    if (state->show_mint_action) {
        lv_label_set_text(
            state->title_label,
            GuiAdaMintAction(state->assets->data[state->page].amount));
    }
    lv_label_set_text_fmt(state->page_label, "Asset %u/%u",
                          (unsigned)(state->page + 1),
                          (unsigned)state->assets->size);
    if (state->page == 0) {
        lv_obj_add_state(state->prev, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(state->prev, LV_STATE_DISABLED);
    }
    if (state->page + 1 >= state->assets->size) {
        lv_obj_add_state(state->next, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(state->next, LV_STATE_DISABLED);
    }
}

static void GuiAdaAssetPagerEvent(lv_event_t *event)
{
    GuiAdaAssetPager_t *state = lv_event_get_user_data(event);
    lv_obj_t *target = lv_event_get_target(event);
    if (target == state->prev && state->page > 0) {
        state->page--;
    } else if (target == state->next && state->page + 1 < state->assets->size) {
        state->page++;
    } else {
        return;
    }
    GuiAdaAssetPagerRefresh(state);
}

static void GuiAdaAssetPagerDelete(lv_event_t *event)
{
    SRAM_FREE(lv_event_get_user_data(event));
}

static lv_obj_t *GuiAdaAppendAssetPager(
    lv_obj_t *parent, lv_obj_t *lastView, const char *title,
    Ptr_VecFFI_DisplayCardanoAsset assets, bool showMintAction)
{
    if (assets == NULL || assets->size == 0) {
        return lastView;
    }
    bool singleAsset = assets->size == 1;
    lv_obj_t *section = CreateRelativeTransactionContentContainer(
        parent, ADA_TAB_CONTENT_WIDTH, 0, lastView);
    lv_obj_clear_flag(section, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(section, LV_OBJ_FLAG_CLICKABLE);

    const char *sectionTitle = showMintAction
                                   ? GuiAdaMintAction(assets->data[0].amount)
                                   : title;
    lv_obj_t *titleLabel = GuiCreateIllustrateLabel(section, sectionTitle);
    lv_obj_set_style_text_color(titleLabel, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 24, 16);

    if (singleAsset) {
        uint16_t height = 58;
        GuiAdaAppendOverviewAsset(section, &height, &assets->data[0], 0,
                                  showMintAction);
        lv_obj_set_height(section, height + 16);
        return section;
    }

    GuiAdaAssetPager_t *state = SRAM_MALLOC(sizeof(GuiAdaAssetPager_t));
    if (state == NULL) {
        return section;
    }
    memset(state, 0, sizeof(GuiAdaAssetPager_t));
    state->assets = assets;
    state->show_mint_action = showMintAction;
    state->section = section;
    state->title_label = titleLabel;
    state->viewport = GuiCreateContainerWithParent(
        section, ADA_TAB_CONTENT_WIDTH, 1);
    lv_obj_align(state->viewport, LV_ALIGN_TOP_LEFT, 0, 58);
    lv_obj_set_style_bg_opa(state->viewport, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(state->viewport, 0, LV_PART_MAIN);
    lv_obj_clear_flag(state->viewport, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(state->viewport, LV_OBJ_FLAG_CLICKABLE);

    state->prev = GuiAdaPagerButton(section, "<");
    state->next = GuiAdaPagerButton(section, ">");
    state->page_label = GuiCreateIllustrateLabel(section, "");
    lv_obj_add_event_cb(state->prev, GuiAdaAssetPagerEvent,
                        LV_EVENT_CLICKED, state);
    lv_obj_add_event_cb(state->next, GuiAdaAssetPagerEvent,
                        LV_EVENT_CLICKED, state);
    lv_obj_add_event_cb(section, GuiAdaAssetPagerDelete,
                        LV_EVENT_DELETE, state);
    GuiAdaAssetPagerRefresh(state);
    return section;
}

static void GuiAdaUtxoPagerRefresh(GuiAdaUtxoPager_t *state)
{
    bool twoLinePageLabel = false;
    lv_obj_clean(state->viewport);
    if (state->outputs) {
        size_t outputIndex = 0;
        size_t assetPage = 0;
        size_t assetStart = 0;
        if (GuiAdaResolveOutputPage(state->tx, state->page, &outputIndex,
                                    &assetPage, &assetStart)) {
            DisplayCardanoTo *output = &state->tx->to->data[outputIndex];
            GuiAdaCreateOutputCard(state->viewport, NULL, state->tx,
                                   output, outputIndex,
                                   assetStart, ADA_ASSETS_PER_PAGE);
            size_t outputCount = state->tx->to->size;
            size_t assetPages = GuiAdaOutputPageCount(output);
            if (assetPages > 1) {
                twoLinePageLabel = true;
                lv_label_set_text_fmt(
                    state->page_label, "Output %u/%u\nAsset page %u/%u",
                    (unsigned)(outputIndex + 1), (unsigned)outputCount,
                    (unsigned)(assetPage + 1), (unsigned)assetPages);
            } else {
                lv_label_set_text_fmt(state->page_label, "Output %u/%u",
                                      (unsigned)(outputIndex + 1),
                                      (unsigned)outputCount);
            }
        }
    } else if (state->tx->from != NULL && state->page < state->tx->from->size) {
        GuiAdaCreateInputCard(state->viewport, NULL,
                              &state->tx->from->data[state->page], state->page);
        lv_label_set_text_fmt(state->page_label, "Input %u/%u",
                              (unsigned)(state->page + 1),
                              (unsigned)state->page_count);
    }
    lv_obj_update_layout(state->viewport);
    lv_obj_scroll_to_y(state->viewport, 0, LV_ANIM_OFF);
    lv_obj_update_layout(state->page_label);
    lv_obj_align(state->page_label, LV_ALIGN_BOTTOM_MID, 0,
                 twoLinePageLabel ? -1 : -9);
    if (state->page == 0) {
        lv_obj_add_state(state->prev, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(state->prev, LV_STATE_DISABLED);
    }
    if (state->page + 1 >= state->page_count) {
        lv_obj_add_state(state->next, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(state->next, LV_STATE_DISABLED);
    }
}

static void GuiAdaUtxoPagerEvent(lv_event_t *event)
{
    GuiAdaUtxoPager_t *state = lv_event_get_user_data(event);
    lv_obj_t *target = lv_event_get_target(event);
    if (target == state->prev && state->page > 0) {
        state->page--;
    } else if (target == state->next && state->page + 1 < state->page_count) {
        state->page++;
    } else {
        return;
    }
    GuiAdaUtxoPagerRefresh(state);
}

static void GuiAdaUtxoPagerDelete(lv_event_t *event)
{
    SRAM_FREE(lv_event_get_user_data(event));
}

static void GuiShowAdaUtxos(lv_obj_t *parent, DisplayCardanoTx *tx, bool outputs)
{
    size_t count = outputs ? (tx->to == NULL ? 0 : tx->to->size)
                           : (tx->from == NULL ? 0 : tx->from->size);
    if (count == 0) {
        if (outputs) {
            lv_obj_t *emptyCard = CreateTransactionItemViewWithWidth(
                parent, "Outputs", "No outputs", NULL, ADA_TAB_CONTENT_WIDTH);
            lv_obj_align(emptyCard, LV_ALIGN_TOP_MID, 0, 0);
            lv_obj_clear_flag(emptyCard, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(emptyCard, LV_OBJ_FLAG_CLICKABLE);
        }
        return;
    }

    GuiAdaUtxoPager_t *state = SRAM_MALLOC(sizeof(GuiAdaUtxoPager_t));
    if (state == NULL) {
        return;
    }
    memset(state, 0, sizeof(GuiAdaUtxoPager_t));
    state->tx = tx;
    state->outputs = outputs;
    if (outputs) {
        state->page_count = 0;
        for (size_t i = 0; i < count; i++) {
            state->page_count += GuiAdaOutputPageCount(&tx->to->data[i]);
        }
    } else {
        state->page_count = count;
    }

    lv_obj_set_height(parent, ADA_TX_CONTENT_HEIGHT);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    state->viewport = GuiCreateContainerWithParent(
        parent, ADA_TAB_CONTENT_WIDTH, ADA_TX_PAGER_HEIGHT);
    lv_obj_align(state->viewport, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(state->viewport, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(state->viewport, 0, LV_PART_MAIN);
    lv_obj_set_scroll_dir(state->viewport, LV_DIR_VER);
    lv_obj_add_flag(state->viewport, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(state->viewport, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(state->viewport, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_clear_flag(state->viewport, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_clear_flag(state->viewport, LV_OBJ_FLAG_GESTURE_BUBBLE);

    state->prev = GuiAdaPagerButton(parent, "<");
    lv_obj_align(state->prev, LV_ALIGN_BOTTOM_LEFT, 16, 0);
    state->next = GuiAdaPagerButton(parent, ">");
    lv_obj_align(state->next, LV_ALIGN_BOTTOM_RIGHT, -16, 0);
    state->page_label = GuiCreateIllustrateLabel(parent, "");
    lv_obj_set_style_text_align(state->page_label, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN);
    lv_obj_align(state->page_label, LV_ALIGN_BOTTOM_MID, 0, -1);

    lv_obj_add_event_cb(state->prev, GuiAdaUtxoPagerEvent, LV_EVENT_CLICKED, state);
    lv_obj_add_event_cb(state->next, GuiAdaUtxoPagerEvent, LV_EVENT_CLICKED, state);
    lv_obj_add_event_cb(parent, GuiAdaUtxoPagerDelete, LV_EVENT_DELETE, state);
    GuiAdaUtxoPagerRefresh(state);
}

void GuiShowAdaInputs(lv_obj_t *parent, void *totalData)
{
    GuiShowAdaUtxos(parent, (DisplayCardanoTx *)totalData, false);
}

void GuiShowAdaOutputs(lv_obj_t *parent, void *totalData)
{
    GuiShowAdaUtxos(parent, (DisplayCardanoTx *)totalData, true);
}

static lv_obj_t *GuiAdaAppendOverviewItem(lv_obj_t *parent, lv_obj_t *lastView,
                                           const char *title, const char *value)
{
    lv_obj_t *card = CreateTransactionItemViewWithWidth(
        parent, title, value == NULL ? "" : value, lastView, ADA_TAB_CONTENT_WIDTH);
    if (lastView == NULL) {
        lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 0);
    }
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
    return card;
}

static lv_obj_t *GuiAdaAppendCertificateCard(
    lv_obj_t *parent, lv_obj_t *lastView,
    const DisplayCardanoCertificate *certificate, size_t index)
{
    lv_obj_t *card = CreateRelativeTransactionContentContainer(
        parent, ADA_TAB_CONTENT_WIDTH, 0, lastView);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
    uint16_t height = 16;
    char title[BUFFER_SIZE_32] = {0};
    snprintf_s(title, sizeof(title), "Certificate #%u", (unsigned)(index + 1));
    lv_obj_t *label = GuiCreateIllustrateLabel(card, title);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    height += 42;

    GuiAdaAppendDetailValue(card, &height, "Type", certificate->cert_type, true);
    if (certificate->fields != NULL) {
        for (size_t i = 0; i < certificate->fields->size; i++) {
            DisplayCertField *field = &certificate->fields->data[i];
            GuiAdaAppendDetailValue(card, &height, field->label, field->value, false);
        }
    }
    lv_obj_set_height(card, height + 4);
    return card;
}

static lv_obj_t *GuiAdaAppendWithdrawalCard(
    lv_obj_t *parent, lv_obj_t *lastView,
    const DisplayCardanoWithdrawal *withdrawal, size_t index)
{
    lv_obj_t *card = CreateRelativeTransactionContentContainer(
        parent, ADA_TAB_CONTENT_WIDTH, 0, lastView);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
    uint16_t height = 16;
    char title[BUFFER_SIZE_32] = {0};
    snprintf_s(title, sizeof(title), "Withdrawal #%u", (unsigned)(index + 1));
    lv_obj_t *label = GuiCreateIllustrateLabel(card, title);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    height += 42;

    GuiAdaAppendDetailValue(card, &height, "Amount", withdrawal->amount, true);
    GuiAdaAppendDetailValue(card, &height, "Stake address", withdrawal->address, false);
    lv_obj_set_height(card, height + 4);
    return card;
}

static lv_obj_t *GuiAdaAppendVotingProcedureCard(
    lv_obj_t *parent, lv_obj_t *lastView,
    const DisplayVotingProcedure *procedure, size_t index)
{
    lv_obj_t *card = CreateRelativeTransactionContentContainer(
        parent, ADA_TAB_CONTENT_WIDTH, 0, lastView);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
    uint16_t height = 16;
    char title[BUFFER_SIZE_32] = {0};
    snprintf_s(title, sizeof(title), "Voting procedure #%u",
               (unsigned)(index + 1));
    lv_obj_t *label = GuiCreateIllustrateLabel(card, title);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    height += 42;

    GuiAdaAppendDetailValue(card, &height, "Voter type",
                            procedure->voter_type, false);
    GuiAdaAppendDetailValue(card, &height, "Voter", procedure->voter, false);
    GuiAdaAppendDetailValue(
        card, &height, "Transaction ID", procedure->transaction_id, false);
    GuiAdaAppendDetailValue(card, &height, "Index", procedure->index, false);
    GuiAdaAppendDetailValue(card, &height, "Vote", procedure->vote, false);
    lv_obj_set_height(card, height + 4);
    return card;
}

static lv_obj_t *GuiAdaAppendVotingProposalCard(
    lv_obj_t *parent, lv_obj_t *lastView,
    const DisplayVotingProposal *proposal, size_t index)
{
    lv_obj_t *card = CreateRelativeTransactionContentContainer(
        parent, ADA_TAB_CONTENT_WIDTH, 0, lastView);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
    uint16_t height = 16;
    char title[BUFFER_SIZE_32] = {0};
    snprintf_s(title, sizeof(title), "Voting proposal #%u",
               (unsigned)(index + 1));
    lv_obj_t *label = GuiCreateIllustrateLabel(card, title);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    height += 42;
    GuiAdaAppendDetailValue(card, &height, "Governance action",
                            proposal->action, false);
    GuiAdaAppendDetailValue(card, &height, "Deposit", proposal->deposit, true);
    GuiAdaAppendDetailValue(card, &height, "Reward account",
                            proposal->reward_account, false);
    GuiAdaAppendDetailValue(card, &height, "Anchor URL",
                            proposal->anchor_url, false);
    GuiAdaAppendDetailValue(card, &height, "Anchor data hash",
                            proposal->anchor_data_hash, false);
    lv_obj_set_height(card, height + 4);
    return card;
}

void GuiShowAdaSignData(lv_obj_t *parent, void *totalData)
{
    DisplayCardanoSignData *data = (DisplayCardanoSignData *)totalData;
    lv_obj_t *templateParent = lv_obj_get_parent(parent);
    if (templateParent != NULL) {
        lv_obj_clear_flag(templateParent, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(templateParent, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_scrollbar_mode(templateParent, LV_SCROLLBAR_MODE_OFF);
        lv_obj_scroll_to_y(templateParent, 0, LV_ANIM_OFF);
    }
    lv_obj_set_size(parent, ADA_TAB_WIDTH, 542);
    lv_obj_set_scroll_dir(parent, LV_DIR_VER);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);

    lv_obj_t *lastView = NULL;
    if (data->payload_is_hex) {
        lv_obj_t *card = GuiCreateContainerWithParent(
            parent, ADA_TAB_CONTENT_WIDTH, LV_SIZE_CONTENT);
        lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(card, YELLOW_COLOR, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(card, LV_OPA_20, LV_PART_MAIN);
        lv_obj_set_style_radius(card, 24, LV_PART_MAIN);

        lv_obj_t *icon = GuiCreateImg(card, &imgWarning);
        lv_obj_align(icon, LV_ALIGN_TOP_LEFT, 24, 20);
        lv_obj_t *title = GuiCreateTextLabel(card, "Warning");
        lv_obj_set_style_text_color(title, YELLOW_COLOR, LV_PART_MAIN);
        lv_obj_align_to(title, icon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
        lv_obj_t *content = GuiCreateIllustrateLabel(
            card, "This message cannot be displayed as text. Verify the raw hex before signing.");
        lv_obj_set_width(content, ADA_TAB_CONTENT_WIDTH - 48);
        lv_label_set_long_mode(content, LV_LABEL_LONG_WRAP);
        lv_obj_align(content, LV_ALIGN_TOP_LEFT, 24, 64);
        lv_obj_update_layout(content);
        lv_obj_set_height(card, 64 + lv_obj_get_height(content) + 24);
        lastView = card;
    }

    lastView = GuiAdaAppendOverviewItem(
        parent, lastView, "Signing type",
        g_viewTypeIndex == CardanoSignCip8Data ? "CIP-8 message" : "Cardano sign data");
    lastView = GuiAdaAppendOverviewItem(parent, lastView, "Path", data->derivation_path);
    lastView = GuiAdaAppendOverviewItem(parent, lastView, "Public key", data->xpub);
    if (data->hash_payload) {
        lastView = GuiAdaAppendOverviewItem(
            parent, lastView, "Signing hash", data->message_hash);
    }
    GuiAdaAppendOverviewItem(parent, lastView, "Message", data->payload);
    lv_obj_update_layout(parent);
    lv_obj_scroll_to_y(parent, 0, LV_ANIM_OFF);
}

static void GuiShowAdaOverview(lv_obj_t *parent, DisplayCardanoTx *tx)
{
    lv_obj_set_height(parent, ADA_TX_CONTENT_HEIGHT);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scroll_dir(parent, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *lastView = GuiShowAdaReviewWarning(parent, tx);
    lastView = GuiAdaAppendOverviewItem(parent, lastView, "Network", tx->network);
    lastView = GuiAdaAppendOverviewItem(
        parent, lastView, tx->has_unknown_inputs ? "Known input total" : "Total input",
        tx->total_input);
    if (tx->withdrawals_total != NULL && tx->withdrawals_total[0] != '\0') {
        lastView = GuiAdaAppendOverviewItem(
            parent, lastView, "Withdrawals total", tx->withdrawals_total);
    }
    lastView = GuiAdaAppendOverviewItem(parent, lastView, "Total output", tx->total_output);
    lastView = GuiAdaAppendOverviewItem(parent, lastView, "Fee", tx->fee);
    char countText[BUFFER_SIZE_32] = {0};
    if (tx->validity_start != NULL && tx->validity_start[0] != '\0') {
        lastView = GuiAdaAppendOverviewItem(
            parent, lastView, "Validity start", tx->validity_start);
    }
    if (tx->ttl != NULL && tx->ttl[0] != '\0') {
        lastView = GuiAdaAppendOverviewItem(parent, lastView, "TTL", tx->ttl);
    }
    lastView = GuiAdaAppendOverviewItem(
        parent, lastView, "Script validity",
        tx->transaction_is_valid ? "Valid flag" : "Invalid flag");
    lastView = GuiAdaAppendAssetPager(
        parent, lastView, "Mint / burn", tx->mint_assets, true);
    if (tx->collateral_inputs_count > 0) {
        snprintf_s(countText, sizeof(countText), "%u",
                   (unsigned)tx->collateral_inputs_count);
        lastView = GuiAdaAppendOverviewItem(
            parent, lastView, "Collateral inputs", countText);
    }
    if (tx->total_collateral != NULL && tx->total_collateral[0] != '\0') {
        lastView = GuiAdaAppendOverviewItem(
            parent, lastView, "Total collateral", tx->total_collateral);
    }
    if (tx->collateral_return != NULL && tx->collateral_return[0] != '\0') {
        lastView = GuiAdaAppendOverviewItem(
            parent, lastView, "Collateral return", tx->collateral_return);
    }
    lastView = GuiAdaAppendAssetPager(
        parent, lastView, "Collateral return assets", tx->collateral_assets, false);
    if (tx->reference_inputs_count > 0) {
        snprintf_s(countText, sizeof(countText), "%u",
                   (unsigned)tx->reference_inputs_count);
        lastView = GuiAdaAppendOverviewItem(
            parent, lastView, "Reference inputs", countText);
    }
    if (tx->donation != NULL && tx->donation[0] != '\0') {
        lastView = GuiAdaAppendOverviewItem(parent, lastView, "Donation", tx->donation);
    }
    if (tx->certificates != NULL && tx->certificates->size > 0) {
        for (size_t i = 0; i < tx->certificates->size; i++) {
            lastView = GuiAdaAppendCertificateCard(
                parent, lastView, &tx->certificates->data[i], i);
        }
    }
    if (tx->withdrawals != NULL && tx->withdrawals->size > 0) {
        for (size_t i = 0; i < tx->withdrawals->size; i++) {
            lastView = GuiAdaAppendWithdrawalCard(
                parent, lastView, &tx->withdrawals->data[i], i);
        }
    }
    if (tx->voting_procedures != NULL && tx->voting_procedures->size > 0) {
        for (size_t i = 0; i < tx->voting_procedures->size; i++) {
            lastView = GuiAdaAppendVotingProcedureCard(
                parent, lastView, &tx->voting_procedures->data[i], i);
        }
    }
    if (tx->voting_proposals != NULL && tx->voting_proposals->size > 0) {
        for (size_t i = 0; i < tx->voting_proposals->size; i++) {
            lastView = GuiAdaAppendVotingProposalCard(
                parent, lastView, &tx->voting_proposals->data[i], i);
        }
    }
    if (tx->required_signers_count > 0) {
        snprintf_s(countText, sizeof(countText), "%u",
                   (unsigned)tx->required_signers_count);
        lastView = GuiAdaAppendOverviewItem(parent, lastView, "Required signers", countText);
    }
    if (tx->auxiliary_data != NULL && tx->auxiliary_data[0] != '\0') {
        lastView = GuiAdaAppendOverviewItem(parent, lastView, "Auxiliary data", "Present");
    }
    lv_obj_scroll_to_y(parent, 0, LV_ANIM_OFF);
}

static void GuiAdaStyleTab(lv_obj_t *tab)
{
    lv_obj_set_style_pad_all(tab, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(tab, ADA_TAB_CONTENT_TOP_GAP, LV_PART_MAIN);
    lv_obj_set_style_border_width(tab, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(tab, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tab, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(tab, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(tab, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_clear_flag(tab, LV_OBJ_FLAG_SCROLLABLE);
}

static void GuiShowAdaJsonData(lv_obj_t *parent, const char *json);

void GuiShowAdaTx(lv_obj_t *parent, void *totalData)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)totalData;
    bool hasAdvanced = tx->advanced_data != NULL && tx->advanced_data[0] != '\0';
    lv_obj_t *templateParent = lv_obj_get_parent(parent);
    if (templateParent != NULL) {
        lv_obj_clear_flag(templateParent, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(templateParent, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_scrollbar_mode(templateParent, LV_SCROLLBAR_MODE_OFF);
    }
    lv_obj_set_size(parent, ADA_TAB_WIDTH, ADA_TX_VIEW_HEIGHT);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *tabview = lv_tabview_create(parent, LV_DIR_TOP,
                                          ADA_TX_TAB_BAR_HEIGHT);
    lv_obj_set_size(tabview, ADA_TAB_WIDTH, ADA_TX_VIEW_HEIGHT);
    lv_obj_set_style_bg_color(tabview, BLACK_COLOR, LV_PART_MAIN | LV_PART_ITEMS);
    lv_obj_set_style_border_width(tabview, 0, LV_PART_MAIN);
    lv_obj_clear_flag(tabview, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tabview, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scrollbar_mode(tabview, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *tabContent = lv_tabview_get_content(tabview);
    lv_obj_set_style_bg_color(tabContent, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tabContent, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_scroll_dir(tabContent, LV_DIR_HOR);
    lv_obj_clear_flag(tabContent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tabContent, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_set_scrollbar_mode(tabContent, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *tabButtons = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_width(tabButtons, ADA_TAB_WIDTH);
    lv_obj_set_style_bg_color(tabButtons, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_font(tabButtons, g_defIllustrateFont, LV_PART_ITEMS);
    lv_obj_set_style_text_color(tabButtons, WHITE_COLOR, LV_PART_ITEMS);
    lv_obj_set_style_text_color(tabButtons, WHITE_COLOR,
                                LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(tabButtons, LV_OPA_60, LV_PART_ITEMS);
    lv_obj_set_style_text_opa(tabButtons, LV_OPA_COVER,
                              LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(tabButtons, ORANGE_COLOR,
                                  LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_border_side(tabButtons, LV_BORDER_SIDE_BOTTOM,
                                 LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_pad_left(tabButtons, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_right(tabButtons, 0, LV_PART_ITEMS);
    lv_obj_set_style_text_letter_space(tabButtons, 0, LV_PART_ITEMS);

    lv_obj_t *overview = lv_tabview_add_tab(tabview, "Overview");
    lv_obj_t *inputs = lv_tabview_add_tab(tabview, "Inputs");
    lv_obj_t *outputs = lv_tabview_add_tab(tabview, "Outputs");
    lv_obj_t *advanced = hasAdvanced
                             ? lv_tabview_add_tab(tabview, "Advanced")
                             : NULL;
    lv_obj_t *raw = lv_tabview_add_tab(tabview, "Raw");
    lv_btnmatrix_set_btn_width(tabButtons, 0, 7);
    lv_btnmatrix_set_btn_width(tabButtons, 1, 5);
    lv_btnmatrix_set_btn_width(tabButtons, 2, 6);
    if (hasAdvanced) {
        lv_btnmatrix_set_btn_width(tabButtons, 3, 7);
        lv_btnmatrix_set_btn_width(tabButtons, 4, 3);
    } else {
        lv_btnmatrix_set_btn_width(tabButtons, 3, 3);
    }
    GuiAdaStyleTab(overview);
    GuiAdaStyleTab(inputs);
    GuiAdaStyleTab(outputs);
    if (advanced != NULL) {
        GuiAdaStyleTab(advanced);
    }
    GuiAdaStyleTab(raw);

    GuiShowAdaOverview(overview, tx);
    GuiShowAdaInputs(inputs, tx);
    GuiShowAdaOutputs(outputs, tx);
    if (advanced != NULL) {
        GuiShowAdaJsonData(advanced, tx->advanced_data);
    }
    GuiShowAdaRawData(raw, tx);
}

static void GuiShowAdaJsonData(lv_obj_t *parent, const char *json)
{
    if (json == NULL || json[0] == '\0') {
        return;
    }

    lv_obj_set_height(parent, ADA_TX_CONTENT_HEIGHT);
    lv_obj_t *content = GuiCreateContainerWithParent(
        parent, ADA_TAB_CONTENT_WIDTH,
        ADA_TX_CONTENT_HEIGHT - ADA_TAB_CONTENT_TOP_GAP);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, LV_PART_MAIN);
    GuiShowPagedMessageText(content, json, true, NULL, NULL);
}

void GuiShowAdaRawData(lv_obj_t *parent, void *totalData)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)totalData;
    if (tx->raw_data == NULL) {
        return;
    }

    lv_obj_set_height(parent, ADA_TX_CONTENT_HEIGHT);
    lv_obj_t *content = GuiCreateContainerWithParent(
        parent, ADA_TAB_CONTENT_WIDTH,
        ADA_TX_CONTENT_HEIGHT - ADA_TAB_CONTENT_TOP_GAP);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, LV_PART_MAIN);
    GuiShowPagedMessageText(content, tx->raw_data, true, NULL, NULL);
}

void *GetAdaInputDetail(uint8_t *row, uint8_t *col, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *col = 1;
    *row = 3 * tx->from->size;
    int i = 0, j = 0;
    char ***indata = (char ***)SRAM_MALLOC(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        if (*row == 0) {
            indata[i] = NULL;
            break;
        }
        indata[i] = SRAM_MALLOC(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            uint32_t index = j / 3;
            indata[i][j] = SRAM_MALLOC(BUFFER_SIZE_128);
            if (j % 3 == 0) {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "%d #F5870A %s#", (int)(index + 1), tx->from->data[index].amount);
            } else if (j % 3 == 1) {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "%s", tx->from->data[index].address);
            } else {
                if (tx->from->data[index].has_path) {
                    strcpy_s(indata[i][j], BUFFER_SIZE_128, tx->from->data[index].path);
                } else {
                    memset_s(indata[i][j], BUFFER_SIZE_128, 0, BUFFER_SIZE_128);
                }
            }
        }
    }
    return (void *)indata;
}

void *GetAdaOutputDetail(uint8_t *row, uint8_t *col, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *col = 1;
    if (tx->to->size == 0) {
        *row = 0;
        return NULL;
    }
    *row = 2 * tx->to->size;
    int i = 0, j = 0;
    char ***indata = (char ***)SRAM_MALLOC(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        indata[i] = SRAM_MALLOC(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            indata[i][j] = SRAM_MALLOC(BUFFER_SIZE_128);
            uint32_t index = j / 2;
            if (j % 2 == 0) {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "%d #F5870A %s#", (int)(index + 1), tx->to->data[index].amount);
            } else {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "%s", tx->to->data[index].address);
            }
        }
    }
    return (void *)indata;
}
void GetAdaInputDetailSize(uint16_t *width, uint16_t *height, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *width = 408;
    *height = 16 + 30 + 218 * tx->from->size + 16;
}
void GetAdaOutputDetailSize(uint16_t *width, uint16_t *height, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *width = 408;
    if (tx->to->size == 0) {
        *height = 0;
    } else {
        *height = 16 + 30 + 154 * tx->to->size + 16;
    }
}

void GetAdaVotingProceduresLabel(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    snprintf_s((char *)indata,  maxLen, "%d Voting Procedure(s)", tx->voting_procedures->size);
}

bool GetAdaVotingProceduresExist(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    return tx->voting_procedures->size > 0;
}

bool GetAdaVotingProposalsExist(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    return tx->voting_proposals->size > 0;
}

void GetAdaVotingProposalsLabel(void *indata, void *param, uint32_t maxLen)
{
    snprintf_s((char *)indata,  maxLen, "%s", _("ada_proposals_tx_notice"));
}

void GetAdaVotingProceduresSize(uint16_t *width, uint16_t *height, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *width = 408;
    *height = 16 + 30 + 312 * tx->voting_procedures->size + 16;
}

void *GetAdaVotingProceduresData(uint8_t *row, uint8_t *col, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *col = 1;
    *row = 5 * tx->voting_procedures->size;
    int i = 0, j = 0;
    char ***indata = (char ***)SRAM_MALLOC(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        indata[i] = SRAM_MALLOC(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            uint32_t index = j / 5;
            indata[i][j] = SRAM_MALLOC(BUFFER_SIZE_128);
            if (j % 5 == 0) {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "##%d\n#F5870A Voting Procedure#", (int)(index + 1));
            } else if (j % 5 == 1) {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "Voter: %s", tx->voting_procedures->data[index].voter);
            } else if (j % 5 == 2) {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "Transaction Id: %s", tx->voting_procedures->data[index].transaction_id);
            } else if (j % 5 == 3) {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "Index: %s", tx->voting_procedures->data[index].index);
            } else {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "Vote: %s", tx->voting_procedures->data[index].vote);
            }
        }
    }
    return (void *)indata;
}

bool GetAdaWithdrawalsExist(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    return tx->withdrawals->size > 0;
}

void GetAdaWithdrawalsSize(uint16_t *width, uint16_t *height, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *width = 408;
    *height = 16 + 128 * tx->withdrawals->size + 16;
}

void *GetAdaWithdrawalsData(uint8_t *row, uint8_t *col, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *col = 1;
    *row = 2 * tx->withdrawals->size;
    int i = 0, j = 0;
    char ***indata = (char ***)SRAM_MALLOC(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        indata[i] = SRAM_MALLOC(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            uint32_t index = j / 2;
            indata[i][j] = SRAM_MALLOC(BUFFER_SIZE_128);
            if (j % 2 == 0) {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "%d #F5870A %s#", (int)(index + 1), tx->withdrawals->data[index].amount);
            } else {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "Address: %s", tx->withdrawals->data[index].address);
            }
        }
    }
    return (void *)indata;
}

UREncodeResult *GuiGetAdaSignCatalystVotingRegistrationQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);

    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t entropy[64];
        uint8_t len = 0;
        bool isSlip39 = GetMnemonicType() == MNEMONIC_TYPE_SLIP39;
        GetAccountAdaEntropy(GetCurrentAccountIndex(), entropy, &len, SecretCacheGetPassword(), isSlip39);
        if (GetAdaXPubType() == LEDGER_ADA) {
            char *mnemonic = NULL;
            bip39_mnemonic_from_bytes(NULL, entropy, len, &mnemonic);
            encodeResult = cardano_sign_catalyst_with_ledger_bitbox02(data, mnemonic, GetPassphrase(GetCurrentAccountIndex()));
            if (mnemonic != NULL) {
                ClearSensitiveCString(mnemonic);
                SRAM_FREE(mnemonic);
            }
        } else {
            encodeResult = cardano_sign_catalyst(data, entropy, len, GetPassphrase(GetCurrentAccountIndex()), isSlip39);
        }
        memset_s(entropy, sizeof(entropy), 0, sizeof(entropy));
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

UREncodeResult *GuiGetAdaSignSignDataQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);

    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t entropy[64];
        uint8_t len = 0;
        bool isSlip39 = GetMnemonicType() == MNEMONIC_TYPE_SLIP39;
        GetAccountAdaEntropy(GetCurrentAccountIndex(), entropy, &len, SecretCacheGetPassword(), isSlip39);
        if (GetAdaXPubType() == LEDGER_ADA) {
            char *mnemonic = NULL;
            bip39_mnemonic_from_bytes(NULL, entropy, len, &mnemonic);
            encodeResult = cardano_sign_sign_data_with_ledger_bitbox02(data, mnemonic, GetPassphrase(GetCurrentAccountIndex()));
            if (mnemonic != NULL) {
                ClearSensitiveCString(mnemonic);
                SRAM_FREE(mnemonic);
            }
        } else {
            encodeResult = cardano_sign_sign_data(data, entropy, len, GetPassphrase(GetCurrentAccountIndex()), isSlip39);
        }
        memset_s(entropy, sizeof(entropy), 0, sizeof(entropy));
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

UREncodeResult *GuiGetAdaSignSignCip8DataQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);

    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t entropy[64];
        uint8_t len = 0;
        bool isSlip39 = GetMnemonicType() == MNEMONIC_TYPE_SLIP39;
        GetAccountAdaEntropy(GetCurrentAccountIndex(), entropy, &len, SecretCacheGetPassword(), isSlip39);
        if (GetAdaXPubType() == LEDGER_ADA) {
            char *mnemonic = NULL;
            bip39_mnemonic_from_bytes(NULL, entropy, len, &mnemonic);
            encodeResult = cardano_sign_sign_cip8_data_with_ledger_bitbox02(data, mnemonic, GetPassphrase(GetCurrentAccountIndex()));
            if (mnemonic != NULL) {
                ClearSensitiveCString(mnemonic);
                SRAM_FREE(mnemonic);
            }
        } else {
            encodeResult = cardano_sign_sign_cip8_data(data, entropy, len, GetPassphrase(GetCurrentAccountIndex()), isSlip39);
        }
        memset_s(entropy, sizeof(entropy), 0, sizeof(entropy));
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

UREncodeResult *GuiGetAdaSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);

    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t entropy[64];
        uint8_t len = 0;
        bool isSlip39 = GetMnemonicType() == MNEMONIC_TYPE_SLIP39;
        GetAccountAdaEntropy(GetCurrentAccountIndex(), entropy, &len, SecretCacheGetPassword(), isSlip39);
        if (GetAdaXPubType() == LEDGER_ADA) {
            char *mnemonic = NULL;
            bip39_mnemonic_from_bytes(NULL, entropy, len, &mnemonic);
            encodeResult = cardano_sign_tx_with_ledger_bitbox02(data, mfp, g_xpub, mnemonic, GetPassphrase(GetCurrentAccountIndex()), false);
            if (mnemonic != NULL) {
                ClearSensitiveCString(mnemonic);
                SRAM_FREE(mnemonic);
            }
        } else {
            encodeResult = cardano_sign_tx(data, mfp, g_xpub, entropy, len, GetPassphrase(GetCurrentAccountIndex()), false, isSlip39);
        }
        memset_s(entropy, sizeof(entropy), 0, sizeof(entropy));
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}
static void SetContainerDefaultStyle(lv_obj_t *container)
{
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(container, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
}

UREncodeResult *GuiGetAdaSignTxHashQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);

    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t entropy[64];
        uint8_t len = 0;
        bool isSlip39 = GetMnemonicType() == MNEMONIC_TYPE_SLIP39;
        GetAccountAdaEntropy(GetCurrentAccountIndex(), entropy, &len, SecretCacheGetPassword(), isSlip39);
        if (GetAdaXPubType() == LEDGER_ADA) {
            char *mnemonic = NULL;
            bip39_mnemonic_from_bytes(NULL, entropy, len, &mnemonic);
            encodeResult = cardano_sign_tx_with_ledger_bitbox02(data, mfp, g_xpub, mnemonic, GetPassphrase(GetCurrentAccountIndex()), true);
            if (mnemonic != NULL) {
                ClearSensitiveCString(mnemonic);
                SRAM_FREE(mnemonic);
            }
        } else {
            encodeResult = cardano_sign_tx(data, mfp, g_xpub, entropy, len, GetPassphrase(GetCurrentAccountIndex()), true, isSlip39);
        }
        memset_s(entropy, sizeof(entropy), 0, sizeof(entropy));
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}
lv_obj_t *GuiCreateAdaAutoHeightContainer(lv_obj_t *parent, uint16_t width, uint16_t padding_x)
{
    lv_obj_t * container = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    SetContainerDefaultStyle(container);
    lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(container, padding_x, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(container, padding_x, LV_PART_MAIN);
    return container;
}

lv_obj_t* GuiCreateAdaNoticeCard(lv_obj_t* parent)
{
    lv_obj_t* card = GuiCreateAdaAutoHeightContainer(parent, 408, 24);
    SetContainerDefaultStyle(card);
    lv_obj_set_style_bg_color(card, YELLOW_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_radius(card, 24, LV_PART_MAIN);

    lv_obj_t* warningIcon = GuiCreateImg(card, &imgWarning);
    lv_obj_align(warningIcon, LV_ALIGN_TOP_LEFT, 24, 0);

    lv_obj_t* title_label = GuiCreateTextLabel(card, "Blind signing");
    lv_obj_set_style_text_color(title_label, YELLOW_COLOR, LV_PART_MAIN);
    lv_obj_align_to(title_label, warningIcon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    lv_obj_t* content_label = GuiCreateIllustrateLabel(
        card,
        "Only the transaction hash is available. The recipient, amount, and other transaction details cannot be verified on this device.");
    lv_obj_set_style_text_color(content_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_width(content_label, 360);
    lv_label_set_long_mode(content_label, LV_LABEL_LONG_WRAP);
    lv_obj_align_to(content_label, warningIcon, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    return card;
}
static void SetTitleLabelStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void SetFlexContainerStyle(lv_obj_t *container, lv_flex_flow_t flow, lv_coord_t padding_y)
{
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(container, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(container, flow);
    lv_obj_set_style_pad_top(container, padding_y, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(container, padding_y, LV_PART_MAIN);
    lv_obj_set_style_pad_left(container, 24, LV_PART_MAIN);
}

lv_obj_t *GuiAdaCreateLabelCard(lv_obj_t *parent, const char *title, const char *content)
{
    lv_obj_t *card = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    SetFlexContainerStyle(card, LV_FLEX_FLOW_ROW, 16);
    lv_obj_t *title_label = GuiCreateTextLabel(card, title);
    lv_obj_align_to(title_label, card, LV_ALIGN_OUT_TOP_LEFT, 24, 16);
    SetTitleLabelStyle(title_label);
    lv_obj_t *content_label = GuiCreateIllustrateLabel(card, content);
    lv_obj_align_to(content_label, title_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    return card;
}

void GuiShowAdaSignTxHashOverview(lv_obj_t *parent, void *totalData)
{
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    DisplayCardanoSignTxHash *hashData = (DisplayCardanoSignTxHash *)totalData;
    lv_obj_t * noticeCard = GuiCreateAdaNoticeCard(parent);
    lv_obj_align(noticeCard, LV_ALIGN_DEFAULT, 0, 0);
    lv_obj_update_layout(noticeCard);
    int containerYOffset = lv_obj_get_height(noticeCard) + 16;
    // network  container
    lv_obj_t *network_card = GuiAdaCreateLabelCard(parent, "Network", hashData->network);
    lv_obj_align(network_card, LV_ALIGN_DEFAULT, 0, containerYOffset);
    lv_obj_update_layout(network_card);
    containerYOffset += lv_obj_get_height(network_card) + 16;
    // tx hash container
    lv_obj_t *tx_hash_card = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    lv_obj_align(tx_hash_card, LV_ALIGN_DEFAULT, 0, containerYOffset);
    SetFlexContainerStyle(tx_hash_card, LV_FLEX_FLOW_COLUMN, 16);
    lv_obj_t *tx_hash_label = GuiCreateTextLabel(tx_hash_card, "Hash");
    lv_obj_set_style_text_opa(tx_hash_label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(tx_hash_label, tx_hash_card, LV_ALIGN_OUT_BOTTOM_LEFT, 24, 16);

    // color tx hash
    char hash[128] = {0};
    strncpy(hash, hashData->tx_hash, sizeof(hash) - 1);
    char tempBuf[128] = {0};
    snprintf(tempBuf, sizeof(tempBuf), "#F5870A %.8s#%.24s\n%.24s#F5870A %.8s#", hash, &hash[8], &hash[32], &hash[56]);

    lv_obj_t *tx_hash_value = GuiCreateIllustrateLabel(tx_hash_card, tempBuf);
    lv_label_set_recolor(tx_hash_value, true);
    lv_obj_align_to(tx_hash_value, tx_hash_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
}

void GuiShowAdaSignTxHashDetails(lv_obj_t *parent, void *totalData)
{
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    DisplayCardanoSignTxHash *hashData = (DisplayCardanoSignTxHash *)totalData;
    // network card
    lv_obj_t *network_card = GuiAdaCreateLabelCard(parent, "Network", hashData->network);
    lv_obj_align(network_card, LV_ALIGN_DEFAULT, 0, 0);
    lv_obj_update_layout(network_card);
    int containerYOffset = lv_obj_get_height(network_card) + 16;
    // From Conatiner
    lv_obj_t *from_container = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    lv_obj_align(from_container, LV_ALIGN_DEFAULT, 0, containerYOffset);
    SetFlexContainerStyle(from_container, LV_FLEX_FLOW_COLUMN, 16);
    lv_obj_t *from_label = GuiCreateTextLabel(from_container, "From");
    lv_obj_set_style_text_opa(from_label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(from_label, from_container, LV_ALIGN_OUT_BOTTOM_LEFT, 24, 16);
    // address + path card
    Ptr_VecFFI_PtrString addressList = hashData->address_list;
    Ptr_VecFFI_PtrString pathList = hashData->path;
    if (addressList == NULL || pathList == NULL || addressList->size != pathList->size ||
        (addressList->size > 0 && (addressList->data == NULL || pathList->data == NULL))) {
        return;
    }
    for (size_t i = 0; i < addressList->size; i++) {
        if (addressList->data[i] == NULL || pathList->data[i] == NULL) {
            return;
        }
    }
    for (size_t i = 0; i < addressList->size; i++) {
        char *address = addressList->data[i];
        char *path = pathList->data[i];
        char formattedPath[128] = {0};
        snprintf(formattedPath, sizeof(formattedPath), "m/%s", path);
        char num[BUFFER_SIZE_32] = {0};
        snprintf(num, sizeof(num), "%u", (unsigned int)(i + 1));
        lv_obj_t *num_label = GuiCreateIllustrateLabel(from_container, num);
        lv_obj_set_style_text_color(num_label, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_opa(num_label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align_to(num_label, from_container, LV_ALIGN_OUT_TOP_LEFT, 24, 16);
        lv_obj_t *address_label = GuiCreateIllustrateLabel(from_container, address);
        lv_obj_set_width(address_label, lv_pct(90));
        lv_obj_set_style_text_color(address_label, WHITE_COLOR, LV_PART_MAIN);
        lv_label_set_long_mode(address_label, LV_LABEL_LONG_WRAP);
        lv_obj_align_to(address_label, num_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
        lv_obj_t *path_label = GuiCreateIllustrateLabel(from_container, formattedPath);
        lv_obj_set_width(path_label, lv_pct(90));
        lv_label_set_long_mode(path_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_color(path_label, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_opa(path_label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align_to(path_label, address_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    }
    // calculate the height of from_container
    lv_obj_update_layout(from_container);
    containerYOffset += lv_obj_get_height(from_container) + 16;
    // tx hash container
    lv_obj_t *tx_hash_card = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    lv_obj_align(tx_hash_card, LV_ALIGN_DEFAULT, 0, containerYOffset);
    SetFlexContainerStyle(tx_hash_card, LV_FLEX_FLOW_COLUMN, 16);
    lv_obj_t *tx_hash_label = GuiCreateTextLabel(tx_hash_card, "Hash");
    lv_obj_set_style_text_opa(tx_hash_label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(tx_hash_label, tx_hash_card, LV_ALIGN_OUT_BOTTOM_LEFT, 24, 1);

    // color message hash
    char hash[128] = {0};
    strncpy(hash, hashData->tx_hash, sizeof(hash) - 1);
    char tempBuf[128] = {0};
    snprintf(tempBuf, sizeof(tempBuf), "#F5870A %.8s#%.24s\n%.24s#F5870A %.8s#", hash, &hash[8], &hash[32], &hash[56]);

    lv_obj_t *tx_hash_value = GuiCreateIllustrateLabel(tx_hash_card, tempBuf);
    lv_label_set_recolor(tx_hash_value, true);
    lv_obj_align_to(tx_hash_value, tx_hash_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_label_set_long_mode(tx_hash_value, LV_LABEL_LONG_WRAP);

    lv_obj_t *tx_hash_notice_content = GuiCreateIllustrateLabel(
        tx_hash_card,
        "Blind signing: verify this hash with the software wallet before signing.");
    lv_obj_set_width(tx_hash_notice_content, lv_pct(90));
    lv_obj_set_style_text_color(tx_hash_notice_content, lv_color_hex(0xF5C131), LV_PART_MAIN);
    lv_obj_align_to(tx_hash_notice_content, tx_hash_value, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_label_set_long_mode(tx_hash_notice_content, LV_LABEL_LONG_WRAP);
}

UREncodeResult *GuiGetAdaSignUrDataUnlimited(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);

    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t entropy[64];
        uint8_t len = 0;
        bool isSlip39 = GetMnemonicType() == MNEMONIC_TYPE_SLIP39;
        GetAccountAdaEntropy(GetCurrentAccountIndex(), entropy, &len, SecretCacheGetPassword(), isSlip39);
        if (GetAdaXPubType() == LEDGER_ADA) {
            char *mnemonic = NULL;
            bip39_mnemonic_from_bytes(NULL, entropy, len, &mnemonic);
            encodeResult = cardano_sign_tx_with_ledger_bitbox02_unlimited(data, mfp, g_xpub, mnemonic, GetPassphrase(GetCurrentAccountIndex()));
            if (mnemonic != NULL) {
                ClearSensitiveCString(mnemonic);
                SRAM_FREE(mnemonic);
            }
        } else {
            encodeResult = cardano_sign_tx_unlimited(data, mfp, g_xpub, entropy, len, GetPassphrase(GetCurrentAccountIndex()), isSlip39);
        }
        memset_s(entropy, sizeof(entropy), 0, sizeof(entropy));
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

ChainType GetAdaXPubTypeByIndexAndDerivationType(AdaXPubType type, uint32_t index)
{
    bool valid = index <= XPUB_TYPE_ADA_23 - XPUB_TYPE_ADA_0 &&
                 (type == STANDARD_ADA || type == LEDGER_ADA);
    if (!valid) {
        ClearSecretCache();
    }
    ASSERT(valid);
    ChainType base = (type == STANDARD_ADA) ? XPUB_TYPE_ADA_0 : XPUB_TYPE_LEDGER_ADA_0;
    return (ChainType)(base + index);
}

ChainType GetAdaXPubTypeByIndex(uint32_t index)
{
    return GetAdaXPubTypeByIndexAndDerivationType(GetAdaXPubType(), index);
}

ChainType GetReceivePageAdaXPubTypeByIndex(uint32_t index)
{
    return GetAdaXPubTypeByIndexAndDerivationType(GetReceivePageAdaXPubType(), index);
}

char *GuiGetADABaseAddressByXPub(char *xPub)
{
    memset_s(g_adaBaseAddr, sizeof(g_adaBaseAddr), 0, sizeof(g_adaBaseAddr));
    SimpleResponse_c_char *result = NULL;
    if (xPub != NULL && xPub[0] != '\0') {
        result = cardano_get_base_address(xPub, 0, 1);
    }
    bool valid = result != NULL && result->error_code == 0 && result->data != NULL && result->data[0] != '\0' &&
                 strnlen_s(result->data, sizeof(g_adaBaseAddr)) < sizeof(g_adaBaseAddr);
    if (valid) {
        valid = strcpy_s(g_adaBaseAddr, sizeof(g_adaBaseAddr), result->data) == 0;
    }
    if (result != NULL) {
        free_simple_response_c_char(result);
    }
    if (!valid) {
        memset_s(g_adaBaseAddr, sizeof(g_adaBaseAddr), 0, sizeof(g_adaBaseAddr));
        ClearSecretCache();
    }
    ASSERT(valid);
    return g_adaBaseAddr;
}

static bool IsLocalAdaPath(char *path)
{
    return strcmp("1852'/1815'/0'", path) == 0 ||
           strcmp("1852'/1815'/1'", path) == 0 ||
           strcmp("1852'/1815'/2'", path) == 0 ||
           strcmp("1852'/1815'/3'", path) == 0 ||
           strcmp("1852'/1815'/4'", path) == 0 ||
           strcmp("1852'/1815'/5'", path) == 0 ||
           strcmp("1852'/1815'/6'", path) == 0 ||
           strcmp("1852'/1815'/7'", path) == 0 ||
           strcmp("1852'/1815'/8'", path) == 0 ||
           strcmp("1852'/1815'/9'", path) == 0 ||
           strcmp("1852'/1815'/10'", path) == 0 ||
           strcmp("1852'/1815'/11'", path) == 0 ||
           strcmp("1852'/1815'/12'", path) == 0 ||
           strcmp("1852'/1815'/13'", path) == 0 ||
           strcmp("1852'/1815'/14'", path) == 0 ||
           strcmp("1852'/1815'/15'", path) == 0 ||
           strcmp("1852'/1815'/16'", path) == 0 ||
           strcmp("1852'/1815'/17'", path) == 0 ||
           strcmp("1852'/1815'/18'", path) == 0 ||
           strcmp("1852'/1815'/19'", path) == 0 ||
           strcmp("1852'/1815'/20'", path) == 0 ||
           strcmp("1852'/1815'/21'", path) == 0 ||
           strcmp("1852'/1815'/22'", path) == 0 ||
           strcmp("1852'/1815'/23'", path) == 0;
}

static uint8_t GetXPubIndexByPath(char *path)
{
    if (strcmp("1852'/1815'/1'", path) == 0)
        return GetAdaXPubTypeByIndex(1);
    if (strcmp("1852'/1815'/2'", path) == 0)
        return GetAdaXPubTypeByIndex(2);
    if (strcmp("1852'/1815'/3'", path) == 0)
        return GetAdaXPubTypeByIndex(3);
    if (strcmp("1852'/1815'/4'", path) == 0)
        return GetAdaXPubTypeByIndex(4);
    if (strcmp("1852'/1815'/5'", path) == 0)
        return GetAdaXPubTypeByIndex(5);
    if (strcmp("1852'/1815'/6'", path) == 0)
        return GetAdaXPubTypeByIndex(6);
    if (strcmp("1852'/1815'/7'", path) == 0)
        return GetAdaXPubTypeByIndex(7);
    if (strcmp("1852'/1815'/8'", path) == 0)
        return GetAdaXPubTypeByIndex(8);
    if (strcmp("1852'/1815'/9'", path) == 0)
        return GetAdaXPubTypeByIndex(9);
    if (strcmp("1852'/1815'/10'", path) == 0)
        return GetAdaXPubTypeByIndex(10);
    if (strcmp("1852'/1815'/11'", path) == 0)
        return GetAdaXPubTypeByIndex(11);
    if (strcmp("1852'/1815'/12'", path) == 0)
        return GetAdaXPubTypeByIndex(12);
    if (strcmp("1852'/1815'/13'", path) == 0)
        return GetAdaXPubTypeByIndex(13);
    if (strcmp("1852'/1815'/14'", path) == 0)
        return GetAdaXPubTypeByIndex(14);
    if (strcmp("1852'/1815'/15'", path) == 0)
        return GetAdaXPubTypeByIndex(15);
    if (strcmp("1852'/1815'/16'", path) == 0)
        return GetAdaXPubTypeByIndex(16);
    if (strcmp("1852'/1815'/17'", path) == 0)
        return GetAdaXPubTypeByIndex(17);
    if (strcmp("1852'/1815'/18'", path) == 0)
        return GetAdaXPubTypeByIndex(18);
    if (strcmp("1852'/1815'/19'", path) == 0)
        return GetAdaXPubTypeByIndex(19);
    if (strcmp("1852'/1815'/20'", path) == 0)
        return GetAdaXPubTypeByIndex(20);
    if (strcmp("1852'/1815'/21'", path) == 0)
        return GetAdaXPubTypeByIndex(21);
    if (strcmp("1852'/1815'/22'", path) == 0)
        return GetAdaXPubTypeByIndex(22);
    if (strcmp("1852'/1815'/23'", path) == 0)
        return GetAdaXPubTypeByIndex(23);
    return GetAdaXPubTypeByIndex(0);
}

void GetCatalystNonce(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoCatalyst *data = (DisplayCardanoCatalyst *)param;
    if (data->nonce == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, data->nonce);
}

void GetCatalystVotePublicKey(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoCatalyst *data = (DisplayCardanoCatalyst *)param;
    if (data->stake_key == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, data->stake_key);
}

void GetCatalystRewardsNotice(lv_obj_t *parent, void *totalData)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(parent, 360, 60);
    lv_obj_set_width(parent, 360);
    lv_obj_set_style_bg_opa(parent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *notice = lv_label_create(parent);
    lv_obj_set_width(notice, 360);
    lv_label_set_long_mode(notice, LV_LABEL_LONG_WRAP);
    lv_label_set_text(notice, _("catalyst_transactions_notice"));
    lv_obj_set_style_text_font(notice, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(notice, ORANGE_COLOR, LV_PART_MAIN);
}

void GetCatalystRewards(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoCatalyst *data = (DisplayCardanoCatalyst *)param;
    if (data->rewards == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, data->rewards);
}

void GetCatalystVoteKeys(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoCatalyst *data = (DisplayCardanoCatalyst *)param;
    if (data->vote_keys->size == 0) {
        return;
    }
    memset_s(indata, maxLen, 0, maxLen);
    for (uint32_t i = 0; i < data->vote_keys->size; i++) {
        strcat_s((char *)indata, maxLen, data->vote_keys->data[i]);
        if (i != data->vote_keys->size - 1) {
            strcat_s((char *)indata, maxLen, "\n");
        }
    }
}

void GetCatalystVoteKeysSize(uint16_t *width, uint16_t *height, void *param)
{
    DisplayCardanoCatalyst *data = (DisplayCardanoCatalyst *)param;
    *width = 408;
    *height = 62 + 60 * data->vote_keys->size;
}
