#ifndef _GUI_MODEL_H
#define _GUI_MODEL_H

#ifndef COMPILE_SIMULATOR
#include "user_sqlite3.h"
#include "fingerprint_process.h"
#include "screen_manager.h"
#include "anti_tamper.h"
#include "presetting.h"
#include "drv_sdcard.h"
#include "log.h"
#include "presetting.h"
#include "anti_tamper.h"
#else
#include "simulator_model.h"
#endif
#include "rsa.h"
#include "drv_rtc.h"
#include "drv_battery.h"
#include "gui_animating_qrcode.h"
#include "account_manager.h"

#define MAX_LOGIN_PASSWORD_ERROR_COUNT  10
#define MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX 4
#define MAX_CURRENT_PASSWORD_ERROR_COUNT_WIPE_DEVICE 14

typedef struct {
    uint8_t threShold;
    uint8_t memberCnt;
    uint8_t wordCnt;
    bool forget;
} Slip39Data_t;

typedef struct {
    uint8_t wordCnt;
    bool forget;
} Bip39Data_t;

typedef struct {
    bool forget;
} TonData_t;

typedef struct {
    uint8_t iconIndex;
    char name[WALLET_NAME_MAX_LEN + 1];
} WalletDesc_t;

typedef struct PasswordVerifyResult {
    void *signal;
    uint16_t errorCount;
} PasswordVerifyResult_t;

typedef void *(*ReturnVoidPointerFunc)(void);

void GuiModelWriteSe(void);
void GuiModelTonWriteSe(void);
void GuiModelSlip39CalWriteSe(Slip39Data_t slip39);
void GuiModelBip39CalWriteSe(Bip39Data_t bip39);
void GuiModelTonCalWriteSe(TonData_t ton);
void GuiModelSettingSaveWalletDesc(WalletDesc_t *wallet);
void GuiModelSettingDelWalletDesc(void);
void GuiModelLockedDeviceDelAllWalletDesc(void);
void GuiModelChangeAccountPassWord(void);
void GuiModelVerifyAccountPassWord(uint16_t *param);
void GuiModelBip39UpdateMnemonic(uint8_t wordCnt);
void GuiModelBip39UpdateMnemonicWithDiceRolls(uint8_t wordCnt);
void GuiModelTonUpdateMnemonic(void);
void GuiModelSlip39UpdateMnemonic(Slip39Data_t slip39);
void GuiModelSlip39UpdateMnemonicWithDiceRolls(Slip39Data_t slip39);
void GuiModelBip39RecoveryCheck(uint8_t wordsCnt);
void GuiModelTonRecoveryCheck();
void GuiModeGetWalletDesc(void);
void GuiModeGetAccount(void);
void GuiModeControlQrDecode(bool en);
void GuiModelSlip39WriteSe(uint8_t wordCnt);
void GuiModelBip39ForgetPassword(uint8_t wordsCnt);
void GuiModelTonForgetPassword();
void GuiModelSlip39ForgetPassword(Slip39Data_t slip39);
void GuiModelWriteLastLockDeviceTime(uint32_t time);
void GuiModelCalculateWebAuthCode(void *webAuthData);
void GuiModelCopySdCardOta(void);
void GuiModelURGenerateQRCode(GenerateUR func);
void GuiModelURUpdate(void);
void GuiModelURClear(void);
void GuiModelCheckTransaction(ViewType ViewType);
int32_t RsaGenerateKeyPair(bool needEmitSignal);
void GuiModelRsaGenerateKeyPair(void);
void GuiModelTransactionCheckResultClear(void);
void GuiModelParseTransaction(ReturnVoidPointerFunc func);
bool ModelGetPassphraseQuickAccess(void);
void GuiModelCalculateCheckSum(void);
void GuiModelStopCalculateCheckSum(void);
void GuiModelSettingWritePassphrase(void);
void GuiModelCalculateBinSha256(void);
void GuiModelFormatMicroSd(void);

#endif /* _GUI_MODEL_H */

