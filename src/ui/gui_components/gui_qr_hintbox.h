#ifndef _GUI_QR_HINTBOX_H
#define _GUI_QR_HINTBOX_H

void GuiQRHintBoxRemove();
void GuiQRCodeHintBoxOpen(const char *qrdata, const char *title, const  char *subtitle);
void GuiQRCodeHintBoxOpenBig(const char *qrdata, const char *title, const char *content, const char *url);
void GuiNormalHitBoxOpen(const char *title, const char *content);
bool GuiQRHintBoxIsActive();

#endif