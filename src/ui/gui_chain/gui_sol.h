#ifndef _GUI_SOL_H
#define _GUI_SOL_H

#include "rust.h"

typedef struct {
  char *version;
  char *name;
  char *account;
} SolProgramInfo_t;

void GuiSetSolUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetSolData(void);
UREncodeResult *GuiGetSolSignQrCodeData(void);
PtrT_TransactionCheckResult GuiGetSolCheckResult(void);
void *GuiGetSolMessageData(void);
void GetSolMessageType(void *indata, void *param);
void GetSolMessageFrom(void *indata, void *param);
void GetSolMessageUtf8(void *indata, void *param);
void GetSolMessageRaw(void *indata, void *param);

void FreeSolMemory(void);

void GuiShowSolTxOverview(lv_obj_t *parent, void *g_totalData);
void GuiShowSolTxDetail(lv_obj_t *parent, void *g_totalData);
#endif