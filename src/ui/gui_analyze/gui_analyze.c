#include <stdio.h>
#include <stdlib.h>
#include "cjson/cJSON.h"
#include "lvgl.h"
#include "gui_analyze.h"
#include "gui_chain.h"
#include "gui_model.h"

#ifndef COMPILE_SIMULATOR
#include "safe_mem_lib.h"
#endif

#define PC_SIMULATOR_PATH "C:/assets"
#define OBJ_VECTOR_MAX_LEN 5

typedef void (*SetLvglFlagFunc)(lv_obj_t *obj, lv_obj_flag_t);

#include "gui_resource.h"
#include "gui.h"
#include "librust_c.h"
#include "user_memory.h"
typedef struct {
    GuiRemapViewType index;
    const char *config;
    GetChainDataFunc func;
    GetLabelDataFunc typeFunc;
    FreeChainDataFunc freeFunc;
} GuiAnalyze_t;

#define GUI_ANALYZE_TABVIEW_CNT 2
typedef struct {
    lv_obj_t *obj[GUI_ANALYZE_TABVIEW_CNT];
    lv_obj_t *img[GUI_ANALYZE_TABVIEW_CNT];
    uint8_t tabviewIndex;
} GuiAnalyzeTabview_t;
static GuiAnalyzeTabview_t g_analyzeTabview;

typedef struct {
    void *totalPtr;
    uint8_t col;
    uint8_t row;
} GuiAnalyzeTable_t;

const static GuiAnalyze_t g_analyzeArray[] = {
    {
        REMAPVIEW_BTC,
#ifndef COMPILE_SIMULATOR
        "{\"name\":\"btc_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,774],\"bg_color\":0,\"border_width\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiBtcTxOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiBtcTxDetail\"}]}]}",
#else
        PC_SIMULATOR_PATH "/page_btc.json",
#endif
        GuiGetParsedQrData,
        NULL,
        FreePsbtUxtoMemory,
    },
    {
        REMAPVIEW_BTC_MESSAGE,
#ifndef COMPILE_SIMULATOR
        "{\"type\":\"container\",\"pos\":[36,0],\"size\":[408,526],\"bg_opa\":0,\"children\":[{\"type\":\"label\",\"text\":\"Message\",\"text_width\":360,\"text_opa\":144,\"pos\":[0,0],\"font\":\"openSansEnIllustrate\"},{\"type\":\"container\",\"pos\":[0,38],\"size\":[408,488],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetBtcMsgDetail\",\"text_len_func\":\"GetBtcMsgDetailLen\",\"text_width\":360,\"pos\":[24,24],\"font\":\"openSansEnIllustrate\"}]}]}",
#else
        PC_SIMULATOR_PATH "/page_btc_msg.json",
#endif
        GuiGetParsedQrData,
        NULL,
        FreeBtcMsgMemory,
    },
#ifndef BTC_ONLY
    // temper test the ethereum page view not for production usage
    {
        REMAPVIEW_ETH,
#ifndef COMPILE_SIMULATOR
        // ethJson,
        "{\"name\":\"eth_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,900],\"bg_color\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"container\",\"pos\":[0,12],\"size\":[408,152],\"bg_opa\":51,\"radius\":24,\"bg_color\":16078897,\"exist_func\":\"GetErc20WarningExist\",\"children\":[{\"type\":\"img\",\"pos\":[24,24],\"img_src\":\"imgWarningRed\"},{\"type\":\"label\",\"text\":\"WARNING\",\"pos\":[68,24],\"font\":\"openSansEnText\",\"text_color\":16078897},{\"type\":\"label\",\"text\":\"unknown_erc20_warning\",\"pos\":[24,68],\"text_color\":16777215,\"font\":\"illustrate\",\"text_width\":360}]},{\"type\":\"container\",\"pos\":[0,12],\"size\":[408,144],\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text\":\"Max Txn Fee\",\"pos\":[24,98],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthValue\",\"pos\":[24,50],\"text_color\":16090890,\"font\":\"openSansEnLittleTitle\"},{\"type\":\"label\",\"text_func\":\"GetEthTxFee\",\"pos\":[156,98],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,62],\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Network\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthNetWork\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size_func\":\"GetEthToFromSize\",\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"From\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthGetFromAddress\",\"text_width\":360,\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"},{\"type\":\"img\",\"pos\":[24,129],\"exist_func\":\"GetEthEnsExist\",\"img_src\":\"imgEns\"},{\"type\":\"label\",\"text_func\":\"GetEthEnsName\",\"exist_func\":\"GetEthEnsExist\",\"pos\":[56,126],\"font\":\"openSansEnIllustrate\",\"text_color\":1827014},{\"type\":\"label\",\"text\":\"To\",\"pos_func\":\"GetEthToLabelPos\",\"text_opa\":144,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text_func\":\"GetEthGetToAddress\",\"text_width\":360,\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"font\":\"openSansEnIllustrate\"},{\"type\":\"img\",\"pos\":[0,11],\"align_to\":-2,\"align\":13,\"exist_func\":\"GetToEthEnsExist\",\"img_src\":\"imgEns\"},{\"type\":\"label\",\"text_func\":\"GetToEthEnsName\",\"exist_func\":\"GetToEthEnsExist\",\"pos\":[8,0],\"align_to\":-2,\"align\":20,\"font\":\"openSansEnIllustrate\",\"text_color\":1827014},{\"type\":\"img\",\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"exist_func\":\"GetEthContractDataExist\",\"img_src\":\"imgContract\"},{\"type\":\"label\",\"text_func\":\"GetEthContractName\",\"exist_func\":\"GetEthContractDataExist\",\"pos\":[38,8],\"align_to\":-3,\"align\":13,\"font\":\"openSansEnIllustrate\",\"text_color\":10782207}]}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"table\":{\"FeeMarket\":{\"type\":\"container\",\"pos\":[0,12],\"size\":[408,316],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthValue\",\"pos\":[92,16],\"text_color\":16090890,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Max Fee\",\"pos\":[24,54],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthMaxFee\",\"pos\":[118,54],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"·   Max Fee Price * Gas Limit\",\"pos\":[24,92],\"text_opa\":144,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Max Priority\",\"pos\":[24,124],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthMaxPriority\",\"pos\":[153,124],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"·   Max Priority Fee Price * Gas Limit\",\"pos\":[24,162],\"text_opa\":144,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Max Fee Price\",\"pos\":[24,194],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthMaxFeePrice\",\"pos\":[169,194],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Max Priority Fee Price\",\"pos\":[24,232],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthMaxPriorityFeePrice\",\"pos\":[242,232],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Gas Limit\",\"pos\":[24,270],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthGasLimit\",\"pos\":[127,270],\"font\":\"openSansEnIllustrate\"}]},\"legacy\":{\"type\":\"container\",\"pos\":[0,12],\"size\":[408,208],\"align\":2,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthValue\",\"pos\":[92,16],\"text_color\":16090890,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Max Txn Fee\",\"pos\":[24,54],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthTxFee\",\"pos\":[156,54],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text_func\":\"GetTxnFeeDesc\",\"pos\":[24,92],\"text_opa\":144,\"font\":\"openSansDesc\"},{\"type\":\"label\",\"text\":\"Gas Price\",\"pos\":[24,124],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthGasPrice\",\"pos\":[127,124],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Gas Limit\",\"pos\":[24,162],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthGasLimit\",\"pos\":[127,162],\"font\":\"openSansEnIllustrate\"}]}}},{\"type\":\"container\",\"pos\":[16,16],\"size\":[408,62],\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Network\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthNetWork\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"exist_func\":\"GetEthContractDataExist\",\"pos\":[0,16],\"size\":[408,62],\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthMethodName\",\"pos\":[113,16],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size_func\":\"GetEthToFromSize\",\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"From\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthGetFromAddress\",\"text_width\":360,\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"},{\"type\":\"img\",\"pos\":[24,129],\"exist_func\":\"GetEthEnsExist\",\"img_src\":\"imgEns\"},{\"type\":\"label\",\"text_func\":\"GetEthEnsName\",\"exist_func\":\"GetEthEnsExist\",\"pos\":[56,126],\"font\":\"openSansEnIllustrate\",\"text_color\":1827014},{\"type\":\"label\",\"text\":\"To\",\"pos_func\":\"GetEthToLabelPos\",\"text_opa\":144,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text_func\":\"GetEthGetToAddress\",\"text_width\":360,\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"font\":\"openSansEnIllustrate\"},{\"type\":\"img\",\"pos\":[0,11],\"align_to\":-2,\"align\":13,\"exist_func\":\"GetToEthEnsExist\",\"img_src\":\"imgEns\"},{\"type\":\"label\",\"text_func\":\"GetToEthEnsName\",\"exist_func\":\"GetToEthEnsExist\",\"pos\":[8,0],\"align_to\":-2,\"align\":20,\"font\":\"openSansEnIllustrate\",\"text_color\":1827014},{\"type\":\"img\",\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"exist_func\":\"GetEthContractDataExist\",\"img_src\":\"imgContract\"},{\"type\":\"label\",\"text_func\":\"GetEthContractName\",\"exist_func\":\"GetEthContractDataExist\",\"pos\":[38,8],\"align_to\":-3,\"align\":13,\"font\":\"openSansEnIllustrate\",\"text_color\":10782207}]},{\"type\":\"label\",\"text\":\"Input Data\",\"align_to\":-2,\"align\":13,\"exist_func\":\"GetEthInputDataExist\",\"pos\":[0,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"container\",\"pos\":[0,16],\"size_func\":\"GetEthContractDataSize\",\"exist_func\":\"GetEthInputDataExist\",\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"exist_func\":\"GetEthContractDataNotExist\",\"text_func\":\"GetEthTransactionData\",\"text_width\":360,\"pos\":[24,16],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"exist_func\":\"GetEthContractDataNotExist\",\"text\":\"Unknown Contract\",\"text_width\":360,\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"text_color\":16105777,\"font\":\"openSansEnIllustrate\"},{\"type\":\"container\",\"exist_func\":\"GetEthContractDataNotExist\",\"aflag\":2,\"cb\":\"EthContractLearnMore\",\"pos\":[0,8],\"size\":[144,30],\"align_to\":-2,\"align\":13,\"bg_color\":1907997,\"children\":[{\"type\":\"label\",\"text\":\"Learn More\",\"text_width\":360,\"pos\":[0,0],\"text_color\":1827014,\"font\":\"openSansEnIllustrate\"},{\"type\":\"img\",\"img_src\":\"imgQrcodeTurquoise\",\"pos\":[120,3],\"text_color\":3056500,\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"label\",\"exist_func\":\"GetEthContractDataExist\",\"text\":\"Method\",\"pos\":[24,16],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"exist_func\":\"GetEthContractDataExist\",\"text_func\":\"GetEthMethodName\",\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"},{\"name\":\"contract_data\",\"type\":\"table\",\"width\":360,\"align\":2,\"pos\":[0,100],\"bg_color\":1907997,\"key_width\":30,\"table_func\":\"GetEthContractData\",\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetEthContractDataExist\"}]}]}]}",
#else
        PC_SIMULATOR_PATH "/page_eth.json",
#endif
        GuiGetEthData,
        GetEthTransType,
        FreeEthMemory,
    },
    {
        REMAPVIEW_ETH_PERSONAL_MESSAGE,
#ifndef COMPILE_SIMULATOR
        "{\"table\":{\"utf8_message\":{\"type\":\"container\",\"pos\":[0,39],\"size\":[408,500],\"align\":2,\"bg_color\":16777215,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"From\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetMessageFrom\",\"pos\":[24,54],\"text_width\":360,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Message\",\"pos\":[24,130],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"container\",\"pos\":[24,168],\"size\":[360,332],\"align\":1,\"aflag\":16,\"bg_opa\":0,\"children\":[{\"type\":\"label\",\"text_func\":\"GetMessageUtf8\",\"pos\":[0,0],\"text_width\":360,\"font\":\"openSansEnIllustrate\",\"text_color\":16777215}]}]},\"raw_message\":{\"type\":\"container\",\"pos\":[0,39],\"size\":[408,500],\"align\":2,\"bg_color\":16777215,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Raw Message\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"container\",\"pos\":[24,54],\"size\":[360,450],\"align\":1,\"aflag\":16,\"bg_opa\":0,\"children\":[{\"type\":\"label\",\"text_func\":\"GetMessageRaw\",\"pos\":[0,0],\"text_width\":360,\"font\":\"openSansEnIllustrate\"}]}]}}}",
#else
        PC_SIMULATOR_PATH "/page_eth_person.json",
#endif
        GuiGetEthPersonalMessage,
        GetEthPersonalMessageType,
        FreeEthMemory,
    },
    {
        REMAPVIEW_ETH_TYPEDDATA,
#ifndef COMPILE_SIMULATOR
        "{\"type\":\"container\",\"pos\":[0,0],\"size\":[480,542],\"align\":0,\"bg_opa\":0,\"aflag\":16,\"children\":[{\"type\":\"container\",\"pos\":[36,24],\"size\":[408,298],\"align\":0,\"bg_color\":16777215,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Domain Name\",\"pos\":[24,16],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthTypedDataDomianName\",\"pos\":[24,54],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Verifying Contract\",\"pos\":[24,100],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthTypedDataDomianVerifyContract\",\"pos\":[24,138],\"text_color\":16777215,\"text_width\":360,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Primary Type\",\"pos\":[24,214],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthTypedDataPrimayType\",\"pos\":[24,252],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"label\",\"text\":\"Message\",\"pos\":[36,354],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetEthTypedDataMessage\",\"text_len_func\":\"GetEthTypedDataMessageLen\",\"pos\":[36,396],\"width\":408,\"bg_color\":16777215,\"bg_opa\":31,\"pad_vertical\":16,\"pad_horizontal\":24,\"radius\":24,\"text_color\":16777215,\"font\":\"openSansEnIllustrate\"}]}",
#else
        PC_SIMULATOR_PATH "/page_eth.json",
#endif
        GuiGetEthTypeData,
        NULL,
        FreeEthMemory,
    },
    {
        REMAPVIEW_TRX,
#ifndef COMPILE_SIMULATOR
        "{\"name\":\"trx_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,900],\"bg_color\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"container\",\"pos\":[0,12],\"size\":[408,106],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetTrxValue\",\"pos\":[24,50],\"text_color\":16090890,\"font\":\"openSansEnLittleTitle\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,62],\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetTrxMethod\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,244],\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"From\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetTrxFromAddress\",\"text_width\":360,\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"To\",\"pos\":[24,130],\"text_opa\":144,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text_func\":\"GetTrxToAddress\",\"text_width\":360,\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"font\":\"openSansEnIllustrate\"}]}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"container\",\"pos\":[0,12],\"size\":[408,62],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetTrxValue\",\"pos\":[92,16],\"text_color\":16090890,\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,62],\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetTrxMethod\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,244],\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"From\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetTrxFromAddress\",\"text_width\":360,\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"To\",\"pos\":[24,130],\"text_opa\":144,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text_func\":\"GetTrxToAddress\",\"text_width\":360,\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,130],\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"exist_func\":\"GetTrxContractExist\",\"children\":[{\"type\":\"label\",\"text\":\"Contract Address\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetTrxContract\",\"pos\":[24,54],\"font\":\"openSansEnIllustrate\",\"text_width\":360}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,62],\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"exist_func\":\"GetTrxTokenExist\",\"children\":[{\"type\":\"label\",\"text\":\"Token ID\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetTrxToken\",\"pos\":[123,16],\"font\":\"openSansEnIllustrate\"}]}]}]}",
#else
        PC_SIMULATOR_PATH "/page_eth.json",
#endif
        GuiGetTrxData,
        NULL,
        FreeTrxMemory,
    },
    {
        REMAPVIEW_COSMOS,
#ifndef COMPILE_SIMULATOR
        "{\"table\":{\"tx\":{\"name\":\"cosmos_tx_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,900],\"bg_color\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"list\",\"exist_func\":\"GetCosmosMsgListExist\",\"len_func\":\"GetCosmosMsgLen\",\"item_key_func\":\"GetCosmosMsgKey\",\"item_map\":{\"default\":{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,402],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetCosmosIndex\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_color\":16090890},{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,62],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Value\",\"pos\":[24,96],\"text_color\":16090890,\"text_width\":2000,\"font\":\"openSansEnLittleTitle\"},{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,144],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Method\",\"pos\":[113,144],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"From\",\"pos\":[24,182],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"From\",\"text_width\":360,\"pos\":[24,220],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"To\",\"pos\":[24,288],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"To\",\"text_width\":360,\"pos\":[24,326],\"font\":\"openSansEnIllustrate\"}]},\"Undelegate\":{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,402],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetCosmosIndex\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_color\":16090890},{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,62],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Value\",\"pos\":[24,96],\"text_color\":16090890,\"font\":\"openSansEnLittleTitle\"},{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,144],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Method\",\"pos\":[113,144],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Validator\",\"pos\":[24,182],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Validator\",\"text_width\":360,\"pos\":[24,220],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"To\",\"pos\":[24,288],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"To\",\"text_width\":360,\"pos\":[24,326],\"font\":\"openSansEnIllustrate\"}]},\"Re-delegate\":{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,402],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetCosmosIndex\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_color\":16090890},{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,62],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Value\",\"pos\":[24,96],\"text_color\":16090890,\"font\":\"openSansEnLittleTitle\"},{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,144],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Method\",\"pos\":[113,144],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"To\",\"pos\":[24,182],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"To\",\"text_width\":360,\"pos\":[24,220],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"New Validator\",\"pos\":[24,288],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"New Validator\",\"text_width\":360,\"pos\":[24,326],\"font\":\"openSansEnIllustrate\"}]},\"Withdraw Reward\":{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,320],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetCosmosIndex\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_color\":16090890},{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,62],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Method\",\"pos\":[113,62],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"To\",\"pos\":[24,100],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"To\",\"text_width\":360,\"pos\":[24,138],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Validator\",\"pos\":[24,206],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Validator\",\"text_width\":360,\"pos\":[24,244],\"font\":\"openSansEnIllustrate\"}]},\"Vote\":{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,290],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetCosmosIndex\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_color\":16090890},{\"type\":\"label\",\"text\":\"Proposal\",\"pos\":[24,62],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Proposal\",\"pos\":[123,62],\"font\":\"openSansEnIllustrate\",\"text_color\":16090890},{\"type\":\"label\",\"text\":\"Voted\",\"pos\":[24,100],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Voted\",\"pos\":[95,100],\"font\":\"openSansEnIllustrate\",\"text_color\":16090890},{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,138],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Method\",\"pos\":[113,138],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Voter\",\"pos\":[24,176],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Voter\",\"pos\":[24,214],\"text_width\":360,\"font\":\"openSansEnIllustrate\"}]}}},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,106],\"bg_opa\":31,\"radius\":24,\"exist_func\":\"GetCosmosValueExist\",\"children\":[{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosValue\",\"pos\":[24,50],\"text_color\":16090890,\"text_width\":2000,\"font\":\"openSansEnLittleTitle\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,106],\"bg_opa\":31,\"radius\":24,\"exist_func\":\"GetCosmosVoteExist\",\"children\":[{\"type\":\"label\",\"text\":\"Proposal\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosProposal\",\"pos\":[123,16],\"text_color\":16090890,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Voted\",\"pos\":[24,54],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosVoted\",\"pos\":[95,54],\"text_color\":16090890,\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,62],\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Network\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosNetwork\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,62],\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"exist_func\":\"GetCosmosMethodExist\",\"children\":[{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosMethod\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size_func\":\"GetCosmosOverviewAddrSize\",\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"exist_func\":\"GetCosmosAddrExist\",\"children\":[{\"type\":\"label\",\"text_func\":\"GetCosmosAddress1Label\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosAddress1Value\",\"text_width\":360,\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text_func\":\"GetCosmosAddress2Label\",\"pos\":[24,130],\"text_opa\":144,\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetCosmosAddress2Exist\"},{\"type\":\"label\",\"text_func\":\"GetCosmosAddress2Value\",\"text_width\":360,\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetCosmosAddress2Exist\"}]}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"list\",\"exist_func\":\"GetCosmosMsgListExist\",\"len_func\":\"GetCosmosMsgLen\",\"item_key_func\":\"GetCosmosMsgKey\",\"item_map\":{\"default\":{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,358],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetCosmosIndex\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_color\":16090890},{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,62],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Value\",\"pos\":[92,62],\"text_color\":16090890,\"text_width\":2000,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,100],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Method\",\"pos\":[113,100],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"From\",\"pos\":[24,138],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"From\",\"text_width\":360,\"pos\":[24,176],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"To\",\"pos\":[24,244],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"To\",\"text_width\":360,\"pos\":[24,282],\"font\":\"openSansEnIllustrate\"}]},\"IBC Transfer\":{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,396],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetCosmosIndex\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_color\":16090890},{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,62],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Value\",\"pos\":[92,62],\"text_color\":16090890,\"text_width\":2000,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,100],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Method\",\"pos\":[113,100],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"From\",\"pos\":[24,138],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"From\",\"text_width\":360,\"pos\":[24,176],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"To\",\"pos\":[24,244],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"To\",\"text_width\":360,\"pos\":[24,282],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Source Channel\",\"pos\":[24,350],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Source Channel\",\"text_width\":360,\"pos\":[187,350],\"font\":\"openSansEnIllustrate\"}]},\"Undelegate\":{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,358],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetCosmosIndex\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_color\":16090890},{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,62],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Value\",\"pos\":[92,62],\"text_color\":16090890,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,100],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Method\",\"pos\":[113,100],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Validator\",\"pos\":[24,138],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Validator\",\"text_width\":360,\"pos\":[24,176],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"To\",\"pos\":[24,244],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"To\",\"text_width\":360,\"pos\":[24,282],\"font\":\"openSansEnIllustrate\"}]},\"Re-delegate\":{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,464],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetCosmosIndex\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_color\":16090890},{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,62],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Value\",\"pos\":[92,62],\"text_color\":16090890,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,100],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Method\",\"pos\":[113,100],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"To\",\"pos\":[24,138],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"To\",\"text_width\":360,\"pos\":[24,176],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Old Validator\",\"pos\":[24,244],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Old Validator\",\"text_width\":360,\"pos\":[24,282],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"New Validator\",\"pos\":[24,350],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"New Validator\",\"text_width\":360,\"pos\":[24,388],\"font\":\"openSansEnIllustrate\"}]},\"Withdraw Reward\":{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,322],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetCosmosIndex\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_color\":16090890},{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,62],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Method\",\"pos\":[113,62],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"To\",\"pos\":[24,100],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"To\",\"text_width\":360,\"pos\":[24,138],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Validator\",\"pos\":[24,206],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Validator\",\"text_width\":360,\"pos\":[24,244],\"font\":\"openSansEnIllustrate\"}]},\"Vote\":{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,290],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetCosmosIndex\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_color\":16090890},{\"type\":\"label\",\"text\":\"Proposal\",\"pos\":[24,62],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Proposal\",\"pos\":[123,62],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Voted\",\"pos\":[24,100],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Voted\",\"pos\":[95,100],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Method\",\"pos\":[24,138],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Method\",\"pos\":[113,138],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Voter\",\"pos\":[24,176],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosTextOfKind\",\"text_key\":\"Voter\",\"pos\":[24,214],\"text_width\":360,\"font\":\"openSansEnIllustrate\"}]}}},{\"type\":\"container\",\"pos\":[0,16],\"size_func\":\"GetCosmosDetailMsgSize\",\"exist_func\":\"GetCosmosMethodExist\",\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144,\"exist_func\":\"GetCosmosValueExist\"},{\"type\":\"label\",\"text_func\":\"GetCosmosValue\",\"pos\":[92,16],\"text_color\":16090890,\"text_width\":2000,\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetCosmosValueExist\"},{\"type\":\"label\",\"text\":\"Proposal\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144,\"exist_func\":\"GetCosmosVoteExist\"},{\"type\":\"label\",\"text_func\":\"GetCosmosProposal\",\"pos\":[123,16],\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetCosmosVoteExist\"},{\"type\":\"label\",\"text\":\"Voted\",\"pos\":[24,62],\"font\":\"openSansEnIllustrate\",\"text_opa\":144,\"exist_func\":\"GetCosmosVoteExist\"},{\"type\":\"label\",\"text_func\":\"GetCosmosVoted\",\"pos\":[95,62],\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetCosmosVoteExist\"},{\"type\":\"label\",\"text\":\"Method\",\"pos_func\":\"GetCosmosDetailMethodLabelPos\",\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosMethod\",\"pos_func\":\"GetCosmosDetailMethodValuePos\",\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text_func\":\"GetCosmosAddress1Label\",\"pos_func\":\"GetCosmosDetailAddress1LabelPos\",\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosAddress1Value\",\"text_width\":360,\"pos_func\":\"GetCosmosDetailAddress1ValuePos\",\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Old Validator\",\"pos\":[24,222],\"text_opa\":144,\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetCosmosOldValidatorExist\"},{\"type\":\"label\",\"text_func\":\"GetCosmosOldValidator\",\"text_width\":360,\"pos\":[24,260],\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetCosmosOldValidatorExist\"},{\"type\":\"label\",\"text_func\":\"GetCosmosAddress2Label\",\"pos_func\":\"GetCosmosDetailAddress2LabelPos\",\"text_opa\":144,\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetCosmosAddress2Exist\"},{\"type\":\"label\",\"text_func\":\"GetCosmosAddress2Value\",\"text_width\":360,\"pos_func\":\"GetCosmosDetailAddress2ValuePos\",\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetCosmosAddress2Exist\"},{\"type\":\"label\",\"text\":\"Source Channel\",\"pos\":[24,336],\"text_opa\":144,\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetCosmosChannelExist\"},{\"type\":\"label\",\"text_func\":\"GetCosmosChannel\",\"pos\":[187,336],\"font\":\"openSansEnIllustrate\",\"exist_func\":\"GetCosmosChannelExist\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,170],\"bg_opa\":31,\"radius\":24,\"align\":13,\"align_to\":-2,\"children\":[{\"type\":\"label\",\"text\":\"Max Fee\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosMaxFee\",\"pos\":[118,16],\"text_width\":2000,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"  ·  Max Fee Price * Gas Limit\",\"pos\":[24,54],\"font\":\"openSansDesc\",\"text_opa\":144},{\"type\":\"label\",\"text\":\"Fee\",\"pos\":[24,86],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosFee\",\"pos\":[73,86],\"text_width\":2000,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Gas Limit\",\"pos\":[24,124],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosGasLimit\",\"pos\":[127,124],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,12],\"size\":[408,100],\"bg_opa\":31,\"radius\":24,\"align\":13,\"align_to\":-2,\"children\":[{\"type\":\"label\",\"text\":\"Network\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosNetwork\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Chain ID\",\"pos\":[24,54],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosChainId\",\"pos\":[120,54],\"font\":\"openSansEnIllustrate\"}]}]}]},\"unknown\":{\"name\":\"cosmos_unknown_page\",\"type\":\"container\",\"pos\":[36,0],\"size\":[408,600],\"bg_color\":0,\"children\":[{\"type\":\"container\",\"pos\":[0,80],\"size\":[408,170],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Max Fee\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Max Fee\",\"pos\":[118,16],\"text_width\":2000,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"  ·  Max Fee Price * Gas Limit\",\"pos\":[24,54],\"font\":\"openSansDesc\",\"text_opa\":144},{\"type\":\"label\",\"text\":\"Fee\",\"pos\":[24,86],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Fee\",\"pos\":[73,86],\"text_width\":2000,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Gas Limit\",\"pos\":[24,124],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Gas Limit\",\"pos\":[127,124],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,62],\"bg_opa\":31,\"radius\":24,\"align\":13,\"align_to\":-2,\"children\":[{\"type\":\"label\",\"text\":\"Network\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Network\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,62],\"bg_opa\":31,\"radius\":24,\"align\":13,\"align_to\":-2,\"children\":[{\"type\":\"label\",\"text\":\"Message\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Message\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\",\"text_color\":16105777}]}]},\"msg\":{\"name\":\"cosmos_msg_page\",\"type\":\"container\",\"pos\":[36,0],\"size\":[408,600],\"bg_color\":0,\"children\":[{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,62],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Network\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Network\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,130],\"bg_opa\":31,\"radius\":24,\"align\":13,\"align_to\":-2,\"children\":[{\"type\":\"label\",\"text\":\"Signer\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Signer\",\"text_width\":360,\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,250],\"bg_opa\":31,\"radius\":24,\"align\":13,\"align_to\":-2,\"children\":[{\"type\":\"label\",\"text\":\"Message\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetCosmosDetailItemValue\",\"text_key\":\"Message\",\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"}]}]}}}",
#else
        PC_SIMULATOR_PATH "/page_eth.json",
#endif
        GuiGetCosmosData,
        GuiGetCosmosTmpType,
        FreeCosmosMemory,
    },
    {
        REMAPVIEW_SUI,
#ifndef COMPILE_SIMULATOR
        "{\"type\":\"container\",\"pos\":[36,0],\"size\":[408,526],\"bg_opa\":0,\"children\":[{\"type\":\"label\",\"text\":\"Transaction Raw Data\",\"text_width\":360,\"text_opa\":144,\"pos\":[0,0],\"font\":\"openSansEnIllustrate\"},{\"type\":\"container\",\"pos\":[0,38],\"size\":[408,488],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetSuiDetail\",\"text_len_func\":\"GetSuiDetailLen\",\"text_width\":360,\"pos\":[24,24],\"font\":\"openSansEnIllustrate\"}]}]}",
#else
        PC_SIMULATOR_PATH "/page_eth.json",
#endif
        GuiGetSuiData,
        NULL,
        FreeSuiMemory,
    },
    {
        REMAPVIEW_SOL,
#ifndef COMPILE_SIMULATOR
        "{\"name\":\"sol_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,774],\"bg_color\":0,\"border_width\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowSolTxOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowSolTxDetail\"}]}]}",
#else
        PC_SIMULATOR_PATH "/page_eth.json",
#endif
        GuiGetSolData,
        NULL,
        FreeSolMemory,
    },
    {
        REMAPVIEW_SOL_MESSAGE,
#ifndef COMPILE_SIMULATOR
        "{\"table\":{\"utf8_message\":{\"type\":\"container\",\"pos\":[0,39],\"size\":[408,500],\"align\":2,\"bg_color\":16777215,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"From\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetSolMessageFrom\",\"pos\":[24,54],\"text_width\":360,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Message\",\"pos\":[24,130],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"container\",\"pos\":[24,168],\"size\":[360,332],\"align\":1,\"aflag\":16,\"bg_opa\":0,\"children\":[{\"type\":\"label\",\"text_func\":\"GetSolMessageUtf8\",\"pos\":[0,0],\"text_width\":360,\"font\":\"openSansEnIllustrate\",\"text_color\":16777215}]}]},\"raw_message\":{\"type\":\"container\",\"pos\":[0,39],\"size\":[408,500],\"align\":2,\"bg_color\":16777215,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Raw Message\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"container\",\"pos\":[24,54],\"size\":[360,450],\"align\":1,\"aflag\":16,\"bg_opa\":0,\"children\":[{\"type\":\"label\",\"text_func\":\"GetSolMessageRaw\",\"pos\":[0,0],\"text_width\":360,\"font\":\"openSansEnIllustrate\"}]}]}}}",
#else
        PC_SIMULATOR_PATH "/page_eth.json",
#endif
        GuiGetSolMessageData,
        GetSolMessageType,
        FreeSolMemory,
    },
    {
        REMAPVIEW_APT,
#ifndef COMPILE_SIMULATOR
        "{\"type\":\"container\",\"pos\":[36,0],\"size\":[408,526],\"bg_opa\":0,\"children\":[{\"type\":\"label\",\"text\":\"Transaction Raw Data\",\"text_width\":360,\"text_opa\":144,\"pos\":[0,0],\"font\":\"openSansEnIllustrate\"},{\"type\":\"container\",\"pos\":[0,38],\"size\":[408,488],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetAptosDetail\",\"text_len_func\":\"GetAptosDetailLen\",\"text_width\":360,\"pos\":[24,24],\"font\":\"openSansEnIllustrate\"}]}]}",
#else
        PC_SIMULATOR_PATH "/page_eth.json",
#endif
        GuiGetAptosData,
        NULL,
        FreeAptosMemory,
    },
    {
        REMAPVIEW_ADA,
#ifndef COMPILE_SIMULATOR
        "{\"type\":\"container\",\"pos\":[36,0],\"size\":[408,500],\"bg_color\":0,\"border_width\":0,\"children\":[{\"type\":\"container\",\"bg_opa\":31,\"radius\":24,\"size\":[408,62],\"children\":[{\"type\":\"label\",\"text\":\"Network\",\"pos\":[24,16],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetAdaNetwork\",\"pos\":[120,16],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"bg_opa\":31,\"radius\":24,\"align\":13,\"align_to\":-2,\"pos\":[0,16],\"size\":[408,138],\"children\":[{\"type\":\"label\",\"text\":\"Input Value\",\"pos\":[24,16],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetAdaTotalInput\",\"pos\":[147,16],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Output Value\",\"pos\":[24,54],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetAdaTotalOutput\",\"pos\":[164,54],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Fee\",\"pos\":[24,92],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetAdaFee\",\"pos\":[73,92],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size_func\":\"GetAdaInputDetailSize\",\"align\":13,\"align_to\":-2,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"From\",\"pos\":[24,16],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"name\":\"input\",\"type\":\"table\",\"width\":360,\"align\":2,\"pos\":[0,54],\"bg_color\":2105376,\"key_width\":30,\"table_func\":\"GetAdaInputDetail\",\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size_func\":\"GetAdaOutputDetailSize\",\"align\":13,\"align_to\":-2,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"To\",\"pos\":[24,16],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"name\":\"input\",\"type\":\"table\",\"width\":360,\"align\":2,\"pos\":[0,54],\"bg_color\":2105376,\"key_width\":30,\"table_func\":\"GetAdaOutputDetail\",\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"label\",\"exist_func\":\"GetAdaCertificatesExist\",\"text_func\":\"GetAdaCertificatesLabel\",\"pos\":[0,16],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144,\"align\":13,\"align_to\":-2},{\"name\":\"certificates\",\"type\":\"container\",\"pos\":[0,16],\"exist_func\":\"GetAdaCertificatesExist\",\"size_func\":\"GetAdaCertificatesSize\",\"align\":13,\"align_to\":-2,\"bg_opa\":31,\"radius\":24,\"children\":[{\"name\":\"input\",\"type\":\"table\",\"width\":360,\"align\":2,\"pos\":[0,24],\"bg_color\":2105376,\"key_width\":30,\"table_func\":\"GetAdaCertificatesData\",\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"label\",\"text_func\":\"GetAdaWithdrawalsLabel\",\"exist_func\":\"GetAdaWithdrawalsExist\",\"pos\":[0,16],\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144,\"align\":13,\"align_to\":-2},{\"name\":\"withdrawal\",\"type\":\"container\",\"pos\":[0,16],\"exist_func\":\"GetAdaWithdrawalsExist\",\"size_func\":\"GetAdaWithdrawalsSize\",\"align\":13,\"align_to\":-2,\"bg_opa\":31,\"radius\":24,\"children\":[{\"name\":\"input\",\"type\":\"table\",\"width\":360,\"align\":2,\"pos\":[0,24],\"bg_color\":2105376,\"key_width\":30,\"table_func\":\"GetAdaWithdrawalsData\",\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"label\",\"pos\":[0,16],\"exist_func\":\"GetAdaExtraDataExist\",\"text\":\"Extra Data\",\"align\":13,\"align_to\":-2,\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,500],\"align\":13,\"align_to\":-2,\"bg_opa\":31,\"radius\":24,\"exist_func\":\"GetAdaExtraDataExist\",\"children\":[{\"type\":\"label\",\"text_func\":\"GetAdaExtraData\",\"text_len_func\":\"GetAdaExtraDataLen\",\"text_width\":360,\"pos\":[24,24],\"font\":\"openSansEnIllustrate\"}]}]}",
#else
        PC_SIMULATOR_PATH "/page_ada.json",
#endif
        GuiGetAdaData,
        NULL,
        FreeAdaMemory,
    },
    {
        REMAPVIEW_XRP,
#ifndef COMPILE_SIMULATOR
        "{\"type\":\"container\",\"pos\":[36,0],\"size\":[408,526],\"bg_opa\":0,\"children\":[{\"type\":\"label\",\"text\":\"Transaction Raw Data\",\"text_width\":360,\"text_opa\":144,\"pos\":[0,0],\"font\":\"openSansEnIllustrate\"},{\"type\":\"container\",\"pos\":[0,38],\"size\":[408,488],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text_func\":\"GetXrpDetail\",\"text_len_func\":\"GetXrpDetailLen\",\"text_width\":360,\"pos\":[24,24],\"font\":\"openSansEnIllustrate\"}]}]}",
#else
        PC_SIMULATOR_PATH "/page_xrp.json",
#endif
        GuiGetXrpData,
        NULL,
        FreeXrpMemory,
    },
    {
        REMAPVIEW_AR,
#ifndef COMPILE_SIMULATOR
        "{\"name\":\"ar_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,900],\"bg_color\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"container\",\"pos\":[0,12],\"size\":[408,144],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Value\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetArweaveValue\",\"pos\":[24,50],\"text_color\":16090890,\"font\":\"openSansEnLittleTitle\"},{\"type\":\"label\",\"text\":\"Fee\",\"pos\":[24,98],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetArweaveFee\",\"pos\":[73,98],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,16],\"size\":[408,244],\"align_to\":-2,\"align\":13,\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"From\",\"pos\":[24,16],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetArweaveFromAddress\",\"text_width\":360,\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"To\",\"pos\":[24,130],\"text_opa\":144,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text_func\":\"GetArweaveToAddress\",\"text_width\":360,\"pos\":[0,8],\"align_to\":-2,\"align\":13,\"font\":\"openSansEnIllustrate\"}]}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"container\",\"pos\":[0,12],\"size\":[408,358],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"##1\",\"font\":\"openSansEnIllustrate\",\"text_color\":16090890,\"pos\":[24,16]},{\"type\":\"label\",\"text\":\"Value\",\"font\":\"openSansEnIllustrate\",\"text_opa\":144,\"pos\":[24,62]},{\"type\":\"label\",\"text_func\":\"GetArweaveValue\",\"pos\":[92,62],\"text_color\":16090890,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"Fee\",\"font\":\"openSansEnIllustrate\",\"text_opa\":144,\"pos\":[24,100]},{\"type\":\"label\",\"text_func\":\"GetArweaveFee\",\"pos\":[73,100],\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"From\",\"font\":\"openSansEnIllustrate\",\"text_opa\":144,\"pos\":[24,138]},{\"type\":\"label\",\"text_func\":\"GetArweaveFromAddress\",\"pos\":[24,176],\"text_width\":360,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text\":\"To\",\"pos\":[24,244],\"text_opa\":144,\"font\":\"openSansEnIllustrate\"},{\"type\":\"label\",\"text_func\":\"GetArweaveToAddress\",\"pos\":[24,282],\"text_width\":360,\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"custom_container\",\"pos\":[0,16],\"radius\":24,\"custom_show_func\":\"GuiShowArweaveTxDetail\"}]}]}",
#else
        PC_SIMULATOR_PATH "/page_ar.json",
#endif
        GuiGetArData,
        NULL,
        FreeArMemory,
    },
    {
        REMAPVIEW_AR_MESSAGE,
#ifndef COMPILE_SIMULATOR
        "{\"name\":\"ar_message_page\",\"type\":\"container\",\"pos\":[36,0],\"size\":[408,542],\"bg_color\":0,\"children\":[{\"type\":\"container\",\"size\":[408,130],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Address \",\"pos\":[24,16],\"size\":[408,130],\"font\":\"openSansEnIllustrate\",\"text_opa\":144},{\"type\":\"label\",\"text_func\":\"GetArweaveMessageAddress\",\"text_width\":360,\"pos\":[24,54],\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,146],\"size\":[408,766],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Message(UTF-8)\",\"font\":\"openSansEnIllustrate\",\"text_color\":16090890,\"pos\":[24,16]},{\"type\":\"label\",\"text_func\":\"GetArweaveMessageText\",\"text_len_func\":\"GetArweaveMessageLength\",\"pos\":[24,62],\"text_width\":360,\"font\":\"openSansEnIllustrate\"}]},{\"type\":\"container\",\"pos\":[0,928],\"size\":[408,900],\"bg_opa\":31,\"radius\":24,\"children\":[{\"type\":\"label\",\"text\":\"Message\",\"font\":\"openSansEnIllustrate\",\"text_color\":16090890,\"pos\":[24,16]},{\"type\":\"label\",\"text_func\":\"GetArweaveRawMessage\",\"text_len_func\":\"GetArweaveRawMessageLength\",\"pos\":[24,62],\"text_width\":360,\"font\":\"openSansEnIllustrate\"}]}]}",
#else
        PC_SIMULATOR_PATH "/page_ar_message.json",
#endif
        GuiGetArData,
        NULL,
        FreeArMemory,
    },
    {
        REMAPVIEW_TON,
#ifndef COMPILE_SIMULATOR
        "{\"name\":\"ton_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,900],\"bg_color\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiTonTxOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Raw Data\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiTonTxRawData\"}]}]}",
#else
        PC_SIMULATOR_PATH "/page_ton.json",
#endif
        GuiGetTonGUIData,
        NULL,
        FreeArMemory,
    },
    {
        REMAPVIEW_TON_SIGNPROOF,
#ifndef COMPILE_SIMULATOR
        "{\"name\":\"btc_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,774],\"bg_color\":0,\"border_width\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiTonProofOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Raw Data\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiTonProofRawData\"}]}]}",
#else
        PC_SIMULATOR_PATH "/page_ton_proof.json",
#endif
        GuiGetTonProofGUIData,
        NULL,
        FreeArMemory,
    },
#endif
};

void *GuiTemplateReload(lv_obj_t *parent, uint8_t index);
void GuiTemplateClosePage(void);
static void *GuiWidgetFactoryCreate(lv_obj_t *parent, cJSON *json);

lv_obj_t *g_templateContainer = NULL;
lv_obj_t *g_tableView = NULL;
void *g_totalData;
GuiRemapViewType g_reMapIndex = REMAPVIEW_BUTT;
uint8_t g_viewTypeIndex = ViewTypeUnKnown;
lv_obj_t *g_defaultVector[OBJ_VECTOR_MAX_LEN];
lv_obj_t *g_hiddenVector[OBJ_VECTOR_MAX_LEN];
GuiAnalyzeTable_t g_tableData[8];
uint8_t g_tableDataAmount = 0;

void GuiAnalyzeFreeTable(uint8_t row, uint8_t col, void *param)
{
    int i = 0, j = 0;
    char ***indata = (char ***)param;
    for (i = 0; i < col; i++) {
        for (j = 0; j < row; j++) {
            if (indata[i][j] != NULL) SRAM_FREE(indata[i][j]);
        }
        if (indata[i] != NULL) SRAM_FREE(indata[i]);
    }
    if (indata != NULL) SRAM_FREE(indata);
}

const lv_font_t *GetLvglTextFont(char *fontStr)
{
    if (!strcmp(fontStr, "openSansEnTitle")) {
        return &openSansEnTitle;
    } else if (!strcmp(fontStr, "openSansEnLittleTitle")) {
        return &openSansEnLittleTitle;
    } else if (!strcmp(fontStr, "openSansEnText")) {
        return &openSansEnText;
    } else if (!strcmp(fontStr, "openSansEnIllustrate")) {
        return &openSansEnIllustrate;
    } else if (!strcmp(fontStr, "openSansDesc")) {
        return &openSansDesc;
    } else if (!strcmp(fontStr, "illustrate")) {
        return g_defIllustrateFont;
    }

    return &openSansEnIllustrate;
}

GetContSizeFunc GetPsbtContainerSize(char *type)
{
    if (!strcmp(type, "GetPsbtOverviewSize")) {
        return GetPsbtOverviewSize;
    } else if (!strcmp(type, "GetPsbtDetailSize")) {
        return GetPsbtDetailSize;
    }
    return NULL;
}

#ifndef BTC_ONLY
GetContSizeFunc GetEthContainerSize(char *type)
{
    if (!strcmp(type, "GetEthToFromSize")) {
        return GetEthToFromSize;
    } else if (!strcmp(type, "GetEthContractDataSize")) {
        return GetEthContractDataSize;
    }
    return NULL;
}

GetContSizeFunc GetCosmosContainerSize(char *type)
{
    if (!strcmp(type, "GetCosmosDetailMsgSize")) {
        return GetCosmosDetailMsgSize;
    } else if (!strcmp(type, "GetCosmosOverviewAddrSize")) {
        return GetCosmosOverviewAddrSize;
    }
    return NULL;
}

GetContSizeFunc GetAdaContainerSize(char *type)
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
    if (!strcmp(type, "GetAdaWithdrawalsSize")) {
        return GetAdaWithdrawalsSize;
    }
    return NULL;
}
#endif

GetContSizeFunc GuiTemplateSizeFuncGet(char *type)
{
    switch (g_reMapIndex) {
    case REMAPVIEW_BTC:
        return GetPsbtContainerSize(type);
#ifndef BTC_ONLY
    case REMAPVIEW_ETH:
        return GetEthContainerSize(type);
    case REMAPVIEW_COSMOS:
        return GetCosmosContainerSize(type);
    case REMAPVIEW_ADA:
        return GetAdaContainerSize(type);
#endif
    default:
        return NULL;
    }

    return NULL;
}

#ifndef BTC_ONLY
GetListLenFunc GetCosmosListLen(char *type)
{
    if (!strcmp(type, "GetCosmosMsgLen")) {
        return GetCosmosMsgLen;
    }
    return NULL;
}
#endif

GetListLenFunc GuiTemplateListLenFuncGet(char *type)
{
    switch (g_reMapIndex) {
#ifndef BTC_ONLY
    case REMAPVIEW_COSMOS:
        return GetCosmosListLen(type);
#endif
    default:
        return NULL;
    }
}

#ifndef BTC_ONLY
GetListItemKeyFunc GetCosmosListItemKey(char *type)
{
    if (!strcmp(type, "GetCosmosMsgKey")) {
        return GetCosmosMsgKey;
    }
    return NULL;
}
#endif

GetListItemKeyFunc GuiTemplateListItemKeyFuncGet(char *type)
{
    switch (g_reMapIndex) {
#ifndef BTC_ONLY
    case REMAPVIEW_COSMOS:
        return GetCosmosListItemKey(type);
#endif
    default:
        return NULL;
    }
}

#ifndef BTC_ONLY
GetContSizeFunc GetEthObjPos(char *type)
{
    if (!strcmp(type, "GetEthToLabelPos")) {
        return GetEthToLabelPos;
    }
    return NULL;
}

GetContSizeFunc GetCosmosObjPos(char *type)
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
#endif

GetContSizeFunc GuiTemplatePosFuncGet(char *type)
{
    switch (g_reMapIndex) {
#ifndef BTC_ONLY
    case REMAPVIEW_ETH:
        return GetEthObjPos(type);
    case REMAPVIEW_COSMOS:
        return GetCosmosObjPos(type);
#endif
    default:
        return NULL;
    }

    return NULL;
}

GetLabelDataFunc GuiBtcTextFuncGet(char *type)
{
    if (!strcmp(type, "GetPsbtTotalOutAmount")) {
        return GetPsbtTotalOutAmount;
    } else if (!strcmp(type, "GetPsbtFeeAmount")) {
        return GetPsbtFeeAmount;
    } else if (!strcmp(type, "GetPsbtTotalOutSat")) {
        return GetPsbtTotalOutSat;
    } else if (!strcmp(type, "GetPsbtFeeSat")) {
        return GetPsbtFeeSat;
    } else if (!strcmp(type, "GetPsbtNetWork")) {
        return GetPsbtNetWork;
    } else if (!strcmp(type, "GetPsbtDetailOutputValue")) {
        return GetPsbtDetailOutputValue;
    } else if (!strcmp(type, "GetPsbtDetailInputValue")) {
        return GetPsbtDetailInputValue;
    } else if (!strcmp(type, "GetPsbtDetailFee")) {
        return GetPsbtDetailFee;
    } else if (!strcmp(type, "GetBtcMsgDetail")) {
        return GetBtcMsgDetail;
    }
    return NULL;
}

#ifndef BTC_ONLY
GetLabelDataFunc GuiEthTextFuncGet(char *type)
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
    }

    return NULL;
}

GetLabelDataFunc GuiEthPersonalMessageTextFuncGet(char *type)
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

GetLabelDataFunc GuiSolMessageTextFuncGet(char *type)
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

GetLabelDataFunc GuiEthTypedDataTextFuncGet(char *type)
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
    }
    return NULL;
}

#endif

GetTableDataFunc GuiBtcTableFuncGet(char *type)
{
    if (!strcmp(type, "GetPsbtInputData")) {
        return GetPsbtInputData;
    } else if (!strcmp(type, "GetPsbtOutputData")) {
        return GetPsbtOutputData;
    } else if (!strcmp(type, "GetPsbtInputDetailData")) {
        return GetPsbtInputDetailData;
    } else if (!strcmp(type, "GetPsbtOutputDetailData")) {
        return GetPsbtOutputDetailData;
    }
    return NULL;
}

GetLabelDataLenFunc GuiBtcTextLenFuncGet(char *type)
{
    if (!strcmp(type, "GetBtcMsgDetailLen")) {
        return GetBtcMsgDetailLen;
    }
    return NULL;
}

GetLabelDataLenFunc GuiArTextLenFuncGet(char *type)
{
    if (!strcmp(type, "GetArweaveRawMessageLength")) {
        return GetArweaveRawMessageLength;
    } else if (!strcmp(type, "GetArweaveMessageLength")) {
        return GetArweaveMessageLength;
    }
    return NULL;
}

#ifndef BTC_ONLY
GetTableDataFunc GuiEthTableFuncGet(char *type)
{
    if (!strcmp(type, "GetEthContractData")) {
        return GetEthContractData;
    }
    return NULL;
}

GetTableDataFunc GuiAdaTabelFuncGet(char *type)
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
    return NULL;
}

GetLabelDataFunc GuiTrxTextFuncGet(char *type)
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

GetLabelDataFunc GuiCosmosTextFuncGet(char *type)
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

GetLabelDataFunc GuiSuiTextFuncGet(char *type)
{
    if (!strcmp(type, "GetSuiDetail")) {
        return GetSuiDetail;
    }
    return NULL;
}

GetLabelDataLenFunc GuiSuiTextLenFuncGet(char *type)
{
    if (!strcmp(type, "GetSuiDetailLen")) {
        return GetSuiDetailLen;
    }
    return NULL;
}

GetLabelDataFunc GuiAptosTextFuncGet(char *type)
{
    if (!strcmp(type, "GetAptosDetail")) {
        return GetAptosDetail;
    }
    return NULL;
}

GetLabelDataLenFunc GuiAptosTextLenFuncGet(char *type)
{
    if (!strcmp(type, "GetAptosDetailLen")) {
        return GetAptosDetailLen;
    }
    return NULL;
}

GetLabelDataFunc GuiXrpTextFuncGet(char *type)
{
    if (!strcmp(type, "GetXrpDetail")) {
        return GetXrpDetail;
    }
    return NULL;
}

GetLabelDataFunc GuiArTextFuncGet(char *type)
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

GetLabelDataLenFunc GuiXrpTextLenFuncGet(char *type)
{
    if (!strcmp(type, "GetXrpDetailLen")) {
        return GetXrpDetailLen;
    }
    return NULL;
}

GetLabelDataLenFunc GuiEthTextLenFuncGet(char *type)
{
    if (!strcmp(type, "GetEthTypedDataMessageLen")) {
        return GetEthTypedDataMessageLen;
    }
    return NULL;
}

GetLabelDataFunc GuiAdaTextFuncGet(char *type)
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
    }
    return NULL;
}

GetLabelDataLenFunc GuiAdaTextLenFuncGet(char *type)
{
    if (!strcmp(type, "GetAdaExtraDataLen")) {
        return GetAdaExtraDataLen;
    }
    return NULL;
}
#endif

GetLabelDataLenFunc GuiTemplateTextLenFuncGet(char *type)
{
    switch (g_reMapIndex) {
    case REMAPVIEW_BTC_MESSAGE:
        return GuiBtcTextLenFuncGet(type);
#ifndef BTC_ONLY
    case REMAPVIEW_SUI:
        return GuiSuiTextLenFuncGet(type);
    case REMAPVIEW_APT:
        return GuiAptosTextLenFuncGet(type);
    case REMAPVIEW_ADA:
        return GuiAdaTextLenFuncGet(type);
    case REMAPVIEW_XRP:
        return GuiXrpTextLenFuncGet(type);
    case REMAPVIEW_ETH_TYPEDDATA:
        return GuiEthTextLenFuncGet(type);
    case REMAPVIEW_AR:
    case REMAPVIEW_AR_MESSAGE:
        return GuiArTextLenFuncGet(type);
#endif
    default:
        return NULL;
    }
}

GetLabelDataFunc GuiTemplateTextFuncGet(char *type)
{
    switch (g_reMapIndex) {
    case REMAPVIEW_BTC:
    case REMAPVIEW_BTC_MESSAGE:
        return GuiBtcTextFuncGet(type);
#ifndef BTC_ONLY
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
        return GuiAdaTextFuncGet(type);
    case REMAPVIEW_XRP:
        return GuiXrpTextFuncGet(type);
    case REMAPVIEW_AR:
    case REMAPVIEW_AR_MESSAGE:
        return GuiArTextFuncGet(type);
#endif
    default:
        return NULL;
    }

    return NULL;
}

GetTableDataFunc GuiTemplateTableFuncGet(char *type)
{
    switch (g_reMapIndex) {
    case REMAPVIEW_BTC:
        return GuiBtcTableFuncGet(type);
#ifndef BTC_ONLY
    case REMAPVIEW_ETH:
        return GuiEthTableFuncGet(type);
    case REMAPVIEW_ADA:
        return GuiAdaTabelFuncGet(type);
#endif
    default:
        return NULL;
    }

    return NULL;
}

const void *GetImgSrc(char *type)
{
    if (!strcmp(type, "imgEns")) {
        return &imgEns;
    }
    if (!strcmp(type, "imgWarningRed")) {
        return &imgWarningRed;
    }
    if (!strcmp(type, "imgContract")) {
        return &imgContract;
    }
    if (!strcmp(type, "imgQrcodeTurquoise")) {
        return &imgQrcodeTurquoise;
    }
    if (!strcmp(type, "imgConversion")) {
        return &imgConversion;
    }
    return &imgSwitch;
}

GetObjStateFunc GuiTemplateStateFuncGet(char *type)
{
#ifndef BTC_ONLY
    if (!strcmp(type, "GetEthEnsExist")) {
        return GetEthEnsExist;
    } else if (!strcmp(type, "GetToEthEnsExist")) {
        return GetToEthEnsExist;
    } else if (!strcmp(type, "GetEthContractDataExist")) {
        return GetEthContractDataExist;
    } else if (!strcmp(type, "GetErc20WarningExist")) {
        return GetErc20WarningExist;
    } else if (!strcmp(type, "GetEthContractDataNotExist")) {
        return GetEthContractDataNotExist;
    } else if (!strcmp(type, "GetEthInputDataExist")) {
        return GetEthInputDataExist;
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
    }
#endif
    return NULL;
}

static void SwitchHidden(lv_event_t *e)
{
    SetLvglFlagFunc clearHidden = lv_obj_add_flag;
    SetLvglFlagFunc addHidden = lv_obj_clear_flag;
    if (lv_obj_has_flag(g_defaultVector[0], LV_OBJ_FLAG_HIDDEN)) {
        clearHidden = lv_obj_clear_flag;
        addHidden = lv_obj_add_flag;
    } else {
        clearHidden = lv_obj_add_flag;
        addHidden = lv_obj_clear_flag;
    }

    for (int i = 0; i < OBJ_VECTOR_MAX_LEN && g_defaultVector[i] != NULL; i++) {
        clearHidden(g_defaultVector[i], LV_OBJ_FLAG_HIDDEN);
    }

    for (int i = 0; i < OBJ_VECTOR_MAX_LEN && g_hiddenVector[i] != NULL; i++) {
        addHidden(g_hiddenVector[i], LV_OBJ_FLAG_HIDDEN);
    }
}

lv_event_cb_t GuiTemplateEventCbGet(char *type)
{
    if (!strcmp(type, "SwitchHidden")) {
        return SwitchHidden;
    }
#ifndef BTC_ONLY
    if (!strcmp(type, "EthContractLearnMore")) {
        return EthContractLearnMore;
    }
#endif
    return NULL;
}

void GuiWidgetBaseInit(lv_obj_t *obj, cJSON *json)
{
    GetContSizeFunc func = NULL;
    uint16_t xpos, ypos;
    int xsize, ysize;
    lv_align_t align;
    // virtual void create_lvgl_widget(cJSON *json) {};
    cJSON *item = cJSON_GetObjectItem(json, "pos_func");
    if (item != NULL) {
        func = GuiTemplatePosFuncGet(item->valuestring);
        func(&xpos, &ypos, g_totalData);
        lv_obj_align(obj, LV_ALIGN_DEFAULT, xpos, ypos);
    } else {
        item = cJSON_GetObjectItem(json, "pos");
        if (item != NULL) {
            xpos = item->child->valueint;
            ypos = item->child->next->valueint;
            item = cJSON_GetObjectItem(json, "align");
            if (item != NULL) {
                align = item->valueint;
                item = cJSON_GetObjectItem(json, "align_to");
                if (item != NULL) {
                    lv_obj_t *parent = lv_obj_get_parent(obj);
                    lv_obj_t *alignChild = lv_obj_get_child(parent, lv_obj_get_child_cnt(parent) + item->valueint);
                    lv_obj_align_to(obj, alignChild, align, xpos, ypos);
                } else {
                    lv_obj_align(obj, align, xpos, ypos);
                }
            } else {
                lv_obj_align(obj, LV_ALIGN_DEFAULT, xpos, ypos);
            }
        }
    }
    item = cJSON_GetObjectItem(json, "size");
    if (item != NULL) {
        xsize = item->child->valueint;
        ysize = item->child->next->valueint;
        lv_obj_set_size(obj, xsize, ysize);
    } else {
        item = cJSON_GetObjectItem(json, "width");
        if (item != NULL) {
            lv_obj_set_width(obj, item->valueint);
        }
    }

    item = cJSON_GetObjectItem(json, "bg_color");
    if (item != NULL) {
        lv_obj_set_style_bg_color(obj, lv_color_hex(item->valueint), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_bg_color(obj, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "bg_opa");
    if (item != NULL) {
        lv_obj_set_style_bg_opa(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "radius");
    if (item != NULL) {
        lv_obj_set_style_radius(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "border_width");
    if (item != NULL) {
        lv_obj_set_style_border_width(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "border_color");
    if (item != NULL) {
        lv_obj_set_style_border_color(obj, lv_color_hex(item->valueint), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "pad_vertical");
    if (item != NULL) {
        lv_obj_set_style_pad_top(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "pad_horizontal");
    if (item != NULL) {
        lv_obj_set_style_pad_left(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "aflag");
    if (item != NULL) {
        lv_obj_add_flag(obj, item->valueint);
    }

    item = cJSON_GetObjectItem(json, "cflag");
    if (item != NULL) {
        lv_obj_clear_flag(obj, item->valueint);
    }

    item = cJSON_GetObjectItem(json, "is_show");
    if (item != NULL) {
        // ((1<<(g_viewTypeIndex - 1)) & 0x7fffffff);
        if ((item->valueint & (1 << g_viewTypeIndex)) == 0) {
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
        }
    }

    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);

    cJSON *childrenArray = cJSON_GetObjectItem(json, "children");
    if (childrenArray != NULL) {
        for (cJSON *child = childrenArray->child; child != NULL; child = child->next) {
            GuiWidgetFactoryCreate(obj, child);
        }
    }
}

void *GuiWidgetLabel(lv_obj_t *parent, cJSON *json)
{
    GetLabelDataFunc pFunc = NULL;
    GetLabelDataLenFunc lenFunc = NULL;
    int textWidth = 0;
    int bufLen = BUFFER_SIZE_512;
    cJSON *item = cJSON_GetObjectItem(json, "text_len_func");
    if (item != NULL) {
        lenFunc = GuiTemplateTextLenFuncGet(item->valuestring);
        if (lenFunc != NULL) {
            bufLen = lenFunc(g_totalData) + 1;
        }
    }
    char *text = EXT_MALLOC(bufLen);
    lv_obj_t *obj = lv_label_create(parent);
    item = cJSON_GetObjectItem(json, "text");
    if (item != NULL) {
        lv_label_set_text(obj, _(item->valuestring));
    } else {
        lv_label_set_text(obj, "");
    }

    item = cJSON_GetObjectItem(json, "text_width");
    if (item != NULL) {
        textWidth = item->valueint;
    } else {
        textWidth = 400;
    }

    item = cJSON_GetObjectItem(json, "font");
    if (item != NULL) {
        const lv_font_t *font = GetLvglTextFont(item->valuestring);
        lv_obj_set_style_text_font(obj, font, LV_STATE_DEFAULT | LV_PART_MAIN);
    }

    item = cJSON_GetObjectItem(json, "text_color");
    if (item != NULL) {
        lv_obj_set_style_text_color(obj, lv_color_hex(item->valueint), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_text_color(obj, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "text_func");
    if (item != NULL) {
        pFunc = GuiTemplateTextFuncGet(item->valuestring);
        item = cJSON_GetObjectItem(json, "text_key");
        if (item != NULL) {
            strcpy_s(text, BUFFER_SIZE_512, item->valuestring);
        }
    }

    item = cJSON_GetObjectItem(json, "text_opa");
    if (item != NULL) {
        lv_obj_set_style_text_opa(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    lv_label_set_recolor(obj, true);
    lv_obj_set_style_text_letter_space(obj, LV_STATE_DEFAULT | LV_PART_MAIN, 20);
    if (pFunc) {
        pFunc(text, g_totalData, bufLen);
        lv_label_set_text(obj, text);
    }
    if (lv_obj_get_self_width(obj) >= textWidth) {
        lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(obj, textWidth);
    }
    EXT_FREE(text);
    return obj;
}

void *GuiWidgetContainer(lv_obj_t *parent, cJSON *json)
{
    uint16_t contWidth = 0;
    uint16_t contHeight = 0;
    GetContSizeFunc func = NULL;
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_style_outline_width(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_border_side(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_shadow_width(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);

    cJSON *item = cJSON_GetObjectItem(json, "size_func");
    if (item != NULL) {
        func = GuiTemplateSizeFuncGet(item->valuestring);
        func(&contWidth, &contHeight, g_totalData);
        lv_obj_set_size(obj, contWidth, contHeight);
    }

    item = cJSON_GetObjectItem(json, "cb");
    if (item != NULL) {
        lv_obj_add_event_cb(obj, GuiTemplateEventCbGet(item->valuestring), LV_EVENT_CLICKED, NULL);
    }
    return obj;
}

GetCustomContainerFunc GuiTemplateCustomFunc(char *funcName)
{
    if (!strcmp(funcName, "GuiBtcTxOverview")) {
        return GuiBtcTxOverview;
    } else if (!strcmp(funcName, "GuiBtcTxDetail")) {
        return GuiBtcTxDetail;
    }
#ifndef BTC_ONLY
    if (!strcmp(funcName, "GuiShowSolTxOverview")) {
        return GuiShowSolTxOverview;
    } else if (!strcmp(funcName, "GuiShowSolTxDetail")) {
        return GuiShowSolTxDetail;
    } else if (!strcmp(funcName, "GuiShowArweaveTxDetail")) {
        return GuiShowArweaveTxDetail;
    } else if (!strcmp(funcName, "GuiTonTxOverview")) {
        return GuiTonTxOverview;
    } else if (!strcmp(funcName, "GuiTonTxRawData")) {
        return GuiTonTxRawData;
    } else if (!strcmp(funcName, "GuiTonProofOverview")) {
        return GuiTonProofOverview;
    } else if (!strcmp(funcName, "GuiTonProofRawData")) {
        return GuiTonProofRawData;
    }
#endif
    return NULL;
}

void *GuiWidgetCustomContainer(lv_obj_t *parent, cJSON *json)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_style_outline_width(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_border_side(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_shadow_width(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, 0, LV_STATE_DEFAULT | LV_PART_MAIN);

    GetCustomContainerFunc func = NULL;
    cJSON *item = cJSON_GetObjectItem(json, "custom_show_func");
    if (item != NULL) {
        func = GuiTemplateCustomFunc(item->valuestring);
        if (func != NULL) {
            func(obj, g_totalData);
        }
    }
    return obj;
}

void GuiWidgetList(lv_obj_t *parent, cJSON *json)
{
    GetListLenFunc lenFunc = NULL;
    GetListItemKeyFunc keyFunc = NULL;
    cJSON *item = cJSON_GetObjectItem(json, "len_func");
    uint8_t len = 0;
    if (item != NULL) {
        lenFunc = GuiTemplateListLenFuncGet(item->valuestring);
        lenFunc(&len, g_totalData);
    } else {
        item = cJSON_GetObjectItem(json, "len");
        len = item->valueint;
    }
    if (len != 0) {
        cJSON *itemMap = cJSON_GetObjectItem(json, "item_map");
        cJSON *itemDefault = itemMap == NULL ? NULL : cJSON_GetObjectItem(itemMap, "default");
        cJSON *itemKeyFunc = cJSON_GetObjectItem(json, "item_key_func");
        cJSON *child = cJSON_GetObjectItem(json, "item");
        keyFunc = GuiTemplateListItemKeyFuncGet(itemKeyFunc->valuestring);
        char *key = SRAM_MALLOC(BUFFER_SIZE_64);
        for (uint8_t i = 0; i < len; i++) {
            if (itemKeyFunc != NULL) {
                keyFunc(key, g_totalData, BUFFER_SIZE_64);
            }
            if (itemMap != NULL) {
                child = cJSON_GetObjectItem(itemMap, key);
                if (child == NULL) {
                    if (itemDefault != NULL) {
                        child = itemDefault;
                    } else {
                        printf("key %s not found!\r\n", key);
                    }
                }
            }
            GuiWidgetFactoryCreate(parent, child);
        }
        SRAM_FREE(key);
    }
}

void *GuiWidgetTable(lv_obj_t *parent, cJSON *json)
{
    uint16_t tableWidth = 400;
    char ***tableData;
    uint8_t col;
    uint8_t row;
    uint16_t keyWidth = 50;
    GetTableDataFunc getDataFunc = NULL;
    lv_obj_t *obj = lv_table_create(parent);
    cJSON *item = cJSON_GetObjectItem(json, "width");
    if (item != NULL) {
        tableWidth = item->valueint;
        lv_obj_set_width(obj, tableWidth);
    }

    item = cJSON_GetObjectItem(json, "key_width");
    if (item != NULL) {
        keyWidth = item->valueint;
    }

    item = cJSON_GetObjectItem(json, "bg_color");
    if (item != NULL) {
        lv_obj_set_style_bg_color(obj, lv_color_hex(item->valueint), LV_PART_MAIN | LV_PART_ITEMS);
    }

    item = cJSON_GetObjectItem(json, "font");
    if (item != NULL) {
        const lv_font_t *font = GetLvglTextFont(item->valuestring);
        lv_obj_set_style_text_font(obj, font, LV_STATE_DEFAULT | LV_PART_MAIN);
    }

    item = cJSON_GetObjectItem(json, "table_func");
    if (item != NULL) {
        getDataFunc = GuiTemplateTableFuncGet(item->valuestring);
    }

    lv_obj_set_style_bg_color(obj, lv_color_hex(0X0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, LV_OPA_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_ITEMS);
    lv_obj_set_style_text_line_space(obj, -5, LV_PART_MAIN | LV_PART_ITEMS);
    lv_obj_set_style_outline_pad(obj, 0, LV_PART_MAIN | LV_PART_ITEMS);
    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_PART_ITEMS);
    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_PART_ITEMS);
    lv_obj_set_style_border_color(obj, lv_color_hex(0X202020), LV_PART_ITEMS);
    lv_obj_clear_state(obj, LV_STATE_FOCUS_KEY);

    tableData = (char ***)getDataFunc(&row, &col, g_totalData);

    if (col == 1) {
        lv_table_set_col_width(obj, 0, tableWidth);
    } else {
        lv_table_set_col_width(obj, 0, keyWidth);
        lv_table_set_col_width(obj, 1, tableWidth - keyWidth);
    }

    for (int i = 0; i < col; i++) {
        for (int j = 0; j < row; j++) {
            lv_table_set_cell_value(obj, j, i, (char *)tableData[i][j]);
        }
    }
    g_tableData[g_tableDataAmount].col = col;
    g_tableData[g_tableDataAmount].row = row;
    g_tableData[g_tableDataAmount].totalPtr = tableData;
    g_tableDataAmount++;

    return obj;
}

void *GuiWidgetImg(lv_obj_t *parent, cJSON *json)
{
    lv_obj_t *obj = lv_img_create(parent);
    cJSON *item = cJSON_GetObjectItem(json, "img_src");
    if (item != NULL) {
        lv_img_set_src(obj, GetImgSrc(item->valuestring));
    } else {
        lv_img_set_src(obj, &imgSwitch);
        lv_obj_align(obj, LV_ALIGN_RIGHT_MID, -24, 0);
    }

    item = cJSON_GetObjectItem(json, "cb");
    if (item != NULL) {
        lv_obj_add_event_cb(obj, GuiTemplateEventCbGet(item->valuestring), LV_EVENT_CLICKED, NULL);
    }

    cJSON *array = cJSON_GetObjectItem(json, "default");
    if (array != NULL) {
        int i = 0;
        for (cJSON *iter = array->child; iter != NULL; iter = iter->next) {
            lv_obj_t *parent = lv_obj_get_parent(obj);
            lv_obj_t *child = lv_obj_get_child(parent, lv_obj_get_child_cnt(parent) + iter->valueint);
            g_defaultVector[i++] = child;
        }
    }

    array = cJSON_GetObjectItem(json, "hidden");
    if (array != NULL) {
        int i = 0;
        for (cJSON *iter = array->child; iter != NULL; iter = iter->next) {
            lv_obj_t *parent = lv_obj_get_parent(obj);
            lv_obj_t *child = lv_obj_get_child(parent, lv_obj_get_child_cnt(parent) + iter->valueint);
            g_hiddenVector[i++] = child;
        }
    }

    return obj;
}

void *GuiWidgetTabView(lv_obj_t *parent, cJSON *json)
{
    lv_obj_t *obj = lv_tabview_create(parent, LV_DIR_TOP, 64);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x0), LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x0), LV_PART_ITEMS);
    lv_obj_set_style_border_width(obj, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_ITEMS);

    lv_obj_set_style_bg_color(obj, lv_color_hex(0), LV_PART_ITEMS);
    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_ITEMS);
    lv_obj_set_style_border_color(obj, lv_color_hex(0), LV_PART_ITEMS);

    g_tableView = obj;
    lv_obj_t *line;
    static lv_point_t points[2] = {{0, 0}, {408, 0}};
    line = (lv_obj_t *)GuiCreateLine(parent, points, 2);
    lv_obj_align(line, LV_ALIGN_TOP_LEFT, 36, 64);

    return obj;
}

void *GuiWidgetTabViewChild(lv_obj_t *parent, cJSON *json)
{
    cJSON *item = cJSON_GetObjectItem(json, "tab_name");
    lv_obj_t *obj = lv_tabview_add_tab(g_tableView, item->valuestring);

    lv_obj_t *tab_btns = lv_tabview_get_tab_btns(g_tableView);
    lv_obj_set_style_bg_color(tab_btns, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_font(tab_btns, g_defIllustrateFont, LV_PART_ITEMS);
    lv_obj_set_style_text_color(tab_btns, WHITE_COLOR, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(tab_btns, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_BOTTOM, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_width(tab_btns, 200);

    item = cJSON_GetObjectItem(json, "font");
    if (item != NULL) {
        const lv_font_t *font = GetLvglTextFont(item->valuestring);
        lv_obj_set_style_text_font(obj, font, LV_STATE_DEFAULT | LV_PART_MAIN);
    }

    item = cJSON_GetObjectItem(json, "text_color");
    if (item != NULL) {
        lv_obj_set_style_text_color(obj, lv_color_hex(item->valueint), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    item = cJSON_GetObjectItem(json, "opa");
    if (item != NULL) {
        lv_obj_set_style_text_opa(obj, item->valueint, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    g_analyzeTabview.obj[g_analyzeTabview.tabviewIndex] = obj;
    g_analyzeTabview.tabviewIndex++;

    return obj;
}

static void *GuiWidgetFactoryCreate(lv_obj_t *parent, cJSON *json)
{
    lv_obj_t *obj = NULL;
    cJSON *item = cJSON_GetObjectItem(json, "type");
    if (item == NULL) {
        item = cJSON_GetObjectItem(json, "table");
        if (item != NULL) {
            char typeBuf[16];
            g_analyzeArray[g_reMapIndex].typeFunc(typeBuf, g_totalData, sizeof(typeBuf));
            item = cJSON_GetObjectItem(item, typeBuf);
            if (item != NULL) {
                return GuiWidgetFactoryCreate(parent, item);
            }
        } else {
            return NULL;
        }
    }
    const char *type = item->valuestring;
    if (!type) {
        return NULL;
    }

    item = cJSON_GetObjectItem(json, "exist_func");
    if (item != NULL) {
        GetObjStateFunc func = GuiTemplateStateFuncGet(item->valuestring);
        if (!func(NULL, g_totalData)) {
            return NULL;
        }
    }

    if (0 == strcmp(type, "list")) {
        GuiWidgetList(parent, json);
        return NULL;
    }
    if (0 == strcmp(type, "container")) {
        obj = GuiWidgetContainer(parent, json);
    } else if (0 == strcmp(type, "img")) {
        obj = GuiWidgetImg(parent, json);
    } else if (0 == strcmp(type, "label")) {
        obj = GuiWidgetLabel(parent, json);
    } else if (0 == strcmp(type, "table")) {
        obj = GuiWidgetTable(parent, json);
    } else if (0 == strcmp(type, "tabview")) {
        obj = GuiWidgetTabView(parent, json);
    } else if (0 == strcmp(type, "tabview_child")) {
        obj = GuiWidgetTabViewChild(parent, json);
    } else if (0 == strcmp(type, "custom_container")) {
        obj = GuiWidgetCustomContainer(parent, json);
    } else {
        printf("json type is %s\n", type);
        return NULL;
    }
    GuiWidgetBaseInit(obj, json);
    return obj;
}

void SetParseTransactionResult(void* result)
{
    g_totalData = ((TransactionParseResult_DisplayTx *)result)->data;
}

static void* CreateTransactionDetailWidgets()
{
    int index = -1;
    for (int j = 0; j < NUMBER_OF_ARRAYS(g_analyzeArray); j++) {
        if (g_reMapIndex == g_analyzeArray[j].index) {
            index = j;
            break;
        }
    }
    if (index == -1) {
        return NULL;
    }
#ifdef COMPILE_SIMULATOR
    lv_fs_file_t fd;
    uint32_t size;
#define JSON_MAX_LEN (1024 * 100)
    char buf[JSON_MAX_LEN];
    if (LV_FS_RES_OK != lv_fs_open(&fd, g_analyzeArray[index].config, LV_FS_MODE_RD)) {
        printf("lv_fs_open failed %s\n", g_analyzeArray[index].config);
        return NULL;
    }

    lv_fs_read(&fd, buf, JSON_MAX_LEN, &size);
    buf[size] = '\0';
#endif

#ifdef COMPILE_SIMULATOR
    cJSON *paramJson = cJSON_Parse(buf);
#else
    cJSON *paramJson = cJSON_Parse(g_analyzeArray[index].config);
#endif
    if (paramJson == NULL) {
        printf("cJSON_Parse failed\n");
#ifdef COMPILE_SIMULATOR
        lv_fs_close(&fd);
#endif
        return NULL;
    }
    lv_obj_t *obj = GuiWidgetFactoryCreate(g_templateContainer, paramJson);
#ifdef COMPILE_SIMULATOR
    lv_fs_close(&fd);
#endif
    cJSON_Delete(paramJson);
    return obj;
}

void ParseTransaction(uint8_t index)
{
    g_reMapIndex = ViewTypeReMap(index);
    g_viewTypeIndex = index;

    for (int i = 0; i < NUMBER_OF_ARRAYS(g_analyzeArray); i++) {
        if (g_reMapIndex == g_analyzeArray[i].index) {
            GuiModelParseTransaction(g_analyzeArray[i].func);
            break;
        }
    }
}

GuiRemapViewType ViewTypeReMap(uint8_t viewType)
{
    switch (viewType) {
    case BtcNativeSegwitTx:
    case BtcSegwitTx:
    case BtcLegacyTx:
    case BtcTx:
#ifndef BTC_ONLY
    case LtcTx:
    case DashTx:
    case BchTx:
#endif
        return REMAPVIEW_BTC;
    case BtcMsg:
        return REMAPVIEW_BTC_MESSAGE;
#ifndef BTC_ONLY
    case EthTx:
        return REMAPVIEW_ETH;
    case EthPersonalMessage:
        return REMAPVIEW_ETH_PERSONAL_MESSAGE;
    case EthTypedData:
        return REMAPVIEW_ETH_TYPEDDATA;
    case TronTx:
        return REMAPVIEW_TRX;
    case CosmosTx:
    case CosmosEvmTx:
        return REMAPVIEW_COSMOS;
    case SuiTx:
        return REMAPVIEW_SUI;
    case SolanaTx:
        return REMAPVIEW_SOL;
    case SolanaMessage:
        return REMAPVIEW_SOL_MESSAGE;
    case AptosTx:
        return REMAPVIEW_APT;
    case CardanoTx:
        return REMAPVIEW_ADA;
    case XRPTx:
        return REMAPVIEW_XRP;
    case ArweaveTx:
        return REMAPVIEW_AR;
    case ArweaveMessage:
        return REMAPVIEW_AR_MESSAGE;
    case TonTx:
        return REMAPVIEW_TON;
    case TonSignProof:
        return REMAPVIEW_TON_SIGNPROOF;
#endif
    default:
        return REMAPVIEW_BUTT;
    }
    return REMAPVIEW_BUTT;
}

static lv_obj_t *g_imgCont = NULL;
void GuiAnalyzeViewInit(lv_obj_t *parent)
{
    uint16_t width = 0;
    g_imgCont = (lv_obj_t *)GuiCreateContainerWithParent(parent, 440, 530);
    lv_obj_align(g_imgCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_scroll_dir(g_imgCont, LV_DIR_VER);
    lv_obj_add_flag(g_imgCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_imgCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(g_imgCont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(g_imgCont, LV_OBJ_FLAG_SCROLL_ELASTIC);

    lv_obj_t *tabView = lv_tabview_create(g_imgCont, LV_DIR_TOP, 64);
    lv_obj_set_style_bg_color(tabView, lv_color_hex(0x0), LV_PART_MAIN);
    lv_obj_set_style_bg_color(tabView, lv_color_hex(0x0), LV_PART_ITEMS);
    lv_obj_set_style_border_width(tabView, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(tabView, lv_color_hex(0xffffff), LV_PART_ITEMS);
    lv_obj_clear_flag(tabView, LV_OBJ_FLAG_SCROLL_ELASTIC);

    lv_obj_set_style_bg_color(tabView, lv_color_hex(0), LV_PART_ITEMS);
    lv_obj_set_style_text_color(tabView, lv_color_hex(0xffffff), LV_PART_ITEMS);
    lv_obj_set_style_border_color(tabView, lv_color_hex(0), LV_PART_ITEMS);

    static lv_point_t points[2] = {{0, 0}, {408, 0}};
    lv_obj_t *line = (lv_obj_t *)GuiCreateLine(g_imgCont, points, 2);
    lv_obj_align(line, LV_ALIGN_TOP_LEFT, 0, 64);

    for (int i = 0; i < 2; i++) {
        lv_obj_t *tabChild;
        if (i == 0) {
            tabChild = lv_tabview_add_tab(tabView, _("Overview"));
            lv_obj_t *temp = GuiCreateIllustrateLabel(tabView, _("Overview"));
            width = lv_obj_get_self_width(temp) > 100 ? 300 : 200;
            lv_obj_del(temp);
        } else if (i == 1) {
            tabChild = lv_tabview_add_tab(tabView, _("Details"));
        }
        lv_obj_set_style_pad_all(tabChild, 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(tabChild, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(tabChild, 0, LV_PART_MAIN);
        lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tabView);
        lv_obj_set_style_bg_color(tab_btns, BLACK_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(tab_btns, g_defIllustrateFont, LV_PART_ITEMS);
        lv_obj_set_style_text_color(tab_btns, WHITE_COLOR, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_obj_set_style_text_color(tab_btns, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_BOTTOM, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_obj_set_style_text_opa(tab_btns, 255, LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_set_style_text_opa(tab_btns, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_width(tab_btns, width);
        int childCnt = lv_obj_get_child_cnt(g_analyzeTabview.obj[i]);
        int yOffset = 12;
        for (int j = 0; j < childCnt; j++) {
            lv_obj_t *child = lv_obj_get_child(g_analyzeTabview.obj[i], j);
            if (lv_obj_get_child_cnt(child) == 0) {
                lv_obj_align(child, LV_ALIGN_DEFAULT, 0, yOffset);
            } else {
                lv_obj_align(child, LV_ALIGN_TOP_MID, 0, yOffset);
            }
            yOffset = yOffset + lv_obj_get_content_height(child) + 16;
        }
        lv_obj_set_parent(g_analyzeTabview.obj[i], tabChild);
    }
}

void *GuiTemplateReload(lv_obj_t *parent, uint8_t index)
{
    g_tableView = NULL;
    g_analyzeTabview.tabviewIndex = 0;
    g_reMapIndex = ViewTypeReMap(index);
    if (g_reMapIndex == REMAPVIEW_BUTT) {
        return NULL;
    }
    g_viewTypeIndex = index;
    g_templateContainer = (lv_obj_t *)GuiCreateContainerWithParent(parent, 480, 542);
    lv_obj_align(g_templateContainer, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_scrollbar_mode(g_templateContainer, LV_SCROLLBAR_MODE_OFF);
    if (CreateTransactionDetailWidgets() == NULL) {
        lv_obj_del(g_templateContainer);
        return NULL;
    }

    if (g_tableView == NULL) {
        return g_templateContainer;
    }
    GuiAnalyzeViewInit(parent);
    lv_obj_del(g_templateContainer);
    return g_imgCont;
}

void GuiTemplateClosePage(void)
{
    for (uint32_t i = 0; i < NUMBER_OF_ARRAYS(g_analyzeArray); i++) {
        if (g_reMapIndex == g_analyzeArray[i].index) {
            g_analyzeArray[i].freeFunc();
            break;
        }
    }

    for (uint32_t i = 0; i < g_tableDataAmount; i++) {
        GuiAnalyzeFreeTable(g_tableData[i].row, g_tableData[i].col, g_tableData[i].totalPtr);
        memset_s(&g_tableData[i], sizeof(g_tableData[i]), 0, sizeof(g_tableData[i]));
    }
    g_tableDataAmount = 0;
}
