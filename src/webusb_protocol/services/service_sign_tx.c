#include "service_sign_tx.h"
#include "stdio.h"
#include "protocol_codec.h"
#include "string.h"
#include "sha256.h"
#include "background_task.h"
#include "keystore.h"
#include "account_public_info.h"
#include "librust_c.h"

#define TYPE_SIGN_PUB_PATH               0x01
#define TYPE_SIGN_ROOT_PATH              0x02
#define TYPE_SIGN_TX_CHAIN_ID            0x03

#define TYPE_SIGN_TX_SIGNED_DATA        0x11
#define TYPE_SIGN_TX_SIGNED_STATUS      0x12

#define SIGNATURE_SIZE                 64  // Assume signature size is 64 bytes

bool authenticate_request(const FrameHead_t *head);
uint8_t *ServiceSignTxReq(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen);

const ProtocolServiceCallbackFunc_t g_signTxServiceFunc[] = {
    NULL,                                       //3.0
    ServiceSignTxReq,                           //3.1.1
};

bool authenticate_request(const FrameHead_t *head) {
    // TODO: Implement this function based on your authentication mechanism
    return true;
}

static bool sign_tx(const uint8_t *data, const uint8_t *fromAddress, const uint8_t *chainID, uint8_t *signedTxData)
{
    memcpy(signedTxData, data, strlen(data));
    memcpy(signedTxData + strlen(data), fromAddress, 16);
    memcpy(signedTxData + strlen(data) + strlen(fromAddress), chainID, 16);
    return true;
}

static GetEthAddress()
{
    char *ethStandardXpub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD);
}

static char *USBGetEthAddress(char *hdPath, char *rootPath, char *address)
{
    char *xPub;
    char path[128];
    xPub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD);
    SimpleResponse_c_char  *result = eth_get_address(hdPath, xPub, rootPath);
    if (result->error_code == 0) {
        strcpy(address, result->data);
        strcpy(path, hdPath);
    } else {
        strcpy(address, "Error");
    }
    return *address;
}

uint8_t *ServiceSignTxReq(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen)
{
    // Perform authentication.
    if (!authenticate_request(head)) {
        // Authentication failed. Return an error.
        printf("Authentication failed\n");
        return NULL;
    }

    Tlv_t tlvArray[3] = {0};
    uint32_t tlvNumber;
    uint8_t *txData = NULL;
    uint32_t txDataSize = UINT32_MAX;
    uint8_t *rootPath = NULL;
    uint32_t rootPathSize = UINT32_MAX;
    uint8_t *chainID = NULL;
    uint32_t chainIDSize = UINT32_MAX;

    // Parse incoming TLV data
    tlvNumber = GetTlvFromData(tlvArray, 3, tlvData, head->length);
    for (uint32_t i = 0; i < tlvNumber; i++) {
        switch (tlvArray[i].type) {
        case TYPE_SIGN_PUB_PATH: {
            txData = tlvArray[i].pValue;
            txDataSize = tlvArray[i].length;
        }
        break;
        case TYPE_SIGN_ROOT_PATH: {
            rootPath = tlvArray[i].pValue;
            rootPathSize = tlvArray[i].length;
        }
        break;
        case TYPE_SIGN_TX_CHAIN_ID: {
            chainID = tlvArray[i].pValue;
            chainIDSize = tlvArray[i].length;
        }
        default:
            break;
        }
    }

    // Error checking
    if (txData == NULL || txDataSize == UINT32_MAX) {
        printf("Invalid request\n");
        return NULL;
    }

    // Perform signing operation here. Signature is stored in 'signedTxData'.
    uint8_t signedTxData[SIGNATURE_SIZE]= {0};
    // if (!sign_tx(txData, fromAddress, chainID, signedTxData)) {
    //     // Signing operation failed. Return an error.
    //     printf("Signing operation failed\n");
    //     return NULL;
    // }

    // Prepare response
    FrameHead_t sendHead = {0};
    Tlv_t sendTlvArray[2] = {0};

    sendHead.packetIndex = head->packetIndex;
    sendHead.serviceId = head->serviceId;
    sendHead.commandId = COMMAND_ID_SIGN_TX_RESP;
    sendHead.flag.b.ack = 0;
    sendHead.flag.b.isHost = 0;
    char address[128] = {0};
    USBGetEthAddress(txData, rootPath, address);
    // char *xPub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD);
    // sendTlvArray[0].type = TYPE_SIGN_TX_SIGNED_DATA;
    // sendTlvArray[0].length = strlen(xPub) + 1;
    // sendTlvArray[0].pValue = xPub;
    sendTlvArray[0].type = TYPE_SIGN_TX_SIGNED_DATA;
    sendTlvArray[0].length = strlen(address) + 1;
    sendTlvArray[0].pValue = address;

    char *status = "Success";

    sendTlvArray[1].type = TYPE_SIGN_TX_SIGNED_STATUS;
    sendTlvArray[1].length = strlen(status) + 1;
    sendTlvArray[1].pValue = status;

    *outLen = GetFrameTotalLength(sendTlvArray, 2);
    return BuildFrame(&sendHead, sendTlvArray, 2);
}