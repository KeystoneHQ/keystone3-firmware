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

UREncodeResult *GuiGetBlueWalletBtcData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        return export_multi_sig_wallet_by_ur(mfp, sizeof(mfp), GetDefaultMultisigWallet()->walletConfig);
    }
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    int length = 3;
    ExtendedPublicKey keys[length];
    public_keys->data = keys;
    public_keys->size = length;

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

    UREncodeResult *urEncode = generate_btc_crypto_account_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urEncode);
    SRAM_FREE(public_keys);
    return urEncode;
}

UREncodeResult *GuiGetSparrowWalletBtcData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        return export_multi_sig_wallet_by_ur(mfp, sizeof(mfp), GetDefaultMultisigWallet()->walletConfig);
    }
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    int length = 4;
    ExtendedPublicKey keys[length];
    public_keys->data = keys;
    public_keys->size = length;

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
    UREncodeResult *urEncode = generate_btc_crypto_account_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urEncode);
    SRAM_FREE(public_keys);
    return urEncode;
}

UREncodeResult *GuiGetSpecterWalletBtcData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        return export_multi_sig_wallet_by_ur(mfp, sizeof(mfp), GetDefaultMultisigWallet()->walletConfig);
    }
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    int length = 2;
    ExtendedPublicKey keys[length];
    public_keys->data = keys;
    public_keys->size = length;

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
    UREncodeResult *urEncode = generate_btc_crypto_account_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urEncode);
    SRAM_FREE(public_keys);
    return urEncode;
}