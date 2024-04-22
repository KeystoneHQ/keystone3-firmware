#include "gui_page.h"
#include "user_memory.h"

PageWidget_t *CreatePageWidget(void)
{
    PageWidget_t *pageWidget = SRAM_MALLOC(sizeof(PageWidget_t));

    lv_obj_t *page = GuiCreateContainerWithParent(lv_scr_act(), lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                     GUI_MAIN_AREA_OFFSET_NEW);
    lv_obj_align(page, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET_NEW);
    lv_obj_add_flag(page, LV_OBJ_FLAG_CLICKABLE);
    pageWidget->page = page;

    lv_obj_t *navBar = GuiCreateContainerWithParent(page, lv_obj_get_width(lv_scr_act()), GUI_NAV_BAR_HEIGHT);
    lv_obj_align(navBar, LV_ALIGN_DEFAULT, 0, 0);
    lv_obj_set_style_radius(navBar, 0, 0);
    pageWidget->navBar = navBar;

    lv_obj_t *contentZone = GuiCreateContainerWithParent(page, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET_NEW - GUI_NAV_BAR_HEIGHT);
    lv_obj_align(contentZone, LV_ALIGN_DEFAULT, 0, GUI_NAV_BAR_HEIGHT);
    lv_obj_add_flag(contentZone, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_radius(contentZone, 0, 0);
    pageWidget->contentZone = contentZone;

    pageWidget->navBarWidget = CreateNavBarWidget(navBar);

    return pageWidget;
}

void UpdatePageContentZone(PageWidget_t *pageWidget)
{
    if (pageWidget != NULL && pageWidget->contentZone != NULL && lv_obj_is_valid(pageWidget->contentZone)) {
        lv_obj_del(pageWidget->contentZone);
        lv_obj_t *contentZone = GuiCreateContainerWithParent(pageWidget->page, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET_NEW - GUI_NAV_BAR_HEIGHT);
        lv_obj_align(contentZone, LV_ALIGN_DEFAULT, 0, GUI_NAV_BAR_HEIGHT);
        lv_obj_add_flag(contentZone, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_radius(contentZone, 0, 0);
        pageWidget->contentZone = contentZone;
    }
}

void DestroyPageContentZone(PageWidget_t *pageWidget)
{
    if (pageWidget != NULL && pageWidget->contentZone != NULL && lv_obj_is_valid(pageWidget->contentZone)) {
        lv_obj_del(pageWidget->contentZone);
        pageWidget->contentZone = NULL;
    }
}

void DestroyPageWidget(PageWidget_t *pageWidget)
{
    if (pageWidget != NULL) {
        DestoryNavBarWidget(pageWidget->navBarWidget);
        DestroyPageContentZone(pageWidget);
        if (pageWidget->page != NULL && lv_obj_is_valid(pageWidget->page)) {
            lv_obj_del(pageWidget->page);
            pageWidget->page = NULL;
        }

        SRAM_FREE(pageWidget);
    }
}

void DestroyPageWidgetHandler(lv_event_t *e)
{
        DestroyPageWidget(lv_event_get_user_data(e));
}