#include "gui_attention_hintbox.h"
#include "device_setting.h"
#define MIN_OPERATE_POWER 80

static lv_obj_t *g_attentionCont;
static void (*g_onCancel)(void);
static bool g_isEnableBlindSigning = false;
typedef struct {
    char *title;
    char *context;
    lv_img_dsc_t *icon;
    char *okBtnText;
    char *cancelBtnText;
    uint16_t hintboxHeight;
} AttentionHintboxContext;

typedef struct {
    char *title;
    char *context;
    lv_img_dsc_t *icon;
    char *okBtnText;
    char *checkBoxText;
    uint16_t hintboxHeight;
} EnableBlindSigningHintboxContext;

static uint16_t g_confirmSign = SIG_SETUP_RSA_PRIVATE_KEY_RECEIVE_CONFIRM;

static AttentionHintboxContext *BuildConfirmationHintboxContext();
static EnableBlindSigningHintboxContext *BuildEnableBlindSigningHintboxContext();
static AttentionHintboxContext *BuildLowPowerHintboxContext();
static void CloseAttentionHandler(lv_event_t *e);
static void ConfirmAttentionHandler(lv_event_t *e);
static void EnableBlindSigningHandler(lv_event_t *e);
static uint16_t RecalculateButtonWidth(lv_obj_t *button, uint16_t minButtonWidth);

static AttentionHintboxContext *BuildConfirmationHintboxContext()
{
    AttentionHintboxContext *context = SRAM_MALLOC(sizeof(AttentionHintboxContext));
    context->title = _("ar_preparation_title");
    context->context = _("ar_preparation_address_notice");
    context->icon = &imgWarn;
    context->hintboxHeight = 476;
    context->okBtnText = _("Continue");
    context->cancelBtnText = _("not_now");
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

static AttentionHintboxContext *BuildInitializationCompleteHintboxContext()
{
    AttentionHintboxContext *context = SRAM_MALLOC(sizeof(AttentionHintboxContext));
    context->icon = &imgSuccess;
    context->title = _("initialization_complete_hintbox_title");
    context->context = _("initialization_complete_hintbox_context");
    context->hintboxHeight = 386;
    context->okBtnText = _("initialization_complete_hintbox_ok");
    return context;
}

static void EnableBlindSigningHandler(lv_event_t *e)
{
    // select state
    g_isEnableBlindSigning = !g_isEnableBlindSigning;
}

static EnableBlindSigningHintboxContext *BuildEnableBlindSigningHintboxContext()
{
    EnableBlindSigningHintboxContext *context = SRAM_MALLOC(sizeof(EnableBlindSigningHintboxContext));
    context->title = _("enable_blind_signing_hintbox_title");
    context->context = _("enable_blind_signing_hintbox_context");
    context->icon = &imgWarn;
    context->hintboxHeight = 456;
    context->okBtnText = _("OK");
    context->checkBoxText = _("enable_blind_signing_hintbox_check");
    return context;
}


static uint16_t RecalculateButtonWidth(lv_obj_t *button, uint16_t minButtonWidth)
{
    uint16_t buttonWidth = lv_obj_get_self_width(lv_obj_get_child(button, 0)) + 24;
    return buttonWidth > minButtonWidth ? buttonWidth : minButtonWidth;
}

static void CloseAttentionHandler(lv_event_t *e)
{
    GuiCloseAttentionHintbox();
}

static void CloseEnableBlindSigningHandler(lv_event_t *e)
{
    if (g_attentionCont) {
        lv_obj_add_flag(g_attentionCont, LV_OBJ_FLAG_HIDDEN);
        GUI_DEL_OBJ(g_attentionCont);
    }
    // upsert enable blind signing hintbox
    SetEnableBlindSigning(g_isEnableBlindSigning);
    // flush settings to storage
    SaveDeviceSettings();
}
static void ConfirmAttentionHandler(lv_event_t *e)
{
    g_onCancel = NULL;
    GuiEmitSignal(g_confirmSign, NULL, 0);
    GUI_DEL_OBJ(g_attentionCont);
}

static bool CheckPowerRequirements()
{
#ifdef COMPILE_SIMULATOR
    return true;
#endif
    return GetCurrentDisplayPercent() >= MIN_OPERATE_POWER && GetUsbPowerState() == USB_POWER_STATE_CONNECT;
}

void GuiCloseAttentionHintbox()
{
    void (*onCancel)(void) = g_onCancel;
    g_onCancel = NULL;
    if (g_attentionCont) {
        lv_obj_add_flag(g_attentionCont, LV_OBJ_FLAG_HIDDEN);
        GUI_DEL_OBJ(g_attentionCont);
    }
    if (onCancel != NULL) {
        onCancel();
    }
}

void GuiCreateHardwareCallInvaildParamHintbox(char *title, char *content)
{
    GuiCreateHardwareCallInvaildParamHintboxWithHandler(title, content, CloseAttentionHandler);
}

void GuiCreateHardwareCallInvaildParamHintboxWithHandler(char *title, char *content, lv_event_cb_t okHandler)
{
    AttentionHintboxContext *context = SRAM_MALLOC(sizeof(AttentionHintboxContext));
    context->icon = &imgUnknown;
    context->hintboxHeight = 416;
    context->cancelBtnText = _("power_requirements_hintbox_cancel");
    g_attentionCont = GuiCreateHintBox(context->hintboxHeight);
    lv_obj_t *tempObj = GuiCreateImg(g_attentionCont, context->icon);
    lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 462);

    tempObj = GuiCreateLittleTitleLabel(g_attentionCont, title);
    lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 558);

    tempObj = GuiCreateIllustrateLabel(g_attentionCont, content);
    lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 610);

    tempObj = GuiCreateTextBtn(g_attentionCont, context->cancelBtnText);
    lv_obj_set_size(tempObj, RecalculateButtonWidth(tempObj, 94), 66);
    lv_obj_set_style_radius(tempObj, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(tempObj, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_align(tempObj, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(tempObj, okHandler == NULL ? CloseAttentionHandler : okHandler, LV_EVENT_CLICKED, NULL);
    SRAM_FREE(context);
}


void GuiCreateEnableBlindSigningHintbox()
{
    // load enable blind signing status from storage
    g_isEnableBlindSigning = GetEnableBlindSigning();
    EnableBlindSigningHintboxContext *context = BuildEnableBlindSigningHintboxContext();
    g_attentionCont = GuiCreateHintBox(context->hintboxHeight);
    // create a container for the hintbox
    lv_obj_t *hint_box_container = GuiCreateContainerWithParent(g_attentionCont, 408, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(hint_box_container, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(hint_box_container, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(hint_box_container, LV_ALIGN_TOP_LEFT, 36, 390);
    lv_obj_t *hint_box_icon = GuiCreateImg(hint_box_container, context->icon);
    lv_obj_align(hint_box_icon, LV_ALIGN_DEFAULT, 0, 0);

    lv_obj_t *hint_box_title = GuiCreateLittleTitleLabel(hint_box_container, context->title);
    lv_obj_align_to(hint_box_title, hint_box_icon, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);

    lv_obj_t *hint_box_content = GuiCreateIllustrateLabel(hint_box_container, context->context);
    lv_obj_align_to(hint_box_content, hint_box_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);


    // draw a checkbox and notice content
    lv_obj_t *hint_box_checkBox = GuiCreateBlindSigningCheckBoxWithFont(hint_box_container, "", true, g_defIllustrateFont);
    lv_obj_align_to(hint_box_checkBox, hint_box_content, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
    lv_obj_add_event_cb(hint_box_checkBox, EnableBlindSigningHandler, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *hint_box_notice = GuiCreateIllustrateLabel(hint_box_container, context->checkBoxText);
    lv_obj_align_to(hint_box_notice, hint_box_checkBox, LV_ALIGN_OUT_RIGHT_TOP, 8, 0);
    lv_obj_set_style_text_opa(hint_box_notice, 163, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *okBtn = GuiCreateTextBtn(g_attentionCont, context->okBtnText);
    lv_obj_set_size(okBtn, RecalculateButtonWidth(okBtn, 94), 66);
    lv_obj_set_style_radius(okBtn, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(okBtn, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_align(okBtn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(okBtn, CloseEnableBlindSigningHandler, LV_EVENT_CLICKED, NULL);


    SRAM_FREE(context);
}


void GuiCreateAttentionHintbox(uint16_t confirmSign)
{
    GuiCreateAttentionHintboxWithCancel(confirmSign, NULL);
}

static lv_obj_t *CreateArPreparationHintbox(AttentionHintboxContext *context)
{
    const char *paragraphs[] = {context->context, _("ar_preparation_power_notice")};
    lv_obj_t *cont = GuiCreateHintBox(800);
    lv_obj_t *body = GuiCreateContainerWithParent(cont, 408, 662);
    lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_align(body, LV_ALIGN_TOP_LEFT, 36, 40);
    lv_obj_add_flag(body, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(body, LV_DIR_VER);
    lv_obj_t *icon = GuiCreateImg(body, context->icon);
    lv_obj_align(icon, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_update_layout(icon);
    int32_t y = lv_obj_get_height(icon) + 28;
    lv_obj_t *title = GuiCreateLittleTitleLabel(body, context->title);
    lv_obj_set_width(title, 408);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, y);
    lv_obj_update_layout(title);
    y += lv_obj_get_height(title) + 16;
    for (size_t i = 0; i < 2; i++) {
        lv_obj_t *bullet = GuiCreateIllustrateLabel(body, "·");
        lv_obj_set_width(bullet, 16);
        lv_obj_set_style_text_color(bullet, ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_align(bullet, LV_ALIGN_TOP_LEFT, 0, y);
        lv_obj_t *text = GuiCreateIllustrateLabel(body, paragraphs[i]);
        lv_obj_set_width(text, 388);
        lv_label_set_long_mode(text, LV_LABEL_LONG_WRAP);
        lv_obj_align(text, LV_ALIGN_TOP_LEFT, 20, y);
        lv_obj_update_layout(text);
        y += lv_obj_get_height(text) + 20;
    }
    uint32_t height = y + 142;
    if (height > 800) {
        height = 800;
    }
    GuiHintBoxResize(cont, height);
    lv_obj_set_height(body, height - 138);
    lv_obj_align(body, LV_ALIGN_TOP_LEFT, 36, 800 - height + 40);
    lv_obj_t *leftBtn = GuiCreateTextBtn(cont, context->cancelBtnText);
    lv_obj_set_size(leftBtn, 192, 66);
    lv_obj_set_style_bg_color(leftBtn, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_align(leftBtn, LV_ALIGN_BOTTOM_LEFT, 36, -24);
    lv_obj_add_event_cb(leftBtn, CloseAttentionHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *rightBtn = GuiCreateTextBtn(cont, context->okBtnText);
    lv_obj_set_size(rightBtn, 192, 66);
    lv_obj_set_style_bg_color(rightBtn, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(rightBtn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(rightBtn, ConfirmAttentionHandler, LV_EVENT_CLICKED, NULL);
    return cont;
}

void GuiCreateAttentionHintboxWithCancel(uint16_t confirmSign, void (*onCancel)(void))
{
    GuiCloseAttentionHintbox();
    g_onCancel = onCancel;
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
        lv_obj_set_size(tempObj, RecalculateButtonWidth(tempObj, 94), 66);
        lv_obj_set_style_radius(tempObj, 24, LV_PART_MAIN);
        lv_obj_set_style_bg_color(tempObj, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_add_event_cb(tempObj, CloseAttentionHandler, LV_EVENT_CLICKED, NULL);

        SRAM_FREE(context);
        return;
    }
    g_confirmSign = confirmSign;
    AttentionHintboxContext *context = BuildConfirmationHintboxContext();
    g_attentionCont = CreateArPreparationHintbox(context);

    SRAM_FREE(context);
}

void GuiCreateInitializatioCompleteHintbox()
{
    AttentionHintboxContext *context = BuildInitializationCompleteHintboxContext();
    g_attentionCont = GuiCreateHintBox(context->hintboxHeight);
    lv_obj_t *tempObj = GuiCreateImg(g_attentionCont, context->icon);
    lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 462);

    tempObj = GuiCreateLittleTitleLabel(g_attentionCont, context->title);
    lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 558);

    tempObj = GuiCreateIllustrateLabel(g_attentionCont, context->context);
    lv_obj_align(tempObj, LV_ALIGN_TOP_LEFT, 36, 610);

    tempObj = GuiCreateTextBtn(g_attentionCont, context->okBtnText);
    lv_obj_set_size(tempObj, RecalculateButtonWidth(tempObj, 136), 66);
    lv_obj_set_style_radius(tempObj, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(tempObj, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_align(tempObj, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(tempObj, CloseAttentionHandler, LV_EVENT_CLICKED, NULL);

    SRAM_FREE(context);
}
