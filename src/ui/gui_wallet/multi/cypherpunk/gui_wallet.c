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

void GenerateCakeWalletEncryptPincode(void);

static uint8_t *g_pincode = NULL;

UREncodeResult *GuiGetBlueWalletBtcData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    int length = 3;
    ExtendedPublicKey keys[length];
    public_keys->data = keys;
    public_keys->size = length;
    keys[0].path = "m/84'/0'/0'";
    keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
    keys[1].path = "m/49'/0'/0'";
    keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
    keys[2].path = "m/44'/0'/0'";
    keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);
    UREncodeResult *urEncode = generate_btc_crypto_account_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urEncode);
    SRAM_FREE(public_keys);
    return urEncode;
}

UREncodeResult *GuiGetSparrowWalletBtcData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    int length = 4;
    ExtendedPublicKey keys[length];
    public_keys->data = keys;
    public_keys->size = length;
    keys[0].path = "m/84'/0'/0'";
    keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
    keys[1].path = "m/49'/0'/0'";
    keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
    keys[2].path = "m/44'/0'/0'";
    keys[2].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_LEGACY);
    keys[3].path = "m/86'/0'/0'";
    keys[3].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_TAPROOT);
    UREncodeResult *urEncode = generate_btc_crypto_account_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urEncode);
    SRAM_FREE(public_keys);
    return urEncode;
}

UREncodeResult *GuiGetCakeData(void)
{
    char *xPub = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_0);
    char *pvk = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_PVK_0);
    UREncodeResult *urEncode;
    if (g_pincode == NULL) {
        urEncode = get_connect_cake_wallet_ur(xPub, pvk);
    } else {
        urEncode = get_connect_cake_wallet_ur_encrypted(xPub, pvk, g_pincode);
    }
    CHECK_CHAIN_PRINT(urEncode);
    return urEncode;
}

void ClosePrivateQrMode(void)
{
    if (g_pincode == NULL) {
        return;
    }
    SRAM_FREE(g_pincode);
    g_pincode = NULL;
}

uint8_t *OpenPrivateQrMode(void)
{
    GenerateCakeWalletEncryptPincode();
    return g_pincode;
}

bool IsPrivateQrMode(void)
{
    return g_pincode != NULL;
}

void GenerateCakeWalletEncryptPincode(void)
{
    uint8_t pincode[6];
    GenerateEntropy(pincode, 6, "Monero Connect Wallet Salt");
    for (uint8_t i = 0; i < 6; i++) {
        pincode[i] = pincode[i] % 10;
    }
    g_pincode = SRAM_MALLOC(6);
    memcpy(g_pincode, pincode, 6);
}