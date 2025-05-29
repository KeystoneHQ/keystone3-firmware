#include "define.h"
#include "gui_chain.h"

typedef TransactionCheckResult *(*CheckUrResultHandler)(void);

typedef struct {
    ViewType type;
    GenerateUR handler;
    GenerateUR unlimitHandler;
    CheckUrResultHandler checkHandler;
    GuiChainCoinType coinType;
    GuiRemapViewType remapType;
} ViewHandlerEntry;

bool CheckViewTypeIsAllow(uint8_t viewType)
{
#ifdef WEB3_VERSION
    switch (ViewTypeReMap(viewType)) {
    case REMAPVIEW_ETH:
    case REMAPVIEW_ETH_PERSONAL_MESSAGE:
    case REMAPVIEW_ETH_TYPEDDATA:
    case REMAPVIEW_SOL:
    case REMAPVIEW_SOL_MESSAGE:
    case REMAPVIEW_BTC:
    case REMAPVIEW_BTC_MESSAGE:
    case REMAPVIEW_COSMOS:
    case REMAPVIEW_SUI_SIGN_MESSAGE_HASH:
    case REMAPVIEW_ADA_SIGN_TX_HASH:
    case REMAPVIEW_ADA:
    case REMAPVIEW_ADA_SIGN_DATA:
    case REMAPVIEW_ADA_CATALYST:
    case REMAPVIEW_AVAX:
        return true;
    default:
        return false;
    }
#endif
#ifdef CYPHERPUNK_VERSION
    return ViewTypeReMap(viewType) == REMAPVIEW_BTC || ViewTypeReMap(viewType) == REMAPVIEW_BTC_MESSAGE;
#endif
    return false;
}

static const ViewHandlerEntry g_viewHandlerMap[] = {
    {BtcNativeSegwitTx, GuiGetBtcSignQrCodeData, GuiGetBtcSignUrDataUnlimited, GuiGetPsbtCheckResult, CHAIN_BTC, REMAPVIEW_BTC},
    {BtcSegwitTx, GuiGetBtcSignQrCodeData, GuiGetBtcSignUrDataUnlimited, GuiGetPsbtCheckResult, CHAIN_BTC, REMAPVIEW_BTC},
    {BtcLegacyTx, GuiGetBtcSignQrCodeData, GuiGetBtcSignUrDataUnlimited, GuiGetPsbtCheckResult, CHAIN_BTC, REMAPVIEW_BTC},
    {BtcTx, GuiGetBtcSignQrCodeData, GuiGetBtcSignUrDataUnlimited, GuiGetPsbtCheckResult, CHAIN_BTC, REMAPVIEW_BTC},
    {BtcMsg, GuiGetBtcSignQrCodeData, GuiGetBtcSignUrDataUnlimited, GuiGetPsbtCheckResult, CHAIN_BTC, REMAPVIEW_BTC_MESSAGE},

#ifdef WEB3_VERSION
    {LtcTx, GuiGetBtcSignQrCodeData, GuiGetBtcSignUrDataUnlimited, GuiGetPsbtCheckResult, CHAIN_LTC, REMAPVIEW_BTC},
    {DogeTx, GuiGetBtcSignQrCodeData, GuiGetBtcSignUrDataUnlimited, GuiGetPsbtCheckResult, CHAIN_DOGE, REMAPVIEW_BTC},
    {DashTx, GuiGetBtcSignQrCodeData, GuiGetBtcSignUrDataUnlimited, GuiGetPsbtCheckResult, CHAIN_DASH, REMAPVIEW_BTC},
    {BchTx, GuiGetBtcSignQrCodeData, GuiGetBtcSignUrDataUnlimited, GuiGetPsbtCheckResult, CHAIN_BCH, REMAPVIEW_BTC},
    {EthTx, GuiGetEthSignQrCodeData, GuiGetEthSignUrDataUnlimited, GuiGetEthCheckResult, CHAIN_ETH, REMAPVIEW_ETH},
    {EthPersonalMessage, GuiGetEthSignQrCodeData, GuiGetEthSignUrDataUnlimited, GuiGetEthCheckResult, CHAIN_ETH, REMAPVIEW_ETH_PERSONAL_MESSAGE},
    {EthTypedData, GuiGetEthSignQrCodeData, GuiGetEthSignUrDataUnlimited, GuiGetEthCheckResult, CHAIN_ETH, REMAPVIEW_ETH_TYPEDDATA},

    {TronTx, GuiGetTrxSignQrCodeData, NULL, GuiGetTrxCheckResult, CHAIN_TRX, REMAPVIEW_TRX},

    // avax
    {AvaxTx, GuiGetAvaxSignQrCodeData, GuiGetAvaxSignUrDataUnlimited, GuiGetAvaxCheckResult, CHAIN_AVAX, REMAPVIEW_AVAX},

    // must get from GuiGetCosmosTxChain
    {CosmosTx, GuiGetCosmosSignQrCodeData, GuiGetCosmosSignQrCodeData, GuiGetCosmosCheckResult, CHAIN_ATOM, REMAPVIEW_COSMOS},
    {CosmosEvmTx, GuiGetCosmosSignQrCodeData, GuiGetCosmosSignQrCodeData, GuiGetCosmosCheckResult, CHAIN_ATOM, REMAPVIEW_COSMOS},

    {SuiTx, GuiGetSuiSignQrCodeData, NULL, GuiGetSuiCheckResult, CHAIN_SUI, REMAPVIEW_SUI},
    {SuiSignMessageHash, GuiGetSuiSignHashQrCodeData, GuiGetSuiSignHashQrCodeData, GuiGetSuiSignHashCheckResult, CHAIN_SUI, REMAPVIEW_SUI_SIGN_MESSAGE_HASH},

    {SolanaTx, GuiGetSolSignQrCodeData, GuiGetSolSignQrCodeData, GuiGetSolCheckResult, CHAIN_SOL, REMAPVIEW_SOL},
    {SolanaMessage, GuiGetSolSignQrCodeData, GuiGetSolSignQrCodeData, GuiGetSolCheckResult, CHAIN_SOL, REMAPVIEW_SOL_MESSAGE},

    {AptosTx, GuiGetAptosSignQrCodeData, NULL, GuiGetAptosCheckResult, CHAIN_APT, REMAPVIEW_APT},

    {CardanoSignTxHash, GuiGetAdaSignTxHashQrCodeData, GuiGetAdaSignTxHashQrCodeData, GuiGetAdaSignTxHashCheckResult, CHAIN_ADA, REMAPVIEW_ADA_SIGN_TX_HASH},
    {CardanoSignData, GuiGetAdaSignSignDataQrCodeData, GuiGetAdaSignSignDataQrCodeData, GuiGetAdaSignDataCheckResult, CHAIN_ADA, REMAPVIEW_ADA_SIGN_DATA},
    {CardanoSignCip8Data, GuiGetAdaSignSignCip8DataQrCodeData, GuiGetAdaSignSignCip8DataQrCodeData, GuiGetAdaSignDataCheckResult, CHAIN_ADA, REMAPVIEW_ADA_SIGN_DATA},
    {CardanoCatalystVotingRegistration, GuiGetAdaSignCatalystVotingRegistrationQrCodeData, GuiGetAdaSignCatalystVotingRegistrationQrCodeData, GuiGetAdaCatalystCheckResult, CHAIN_ADA, REMAPVIEW_ADA_CATALYST},
    {CardanoTx, GuiGetAdaSignQrCodeData, GuiGetAdaSignUrDataUnlimited, GuiGetAdaCheckResult, CHAIN_ADA, REMAPVIEW_ADA},

    {XRPTx, GuiGetXrpSignQrCodeData, NULL, GuiGetXrpCheckResult, CHAIN_XRP, REMAPVIEW_XRP},

    {ArweaveTx, GuiGetArweaveSignQrCodeData, NULL, GuiGetArCheckResult, CHAIN_ARWEAVE, REMAPVIEW_AR},
    {ArweaveMessage, GuiGetArweaveSignQrCodeData, NULL, GuiGetArCheckResult, CHAIN_ARWEAVE, REMAPVIEW_AR_MESSAGE},
    {ArweaveDataItem, GuiGetArweaveSignQrCodeData, NULL, GuiGetArCheckResult, CHAIN_ARWEAVE, REMAPVIEW_AR_DATAITEM},

    {StellarTx, GuiGetStellarSignQrCodeData, NULL, GuiGetStellarCheckResult, CHAIN_STELLAR, REMAPVIEW_STELLAR},
    {StellarHash, GuiGetStellarSignQrCodeData, NULL, GuiGetStellarCheckResult, CHAIN_STELLAR, REMAPVIEW_STELLAR_HASH},

    {TonTx, GuiGetTonSignQrCodeData, NULL, GuiGetTonCheckResult, CHAIN_TON, REMAPVIEW_TON},
    {TonSignProof, GuiGetTonProofSignQrCodeData, NULL, GuiGetTonCheckResult, CHAIN_TON, REMAPVIEW_TON_SIGNPROOF},
#endif

#ifdef CYPHERPUNK_VERSION
    {ZcashTx, GuiGetZcashSignQrCodeData, NULL, GuiGetZcashCheckResult, CHAIN_ZCASH, REMAPVIEW_ZCASH},
    {XmrOutput, GuiGetMoneroKeyimagesQrCodeData, NULL, GuiGetMoneroOutputCheckResult, CHAIN_XMR, REMAPVIEW_XMR_OUTPUT},
    {XmrTxUnsigned, GuiGetMoneroSignedTransactionQrCodeData, NULL, GuiGetMoneroUnsignedTxCheckResult, CHAIN_XMR, REMAPVIEW_XMR_UNSIGNED},
    {ErgoTx, GuiGetErgoSignQrCodeData, NULL, GuiGetErgoCheckResult, CHAIN_ERG, REMAPVIEW_ERGO},
#endif
};

static const ViewHandlerEntry *GetViewHandlerEntry(ViewType viewType);

PtrT_TransactionCheckResult CheckUrResult(uint8_t viewType)
{
    const ViewHandlerEntry *entry = GetViewHandlerEntry(viewType);
    if (entry != NULL) {
        return entry->checkHandler();
    }
    return NULL;
}

GuiChainCoinType ViewTypeToChainTypeSwitch(uint8_t viewType)
{
#ifdef WEB3_VERSION
    if (viewType == CosmosTx || viewType == CosmosEvmTx) {
        return GuiGetCosmosTxChain();
    }
#endif

    const ViewHandlerEntry *entry = GetViewHandlerEntry(viewType);
    if (entry != NULL) {
        return entry->coinType;
    }
    return CHAIN_BUTT;
}

#ifdef WEB3_VERSION
bool IsMessageType(uint8_t type)
{
    return type == EthPersonalMessage || type == EthTypedData || IsCosmosMsg(type) || type == SolanaMessage || IsAptosMsg(type) || type == BtcMsg || type == ArweaveMessage || type == CardanoSignData || type == CardanoSignCip8Data;
}

bool isCatalystVotingRegistration(uint8_t type)
{
    return type == CardanoCatalystVotingRegistration;
}

bool isTonSignProof(uint8_t type)
{
    return type == TonSignProof;
}
#endif

static GenerateUR UrGenerator(ViewType viewType, bool isMulti)
{
    const ViewHandlerEntry *entry = GetViewHandlerEntry(viewType);
    if (entry != NULL) {
        return isMulti ? entry->handler : entry->unlimitHandler;
    }
    return NULL;
}

GuiRemapViewType ViewTypeReMap(uint8_t viewType)
{
    const ViewHandlerEntry *entry = GetViewHandlerEntry(viewType);
    if (entry != NULL) {
        return entry->remapType;
    }
    return REMAPVIEW_BUTT;
}

GenerateUR GetUrGenerator(ViewType viewType)
{
    return UrGenerator(viewType, true);
}

GenerateUR GetSingleUrGenerator(ViewType viewType)
{
    return UrGenerator(viewType, false);
}

static const ViewHandlerEntry *GetViewHandlerEntry(ViewType viewType)
{
    for (size_t i = 0; i < NUMBER_OF_ARRAYS(g_viewHandlerMap); i++) {
        if (g_viewHandlerMap[i].type == viewType) {
            return &g_viewHandlerMap[i];
        }
    }
    return NULL;
}