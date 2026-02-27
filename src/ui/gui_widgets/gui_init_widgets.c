#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "gui_about_info_widgets.h"
#include "gui_page.h"
#include "presetting.h"
#include "version.h"
#ifndef COMPILE_SIMULATOR
#include "drv_battery.h"
#else
#include "simulator_model.h"
#endif
