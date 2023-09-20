#include "service_eth_sign.h";
#include "user_delay.h";
#include "gui_chain.h";
#include "gui_views.h";

typedef struct
{
    uint8_t viewType;
    uint8_t urType;
} UrViewType_t;

Response *ProcessEthereumTransactionSignature(uint8_t *data, uint16_t dataLen)
{
    Response *response = (Response *)malloc(sizeof(Response));
    response->length = 0;
    response->data = NULL;

    UREncodeResult *encodeResult;
    uint8_t seed[64];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
    struct URParseResult *urResult = parse_ur(data);
    UrViewType_t urViewType = {0, 0};
    urViewType.viewType = urResult->t;
    urViewType.urType = urResult->ur_type;

    GuiFrameOpenView(&g_qrCodeView);
    handleURResult(urResult, urViewType, false);

    UserDelay(5000);
    // TODO: check error / multiple ur / define error code
    encodeResult = eth_sign_tx(urResult->data, seed, len);
    response->data = encodeResult->data;
    response->length = strlen(encodeResult->data);
    response->error_code = encodeResult->error_code;
    return response;
}