/* INCLUDES */
#include "gui_led.h"
#include "gui_obj.h"

/* DEFINES */

/* TYPEDEFS */

/* FUNC DECLARATION*/

/* STATIC VARIABLES */

/* FUNC */
void GuiSetLedStatus(lv_obj_t *led, PASSCODE_LED_STATUS_ENUM status)
{
    if (status == PASSCODE_LED_ON) {
        lv_led_set_color(led, ORANGE_COLOR);
        lv_led_on(led);
    } else if (status == PASSCODE_LED_OFF) {
        lv_led_set_color(led, WHITE_COLOR);
        lv_led_off(led);
    } else if (status == PASSCODE_LED_ERR) {
        lv_led_set_color(led, RED_COLOR);
        lv_led_on(led);
    }
}

void *GuiCreateLed(lv_obj_t *parent)
{
    lv_obj_t *led = lv_led_create(parent);
    lv_led_set_brightness(led, 150);
    lv_obj_set_style_shadow_width(led, 0, LV_STATE_DEFAULT | LV_PART_MAIN);
    GuiSetLedStatus(led, PASSCODE_LED_OFF);
    lv_obj_set_size(led, 12, 12);
    return led;
}