#ifndef GUI_QR_CODE_H
#define GUI_QR_CODE_H

#include "gui.h"


typedef UREncodeResult *(*GetUR)(void);
typedef lv_res_t (*UpdateFunc)(lv_obj_t *qrcode, const void *data, uint32_t data_len);

void ShowQrCode(GetUR func, lv_obj_t *qr);
void UpdateQrCode(GetUR func, lv_obj_t *qr, UpdateFunc updateFunc);
void CloseQRTimer(void);
void QRCodeControl(bool suspend);

#endif