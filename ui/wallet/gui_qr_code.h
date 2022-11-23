#ifndef GUI_QR_CODE_H
#define GUI_QR_CODE_H

#include "gui.h"
#include "rust.h"

typedef UREncodeResult *(*GetUR)(void);

void ShowQrCode(GetUR func, lv_obj_t *qr);
void CloseQRTimer(void);
void QRCodeControl(bool suspend);

#endif