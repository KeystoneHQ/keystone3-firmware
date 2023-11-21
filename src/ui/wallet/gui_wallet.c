#include "stdio.h"
#include "gui_wallet.h"
#include "keystore.h"
#include "account_public_info.h"
#include "gui_connect_wallet_widgets.h"
#include "version.h"
#include "user_memory.h"
#include "gui_chain.h"
#include "presetting.h"

static UREncodeResult *g_urEncode = NULL;

#ifdef COMPILE_SIMULATOR
struct UREncodeResult *get_connect_blue_wallet_ur(uint8_t *master_fingerprint,
        uint32_t length,
        PtrT_CSliceFFI_ExtendedPublicKey public_keys)
{
    struct UREncodeResult *result = malloc(sizeof(struct UREncodeResult));
    result->is_multi_part = 0;
    result->data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    result->encoder = NULL;
    result->error_code = 0;
    result->error_message = NULL;
    return result;
}

PtrT_UREncodeResult get_connect_companion_app_ur(PtrBytes master_fingerprint,
        uint32_t master_fingerprint_length,
        int device_info,
        CoinConfig *coin_config,
        uint32_t coin_config_length)
{
    struct UREncodeResult *result = malloc(sizeof(struct UREncodeResult));
    result->is_multi_part = 0;
    result->data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    result->encoder = NULL;
    result->error_code = 0;
    result->error_message = NULL;
    return result;
}

char *GetCurrentAccountPublicKey(ChainType chain)
{
    return "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
}

void GetMasterFingerPrint(uint8_t *mfp)
{
    mfp[0] = 0x70;
    mfp[1] = 0x7e;
    mfp[2] = 0xed;
    mfp[3] = 0x6c;
}
#endif

UREncodeResult *GuiGetBlueWalletBtcData(void)
{
#ifndef COMPILE_SIMULATOR
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
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
    UREncodeResult *urencode = get_connect_blue_wallet_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urencode);
    return urencode;
#else
    const uint8_t *data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    return (void *)data;
#endif
}

typedef UREncodeResult *MetamaskUrGetter(PtrBytes master_fingerprint, uint32_t master_fingerprint_length, enum ETHAccountType account_type, PtrT_CSliceFFI_ExtendedPublicKey public_keys);

static get_unlimited_connect_metamask_ur(PtrBytes master_fingerprint, uint32_t master_fingerprint_length, enum ETHAccountType account_type, PtrT_CSliceFFI_ExtendedPublicKey public_keys)
{
    return get_connect_metamask_ur_unlimited(master_fingerprint, master_fingerprint_length, account_type, public_keys);
}

static UREncodeResult *BasicGetMetamaskDataForAccountType(ETHAccountType accountType, MetamaskUrGetter func)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[10];
    public_keys->data = keys;

    if (accountType == Bip44Standard)
    {
        public_keys->size = 1;
        keys[0].path = "";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD);
    }
    else if (accountType == LedgerLive)
    {
        public_keys->size = 10;
        for (int i = XPUB_TYPE_ETH_LEDGER_LIVE_0; i <= XPUB_TYPE_ETH_LEDGER_LIVE_9; i++)
        {
            keys[i - XPUB_TYPE_ETH_LEDGER_LIVE_0].path = "";
            keys[i - XPUB_TYPE_ETH_LEDGER_LIVE_0].xpub = GetCurrentAccountPublicKey(i);
        }
    }
    else if (accountType == LedgerLegacy)
    {
        public_keys->size = 1;
        keys[0].path = "";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LEGACY);
    }

    g_urEncode = func(mfp, sizeof(mfp), accountType, public_keys);
    CHECK_CHAIN_PRINT(g_urEncode);
    SRAM_FREE(public_keys);
    return g_urEncode;
}

#ifndef COMPILE_SIMULATOR
// copy from gui_btc, need to use real data
UREncodeResult *GetMetamaskDataForAccountType(ETHAccountType accountType)
{
    return BasicGetMetamaskDataForAccountType(accountType, get_connect_metamask_ur);   
}

UREncodeResult *GetUnlimitedMetamaskDataForAccountType(ETHAccountType accountType)
{
    return BasicGetMetamaskDataForAccountType(accountType, get_unlimited_connect_metamask_ur);
}
#endif

UREncodeResult *GuiGetMetamaskData(void)
{
#ifndef COMPILE_SIMULATOR
    ETHAccountType accountType = GetMetamaskAccountType();
    return GetMetamaskDataForAccountType(accountType);
#else
    const uint8_t *data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    return (void *)data;
#endif
}

UREncodeResult *GuiGetImTokenData(void)
{
#ifndef COMPILE_SIMULATOR
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);    
    g_urEncode = get_connect_imtoken_ur(mfp, sizeof(mfp), GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD), GetWalletName());
    CHECK_CHAIN_PRINT(g_urEncode);
    return g_urEncode;
#else
    const uint8_t *data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    return (void *)data;
#endif
}

UREncodeResult *GuiGetKeplrData(void)
{
#ifndef COMPILE_SIMULATOR
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
        keys[i].path = SRAM_MALLOC(sizeof(char) * 32);
        sprintf(keys[i].path, "M/44'/%u'/0'/0/0", chain->coinType);
    }

    g_urEncode = get_connect_keplr_wallet_ur(mfp, sizeof(mfp), publicKeys);
    CHECK_CHAIN_PRINT(g_urEncode);
    for (uint8_t i = 0; i < 8; i++) {
        SRAM_FREE(keys[i].path);
    }
    SRAM_FREE(publicKeys);
    return g_urEncode;
#else
    const uint8_t *data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    return (void *)data;
#endif
}

UREncodeResult *GuiGetFewchaDataByCoin(GuiChainCoinType coin)
{
#ifndef COMPILE_SIMULATOR
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
        return;
    }
    for (uint8_t i = 0; i < 10; i++) {
        keys[i].path = SRAM_MALLOC(sizeof(char) * 32);
        sprintf(keys[i].path, "m/44'/%u'/%u'/0'/0'", coinType, i);
        keys[i].xpub = GetCurrentAccountPublicKey(xpubBaseIndex + i);
    }
    if (coin == CHAIN_SUI) {
        g_urEncode = get_connect_sui_wallet_ur(mfp, sizeof(mfp), publicKeys);
    } else {
        g_urEncode = get_connect_aptos_wallet_ur(mfp, sizeof(mfp), publicKeys);
    }
    CHECK_CHAIN_PRINT(g_urEncode);
    for (uint8_t i = 0; i < 10; i++) {
        SRAM_FREE(keys[i].path);
    }
    SRAM_FREE(publicKeys);
    return g_urEncode;
#else
    const uint8_t *data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    return (void *)data;
#endif
}

UREncodeResult *GuiGetPetraData(void)
{
#ifndef COMPILE_SIMULATOR
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey publicKeys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[10];
    publicKeys->data = keys;
    publicKeys->size = 10;
    for (uint8_t i = 0; i < 10; i++) {
        keys[i].path = SRAM_MALLOC(sizeof(char) * 32);
        sprintf(keys[i].path, "m/44'/637'/%u'/0'/0'", i);
        keys[i].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_APT_0 + i);
    }
    g_urEncode = get_connect_aptos_wallet_ur(mfp, sizeof(mfp), publicKeys);
    CHECK_CHAIN_PRINT(g_urEncode);
    for (uint8_t i = 0; i < 10; i++) {
        SRAM_FREE(keys[i].path);
    }
    SRAM_FREE(publicKeys);
    return g_urEncode;
#else
    const uint8_t *data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    return (void *)data;
#endif
}

UREncodeResult *GuiGetXrpToolkitDataByIndex(uint16_t index)
{
#ifndef COMPILE_SIMULATOR
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    char *xpub = GetCurrentAccountPublicKey(XPUB_TYPE_XRP);
    char *rootPath = "m/44'/144'/0'";
    char hdPath[32] = {0};
    sprintf(hdPath, "%s/0/%u", rootPath, index);
    g_urEncode = get_connect_xrp_toolkit_ur(hdPath, xpub, rootPath);
    CHECK_CHAIN_PRINT(g_urEncode);
    return g_urEncode;
#else
    const uint8_t *data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    return (void *)data;
#endif
}

UREncodeResult *GuiGetCompanionAppData(void)
{
    extern CoinState_t g_companionAppcoinState[COMPANION_APP_COINS_BUTT];

    uint8_t mfp[4] = {0x0, 0x0, 0x0, 0x0};
    GetMasterFingerPrint(mfp);

    // BTC
    char *btcPath = "M/49'/0'/0'";
    char *btcXpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);

    AccountConfig btcAccount;
    btcAccount.hd_path = btcPath;
    btcAccount.x_pub = btcXpub;
    btcAccount.address_length = 20;
    btcAccount.is_multi_sign = false;

    CoinConfig btcCoin;
    btcCoin.is_active = g_companionAppcoinState[BTC].state;
    btcCoin.coin_code = "BTC";
    btcCoin.accounts = &btcAccount;
    btcCoin.accounts_length = 1;

    // BTC LEGACY
    char *btcLegacyPath = "M/44'/0'/0'";
    char *btcLegacyXpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);

    AccountConfig btcLegacyAccount;
    btcLegacyAccount.hd_path = btcLegacyPath;
    btcLegacyAccount.x_pub = btcLegacyXpub;
    btcLegacyAccount.address_length = 20;
    btcLegacyAccount.is_multi_sign = false;

    CoinConfig btcLegacyCoin;
    btcLegacyCoin.is_active = g_companionAppcoinState[BTC].state;
    btcLegacyCoin.coin_code = "BTC_LEGACY";
    btcLegacyCoin.accounts = &btcLegacyAccount;
    btcLegacyCoin.accounts_length = 1;

    // BTV NATIVE SEGWIT
    char *btcNativeSegwitPath = "M/84'/0'/0'";
    char *btcNativeSegwitXpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);

    AccountConfig btcNativeSegwitAccount;
    btcNativeSegwitAccount.hd_path = btcNativeSegwitPath;
    btcNativeSegwitAccount.x_pub = btcNativeSegwitXpub;
    btcNativeSegwitAccount.address_length = 20;
    btcNativeSegwitAccount.is_multi_sign = false;

    CoinConfig btcNativeSegwitCoin;
    btcNativeSegwitCoin.is_active = g_companionAppcoinState[BTC].state;
    btcNativeSegwitCoin.coin_code = "BTC_NATIVE_SEGWIT";
    btcNativeSegwitCoin.accounts = &btcNativeSegwitAccount;
    btcNativeSegwitCoin.accounts_length = 1;

    // LTC
    char *ltcPath = "M/49'/2'/0'";
    char *ltcXpub = GetCurrentAccountPublicKey(XPUB_TYPE_LTC);

    AccountConfig ltcAccount;
    ltcAccount.hd_path = ltcPath;
    ltcAccount.x_pub = ltcXpub;
    ltcAccount.address_length = 20;
    ltcAccount.is_multi_sign = false;

    CoinConfig ltcCoin;
    ltcCoin.is_active = g_companionAppcoinState[LTC].state;
    ltcCoin.coin_code = "LTC";
    ltcCoin.accounts = &ltcAccount;
    ltcCoin.accounts_length = 1;

    // DASH
    char *dashPath = "M/44'/5'/0'";
    char *dashXpub = GetCurrentAccountPublicKey(XPUB_TYPE_DASH);

    AccountConfig dashAccount;
    dashAccount.hd_path = dashPath;
    dashAccount.x_pub = dashXpub;
    dashAccount.address_length = 20;
    dashAccount.is_multi_sign = false;

    CoinConfig dashCoin;
    dashCoin.is_active = g_companionAppcoinState[DASH].state;
    dashCoin.coin_code = "DASH";
    dashCoin.accounts = &dashAccount;
    dashCoin.accounts_length = 1;

    // BCH
    char *bchPath = "M/44'/145'/0'";
    char *bchXpub = GetCurrentAccountPublicKey(XPUB_TYPE_BCH);

    AccountConfig bchAccount;
    bchAccount.hd_path = bchPath;
    bchAccount.x_pub = bchXpub;
    bchAccount.address_length = 20;
    bchAccount.is_multi_sign = false;

    CoinConfig bchCoin;
    bchCoin.is_active = g_companionAppcoinState[BCH].state;
    bchCoin.coin_code = "BCH";
    bchCoin.accounts = &bchAccount;
    bchCoin.accounts_length = 1;

    // ETH
    char *ethStandardPath = "M/44'/60'/0'";
    char *ethStandardXpub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD);

    AccountConfig ethAccount;
    ethAccount.hd_path = ethStandardPath;
    ethAccount.x_pub = ethStandardXpub;
    ethAccount.address_length = 20;
    ethAccount.is_multi_sign = false;

    CoinConfig ethCoin;
    ethCoin.is_active = g_companionAppcoinState[ETH].state;
    ;
    ethCoin.coin_code = "ETH";
    ethCoin.accounts = &ethAccount;
    ethCoin.accounts_length = 1;

    // TRX
    char *trxPath = "M/44'/195'/0'";
    char *trxXpub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);

    AccountConfig trxAccount;
    trxAccount.hd_path = trxPath;
    trxAccount.x_pub = trxXpub;
    trxAccount.address_length = 20;
    trxAccount.is_multi_sign = false;

    CoinConfig trxCoin;
    trxCoin.is_active = g_companionAppcoinState[TRON].state;
    trxCoin.coin_code = "TRON";
    trxCoin.accounts = &trxAccount;
    trxCoin.accounts_length = 1;

    CoinConfig coins[8] = {btcCoin, btcLegacyCoin, btcNativeSegwitCoin, ltcCoin, dashCoin, bchCoin, ethCoin, trxCoin};
    UREncodeResult *result = get_connect_companion_app_ur(mfp, sizeof(mfp), SOFTWARE_VERSION, coins, 8);
    CHECK_CHAIN_PRINT(result);
    return result;
}

UREncodeResult *GuiGetOkxWalletData(void)
{
#ifndef COMPILE_SIMULATOR
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    //   btc 3
    // + eth 10
    // + trx 1
    // + ltc 1
    // + dash 1
    // + bch 1
    ExtendedPublicKey keys[17];
    public_keys->data = keys;
    public_keys->size = 17;
    for (int i = XPUB_TYPE_ETH_LEDGER_LIVE_0; i <= XPUB_TYPE_ETH_LEDGER_LIVE_9; i++) {
        keys[i - XPUB_TYPE_ETH_LEDGER_LIVE_0].path = SRAM_MALLOC(64);
        sprintf(keys[i - XPUB_TYPE_ETH_LEDGER_LIVE_0].path, "m/44'/60'/%d'", i - XPUB_TYPE_ETH_LEDGER_LIVE_0);
        keys[i - XPUB_TYPE_ETH_LEDGER_LIVE_0].xpub = GetCurrentAccountPublicKey(i);
    }

    keys[10].path = "m/44'/0'/0'";
    keys[10].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);

    keys[11].path = "m/49'/0'/0'";
    keys[11].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);

    keys[12].path = "m/84'/0'/0'";
    keys[12].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);

    keys[13].path = GetXPubPath(XPUB_TYPE_TRX);
    keys[13].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);

    keys[14].path = GetXPubPath(XPUB_TYPE_LTC);
    keys[14].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_LTC);

    keys[15].path = GetXPubPath(XPUB_TYPE_DASH);
    keys[15].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_DASH);

    keys[16].path = GetXPubPath(XPUB_TYPE_BCH);
    keys[16].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BCH);
    char serialNumber[256];
    GetSerialNumber(serialNumber);
    char firmwareVersion[12];
    GetSoftWareVersionNumber(firmwareVersion);
    g_urEncode = get_okx_wallet_ur(mfp, sizeof(mfp), serialNumber, public_keys, "Keystone 3 Pro", firmwareVersion);
    CHECK_CHAIN_PRINT(g_urEncode);
    SRAM_FREE(public_keys);
    return g_urEncode;
#else
    const uint8_t *data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    return (void *)data;
#endif
}

UREncodeResult *GuiGetSolflareData(void)
{
#ifndef COMPILE_SIMULATOR
    SOLAccountType accountType = GetSolflareAccountType();
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[10];
    public_keys->data = keys;

    if (accountType == SOLBip44) {
        public_keys->size = 10;
        for (int i = XPUB_TYPE_SOL_BIP44_0; i <= XPUB_TYPE_SOL_BIP44_9; i++) {
            char *path = SRAM_MALLOC(sizeof(char) * 32);
            sprintf(path, "m/44'/501'/%d'", i - XPUB_TYPE_SOL_BIP44_0); 
            keys[i - XPUB_TYPE_SOL_BIP44_0].path = path;
            keys[i - XPUB_TYPE_SOL_BIP44_0].xpub = GetCurrentAccountPublicKey(i);
        }
    } else if (accountType == SOLBip44ROOT) {
        public_keys->size = 1;
        char *path = SRAM_MALLOC(sizeof(char) * 32);
        sprintf(path, "m/44'/501'");
        keys[0].path = path;
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_SOL_BIP44_ROOT);
    } else if (accountType == SOLBip44Change) {
        public_keys->size = 10;
        for (int i = XPUB_TYPE_SOL_BIP44_CHANGE_0; i <= XPUB_TYPE_SOL_BIP44_CHANGE_9; i++) {
            char *path = SRAM_MALLOC(sizeof(char) * 32);
            sprintf(path, "m/44'/501'/%d'/0'", i - XPUB_TYPE_SOL_BIP44_CHANGE_0);
            keys[i - XPUB_TYPE_SOL_BIP44_CHANGE_0].path = path;
            keys[i - XPUB_TYPE_SOL_BIP44_CHANGE_0].xpub = GetCurrentAccountPublicKey(i);
        }
    }
    g_urEncode = get_connect_solana_wallet_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(g_urEncode);
    for (int i = 0; i < public_keys->size; i++) {
        SRAM_FREE(public_keys->data[i].path);
    }
    SRAM_FREE(public_keys);
    return g_urEncode;
#else
    const uint8_t *data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    return (void *)data;
#endif


}