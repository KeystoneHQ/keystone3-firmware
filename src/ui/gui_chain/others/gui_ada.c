#ifndef BTC_ONLY
#include "rust.h"
#include "keystore.h"
#include "gui_chain.h"
#include "screen_manager.h"
#include "keystore.h"
#include "account_manager.h"
#include "secret_cache.h"
#include "assert.h"
#include "gui_ada.h"
#include "gui_hintbox.h"
#include "gui.h"
#include "user_memory.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif

#define ADA_ADD_MAX_LEN             (150)

static bool g_isMulti = false;
static struct URParseResult *g_urResult = NULL;
static struct URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static char g_adaBaseAddr[ADA_ADD_MAX_LEN];
static char *xpub = NULL;

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
        uint8_t xpubIndex = GetXPubIndexByPath(adaPath);
        xpub = GetCurrentAccountPublicKey(xpubIndex);
        TransactionParseResult_DisplayCardanoTx *parseResult = cardano_parse_tx(data, mfp, xpub);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    free_simple_response_c_char(path);
    return g_parseResult;
}

void *GuiGetAdaSignDataData(void)
{
    CHECK_FREE_PARSE_SIGN_DATA_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        TransactionParseResult_DisplayCardanoSignData *parseResult = cardano_parse_sign_data(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
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
    uint8_t xpubIndex = GetXPubIndexByPath(adaPath);
    xpub = GetCurrentAccountPublicKey(xpubIndex);
    PtrT_TransactionCheckResult result = cardano_check_tx(data, mfp, xpub);
    free_simple_response_c_char(path);
    return result;
}

PtrT_TransactionCheckResult GuiGetAdaSignDataCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    PtrT_TransactionCheckResult result = cardano_check_sign_data(data, mfp);
    return result;
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

void GetAdaCertificatesLabel(void *indata, void *param, uint32_t maxLen)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    snprintf_s((char *)indata,  maxLen, "%d Certificate(s)", tx->certificates->size);
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
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "%d #F5870A %s#", index + 1, tx->from->data[index].amount);
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
    *row = 2 * tx->to->size;
    int i = 0, j = 0;
    char ***indata = (char ***)SRAM_MALLOC(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        indata[i] = SRAM_MALLOC(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            uint32_t index = j / 2;
            indata[i][j] = SRAM_MALLOC(BUFFER_SIZE_128);
            if (j % 2 == 0) {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "%d #F5870A %s#", index + 1, tx->to->data[index].amount);
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
    *height = 16 + 30 + 188 * tx->from->size + 16;
}
void GetAdaOutputDetailSize(uint16_t *width, uint16_t *height, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *width = 408;
    *height = 16 + 30 + 94 * tx->to->size + 16;
}

bool GetAdaCertificatesExist(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    return tx->certificates->size > 0;
}

void GetAdaCertificatesSize(uint16_t *width, uint16_t *height, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *width = 408;
    *height = 16 + 128 * tx->certificates->size + 16;
}
void *GetAdaCertificatesData(uint8_t *row, uint8_t *col, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *col = 1;
    *row = 3 * tx->certificates->size;
    int i = 0, j = 0;
    char ***indata = (char ***)SRAM_MALLOC(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        indata[i] = SRAM_MALLOC(sizeof(char *) * *row);
        for (j = 0; j < *row; j++) {
            uint32_t index = j / 3;
            indata[i][j] = SRAM_MALLOC(BUFFER_SIZE_128);
            memset_s(indata[i][j], BUFFER_SIZE_128, 0, BUFFER_SIZE_128);
            if (j % 3 == 0) {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "%d #F5870A %s#", index + 1, tx->certificates->data[index].cert_type);
            } else if (j % 3 == 1) {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "%s: %s", tx->certificates->data[index].variant1_label, tx->certificates->data[index].variant1);
            } else {
                if (tx->certificates->data[index].variant2 != NULL) {
                    snprintf_s(indata[i][j], BUFFER_SIZE_128,  "%s: %s", tx->certificates->data[index].variant2_label, tx->certificates->data[index].variant2);
                }
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
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "%d #F5870A %s#", index + 1, tx->withdrawals->data[index].amount);
            } else {
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "Address: %s", tx->withdrawals->data[index].address);
            }
        }
    }
    return (void *)indata;
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
        GetAccountEntropy(GetCurrentAccountIndex(), entropy, &len, SecretCacheGetPassword());
        encodeResult = cardano_sign_sign_data(data, entropy, len, GetPassphrase(GetCurrentAccountIndex()));
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
        GetAccountEntropy(GetCurrentAccountIndex(), entropy, &len, SecretCacheGetPassword());
        encodeResult = cardano_sign_tx(data, mfp, xpub, entropy, len, GetPassphrase(GetCurrentAccountIndex()));
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

char *GuiGetADABaseAddressByIndex(uint16_t index)
{
    char *xPub = NULL;
    SimpleResponse_c_char *result = NULL;
    xPub = GetCurrentAccountPublicKey(XPUB_TYPE_ADA_0 + index);
    result = cardano_get_base_address(xPub, 0, 1);
    if (result->error_code == 0) {
        strcpy(g_adaBaseAddr, result->data);
    }
    free_simple_response_c_char(result);
    return g_adaBaseAddr;
}

static uint8_t GetXPubIndexByPath(char *path)
{
    if (strcmp("1852'/1815'/1'", path) == 0)
        return XPUB_TYPE_ADA_1;
    if (strcmp("1852'/1815'/2'", path) == 0)
        return XPUB_TYPE_ADA_2;
    if (strcmp("1852'/1815'/3'", path) == 0)
        return XPUB_TYPE_ADA_3;
    if (strcmp("1852'/1815'/4'", path) == 0)
        return XPUB_TYPE_ADA_4;
    if (strcmp("1852'/1815'/5'", path) == 0)
        return XPUB_TYPE_ADA_5;
    if (strcmp("1852'/1815'/6'", path) == 0)
        return XPUB_TYPE_ADA_6;
    if (strcmp("1852'/1815'/7'", path) == 0)
        return XPUB_TYPE_ADA_7;
    if (strcmp("1852'/1815'/8'", path) == 0)
        return XPUB_TYPE_ADA_8;
    if (strcmp("1852'/1815'/9'", path) == 0)
        return XPUB_TYPE_ADA_9;
    if (strcmp("1852'/1815'/10'", path) == 0)
        return XPUB_TYPE_ADA_10;
    if (strcmp("1852'/1815'/11'", path) == 0)
        return XPUB_TYPE_ADA_11;
    if (strcmp("1852'/1815'/12'", path) == 0)
        return XPUB_TYPE_ADA_12;
    if (strcmp("1852'/1815'/13'", path) == 0)
        return XPUB_TYPE_ADA_13;
    if (strcmp("1852'/1815'/14'", path) == 0)
        return XPUB_TYPE_ADA_14;
    if (strcmp("1852'/1815'/15'", path) == 0)
        return XPUB_TYPE_ADA_15;
    if (strcmp("1852'/1815'/16'", path) == 0)
        return XPUB_TYPE_ADA_16;
    if (strcmp("1852'/1815'/17'", path) == 0)
        return XPUB_TYPE_ADA_17;
    if (strcmp("1852'/1815'/18'", path) == 0)
        return XPUB_TYPE_ADA_18;
    if (strcmp("1852'/1815'/19'", path) == 0)
        return XPUB_TYPE_ADA_19;
    if (strcmp("1852'/1815'/20'", path) == 0)
        return XPUB_TYPE_ADA_20;
    if (strcmp("1852'/1815'/21'", path) == 0)
        return XPUB_TYPE_ADA_21;
    if (strcmp("1852'/1815'/22'", path) == 0)
        return XPUB_TYPE_ADA_22;
    if (strcmp("1852'/1815'/23'", path) == 0)
        return XPUB_TYPE_ADA_23;
    return XPUB_TYPE_ADA_0;
}

#endif

