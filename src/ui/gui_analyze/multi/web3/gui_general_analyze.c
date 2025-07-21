#include "gui_chain.h"
#include "gui_analyze.h"

static GetLabelDataFunc GuiAdaTextFuncGet(char *type);
static GetLabelDataLenFunc GuiAdaTextLenFuncGet(char *type);
static GetLabelDataLenFunc GuiEthTextLenFuncGet(char *type);
static GetLabelDataLenFunc GuiXrpTextLenFuncGet(char *type);
static GetLabelDataLenFunc GuiStellarTextLenFuncGet(char *type);
static GetLabelDataLenFunc GuiArTextLenFuncGet(char *type);
static GetTableDataFunc GuiEthTableFuncGet(char *type);
static GetTableDataFunc GuiAdaTabelFuncGet(char *type);
static GetLabelDataFunc GuiTrxTextFuncGet(char *type);
static GetLabelDataFunc GuiCosmosTextFuncGet(char *type);
static GetLabelDataFunc GuiSuiTextFuncGet(char *type);
static GetLabelDataLenFunc GuiSuiTextLenFuncGet(char *type);
static GetLabelDataFunc GuiAptosTextFuncGet(char *type);
static GetLabelDataLenFunc GuiAptosTextLenFuncGet(char *type);
static GetLabelDataFunc GuiXrpTextFuncGet(char *type);
static GetLabelDataFunc GuiArTextFuncGet(char *type);
static GetLabelDataFunc GuiStellarTextFuncGet(char *type);
static GetLabelDataFunc GuiSolMessageTextFuncGet(char *type);
static GetLabelDataFunc GuiEthTypedDataTextFuncGet(char *type);
static GetLabelDataFunc GuiEthPersonalMessageTextFuncGet(char *type);
static GetLabelDataFunc GuiEthTextFuncGet(char *type);
static GetContSizeFunc GetEthObjPos(char *type);
static GetContSizeFunc GetCosmosObjPos(char *type);
static GetListItemKeyFunc GetCosmosListItemKey(char *type);
static GetListLenFunc GetCosmosListLen(char *type);
static GetContSizeFunc GetAdaContainerSize(char *type);
static GetContSizeFunc GetCosmosContainerSize(char *type);
static GetContSizeFunc GetEthContainerSize(char *type);


GetContSizeFunc GetOtherChainContainerSize(char *type, GuiRemapViewType remapIndex)
{
    switch (remapIndex) {
    case REMAPVIEW_ETH:
    case REMAPVIEW_ETH_TYPEDDATA:
        return GetEthContainerSize(type);
    case REMAPVIEW_COSMOS:
        return GetCosmosContainerSize(type);
    case REMAPVIEW_ADA:
    case REMAPVIEW_ADA_SIGN_DATA:
    case REMAPVIEW_ADA_CATALYST:
        return GetAdaContainerSize(type);
    default:
        break;
    }
    return NULL;
}

GetContSizeFunc GetOtherChainPos(char *type, GuiRemapViewType remapIndex)
{
    switch (remapIndex) {
    case REMAPVIEW_ETH:
    case REMAPVIEW_ETH_TYPEDDATA:
        return GetEthObjPos(type);
    case REMAPVIEW_COSMOS:
        return GetCosmosObjPos(type);
    default:
        break;
    }
    return NULL;
}

GetCustomContainerFunc GetOtherChainCustomFunc(char *funcName)
{
    if (!strcmp(funcName, "GuiShowSolTxOverview")) {
        return GuiShowSolTxOverview;
    } else if (!strcmp(funcName, "GuiShowSolTxDetail")) {
        return GuiShowSolTxDetail;
    } else if (!strcmp(funcName, "GuiShowArweaveTxDetail")) {
        return GuiShowArweaveTxDetail;
    } else if (!strcmp(funcName, "GetCatalystRewardsNotice")) {
        return GetCatalystRewardsNotice;
    } else if (!strcmp(funcName, "GuiStellarTxNotice")) {
        return GuiStellarTxNotice;
    } else if (!strcmp(funcName, "GuiStellarHashNotice")) {
        return GuiStellarHashNotice;
    } else if (!strcmp(funcName, "GuiTonTxOverview")) {
        return GuiTonTxOverview;
    } else if (!strcmp(funcName, "GuiTonTxRawData")) {
        return GuiTonTxRawData;
    } else if (!strcmp(funcName, "GuiTonProofOverview")) {
        return GuiTonProofOverview;
    } else if (!strcmp(funcName, "GuiTonProofRawData")) {
        return GuiTonProofRawData;
    } else if (!strcmp(funcName, "GuiArDataItemOverview")) {
        return GuiArDataItemOverview;
    } else if (!strcmp(funcName, "GuiArDataItemDetail")) {
        return GuiArDataItemDetail;
    } else if (!strcmp(funcName, "GuiShowSuiSignMessageHashOverview")) {
        return GuiShowSuiSignMessageHashOverview;
    } else if (!strcmp(funcName, "GuiShowSuiSignMessageHashDetails")) {
        return GuiShowSuiSignMessageHashDetails;
    } else if (!strcmp(funcName, "GuiShowAdaSignTxHashOverview")) {
        return GuiShowAdaSignTxHashOverview;
    } else if (!strcmp(funcName, "GuiShowAdaSignTxHashDetails")) {
        return GuiShowAdaSignTxHashDetails;
    } else if (!strcmp(funcName, "GuiAvaxTxOverview")) {
        return GuiAvaxTxOverview;
    } else if (!strcmp(funcName, "GuiAvaxTxRawData")) {
        return GuiAvaxTxRawData;
    } else if (!strcmp(funcName, "GuiIotaTxOverview")) {
        return GuiIotaTxOverview;
    } else if (!strcmp(funcName, "GuiIotaTxRawData")) {
        return GuiIotaTxRawData;
    }

    return NULL;
}

lv_event_cb_t GuiOtherChainEventCbGet(char *type)
{
    if (!strcmp(type, "EthContractLearnMore")) {
        return EthContractLearnMore;
    } else if (!strcmp(type, "EthContractCheckRawData")) {
        return EthContractCheckRawData;
    }

    return NULL;
}

GetObjStateFunc GuiOtherChainStateFuncGet(char *type)
{
    if (!strcmp(type, "GetEthEnsExist")) {
        return GetEthEnsExist;
    } else if (!strcmp(type, "GetEthTypeDataHashExist")) {
        return GetEthTypeDataHashExist;
    } else if (!strcmp(type, "GetToEthEnsExist")) {
        return GetToEthEnsExist;
    } else if (!strcmp(type, "GetEthTypeDataChainExist")) {
        return GetEthTypeDataChainExist;
    } else if (!strcmp(type, "GetEthTypeDataVersionExist")) {
        return GetEthTypeDataVersionExist;
    } else if (!strcmp(type, "GetEthContractDataExist")) {
        return GetEthContractDataExist;
    } else if (!strcmp(type, "GetEthContractDataNotExist")) {
        return GetEthContractDataNotExist;
    } else if (!strcmp(type, "GetEthInputDataExist")) {
        return GetEthInputDataExist;
    } else if (!strcmp(type, "EthInputExistContractNot")) {
        return EthInputExistContractNot;
    } else if (!strcmp(type, "GetTrxContractExist")) {
        return GetTrxContractExist;
    } else if (!strcmp(type, "GetTrxTokenExist")) {
        return GetTrxTokenExist;
    } else if (!strcmp(type, "GetCosmosChannelExist")) {
        return GetCosmosChannelExist;
    } else if (!strcmp(type, "GetCosmosOldValidatorExist")) {
        return GetCosmosOldValidatorExist;
    } else if (!strcmp(type, "GetCosmosValueExist")) {
        return GetCosmosValueExist;
    } else if (!strcmp(type, "GetCosmosVoteExist")) {
        return GetCosmosVoteExist;
    } else if (!strcmp(type, "GetCosmosAddress2Exist")) {
        return GetCosmosAddress2Exist;
    } else if (!strcmp(type, "GetCosmosMsgListExist")) {
        return GetCosmosMsgListExist;
    } else if (!strcmp(type, "GetCosmosMethodExist")) {
        return GetCosmosMethodExist;
    } else if (!strcmp(type, "GetCosmosAddrExist")) {
        return GetCosmosAddrExist;
    } else if (!strcmp(type, "GetAdaWithdrawalsExist")) {
        return GetAdaWithdrawalsExist;
    } else if (!strcmp(type, "GetAdaCertificatesExist")) {
        return GetAdaCertificatesExist;
    } else if (!strcmp(type, "GetAdaExtraDataExist")) {
        return GetAdaExtraDataExist;
    } else if (!strcmp(type, "GetAdaVotingProceduresExist")) {
        return GetAdaVotingProceduresExist;
    } else if (!strcmp(type, "GetAdaVotingProposalsExist")) {
        return GetAdaVotingProposalsExist;
    } else if (!strcmp(type, "GetEthPermitWarningExist")) {
        return GetEthPermitWarningExist;
    } else if (!strcmp(type, "GetEthPermitCantSign")) {
        return GetEthPermitCantSign;
    } else if (!strcmp(type, "GetEthOperationWarningExist")) {
        return GetEthOperationWarningExist;
    } else if (!strcmp(type, "GetIotaIsMessage")) {
        return GetIotaIsMessage;
    } else if (!strcmp(type, "GetIotaIsTransaction")) {
        return GetIotaIsTransaction;
    } else if (!strcmp(type, "GetIotaIsTransfer")) {
        return GetIotaIsTransfer;
    }
    return NULL;
}

GetTableDataFunc GuiOtherChainTableFuncGet(char *type, GuiRemapViewType remapIndex)
{
    switch (remapIndex) {
    case REMAPVIEW_ETH:
        return GuiEthTableFuncGet(type);
    case REMAPVIEW_ADA:
    case REMAPVIEW_ADA_SIGN_DATA:
    case REMAPVIEW_ADA_CATALYST:
        return GuiAdaTabelFuncGet(type);
    default:
        return NULL;
    }
}

GetLabelDataFunc GuiOtherChainTextFuncGet(char *type, GuiRemapViewType remapIndex)
{
    switch (remapIndex) {
    case REMAPVIEW_ETH:
        return GuiEthTextFuncGet(type);
    case REMAPVIEW_ETH_PERSONAL_MESSAGE:
        return GuiEthPersonalMessageTextFuncGet(type);
    case REMAPVIEW_ETH_TYPEDDATA:
        return GuiEthTypedDataTextFuncGet(type);
    case REMAPVIEW_TRX:
        return GuiTrxTextFuncGet(type);
    case REMAPVIEW_COSMOS:
        return GuiCosmosTextFuncGet(type);
    case REMAPVIEW_SUI:
        return GuiSuiTextFuncGet(type);
    case REMAPVIEW_SOL_MESSAGE:
        return GuiSolMessageTextFuncGet(type);
    case REMAPVIEW_APT:
        return GuiAptosTextFuncGet(type);
    case REMAPVIEW_ADA:
    case REMAPVIEW_ADA_SIGN_DATA:
    case REMAPVIEW_ADA_CATALYST:
        return GuiAdaTextFuncGet(type);
    case REMAPVIEW_XRP:
        return GuiXrpTextFuncGet(type);
    case REMAPVIEW_AR:
    case REMAPVIEW_AR_MESSAGE:
        return GuiArTextFuncGet(type);
    case REMAPVIEW_STELLAR:
    case REMAPVIEW_STELLAR_HASH:
        return GuiStellarTextFuncGet(type);
    default:
        return NULL;
    }

    return NULL;
}

GetLabelDataLenFunc GuiOtherChainTextLenFuncGet(char *type, GuiRemapViewType remapIndex)
{
    switch (remapIndex) {
    case REMAPVIEW_SUI:
        return GuiSuiTextLenFuncGet(type);
    case REMAPVIEW_APT:
        return GuiAptosTextLenFuncGet(type);
    case REMAPVIEW_ADA:
    case REMAPVIEW_ADA_SIGN_DATA:
    case REMAPVIEW_ADA_CATALYST:
        return GuiAdaTextLenFuncGet(type);
    case REMAPVIEW_XRP:
        return GuiXrpTextLenFuncGet(type);
    case REMAPVIEW_ETH_TYPEDDATA:
    case REMAPVIEW_ETH:
        return GuiEthTextLenFuncGet(type);
    case REMAPVIEW_AR:
    case REMAPVIEW_AR_MESSAGE:
        return GuiArTextLenFuncGet(type);
    case REMAPVIEW_STELLAR:
        return GuiStellarTextLenFuncGet(type);
    default:
        return NULL;
    }
}


GetListItemKeyFunc GetOtherChainListItemKeyFuncGet(char *type, GuiRemapViewType remapIndex)
{
    switch (remapIndex) {
    case REMAPVIEW_COSMOS:
        return (void *)GetCosmosListItemKey;
    default:
        return NULL;
    }
}

GetListLenFunc GetOtherChainListLenFuncGet(char *type, GuiRemapViewType remapIndex)
{
    switch (remapIndex) {
    case REMAPVIEW_COSMOS:
        return (void *)GetCosmosListLen;
    default:
        return NULL;
    }
}

static GetLabelDataFunc GuiAdaTextFuncGet(char *type)
{
    if (!strcmp(type, "GetAdaExtraData")) {
        return GetAdaExtraData;
    }
    if (!strcmp(type, "GetAdaNetwork")) {
        return GetAdaNetwork;
    }
    if (!strcmp(type, "GetAdaTotalInput")) {
        return GetAdaTotalInput;
    }
    if (!strcmp(type, "GetAdaTotalOutput")) {
        return GetAdaTotalOutput;
    }
    if (!strcmp(type, "GetAdaFee")) {
        return GetAdaFee;
    } else if (!strcmp(type, "GetAdaWithdrawalsLabel")) {
        return GetAdaWithdrawalsLabel;
    } else if (!strcmp(type, "GetAdaCertificatesLabel")) {
        return GetAdaCertificatesLabel;
    } else if (!strcmp(type, "GetAdaSignDataPayloadText")) {
        return GetAdaSignDataPayloadText;
    } else if (!strcmp(type, "GetAdaSignDataDerviationPathText")) {
        return GetAdaSignDataDerviationPathText;
    } else if (!strcmp(type, "GetCatalystNonce")) {
        return GetCatalystNonce;
    } else if (!strcmp(type, "GetCatalystVotePublicKey")) {
        return GetCatalystVotePublicKey;
    } else if (!strcmp(type, "GetCatalystRewards")) {
        return GetCatalystRewards;
    } else if (!strcmp(type, "GetCatalystVoteKeys")) {
        return GetCatalystVoteKeys;
    } else if (!strcmp(type, "GetAdaSignDataMessageHashText")) {
        return GetAdaSignDataMessageHashText;
    } else if (!strcmp(type, "GetAdaSignDataXPubText")) {
        return GetAdaSignDataXPubText;
    } else if (!strcmp(type, "GetAdaVotingProceduresLabel")) {
        return GetAdaVotingProceduresLabel;
    } else if (!strcmp(type, "GetAdaVotingProposalsLabel")) {
        return GetAdaVotingProposalsLabel;
    }
    return NULL;
}

static GetLabelDataLenFunc GuiAdaTextLenFuncGet(char *type)
{
    if (!strcmp(type, "GetAdaExtraDataLen")) {
        return GetAdaExtraDataLen;
    } else if (!strcmp(type, "GetAdaSignDataPayloadLength")) {
        return GetAdaSignDataPayloadLength;
    } else if (!strcmp(type, "GetAdaSignDataMessageHashLength")) {
        return GetAdaSignDataMessageHashLength;
    } else if (!strcmp(type, "GetAdaSignDataXPubLength")) {
        return GetAdaSignDataXPubLength;
    }
    return NULL;
}

static GetLabelDataLenFunc GuiEthTextLenFuncGet(char *type)
{
    if (!strcmp(type, "GetEthTypedDataMessageLen")) {
        return GetEthTypedDataMessageLen;
    } else if (!strcmp(type, "GetEthInputDataLen")) {
        return GetEthInputDataLen;
    }
    return NULL;
}

static GetLabelDataLenFunc GuiXrpTextLenFuncGet(char *type)
{
    if (!strcmp(type, "GetXrpDetailLen")) {
        return GetXrpDetailLen;
    }
    return NULL;
}

static GetLabelDataLenFunc GuiStellarTextLenFuncGet(char *type)
{
    if (!strcmp(type, "GetStellarRawMessageLength")) {
        return GetStellarRawMessageLength;
    }
    return NULL;
}

static GetLabelDataLenFunc GuiArTextLenFuncGet(char *type)
{
    if (!strcmp(type, "GetArweaveRawMessageLength")) {
        return GetArweaveRawMessageLength;
    } else if (!strcmp(type, "GetArweaveMessageLength")) {
        return GetArweaveMessageLength;
    }
    return NULL;
}

static GetTableDataFunc GuiEthTableFuncGet(char *type)
{
    if (!strcmp(type, "GetEthContractData")) {
        return GetEthContractData;
    }
    return NULL;
}

static GetTableDataFunc GuiAdaTabelFuncGet(char *type)
{
    if (!strcmp(type, "GetAdaInputDetail")) {
        return GetAdaInputDetail;
    }
    if (!strcmp(type, "GetAdaOutputDetail")) {
        return GetAdaOutputDetail;
    }
    if (!strcmp(type, "GetAdaWithdrawalsData")) {
        return GetAdaWithdrawalsData;
    }
    if (!strcmp(type, "GetAdaCertificatesData")) {
        return GetAdaCertificatesData;
    }
    if (!strcmp(type, "GetAdaVotingProceduresData")) {
        return GetAdaVotingProceduresData;
    }
    return NULL;
}

static GetLabelDataFunc GuiTrxTextFuncGet(char *type)
{
    if (!strcmp(type, "GetTrxValue")) {
        return GetTrxValue;
    } else if (!strcmp(type, "GetTrxMethod")) {
        return GetTrxMethod;
    } else if (!strcmp(type, "GetTrxFromAddress")) {
        return GetTrxFromAddress;
    } else if (!strcmp(type, "GetTrxToAddress")) {
        return GetTrxToAddress;
    } else if (!strcmp(type, "GetTrxContract")) {
        return GetTrxContract;
    } else if (!strcmp(type, "GetTrxToken")) {
        return GetTrxToken;
    }
    return NULL;
}

static GetLabelDataFunc GuiCosmosTextFuncGet(char *type)
{
    if (!strcmp(type, "GetCosmosValue")) {
        return GetCosmosValue;
    } else if (!strcmp(type, "GetCosmosNetwork")) {
        return GetCosmosNetwork;
    } else if (!strcmp(type, "GetCosmosMethod")) {
        return GetCosmosMethod;
    } else if (!strcmp(type, "GetCosmosAddress1Label")) {
        return GetCosmosAddress1Label;
    } else if (!strcmp(type, "GetCosmosAddress1Value")) {
        return GetCosmosAddress1Value;
    } else if (!strcmp(type, "GetCosmosAddress2Label")) {
        return GetCosmosAddress2Label;
    } else if (!strcmp(type, "GetCosmosAddress2Value")) {
        return GetCosmosAddress2Value;
    } else if (!strcmp(type, "GetCosmosMaxFee")) {
        return GetCosmosMaxFee;
    } else if (!strcmp(type, "GetCosmosFee")) {
        return GetCosmosFee;
    } else if (!strcmp(type, "GetCosmosGasLimit")) {
        return GetCosmosGasLimit;
    } else if (!strcmp(type, "GetCosmosChainId")) {
        return GetCosmosChainId;
    } else if (!strcmp(type, "GetCosmosChannel")) {
        return GetCosmosChannel;
    } else if (!strcmp(type, "GetCosmosOldValidator")) {
        return GetCosmosOldValidator;
    } else if (!strcmp(type, "GetCosmosProposal")) {
        return GetCosmosProposal;
    } else if (!strcmp(type, "GetCosmosVoted")) {
        return GetCosmosVoted;
    } else if (!strcmp(type, "GetCosmosIndex")) {
        return GetCosmosIndex;
    } else if (!strcmp(type, "GetCosmosTextOfKind")) {
        return GetCosmosTextOfKind;
    } else if (!strcmp(type, "GetCosmosDetailItemValue")) {
        return GetCosmosDetailItemValue;
    }
    return NULL;
}

static GetLabelDataFunc GuiSuiTextFuncGet(char *type)
{
    if (!strcmp(type, "GetSuiDetail")) {
        return GetSuiDetail;
    }
    return NULL;
}

static GetLabelDataLenFunc GuiSuiTextLenFuncGet(char *type)
{
    if (!strcmp(type, "GetSuiDetailLen")) {
        return GetSuiDetailLen;
    }
    return NULL;
}

static GetLabelDataFunc GuiAptosTextFuncGet(char *type)
{
    if (!strcmp(type, "GetAptosDetail")) {
        return GetAptosDetail;
    }
    return NULL;
}

static GetLabelDataLenFunc GuiAptosTextLenFuncGet(char *type)
{
    if (!strcmp(type, "GetAptosDetailLen")) {
        return GetAptosDetailLen;
    }
    return NULL;
}

static GetLabelDataFunc GuiXrpTextFuncGet(char *type)
{
    if (!strcmp(type, "GetXrpDetail")) {
        return GetXrpDetail;
    }
    return NULL;
}

static GetLabelDataFunc GuiArTextFuncGet(char *type)
{
    if (!strcmp(type, "GetArweaveValue")) {
        return GetArweaveValue;
    } else if (!strcmp(type, "GetArweaveFee")) {
        return GetArweaveFee;
    } else if (!strcmp(type, "GetArweaveFromAddress")) {
        return GetArweaveFromAddress;
    } else if (!strcmp(type, "GetArweaveToAddress")) {
        return GetArweaveToAddress;
    } else if (!strcmp(type, "GetArweaveValue")) {
        return GetArweaveValue;
    } else if (!strcmp(type, "GetArweaveMessageText")) {
        return GetArweaveMessageText;
    } else if (!strcmp(type, "GetArweaveRawMessage")) {
        return GetArweaveRawMessage;
    } else if (!strcmp(type, "GetArweaveMessageAddress")) {
        return GetArweaveMessageAddress;
    }
    return NULL;
}

static GetLabelDataFunc GuiStellarTextFuncGet(char *type)
{
    if (!strcmp(type, "GetStellarRawMessage")) {
        return GetStellarRawMessage;
    }
    return NULL;
}

static GetLabelDataFunc GuiSolMessageTextFuncGet(char *type)
{
    if (!strcmp(type, "GetSolMessageFrom")) {
        return GetSolMessageFrom;
    } else if (!strcmp(type, "GetSolMessageUtf8")) {
        return GetSolMessageUtf8;
    } else if (!strcmp(type, "GetSolMessageRaw")) {
        return GetSolMessageRaw;
    }
    return NULL;
}

static GetLabelDataFunc GuiEthPersonalMessageTextFuncGet(char *type)
{
    if (!strcmp(type, "GetMessageFrom")) {
        return GetMessageFrom;
    } else if (!strcmp(type, "GetMessageUtf8")) {
        return GetMessageUtf8;
    } else if (!strcmp(type, "GetMessageRaw")) {
        return GetMessageRaw;
    }
    return NULL;
}


static GetLabelDataFunc GuiEthTypedDataTextFuncGet(char *type)
{
    if (!strcmp(type, "GetEthTypedDataDomianName")) {
        return GetEthTypedDataDomianName;
    } else if (!strcmp(type, "GetEthTypedDataDomianVersion")) {
        return GetEthTypedDataDomianVersion;
    } else if (!strcmp(type, "GetEthTypedDataDomianChainId")) {
        return GetEthTypedDataDomianChainId;
    } else if (!strcmp(type, "GetEthTypedDataDomianVerifyContract")) {
        return GetEthTypedDataDomianVerifyContract;
    } else if (!strcmp(type, "GetEthTypedDataDomianSalt")) {
        return GetEthTypedDataDomianSalt;
    } else if (!strcmp(type, "GetEthTypedDataMessage")) {
        return GetEthTypedDataMessage;
    } else if (!strcmp(type, "GetEthTypedDataFrom")) {
        return GetEthTypedDataFrom;
    } else if (!strcmp(type, "GetEthTypedDataPrimayType")) {
        return GetEthTypedDataPrimayType;
    } else if (!strcmp(type, "GetEthGetSignerAddress")) {
        return GetEthGetSignerAddress;
    } else if (!strcmp(type, "GetEthTypedDataMessageHash")) {
        return GetEthTypedDataMessageHash;
    } else if (!strcmp(type, "GetEthTypedDataSafeTxHash")) {
        return GetEthTypedDataSafeTxHash;
    } else if (!strcmp(type, "GetEthTypedDataDomainHash")) {
        return GetEthTypedDataDomainHash;
    }
    return NULL;
}

static GetLabelDataFunc GuiEthTextFuncGet(char *type)
{
    if (!strcmp(type, "GetEthValue")) {
        return GetEthValue;
    } else if (!strcmp(type, "GetEthTxFee")) {
        return GetEthTxFee;
    } else if (!strcmp(type, "GetEthGasPrice")) {
        return GetEthGasPrice;
    } else if (!strcmp(type, "GetEthGasLimit")) {
        return GetEthGasLimit;
    } else if (!strcmp(type, "GetEthNetWork")) {
        return GetEthNetWork;
    } else if (!strcmp(type, "GetEthMaxFee")) {
        return GetEthMaxFee;
    } else if (!strcmp(type, "GetEthMaxPriority")) {
        return GetEthMaxPriority;
    } else if (!strcmp(type, "GetEthMaxFeePrice")) {
        return GetEthMaxFeePrice;
    } else if (!strcmp(type, "GetEthMaxPriorityFeePrice")) {
        return GetEthMaxPriorityFeePrice;
    } else if (!strcmp(type, "GetEthGetFromAddress")) {
        return GetEthGetFromAddress;
    } else if (!strcmp(type, "GetEthGetToAddress")) {
        return GetEthGetToAddress;
    } else if (!strcmp(type, "GetEthGetDetailPageToAddress")) {
        return GetEthGetDetailPageToAddress;
    } else if (!strcmp(type, "GetTxnFeeDesc")) {
        return GetTxnFeeDesc;
    } else if (!strcmp(type, "GetEthEnsName")) {
        return GetEthEnsName;
    } else if (!strcmp(type, "GetToEthEnsName")) {
        return GetToEthEnsName;
    } else if (!strcmp(type, "GetEthMethodName")) {
        return GetEthMethodName;
    } else if (!strcmp(type, "GetEthTransactionData")) {
        return GetEthTransactionData;
    } else if (!strcmp(type, "GetEthContractName")) {
        return GetEthContractName;
    } else if (!strcmp(type, "GetEthInputData")) {
        return GetEthInputData;
    } else if (!strcmp(type, "GetEthNonce")) {
        return GetEthNonce;
    }

    return NULL;
}

static GetContSizeFunc GetEthObjPos(char *type)
{
    if (!strcmp(type, "GetEthToLabelPos")) {
        return GetEthToLabelPos;
    } else if (!strcmp(type, "GetEthTypeDomainPos")) {
        return GetEthTypeDomainPos;
    }
    return NULL;
}

static GetContSizeFunc GetCosmosObjPos(char *type)
{
    if (!strcmp(type, "GetCosmosDetailMethodLabelPos")) {
        return GetCosmosDetailMethodLabelPos;
    } else if (!strcmp(type, "GetCosmosDetailMethodValuePos")) {
        return GetCosmosDetailMethodValuePos;
    } else if (!strcmp(type, "GetCosmosDetailAddress1LabelPos")) {
        return GetCosmosDetailAddress1LabelPos;
    } else if (!strcmp(type, "GetCosmosDetailAddress1ValuePos")) {
        return GetCosmosDetailAddress1ValuePos;
    } else if (!strcmp(type, "GetCosmosDetailAddress2LabelPos")) {
        return GetCosmosDetailAddress2LabelPos;
    } else if (!strcmp(type, "GetCosmosDetailAddress2ValuePos")) {
        return GetCosmosDetailAddress2ValuePos;
    }
    return NULL;
}

static GetListItemKeyFunc GetCosmosListItemKey(char *type)
{
    if (!strcmp(type, "GetCosmosMsgKey")) {
        return GetCosmosMsgKey;
    }
    return NULL;
}

static GetListLenFunc GetCosmosListLen(char *type)
{
    if (!strcmp(type, "GetCosmosMsgLen")) {
        return GetCosmosMsgLen;
    }
    return NULL;
}

static GetContSizeFunc GetEthContainerSize(char *type)
{
    if (!strcmp(type, "GetEthToFromSize")) {
        return GetEthToFromSize;
    } else if (!strcmp(type, "GetEthContractDataSize")) {
        return GetEthContractDataSize;
    } else if (!strcmp(type, "GetEthTypeDomainSize")) {
        return GetEthTypeDomainSize;
    }
    return NULL;
}

static GetContSizeFunc GetCosmosContainerSize(char *type)
{
    if (!strcmp(type, "GetCosmosDetailMsgSize")) {
        return GetCosmosDetailMsgSize;
    } else if (!strcmp(type, "GetCosmosOverviewAddrSize")) {
        return GetCosmosOverviewAddrSize;
    }
    return NULL;
}

static GetContSizeFunc GetAdaContainerSize(char *type)
{
    if (!strcmp(type, "GetAdaInputDetailSize")) {
        return GetAdaInputDetailSize;
    }
    if (!strcmp(type, "GetAdaOutputDetailSize")) {
        return GetAdaOutputDetailSize;
    }
    if (!strcmp(type, "GetAdaCertificatesSize")) {
        return GetAdaCertificatesSize;
    }
    if (!strcmp(type, "GetAdaVotingProceduresSize")) {
        return GetAdaVotingProceduresSize;
    }
    if (!strcmp(type, "GetAdaWithdrawalsSize")) {
        return GetAdaWithdrawalsSize;
    }
    if (!strcmp(type, "GetCatalystVoteKeysSize")) {
        return GetCatalystVoteKeysSize;
    }
    return NULL;
}