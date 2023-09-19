#include "service_eth_sign.h";

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
    // TODO: check error / multiple ur / define error code
    encodeResult = eth_sign_tx(urResult->data, seed, len);
    response->data = encodeResult->data;
    response->length = strlen(encodeResult->data);
    response->error_code = encodeResult->error_code;
    return response;
}