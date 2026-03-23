#include "gui_analyze.h"
#include "rust.h"
#include "account_public_info.h"
#include "keystore.h"
#include "gui_chain.h"
#include "version.h"
#include "secret_cache.h"
#include "screen_manager.h"
#include "account_manager.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;

void GuiSetTrxUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

#define CHECK_FREE_PARSE_RESULT(result)                                                              \
    if (result != NULL)                                                                              \
    {                                                                                                \
        free_TransactionParseResult_DisplayTron((PtrT_TransactionParseResult_DisplayTron)result);    \
        result = NULL;                                                                               \
    }

void *GuiGetTrxData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    QRCodeType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
    char *trxXpub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);
    GetMasterFingerPrint(mfp);
    do {
        PtrT_TransactionParseResult_DisplayTron parseResult = tron_parse_keystone(data, urType, mfp, sizeof(mfp), trxXpub);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetTrxCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    QRCodeType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
    char *trxXpub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);
    GetMasterFingerPrint(mfp);
    return tron_check_keystone(data, urType, mfp, sizeof(mfp), trxXpub);
}
void FreeTrxMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
}

void GetTrxValue(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata, maxLen, trx->detail->value);
}

void GetTrxMethod(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata, maxLen, trx->detail->method);
}

void GetTrxFromAddress(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata, maxLen, trx->detail->from);
}

void GetTrxToAddress(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata, maxLen, trx->detail->to);
}

bool GetTrxContractExist(void *indata, void *param)
{
    DisplayTron *trx = (DisplayTron *)param;
    return strlen(trx->detail->contract_address) > 0;
}

void GetTrxContract(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata, maxLen, trx->detail->contract_address);
}

bool GetTrxTokenExist(void *indata, void *param)
{
    DisplayTron *trx = (DisplayTron *)param;
    return strlen(trx->detail->token) > 0 && strcmp(trx->detail->token, "TRX") != 0;
}

void GetTrxToken(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata,  maxLen, trx->detail->token);
}

void GetTrxSwapDstAsset(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    const char *memo = trx->detail->memo;
    char *out = (char *)indata;

    if (memo == NULL || strlen(memo) == 0) {
        strcpy_s(out, maxLen, "Unknown");
        return;
    }

    const char *first_colon = strchr(memo, ':');
    if (!first_colon) {
        strcpy_s(out, maxLen, "");
        return;
    }

    const char *start = first_colon + 1;
    const char *second_colon = strchr(start, ':');

    if (second_colon) {
        size_t len = second_colon - start;
        if (len >= maxLen) len = maxLen - 1;
        strncpy_s(out, maxLen, start, len);
    } else {
        strcpy_s(out, maxLen, start);
    }
}

void GetTrxSwapDstAddress(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    const char *memo = trx->detail->memo;
    char *out = (char *)indata;

    const char *first_colon = strchr(memo, ':');
    if (first_colon) {
        const char *second_colon = strchr(first_colon + 1, ':');
        if (second_colon) {
            const char *start = second_colon + 1;
            const char *third_colon = strchr(start, ':');
            
            if (third_colon) {
                size_t len = third_colon - start;
                if (len >= maxLen) len = maxLen - 1;
                strncpy_s(out, maxLen, start, len);
                return;
            } else {
                strcpy_s(out, maxLen, start);
                return;
            }
        }
    }
    strcpy_s(out, maxLen, "");
}

void GetTrxMemo(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata, maxLen, trx->detail->memo);
}

void GetTrxNetwork(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata, maxLen, trx->detail->network);
}

void GetTrxExpiration(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata, maxLen, trx->detail->expiration);
}

void GetTrxValueRaw(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata, maxLen, trx->detail->raw_value);
}

void TrxCheckVault(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t *v_btn = lv_event_get_current_target(e); 
        
        lv_obj_t *address_card = lv_obj_get_parent(v_btn);
        if (!address_card) return;

        lv_obj_t *t_val_obj = lv_obj_get_child(address_card, 3);
        
        if (t_val_obj) {
            const char *address = lv_label_get_text(t_val_obj);
            if (address && strlen(address) > 10) {
                char url[256] = {0};
                snprintf(url, sizeof(url), "https://tronscan.org/#/address/%s", address);
                GuiQRCodeHintBoxOpen(url, _("SwapKit Tron Vault"), url);
            }
        }
    }
}


UREncodeResult *GuiGetTrxSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult = NULL;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    QRCodeType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    uint8_t seed[SEED_LEN];
    do {
        int ret = GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        if (ret != 0) {
            break;
        }
        encodeResult = tron_sign_keystone(data, urType, mfp, sizeof(mfp), GetCurrentAccountPublicKey(XPUB_TYPE_TRX),
                                          SOFTWARE_VERSION, seed, GetCurrentAccountSeedLen());
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    memset_s(seed, sizeof(seed), 0, sizeof(seed));
    ClearSecretCache();
    SetLockScreen(enable);
    return encodeResult;
}