#include "gui_attention_hintbox.h"

#define MIN_OPERATE_POWER 60

static lv_obj_t *g_attentionCont;

typedef struct {
    char *title;
    char *context;
    lv_img_dsc_t *icon;
    char *okBtnText;
    char *cancelBtnText;
    uint16_t hintboxHeight;
} AttentionHintboxContext;

static uint16_t g_confirmSign = SIG_SETUP_RSA_PRIVATE_KEY_RECEIVE_CONFIRM;

static AttentionHintboxContext *BuildConfirmationHintboxContext();
static AttentionHintboxContext *BuildLowPowerHintboxContext();
static void CloseAttentionHandler(lv_event_t *e);
static void ConfirmAttentionHandler(lv_event_t *e);

static AttentionHintboxContext *BuildConfirmationHintboxContext()
{
    AttentionHintboxContext *context = SRAM_MALLOC(sizeof(AttentionHintboxContext));
    context->title =  _("rsa_confirm_hintbox_title");
    context->context = _("rsa_confirm_hintbox_context");
    context->icon = &imgAttentionLock;
    context->hintboxHeight = 476;
    context->okBtnText = _("rsa_confirm_hintbox_ok");
    context->cancelBtnText = _("rsa_confirm_hintbox_cancel");
    return context;
}

static AttentionHintboxContext *BuildLowPowerHintboxContext()
{
    AttentionHintboxContext *context = SRAM_MALLOC(sizeof(AttentionHintboxContext));
    context->icon = &imgWarn;
    context->title = _("power_requirements_hintbox_title");
    context->context = _("power_requirements_hintbox_context");
    context->hintboxHeight = 416;
    context->cancelBtnText = _("power_requirements_hintbox_cancel");
    return context;
}

static void CloseAttentionHandler(lv_event_t *e)
{
    GuiCloseAttentionHintbox();
}

static void ConfirmAttentionHandler(lv_event_t *e)
{
    GuiEmitSignal(g_confirmSign, NULL, 0);
    lv_obj_add_flag(g_attentionCont, LV_OBJ_FLAG_HIDDEN);
    GUI_DEL_OBJ(g_attentionCont);
}

static bool CheckPowerRequirements()
{
#ifdef COMPILE_SIMULATOR
    return true;
#endif
    return GetBatterPercent() >= MIN_OPERATE_POWER && GetUsbPowerState() == USB_POWER_STATE_CONNECT;
}

void GuiCloseAttentionHintbox()
{
    if (g_attentionCont) {
        lv_obj_add_flag(g_attentionCont, LV_OBJ_FLAG_HIDDEN);
        GUI_DEL_OBJ(g_attentionCont);
    }
}

void GuiCreateAttentionHintbox(uint16_t confirmSign)
{
    if (!CheckPowerRequirements()) {
        AttentionHintboxContext *context = BuildLowPowerHintboxContext();
        g_attentionCont = GuiCreateHintBox(context->hintboxHeight);
        lv_obj_t *tempObj = GuiCreateImg(g_attentionCont, context->icon);
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 432);

        tempObj = GuiCreateLittleTitleLabel(g_attentionCont, context->title);
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 528);

        tempObj = GuiCreateIllustrateLabel(g_attentionCont, context->context);
        lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 580);

        tempObj = GuiCreateTextBtn(g_attentionCont, context->cancelBtnText);
        lv_obj_set_size(tempObj, 94, 66);
        lv_obj_set_style_radius(tempObj, 24, LV_PART_MAIN);
        lv_obj_set_style_bg_color(tempObj, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_add_event_cb(tempObj, CloseAttentionHandler, LV_EVENT_CLICKED, NULL);

        SRAM_FREE(context);
        return;
    }
    g_confirmSign = confirmSign;
    AttentionHintboxContext *context = BuildConfirmationHintboxContext();
    g_attentionCont = GuiCreateHintBox(context->hintboxHeight);
    lv_obj_t *tempObj = GuiCreateImg(g_attentionCont, context->icon);
    lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 372);

    tempObj = GuiCreateLittleTitleLabel(g_attentionCont, context->title);
    lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 468);

    tempObj = GuiCreateIllustrateLabel(g_attentionCont, context->context);
    lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 520);

    tempObj = GuiCreateTextBtn(g_attentionCont, context->okBtnText);
    lv_obj_set_size(tempObj, 192, 66);
    lv_obj_set_style_radius(tempObj, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(tempObj, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(tempObj, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(tempObj, ConfirmAttentionHandler, LV_EVENT_CLICKED, NULL);

    tempObj = GuiCreateTextBtn(g_attentionCont, context->cancelBtnText);
    lv_obj_set_size(tempObj, 192, 66);
    lv_obj_set_style_radius(tempObj, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(tempObj, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_align(tempObj, LV_ALIGN_BOTTOM_LEFT, 36, -24);
    lv_obj_add_event_cb(tempObj, CloseAttentionHandler, LV_EVENT_CLICKED, NULL);

    SRAM_FREE(context);
}