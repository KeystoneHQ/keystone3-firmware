#include "gui_zcash_template.h"
#ifdef CYPHERPUNK_VERSION
#ifndef COMPILE_SIMULATOR
#ifndef _GUI_CYPHERPUNK_ANALYZE_WIDGETS_H
#define _GUI_CYPHERPUNK_ANALYZE_WIDGETS_H
#include "gui_monero_template.h"
#define GUI_ANALYZE_OBJ_SURPLUS \
    { \
        REMAPVIEW_ZCASH, \
        GUI_ZCASH_PAGE_TEMPLATE, \
        GuiGetZcashGUIData, \
        NULL, \
        FreeZcashMemory, \
    }, \
    { \
        REMAPVIEW_XMR_OUTPUT, \
        GUI_XMR_OUTPUT_TEMPLATE, \
        GuiGetMoneroOutputData, \
        NULL, \
        FreeMoneroMemory, \
    }, \
    { \
        REMAPVIEW_XMR_UNSIGNED, \
        GUI_XMR_UNSIGNED_TEMPLATE, \
        GuiGetMoneroUnsignedTxData, \
        NULL, \
        FreeMoneroMemory, \
    }

#endif
#endif
#endif