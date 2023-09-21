#include "service_eth_sign.h";
#include "user_delay.h";
#include "gui_chain.h";
#include "gui_views.h";

typedef struct
{
    uint8_t viewType;
    uint8_t urType;
} UrViewType_t;

static void HandleSuccessFunc(const void * data, uint32_t data_len)
{
    if (g_handleURCallback != NULL)
    {
        g_handleURCallback(APDU_PROTOCOL_HEADER, CMD_SIGN_ETH_TX, (uint8_t *)data, data_len);
    }
};

void *ProcessEthereumTransactionSignature(uint8_t *data, uint32_t dataLen, ResponseHandler *sendResponse)
{
    g_handleURCallback = sendResponse;

    UREncodeResult *encodeResult;
    uint8_t seed[64];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
    struct URParseResult *urResult = parse_ur(data);
    UrViewType_t urViewType = {0, 0};
    urViewType.viewType = urResult->t;
    urViewType.urType = urResult->ur_type;
    GuiFrameOpenViewWithParam(&g_USBTransportView, HandleSuccessFunc, sizeof(HandleSuccessFunc));
    UserDelay(500);
    handleURResult(urResult, urViewType, false);
}