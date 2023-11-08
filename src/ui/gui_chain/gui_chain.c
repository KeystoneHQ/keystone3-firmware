#include "gui_chain.h"

PtrT_TransactionCheckResult CheckScanResult(uint8_t viewType)
{
    switch (ViewTypeReMap(viewType)) {
        case REMAPVIEW_BTC:
            return GuiGetPsbtCheckResult();
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
        default:
            return NULL;
    }
}

GuiChainCoinType ViewTypeToChainTypeSwitch(uint8_t ViewType)
{
    switch (ViewType)
    {
    case BtcNativeSegwitTx:
    case BtcSegwitTx:
    case BtcLegacyTx:
    case BtcTx:
        return CHAIN_BTC;
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
    default:
        return CHAIN_BUTT;
    }
    return CHAIN_BUTT;
}

bool IsMessageType(uint8_t type)
{
    return type == EthPersonalMessage || type == EthTypedData || IsCosmosMsg(type) || type == SolanaMessage || IsAptosMsg(type);
}