#include "gui_chain.h"

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
    case REMAPVIEW_SOL:
    case REMAPVIEW_SOL_MESSAGE:
        return GuiGetSolCheckResult();
    case REMAPVIEW_APT:
        return GuiGetAptosCheckResult();
    case REMAPVIEW_ADA:
        return GuiGetAdaCheckResult();
    case REMAPVIEW_XRP:
        return GuiGetXrpCheckResult();
    case REMAPVIEW_AR:
    case REMAPVIEW_AR_MESSAGE:
        return GuiGetArCheckResult();
    case REMAPVIEW_TON:
    case REMAPVIEW_TON_SIGNPROOF:
        return GuiGetTonCheckResult();
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
    case SolanaTx:
    case SolanaMessage:
        return CHAIN_SOL;
    case AptosTx:
        return CHAIN_APT;
    case CardanoTx:
        return CHAIN_ADA;
    case XRPTx:
        return CHAIN_XRP;
    case ArweaveTx:
    case ArweaveMessage:
        return CHAIN_ARWEAVE;
    case TonTx:
    case TonSignProof:
        return CHAIN_TON;
#endif
    default:
        return CHAIN_BUTT;
    }
    return CHAIN_BUTT;
}

#ifndef BTC_ONLY
bool IsMessageType(uint8_t type)
{
    return type == EthPersonalMessage || type == EthTypedData || IsCosmosMsg(type) || type == SolanaMessage || IsAptosMsg(type) || type == BtcMsg || type == ArweaveMessage;
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
        func = GuiGetSignQrCodeData;
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
    case SolanaTx:
    case SolanaMessage:
        func = GuiGetSolSignQrCodeData;
        break;
    case AptosTx:
        func = GuiGetAptosSignQrCodeData;
        break;
    case CardanoTx:
        func = GuiGetAdaSignQrCodeData;
        break;
    case XRPTx:
        func = GuiGetXrpSignQrCodeData;
        break;
    case ArweaveMessage:
    case ArweaveTx:
        func = GuiGetArweaveSignQrCodeData;
        break;
    case TonTx:
        func = GuiGetTonSignQrCodeData;
        break;
    case TonSignProof:
        func = GuiGetTonProofSignQrCodeData;
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