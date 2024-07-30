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

UREncodeResult *GuiGetBlueWalletBtcData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
#ifdef BTC_ONLY
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        return export_multi_sig_wallet_by_ur(mfp, sizeof(mfp), GetDefaultMultisigWallet()->walletConfig);
    }
#endif
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[3];
    public_keys->data = keys;
    public_keys->size = 3;
#ifndef BTC_ONLY
    keys[0].path = "m/84'/0'/0'";
    keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
    keys[1].path = "m/49'/0'/0'";
    keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
    keys[2].path = "m/44'/0'/0'";
    keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);
#else
    if (GetIsTestNet()) {
        keys[0].path = "m/84'/1'/0'";
        keys[1].path = "m/49'/1'/0'";
        keys[2].path = "m/44'/1'/0'";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT_TEST);
        keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TEST);
        keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY_TEST);
    } else {
        keys[0].path = "m/84'/0'/0'";
        keys[1].path = "m/49'/0'/0'";
        keys[2].path = "m/44'/0'/0'";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
        keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
        keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);
    }
#endif
    UREncodeResult *urencode = get_connect_blue_wallet_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urencode);
    return urencode;
}

UREncodeResult *GuiGetSparrowWalletBtcData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
#ifdef BTC_ONLY
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        return export_multi_sig_wallet_by_ur(mfp, sizeof(mfp), GetDefaultMultisigWallet()->walletConfig);
    }
#endif
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[4];
    public_keys->data = keys;
    public_keys->size = 4;
#ifndef BTC_ONLY
    keys[0].path = "m/84'/0'/0'";
    keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
    keys[1].path = "m/49'/0'/0'";
    keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
    keys[2].path = "m/44'/0'/0'";
    keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);
    keys[3].path = "m/86'/0'/0'";
    keys[3].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TAPROOT);
#else
    if (GetIsTestNet()) {
        keys[0].path = "m/84'/1'/0'";
        keys[1].path = "m/49'/1'/0'";
        keys[2].path = "m/44'/1'/0'";
        keys[3].path = "m/86'/1'/0'";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT_TEST);
        keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TEST);
        keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY_TEST);
        keys[3].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TAPROOT_TEST);
    } else {
        keys[0].path = "m/84'/0'/0'";
        keys[1].path = "m/49'/0'/0'";
        keys[2].path = "m/44'/0'/0'";
        keys[3].path = "m/86'/0'/0'";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
        keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
        keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);
        keys[3].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TAPROOT);
    }
#endif
    UREncodeResult *urencode = get_connect_sparrow_wallet_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urencode);
    return urencode;
}

UREncodeResult *GuiGetSpecterWalletBtcData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
#ifdef BTC_ONLY
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        return export_multi_sig_wallet_by_ur(mfp, sizeof(mfp), GetDefaultMultisigWallet()->walletConfig);
    }
#endif
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[2];
    public_keys->data = keys;
    public_keys->size = 2;
#ifndef BTC_ONLY
    keys[0].path = "m/84'/0'/0'";
    keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
    keys[1].path = "m/49'/0'/0'";
    keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
#else
    if (GetIsTestNet()) {
        keys[0].path = "m/84'/1'/0'";
        keys[1].path = "m/49'/1'/0'";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT_TEST);
        keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TEST);
    } else {
        keys[0].path = "m/84'/0'/0'";
        keys[1].path = "m/49'/0'/0'";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
        keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
    }
#endif
    UREncodeResult *urencode = get_connect_specter_wallet_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urencode);
    return urencode;
}

#ifndef BTC_ONLY

typedef UREncodeResult *MetamaskUrGetter(PtrBytes master_fingerprint, uint32_t master_fingerprint_length, enum ETHAccountType account_type, PtrT_CSliceFFI_ExtendedPublicKey public_keys);

static UREncodeResult *get_unlimited_connect_metamask_ur(PtrBytes master_fingerprint, uint32_t master_fingerprint_length, enum ETHAccountType account_type, PtrT_CSliceFFI_ExtendedPublicKey public_keys)
{
    return get_connect_metamask_ur_unlimited(master_fingerprint, master_fingerprint_length, account_type, public_keys);
}



static UREncodeResult *BasicGetBitgetDataForAccountType(ETHAccountType accountType)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[13];
    public_keys->data = keys;

    if (accountType == Bip44Standard) {
        public_keys->size = 4;
        keys[0].path = "m/44'/60'/0'";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD);
    } else if (accountType == LedgerLive) {
        public_keys->size = 13;
        for (int i = XPUB_TYPE_ETH_LEDGER_LIVE_0; i <= XPUB_TYPE_ETH_LEDGER_LIVE_9; i++) {
            keys[i - XPUB_TYPE_ETH_LEDGER_LIVE_0].path = SRAM_MALLOC(BUFFER_SIZE_64);
            snprintf_s(keys[i - XPUB_TYPE_ETH_LEDGER_LIVE_0].path, BUFFER_SIZE_64, "m/44'/60'/%d'", i - XPUB_TYPE_ETH_LEDGER_LIVE_0);
            keys[i - XPUB_TYPE_ETH_LEDGER_LIVE_0].xpub = GetCurrentAccountPublicKey(i);
        }
    } else if (accountType == LedgerLegacy) {
        public_keys->size = 4;
        keys[0].path = "m/44'/60'/0'";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LEGACY);
    }
    if (public_keys->size == 4) {

        keys[1].path = "m/44'/0'/0'";
        keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);

        keys[2].path = "m/49'/0'/0'";
        keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);

        keys[3].path = "m/84'/0'/0'";
        keys[3].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
    } else {
        keys[10].path = "m/44'/0'/0'";
        keys[10].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);

        keys[11].path = "m/49'/0'/0'";
        keys[11].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);

        keys[12].path = "m/84'/0'/0'";
        keys[12].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
    }


    char serialNumber[256];
    GetSerialNumber(serialNumber);
    char firmwareVersion[12];
    GetSoftWareVersionNumber(firmwareVersion);
    g_urEncode = get_bitget_wallet_ur(mfp, sizeof(mfp), serialNumber, accountType, public_keys, "Keystone 3 Pro", firmwareVersion);
    CHECK_CHAIN_PRINT(g_urEncode);
    SRAM_FREE(public_keys);
    return g_urEncode;
}


static UREncodeResult *BasicGetMetamaskDataForAccountType(ETHAccountType accountType, MetamaskUrGetter func)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[10];
    public_keys->data = keys;

    if (accountType == Bip44Standard) {
        public_keys->size = 1;
        keys[0].path = "";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD);
    } else if (accountType == LedgerLive) {
        public_keys->size = 10;
        for (int i = XPUB_TYPE_ETH_LEDGER_LIVE_0; i <= XPUB_TYPE_ETH_LEDGER_LIVE_9; i++) {
            keys[i - XPUB_TYPE_ETH_LEDGER_LIVE_0].path = "";
            keys[i - XPUB_TYPE_ETH_LEDGER_LIVE_0].xpub = GetCurrentAccountPublicKey(i);
        }
    } else if (accountType == LedgerLegacy) {
        public_keys->size = 1;
        keys[0].path = "";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LEGACY);
    }

    g_urEncode = func(mfp, sizeof(mfp), accountType, public_keys);
    CHECK_CHAIN_PRINT(g_urEncode);
    SRAM_FREE(public_keys);
    return g_urEncode;
}

// copy from gui_btc, need to use real data
UREncodeResult *GetMetamaskDataForAccountType(ETHAccountType accountType)
{
    return BasicGetMetamaskDataForAccountType(accountType, get_connect_metamask_ur);
}

UREncodeResult *GetBitgetDataForAccountType(ETHAccountType accountType)
{
    return BasicGetBitgetDataForAccountType(accountType);
}

UREncodeResult *GetUnlimitedMetamaskDataForAccountType(ETHAccountType accountType)
{
    return BasicGetMetamaskDataForAccountType(accountType, get_unlimited_connect_metamask_ur);
}
UREncodeResult *GuiGetMetamaskData(void)
{
    ETHAccountType accountType = GetMetamaskAccountType();
    return GetMetamaskDataForAccountType(accountType);
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

UREncodeResult *GuiGetArConnectData(void)
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
    g_urEncode = get_connect_arconnect_wallet_ur_from_xpub(mfp, sizeof(mfp), arXpub);
    printf("\ng_urEncode: %s\n", g_urEncode->data);
    CHECK_CHAIN_PRINT(g_urEncode);
    return g_urEncode;
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
}

UREncodeResult *GuiGetPetraData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey publicKeys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[10];
    publicKeys->data = keys;
    publicKeys->size = 10;
    for (uint8_t i = 0; i < 10; i++) {
        keys[i].path = SRAM_MALLOC(BUFFER_SIZE_32);
        snprintf_s(keys[i].path, BUFFER_SIZE_32, "m/44'/637'/%u'/0'/0'", i);
        keys[i].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_APT_0 + i);
    }
    g_urEncode = get_connect_aptos_wallet_ur(mfp, sizeof(mfp), publicKeys);
    CHECK_CHAIN_PRINT(g_urEncode);
    for (uint8_t i = 0; i < 10; i++) {
        SRAM_FREE(keys[i].path);
    }
    SRAM_FREE(publicKeys);
    return g_urEncode;
}

static uint8_t MapAdaIndex2ChainType(uint16_t index)
{
    switch (index) {
    case 0:
        return XPUB_TYPE_ADA_0;
    case 1:
        return XPUB_TYPE_ADA_1;
    case 2:
        return XPUB_TYPE_ADA_2;
    case 3:
        return XPUB_TYPE_ADA_3;
    case 4:
        return XPUB_TYPE_ADA_4;
    case 5:
        return XPUB_TYPE_ADA_5;
    case 6:
        return XPUB_TYPE_ADA_6;
    case 7:
        return XPUB_TYPE_ADA_7;
    case 8:
        return XPUB_TYPE_ADA_8;
    case 9:
        return XPUB_TYPE_ADA_9;
    case 10:
        return XPUB_TYPE_ADA_10;
    case 11:
        return XPUB_TYPE_ADA_11;
    case 12:
        return XPUB_TYPE_ADA_12;
    case 13:
        return XPUB_TYPE_ADA_13;
    case 14:
        return XPUB_TYPE_ADA_14;
    case 15:
        return XPUB_TYPE_ADA_15;
    case 16:
        return XPUB_TYPE_ADA_16;
    case 17:
        return XPUB_TYPE_ADA_17;
    case 18:
        return XPUB_TYPE_ADA_18;
    case 19:
        return XPUB_TYPE_ADA_19;
    case 20:
        return XPUB_TYPE_ADA_20;
    case 21:
        return XPUB_TYPE_ADA_21;
    case 22:
        return XPUB_TYPE_ADA_22;
    case 23:
        return XPUB_TYPE_ADA_23;
    default:
        return XPUB_TYPE_ADA_0;
    }
}

UREncodeResult *GuiGetADADataByIndex(uint16_t index)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    char* xpub = GetCurrentAccountPublicKey(MapAdaIndex2ChainType(index));
    char path[BUFFER_SIZE_32] = {0};
    sprintf(path, "1852'/1815'/%u'", index);
    ExtendedPublicKey xpubs[1];
    xpubs[0].path = path;
    xpubs[0].xpub = xpub;
    CSliceFFI_ExtendedPublicKey keys;
    keys.data = xpubs;
    keys.size = 1;
    return generate_key_derivation_ur(mfp, 4, &keys);
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

    g_urEncode = get_connect_keplr_wallet_ur(mfp, sizeof(mfp), publicKeys);
    CHECK_CHAIN_PRINT(g_urEncode);
    for (uint8_t i = 0; i < 8; i++) {
        SRAM_FREE(keys[i].path);
    }
    SRAM_FREE(publicKeys);
    return g_urEncode;
}

UREncodeResult *GuiGetXrpToolkitDataByIndex(uint16_t index)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    char *xpub = GetCurrentAccountPublicKey(XPUB_TYPE_XRP);
    char *rootPath = "m/44'/144'/0'";
    char hdPath[BUFFER_SIZE_32] = {0};
    snprintf_s(hdPath, BUFFER_SIZE_32, "%s/0/%u", rootPath, index);
    g_urEncode = get_connect_xrp_toolkit_ur(hdPath, xpub, rootPath);
    CHECK_CHAIN_PRINT(g_urEncode);
    return g_urEncode;
}

#endif

UREncodeResult *GuiGetKeystoneWalletData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    //   btc 3
    // + eth 1
    // + bch 1
    // + dash 1
    // + dot 1
    // + ltc 1
    // + trx 1
    // + xrp 1

    // total xpub = 9
    uint8_t XPUB_AMMOUNT = 9;
    ExtendedPublicKey keys[XPUB_AMMOUNT];
    public_keys->data = keys;
    public_keys->size = XPUB_AMMOUNT;

    keys[0].path = "m/44'/0'/0'";
    keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);

    keys[1].path = "m/49'/0'/0'";
    keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);

    keys[2].path = "m/84'/0'/0'";
    keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);

    keys[3].path = GetXPubPath(XPUB_TYPE_BCH);
    keys[3].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BCH);

    keys[4].path = GetXPubPath(XPUB_TYPE_DASH);
    keys[4].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_DASH);


    keys[5].path = GetXPubPath(XPUB_TYPE_LTC);
    keys[5].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_LTC);

    keys[6].path = GetXPubPath(XPUB_TYPE_TRX);
    keys[6].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);

    keys[7].path = GetXPubPath(XPUB_TYPE_XRP);
    keys[7].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_XRP);

    keys[8].path = GetXPubPath(XPUB_TYPE_ETH_BIP44_STANDARD);
    keys[8].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD);



    char serialNumber[256];
    GetSerialNumber(serialNumber);
    char firmwareVersion[12];
    GetSoftWareVersionNumber(firmwareVersion);
    g_urEncode = get_keystone_wallet_ur(mfp, sizeof(mfp), serialNumber, public_keys, "Keystone 3 Pro", firmwareVersion);

    CHECK_CHAIN_PRINT(g_urEncode);
    SRAM_FREE(public_keys);
    return g_urEncode;
}
UREncodeResult *GuiGetBitgetWalletData(void)
{
    ETHAccountType accountType = GetMetamaskAccountType();
    return GetBitgetDataForAccountType(accountType);
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
#ifndef BTC_ONLY

    ExtendedPublicKey keys[18];
    public_keys->data = keys;
    public_keys->size = 18;
    for (int i = XPUB_TYPE_ETH_LEDGER_LIVE_0; i <= XPUB_TYPE_ETH_LEDGER_LIVE_9; i++) {
        keys[i - XPUB_TYPE_ETH_LEDGER_LIVE_0].path = SRAM_MALLOC(BUFFER_SIZE_64);
        snprintf_s(keys[i - XPUB_TYPE_ETH_LEDGER_LIVE_0].path, BUFFER_SIZE_64, "m/44'/60'/%d'", i - XPUB_TYPE_ETH_LEDGER_LIVE_0);
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

    keys[17].path = "m/86'/0'/0'";
    keys[17].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TAPROOT);

#else
    ExtendedPublicKey keys[4];
    public_keys->data = keys;
    public_keys->size = 4;

    keys[0].path = "m/44'/0'/0'";
    keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);

    keys[1].path = "m/49'/0'/0'";
    keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);

    keys[2].path = "m/84'/0'/0'";
    keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);

    keys[3].path = "m/86'/0'/0'";
    keys[3].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TAPROOT);
#endif
    char serialNumber[256];
    GetSerialNumber(serialNumber);
    char firmwareVersion[12];
    GetSoftWareVersionNumber(firmwareVersion);
#ifndef BTC_ONLY
    g_urEncode = get_okx_wallet_ur(mfp, sizeof(mfp), serialNumber, public_keys, "Keystone 3 Pro", firmwareVersion);
#else
    g_urEncode = get_okx_wallet_ur_btc_only(mfp, sizeof(mfp), serialNumber, public_keys, "Keystone 3 Pro", firmwareVersion);
#endif
    CHECK_CHAIN_PRINT(g_urEncode);
    SRAM_FREE(public_keys);
    return g_urEncode;
}

#ifndef BTC_ONLY
UREncodeResult *GuiGetSolflareData(void)
{
    SOLAccountType accountType = GetSolflareAccountType();
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[10];
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
        char *path = SRAM_MALLOC(sizeof(char) * 32);
        snprintf_s(path, BUFFER_SIZE_32, "m/44'/501'");
        keys[0].path = path;
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_SOL_BIP44_ROOT);
    } else if (accountType == SOLBip44Change) {
        public_keys->size = 10;
        for (int i = XPUB_TYPE_SOL_BIP44_CHANGE_0; i <= XPUB_TYPE_SOL_BIP44_CHANGE_9; i++) {
            char *path = SRAM_MALLOC(sizeof(char) * 32);
            snprintf_s(path, BUFFER_SIZE_32, "m/44'/501'/%d'/0'", i - XPUB_TYPE_SOL_BIP44_CHANGE_0);
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
}
#endif

#ifndef BTC_ONLY
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
    g_urEncode = get_connect_xbull_wallet_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(g_urEncode);
    for (int i = 0; i < public_keys->size; i++) {
        SRAM_FREE(public_keys->data[i].path);
    }
    SRAM_FREE(public_keys);
    return g_urEncode;
}

UREncodeResult *GuiGetBackpackData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[31];
    public_keys->data = keys;
    public_keys->size = 31;

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

    g_urEncode = get_backpack_wallet_ur(mfp, sizeof(mfp), public_keys);

    CHECK_CHAIN_PRINT(g_urEncode);
    for (int i = 0; i < public_keys->size; i++) {
        SRAM_FREE(public_keys->data[i].path);
    }
    SRAM_FREE(public_keys);
    return g_urEncode;
}
#endif
