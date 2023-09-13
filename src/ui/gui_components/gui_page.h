#ifndef _GUI_PAGE_H
#define _GUI_PAGE_H

#include "gui.h"
#include "gui_status_bar.h"

typedef struct PageWidget {
    lv_obj_t *page;
    lv_obj_t *navBar;
    lv_obj_t *contentZone;
    NavBarWidget_t *navBarWidget;
} PageWidget_t;

PageWidget_t *CreatePageWidget(void);

void DestroyPageWidget(PageWidget_t *pageWidget);


#endif /* _GUI_PAGE_H */
