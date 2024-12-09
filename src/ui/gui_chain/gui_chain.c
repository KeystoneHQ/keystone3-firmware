#include "gui_chain.h"

#ifndef BTC_ONLY
bool CheckViewTypeIsAllow(uint8_t viewType)
{
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
        return true;
    default:
        return false;
    }
}
#endif

PtrT_TransactionCheckResult CheckUrResult(uint8_t viewType)
{
    switch (ViewTypeReMap(viewType)) {
    case REMAPVIEW_BTC:
    case REMAPVIEW_BTC_MESSAGE:
        return GuiGetPsbtCheckResult();
#ifndef BTC_ONLY
    case REMAPVIEW_ETH:
    case REMAPVIEW_ETH_PERSONAL_MESSAGE:
    case REMAPVIEW_ETH_TYPEDDATA:
        return GuiGetEthCheckResult();
    case REMAPVIEW_TRX:
        return GuiGetTrxCheckResult();
    case REMAPVIEW_COSMOS:
        return GuiGetCosmosCheckResult();
    case REMAPVIEW_SUI:
        return GuiGetSuiCheckResult();
    case REMAPVIEW_SUI_SIGN_MESSAGE_HASH:
        return GuiGetSuiSignHashCheckResult();
    case REMAPVIEW_SOL:
    case REMAPVIEW_SOL_MESSAGE:
        return GuiGetSolCheckResult();
    case REMAPVIEW_APT:
        return GuiGetAptosCheckResult();
    case REMAPVIEW_ADA:
        return GuiGetAdaCheckResult();
    case REMAPVIEW_ADA_SIGN_DATA:
        return GuiGetAdaSignDataCheckResult();
    case REMAPVIEW_ADA_CATALYST:
        return GuiGetAdaCatalystCheckResult();
    case REMAPVIEW_XRP:
        return GuiGetXrpCheckResult();
    case REMAPVIEW_AR:
    case REMAPVIEW_AR_MESSAGE:
    case REMAPVIEW_AR_DATAITEM:
        return GuiGetArCheckResult();
    case REMAPVIEW_STELLAR:
    case REMAPVIEW_STELLAR_HASH:
        return GuiGetStellarCheckResult();
    case REMAPVIEW_TON:
    case REMAPVIEW_TON_SIGNPROOF:
        return GuiGetTonCheckResult();
    case REMAPVIEW_ZCASH:
        return GuiGetZcashCheckResult();
#endif
    default:
        return NULL;
    }
}

GuiChainCoinType ViewTypeToChainTypeSwitch(uint8_t ViewType)
{
    switch (ViewType) {
    case BtcNativeSegwitTx:
    case BtcSegwitTx:
    case BtcLegacyTx:
    case BtcTx:
    case BtcMsg:
        return CHAIN_BTC;
#ifndef BTC_ONLY
    case LtcTx:
        return CHAIN_LTC;
    case DashTx:
        return CHAIN_DASH;
    case BchTx:
        return CHAIN_BCH;
    case EthPersonalMessage:
    case EthTx:
    case EthTypedData:
        return CHAIN_ETH;
    case TronTx:
        return CHAIN_TRX;
    case CosmosTx:
    case CosmosEvmTx:
        return GuiGetCosmosTxChain();
    case SuiTx:
        return CHAIN_SUI;
    case SuiSignMessageHash:
        return CHAIN_SUI;
    case SolanaTx:
    case SolanaMessage:
        return CHAIN_SOL;
    case AptosTx:
        return CHAIN_APT;
    case CardanoTx:
    case CardanoSignData:
    case CardanoCatalystVotingRegistration:
        return CHAIN_ADA;
    case XRPTx:
        return CHAIN_XRP;
    case ArweaveTx:
    case ArweaveMessage:
    case ArweaveDataItem:
        return CHAIN_ARWEAVE;
    case StellarTx:
    case StellarHash:
        return CHAIN_STELLAR;
    case TonTx:
    case TonSignProof:
        return CHAIN_TON;
    case ZcashTx:
        return CHAIN_ZCASH;
#endif
    default:
        return CHAIN_BUTT;
    }
    return CHAIN_BUTT;
}

#ifndef BTC_ONLY
bool IsMessageType(uint8_t type)
{
    return type == EthPersonalMessage || type == EthTypedData || IsCosmosMsg(type) || type == SolanaMessage || IsAptosMsg(type) || type == BtcMsg || type == ArweaveMessage || type == CardanoSignData;
}

bool isTonSignProof(uint8_t type)
{
    return type == TonSignProof;
}

bool isCatalystVotingRegistration(uint8_t type)
{
    return type == CardanoCatalystVotingRegistration;
}
#endif

static GenerateUR UrGenerator(ViewType viewType, bool isMulti)
{
    GenerateUR func = NULL;
    switch (viewType) {
    case BtcNativeSegwitTx:
    case BtcSegwitTx:
    case BtcLegacyTx:
    case BtcTx:
    case BtcMsg:
#ifndef BTC_ONLY
    case LtcTx:
    case DashTx:
    case BchTx:
#endif
        func = isMulti ? GuiGetBtcSignQrCodeData : GuiGetBtcSignUrDataUnlimited;
        break;
#ifndef BTC_ONLY
    case EthTx:
    case EthPersonalMessage:
    case EthTypedData:
        func = isMulti ? GuiGetEthSignQrCodeData : GuiGetEthSignUrDataUnlimited;
        break;
    case TronTx:
        func = GuiGetTrxSignQrCodeData;
        break;
    case CosmosTx:
    case CosmosEvmTx:
        func = GuiGetCosmosSignQrCodeData;
        break;
    case SuiTx:
        func = GuiGetSuiSignQrCodeData;
        break;
    case SuiSignMessageHash:
        func = GuiGetSuiSignHashQrCodeData;
        break;
    case SolanaTx:
    case SolanaMessage:
        func = GuiGetSolSignQrCodeData;
        break;
    case AptosTx:
        func = GuiGetAptosSignQrCodeData;
        break;
    case CardanoSignData:
        func = GuiGetAdaSignSignDataQrCodeData;
        break;
    case CardanoCatalystVotingRegistration:
        func = GuiGetAdaSignCatalystVotingRegistrationQrCodeData;
        break;
    case CardanoTx:
        func = GuiGetAdaSignQrCodeData;
        break;
    case XRPTx:
        func = GuiGetXrpSignQrCodeData;
        break;
    case ArweaveTx:
    case ArweaveMessage:
    case ArweaveDataItem:
        func = GuiGetArweaveSignQrCodeData;
        break;
    case StellarTx:
    case StellarHash:
        func = GuiGetStellarSignQrCodeData;
        break;
    case TonTx:
        func = GuiGetTonSignQrCodeData;
        break;
    case TonSignProof:
        func = GuiGetTonProofSignQrCodeData;
        break;
    case ZcashTx:
        func = GuiGetZcashSignQrCodeData;
        break;
#endif
    default:
        break;
    }
    return func;
}

GenerateUR GetUrGenerator(ViewType viewType)
{
    return UrGenerator(viewType, true);
}

GenerateUR GetSingleUrGenerator(ViewType viewType)
{
    return UrGenerator(viewType, false);
}