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

static UREncodeResult *g_urEncode = NULL;
static uint8_t *g_pincode = NULL;

UREncodeResult *GuiGetBlueWalletBtcData(void)
{
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
}

UREncodeResult *GuiGetSparrowWalletBtcData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
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
    UREncodeResult *urencode = get_connect_sparrow_wallet_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urencode);
    return urencode;
}

UREncodeResult *GuiGetSpecterWalletBtcData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    ExtendedPublicKey keys[2];
    public_keys->data = keys;
    public_keys->size = 2;
    keys[0].path = "m/84'/0'/0'";
    keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC_NATIVE_SEGWIT);
    keys[1].path = "m/49'/0'/0'";
    keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
    UREncodeResult *urencode = get_connect_specter_wallet_ur(mfp, sizeof(mfp), public_keys);
    CHECK_CHAIN_PRINT(urencode);
    return urencode;
}

UREncodeResult *GuiGetErgoData(void)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
    // + ergo 1
    ExtendedPublicKey keys[1];
    public_keys->data = keys;
    public_keys->size = NUMBER_OF_ARRAYS(keys);

    // eth standard
    keys[0].path = GetXPubPath(XPUB_TYPE_ERG);
    keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_ERG);

    char serialNumber[256];
    GetSerialNumber(serialNumber);
    char firmwareVersion[12];
    GetSoftWareVersionNumber(firmwareVersion);
    g_urEncode = generate_ergo_wallet_ur(mfp, sizeof(mfp), serialNumber, public_keys, "Keystone 3 Pro", firmwareVersion);
    CHECK_CHAIN_PRINT(g_urEncode);
    SRAM_FREE(public_keys);
    return g_urEncode;
}

UREncodeResult *GuiGetCakeData(void)
{
    char *xPub = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_0);
    char *pvk = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_PVK_0);
    if (g_pincode == NULL) {
        g_urEncode = get_connect_cake_wallet_ur(xPub, pvk);
    } else {
        g_urEncode = get_connect_cake_wallet_ur_encrypted(xPub, pvk, g_pincode);
    }
    CHECK_CHAIN_PRINT(g_urEncode);
    return g_urEncode;
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