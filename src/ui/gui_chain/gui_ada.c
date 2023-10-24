#include "rust.h"
#include "keystore.h"
#include "user_memory.h"
#include "gui_chain.h"
#include "screen_manager.h"
#include "keystore.h"
#include "account_manager.h"
#include "secret_cache.h"
#include "assert.h"
#include "gui_ada.h"
#include "gui_hintbox.h"
#include "gui.h"

static bool g_isMulti = false;
static void *g_urResult = NULL;
static void *g_parseResult = NULL;
static char *xpub = NULL;
static lv_obj_t *g_waitAnimCont = NULL;

static uint8_t GetXPubIndexByPath(char *path);

void GuiSetupAdaUrData(void *data, bool multi)
{
#ifndef COMPILE_SIMULATOR
    g_urResult = data;
    g_isMulti = multi;
#endif
}

#define CHECK_FREE_PARSE_RESULT(result)                                                                         \
    if (result != NULL)                                                                                         \
    {                                                                                                           \
        free_transaction_parse_result_display_cardano_tx((PtrT_TransactionParseResult_DisplayCardanoTx)result); \
        result = NULL;                                                                                          \
    }

void *GuiGetAdaData(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    TransactionCheckResult *result = NULL;
    SimpleResponse_c_char *path = NULL;
    do
    {
        path = cardano_get_path(data);
        CHECK_CHAIN_BREAK(path);
        char *adaPath = path->data;
        uint8_t xpubIndex = GetXPubIndexByPath(adaPath);
        xpub = GetCurrentAccountPublicKey(xpubIndex);
        result = cardano_check_tx(data, mfp, xpub);
        CHECK_CHAIN_BREAK(result);
        TransactionParseResult_DisplayCardanoTx *parseResult = cardano_parse_tx(data, mfp, xpub);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    free_TransactionCheckResult(result);
    return g_parseResult;
#else
    TransactionParseResult_DisplayCardanoTx *parseResult = SRAM_MALLOC(sizeof(TransactionParseResult_DisplayCardanoTx));
    parseResult->error_code = 0;
    parseResult->error_message = "";
    DisplayCardanoTx *data = SRAM_MALLOC(sizeof(DisplayCardanoTx));
    parseResult->data = data;
    data->auxiliary_data = "test data";
    data->network = "Cardano Mainnet";
    data->total_input = "200 ADA";
    data->total_output = "100 ADA";
    data->fee = "0.5 ADA";
    data->from = SRAM_MALLOC(sizeof(VecFFI_DisplayCardanoFrom));
    data->from->cap = 2;
    data->from->size = 2;
    data->from->data = SRAM_MALLOC(2 * sizeof(DisplayCardanoFrom));
    data->from->data[0].address = "addr1qxmst0syfr5adp29ypq744zwxhrjyghvj7m56my0xhyv044zsqazww2j94e9myatt6ng5a4vqhqclaa394rgumhcw9cqtzv4fg";
    data->from->data[0].amount = "100 ADA";
    data->from->data[0].has_path = true;
    data->from->data[0].path = "m/1852'/1815'/0'/0/0";

    data->from->data[1].address = "addr1q8am662n4zluq5cg8teephya2nznp3vhc6a5xgusuffw93azsqazww2j94e9myatt6ng5a4vqhqclaa394rgumhcw9cqc9wgkc";
    data->from->data[1].amount = "100 ADA";
    data->from->data[1].has_path = true;
    data->from->data[1].path = "m/1852'/1815'/0'/0/1";

    data->to = SRAM_MALLOC(sizeof(VecFFI_DisplayCardanoTo));
    data->to->cap = 1;
    data->to->size = 1;
    data->to->data = SRAM_MALLOC(sizeof(DisplayCardanoTo));
    data->to->data[0].address = "addr1q8am662n4zluq5cg8teephya2nznp3vhc6a5xgusuffw93azsqazww2j94e9myatt6ng5a4vqhqclaa394rgumhcw9cqc9wgkc";
    data->to->data[0].amount = "150 ADA";

    data->certificates = SRAM_MALLOC(sizeof(VecFFI_DisplayCardanoCertificate));
    data->certificates->cap = 3;
    data->certificates->size = 3;
    data->certificates->data = SRAM_MALLOC(3 * sizeof(DisplayCardanoCertificate));
    data->certificates->data[0].address = "stake1ux3gqw3889fz6ujajw44af52w6kqtsv077cj635wdmu8zuqyksq6g";
    data->certificates->data[0].cert_type = "stake registration";
    data->certificates->data[1].address = "stake1ux3gqw3889fz6ujajw44af52w6kqtsv077cj635wdmu8zuqyksq6g";
    data->certificates->data[1].cert_type = "stake deregistration";
    data->certificates->data[2].address = "stake1ux3gqw3889fz6ujajw44af52w6kqtsv077cj635wdmu8zuqyksq6g";
    data->certificates->data[2].cert_type = "stake delegation";
    data->certificates->data[2].pool = "pool1ld9hkah2dkzh73pvh9tf6xr0x28us34msv3zcv2sase5vhvq962";

    data->withdrawals = SRAM_MALLOC(sizeof(VecFFI_DisplayCardanoWithdrawal));
    data->withdrawals->cap = 1;
    data->withdrawals->size = 1;
    data->withdrawals->data = SRAM_MALLOC(sizeof(DisplayCardanoWithdrawal));
    data->withdrawals->data[0].address = "stake1ux3gqw3889fz6ujajw44af52w6kqtsv077cj635wdmu8zuqyksq6g";
    data->withdrawals->data[0].amount = "6.00135 ADA";
    return parseResult;
#endif
}

void FreeAdaMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, g_isMulti);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
#endif
}

bool GetAdaExtraDataExist(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    if (tx->auxiliary_data == NULL)
    {
        return false;
    }
    return strlen(tx->auxiliary_data) > 0;   
}

int GetAdaExtraDataLen(void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    return strlen(tx->auxiliary_data);
}

void GetAdaExtraData(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    sprintf((char *)indata, "%s", tx->auxiliary_data);
}

void GetAdaNetwork(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    sprintf((char *)indata, "%s", tx->network);
}

void GetAdaWithdrawalsLabel(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    sprintf((char *)indata, "%d Withdraw(s)", tx->withdrawals->size);
}
void GetAdaCertificatesLabel(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    sprintf((char *)indata, "%d Certificate(s)", tx->certificates->size);
}

void GetAdaTotalInput(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    sprintf((char *)indata, "%s", tx->total_input);
}

void GetAdaTotalOutput(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    sprintf((char *)indata, "%s", tx->total_output);
}

void GetAdaFee(void *indata, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    sprintf((char *)indata, "%s", tx->fee);
}

void *GetAdaInputDetail(uint8_t *row, uint8_t *col, void *param)
{
    DisplayCardanoTx *tx = (DisplayCardanoTx *)param;
    *col = 1;
    *row = 3 * tx->from->size;
    int i = 0, j = 0;
    char ***indata = (char ***)malloc(sizeof(char **) * *col);
    for (i = 0; i < *col; i++)
    {
        indata[i] = malloc(sizeof(char *) * *row);
        for (j = 0; j < *row; j++)
        {
            uint32_t index = j / 3;
            indata[i][j] = malloc(128);
            if (j % 3 == 0)
            {
                sprintf(indata[i][j], "%d #F5870A %s#", index + 1, tx->from->data[index].amount);
            }
            else if (j % 3 == 1)
            {
                sprintf(indata[i][j], "%s", tx->from->data[index].address);
            }
            else
            {
                if (tx->from->data[index].has_path)
                {
                    strcpy(indata[i][j], tx->from->data[index].path);
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
    char ***indata = (char ***)malloc(sizeof(char **) * *col);
    for (i = 0; i < *col; i++)
    {
        indata[i] = malloc(sizeof(char *) * *row);
        for (j = 0; j < *row; j++)
        {
            uint32_t index = j / 2;
            indata[i][j] = malloc(128);
            if (j % 2 == 0)
            {
                sprintf(indata[i][j], "%d #F5870A %s#", index + 1, tx->to->data[index].amount);
            }
            else
            {
                sprintf(indata[i][j], "%s", tx->to->data[index].address);
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
    *height = 16 + 30 + 94 * tx->from->size + 16;
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
    char ***indata = (char ***)malloc(sizeof(char **) * *col);
    for (i = 0; i < *col; i++)
    {
        indata[i] = malloc(sizeof(char *) * *row);
        for (j = 0; j < *row; j++)
        {
            uint32_t index = j / 3;
            indata[i][j] = malloc(128);
            if (j % 3 == 0)
            {
                sprintf(indata[i][j], "%d #F5870A %s#", index + 1, tx->certificates->data[index].cert_type);
            }
            else if (j % 3 == 1)
            {
                sprintf(indata[i][j], "Address: %s", tx->certificates->data[index].address);
            }
            else
            {
                if (tx->certificates->data[index].pool != NULL)
                {
                    sprintf(indata[i][j], "Pool: %s", tx->certificates->data[index].pool);
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
    char ***indata = (char ***)malloc(sizeof(char **) * *col);
    for (i = 0; i < *col; i++)
    {
        indata[i] = malloc(sizeof(char *) * *row);
        for (j = 0; j < *row; j++)
        {
            uint32_t index = j / 2;
            indata[i][j] = malloc(128);
            if (j % 2 == 0)
            {
                sprintf(indata[i][j], "%d #F5870A %s#", index + 1, tx->withdrawals->data[index].amount);
            }
            else
            {
                sprintf(indata[i][j], "Address: %s", tx->withdrawals->data[index].address);
            }
        }
    }
    return (void *)indata;
}

UREncodeResult *GuiGetAdaSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    GUI_DEL_OBJ(g_waitAnimCont);

    g_waitAnimCont = GuiCreateAnimHintBox(lv_scr_act(), 480, 386, 82);
    lv_obj_t *title = GuiCreateTextLabel(g_waitAnimCont, _("sign_transaction"));
    lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, -194);
    lv_obj_t *desc = GuiCreateNoticeLabel(g_waitAnimCont, _("sign_transaction_desc"));
    lv_obj_align(desc, LV_ALIGN_BOTTOM_MID, 0, -86);
    lv_obj_set_style_text_align(desc, LV_TEXT_ALIGN_CENTER, 0);

    UREncodeResult *encodeResult;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
    do
    {
        uint8_t entropy[64];
        uint32_t len = GetCurrentAccountEntropyLen();
        GetAccountEntropy(GetCurrentAccountIndex(), entropy, len, SecretCacheGetPassword());
        char *path = cardano_get_path(data);
        char pubkeyIndex = GetXPubIndexByPath(path);
        char *pubKey = GetCurrentAccountPublicKey(pubkeyIndex);
        encodeResult = cardano_sign_tx(data, mfp, xpub, entropy, len, GetPassphrase(GetCurrentAccountIndex()));
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    GUI_DEL_OBJ(g_waitAnimCont);
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
