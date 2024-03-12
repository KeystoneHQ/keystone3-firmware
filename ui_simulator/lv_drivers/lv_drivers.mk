LV_DRIVERS_DIR_NAME ?= lv_drivers

CSRCS += $(wildcard $(LVGL_DIR)/$(LV_DRIVERS_DIR_NAME)/*.c)
CSRCS += $(wildcard $(LVGL_DIR)/$(LV_DRIVERS_DIR_NAME)/indev/*.c)

