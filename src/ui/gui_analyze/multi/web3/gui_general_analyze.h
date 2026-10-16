#ifndef _GUI_GENERAL_ANALYZE_WIDGETS_H
#define _GUI_GENERAL_ANALYZE_WIDGETS_H

#ifdef WEB3_VERSION
#define GUI_SOL_PAGE_TEMPLATE "{\"name\":\"sol_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,530],\"bg_color\":0,\"border_width\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowSolTxOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Advanced\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowSolTxAdvanced\"}]},{\"type\":\"tabview_child\",\"index\":3,\"tab_name\":\"Details\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowSolTxDetail\"}]}]}"

#define GUI_SOL_MESSAGE_TEMPLATE "{\"table\":{\"utf8_message\":{\"type\":\"custom_container\",\"pos\":[0,39],\"size\":[408,500],\"align\":2,\"bg_color\":16777215,\"bg_opa\":31,\"radius\":24,\"custom_show_func\":\"GuiShowSolMessageReview\"},\"raw_message\":{\"type\":\"custom_container\",\"pos\":[0,39],\"size\":[408,500],\"align\":2,\"bg_color\":16777215,\"bg_opa\":31,\"radius\":24,\"custom_show_func\":\"GuiShowSolMessageReview\"}}}"

#define GUI_ADA_PAGE_TEMPLATE "{\"type\":\"custom_container\",\"pos\":[36,0],\"size\":[408,542],\"bg_color\":0,\"bg_opa\":0,\"custom_show_func\":\"GuiShowAdaTx\"}"

#ifndef COMPILE_SIMULATOR

#include "gui_trx_template.h"

#define GUI_ANALYZE_OBJ_SURPLUS \
    {\
        REMAPVIEW_ETH,\
        "{\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,900],\"bg_color\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiEthTxOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiEthTxDetails\"}]}]}", \
        GuiGetEthData,\
        NULL,\
        FreeEthMemory,\
    },\
    {\
        REMAPVIEW_ETH_PERSONAL_MESSAGE,\
        "{\"table\":{\"utf8_message\":{\"type\":\"container\",\"pos\":[0,39],\"size\":[408,500],\"align\":2,\"bg_color\":16777215,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"From\",\"pos\":[24,16],\"exist_func\":\"GetEthMessageFromExist\",\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetMessageFrom\",\"pos\":[24,54],\"text_width\":360,\"exist_func\":\"GetEthMessageFromExist\",\"font\":\"openSansEnIllustrate\"},{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"exist_func\":\"GetEthMessageFromNotExist\",\"pos\":[0,0],\"custom_show_func\":\"GuiCustomPathNotice\"},{\"type\":\"label\",\"text\":\"Message\",\"pos_func\":\"GetEthMessagePos\",\"align_to\":-2,\"align\":13,\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"container\",\"size\":[360,332],\"pos\":[0,11],\"align_to\":-2,\"align\":13,\"aflag\":16,\"bg_opa\":0,\"children\":[{\"type\":\"label\",\"text_func\":\"GetMessageUtf8\",\"pos\":[0,0],\"text_width\":360,\"font\":\"openSansEnIllustrate\",\"text_color\":16777215}]}]},\"raw_message\":{\"type\":\"container\",\"pos\":[0,39],\"size\":[408,500],\"align\":2,\"bg_color\":16777215,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Raw Message\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"container\",\"pos\":[24,54],\"size\":[360,450],\"align\":1,\"aflag\":16,\"bg_opa\":0,\"children\":[{\"type\":\"label\",\"text_func\":\"GetMessageRaw\",\"pos\":[0,0],\"text_width\":360,\"font\":\"illustrate\"}]}]}}}", \
        GuiGetEthPersonalMessage,\
        GetEthPersonalMessageType,\
        FreeEthMemory,\
    },\
    {\
        REMAPVIEW_ETH_TYPEDDATA,\
        "{\"name\":\"eth_page\",\"type\":\"tabview\",\"pos\":[24,0],\"size\":[408,900],\"bg_color\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"container\",\"pos\":[0,0],\"size\":[480,542],\"align\":0,\"bg_opa\":0,\"aflag\":16,\"children\":[{\"exist_func\":\"GetEthPermitWarningExist\",\"type\":\"container\",\"pos\":[36,24],\"size\":[408,152],\"align\":0,\"bg_color\":16078897,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"img\",\"pos\":[24,24],\"img_src\":\"imgWarningRed\"},{\"type\":\"label\",\"text\":\"WARNING\",\"pos\":[68,24],\"font\":\"openSansEnText\",\"text_color\":16078897},{\"type\":\"label\",\"text\":\"sign_eth_permit_warn\",\"pos\":[24,68],\"text_color\":16777215,\"font\":\"illustrate\",\"text_width\":360}]},{\"type\":\"container\",\"size_func\":\"GetEthTypeDomainSize\",\"pos_func\":\"GetEthTypeDomainPos\",\"align\":0,\"bg_color\":16777215,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"DomainName\",\"pos\":[24,16],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthTypedDataDomianName\",\"pos\":[24,54],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Signer\",\"pos\":[0,16],\"align_to\":-2,\"align\":13,\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetEthTypeDataHashExist\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthGetSignerAddress\",\"pos\":[0,8],\"exist_func\":\"GetEthTypeDataHashExist\",\"text_width\":360,\"align_to\":-2,\"align\":13,\"text_color\":16777215,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Version\",\"exist_func\":\"GetEthTypeDataVersionExist\",\"pos\":[0,16],\"align_to\":-2,\"align\":13,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"exist_func\":\"GetEthTypeDataVersionExist\",\"text_func\":\"GetEthTypedDataDomianVersion\",\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"text_color\":16777215,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"chainId\",\"exist_func\":\"GetEthTypeDataChainExist\",\"pos\":[0,16],\"align_to\":-2,\"align\":13,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"exist_func\":\"GetEthTypeDataChainExist\",\"text_func\":\"GetEthTypedDataDomianChainId\",\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"text_color\":16777215,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"VerifyingContract\",\"pos\":[0,16],\"align_to\":-2,\"align\":13,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthTypedDataDomianVerifyContract\",\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"font\":\"openSansEnIllustrate\",\"text_width\":360},{\"type\":\"label\",\"text\":\"PrimaryType\",\"pos\":[0,16],\"align_to\":-2,\"align\":13,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthTypedDataPrimayType\",\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"size\":[408,415],\"pos\":[0,16],\"align_to\":-2,\"align\":13,\"exist_func\":\"GetEthTypeDataHashExist\",\"bg_color\":16777215,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"DomainHash\",\"pos\":[24,16],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthTypedDataDomainHash\",\"pos\":[24,54],\"text_width\":360,\"text_color\":16777215,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"MessageHash\",\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthTypedDataMessageHash\",\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"text_color\":16777215,\"text_width\":360,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"SafeTxHash\",\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthTypedDataSafeTxHash\",\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"text_color\":16777215,\"text_width\":360,\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"label\",\"text\":\"Message\",\"pos\":[0,16],\"align_to\":-2,\"align\":13,\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"container\",\"size\":[408,415],\"pos\":[0,16],\"align_to\":-2,\"align\":13,\"bg_color\":16777215,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"json_label\",\"text_func\":\"GetEthTypedDataMessage\",\"text_len_func\":\"GetEthTypedDataMessageLen\",\"pos\":[0,16],\"align_to\":-2,\"align\":13,\"radius\":24,\"text_color\":16777215,\"font\":\"openSansEnIllustrate\"}]},{\"exist_func\":\"GetEthPermitCantSign\",\"type\":\"container\",\"pos\":[0,16],\"align_to\":-2,\"align\":13,\"size\":[408,182],\"bg_color\":16777215,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"img\",\"pos\":[24,24],\"img_src\":\"imgWarningRed\"},{\"type\":\"label\",\"text\":\"Cant'tSignitNow\",\"pos\":[68,24],\"font\":\"openSansEnText\",\"text_color\":16078897},{\"type\":\"label\",\"text\":\"sign_eth_permit_deny_sing\",\"pos\":[24,68],\"text_color\":16777215,\"font\":\"illustrate\",\"text_width\":360}]}]}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"RawData\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"container\",\"pos\":[0,24],\"size\":[408,440],\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Message\",\"pos\":[24,16],\"text_opa\":144},{\"type\":\"textarea\",\"text_func\":\"GetEthTypedDataMessage\",\"text_len_func\":\"GetEthTypedDataMessageLen\",\"text_width\":360,\"bg_opa\":0,\"pos\":[24,74]}]}]}]}", \
        GuiGetEthTypeData,\
        NULL,\
        FreeEthMemory,\
    },\
    {\
        REMAPVIEW_TRX,\
        GUI_TRX_PAGE_TEMPLATE, \
        GuiGetTrxData,\
        NULL,\
        FreeTrxMemory,\
    },\
    {\
        REMAPVIEW_TRX_PERSONAL_MESSAGE,\
        GUI_TRX_PERSONAL_MESSAGE_TEMPLATE, \
        GuiGetTrxPersonalMessage,\
        GetTrxPersonalMessageType,\
        FreeTrxMemory,\
    },\
    {\
        REMAPVIEW_TRX_SWAP,\
        GUI_TRX_SWAP_TEMPLATE,\
        GuiGetTrxData,\
        NULL,\
        FreeTrxMemory,\
    },\
    {\
        REMAPVIEW_COSMOS,\
        "{\"table\":{\"tx\":{\"name\":\"cosmos_tx_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,530],\"bg_color\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiCosmosTxOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiCosmosTxDetails\"}]}]},\"unknown\":{\"name\":\"cosmos_unknown_page\",\"type\":\"container\",\"pos\":[36,0],\"size\":[408,600],\"bg_color\":0,\"children\":[{\"type\":\"container\",\"pos\":[0,80],\"size\":[408,100],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Fee\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Fee\",\"pos\":[73,16],\"text_width\":2000,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Gas Limit\",\"pos\":[24,54],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Gas Limit\",\"pos\":[127,54],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,62],\"bg_opa\":31,\"radius\":24,\"align\":13,\"align_to\":-2,\"children\":[{\"type\":\"label\",\"text\":\"Network\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Network\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,62],\"bg_opa\":31,\"radius\":24,\"align\":13,\"align_to\":-2,\"children\":[{\"type\":\"label\",\"text\":\"Message\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Message\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\",\"text_color\":16105777}]}]},\"msg\":{\"name\":\"cosmos_msg_page\",\"type\":\"container\",\"pos\":[36,0],\"size\":[408,600],\"bg_color\":0,\"children\":[{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,62],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Network\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Network\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,130],\"bg_opa\":31,\"radius\":24,\"align\":13,\"align_to\":-2,\"children\":[{\"type\":\"label\",\"text\":\"Signer\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Signer\",\"text_width\":360,\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,250],\"bg_opa\":31,\"radius\":24,\"align\":13,\"align_to\":-2,\"children\":[{\"type\":\"label\",\"text\":\"Message\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Message\",\"pos\":[24,54],\"font\":\"openSansEnIllustrate\",\"text_width\":360}]}]}}}", \
        GuiGetCosmosData,\
        GuiGetCosmosTmpType,\
        FreeCosmosMemory,\
    },\
    {\
        REMAPVIEW_SUI,\
        "{\"type\":\"container\",\"pos\":[36,0],\"size\":[408,526],\"bg_opa\":0,\"children\":[{\"type\":\"label\",\"text\":\"Transaction Raw Data\",\"text_width\":360,\"text_opa\":144,\"pos\":[0,0],\"font\":\"openSansEnIllustrate\"},{\"type\":\"container\",\"pos\":[0,38],\"size\":[408,488],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetSuiDetail\",\"text_len_func\":\"GetSuiDetailLen\",\"text_width\":360,\"pos\":[24,24],\"font\":\"openSansEnIllustrate\"}]}]}",\
        GuiGetSuiData,\
        NULL,\
        FreeSuiMemory,\
    },\
    {\
        REMAPVIEW_IOTA,\
        "{\"name\":\"iota_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,900],\"bg_color\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetIotaShowOverview\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiIotaTxOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Raw Data\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetIotaIsTransaction\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiIotaTxRawData\"}]},{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"exist_func\":\"GetIotaIsMessage\",\"pos\":[24,12],\"custom_show_func\":\"GuiIotaTxOverview\"}]}", \
        GuiGetIotaData,\
        NULL,\
        FreeIotaMemory,\
    },\
    {REMAPVIEW_IOTA_SIGN_MESSAGE_HASH,\
        "{\"name\":\"iota_sign_hash_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,774],\"bg_color\":0,\"border_width\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowSuiSignMessageHashOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowSuiSignMessageHashDetails\"}]}]}",\
        GuiGetIotaSignMessageHashData,\
        NULL,\
        FreeIotaMemory},\
    {REMAPVIEW_SUI_SIGN_MESSAGE_HASH,\
        "{\"name\":\"sui_sign_hash_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,774],\"bg_color\":0,\"border_width\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowSuiSignMessageHashOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowSuiSignMessageHashDetails\"}]}]}",\
        GuiGetSuiSignMessageHashData,\
        NULL,\
        FreeSuiMemory},\
    {\
        REMAPVIEW_SOL,\
        GUI_SOL_PAGE_TEMPLATE,\
        GuiGetSolData,\
        NULL,\
        FreeSolMemory,\
    },\
    {\
        REMAPVIEW_SOL_MESSAGE,\
        GUI_SOL_MESSAGE_TEMPLATE, \
        GuiGetSolMessageData,\
        GetSolMessageType,\
        FreeSolMemory,\
    },\
    {\
        REMAPVIEW_APT,\
        "{\"type\":\"container\",\"pos\":[36,0],\"size\":[408,526],\"bg_opa\":0,\"children\":[{\"type\":\"label\",\"text\":\"Transaction Raw Data\",\"text_width\":360,\"text_opa\":144,\"pos\":[0,0],\"font\":\"openSansEnIllustrate\"},{\"type\":\"container\",\"pos\":[0,38],\"size\":[408,488],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetAptosDetail\",\"text_len_func\":\"GetAptosDetailLen\",\"text_width\":360,\"pos\":[24,24],\"font\":\"openSansEnIllustrate\"}]}]}",\
        GuiGetAptosData,\
        NULL,\
        FreeAptosMemory,\
    },\
    {\
        REMAPVIEW_ADA,\
        GUI_ADA_PAGE_TEMPLATE,\
        GuiGetAdaData,\
        NULL,\
        FreeAdaMemory,\
    },\
    {\
        REMAPVIEW_ADA_SIGN_TX_HASH,\
        "{\"name\":\"ada_sign_tx_hash_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,774],\"bg_color\":0,\"border_width\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowAdaSignTxHashOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowAdaSignTxHashDetails\"}]}]}",\
        GuiGetAdaSignTxHashData,\
        NULL,\
        FreeAdaSignTxHashMemory},\
    {\
        REMAPVIEW_ADA_SIGN_DATA,\
        "{\"type\":\"custom_container\",\"pos\":[36,0],\"size\":[408,542],\"bg_color\":0,\"bg_opa\":0,\"custom_show_func\":\"GuiShowAdaSignData\"}",\
        GuiGetAdaSignDataData,\
        NULL,\
        FreeAdaSignDataMemory,\
    },\
    {\
        REMAPVIEW_ADA_CATALYST,\
        "{\"name\":\"ada_catalyst_page\",\"type\":\"container\",\"pos\":[36,0],\"size\":[408,542],\"bg_color\":0,\"children\":[{\"type\":\"container\",\"pos\":[0,0],\"size\":[408,62],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text\":\"Catalyst Key Registration\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,78],\"size\":[408,62],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Nonce\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCatalystNonce\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,156],\"size\":[408,160],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"StakeKey\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCatalystVotePublicKey\",\"text_width\":360,\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,332],\"size\":[408,258],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"RewardsGoTo\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCatalystRewards\",\"text_width\":360,\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"},{\"type\":\"custom_container\",\"pos\":[24,182],\"custom_show_func\":\"GetCatalystRewardsNotice\"}]},{\"type\":\"container\",\"pos\":[0,606],\"size_func\":\"GetCatalystVoteKeysSize\",\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"VoteKey\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCatalystVoteKeys\",\"text_width\":360,\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"}]}]}",\
        GuiGetAdaCatalyst,\
        NULL,\
        FreeAdaCatalystMemory,\
    },\
    {\
        REMAPVIEW_XRP,\
        "{\"type\":\"container\",\"pos\":[36,0],\"size\":[408,526],\"bg_opa\":0,\"children\":[{\"type\":\"label\",\"text\":\"Transaction Raw Data\",\"text_width\":360,\"text_opa\":144,\"pos\":[0,0],\"font\":\"openSansEnIllustrate\"},{\"type\":\"container\",\"pos\":[0,38],\"size\":[408,488],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetXrpDetail\",\"text_len_func\":\"GetXrpDetailLen\",\"text_width\":360,\"pos\":[24,24],\"font\":\"openSansEnIllustrate\"}]}]}",\
        GuiGetXrpData,\
        NULL,\
        FreeXrpMemory,\
    },\
    {\
        REMAPVIEW_AR,\
        "{\"name\":\"ar_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,530],\"bg_color\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiArTxOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiArTxDetails\"}]}]}",\
        GuiGetArData,\
        NULL,\
        FreeArMemory,\
    },\
    {\
        REMAPVIEW_AR_MESSAGE,\
        "{\"name\":\"ar_message_page\",\"type\":\"container\",\"pos\":[36,0],\"size\":[408,542],\"bg_color\":0,\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,0],\"custom_show_func\":\"GuiArMessageOverview\"}]}",\
        GuiGetArData,\
        NULL,\
        FreeArMemory,\
    },\
    {\
        REMAPVIEW_STELLAR,\
        "{\"name\":\"ar_message_page\",\"type\":\"container\",\"pos\":[36,0],\"size\":[408,542],\"bg_color\":0,\"children\":[{\"type\":\"custom_container\",\"size\":[408,212],\"pos\":[0,0],\"radius\":24,\"custom_show_func\":\"GuiStellarTxNotice\"},{\"type\":\"container\",\"size\":[408,310],\"pos\":[0,236],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"XDR\",\"text_color\":16090890,\"pos\":[24,16],\"size\":[408,130],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text_func\":\"GetStellarRawMessage\",\"text_len_func\":\"GetStellarRawMessageLength\",\"text_width\":360,\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"}]}]}",\
        GuiGetStellarData,\
        NULL,\
        FreeStellarMemory,\
    },\
    {\
        REMAPVIEW_STELLAR_HASH,\
        "{\"name\":\"ar_message_page\",\"type\":\"container\",\"pos\":[36,0],\"size\":[408,542],\"bg_color\":0,\"children\":[{\"type\":\"custom_container\",\"size\":[408,260],\"pos\":[0,0],\"radius\":24,\"custom_show_func\":\"GuiStellarHashNotice\"},{\"type\":\"container\",\"size\":[408,130],\"pos\":[0,284],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Hash\",\"text_color\":16090890,\"pos\":[24,16],\"size\":[408,130],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text_func\":\"GetStellarRawMessage\",\"text_len_func\":\"GetStellarRawMessageLength\",\"text_width\":360,\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"}]}]}",\
        GuiGetStellarData,\
        NULL,\
        FreeStellarMemory,\
    },\
    {\
        REMAPVIEW_AR_DATAITEM,\
        "{\"name\":\"ar_data_item_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,530],\"bg_color\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiArDataItemOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Additions\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiArDataItemDetail\"}]}]}",\
        GuiGetArData,\
        NULL,\
        FreeArMemory,\
    },\
    {\
        REMAPVIEW_TON,\
        "{\"name\":\"ton_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,900],\"bg_color\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiTonTxOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Raw Data\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiTonTxRawData\"}]}]}",\
        GuiGetTonGUIData,\
        NULL,\
        FreeArMemory,\
    },\
    {\
        REMAPVIEW_TON_SIGNPROOF,\
        "{\"name\":\"btc_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,774],\"bg_color\":0,\"border_width\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiTonProofOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Raw Data\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiTonProofRawData\"}]}]}",\
        GuiGetTonProofGUIData,\
        NULL,\
        FreeArMemory,\
    },\
    {\
        REMAPVIEW_AVAX,\
        "{\"name\":\"avax_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,530],\"bg_color\":0,\"border_width\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiAvaxTxOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiAvaxTxRawData\"}]}]}",\
        GuiGetAvaxGUIData,\
        NULL,\
        FreeAvaxMemory,\
    },\
    { \
        REMAPVIEW_ZCASH, \
        "{\"name\":\"zcash_page\",\"type\":\"custom_container\",\"pos\":[36,0],\"size\":[408,900],\"bg_color\":0,\"custom_show_func\":\"GuiZcashOverview\"}", \
        GuiGetZcashGUIData, \
        NULL, \
        FreeZcashMemory, \
    }
#endif
#endif
#endif
