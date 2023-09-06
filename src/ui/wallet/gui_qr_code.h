#ifndef GUI_QR_CODE_H
#define GUI_QR_CODE_H

#include "gui.h"
#include "rust.h"

typedef UREncodeResult *(*GetUR)(void);

void ShowQrCode(GetUR func, lv_obj_t *qr);
void UpdateQrCode(GetUR func, lv_obj_t *qr, lv_res_t *updateFunc(lv_obj_t * qrcode, const void * data, uint32_t data_len));
void CloseQRTimer(void);
void QRCodeControl(bool suspend);

#endif