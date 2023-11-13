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
        free_transaction_parse_result_display_tx(g_parseResult);    \
        g_parseResult = NULL;                                       \
    }

#ifndef COMPILE_SIMULATOR
static bool g_isMulti = false;
static void *g_urResult = NULL;
static TransactionParseResult_DisplayTx *g_parseResult = NULL;
#endif

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

void GuiSetPsbtUrData(void *data, bool multi)
{
#ifndef COMPILE_SIMULATOR
    g_urResult = data;
    g_isMulti = multi;
#endif
}

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

// The results here are released in the close qr timer species
UREncodeResult *GuiGetSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    enum URType urType = URTypeUnKnown;
    enum ViewType viewType = ViewTypeUnKnown;
    void *data = NULL;
    if (g_isMulti) {
        URParseMultiResult *urResult = (URParseMultiResult *)g_urResult;
        urType = urResult->ur_type;
        viewType = urResult->t;
        data = urResult->data;
    } else {
        URParseResult *urResult = (URParseResult *)g_urResult;
        urType = urResult->ur_type;
        viewType = urResult->t;
        data = urResult->data;
    }
    UREncodeResult *encodeResult = NULL;
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    if (urType == CryptoPSBT) {
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = btc_sign_psbt(data, seed, len);
    } else if (urType == Bytes) {
        char *hdPath = NULL;
        char *xPub = NULL;
        if (0 != GuiGetUtxoPubKeyAndHdPath(viewType, &xPub, &hdPath)) {
            return NULL;
        }
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = utxo_sign_companion_app(data, urType, mfp, sizeof(mfp), xPub, SOFTWARE_VERSION, seed, len);
    }
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
    enum ViewType viewType = ViewTypeUnKnown;
    void *crypto = NULL;
    if (g_isMulti) {
        URParseMultiResult *urResult = (URParseMultiResult *)g_urResult;
        crypto = urResult->data;
        urType = urResult->ur_type;
        viewType = urResult->t;
    } else {
        URParseResult *urResult = (URParseResult *)g_urResult;
        crypto = urResult->data;
        urType = urResult->ur_type;
        viewType = urResult->t;
    }
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    do {
        if (urType == CryptoPSBT) {
            PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
            ExtendedPublicKey keys[3];
            public_keys->data = keys;
            public_keys->size = 3;
            keys[0].path = "m/84'/0'/0'";
            keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
            keys[1].path = "m/49'/0'/0'";
            keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
            keys[2].path = "m/44'/0'/0'";
            keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);
            g_parseResult = btc_parse_psbt(crypto, mfp, sizeof(mfp), public_keys);
            CHECK_CHAIN_RETURN(g_parseResult);
            SRAM_FREE(public_keys);
            return g_parseResult;
        } else if (urType == Bytes) {
            char *hdPath = NULL;
            char *xPub = NULL;
            if (0 != GuiGetUtxoPubKeyAndHdPath(viewType, &xPub, &hdPath)) {
                return NULL;
            }
            g_parseResult = utxo_parse_companion_app(crypto, urType, mfp, sizeof(mfp), xPub);
            CHECK_CHAIN_RETURN(g_parseResult);
            return g_parseResult;
        }
    } while (0);
#else
    TransactionParseResult_DisplayTx parseResult;
    TransactionParseResult_DisplayTx *g_parseResult = &parseResult;
    g_parseResult->data = malloc(sizeof(DisplayTx));
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
    enum ViewType viewType = ViewTypeUnKnown;
    void *crypto = NULL;
    if (g_isMulti) {
        URParseMultiResult *urResult = (URParseMultiResult *)g_urResult;
        crypto = urResult->data;
        urType = urResult->ur_type;
        viewType = urResult->t;
    } else {
        URParseResult *urResult = (URParseResult *)g_urResult;
        crypto = urResult->data;
        urType = urResult->ur_type;
        viewType = urResult->t;
    }
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    if (urType == CryptoPSBT) {
        PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
        ExtendedPublicKey keys[3];
        public_keys->data = keys;
        public_keys->size = 3;
        keys[0].path = "m/84'/0'/0'";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
        keys[1].path = "m/49'/0'/0'";
        keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
        keys[2].path = "m/44'/0'/0'";
        keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);
        result = btc_check_psbt(crypto, mfp, sizeof(mfp), public_keys);
        SRAM_FREE(public_keys);
    } else if (urType == Bytes) {
        char *hdPath = NULL;
        char *xPub = NULL;
        if (0 != GuiGetUtxoPubKeyAndHdPath(viewType, &xPub, &hdPath)) {
            return NULL;
        }
        result = utxo_check_companion_app(crypto, urType, mfp, sizeof(mfp), xPub);
    }
    return result;
#else
    return NULL;
 #endif   
}

void GetPsbtFeeValue(void *indata, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    strcpy((char *)indata, psbt->overview->fee_amount);
}

void GetPsbtTotalOutAmount(void *indata, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    strcpy((char *)indata, psbt->overview->total_output_amount);
}

void GetPsbtFeeAmount(void *indata, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    if (psbt->overview->fee_larger_than_amount) {
        sprintf((char *)indata, "#F55831 %s#", psbt->overview->fee_amount);
    } else {
        strcpy((char *)indata, psbt->overview->fee_amount);
    }
}

void GetPsbtTotalOutSat(void *indata, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    strcpy((char *)indata, psbt->overview->total_output_sat);
}

void GetPsbtFeeSat(void *indata, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    if (psbt->overview->fee_larger_than_amount) {
        sprintf((char *)indata, "#F55831 %s#", psbt->overview->fee_sat);
    } else {
        strcpy((char *)indata, psbt->overview->fee_sat);
    }
}

void GetPsbtNetWork(void *indata, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    strcpy((char *)indata, psbt->overview->network);
}

void GetPsbtDetailInputValue(void *indata, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    strcpy((char *)indata, psbt->detail->total_input_amount);
}

void GetPsbtDetailOutputValue(void *indata, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    strcpy((char *)indata, psbt->detail->total_output_amount);
}

void GetPsbtDetailFee(void *indata, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    strcpy((char *)indata, psbt->detail->fee_amount);
}

void *GetPsbtInputData(uint8_t *row, uint8_t *col, void *param)
{
    DisplayTx *psbt = (DisplayTx *)param;
    *col = 2;
    *row = psbt->overview->from->size;
    int i = 0, j = 0;
    char ***indata = (char ***)malloc(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        indata[i] = malloc(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            indata[i][j] = malloc(64);
            if (i == 0) {
                sprintf(indata[i][j], "%d\n", j + 1);
            } else {
                strcpy(indata[i][j], psbt->overview->from->data[j].address);
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
    char ***indata = (char ***)malloc(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        indata[i] = malloc(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            indata[i][j] = malloc(64);
            if (i == 0) {
                sprintf(indata[i][j], "%d\n", j + 1);
            } else {
                strcpy(indata[i][j], psbt->overview->to->data[j].address);
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
    char ***indata = (char ***)malloc(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        indata[i] = malloc(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            indata[i][j] = malloc(64);
            if (i == 0) {
                sprintf(indata[i][j], "%d\n", j + 1);
            } else {
                // sprintf(indata[i][j], "%d\n", j + 1);
                strcpy(indata[i][j], psbt->detail->from->data[j].address);
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
            indata[i][j] = SRAM_MALLOC(64);
            if (i == 0) {
                sprintf(indata[i][j], "%d\n", j + 1);
            } else {
                strcpy(indata[i][j], psbt->detail->to->data[j].address);
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
    CHECK_FREE_UR_RESULT(g_urResult, g_isMulti);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
#endif
}