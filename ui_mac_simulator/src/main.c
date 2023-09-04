#include "lvgl.h"
#include "app_hal.h"
#include "gui.h"
#include "gui_framework.h"
#include "gui_views.h"

int main(void)
{
	lv_init();

	hal_setup();

	GuiFrameOpenView(&g_initView);

	hal_loop();
}
