#ifndef _SIMULATOR_MODEL_H
#define _SIMULATOR_MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "err_code.h"
#include "qrdecode_task.h"
#include "gui_lock_device_widgets.h"
extern bool g_fingerUnlockDeviceFlag;
extern bool g_fingerSingTransitionsFlag;
extern bool fingerRegisterState[3];
#define JSON_MAX_LEN (1024 * 100)
#define ACCOUNT_PUBLIC_HOME_COIN_PATH "C:/assets/coin.json"

size_t xPortGetFreeHeapSize(void);
int32_t CheckPasswordExisted(const char *password, uint8_t excludeIndex);
uint8_t GetCurrentAccountIndex(void);
uint8_t GetBatterPercent(void);
uint8_t GetCurrentDisplayPercent(void);
int Slip39CheckFirstWordList(char *wordsList, uint8_t wordCnt, uint8_t *threshold);
bool GetPassphraseQuickAccess(void);
bool SdCardInsert(void);
void SdCardIntHandler(void);
bool UsbInitState(void);
void CloseUsb();
void UpdateFingerSignFlag(uint8_t index, bool signFlag);
int FormatSdFatfs();
int FatfsFileWrite(const char* path, const uint8_t *data, uint32_t len);
int32_t read_qrcode();
char *FatfsFileRead(const char* path);
uint8_t *FatfsFileReadBytes(const char* path, uint32_t* readBytes);
void FatfsGetFileName(const char *path, char *fileName[], uint32_t maxLen, uint32_t *number, const char *contain);
uint32_t GetCurrentStampTime(void);
bool FatfsFileExist(const char *path);
bool GetEnsName(const char *addr, char *name);
void FpWipeManageInfo(void);
void SetPageLockScreen(bool enable);
uint8_t GetFingerSignFlag(void);
bool IsPreviousLockScreenEnable(void);
void SetLockScreen(bool enable);

extern bool g_reboot;

#undef GUI_ANALYZE_OBJ_SURPLUS
#ifdef CYPHERPUNK_VERSION
#define GUI_ANALYZE_OBJ_SURPLUS \
    { \
        REMAPVIEW_ZCASH, \
        PC_SIMULATOR_PATH "/page_zcash.json", \
        GuiGetZcashGUIData, \
        NULL, \
        FreeZcashMemory, \
    }, \
    { \
        REMAPVIEW_XMR_OUTPUT, \
        PC_SIMULATOR_PATH "/page_xmr_output.json", \
        GuiGetMoneroOutputData, \
        NULL, \
        FreeMoneroMemory, \
    }, \
    { \
        REMAPVIEW_XMR_UNSIGNED, \
        PC_SIMULATOR_PATH "/page_xmr_unsigned.json", \
        GuiGetMoneroUnsignedTxData, \
        NULL, \
        FreeMoneroMemory, \
    }
#endif

#ifdef WEB3_VERSION
#define GUI_ANALYZE_OBJ_SURPLUS           \
    { \
        REMAPVIEW_ETH, \
        PC_SIMULATOR_PATH "/page_eth.json", \
        GuiGetEthData, \
        GetEthTransType, \
        FreeEthMemory, \
    }, \
    { \
        REMAPVIEW_ETH_PERSONAL_MESSAGE, \
        PC_SIMULATOR_PATH "/page_eth_person.json", \
        GuiGetEthPersonalMessage, \
        GetEthPersonalMessageType, \
        FreeEthMemory, \
    }, \
    { \
        REMAPVIEW_ETH_TYPEDDATA, \
        PC_SIMULATOR_PATH "/page_eth_type.json", \
        GuiGetEthTypeData, \
        NULL, \
        FreeEthMemory, \
    }, \
    { \
        REMAPVIEW_TRX, \
        PC_SIMULATOR_PATH "/page_eth.json", \
        GuiGetTrxData, \
        NULL, \
        FreeTrxMemory, \
    }, \
    { \
        REMAPVIEW_COSMOS, \
        PC_SIMULATOR_PATH "/page_cosmos.json", \
        GuiGetCosmosData, \
        GuiGetCosmosTmpType, \
        FreeCosmosMemory, \
    }, \
    { \
        REMAPVIEW_SUI, \
        PC_SIMULATOR_PATH "/page_sui.json", \
        GuiGetSuiData, \
        NULL, \
        FreeSuiMemory, \
    }, \
    { \
        REMAPVIEW_SUI_SIGN_MESSAGE_HASH, \
        PC_SIMULATOR_PATH "/page_sign_hash.json", \
        GuiGetSuiSignMessageHashData, \
        NULL, \
        FreeSuiMemory \
    }, \
    { \
        REMAPVIEW_SOL, \
        PC_SIMULATOR_PATH "/page_sol.json", \
        GuiGetSolData, \
        NULL, \
        FreeSolMemory, \
    }, \
    { \
        REMAPVIEW_SOL_MESSAGE, \
        PC_SIMULATOR_PATH "/page_sol_message.json", \
        GuiGetSolMessageData, \
        GetSolMessageType, \
        FreeSolMemory, \
    }, \
    { \
        REMAPVIEW_APT, \
        PC_SIMULATOR_PATH "/page_eth.json", \
        GuiGetAptosData, \
        NULL, \
        FreeAptosMemory, \
    }, \
    { \
        REMAPVIEW_ADA, \
        PC_SIMULATOR_PATH "/page_ada.json", \
        GuiGetAdaData, \
        NULL, \
        FreeAdaMemory, \
    }, \
    { \
        REMAPVIEW_ADA_SIGN_TX_HASH, \
        PC_SIMULATOR_PATH "/page_sign_ada_tx_hash.json", \
        GuiGetAdaSignTxHashData, \
        NULL, \
        FreeAdaMemory \
    }, \
    { \
        REMAPVIEW_ADA_SIGN_DATA, \
        PC_SIMULATOR_PATH "/page_ada_sign_data.json", \
        GuiGetAdaSignDataData, \
        NULL, \
        FreeAdaSignDataMemory, \
    }, \
    { \
        REMAPVIEW_ADA_CATALYST, \
        PC_SIMULATOR_PATH "/page_ada_catalyst.json", \
        GuiGetAdaCatalyst, \
        NULL, \
        FreeAdaCatalystMemory, \
    }, \
    { \
        REMAPVIEW_XRP, \
        PC_SIMULATOR_PATH "/page_xrp.json", \
        GuiGetXrpData, \
        NULL, \
        FreeXrpMemory, \
    }, \
    { \
        REMAPVIEW_AR, \
        PC_SIMULATOR_PATH "/page_ar.json", \
        GuiGetArData, \
        NULL, \
        FreeArMemory, \
    }, \
    { \
        REMAPVIEW_AR_MESSAGE, \
        PC_SIMULATOR_PATH "/page_ar_message.json", \
        GuiGetArData, \
        NULL, \
        FreeArMemory, \
    }, \
    { \
        REMAPVIEW_STELLAR, \
        PC_SIMULATOR_PATH "/page_stellar.json", \
        GuiGetStellarData, \
        NULL, \
        FreeStellarMemory, \
    }, \
    { \
        REMAPVIEW_STELLAR_HASH, \
        PC_SIMULATOR_PATH "/page_stellar_hash.json", \
        GuiGetStellarData, \
        NULL, \
        FreeStellarMemory, \
    }, \
    { \
        REMAPVIEW_AR_DATAITEM, \
        PC_SIMULATOR_PATH "/page_ar_data_item.json", \
        GuiGetArData, \
        NULL, \
        FreeArMemory, \
    }, \
    { \
        REMAPVIEW_TON, \
        PC_SIMULATOR_PATH "/page_ton.json", \
        GuiGetTonGUIData, \
        NULL, \
        FreeArMemory, \
    }, \
    { \
        REMAPVIEW_TON_SIGNPROOF, \
        PC_SIMULATOR_PATH "/page_ton_proof.json", \
        GuiGetTonProofGUIData, \
        NULL, \
        FreeArMemory, \
    }, \
    { \
        REMAPVIEW_AVAX, \
        PC_SIMULATOR_PATH "/page_avax.json", \
        GuiGetAvaxGUIData, \
        NULL, \
        FreeAvaxMemory, \
    },
#endif

#endif