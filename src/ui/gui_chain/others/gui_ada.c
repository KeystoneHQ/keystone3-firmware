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

#define ADA_ADD_MAX_LEN             (150)

static bool g_isMulti = false;
static struct URParseResult *g_urResult = NULL;
static struct URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static char g_adaBaseAddr[ADA_ADD_MAX_LEN];
static char *xpub = NULL;
static AdaXPubType g_adaXpubTypes[3] = {STANDARD_ADA, STANDARD_ADA, STANDARD_ADA};
static AdaXPubType g_receivePageAdaXpubTypes[3] = {STANDARD_ADA, STANDARD_ADA, STANDARD_ADA};
static AdaXPubType g_keyDerivationAdaXpubTypes[3] = {STANDARD_ADA, STANDARD_ADA, STANDARD_ADA};

static void Try2FixAdaPathType();

void SetAdaXPubType(AdaXPubType type)
{
    g_adaXpubTypes[GetCurrentAccountIndex()] = type;
}

AdaXPubType GetAdaXPubType(void)
{
    return g_adaXpubTypes[GetCurrentAccountIndex()];
}

void SetReceivePageAdaXPubType(AdaXPubType type)
{
    g_receivePageAdaXpubTypes[GetCurrentAccountIndex()] = type;
}

AdaXPubType GetReceivePageAdaXPubType(void)
{
    return g_receivePageAdaXpubTypes[GetCurrentAccountIndex()];
}

void SetKeyDerivationAdaXPubType(AdaXPubType type)
{
    g_keyDerivationAdaXpubTypes[GetCurrentAccountIndex()] = type;
}

AdaXPubType GetKeyDerivationAdaXPubType(void)
{
    return g_keyDerivationAdaXpubTypes[GetCurrentAccountIndex()];
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
        TransactionParseResult_DisplayCardanoSignData *parseResult = cardano_parse_sign_data(data);
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
    xpub = GetCurrentAccountPublicKey(GetXPubIndexByPath(adaPath));
    PtrT_TransactionCheckResult result = cardano_check_tx(data, mfp, xpub);
    if (result->error_code != 0) {
        free_TransactionCheckResult(result);
        Try2FixAdaPathType();
        xpub = GetCurrentAccountPublicKey(GetXPubIndexByPath(adaPath));
        result = cardano_check_tx(data, mfp, xpub);
    }
    free_simple_response_c_char(path);
    return result;
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
    if (master_key_index->error_code != 0) {
        return NULL;
    }
    uint16_t index = atoi(master_key_index->data);
    char *xpub = GetCurrentAccountPublicKey(GetAdaXPubTypeByIndex(index));
    PtrT_TransactionCheckResult precheckResult;
    precheckResult= cardano_check_catalyst_path_type(data, xpub);
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
    if (GetAdaXPubType() == LEDGER_ADA) {
        SetAdaXPubType(STANDARD_ADA);
    } else {
        SetAdaXPubType(LEDGER_ADA);
    }
}

PtrT_TransactionCheckResult GuiGetAdaSignDataCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    Ptr_SimpleResponse_c_char master_key_index = cardano_get_sign_data_root_index(data);
    if (master_key_index->error_code != 0) {
        return NULL;
    }
    uint16_t index = atoi(master_key_index->data);
    char *xpub = GetCurrentAccountPublicKey(GetAdaXPubTypeByIndex(index));
    PtrT_TransactionCheckResult precheckResult;
    precheckResult= cardano_check_sign_data_path_type(data, xpub);
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
    *height = 16 + 30 + 218 * tx->from->size + 16;
}
void GetAdaOutputDetailSize(uint16_t *width, uint16_t *height, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *width = 408;
    *height = 16 + 30 + 154 * tx->to->size + 16;
}

bool GetAdaCertificatesExist(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    return tx->certificates->size > 0;
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
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    snprintf_s((char *)indata,  maxLen, _("ada_proposals_tx_notice"));
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
                snprintf_s(indata[i][j], BUFFER_SIZE_128,  "##%d\n#F5870A Voting Procedure#", index + 1);
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

static uint8_t calculateCertificateField(DisplayCardanoCertificate *cert)
{
    return cert->fields->size;
}

static uint8_t calculateCertificateRow(DisplayCardanoTx *tx)
{
    uint8_t row = tx->certificates->size;
    for (int i = 0; i < tx->certificates->size; i++) {
        row += calculateCertificateField(&tx->certificates->data[i]);
    }
    return row;
}

void GetAdaCertificatesSize(uint16_t *width, uint16_t *height, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *width = 408;
    *height = 16 + 30 + 60 * calculateCertificateRow(tx) + 16;
}

void *GetAdaCertificatesData(uint8_t *row, uint8_t *col, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *col = 1;
    *row = calculateCertificateRow(tx);
    int i = 0, j = 0;
    char ***indata = (char ***)SRAM_MALLOC(sizeof(char **) * *col);
    for (i = 0; i < *col; i++) {
        indata[i] = SRAM_MALLOC(sizeof(char *) * *row);
        uint32_t fieldCount = 0;
        for (j = 0; j < tx->certificates->size; j++) {
            for (int k = fieldCount; k < fieldCount + calculateCertificateField(&tx->certificates->data[j]) + 1; k++) {
                indata[i][k] = SRAM_MALLOC(BUFFER_SIZE_128);
                if (k == fieldCount) {
                    snprintf_s(indata[i][k], BUFFER_SIZE_128,  "##%d\n#F5870A %s#", j + 1, tx->certificates->data[j].cert_type);
                } else {
                    snprintf_s(indata[i][k], BUFFER_SIZE_128,  "%s: %s", tx->certificates->data[j].fields->data[k - fieldCount - 1].label, tx->certificates->data[j].fields->data[k - fieldCount - 1].value);
                }
            }
            fieldCount += calculateCertificateField(&tx->certificates->data[j]) + 1;
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
        GetAccountEntropy(GetCurrentAccountIndex(), entropy, &len, SecretCacheGetPassword());
        if (GetAdaXPubType() == LEDGER_ADA) {
            char *mnemonic = NULL;
            bip39_mnemonic_from_bytes(NULL, entropy, len, &mnemonic);
            encodeResult = cardano_sign_catalyst_with_ledger_bitbox02(data, mnemonic, GetPassphrase(GetCurrentAccountIndex()));
        } else {
            encodeResult = cardano_sign_catalyst(data, entropy, len, GetPassphrase(GetCurrentAccountIndex()));
        }
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
        GetAccountEntropy(GetCurrentAccountIndex(), entropy, &len, SecretCacheGetPassword());
        if (GetAdaXPubType() == LEDGER_ADA) {
            char *mnemonic = NULL;
            bip39_mnemonic_from_bytes(NULL, entropy, len, &mnemonic);
            encodeResult = cardano_sign_sign_data_with_ledger_bitbox02(data, mnemonic, GetPassphrase(GetCurrentAccountIndex()));
        } else {
            encodeResult = cardano_sign_sign_data(data, entropy, len, GetPassphrase(GetCurrentAccountIndex()));
        }
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
        if (GetAdaXPubType() == LEDGER_ADA) {
            char *mnemonic = NULL;
            bip39_mnemonic_from_bytes(NULL, entropy, len, &mnemonic);
            encodeResult = cardano_sign_tx_with_ledger_bitbox02(data, mfp, xpub, mnemonic, GetPassphrase(GetCurrentAccountIndex()));
        } else {
            encodeResult = cardano_sign_tx(data, mfp, xpub, entropy, len, GetPassphrase(GetCurrentAccountIndex()));
        }
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

ChainType GetAdaXPubTypeByIndex(uint16_t index)
{
    switch (index) {
    case 0:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_0 : XPUB_TYPE_LEDGER_ADA_0;
    case 1:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_1 : XPUB_TYPE_LEDGER_ADA_1;
    case 2:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_2 : XPUB_TYPE_LEDGER_ADA_2;
    case 3:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_3 : XPUB_TYPE_LEDGER_ADA_3;
    case 4:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_4 : XPUB_TYPE_LEDGER_ADA_4;
    case 5:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_5 : XPUB_TYPE_LEDGER_ADA_5;
    case 6:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_6 : XPUB_TYPE_LEDGER_ADA_6;
    case 7:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_7 : XPUB_TYPE_LEDGER_ADA_7;
    case 8:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_8 : XPUB_TYPE_LEDGER_ADA_8;
    case 9:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_9 : XPUB_TYPE_LEDGER_ADA_9;
    case 10:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_10 : XPUB_TYPE_LEDGER_ADA_10;
    case 11:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_11 : XPUB_TYPE_LEDGER_ADA_11;
    case 12:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_12 : XPUB_TYPE_LEDGER_ADA_12;
    case 13:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_13 : XPUB_TYPE_LEDGER_ADA_13;
    case 14:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_14 : XPUB_TYPE_LEDGER_ADA_14;
    case 15:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_15 : XPUB_TYPE_LEDGER_ADA_15;
    case 16:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_16 : XPUB_TYPE_LEDGER_ADA_16;
    case 17:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_17 : XPUB_TYPE_LEDGER_ADA_17;
    case 18:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_18 : XPUB_TYPE_LEDGER_ADA_18;
    case 19:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_19 : XPUB_TYPE_LEDGER_ADA_19;
    case 20:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_20 : XPUB_TYPE_LEDGER_ADA_20;
    case 21:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_21 : XPUB_TYPE_LEDGER_ADA_21;
    case 22:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_22 : XPUB_TYPE_LEDGER_ADA_22;
    case 23:
        return GetAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_23 : XPUB_TYPE_LEDGER_ADA_23;
    default:
        return XPUB_TYPE_ADA_0;
    }
}

ChainType GetKeyDerivationAdaXPubTypeByIndex(uint16_t index)
{
    switch (index) {
    case 0:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_0 : XPUB_TYPE_LEDGER_ADA_0;
    case 1:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_1 : XPUB_TYPE_LEDGER_ADA_1;
    case 2:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_2 : XPUB_TYPE_LEDGER_ADA_2;
    case 3:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_3 : XPUB_TYPE_LEDGER_ADA_3;
    case 4:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_4 : XPUB_TYPE_LEDGER_ADA_4;
    case 5:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_5 : XPUB_TYPE_LEDGER_ADA_5;
    case 6:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_6 : XPUB_TYPE_LEDGER_ADA_6;
    case 7:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_7 : XPUB_TYPE_LEDGER_ADA_7;
    case 8:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_8 : XPUB_TYPE_LEDGER_ADA_8;
    case 9:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_9 : XPUB_TYPE_LEDGER_ADA_9;
    case 10:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_10 : XPUB_TYPE_LEDGER_ADA_10;
    case 11:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_11 : XPUB_TYPE_LEDGER_ADA_11;
    case 12:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_12 : XPUB_TYPE_LEDGER_ADA_12;
    case 13:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_13 : XPUB_TYPE_LEDGER_ADA_13;
    case 14:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_14 : XPUB_TYPE_LEDGER_ADA_14;
    case 15:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_15 : XPUB_TYPE_LEDGER_ADA_15;
    case 16:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_16 : XPUB_TYPE_LEDGER_ADA_16;
    case 17:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_17 : XPUB_TYPE_LEDGER_ADA_17;
    case 18:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_18 : XPUB_TYPE_LEDGER_ADA_18;
    case 19:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_19 : XPUB_TYPE_LEDGER_ADA_19;
    case 20:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_20 : XPUB_TYPE_LEDGER_ADA_20;
    case 21:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_21 : XPUB_TYPE_LEDGER_ADA_21;
    case 22:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_22 : XPUB_TYPE_LEDGER_ADA_22;
    case 23:
        return GetKeyDerivationAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_23 : XPUB_TYPE_LEDGER_ADA_23;
    default:
        return XPUB_TYPE_ADA_0;
    }
}

ChainType GetReceivePageAdaXPubTypeByIndex(uint16_t index)
{
    switch (index) {
    case 0:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_0 : XPUB_TYPE_LEDGER_ADA_0;
    case 1:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_1 : XPUB_TYPE_LEDGER_ADA_1;
    case 2:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_2 : XPUB_TYPE_LEDGER_ADA_2;
    case 3:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_3 : XPUB_TYPE_LEDGER_ADA_3;
    case 4:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_4 : XPUB_TYPE_LEDGER_ADA_4;
    case 5:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_5 : XPUB_TYPE_LEDGER_ADA_5;
    case 6:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_6 : XPUB_TYPE_LEDGER_ADA_6;
    case 7:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_7 : XPUB_TYPE_LEDGER_ADA_7;
    case 8:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_8 : XPUB_TYPE_LEDGER_ADA_8;
    case 9:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_9 : XPUB_TYPE_LEDGER_ADA_9;
    case 10:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_10 : XPUB_TYPE_LEDGER_ADA_10;
    case 11:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_11 : XPUB_TYPE_LEDGER_ADA_11;
    case 12:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_12 : XPUB_TYPE_LEDGER_ADA_12;
    case 13:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_13 : XPUB_TYPE_LEDGER_ADA_13;
    case 14:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_14 : XPUB_TYPE_LEDGER_ADA_14;
    case 15:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_15 : XPUB_TYPE_LEDGER_ADA_15;
    case 16:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_16 : XPUB_TYPE_LEDGER_ADA_16;
    case 17:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_17 : XPUB_TYPE_LEDGER_ADA_17;
    case 18:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_18 : XPUB_TYPE_LEDGER_ADA_18;
    case 19:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_19 : XPUB_TYPE_LEDGER_ADA_19;
    case 20:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_20 : XPUB_TYPE_LEDGER_ADA_20;
    case 21:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_21 : XPUB_TYPE_LEDGER_ADA_21;
    case 22:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_22 : XPUB_TYPE_LEDGER_ADA_22;
    case 23:
        return GetReceivePageAdaXPubType() == STANDARD_ADA ? XPUB_TYPE_ADA_23 : XPUB_TYPE_LEDGER_ADA_23;
    default:
        return XPUB_TYPE_ADA_0;
    }
}

char *GuiGetADABaseAddressByIndex(uint16_t index)
{
    char *xPub = NULL;
    SimpleResponse_c_char *result = NULL;
    xPub = GetCurrentAccountPublicKey(GetAdaXPubTypeByIndex(index));
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

#endif

