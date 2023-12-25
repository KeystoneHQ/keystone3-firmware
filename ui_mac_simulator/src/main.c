#include "lvgl.h"
#include "app_hal.h"
#include "gui.h"
#include "gui_framework.h"
#include "gui_views.h"

void printInfo()
{
    lv_fs_dir_t dir;
    
    lv_fs_dir_open(&dir, "C:/");
    char fn[128];
    lv_fs_dir_read(&dir, fn);
    printf("fn: %s\r\n", fn);

    lv_fs_dir_read(&dir, fn);
    printf("fn: %s\r\n", fn);

    lv_fs_dir_read(&dir, fn);
    printf("fn: %s\r\n", fn);

    lv_fs_dir_read(&dir, fn);
    printf("fn: %s\r\n", fn);
}

int main(void)
{

    lv_init();

    //printInfo();

    hal_setup();

    GuiFrameOpenView(&g_initView);

    hal_loop();
}
