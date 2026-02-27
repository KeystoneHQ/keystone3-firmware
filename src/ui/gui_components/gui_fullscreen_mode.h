#ifndef _GUI_FULLSCREEN_MODE_H
#define _GUI_FULLSCREEN_MODE_H

#include "lvgl.h"
#include "gui.h"

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 800

/**
 * Initializes fullscreen mode.
 *
 * @param width The width of the fullscreen container.
 * @param height The height of the fullscreen container.
 * @param bg_color The background color of the fullscreen container.
 */
void GuiFullscreenModeInit(uint16_t width, uint16_t height, lv_color_t bg_color);

/**
 * Creates an object in fullscreen mode.
 *
 * @param create_object_func A function pointer that creates the object.
 * @param size The size of the object.
 * @return Pointer to the created object.
 */
lv_obj_t *GuiFullscreenModeCreateObject(lv_obj_t* (*create_object_func)(lv_obj_t*, uint16_t, uint16_t), uint16_t w, uint16_t h);

/**
 * Toggles between fullscreen and regular mode.
 */
void GuiFullscreenModeToggle();

/**
 * Cleans up resources used in fullscreen mode.
 */
void GuiFullscreenModeCleanUp();
lv_obj_t *GuiFullscreenModeGetCreatedObject();
lv_obj_t *GuiFullscreenModeGetCreatedObjectWhenVisible(void);
void GuiFullscreenModeHandler(lv_event_t * e);
#endif /* _GUI_FULLSCREEN_MODE_H */