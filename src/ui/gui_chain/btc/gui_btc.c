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
#include "gui_transaction_detail_widgets.h"
#ifdef BTC_ONLY
#include "gui_multisig_transaction_signature_widgets.h"
#include "gui_btc_home_widgets.h"
#endif

#ifndef COMPILE_SIMULATOR
#include "safe_str_lib.h"
#else
#include "simulator_mock_define.h"
#endif

#define MAX_WALLET_CONFIG_LEN 3000
#define MAX_VERIFY_CODE_LEN 12

#define CHECK_FREE_PARSE_RESULT(result)                             \
    if (result != NULL) {                                           \
        free_TransactionParseResult_DisplayTx(g_parseResult);       \
        g_parseResult = NULL;                                       \
    }

#define CHECK_FREE_PARSE_MSG_RESULT(result)                             \
    if (result != NULL) {                                           \
        free_TransactionParseResult_DisplayBtcMsg(g_parseMsgResult);       \
        g_parseMsgResult = NULL;                                       \
    }

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;

static uint8_t* g_psbtBytes = NULL;
static uint32_t g_psbtBytesLen = 0;

static TransactionParseResult_DisplayTx *g_parseResult = NULL;
static TransactionParseResult_DisplayBtcMsg *g_parseMsgResult = NULL;

static bool IsMultiSigTx(DisplayTx *data);

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
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

void GuiSetPsbtStrData(char *psbtBytes, uint32_t psbtBytesLen)
{
    g_psbtBytes = psbtBytes;
    g_psbtBytesLen = psbtBytesLen;
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

#ifdef BTC_ONLY
static UREncodeResult *GuiGetSignPsbtBytesCodeData(void)
{
    UREncodeResult *encodeResult = NULL;
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    uint8_t seed[64];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
    MultisigSignResult *result = btc_sign_multisig_psbt_bytes(g_psbtBytes, g_psbtBytesLen, seed, len, mfp, sizeof(mfp));
    encodeResult = result->ur_result;
    GuiMultisigTransactionSignatureSetSignStatus(result->sign_status, result->is_completed, result->psbt_hex, result->psbt_len);
    free_MultisigSignResult(result);
    CHECK_CHAIN_PRINT(encodeResult);
    ClearSecretCache();
    return encodeResult;
}
#endif

// The results here are released in the close qr timer species
UREncodeResult *GuiGetSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);

#ifdef BTC_ONLY
    if (g_psbtBytes != NULL) {
        return GuiGetSignPsbtBytesCodeData();
    }
#endif

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
#ifdef BTC_ONLY
        if (GuiGetCurrentTransactionType() == TRANSACTION_TYPE_BTC_MULTISIG) {
            MultisigSignResult *result = btc_sign_multisig_psbt(data, seed, len, mfp, sizeof(mfp));
            encodeResult = result->ur_result;
            GuiMultisigTransactionSignatureSetSignStatus(result->sign_status, result->is_completed, result->psbt_hex, result->psbt_len);
            free_MultisigSignResult(result);
        } else {
#else
        {
            encodeResult = btc_sign_psbt(data, seed, len, mfp, sizeof(mfp));
#endif
        }
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
    else if (urType == BtcSignRequest) {
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = btc_sign_msg(data, seed, len, mfp, sizeof(mfp));
    }
    CHECK_CHAIN_PRINT(encodeResult);
    ClearSecretCache();
    SetLockScreen(enable);
    return encodeResult;
}

#ifdef BTC_ONLY
static void *GuiGetParsedPsbtStrData(void)
{
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[14];
    public_keys->data = keys;
    public_keys->size = 14;
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

    keys[8].path = "m/45'";
    keys[8].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2SH);
    keys[9].path = "m/48'/0'/0'/1'";
    keys[9].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH);
    keys[10].path = "m/48'/0'/0'/2'";
    keys[10].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH);
    keys[11].path = "m/45'";
    keys[11].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2SH_TEST);
    keys[12].path = "m/48'/1'/0'/1'";
    keys[12].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH_TEST);
    keys[13].path = "m/48'/1'/0'/2'";
    keys[13].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH_TEST);

    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    char *wallet_config = NULL;
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        MultiSigWalletItem_t *item = GetDefaultMultisigWallet();
        if (item != NULL) {
            wallet_config = SRAM_MALLOC(MAX_WALLET_CONFIG_LEN);
            strncpy_s(wallet_config, MAX_WALLET_CONFIG_LEN, item->walletConfig, strnlen_s(item->walletConfig, MAX_WALLET_CONFIG_LEN));
        }
    }

    g_parseResult = btc_parse_psbt_bytes(g_psbtBytes, g_psbtBytesLen, mfp, sizeof(mfp), public_keys, wallet_config);
    CHECK_CHAIN_RETURN(g_parseResult);
    if (IsMultiSigTx(g_parseResult->data)) {
        GuiSetCurrentTransactionType(TRANSACTION_TYPE_BTC_MULTISIG);
    }
    SRAM_FREE(public_keys);
    return g_parseResult;

}
#endif

void *GuiGetParsedQrData(void)
{
#ifdef BTC_ONLY
    if (g_psbtBytes != NULL) {
        return GuiGetParsedPsbtStrData();
    }
#endif
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
            ExtendedPublicKey keys[14];
            public_keys->data = keys;
            public_keys->size = 14;
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

            keys[8].path = "m/45'";
            keys[8].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2SH);
            keys[9].path = "m/48'/0'/0'/1'";
            keys[9].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH);
            keys[10].path = "m/48'/0'/0'/2'";
            keys[10].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH);
            keys[11].path = "m/45'";
            keys[11].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2SH_TEST);
            keys[12].path = "m/48'/1'/0'/1'";
            keys[12].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH_TEST);
            keys[13].path = "m/48'/1'/0'/2'";
            keys[13].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH_TEST);
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
#ifdef BTC_ONLY
            char *wallet_config = NULL;
            if (GetCurrentWalletIndex() != SINGLE_WALLET) {
                MultiSigWalletItem_t *item = GetDefaultMultisigWallet();
                if (item != NULL) {
                    wallet_config = SRAM_MALLOC(MAX_WALLET_CONFIG_LEN);
                    strncpy_s(wallet_config, MAX_WALLET_CONFIG_LEN, item->walletConfig, strnlen_s(item->walletConfig, MAX_WALLET_CONFIG_LEN));
                }
            }
            g_parseResult = btc_parse_psbt(crypto, mfp, sizeof(mfp), public_keys, wallet_config);
#else
            g_parseResult = btc_parse_psbt(crypto, mfp, sizeof(mfp), public_keys, NULL);
#endif
            CHECK_CHAIN_RETURN(g_parseResult);
            if (IsMultiSigTx(g_parseResult->data)) {
                GuiSetCurrentTransactionType(TRANSACTION_TYPE_BTC_MULTISIG);
            }
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
        else if (urType == BtcSignRequest) {
            g_parseMsgResult = btc_parse_msg(crypto, mfp, sizeof(mfp));
            CHECK_CHAIN_RETURN(g_parseMsgResult);
            return g_parseMsgResult;
        }
    } while (0);
    return g_parseResult;
}

#ifdef BTC_ONLY
PtrT_TransactionCheckResult GuiGetPsbtStrCheckResult(void)
{
    PtrT_TransactionCheckResult result = NULL;
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[14];
    public_keys->data = keys;
    public_keys->size = 14;
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

    keys[8].path = "m/45'";
    keys[8].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2SH);
    keys[9].path = "m/48'/0'/0'/1'";
    keys[9].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH);
    keys[10].path = "m/48'/0'/0'/2'";
    keys[10].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH);
    keys[11].path = "m/45'";
    keys[11].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2SH_TEST);
    keys[12].path = "m/48'/1'/0'/1'";
    keys[12].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH_TEST);
    keys[13].path = "m/48'/1'/0'/2'";
    keys[13].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH_TEST);

    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    char *verify_code = NULL;
    char *wallet_config = NULL;
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        MultiSigWalletItem_t *item = GetDefaultMultisigWallet();
        if (item != NULL) {
            verify_code = SRAM_MALLOC(MAX_VERIFY_CODE_LEN);
            strncpy_s(verify_code, MAX_VERIFY_CODE_LEN, item->verifyCode, strnlen_s(item->verifyCode, MAX_VERIFY_CODE_LEN));
            wallet_config = SRAM_MALLOC(MAX_WALLET_CONFIG_LEN);
            strncpy_s(wallet_config, MAX_WALLET_CONFIG_LEN, item->walletConfig, strnlen_s(item->walletConfig, MAX_WALLET_CONFIG_LEN));
        }
    }

    result = btc_check_psbt_bytes(g_psbtBytes, g_psbtBytesLen, mfp, sizeof(mfp), public_keys, verify_code, wallet_config);
    SRAM_FREE(public_keys);
    return result;
}
#endif

PtrT_TransactionCheckResult GuiGetPsbtCheckResult(void)
{
#ifdef BTC_ONLY
    if (g_psbtBytes != NULL) {
        return GuiGetPsbtStrCheckResult();
    }
#endif
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
        ExtendedPublicKey keys[14];
        public_keys->data = keys;
        public_keys->size = 14;
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

        keys[8].path = "m/45'";
        keys[8].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2SH);
        keys[9].path = "m/48'/0'/0'/1'";
        keys[9].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH);
        keys[10].path = "m/48'/0'/0'/2'";
        keys[10].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH);
        keys[11].path = "m/45'";
        keys[11].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2SH_TEST);
        keys[12].path = "m/48'/1'/0'/1'";
        keys[12].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH_TEST);
        keys[13].path = "m/48'/1'/0'/2'";
        keys[13].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_MULTI_SIG_P2WSH_TEST);
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
#ifdef BTC_ONLY
        char *verify_code = NULL;
        char *wallet_config = NULL;
        if (GetCurrentWalletIndex() != SINGLE_WALLET) {
            MultiSigWalletItem_t *item = GetDefaultMultisigWallet();
            if (item != NULL) {
                verify_code = SRAM_MALLOC(MAX_VERIFY_CODE_LEN);
                strncpy_s(verify_code, MAX_VERIFY_CODE_LEN, item->verifyCode, strnlen_s(item->verifyCode, MAX_VERIFY_CODE_LEN));
                wallet_config = SRAM_MALLOC(MAX_WALLET_CONFIG_LEN);
                strncpy_s(wallet_config, MAX_WALLET_CONFIG_LEN, item->walletConfig, strnlen_s(item->walletConfig, MAX_WALLET_CONFIG_LEN));
            }
        }
        result = btc_check_psbt(crypto, mfp, sizeof(mfp), public_keys, verify_code, wallet_config);
#else
        result = btc_check_psbt(crypto, mfp, sizeof(mfp), public_keys, NULL, NULL);
#endif
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
    else if (urType == BtcSignRequest) {
        result = btc_check_msg(crypto, mfp, sizeof(mfp));
    }
    return result;
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

int GetBtcMsgDetailLen(void *param)
{
    DisplayBtcMsg *tx = (DisplayBtcMsg *)param;
    return strlen(tx->detail) + 1;
}

void GetBtcMsgDetail(void *indata, void *param, uint32_t maxLen)
{
    DisplayBtcMsg *tx = (DisplayBtcMsg *)param;
    strcpy_s((char *)indata, maxLen, tx->detail);
}

void FreePsbtUxtoMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    if (g_psbtBytes != NULL) {
        EXT_FREE(g_psbtBytes);
        g_psbtBytes = NULL;
    }
}

void FreeBtcMsgMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_MSG_RESULT(g_parseMsgResult);
}

static bool IsMultiSigTx(DisplayTx *data)
{
    return data->overview->is_multisig;
}

static void SetContainerDefaultStyle(lv_obj_t *container)
{
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(container, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void SetTitleLabelStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void SetContentLableStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
}

typedef struct ClickParamItem {
    lv_obj_t *amountValue;
    lv_obj_t *feeValue;
    DisplayTxOverview *overviewData;
    bool *isSat;
} ClickParamItem_t;

static void SwitchValueUnit(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        ClickParamItem_t *item = (ClickParamItem_t *)lv_event_get_user_data(e);
        if (item != NULL) {
            if (*(item->isSat) == true) {
                lv_label_set_text(item->amountValue, item->overviewData->total_output_amount);
                lv_label_set_text(item->feeValue, item->overviewData->fee_amount);
            } else {
                lv_label_set_text(item->amountValue, item->overviewData->total_output_sat);
                lv_label_set_text(item->feeValue, item->overviewData->fee_sat);
            }
            *item->isSat = !(*item->isSat);
        }
    }
}

static ClickParamItem_t clickParam;
static bool isSat = false;

static lv_obj_t *CreateSignStatusView(lv_obj_t *parent, char *multi_sig_status)
{
    lv_obj_t *signStatusContainer = GuiCreateContainerWithParent(parent, 408, 62);
    lv_obj_align(signStatusContainer, LV_ALIGN_DEFAULT, 0, 0);
    SetContainerDefaultStyle(signStatusContainer);

    lv_obj_t *label = lv_label_create(signStatusContainer);
    lv_label_set_text(label, "Status");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    SetTitleLabelStyle(label);

    lv_obj_t *value = lv_label_create(signStatusContainer);
    lv_label_set_text(value, multi_sig_status);
    lv_obj_align_to(value, label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    SetContentLableStyle(value);

    return signStatusContainer;
}

static lv_obj_t *CreateOverviewAmountView(lv_obj_t *parent, DisplayTxOverview *overviewData, lv_obj_t *lastView)
{
    lv_obj_t *amountContainer = GuiCreateContainerWithParent(parent, 408, 144);
    if (lastView == NULL) {
        lv_obj_align(amountContainer, LV_ALIGN_DEFAULT, 0, 0);
    } else {
        lv_obj_align_to(amountContainer, lastView, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    }
    SetContainerDefaultStyle(amountContainer);

    lv_obj_t *label = lv_label_create(amountContainer);
    lv_label_set_text(label, "Amount");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    SetTitleLabelStyle(label);

    lv_obj_t *amountValue = lv_label_create(amountContainer);
    lv_label_set_text(amountValue, overviewData->total_output_amount);
    lv_obj_align_to(amountValue, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

    lv_obj_set_style_text_font(amountValue, &openSansEnLittleTitle, LV_PART_MAIN);
    lv_obj_set_style_text_color(amountValue, lv_color_hex(16090890), LV_PART_MAIN);

    label = lv_label_create(amountContainer);
    lv_label_set_text(label, "Fee");
    lv_obj_align_to(label, amountValue, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    SetTitleLabelStyle(label);

    lv_obj_t *feeValue = lv_label_create(amountContainer);
    lv_label_set_text(feeValue, overviewData->fee_amount);
    lv_obj_align_to(feeValue, label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

    lv_obj_set_style_text_font(feeValue, &openSansEnIllustrate, LV_PART_MAIN);
    lv_obj_set_style_text_color(feeValue, lv_color_hex(0xf55831), LV_PART_MAIN);

    lv_obj_t *switchIcon = GuiCreateImg(amountContainer, &imgConversion);
    lv_obj_align(switchIcon, LV_ALIGN_RIGHT_MID, -24, 0);
    lv_obj_add_flag(switchIcon, LV_OBJ_FLAG_CLICKABLE);

    clickParam.amountValue = amountValue;
    clickParam.feeValue = feeValue;
    clickParam.overviewData = overviewData;
    clickParam.isSat = &isSat;
    isSat = false;
    lv_obj_add_event_cb(switchIcon, SwitchValueUnit, LV_EVENT_CLICKED, &clickParam);

    return amountContainer;
}

static lv_obj_t *CreateNetworkView(lv_obj_t *parent, char *network, lv_obj_t *lastView)
{
    lv_obj_t *networkContainer = GuiCreateContainerWithParent(parent, 408, 62);
    if (lastView == NULL) {
        lv_obj_align(networkContainer, LV_ALIGN_DEFAULT, 0, 0);
    } else {
        lv_obj_align_to(networkContainer, lastView, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    }
    SetContainerDefaultStyle(networkContainer);

    lv_obj_t *label = lv_label_create(networkContainer);
    lv_label_set_text(label, "Network");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    SetTitleLabelStyle(label);

    lv_obj_t *networkValue = lv_label_create(networkContainer);
    lv_label_set_text(networkValue, network);
    lv_obj_align_to(networkValue, label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    SetContentLableStyle(networkValue);
    return networkContainer;
}

static lv_obj_t *CreateOverviewFromView(lv_obj_t *parent, DisplayTxOverview *overviewData, lv_obj_t *lastView)
{
    lv_obj_t *formContainer = GuiCreateContainerWithParent(parent, 408, 0);
    SetContainerDefaultStyle(formContainer);
    lv_obj_align_to(formContainer, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    lv_obj_t * label = lv_label_create(formContainer);
    lv_label_set_text(label, "From");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    SetTitleLabelStyle(label);

    lastView = label;

    PtrT_VecFFI_DisplayTxOverviewInput from = overviewData->from;

    lv_obj_t *formInnerContainer;
    int fromContainerHeight = 78;
    bool hasMultiFromAddress = (from->size > 1);
    lv_obj_t *orderLabel;
    for (int i = 0; i < from->size; i++) {
        formInnerContainer = GuiCreateContainerWithParent(formContainer, 360, 0);
        lv_obj_set_style_bg_opa(formInnerContainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        if (hasMultiFromAddress) {
            orderLabel = lv_label_create(formInnerContainer);
            char str[4] = {0};
            sprintf(str, "%d", (i + 1));
            lv_label_set_text(orderLabel, str);
            lv_obj_align(orderLabel, LV_ALIGN_DEFAULT, 0, 0);
            SetTitleLabelStyle(orderLabel);
        }

        lv_obj_t *addressLabel = lv_label_create(formInnerContainer);
        lv_obj_set_width(addressLabel, 332);
        lv_label_set_long_mode(addressLabel, LV_LABEL_LONG_WRAP);
        lv_label_set_text(addressLabel, from->data[i].address);
        SetContentLableStyle(addressLabel);

        if (hasMultiFromAddress) {
            lv_obj_align_to(addressLabel, orderLabel, LV_ALIGN_OUT_RIGHT_TOP, 16, 0);
        } else {
            lv_obj_align(addressLabel, LV_ALIGN_DEFAULT, 0, 0);
            lv_obj_update_layout(addressLabel);
        }

        int addressLabelHeight =  lv_obj_get_y2(addressLabel);

        lv_obj_set_height(formInnerContainer, addressLabelHeight);

        lv_obj_align_to(formInnerContainer, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

        fromContainerHeight += (addressLabelHeight + 8);
        lastView = formInnerContainer;
    }

    lv_obj_set_height(formContainer, fromContainerHeight);

    return formContainer;
}

static lv_obj_t *CreateOverviewToView(lv_obj_t *parent, DisplayTxOverview *overviewData, lv_obj_t *lastView)
{
    lv_obj_t *toContainer = GuiCreateContainerWithParent(parent, 408, 62);
    SetContainerDefaultStyle(toContainer);
    lv_obj_align_to(toContainer, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    lv_obj_t *label = lv_label_create(toContainer);
    lv_label_set_text(label, "To");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    SetTitleLabelStyle(label);

    lastView = label;

    PtrT_VecFFI_DisplayTxOverviewOutput to = overviewData->to;

    lv_obj_t *toInnerContainer;
    int toContainerHeight = 78;
    bool hasMultiToAddress = (to->size > 1);
    lv_obj_t *toOrderLabel;

    for (int i = 0; i < to->size; i++) {
        toInnerContainer = GuiCreateContainerWithParent(toContainer, 360, 0);
        lv_obj_set_style_bg_opa(toInnerContainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        if (hasMultiToAddress) {
            toOrderLabel = lv_label_create(toInnerContainer);
            char str[4] = {0};
            sprintf(str, "%d", (i + 1));
            lv_label_set_text(toOrderLabel, str);
            lv_obj_align(toOrderLabel, LV_ALIGN_DEFAULT, 0, 0);
            SetTitleLabelStyle(toOrderLabel);
        }

        lv_obj_t *addressLabel = lv_label_create(toInnerContainer);
        lv_obj_set_width(addressLabel, 332);
        lv_label_set_long_mode(addressLabel, LV_LABEL_LONG_WRAP);
        lv_label_set_text(addressLabel, to->data[i].address);
        SetContentLableStyle(addressLabel);

        if (hasMultiToAddress) {
            lv_obj_align_to(addressLabel, toOrderLabel, LV_ALIGN_OUT_RIGHT_TOP, 16, 0);
        } else {
            lv_obj_align(addressLabel, LV_ALIGN_DEFAULT, 0, 0);
            lv_obj_update_layout(addressLabel);
        }

        int addressLabelHeight =  lv_obj_get_y2(addressLabel);

        lv_obj_set_height(toInnerContainer, addressLabelHeight);

        lv_obj_align_to(toInnerContainer, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

        toContainerHeight += (addressLabelHeight + 8);
        lastView = toInnerContainer;
    }

    lv_obj_set_height(toContainer, toContainerHeight);

    return toContainer;

}

static lv_obj_t *CreateDetailAmountView(lv_obj_t *parent, DisplayTxDetail *detailData, lv_obj_t *lastView)
{
    lv_obj_t *amountContainer = GuiCreateContainerWithParent(parent, 408, 138);
    lv_obj_align_to(amountContainer, lastView, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    SetContainerDefaultStyle(amountContainer);

    lv_obj_t *label = lv_label_create(amountContainer);
    lv_label_set_text(label, "Input Value");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    SetTitleLabelStyle(label);

    lv_obj_t *inputValue = lv_label_create(amountContainer);
    lv_label_set_text(inputValue, detailData->total_input_amount);
    lv_obj_align_to(inputValue, label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    SetContentLableStyle(inputValue);

    lastView = label;
    label = lv_label_create(amountContainer);
    lv_label_set_text(label, "Output Value");
    lv_obj_align_to(label, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    SetTitleLabelStyle(label);

    lv_obj_t *outputValue = lv_label_create(amountContainer);
    lv_label_set_text(outputValue, detailData->total_output_amount);
    lv_obj_align_to(outputValue, label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    SetContentLableStyle(outputValue);

    lastView = label;
    label = lv_label_create(amountContainer);
    lv_label_set_text(label, "Fee");
    lv_obj_align_to(label, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    SetTitleLabelStyle(label);

    lv_obj_t *feeValue = lv_label_create(amountContainer);
    lv_label_set_text(feeValue, detailData->fee_amount);
    lv_obj_align_to(feeValue, label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    SetContentLableStyle(feeValue);

    return amountContainer;
}

static lv_obj_t *CreateDetailFromView(lv_obj_t *parent, DisplayTxDetail *detailData, lv_obj_t *lastView)
{
    lv_obj_t *formContainer = GuiCreateContainerWithParent(parent, 408, 0);
    SetContainerDefaultStyle(formContainer);
    lv_obj_align_to(formContainer, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    lv_obj_t * label = lv_label_create(formContainer);
    lv_label_set_text(label, "From");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    SetTitleLabelStyle(label);

    lastView = label;

    PtrT_VecFFI_DisplayTxDetailInput from = detailData->from;

    lv_obj_t *formInnerContainer;
    int fromContainerHeight = 78;
    lv_obj_t *orderLabel;
    for (int i = 0; i < from->size; i++) {
        formInnerContainer = GuiCreateContainerWithParent(formContainer, 360, 0);
        lv_obj_set_style_bg_opa(formInnerContainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        orderLabel = lv_label_create(formInnerContainer);
        char str[4] = {0};
        sprintf(str, "%d", (i + 1));
        lv_label_set_text(orderLabel, str);
        lv_obj_align(orderLabel, LV_ALIGN_DEFAULT, 0, 0);
        SetTitleLabelStyle(orderLabel);

        lv_obj_t *valueLabel = lv_label_create(formInnerContainer);
        lv_obj_set_width(valueLabel, 332);
        lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_WRAP);
        lv_label_set_text(valueLabel, from->data[i].amount);
        lv_obj_set_style_text_font(valueLabel, &openSansEnIllustrate, LV_PART_MAIN);
        lv_obj_set_style_text_color(valueLabel, lv_color_hex(0xf5870a), LV_PART_MAIN);
        lv_obj_align_to(valueLabel, orderLabel, LV_ALIGN_OUT_RIGHT_TOP, 16, 0);

        lv_obj_t *addressLabel = lv_label_create(formInnerContainer);
        lv_obj_set_width(addressLabel, 332);
        lv_label_set_long_mode(addressLabel, LV_LABEL_LONG_WRAP);
        lv_label_set_text(addressLabel, from->data[i].address);
        SetContentLableStyle(addressLabel);
        lv_obj_align_to(addressLabel, orderLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
        lv_obj_update_layout(addressLabel);

        lv_obj_t *pathLabel = lv_label_create(formInnerContainer);
        lv_obj_set_width(pathLabel, 332);
        lv_label_set_long_mode(pathLabel, LV_LABEL_LONG_WRAP);
        lv_label_set_text(pathLabel, from->data[i].path);
        SetTitleLabelStyle(pathLabel);
        lv_obj_align_to(pathLabel, addressLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
        lv_obj_update_layout(pathLabel);

        int pathLabelBottom =  lv_obj_get_y2(pathLabel);

        lv_obj_set_height(formInnerContainer, pathLabelBottom);

        lv_obj_align_to(formInnerContainer, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

        fromContainerHeight += (pathLabelBottom + 8);
        lastView = formInnerContainer;
    }

    lv_obj_set_height(formContainer, fromContainerHeight);

    return formContainer;
}

static lv_obj_t *CreateDetailToView(lv_obj_t *parent, DisplayTxDetail *detailData, lv_obj_t *lastView)
{
    lv_obj_t *toContainer = GuiCreateContainerWithParent(parent, 408, 0);
    SetContainerDefaultStyle(toContainer);
    lv_obj_align_to(toContainer, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    lv_obj_t * label = lv_label_create(toContainer);
    lv_label_set_text(label, "To");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    SetTitleLabelStyle(label);

    lastView = label;

    PtrT_VecFFI_DisplayTxDetailOutput to = detailData->to;

    lv_obj_t *toInnerContainer;
    int toContainerHeight = 78;
    lv_obj_t *orderLabel;
    for (int i = 0; i < to->size; i++) {
        toInnerContainer = GuiCreateContainerWithParent(toContainer, 360, 0);
        lv_obj_set_style_bg_opa(toInnerContainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        orderLabel = lv_label_create(toInnerContainer);
        char str[4] = {0};
        sprintf(str, "%d", (i + 1));
        lv_label_set_text(orderLabel, str);
        lv_obj_align(orderLabel, LV_ALIGN_DEFAULT, 0, 0);
        SetTitleLabelStyle(orderLabel);

        lv_obj_t *valueLabel = lv_label_create(toInnerContainer);
        lv_label_set_text(valueLabel, to->data[i].amount);
        lv_obj_set_style_text_font(valueLabel, &openSansEnIllustrate, LV_PART_MAIN);
        lv_obj_set_style_text_color(valueLabel, lv_color_hex(0xf5870a), LV_PART_MAIN);
        lv_obj_align_to(valueLabel, orderLabel, LV_ALIGN_OUT_RIGHT_TOP, 16, 0);

        if (to->data[i].is_mine) {
            lv_obj_t *changeContainer = GuiCreateContainerWithParent(toInnerContainer, 87, 30);
            lv_obj_set_style_radius(changeContainer, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(changeContainer, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(changeContainer, 30, LV_PART_MAIN | LV_STATE_DEFAULT);

            lv_obj_t *changeLabel = lv_label_create(changeContainer);
            lv_label_set_text(changeLabel, "Change");
            lv_obj_set_style_text_font(changeLabel, g_defIllustrateFont, LV_PART_MAIN);
            lv_obj_set_style_text_color(changeLabel, WHITE_COLOR, LV_PART_MAIN);
            lv_obj_set_style_text_opa(changeLabel, 163, LV_PART_MAIN);
            lv_obj_align(changeLabel, LV_ALIGN_CENTER, 0, 0);

            lv_obj_align_to(changeContainer, valueLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

        }

        lv_obj_t *addressLabel = lv_label_create(toInnerContainer);
        lv_obj_set_width(addressLabel, 332);
        lv_label_set_long_mode(addressLabel, LV_LABEL_LONG_WRAP);
        lv_label_set_text(addressLabel, to->data[i].address);
        SetContentLableStyle(addressLabel);
        lv_obj_align_to(addressLabel, orderLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
        lv_obj_update_layout(addressLabel);

        int bottom =  lv_obj_get_y2(addressLabel);

        lv_obj_set_height(toInnerContainer, bottom);
        lv_obj_align_to(toInnerContainer, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

        toContainerHeight += (bottom + 8);
        lastView = toInnerContainer;
    }

    lv_obj_set_height(toContainer, toContainerHeight);

    return toContainer;
}

void GuiBtcTxOverview(lv_obj_t *parent, void *totalData)
{
    DisplayTx *txData = (DisplayTx*)totalData;
    DisplayTxOverview *overviewData  = txData->overview;

    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *lastView = NULL;

    if (IsMultiSigTx(txData)) {
        lastView = CreateSignStatusView(parent, overviewData->sign_status);
    }
    lastView = CreateOverviewAmountView(parent, overviewData, lastView);
    lastView = CreateNetworkView(parent, overviewData->network, lastView);
    lastView = CreateOverviewFromView(parent, overviewData, lastView);
    CreateOverviewToView(parent, overviewData, lastView);
}

void GuiBtcTxDetail(lv_obj_t *parent, void *totalData)
{

    DisplayTx *txData = (DisplayTx*)totalData;
    DisplayTxDetail *detailData  = txData->detail;

    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *lastView = NULL;
    if (IsMultiSigTx(txData)) {
        lastView = CreateSignStatusView(parent, detailData->sign_status);
    }
    lastView = CreateNetworkView(parent, detailData->network, lastView);
    lastView = CreateDetailAmountView(parent, detailData, lastView);
    lastView = CreateDetailFromView(parent, detailData, lastView);
    CreateDetailToView(parent, detailData, lastView);
}
