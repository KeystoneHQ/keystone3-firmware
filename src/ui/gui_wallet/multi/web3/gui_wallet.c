#include "stdio.h"
#include "gui_wallet.h"
#include "keystore.h"
#include "account_public_info.h"
#include "gui_connect_wallet_widgets.h"
#include "version.h"
#include "user_memory.h"
#include "gui_chain.h"
#include "presetting.h"
#include "version.h"

typedef struct {
    char *path;
    ChainType chainType;
} ChainPath_t;

typedef struct {
    int count;
    int allocatedStart;
    int allocatedEnd;
} PathAddResult_t;

static int AddBTCPathsStandard(ExtendedPublicKey *keys, int startIndex, bool includeTaproot)
{
    int index = startIndex;
    keys[index].path = "m/84'/0'/0'";
    keys[index].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
    index++;

    keys[index].path = "m/49'/0'/0'";
    keys[index].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
    index++;

    keys[index].path = "m/44'/0'/0'";
    keys[index].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);
    index++;

    if (includeTaproot) {
        keys[index].path = "m/86'/0'/0'";
        keys[index].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TAPROOT);
        index++;
    }

    return index - startIndex;
}


static PathAddResult_t AddETHLedgerLivePaths(ExtendedPublicKey *keys, int startIndex)
{
    int index = startIndex;
    for (int i = XPUB_TYPE_ETH_LEDGER_LIVE_0; i <= XPUB_TYPE_ETH_LEDGER_LIVE_9; i++) {
        keys[index].path = SRAM_MALLOC(BUFFER_SIZE_64);
        snprintf_s(keys[index].path, BUFFER_SIZE_64, "m/44'/60'/%d'", i - XPUB_TYPE_ETH_LEDGER_LIVE_0);
        keys[index].xpub = GetCurrentAccountPublicKey(i);
        index++;
    }
    return (PathAddResult_t) {
        .count = index - startIndex,
        .allocatedStart = startIndex,
        .allocatedEnd = index
    };
}

static void FreeAllocatedPaths(ExtendedPublicKey *keys, int startIndex, int endIndex)
{
    for (int i = startIndex; i < endIndex; i++) {
        if (keys[i].path != NULL) {
            SRAM_FREE(keys[i].path);
        }
    }
}

static PtrT_CSliceFFI_ExtendedPublicKey BuildSOLAccountKeys(SOLAccountType accountType, ExtendedPublicKey *keys)
{
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    public_keys->data = keys;

    if (accountType == SOLBip44) {
        public_keys->size = 10;
        for (int i = XPUB_TYPE_SOL_BIP44_0; i <= XPUB_TYPE_SOL_BIP44_9; i++) {
            char *path = SRAM_MALLOC(BUFFER_SIZE_32);
            snprintf_s(path, BUFFER_SIZE_32, "m/44'/501'/%d'", i - XPUB_TYPE_SOL_BIP44_0);
            keys[i - XPUB_TYPE_SOL_BIP44_0].path = path;
            keys[i - XPUB_TYPE_SOL_BIP44_0].xpub = GetCurrentAccountPublicKey(i);
        }
    } else if (accountType == SOLBip44ROOT) {
        public_keys->size = 1;
        char *path = SRAM_MALLOC(BUFFER_SIZE_32);
        snprintf_s(path, BUFFER_SIZE_32, "m/44'/501'");
        keys[0].path = path;
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_SOL_BIP44_ROOT);
    } else if (accountType == SOLBip44Change) {
        public_keys->size = 10;
        for (int i = XPUB_TYPE_SOL_BIP44_CHANGE_0; i <= XPUB_TYPE_SOL_BIP44_CHANGE_9; i++) {
            char *path = SRAM_MALLOC(BUFFER_SIZE_32);
            snprintf_s(path, BUFFER_SIZE_32, "m/44'/501'/%d'/0'", i - XPUB_TYPE_SOL_BIP44_CHANGE_0);
            keys[i - XPUB_TYPE_SOL_BIP44_CHANGE_0].path = path;
            keys[i - XPUB_TYPE_SOL_BIP44_CHANGE_0].xpub = GetCurrentAccountPublicKey(i);
        }
    }
    return public_keys;
}

PtrT_CSliceFFI_ExtendedPublicKey BuildChainPaths(ChainPath_t *chainPaths, ExtendedPublicKey *keys, uint8_t size)
{
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    public_keys->data = keys;
    public_keys->size = size;
    for (uint8_t i = 0; i < size; i++) {
        keys[i].path = chainPaths[i].path;
        keys[i].xpub = GetCurrentAccountPublicKey(chainPaths[i].chainType);
    }
    return public_keys;
}

UREncodeResult *GuiGetStandardBtcData(void)
{
    ChainPath_t chainPaths[] = {
        {.path = "m/84'/0'/0'", .chainType = XPUB_TYPE_BTC_NATIVE_SEGWIT},
        {.path = "m/49'/0'/0'", .chainType = XPUB_TYPE_BTC},
        {.path = "m/44'/0'/0'", .chainType = XPUB_TYPE_BTC_LEGACY},
        {.path = "m/86'/0'/0'", .chainType = XPUB_TYPE_BTC_TAPROOT},
    };
    int length = NUMBER_OF_ARRAYS(chainPaths);
    ExtendedPublicKey keys[length];
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = BuildChainPaths(chainPaths, keys, length);
    UREncodeResult *urEncode = generate_btc_crypto_account_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urEncode);
    SRAM_FREE(public_keys);
    return urEncode;
}

typedef UREncodeResult *MetamaskUrGetter(PtrBytes master_fingerprint, uint32_t master_fingerprint_length, enum ETHAccountType account_type, PtrT_CSliceFFI_ExtendedPublicKey public_keys, PtrString wallet_name);

static UREncodeResult *get_unlimited_connect_metamask_ur(PtrBytes master_fingerprint, uint32_t master_fingerprint_length, enum ETHAccountType account_type, PtrT_CSliceFFI_ExtendedPublicKey public_keys)
{
    return get_connect_metamask_ur_unlimited(master_fingerprint, master_fingerprint_length, account_type, public_keys, GetWalletName());
}

static UREncodeResult *BasicGetMetamaskDataForAccountType(ETHAccountType accountType, MetamaskUrGetter func)
{
    if (func == NULL) {
        return NULL;
    }

    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    if (public_keys == NULL) {
        return NULL;
    }

    ExtendedPublicKey keys[10] = {0};
    public_keys->data = keys;

    switch (accountType) {
    case Bip44Standard:
        public_keys->size = 1;
        keys[0].path = "";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD);
        break;
    case LedgerLive:
        public_keys->size = 10;
        for (int i = XPUB_TYPE_ETH_LEDGER_LIVE_0; i <= XPUB_TYPE_ETH_LEDGER_LIVE_9; i++) {
            keys[i - XPUB_TYPE_ETH_LEDGER_LIVE_0].path = "";
            keys[i - XPUB_TYPE_ETH_LEDGER_LIVE_0].xpub = GetCurrentAccountPublicKey(i);
        }
        break;
    case LedgerLegacy:
        public_keys->size = 1;
        keys[0].path = "";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LEGACY);
        break;
    default:
        SRAM_FREE(public_keys);
        return NULL;
    }

    UREncodeResult *urEncode = func(mfp, sizeof(mfp), accountType, public_keys, GetWalletName());
    if (urEncode == NULL) {
        SRAM_FREE(public_keys);
        return NULL;
    }

    SRAM_FREE(public_keys);
    return urEncode;
}
// copy from gui_btc, need to use real data
UREncodeResult *GetMetamaskDataForAccountType(ETHAccountType accountType)
{
    return BasicGetMetamaskDataForAccountType(accountType, get_connect_metamask_ur);
}

UREncodeResult *GetUnlimitedMetamaskDataForAccountType(ETHAccountType accountType)
{
    return BasicGetMetamaskDataForAccountType(accountType, get_connect_metamask_ur_unlimited);
}

UREncodeResult *GuiGetMetamaskData(void)
{
    ETHAccountType accountType = GetMetamaskAccountType();
    return GetMetamaskDataForAccountType(accountType);
}

UREncodeResult *GuiGetImTokenData(void)
{
    return GetMetamaskDataForAccountType(Bip44Standard);
}

UREncodeResult *GuiGetCoreWalletData(void)
{
    ChainPath_t chainPaths[] = {
        {.path = "m/44'/60'/0'", .chainType = XPUB_TYPE_ETH_BIP44_STANDARD},
        {.path = "m/44'/9000'/0'", .chainType = XPUB_TYPE_AVAX_X_P},
    };
    ExtendedPublicKey keys[NUMBER_OF_ARRAYS(chainPaths)];
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = BuildChainPaths(chainPaths, keys, NUMBER_OF_ARRAYS(chainPaths));
    UREncodeResult *urEncode = get_core_wallet_ur(mfp, sizeof(mfp), public_keys, "Keystone3");
    CHECK_CHAIN_PRINT(urEncode);
    SRAM_FREE(public_keys);
    return urEncode;
}

UREncodeResult *GuiGetWanderData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    char *arXpub = GetCurrentAccountPublicKey(XPUB_TYPE_ARWEAVE);
    if (arXpub == NULL || strlen(arXpub) != 1024) {
        GuiSetupArConnectWallet();
        arXpub = GetCurrentAccountPublicKey(XPUB_TYPE_ARWEAVE);
        ClearSecretCache();
    }
    ASSERT(arXpub != NULL);
    UREncodeResult *urEncode = get_connect_arconnect_wallet_ur_from_xpub(mfp, sizeof(mfp), arXpub);
    CHECK_CHAIN_PRINT(urEncode);
    return urEncode;
}

UREncodeResult *GuiGetWalletDataByCoin(bool onlySui)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    PtrT_CSliceFFI_ExtendedPublicKey publicKeys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
#define SUI_XPUB_COUNT 10
#define IOTA_XPUB_COUNT 10
#define APT_XPUB_COUNT 10
#define NIGHTLY_XPUB_COUNT (SUI_XPUB_COUNT + IOTA_XPUB_COUNT + APT_XPUB_COUNT)
    ExtendedPublicKey keys[NIGHTLY_XPUB_COUNT];
    publicKeys->data = keys;
    uint8_t xpubIndex = 0;
    for (xpubIndex = 0; xpubIndex < SUI_XPUB_COUNT; xpubIndex++) {
        keys[xpubIndex].path = SRAM_MALLOC(BUFFER_SIZE_32);
        snprintf_s(keys[xpubIndex].path, BUFFER_SIZE_32, "m/44'/784'/%u'/0'/0'", xpubIndex);
        keys[xpubIndex].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_SUI_0 + xpubIndex);
    }
    if (!onlySui) {
        for (uint8_t startIndex = 0; startIndex < IOTA_XPUB_COUNT; xpubIndex++, startIndex++) {
            keys[xpubIndex].path = SRAM_MALLOC(BUFFER_SIZE_32);
            snprintf_s(keys[xpubIndex].path, BUFFER_SIZE_32, "m/44'/4218'/%u'/0'/0'", startIndex);
            keys[xpubIndex].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_IOTA_0 + startIndex);
        }
        for (uint8_t startIndex = 0; startIndex < APT_XPUB_COUNT; xpubIndex++, startIndex++) {
            keys[xpubIndex].path = SRAM_MALLOC(BUFFER_SIZE_32);
            snprintf_s(keys[xpubIndex].path, BUFFER_SIZE_32, "m/44'/637'/%u'/0'/0'", startIndex);
            keys[xpubIndex].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_APT_0 + startIndex);
        }
        publicKeys->size = NIGHTLY_XPUB_COUNT;
    } else {
        publicKeys->size = SUI_XPUB_COUNT;
    }

    UREncodeResult *urEncode = generate_common_crypto_multi_accounts_ur(mfp, sizeof(mfp), publicKeys, "SUI");
    CHECK_CHAIN_PRINT(urEncode);
    FreeAllocatedPaths(keys, 0, publicKeys->size);
    SRAM_FREE(publicKeys);
    return urEncode;
}

UREncodeResult *GuiGetIotaWalletData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey publicKeys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
#define IOTA_XPUB_COUNT 10
    ExtendedPublicKey keys[IOTA_XPUB_COUNT];
    publicKeys->data = keys;
    publicKeys->size = IOTA_XPUB_COUNT;
    for (uint8_t startIndex = 0; startIndex < IOTA_XPUB_COUNT; startIndex++) {
        keys[startIndex].path = SRAM_MALLOC(BUFFER_SIZE_32);
        snprintf_s(keys[startIndex].path, BUFFER_SIZE_32, "m/44'/4218'/%u'/0'/0'", startIndex);
        keys[startIndex].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_IOTA_0 + startIndex);
    }
    UREncodeResult *urEncode = generate_common_crypto_multi_accounts_ur(mfp, sizeof(mfp), publicKeys, "IOTA");
    CHECK_CHAIN_PRINT(urEncode);
    FreeAllocatedPaths(keys, 0, publicKeys->size);
    SRAM_FREE(publicKeys);
    return urEncode;
}

UREncodeResult *GuiGetFewchaDataByCoin(GuiChainCoinType coin)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey publicKeys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[10];
    publicKeys->data = keys;
    publicKeys->size = 10;
    int16_t coinType = 0;
    int16_t xpubBaseIndex = 0;
    switch (coin) {
    case CHAIN_SUI:
        coinType = 784;
        xpubBaseIndex = XPUB_TYPE_SUI_0;
        break;
    case CHAIN_APT:
        coinType = 637;
        xpubBaseIndex = XPUB_TYPE_APT_0;
        break;
    default:
        printf("invalid coin type\r\n");
        return NULL;
    }
    for (uint8_t i = 0; i < 10; i++) {
        keys[i].path = SRAM_MALLOC(BUFFER_SIZE_32);
        snprintf_s(keys[i].path, BUFFER_SIZE_32, "m/44'/%u'/%u'/0'/0'", coinType, i);
        keys[i].xpub = GetCurrentAccountPublicKey(xpubBaseIndex + i);
    }
    UREncodeResult *urEncode = generate_common_crypto_multi_accounts_ur(mfp, sizeof(mfp), publicKeys, (coin == CHAIN_SUI) ? "SUI" : "APT");
    CHECK_CHAIN_PRINT(urEncode);
    FreeAllocatedPaths(keys, 0, publicKeys->size);
    SRAM_FREE(publicKeys);
    return urEncode;
}

UREncodeResult *GuiGetPetraData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    ExtendedPublicKey keys[10];
    PtrT_CSliceFFI_ExtendedPublicKey publicKeys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    publicKeys->data = keys;
    publicKeys->size = 10;
    for (uint8_t i = 0; i < 10; i++) {
        keys[i].path = SRAM_MALLOC(BUFFER_SIZE_32);
        snprintf_s(keys[i].path, BUFFER_SIZE_32, "m/44'/637'/%u'/0'/0'", i);
        keys[i].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_APT_0 + i);
    }
    UREncodeResult *urEncode = generate_common_crypto_multi_accounts_ur(mfp, sizeof(mfp), publicKeys, "APT");
    CHECK_CHAIN_PRINT(urEncode);
    FreeAllocatedPaths(keys, 0, publicKeys->size);
    SRAM_FREE(publicKeys);
    return urEncode;
}

UREncodeResult *GuiGetADADataByIndex(char *walletName)
{
    uint32_t index = GetConnectWalletAccountIndex(walletName);
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    char* xpub = GetCurrentAccountPublicKey(GetAdaXPubTypeByIndexAndDerivationType(
            GetConnectWalletPathIndex(walletName), index));
    char path[BUFFER_SIZE_32] = {0};
    sprintf(path, "1852'/1815'/%u'", index);
    ExtendedPublicKey xpubs[1];
    xpubs[0].path = path;
    xpubs[0].xpub = xpub;
    CSliceFFI_ExtendedPublicKey keys;
    keys.data = xpubs;
    keys.size = 1;
    char firmwareVersion[BUFFER_SIZE_32];
    GetSoftWareVersionNumber(firmwareVersion);
    return generate_key_derivation_ur(mfp, 4, &keys, firmwareVersion);
}
UREncodeResult *GuiGetKeplrDataByIndex(uint32_t index)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_KeplrAccount publicKeys = SRAM_MALLOC(sizeof(CSliceFFI_KeplrAccount));
    GuiChainCoinType chains[8] = {
        CHAIN_ATOM,
        CHAIN_SCRT,
        CHAIN_CRO,
        CHAIN_IOV,
        CHAIN_BLD,
        CHAIN_KAVA,
        CHAIN_EVMOS,
        CHAIN_LUNA,
    };
    KeplrAccount keys[8];
    publicKeys->data = keys;
    publicKeys->size = 8;

    for (uint8_t i = 0; i < 8; i++) {
        const CosmosChain_t *chain = GuiGetCosmosChain(chains[i]);
        keys[i].xpub = GetCurrentAccountPublicKey(chain->xpubType);
        keys[i].name = "Account-1";
        keys[i].path = SRAM_MALLOC(BUFFER_SIZE_32);
        snprintf_s(keys[i].path, BUFFER_SIZE_32, "M/44'/%u'/0'/0/%u", chain->coinType, index);
    }

    UREncodeResult *urEncode = get_connect_keplr_wallet_ur(mfp, sizeof(mfp), publicKeys);
    CHECK_CHAIN_PRINT(urEncode);
    for (uint8_t i = 0; i < 8; i++) {
        SRAM_FREE(keys[i].path);
    }
    SRAM_FREE(publicKeys);
    return urEncode;
}

UREncodeResult *GuiGetLeapData(void)
{
#define CHAIN_AMOUNT 4
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_KeplrAccount publicKeys = SRAM_MALLOC(sizeof(CSliceFFI_KeplrAccount));
    GuiChainCoinType chains[CHAIN_AMOUNT] = {
        CHAIN_ATOM,
        CHAIN_EVMOS,
        CHAIN_RUNE,
        CHAIN_SCRT
    };
    KeplrAccount keys[CHAIN_AMOUNT];
    publicKeys->data = keys;
    publicKeys->size = CHAIN_AMOUNT;

    for (uint8_t i = 0; i < CHAIN_AMOUNT; i++) {
        const CosmosChain_t *chain = GuiGetCosmosChain(chains[i]);
        keys[i].xpub = GetCurrentAccountPublicKey(chain->xpubType);
        keys[i].name = "Account-1";
        keys[i].path = SRAM_MALLOC(BUFFER_SIZE_32);
        snprintf_s(keys[i].path, BUFFER_SIZE_32, "M/44'/%u'/0'/0/0", chain->coinType);
    }

    UREncodeResult *urEncode = get_connect_keplr_wallet_ur(mfp, sizeof(mfp), publicKeys);
    CHECK_CHAIN_PRINT(urEncode);
    for (uint8_t i = 0; i < CHAIN_AMOUNT; i++) {
        SRAM_FREE(keys[i].path);
    }
    SRAM_FREE(publicKeys);
    return urEncode;
}

UREncodeResult *GuiGetXrpToolkitDataByIndex(uint16_t index)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    char *xpub = GetCurrentAccountPublicKey(XPUB_TYPE_XRP);
    char *rootPath = "m/44'/144'/0'";
    char hdPath[BUFFER_SIZE_32] = {0};
    snprintf_s(hdPath, BUFFER_SIZE_32, "%s/0/%u", rootPath, index);
    UREncodeResult *urEncode = get_connect_xrp_toolkit_ur(hdPath, xpub, rootPath);
    CHECK_CHAIN_PRINT(urEncode);
    return urEncode;
}

UREncodeResult *GuiGetKeystoneConnectWalletData(void)
{
    ChainPath_t chainPaths[] = {
        {.path = "m/44'/60'/0'", .chainType = XPUB_TYPE_ETH_BIP44_STANDARD},
        {.path = "m/84'/0'/0'", .chainType = XPUB_TYPE_BTC_NATIVE_SEGWIT},
        {.path = "m/49'/0'/0'", .chainType = XPUB_TYPE_BTC},
        {.path = "m/44'/0'/0'", .chainType = XPUB_TYPE_BTC_LEGACY},
        {.path = "m/86'/0'/0'", .chainType = XPUB_TYPE_BTC_TAPROOT},
        {.path = GetXPubPath(XPUB_TYPE_TRX), .chainType = XPUB_TYPE_TRX},
        {.path = GetXPubPath(XPUB_TYPE_DOGE), .chainType = XPUB_TYPE_DOGE},
        {.path = GetXPubPath(XPUB_TYPE_XRP), .chainType = XPUB_TYPE_XRP},
    };
    ExtendedPublicKey keys[NUMBER_OF_ARRAYS(chainPaths)];
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = BuildChainPaths(chainPaths, keys, NUMBER_OF_ARRAYS(chainPaths));
    char serialNumber[256];
    GetSerialNumber(serialNumber);
    char firmwareVersion[12];
    GetSoftWareVersionNumber(firmwareVersion);
    UREncodeResult *urEncode = get_keystone_connect_wallet_ur(mfp, sizeof(mfp), serialNumber, public_keys, "Keystone 3 Pro", firmwareVersion);
    CHECK_CHAIN_PRINT(urEncode);
    SRAM_FREE(public_keys);
    return urEncode;
}

UREncodeResult *GuiGetBitgetWalletData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    //   btc 4
    // + eth 10
    ExtendedPublicKey keys[15];
    public_keys->data = keys;
    public_keys->size = 15;

    int index = 0;
    PathAddResult_t ethResult = AddETHLedgerLivePaths(keys, index);
    index += ethResult.count;

    index += AddBTCPathsStandard(keys, index, true);

    keys[index].path = GetXPubPath(XPUB_TYPE_TON_BIP39);
    keys[index].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_TON_BIP39);

    char serialNumber[256];
    GetSerialNumber(serialNumber);
    char firmwareVersion[BUFFER_SIZE_32];
    GetSoftWareVersionNumber(firmwareVersion);
    UREncodeResult *urEncode = get_bitget_wallet_ur(mfp, sizeof(mfp), serialNumber, public_keys, "Keystone 3 Pro", firmwareVersion);
    CHECK_CHAIN_PRINT(urEncode);

    if (ethResult.allocatedStart >= 0) {
        FreeAllocatedPaths(keys, ethResult.allocatedStart, ethResult.allocatedEnd);
    }
    SRAM_FREE(public_keys);
    return urEncode;
}

UREncodeResult *GuiGetOkxWalletData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    //   btc 4
    // + eth 10
    // + trx 1
    // + ltc 1
    // + dash 1
    // + bch 1
    ExtendedPublicKey keys[18];
    public_keys->data = keys;
    public_keys->size = 18;

    int index = 0;
    PathAddResult_t ethResult = AddETHLedgerLivePaths(keys, index);
    index += ethResult.count;

    index += AddBTCPathsStandard(keys, index, false);

    keys[index].path = GetXPubPath(XPUB_TYPE_TRX);
    keys[index].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);
    index++;

    keys[index].path = GetXPubPath(XPUB_TYPE_LTC);
    keys[index].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_LTC);
    index++;

    keys[index].path = GetXPubPath(XPUB_TYPE_DASH);
    keys[index].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_DASH);
    index++;

    keys[index].path = GetXPubPath(XPUB_TYPE_BCH);
    keys[index].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BCH);
    index++;

    keys[index].path = "m/86'/0'/0'";
    keys[index].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TAPROOT);

    char serialNumber[256];
    GetSerialNumber(serialNumber);
    char firmwareVersion[BUFFER_SIZE_32];
    GetSoftWareVersionNumber(firmwareVersion);
    UREncodeResult *urEncode = get_okx_wallet_ur(mfp, sizeof(mfp), serialNumber, public_keys, "Keystone 3 Pro", firmwareVersion);
    CHECK_CHAIN_PRINT(urEncode);

    if (ethResult.allocatedStart >= 0) {
        FreeAllocatedPaths(keys, ethResult.allocatedStart, ethResult.allocatedEnd);
    }
    SRAM_FREE(public_keys);
    return urEncode;
}

UREncodeResult *GuiGetSolflareData(void)
{
    SOLAccountType accountType = GetSolflareAccountType();
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    ExtendedPublicKey keys[10];
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = BuildSOLAccountKeys(accountType, keys);
    UREncodeResult *urEncode = generate_common_crypto_multi_accounts_ur(mfp, sizeof(mfp), public_keys, "SOL");
    CHECK_CHAIN_PRINT(urEncode);
    for (int i = 0; i < public_keys->size; i++) {
        SRAM_FREE(public_keys->data[i].path);
    }
    SRAM_FREE(public_keys);
    return urEncode;
}

UREncodeResult *GuiGetHeliumData(void)
{
    SOLAccountType accountType = GetHeliumAccountType();
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    ExtendedPublicKey keys[10];
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = BuildSOLAccountKeys(accountType, keys);
    UREncodeResult *urEncode = generate_common_crypto_multi_accounts_ur(mfp, sizeof(mfp), public_keys, "SOL");
    CHECK_CHAIN_PRINT(urEncode);
    FreeAllocatedPaths(keys, 0, public_keys->size);
    SRAM_FREE(public_keys);
    return urEncode;
}

UREncodeResult *GuiGetXBullData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[5];
    public_keys->data = keys;
    public_keys->size = 5;
    for (int i = XPUB_TYPE_STELLAR_0; i <= XPUB_TYPE_STELLAR_4; i++) {
        char *path = SRAM_MALLOC(BUFFER_SIZE_32);
        snprintf_s(path, BUFFER_SIZE_32, "m/44'/148'/%d'", i - XPUB_TYPE_STELLAR_0);
        keys[i - XPUB_TYPE_STELLAR_0].path = path;
        keys[i - XPUB_TYPE_STELLAR_0].xpub = GetCurrentAccountPublicKey(i);
    }
    UREncodeResult *urEncode = generate_common_crypto_multi_accounts_ur(mfp, sizeof(mfp), public_keys, "STELLAR");
    CHECK_CHAIN_PRINT(urEncode);
    for (int i = 0; i < public_keys->size; i++) {
        SRAM_FREE(public_keys->data[i].path);
    }
    SRAM_FREE(public_keys);
    return urEncode;
}

UREncodeResult *GuiGetBackpackData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[41];
    public_keys->data = keys;
    public_keys->size = 41;

    uint8_t count = 0;
    // SOLBip44ROOT
    char *solPath = SRAM_MALLOC(sizeof(char) * 32);
    snprintf_s(solPath, BUFFER_SIZE_32, "m/44'/501'");
    keys[count].path = solPath;
    keys[count].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_SOL_BIP44_ROOT);
    count++;

    // SOLBip44
    for (uint8_t i = XPUB_TYPE_SOL_BIP44_0; i <= XPUB_TYPE_SOL_BIP44_9; i++, count++) {
        char *path = SRAM_MALLOC(BUFFER_SIZE_32);
        snprintf_s(path, BUFFER_SIZE_32, "m/44'/501'/%d'", i - XPUB_TYPE_SOL_BIP44_0);
        keys[count].path = path;
        keys[count].xpub = GetCurrentAccountPublicKey(i);
    }

    // SOLBip44Change
    for (uint8_t i = XPUB_TYPE_SOL_BIP44_CHANGE_0; i <= XPUB_TYPE_SOL_BIP44_CHANGE_9; i++, count++) {
        char *path = SRAM_MALLOC(sizeof(char) * 32);
        snprintf_s(path, BUFFER_SIZE_32, "m/44'/501'/%d'/0'", i - XPUB_TYPE_SOL_BIP44_CHANGE_0);
        keys[count].path = path;
        keys[count].xpub = GetCurrentAccountPublicKey(i);
    }

    // ETH - Bip44Standard + LedgerLive
    for (uint8_t i = XPUB_TYPE_ETH_LEDGER_LIVE_0; i <= XPUB_TYPE_ETH_LEDGER_LIVE_9; i++, count++) {
        char *path = SRAM_MALLOC(sizeof(char) * 32);
        snprintf_s(path, BUFFER_SIZE_32, "m/44'/60'/%d'", i - XPUB_TYPE_ETH_LEDGER_LIVE_0);
        keys[count].path = path;
        keys[count].xpub = GetCurrentAccountPublicKey(i);
    }

    // SUI same with other sui wallet - return 10 keys
    for (uint8_t i = XPUB_TYPE_SUI_0; i <= XPUB_TYPE_SUI_9; i++, count++) {
        char *path = SRAM_MALLOC(sizeof(char) * 32);
        snprintf_s(path, BUFFER_SIZE_32, "m/44'/784'/%u'/0'/0'", i - XPUB_TYPE_SUI_0);
        keys[count].path = path;
        keys[count].xpub = GetCurrentAccountPublicKey(i);
    }
    UREncodeResult *urEncode = get_backpack_wallet_ur(mfp, sizeof(mfp), public_keys);

    CHECK_CHAIN_PRINT(urEncode);
    for (int i = 0; i < public_keys->size; i++) {
        SRAM_FREE(public_keys->data[i].path);
    }
    SRAM_FREE(public_keys);
    return urEncode;
}

UREncodeResult *GuiGetThorWalletData(void)
{
    ChainPath_t chainPaths[] = {
        {.path = "m/84'/0'/0'", .chainType = XPUB_TYPE_BTC_NATIVE_SEGWIT},
        {.path = "m/49'/0'/0'", .chainType = XPUB_TYPE_BTC},
        {.path = "m/44'/0'/0'", .chainType = XPUB_TYPE_BTC_LEGACY},
        {.path = "m/44'/931'/0'", .chainType = XPUB_TYPE_THOR},
        {.path = "m/44'/60'/0'", .chainType = XPUB_TYPE_ETH_BIP44_STANDARD},
    };
    ExtendedPublicKey keys[NUMBER_OF_ARRAYS(chainPaths)];
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = BuildChainPaths(chainPaths, keys, NUMBER_OF_ARRAYS(chainPaths));
    char serialNumber[256];
    GetSerialNumber(serialNumber);
    char firmwareVersion[BUFFER_SIZE_32];
    GetSoftWareVersionNumber(firmwareVersion);
    UREncodeResult *urEncode = get_connect_thor_wallet_ur(mfp, sizeof(mfp), serialNumber, public_keys,
                               "Keystone 3 Pro", firmwareVersion);
    CHECK_CHAIN_PRINT(urEncode);
    SRAM_FREE(public_keys);
    return urEncode;
}