#ifndef _GUI_USB_TRANSPORT_WIDGETS_H
#define _GUI_USB_TRANSPORT_WIDGETS_H
#include "eapdu_protocol_parser.h"

void GuiUSBTransportWidgetsInit(EAPDUResultPage_t *param);
void GuiUSBTransportWidgetsDeInit();
void GuiUSBTransportWidgetsRefresh();
void UsbGoToHomeView(void);

#endif