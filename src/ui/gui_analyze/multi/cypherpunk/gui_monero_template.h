#ifndef GUI_MONERO_TEMPLATE_H
#define GUI_MONERO_TEMPLATE_H

#define GUI_XMR_OUTPUT_TEMPLATE "{\"name\":\"monero_output_page\",\"type\":\"container\",\"pos\":[36,0],\"size\":[408,542],\"bg_color\":0,\"children\":[{\"type\":\"custom_container\",\"pos\":[0,0],\"bg_opa\":0,\"custom_show_func\":\"GuiShowXmrOutputsDetails\"}]}"

#define GUI_XMR_UNSIGNED_TEMPLATE "{\"name\":\"monero_transaction_page\",\"type\":\"tabview\",\"pos\":[36,0],\"size\":[408,602],\"bg_color\":0,\"border_width\":0,\"children\":[{\"type\":\"tabview_child\",\"index\":1,\"tab_name\":\"Overview\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowXmrTransactionOverview\"}]},{\"type\":\"tabview_child\",\"index\":2,\"tab_name\":\"Details\",\"text_color\":16777215,\"font\":\"openSansEnIllustrate\",\"children\":[{\"type\":\"custom_container\",\"bg_color\":0,\"bg_opa\":0,\"pos\":[0,12],\"custom_show_func\":\"GuiShowXmrTransactionDetails\"}]}]}"

#endif
