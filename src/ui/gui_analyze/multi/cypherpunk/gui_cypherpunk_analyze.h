#ifdef CYPHERPUNK_VERSION
#ifndef _GUI_CYPHERPUNK_ANALYZE_WIDGETS_H
#define _GUI_CYPHERPUNK_ANALYZE_WIDGETS_H

// temper test the ethereum page view not for production usage
#define GUI_ANALYZE_OBJ_SURPLUS \
    { \
        REMAPVIEW_ZCASH, \
        "{\"name\":\"zcash_page\",\"type\":\"custom_container\",\"pos\":[36,0],\"size\":[408,900],\"bg_color\":0,\"custom_show_func\":\"GuiZcashOverview\"}", \
        GuiGetZcashGUIData, \
        NULL, \
        FreeZcashMemory, \
    }, \
    { \
        REMAPVIEW_XMR_OUTPUT, \
        "{\"name\":\"monero_output_page\",\"type\":\"container\",\"pos\":[36,0],\"size\":[408,542],\"bg_color\":0,\"children\":[{\"type\":\"custom_container\",\"pos\":[0,0],\"bg_opa\":0,\"custom_show_func\":\"GuiShowXmrOutputsDetails\"}]}", \
        GuiGetMoneroOutputData, \
        NULL, \
        FreeMoneroMemory, \
    }, \
    { \
        REMAPVIEW_XMR_UNSIGNED, \
        "{\"name\":\"monero_transaction_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,602],\"bg_color\":0,\"border_width\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowXmrTransactionOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowXmrTransactionDetails\"}]}]}", \
        GuiGetMoneroUnsignedTxData, \
        NULL, \
        FreeMoneroMemory, \
    }

#endif
#endif