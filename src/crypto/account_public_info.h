#ifndef _ACCOUNT_PUBLIC_INFO_H
#define _ACCOUNT_PUBLIC_INFO_H

#include "stdint.h"
#include "stdbool.h"
#include "gui_home_widgets.h"
#include "multi_sig_wallet_manager.h"
#include "rsa.h"
#include "bip39.h"

typedef struct {
    int32_t addressType;
    int32_t addressIndex;
} AccountSettingsItem_t;

typedef enum {
    XPUB_TYPE_BTC,
    XPUB_TYPE_BTC_LEGACY,
    XPUB_TYPE_BTC_NATIVE_SEGWIT,
    XPUB_TYPE_BTC_TAPROOT,
#ifdef WEB3_VERSION
    XPUB_TYPE_LTC,
    XPUB_TYPE_LTC_NATIVE_SEGWIT,
    XPUB_TYPE_DOGE,
    XPUB_TYPE_DASH,
    XPUB_TYPE_BCH,
    XPUB_TYPE_ETH_BIP44_STANDARD,
    XPUB_TYPE_ETH_LEDGER_LEGACY,
    XPUB_TYPE_ETH_LEDGER_LIVE_0,
    XPUB_TYPE_ETH_LEDGER_LIVE_1,
    XPUB_TYPE_ETH_LEDGER_LIVE_2,
    XPUB_TYPE_ETH_LEDGER_LIVE_3,
    XPUB_TYPE_ETH_LEDGER_LIVE_4,
    XPUB_TYPE_ETH_LEDGER_LIVE_5,
    XPUB_TYPE_ETH_LEDGER_LIVE_6,
    XPUB_TYPE_ETH_LEDGER_LIVE_7,
    XPUB_TYPE_ETH_LEDGER_LIVE_8,
    XPUB_TYPE_ETH_LEDGER_LIVE_9,
    XPUB_TYPE_TRX,
    XPUB_TYPE_COSMOS,
    XPUB_TYPE_SCRT,
    XPUB_TYPE_CRO,
    XPUB_TYPE_IOV,
    XPUB_TYPE_BLD,
    XPUB_TYPE_KAVA,
    XPUB_TYPE_TERRA,
    XPUB_TYPE_XRP,
    XPUB_TYPE_THOR,
    XPUB_TYPE_AVAX_BIP44_STANDARD,
    XPUB_TYPE_AVAX_X_P,
    XPUB_TYPE_IOTA_0,
    XPUB_TYPE_IOTA_1,
    XPUB_TYPE_IOTA_2,
    XPUB_TYPE_IOTA_3,
    XPUB_TYPE_IOTA_4,
    XPUB_TYPE_IOTA_5,
    XPUB_TYPE_IOTA_6,
    XPUB_TYPE_IOTA_7,
    XPUB_TYPE_IOTA_8,
    XPUB_TYPE_IOTA_9,
    XPUB_TYPE_SOL_BIP44_0,
    XPUB_TYPE_SOL_BIP44_1,
    XPUB_TYPE_SOL_BIP44_2,
    XPUB_TYPE_SOL_BIP44_3,
    XPUB_TYPE_SOL_BIP44_4,
    XPUB_TYPE_SOL_BIP44_5,
    XPUB_TYPE_SOL_BIP44_6,
    XPUB_TYPE_SOL_BIP44_7,
    XPUB_TYPE_SOL_BIP44_8,
    XPUB_TYPE_SOL_BIP44_9,
    XPUB_TYPE_SOL_BIP44_10,
    XPUB_TYPE_SOL_BIP44_11,
    XPUB_TYPE_SOL_BIP44_12,
    XPUB_TYPE_SOL_BIP44_13,
    XPUB_TYPE_SOL_BIP44_14,
    XPUB_TYPE_SOL_BIP44_15,
    XPUB_TYPE_SOL_BIP44_16,
    XPUB_TYPE_SOL_BIP44_17,
    XPUB_TYPE_SOL_BIP44_18,
    XPUB_TYPE_SOL_BIP44_19,
    XPUB_TYPE_SOL_BIP44_20,
    XPUB_TYPE_SOL_BIP44_21,
    XPUB_TYPE_SOL_BIP44_22,
    XPUB_TYPE_SOL_BIP44_23,
    XPUB_TYPE_SOL_BIP44_24,
    XPUB_TYPE_SOL_BIP44_25,
    XPUB_TYPE_SOL_BIP44_26,
    XPUB_TYPE_SOL_BIP44_27,
    XPUB_TYPE_SOL_BIP44_28,
    XPUB_TYPE_SOL_BIP44_29,
    XPUB_TYPE_SOL_BIP44_30,
    XPUB_TYPE_SOL_BIP44_31,
    XPUB_TYPE_SOL_BIP44_32,
    XPUB_TYPE_SOL_BIP44_33,
    XPUB_TYPE_SOL_BIP44_34,
    XPUB_TYPE_SOL_BIP44_35,
    XPUB_TYPE_SOL_BIP44_36,
    XPUB_TYPE_SOL_BIP44_37,
    XPUB_TYPE_SOL_BIP44_38,
    XPUB_TYPE_SOL_BIP44_39,
    XPUB_TYPE_SOL_BIP44_40,
    XPUB_TYPE_SOL_BIP44_41,
    XPUB_TYPE_SOL_BIP44_42,
    XPUB_TYPE_SOL_BIP44_43,
    XPUB_TYPE_SOL_BIP44_44,
    XPUB_TYPE_SOL_BIP44_45,
    XPUB_TYPE_SOL_BIP44_46,
    XPUB_TYPE_SOL_BIP44_47,
    XPUB_TYPE_SOL_BIP44_48,
    XPUB_TYPE_SOL_BIP44_49,
    XPUB_TYPE_SOL_BIP44_ROOT,
    XPUB_TYPE_SOL_BIP44_CHANGE_0,
    XPUB_TYPE_SOL_BIP44_CHANGE_1,
    XPUB_TYPE_SOL_BIP44_CHANGE_2,
    XPUB_TYPE_SOL_BIP44_CHANGE_3,
    XPUB_TYPE_SOL_BIP44_CHANGE_4,
    XPUB_TYPE_SOL_BIP44_CHANGE_5,
    XPUB_TYPE_SOL_BIP44_CHANGE_6,
    XPUB_TYPE_SOL_BIP44_CHANGE_7,
    XPUB_TYPE_SOL_BIP44_CHANGE_8,
    XPUB_TYPE_SOL_BIP44_CHANGE_9,
    XPUB_TYPE_SOL_BIP44_CHANGE_10,
    XPUB_TYPE_SOL_BIP44_CHANGE_11,
    XPUB_TYPE_SOL_BIP44_CHANGE_12,
    XPUB_TYPE_SOL_BIP44_CHANGE_13,
    XPUB_TYPE_SOL_BIP44_CHANGE_14,
    XPUB_TYPE_SOL_BIP44_CHANGE_15,
    XPUB_TYPE_SOL_BIP44_CHANGE_16,
    XPUB_TYPE_SOL_BIP44_CHANGE_17,
    XPUB_TYPE_SOL_BIP44_CHANGE_18,
    XPUB_TYPE_SOL_BIP44_CHANGE_19,
    XPUB_TYPE_SOL_BIP44_CHANGE_20,
    XPUB_TYPE_SOL_BIP44_CHANGE_21,
    XPUB_TYPE_SOL_BIP44_CHANGE_22,
    XPUB_TYPE_SOL_BIP44_CHANGE_23,
    XPUB_TYPE_SOL_BIP44_CHANGE_24,
    XPUB_TYPE_SOL_BIP44_CHANGE_25,
    XPUB_TYPE_SOL_BIP44_CHANGE_26,
    XPUB_TYPE_SOL_BIP44_CHANGE_27,
    XPUB_TYPE_SOL_BIP44_CHANGE_28,
    XPUB_TYPE_SOL_BIP44_CHANGE_29,
    XPUB_TYPE_SOL_BIP44_CHANGE_30,
    XPUB_TYPE_SOL_BIP44_CHANGE_31,
    XPUB_TYPE_SOL_BIP44_CHANGE_32,
    XPUB_TYPE_SOL_BIP44_CHANGE_33,
    XPUB_TYPE_SOL_BIP44_CHANGE_34,
    XPUB_TYPE_SOL_BIP44_CHANGE_35,
    XPUB_TYPE_SOL_BIP44_CHANGE_36,
    XPUB_TYPE_SOL_BIP44_CHANGE_37,
    XPUB_TYPE_SOL_BIP44_CHANGE_38,
    XPUB_TYPE_SOL_BIP44_CHANGE_39,
    XPUB_TYPE_SOL_BIP44_CHANGE_40,
    XPUB_TYPE_SOL_BIP44_CHANGE_41,
    XPUB_TYPE_SOL_BIP44_CHANGE_42,
    XPUB_TYPE_SOL_BIP44_CHANGE_43,
    XPUB_TYPE_SOL_BIP44_CHANGE_44,
    XPUB_TYPE_SOL_BIP44_CHANGE_45,
    XPUB_TYPE_SOL_BIP44_CHANGE_46,
    XPUB_TYPE_SOL_BIP44_CHANGE_47,
    XPUB_TYPE_SOL_BIP44_CHANGE_48,
    XPUB_TYPE_SOL_BIP44_CHANGE_49,
    XPUB_TYPE_SUI_0,
    XPUB_TYPE_SUI_1,
    XPUB_TYPE_SUI_2,
    XPUB_TYPE_SUI_3,
    XPUB_TYPE_SUI_4,
    XPUB_TYPE_SUI_5,
    XPUB_TYPE_SUI_6,
    XPUB_TYPE_SUI_7,
    XPUB_TYPE_SUI_8,
    XPUB_TYPE_SUI_9,
    XPUB_TYPE_APT_0,
    XPUB_TYPE_APT_1,
    XPUB_TYPE_APT_2,
    XPUB_TYPE_APT_3,
    XPUB_TYPE_APT_4,
    XPUB_TYPE_APT_5,
    XPUB_TYPE_APT_6,
    XPUB_TYPE_APT_7,
    XPUB_TYPE_APT_8,
    XPUB_TYPE_APT_9,
    XPUB_TYPE_ADA_0,
    XPUB_TYPE_ADA_1,
    XPUB_TYPE_ADA_2,
    XPUB_TYPE_ADA_3,
    XPUB_TYPE_ADA_4,
    XPUB_TYPE_ADA_5,
    XPUB_TYPE_ADA_6,
    XPUB_TYPE_ADA_7,
    XPUB_TYPE_ADA_8,
    XPUB_TYPE_ADA_9,
    XPUB_TYPE_ADA_10,
    XPUB_TYPE_ADA_11,
    XPUB_TYPE_ADA_12,
    XPUB_TYPE_ADA_13,
    XPUB_TYPE_ADA_14,
    XPUB_TYPE_ADA_15,
    XPUB_TYPE_ADA_16,
    XPUB_TYPE_ADA_17,
    XPUB_TYPE_ADA_18,
    XPUB_TYPE_ADA_19,
    XPUB_TYPE_ADA_20,
    XPUB_TYPE_ADA_21,
    XPUB_TYPE_ADA_22,
    XPUB_TYPE_ADA_23,
    XPUB_TYPE_LEDGER_ADA_0,
    XPUB_TYPE_LEDGER_ADA_1,
    XPUB_TYPE_LEDGER_ADA_2,
    XPUB_TYPE_LEDGER_ADA_3,
    XPUB_TYPE_LEDGER_ADA_4,
    XPUB_TYPE_LEDGER_ADA_5,
    XPUB_TYPE_LEDGER_ADA_6,
    XPUB_TYPE_LEDGER_ADA_7,
    XPUB_TYPE_LEDGER_ADA_8,
    XPUB_TYPE_LEDGER_ADA_9,
    XPUB_TYPE_LEDGER_ADA_10,
    XPUB_TYPE_LEDGER_ADA_11,
    XPUB_TYPE_LEDGER_ADA_12,
    XPUB_TYPE_LEDGER_ADA_13,
    XPUB_TYPE_LEDGER_ADA_14,
    XPUB_TYPE_LEDGER_ADA_15,
    XPUB_TYPE_LEDGER_ADA_16,
    XPUB_TYPE_LEDGER_ADA_17,
    XPUB_TYPE_LEDGER_ADA_18,
    XPUB_TYPE_LEDGER_ADA_19,
    XPUB_TYPE_LEDGER_ADA_20,
    XPUB_TYPE_LEDGER_ADA_21,
    XPUB_TYPE_LEDGER_ADA_22,
    XPUB_TYPE_LEDGER_ADA_23,
    XPUB_TYPE_ARWEAVE,
    XPUB_TYPE_STELLAR_0,
    XPUB_TYPE_STELLAR_1,
    XPUB_TYPE_STELLAR_2,
    XPUB_TYPE_STELLAR_3,
    XPUB_TYPE_STELLAR_4,
    XPUB_TYPE_TON_BIP39,
    XPUB_TYPE_TON_NATIVE,
    PUBLIC_INFO_TON_CHECKSUM,
#endif

#ifdef CYPHERPUNK_VERSION
    ZCASH_UFVK_ENCRYPTED_0,
    XPUB_TYPE_MONERO_0,
    XPUB_TYPE_MONERO_PVK_0,
#endif

#ifdef BTC_ONLY
    XPUB_TYPE_BTC_TEST,
    XPUB_TYPE_BTC_LEGACY_TEST,
    XPUB_TYPE_BTC_NATIVE_SEGWIT_TEST,
    XPUB_TYPE_BTC_TAPROOT_TEST,

    XPUB_TYPE_BTC_MULTI_SIG_P2SH,
    XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH,
    XPUB_TYPE_BTC_MULTI_SIG_P2WSH,
    XPUB_TYPE_BTC_MULTI_SIG_P2SH_TEST,
    XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH_TEST,
    XPUB_TYPE_BTC_MULTI_SIG_P2WSH_TEST,

#endif
    XPUB_TYPE_NUM,
} ChainType;

bool GetIsTempAccount(void);
int32_t AccountPublicInfoSwitch(uint8_t accountIndex, const char *password, bool newKey);
int32_t TempAccountPublicInfo(uint8_t accountIndex, const char *password, bool set);
void DeleteAccountPublicInfo(uint8_t accountIndex);
char *GetCurrentAccountPublicKey(ChainType chain);
uint8_t SpecifiedXPubExist(const char *xPub, bool isTon);
void AccountPublicInfoTest(int argc, char *argv[]);
bool GetFirstReceive(const char* chainName);
void SetFirstReceive(const char* chainName, bool isFirst);
void AccountPublicHomeCoinGet(WalletState_t *walletList, uint8_t count);
char *GetXPubPath(uint8_t index);
void CalculateTonChecksum(uint8_t *entropy, char* output);
uint32_t GetAccountReceiveIndex(const char* chainName);
void SetAccountReceiveIndex(const char* chainName, uint32_t index);
uint32_t GetAccountReceivePath(const char* chainName);
void SetAccountReceivePath(const char* chainName, uint32_t index);
uint32_t GetAccountIndex(const char* chainName);
void SetAccountIndex(const char* chainName, uint32_t index);
void SetConnectWalletPathIndex(const char* walletName, uint32_t index);
uint32_t GetConnectWalletPathIndex(const char* walletName);
uint32_t GetConnectWalletAccountIndex(const char* walletName);
void SetConnectWalletAccountIndex(const char* walletName, uint32_t index);
uint32_t GetConnectWalletNetwork(const char* walletName);
void SetConnectWalletNetwork(const char* walletName, uint32_t index);

#ifdef WEB3_VERSION
ChainType CheckSolPathSupport(char *path);
#endif

#ifdef BTC_ONLY
void ExportMultiSigXpub(ChainType chainType);
void MultiSigWalletSave(MultiSigWalletManager_t *manager);
int32_t MultiSigWalletGet(uint8_t accountIndex, const char *password, MultiSigWalletManager_t *manager);

void SetAccountMultiReceiveIndex(uint32_t index, char *verifyCode);
uint32_t GetAccountMultiReceiveIndex(char *verifyCode);
uint32_t GetAccountTestReceiveIndex(const char* chainName);
void SetAccountTestReceiveIndex(const char* chainName, uint32_t index);
uint32_t GetAccountTestReceivePath(const char* chainName);
void SetAccountTestReceivePath(const char* chainName, uint32_t index);
void DeleteAccountMultiReceiveIndex(const char* chainName, char *verifyCode);
#endif
#endif

