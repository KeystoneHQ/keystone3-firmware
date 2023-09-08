#ifndef _GUI_QRCODE_WIDGETS_H
#define _GUI_QRCODE_WIDGETS_H

void GuiQrCodeScreenInit();
void GuiQrCodeRefresh(void);
void GuiQrCodeDeInit(void);
void GuiQrCodeScanResult(bool result, void *errCode);
void GuiQrCodeVerifyPasswordSuccess(void);
void GuiQrCodeDealFingerRecognize(void *param);
void GuiQrCodeVerifyPasswordErrorCount(void *param);
void GuiClearQrcodeSignCnt(void);

#endif /* _GUI_QRCODE_WIDGETS_H */


