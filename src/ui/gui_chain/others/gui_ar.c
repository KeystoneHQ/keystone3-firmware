#ifndef BTC_ONLY
#include "gui_ar.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static ArweaveRequestType g_requestType = ArweaveRequestTypeTransaction;
static void *g_parseResult = NULL;
static lv_obj_t *g_parseErrorHintBox = NULL;

#define CHECK_FREE_PARSE_RESULT(result)                                                                                           \
    if (result != NULL)                                                                                                           \
    {                                                                                                                             \
        switch (g_requestType)                                                                                                    \
        {                                                                                                                         \
        case ArweaveRequestTypeTransaction:                                                                                       \
            free_TransactionParseResult_DisplayArweaveTx((PtrT_TransactionParseResult_DisplayArweaveTx)result);                   \
            break;                                                                                                                \
        case ArweaveRequestTypeMessage:                                                                                           \
            free_TransactionParseResult_DisplayArweaveMessage((PtrT_TransactionParseResult_DisplayArweaveMessage)result);         \
            break;                                                                                                                \
        default:                                                                                                                  \
            break;                                                                                                                \
        }                                                                                                                         \
        result = NULL;                                                                                                            \
    }

static void ParseRequestType();
static void SetTitleLabelStyle(lv_obj_t *label);
static void TagsRender(cJSON *root, int size, lv_obj_t *parent);
static void ThrowError(int32_t errorCode);

static void ThrowError(int32_t errorCode)
{
    g_parseErrorHintBox = GuiCreateErrorCodeWindow(errorCode, &g_parseErrorHintBox, NULL);
}

static void ParseRequestType()
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    SimpleResponse_ArweaveRequestType *requestType = ar_request_type(data);
    if (requestType->error_code != 0) {
        g_requestType = ArweaveRequestTypeUnknown;
    }
    g_requestType = *requestType->data;
}

static void SetTitleLabelStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void TagsRender(cJSON *root, int size, lv_obj_t *parent)
{
    int height = 62 + 84 * size;
    for (int i = 0; i < size; i++) {
        cJSON *item = cJSON_GetArrayItem(root, i);
        cJSON *key = cJSON_GetObjectItem(item, "name");
        cJSON *value = cJSON_GetObjectItem(item, "value");
        if (key == NULL || value == NULL) {
            continue;
        }
        lv_obj_t *keyLabel = lv_label_create(parent);
        lv_label_set_text(keyLabel, "Name");
        SetTitleLabelStyle(keyLabel);
        lv_obj_align(keyLabel, LV_ALIGN_TOP_LEFT, 24, 62 + 84 * i);

        lv_obj_t *keyValueLabel = lv_label_create(parent);
        lv_label_set_text(keyValueLabel, key->valuestring);
        lv_obj_set_style_text_font(keyValueLabel, g_defIllustrateFont, LV_PART_MAIN);
        lv_obj_set_style_text_color(keyValueLabel, lv_color_hex(16090890), LV_PART_MAIN);
        lv_obj_align(keyValueLabel, LV_ALIGN_TOP_LEFT, 96, 62 + 84 * i);

        lv_obj_t *valueLabel = lv_label_create(parent);
        lv_label_set_text(valueLabel, "Value");
        SetTitleLabelStyle(valueLabel);
        lv_obj_align(valueLabel, LV_ALIGN_TOP_LEFT, 24, 62 + 84 * i + 38);

        lv_obj_t *valueValueLabel = lv_label_create(parent);
        lv_label_set_text(valueValueLabel, value->valuestring);
        lv_obj_set_style_text_font(valueValueLabel, g_defIllustrateFont, LV_PART_MAIN);
        lv_obj_set_style_text_color(valueValueLabel, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_align(valueValueLabel, LV_ALIGN_TOP_LEFT, 96, 62 + 84 * i + 38);
    }
    lv_obj_set_size(parent, 408, height);
}

bool IsArweaveSetupComplete(void)
{
    char *xPub = GetCurrentAccountPublicKey(XPUB_TYPE_ARWEAVE);
    return xPub != NULL && strlen(xPub) == 1024;
}

PtrT_TransactionCheckResult GuiGetArCheckResult(void)
{
#ifndef COMPILE_SIMULATOR
    if (!IsArweaveSetupComplete()) {
        ThrowError(ERR_INVALID_QRCODE);
    }
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return ar_check_tx(data, mfp, sizeof(mfp));
#else
    return NULL;
#endif
}

void GetArweaveMessageText(void *indata, void *param, uint32_t maxLen)
{
    DisplayArweaveMessage *data = (DisplayArweaveMessage *)param;
    if (data->message == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, data->message);
}

void GetArweaveRawMessage(void *indata, void *param, uint32_t maxLen)
{
    DisplayArweaveMessage *data = (DisplayArweaveMessage *)param;
    if (data->raw_message == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, data->raw_message);
}

void GetArweaveMessageAddress(void *indata, void *param, uint32_t maxLen)
{
    char *xPub = GetCurrentAccountPublicKey(XPUB_TYPE_ARWEAVE);
    SimpleResponse_c_char *result = arweave_get_address(xPub);
    if (result->error_code == 0) {
        SimpleResponse_c_char *fixedAddress = fix_arweave_address(result->data);
        if (fixedAddress->error_code == 0) {
            strcpy_s((char *)indata, maxLen, fixedAddress->data);
        }
        free_simple_response_c_char(fixedAddress);
    }
    free_simple_response_c_char(result);
}

void GetArweaveValue(void *indata, void *param, uint32_t maxLen)
{
    DisplayArweaveTx *tx = (DisplayArweaveTx *)param;
    if (tx->value == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, tx->value);
}

void GetArweaveFee(void *indata, void *param, uint32_t maxLen)
{
    DisplayArweaveTx *tx = (DisplayArweaveTx *)param;
    if (tx->fee == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, tx->fee);
}

void GetArweaveFromAddress(void *indata, void *param, uint32_t maxLen)
{
    DisplayArweaveTx *tx = (DisplayArweaveTx *)param;
    if (tx->from == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, tx->from);
}

void GetArweaveToAddress(void *indata, void *param, uint32_t maxLen)
{
    DisplayArweaveTx *tx = (DisplayArweaveTx *)param;
    if (tx->to == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, tx->to);
}

void GuiSetArUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
    ParseRequestType();
}

void *GuiGetArData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    do {
        if (g_requestType == ArweaveRequestTypeUnknown) {
            printf("Unknown request type\n");
            break;
        }
        if (g_requestType == ArweaveRequestTypeTransaction) {
            PtrT_TransactionParseResult_DisplayArweaveTx parseResult = ar_parse(data);
            CHECK_CHAIN_BREAK(parseResult);
            g_parseResult = (void *)parseResult;
        } else if (g_requestType == ArweaveRequestTypeMessage) {
            PtrT_TransactionParseResult_DisplayArweaveMessage parseResult = ar_message_parse(data);
            CHECK_CHAIN_BREAK(parseResult);
            g_parseResult = (void *)parseResult;
        }
    } while (0);
    return g_parseResult;
}

void FreeArMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
}

void GuiShowArweaveTxDetail(lv_obj_t *parent, void *totalData)
{
    cJSON *root;
    int size;
    bool shouldShowContainer = true;
    DisplayArweaveTx *txData = (DisplayArweaveTx *)totalData;
    PtrString txDetail = txData->detail;
    if (txDetail == NULL) {
        shouldShowContainer = false;
    } else {
        root = cJSON_Parse((const char *)txDetail);
        size = cJSON_GetArraySize(root);
        if (size <= 0) {
            shouldShowContainer = false;
        }
    }

    if (!shouldShowContainer) {
        lv_obj_add_flag(parent, LV_OBJ_FLAG_HIDDEN);
        return;
    } else {
        lv_obj_clear_flag(parent, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(parent, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(parent, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(parent, 31, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "#2");
    lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(16090890), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);

    TagsRender(root, size, parent);
}

UREncodeResult *GuiGetArweaveSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    UREncodeResult *encodeResult = NULL;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        Rsa_primes_t *primes = FlashReadRsaPrimes();
        if (primes == NULL) {
            encodeResult = NULL;
            break;
        }
        encodeResult = ar_sign_tx(data, primes->p, 256, primes->q, 256);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
#else
    UREncodeResult *encodeResult = NULL;
    encodeResult->is_multi_part = 0;
    encodeResult->data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    encodeResult->encoder = NULL;
    encodeResult->error_code = 0;
    encodeResult->error_message = NULL;
    return encodeResult;
#endif
}

#endif