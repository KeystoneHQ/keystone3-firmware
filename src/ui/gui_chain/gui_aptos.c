#include "rust.h"
#include "keystore.h"
#include "user_memory.h"
#include "gui_chain.h"
#include "screen_manager.h"
#include "keystore.h"
#include "account_manager.h"
#include "secret_cache.h"
#include "assert.h"

static uint8_t GetAptosPublickeyIndex(char* rootPath);

static bool g_isMulti = false;
static void *g_urResult = NULL;
static void *g_parseResult = NULL;

void GuiSetAptosUrData(void *data, bool multi)
{
#ifndef COMPILE_SIMULATOR
    g_urResult = data;
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
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
    GetMasterFingerPrint(mfp);
    do {
        PtrT_TransactionParseResult_DisplayAptosTx parseResult = aptos_parse(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
#else
    return NULL;
#endif
}

PtrT_TransactionCheckResult GuiGetAptosCheckResult(void)
{
#ifndef COMPILE_SIMULATOR
    uint8_t mfp[4];
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
    GetMasterFingerPrint(mfp);
    return aptos_check_request(data, mfp, sizeof(mfp));
#else
    return NULL;
#endif
}

void FreeAptosMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, g_isMulti);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
#endif
}

int GetAptosDetailLen(void *param)
{
    DisplayAptosTx *tx = (DisplayAptosTx *)param;
    return strlen(tx->detail);
}

void GetAptosDetail(void *indata, void *param)
{
    DisplayAptosTx *tx = (DisplayAptosTx *)param;
    sprintf((char *)indata, "%s", tx->detail);
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
#ifndef COMPILE_SIMULATOR
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
    do {
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        char *path = aptos_get_path(data);
        char pubkeyIndex = GetAptosPublickeyIndex(path);
        char *pubKey = GetCurrentAccountPublicKey(pubkeyIndex);
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        encodeResult = aptos_sign_tx(data, seed, len, pubKey);
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
}
