#ifdef CYBERPUNK_VERSION
#ifndef _GUI_CYBERPUNK_ANALYZE_WIDGETS_H
#define _GUI_CYBERPUNK_ANALYZE_WIDGETS_H

// temper test the ethereum page view not for production usage
#define GUI_ANALYZE_OBJ_SURPLUS \
    { \
        REMAPVIEW_ZCASH, \
        "{\"name\":\"zcash_page\",\"type\":\"custom_container\",\"pos\":[36,0],\"size\":[408,900],\"bg_color\":0,\"custom_show_func\":\"GuiZcashOverview\"}", \
        GuiGetZcashGUIData, \
        NULL, \
        FreeZcashMemory, \
    }

#endif
#endif