#include "define.h"
#include "rust.h"
#include "keystore.h"
#include "gui_chain.h"
#include "screen_manager.h"
#include "keystore.h"
#include "account_manager.h"
#include "secret_cache.h"
#include "assert.h"
#include "user_memory.h"

static uint8_t GetAptosPublickeyIndex(char* rootPath);

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;

void GuiSetAptosUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
#ifndef COMPILE_SIMULATOR
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
#endif
}

#define CHECK_FREE_PARSE_RESULT(result)                                                                 \
    if (result != NULL)                                                                                 \
    {                                                                                                   \
        free_TransactionParseResult_DisplayAptosTx((PtrT_TransactionParseResult_DisplayAptosTx)result); \
        result = NULL;                                                                                  \
    }

void *GuiGetAptosData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    do {
        PtrT_TransactionParseResult_DisplayAptosTx parseResult = aptos_parse(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetAptosCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return aptos_check_request(data, mfp, sizeof(mfp));
}

void FreeAptosMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
}

int GetAptosDetailLen(void *param)
{
    DisplayAptosTx *tx = (DisplayAptosTx *)param;
    return strlen(tx->detail) + 1;
}

void GetAptosDetail(void *indata, void *param, uint32_t maxLen)
{
    DisplayAptosTx *tx = (DisplayAptosTx *)param;
    strcpy_s((char *)indata, maxLen, tx->detail);
}

bool IsAptosMsg(ViewType viewType)
{
    if (viewType != AptosTx) {
        return false;
    }
    DisplayAptosTx *tx = ((PtrT_TransactionParseResult_DisplayAptosTx)g_parseResult)->data;
    return tx->is_msg;
}

UREncodeResult *GuiGetAptosSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult = NULL;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t seed[SEED_LEN];
    int ret = 0;

    do {
        ret = GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        if (ret != SUCCESS_CODE) {
            break;
        }
        char *path = aptos_get_path(data);
        char *pubKey = GetCurrentAccountPublicKey(GetAptosPublickeyIndex(path));
        free_ptr_string(path);
        encodeResult = aptos_sign_tx(data, seed, GetCurrentAccountSeedLen(), pubKey);
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    memset_s(seed, sizeof(seed), 0, sizeof(seed));
    ClearSecretCache();
    SetLockScreen(enable);
    return encodeResult;
}

static uint8_t GetAptosPublickeyIndex(char* rootPath)
{
    if (strcmp(rootPath, "44'/637'/0'/0'/0'") == 0) return XPUB_TYPE_APT_0;
    if (strcmp(rootPath, "44'/637'/1'/0'/0'") == 0) return XPUB_TYPE_APT_1;
    if (strcmp(rootPath, "44'/637'/2'/0'/0'") == 0) return XPUB_TYPE_APT_2;
    if (strcmp(rootPath, "44'/637'/3'/0'/0'") == 0) return XPUB_TYPE_APT_3;
    if (strcmp(rootPath, "44'/637'/4'/0'/0'") == 0) return XPUB_TYPE_APT_4;
    if (strcmp(rootPath, "44'/637'/5'/0'/0'") == 0) return XPUB_TYPE_APT_5;
    if (strcmp(rootPath, "44'/637'/6'/0'/0'") == 0) return XPUB_TYPE_APT_6;
    if (strcmp(rootPath, "44'/637'/7'/0'/0'") == 0) return XPUB_TYPE_APT_7;
    if (strcmp(rootPath, "44'/637'/8'/0'/0'") == 0) return XPUB_TYPE_APT_8;
    if (strcmp(rootPath, "44'/637'/9'/0'/0'") == 0) return XPUB_TYPE_APT_9;
    ASSERT(0);
    return -1;
}
