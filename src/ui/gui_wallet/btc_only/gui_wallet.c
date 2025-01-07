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

static UREncodeResult *g_urEncode = NULL;

UREncodeResult *GuiGetBlueWalletBtcData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        return export_multi_sig_wallet_by_ur(mfp, sizeof(mfp), GetDefaultMultisigWallet()->walletConfig);
    }
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[3];
    public_keys->data = keys;
    public_keys->size = 3;

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
    UREncodeResult *urencode = get_connect_blue_wallet_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urencode);
    return urencode;
}

UREncodeResult *GuiGetSparrowWalletBtcData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        return export_multi_sig_wallet_by_ur(mfp, sizeof(mfp), GetDefaultMultisigWallet()->walletConfig);
    }
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[4];
    public_keys->data = keys;
    public_keys->size = 4;

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
    UREncodeResult *urencode = get_connect_sparrow_wallet_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urencode);
    return urencode;
}

UREncodeResult *GuiGetSpecterWalletBtcData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        return export_multi_sig_wallet_by_ur(mfp, sizeof(mfp), GetDefaultMultisigWallet()->walletConfig);
    }
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[2];
    public_keys->data = keys;
    public_keys->size = 2;

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
    UREncodeResult *urencode = get_connect_specter_wallet_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urencode);
    return urencode;
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
    char serialNumber[256];
    GetSerialNumber(serialNumber);
    char firmwareVersion[BUFFER_SIZE_32];
    GetSoftWareVersionNumber(firmwareVersion);
    g_urEncode = get_okx_wallet_ur_btc_only(mfp, sizeof(mfp), serialNumber, public_keys, "Keystone 3 Pro", firmwareVersion);
    CHECK_CHAIN_PRINT(g_urEncode);
    SRAM_FREE(public_keys);
    return g_urEncode;
}

