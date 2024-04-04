#include "gui_multisig_transaction_signature_widgets.h"
#include <gui_page.h>

static PageWidget_t *g_pageWidget;

void GuiMultisigTransactionSignatureNVSBarInit();

void GuiMultisigTransactionSignaureWidgetsInit(){
    g_pageWidget = CreatePageWidget();
    GuiMultisigTransactionSignatureNVSBarInit();
    // GuiCreateSignatureQRCode(g_pageWidget->contentZone);
}

void GuiMultisigTransactionSignatureNVSBarInit(){
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, GoToHomeViewHandler, NULL);
}

void GuiMultisigTransactionSignaureWidgetsDeInit(){

}
void GuiMultisigTransactionSignaureWidgetsRefresh(){

}