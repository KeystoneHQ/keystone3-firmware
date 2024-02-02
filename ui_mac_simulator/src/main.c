#include "lvgl.h"
#include "app_hal.h"
#include "gui.h"
#include "gui_framework.h"
#include "gui_views.h"
#include "librust_c.h"
#include "log_print.h"

#define memset_s memset

void printInfo()
{
    uint8_t seed[32] = {0};
    SimpleResponse_u8 *result = get_master_fingerprint(seed, 32);
    PrintArray("mfp", result->data, 4);
}

int main(void)
{

    lv_init();

    printInfo();

    hal_setup();

    GuiFrameOpenView(&g_initView);

    hal_loop();
}
