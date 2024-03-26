#include <stdio.h>
#include <string.h>
#include "gui_analyze.h"
#include "user_memory.h"
#include "rust.h"
#include "account_public_info.h"
#include "keystore.h"
#include "version.h"
#include "secret_cache.h"
#include "screen_manager.h"
#include "account_manager.h"

#define CHECK_FREE_PARSE_RESULT(result)                             \
    if (result != NULL) {                                           \
        free_TransactionParseResult_DisplayTx(g_parseResult);       \
        g_parseResult = NULL;                                       \
    }

#ifndef COMPILE_SIMULATOR
static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static TransactionParseResult_DisplayTx *g_parseResult = NULL;
#endif

#ifndef BTC_ONLY
typedef struct UtxoViewToChain {
    ViewType viewType;
    ChainType chainType;
    char *hdPath;
} UtxoViewToChain_t;

static UtxoViewToChain_t g_UtxoViewToChainMap[] = {
    {BtcNativeSegwitTx, XPUB_TYPE_BTC_NATIVE_SEGWIT, "m/84'/0'/0'"},
    {BtcSegwitTx,       XPUB_TYPE_BTC,               "m/49'/0'/0'"},
    {BtcLegacyTx,       XPUB_TYPE_BTC_LEGACY,        "m/44'/0'/0'"},
    {LtcTx,             XPUB_TYPE_LTC,               "m/49'/2'/0'"},
    {DashTx,            XPUB_TYPE_DASH,              "m/44'/5'/0'"},
    {BchTx,             XPUB_TYPE_BCH,               "m/44'/145'/0'"},
};
#endif

void GuiSetPsbtUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
#ifndef COMPILE_SIMULATOR
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
#endif
}

#ifndef BTC_ONLY
static int32_t GuiGetUtxoPubKeyAndHdPath(ViewType viewType, char **xPub, char **hdPath)
{
    int32_t ret = 0;
    uint32_t i = 0;
    for (i = 0; i < NUMBER_OF_ARRAYS(g_UtxoViewToChainMap); i++) {
        if (viewType == g_UtxoViewToChainMap[i].viewType) {
            *hdPath = g_UtxoViewToChainMap[i].hdPath;
            *xPub = GetCurrentAccountPublicKey(g_UtxoViewToChainMap[i].chainType);
            break;
        }
    }
    if (i == NUMBER_OF_ARRAYS(g_UtxoViewToChainMap)) {
        *xPub = NULL;
    }
    if (*xPub == NULL) {
        printf("error When GuiGetSignQrCodeData. xPub is NULL, viewType = %d\r\n", viewType);
        ret = -1;
    }
    return ret;
}
#endif

// The results here are released in the close qr timer species
UREncodeResult *GuiGetSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    enum URType urType = URTypeUnKnown;
#ifndef BTC_ONLY
    enum ViewType viewType = ViewTypeUnKnown;
#endif
    void *data = NULL;
    if (g_isMulti) {
        urType = g_urMultiResult->ur_type;
#ifndef BTC_ONLY
        viewType = g_urMultiResult->t;
#endif
        data = g_urMultiResult->data;
    } else {
        urType = g_urResult->ur_type;
#ifndef BTC_ONLY
        viewType = g_urResult->t;
#endif
        data = g_urResult->data;
    }
    UREncodeResult *encodeResult = NULL;
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    if (urType == CryptoPSBT) {
        uint8_t mfp[4] = {0};
        GetMasterFingerPrint(mfp);
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = btc_sign_psbt(data, seed, len, mfp, sizeof(mfp));
    }
#ifndef BTC_ONLY
    else if (urType == Bytes || urType == KeystoneSignRequest) {
        char *hdPath = NULL;
        char *xPub = NULL;
        if (0 != GuiGetUtxoPubKeyAndHdPath(viewType, &xPub, &hdPath)) {
            return NULL;
        }
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = utxo_sign_keystone(data, urType, mfp, sizeof(mfp), xPub, SOFTWARE_VERSION, seed, len);
    }
#endif
    CHECK_CHAIN_PRINT(encodeResult);
    ClearSecretCache();
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

void *GuiGetParsedQrData(void)
{
#ifndef COMPILE_SIMULATOR
    enum URType urType = URTypeUnKnown;
#ifndef BTC_ONLY
    enum ViewType viewType = ViewTypeUnKnown;
#endif
    void *crypto = NULL;
    if (g_isMulti) {
        crypto = g_urMultiResult->data;
        urType = g_urMultiResult->ur_type;
#ifndef BTC_ONLY
        viewType = g_urMultiResult->t;
#endif
    } else {
        crypto = g_urResult->data;
        urType = g_urResult->ur_type;
#ifndef BTC_ONLY
        viewType = g_urResult->t;
#endif
    }
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    do {
        if (urType == CryptoPSBT) {
            PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
#ifdef BTC_ONLY
            ExtendedPublicKey keys[8];
            public_keys->data = keys;
            public_keys->size = 8;
            keys[0].path = "m/84'/0'/0'";
            keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
            keys[1].path = "m/49'/0'/0'";
            keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
            keys[2].path = "m/44'/0'/0'";
            keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);
            keys[3].path = "m/86'/0'/0'";
            keys[3].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TAPROOT);
            keys[4].path = "m/84'/1'/0'";
            keys[4].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT_TEST);
            keys[5].path = "m/49'/1'/0'";
            keys[5].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TEST);
            keys[6].path = "m/44'/1'/0'";
            keys[6].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY_TEST);
            keys[7].path = "m/86'/1'/0'";
            keys[7].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TAPROOT_TEST);
#else
            ExtendedPublicKey keys[4];
            public_keys->data = keys;
            public_keys->size = 4;
            keys[0].path = "m/84'/0'/0'";
            keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
            keys[1].path = "m/49'/0'/0'";
            keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
            keys[2].path = "m/44'/0'/0'";
            keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);
            keys[3].path = "m/86'/0'/0'";
            keys[3].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TAPROOT);
#endif
            g_parseResult = btc_parse_psbt(crypto, mfp, sizeof(mfp), public_keys);
            CHECK_CHAIN_RETURN(g_parseResult);
            SRAM_FREE(public_keys);
            return g_parseResult;
        }
#ifndef BTC_ONLY
        else if (urType == Bytes || urType == KeystoneSignRequest) {
            char *hdPath = NULL;
            char *xPub = NULL;
            if (0 != GuiGetUtxoPubKeyAndHdPath(viewType, &xPub, &hdPath)) {
                return NULL;
            }
            g_parseResult = utxo_parse_keystone(crypto, urType, mfp, sizeof(mfp), xPub);
            CHECK_CHAIN_RETURN(g_parseResult);
            return g_parseResult;
        }
#endif
    } while (0);
#else
    TransactionParseResult_DisplayTx parseResult;
    TransactionParseResult_DisplayTx *g_parseResult = &parseResult;
    g_parseResult->data = SRAM_MALLOC(sizeof(DisplayTx));
    g_parseResult->error_code = 0;
    g_parseResult->error_message = NULL;
#endif
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetPsbtCheckResult(void)
{
#ifndef COMPILE_SIMULATOR
    PtrT_TransactionCheckResult result = NULL;
    enum URType urType = URTypeUnKnown;
#ifndef BTC_ONLY
    enum ViewType viewType = ViewTypeUnKnown;
#endif
    void *crypto = NULL;
    if (g_isMulti) {
        crypto = g_urMultiResult->data;
        urType = g_urMultiResult->ur_type;
#ifndef BTC_ONLY
        viewType = g_urMultiResult->t;
#endif
    } else {
        crypto = g_urResult->data;
        urType = g_urResult->ur_type;
#ifndef BTC_ONLY
        viewType = g_urResult->t;
#endif
    }
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    if (urType == CryptoPSBT) {
        PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
#ifdef BTC_ONLY
        ExtendedPublicKey keys[8];
        public_keys->data = keys;
        public_keys->size = 8;
        keys[0].path = "m/84'/0'/0'";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
        keys[1].path = "m/49'/0'/0'";
        keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
        keys[2].path = "m/44'/0'/0'";
        keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);
        keys[3].path = "m/86'/0'/0'";
        keys[3].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TAPROOT);
        keys[4].path = "m/84'/1'/0'";
        keys[4].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT_TEST);
        keys[5].path = "m/49'/1'/0'";
        keys[5].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TEST);
        keys[6].path = "m/44'/1'/0'";
        keys[6].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY_TEST);
        keys[7].path = "m/86'/1'/0'";
        keys[7].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TAPROOT_TEST);
#else
        ExtendedPublicKey keys[4];
        public_keys->data = keys;
        public_keys->size = 4;
        keys[0].path = "m/84'/0'/0'";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
        keys[1].path = "m/49'/0'/0'";
        keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
        keys[2].path = "m/44'/0'/0'";
        keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);
        keys[3].path = "m/86'/0'/0'";
        keys[3].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TAPROOT);
#endif
        result = btc_check_psbt(crypto, mfp, sizeof(mfp), public_keys);
        SRAM_FREE(public_keys);
    }
#ifndef BTC_ONLY
    else if (urType == Bytes || urType == KeystoneSignRequest) {
        char *hdPath = NULL;
        char *xPub = NULL;
        if (0 != GuiGetUtxoPubKeyAndHdPath(viewType, &xPub, &hdPath)) {
            return NULL;
        }
        result = utxo_check_keystone(crypto, urType, mfp, sizeof(mfp), xPub);
    }
#endif
    return result;
#else
    return NULL;
#endif
}

void GetPsbtTotalOutAmount(void *indata, void *param, uint32_t maxLen)
{
    DisplayTx *psbt = (DisplayTx *)param;
    strcpy_s((char *)indata, maxLen, psbt->overview->total_output_amount);
}

void GetPsbtFeeAmount(void *indata, void *param, uint32_t maxLen)
{
    DisplayTx *psbt = (DisplayTx *)param;
    if (psbt->overview->fee_larger_than_amount) {
        snprintf_s((char *)indata,  maxLen, "#F55831 %s#", psbt->overview->fee_amount);
    } else {
        strcpy_s((char *)indata, maxLen, psbt->overview->fee_amount);
    }
}

void GetPsbtTotalOutSat(void *indata, void *param, uint32_t maxLen)
{
    DisplayTx *psbt = (DisplayTx *)param;
    strcpy_s((char *)indata, maxLen, psbt->overview->total_output_sat);
}

void GetPsbtFeeSat(void *indata, void *param, uint32_t maxLen)
{
    DisplayTx *psbt = (DisplayTx *)param;
    if (psbt->overview->fee_larger_than_amount) {
        snprintf_s((char *)indata,  maxLen, "#F55831 %s#", psbt->overview->fee_sat);
    } else {
        strcpy_s((char *)indata, maxLen, psbt->overview->fee_sat);
    }
}

void GetPsbtNetWork(void *indata, void *param, uint32_t maxLen)
{
    DisplayTx *psbt = (DisplayTx *)param;
    strcpy_s((char *)indata, maxLen, psbt->overview->network);
}

void GetPsbtDetailInputValue(void *indata, void *param, uint32_t maxLen)
{
    DisplayTx *psbt = (DisplayTx *)param;
    strcpy_s((char *)indata, maxLen, psbt->detail->total_input_amount);
}

void GetPsbtDetailOutputValue(void *indata, void *param, uint32_t maxLen)
{
    DisplayTx *psbt = (DisplayTx *)param;
    strcpy_s((char *)indata, maxLen, psbt->detail->total_output_amount);
}

void GetPsbtDetailFee(void *indata, void *param, uint32_t maxLen)
{
    DisplayTx *psbt = (DisplayTx *)param;
    strcpy_s((char *)indata, maxLen, psbt->detail->fee_amount);
}

void *GetPsbtInputData(uint8_t *row, uint8_t *col, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    *col = 2;
    *row = psbt->overview->from->size;
    int i = 0, j = 0;
    char ***indata = (char ***)SRAM_MALLOC(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        indata[i] = SRAM_MALLOC(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            indata[i][j] = SRAM_MALLOC(BUFFER_SIZE_64);
            if (i == 0) {
                snprintf_s(indata[i][j], BUFFER_SIZE_64, "%d\n", j + 1);
            } else {
                strcpy_s(indata[i][j], BUFFER_SIZE_64, psbt->overview->from->data[j].address);
            }
        }
    }
    return (void *)indata;
}

void *GetPsbtOutputData(uint8_t *row, uint8_t *col, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    *col = 2;
    *row = psbt->overview->to->size;
    int i = 0, j = 0;
    char ***indata = (char ***)SRAM_MALLOC(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        indata[i] = SRAM_MALLOC(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            indata[i][j] = SRAM_MALLOC(BUFFER_SIZE_64);
            if (i == 0) {
                snprintf_s(indata[i][j], BUFFER_SIZE_64, "%d\n", j + 1);
            } else {
                strcpy_s(indata[i][j], BUFFER_SIZE_64, psbt->overview->to->data[j].address);
            }
        }
    }
    return (void *)indata;
}

void *GetPsbtInputDetailData(uint8_t *row, uint8_t *col, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    *col = 2;
    *row = psbt->detail->from->size;
    int i = 0, j = 0;
    char ***indata = (char ***)SRAM_MALLOC(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        indata[i] = SRAM_MALLOC(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            indata[i][j] = SRAM_MALLOC(BUFFER_SIZE_64);
            if (i == 0) {
                snprintf_s(indata[i][j], BUFFER_SIZE_64, "%d\n", j + 1);
            } else {
                strcpy_s(indata[i][j], BUFFER_SIZE_64, psbt->detail->from->data[j].address);
            }
        }
    }
    return (void *)indata;
}

void *GetPsbtOutputDetailData(uint8_t *row, uint8_t *col, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    *col = 2;
    *row = psbt->detail->to->size;
    int i = 0, j = 0;
    char ***indata = (char ***)SRAM_MALLOC(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        indata[i] = SRAM_MALLOC(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            indata[i][j] = SRAM_MALLOC(BUFFER_SIZE_64);
            if (i == 0) {
                snprintf_s(indata[i][j], BUFFER_SIZE_64, "%d\n", j + 1);
            } else {
                strcpy_s(indata[i][j], BUFFER_SIZE_64, psbt->detail->to->data[j].address);
            }
        }
    }
    return (void *)indata;
}

void GetPsbtOverviewSize(uint16_t *width, uint16_t *height, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    *width = 408;
    *height = 16 + 30 + 8 + (psbt->overview->from->size - 1) * 8 + psbt->overview->from->size * 60 +
              16 + (psbt->overview->from->size - 1) * 8 + 30 + psbt->overview->to->size * 60 + 16;
}

void GetPsbtDetailSize(uint16_t *width, uint16_t *height, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    *width = 408;
    *height = 16 + 30 + 8 + (psbt->detail->from->size - 1) * 8 + psbt->detail->from->size * 60 +
              16 + (psbt->detail->from->size - 1) * 8 + 30 + psbt->detail->to->size * 60 + 16;
}

void FreePsbtUxtoMemory(void)
{
#ifndef COMPILE_SIMULATOR
    printf("free g_urResult: %p\r\n", g_urResult);
    CHECK_FREE_UR_RESULT(g_urResult, false);
    printf("free g_urMultiResult: %p\r\n", g_urMultiResult);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
#endif
}