/* INCLUDES */
#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_enter_passcode.h"
#include "gui_model.h"
#include "gui_setting_widgets.h"
#include "gui_transaction_detail_widgets.h"
#include "gui_lock_widgets.h"
#include "gui_qr_hintbox.h"
#include "user_memory.h"
#include "secret_cache.h"
#include "keystore.h"
#include "presetting.h"
#include "assert.h"
#include "motor_manager.h"
#include "fingerprint_process.h"
#include "account_manager.h"
#include "screen_manager.h"
#ifndef COMPILE_SIMULATOR
#include "sha256.h"
#include "keystore.h"
#else
#include "simulator_model.h"
#define FP_SUCCESS_CODE                                                             (0)
#define RECOGNIZE_UNLOCK                                                            (0)
#define RECOGNIZE_OPEN_SIGN                                                         (1)
#define RECOGNIZE_SIGN                                                              (2)
#define NO_ENCRYPTION                                                               (0)
#define AES_KEY_ENCRYPTION                                                          (1)
#define RESET_AES_KEY_ENCRYPTION                                                    (2)
#define FINGERPRINT_EN_SING_ERR_TIMES                                               (5)
#endif

/* DEFINES */

/* TYPEDEFS */

/* FUNC DECLARATION*/
static void RecognizeSuccussHandler(lv_timer_t *timer);
static void RecognizeFailHandler(lv_timer_t *timer);
static void FingerButtonHandler(lv_event_t *e);
static void FingerUnlockDeviceHandler(lv_event_t *e);
static void FingerDeleteDialogsHandler(lv_event_t *e);
static void FingerDeleteHandler(lv_event_t *e);
static void FingerCancelDeleteHandler(lv_event_t *e);
static void FirstAddFingerToManagerView(lv_event_t *e);
static void AddFingerToManagerView(lv_event_t *e);
static void ClearFingerErrorStateView(lv_event_t *e);

/* STATIC VARIABLES */
static lv_obj_t *g_fpDeleteCont = NULL;
static lv_obj_t *g_fpAddCont = NULL;
static lv_obj_t *g_imgFinger = NULL;
static lv_obj_t *g_arcProgress = NULL;
static lv_obj_t *g_fpRegLabel = NULL;
static lv_obj_t *g_imgSignFinger = NULL;
static lv_obj_t *g_verifyFingerCont = NULL;
static lv_obj_t *g_fpSingerSwitch = NULL;
static lv_obj_t *g_fingerManagerContainer = NULL;
static lv_obj_t *g_labelSignFailed = NULL;
static lv_obj_t *g_fpUnlockSwitch = NULL;
static lv_timer_t *g_fpRecognizeTimer = NULL;
static bool g_firstAddFingerFlag = false;
static uint8_t g_fpEnSignCnt = 0;
static uint8_t g_deleteFingerIndex = 0;

/* FUNC */
uint8_t GuiGetFingerSettingIndex(void)
{
    if (GetRegisteredFingerNum() == 0) {
        g_firstAddFingerFlag = true;
        return DEVICE_SETTING_FINGER_ADD_ENTER;
    } else {
        return DEVICE_SETTING_FINGER_MANAGER;
    }
}

void CancelVerifyFingerHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        ClearSecretCache();
        FpCancelCurOperate();
        GUI_DEL_OBJ(g_verifyFingerCont)
    }
}

void CancelCurFingerHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        FpCancelCurOperate();
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    }
}

void GuiFingerManagerDestruct(void *obj, void *param)
{
    if (g_fpRecognizeTimer != NULL) {
        lv_timer_del(g_fpRecognizeTimer);
        g_fpRecognizeTimer = NULL;
    }
    ClearSecretCache();
}

void GuiFpVerifyDestruct(void)
{
    GUI_DEL_OBJ(g_verifyFingerCont)
    GUI_DEL_OBJ(g_fpDeleteCont)
}

void GuiWalletFingerOpenSign(void)
{
    g_verifyFingerCont = GuiCreateHintBox(428);
    lv_obj_t *cont = g_verifyFingerCont;
    lv_obj_t *label = GuiCreateIllustrateLabel(cont, _("scan_qr_code_sign_fingerprint_verify_fingerprint"));
    lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 402);

    lv_obj_t *img = GuiCreateImg(cont, &imgClose);
    GuiButton_t table[] = {
        {
            .obj = img,
            .align = LV_ALIGN_DEFAULT,
            .position = {14, 14},
        }
    };
    lv_obj_t *button = GuiCreateButton(cont, 64, 64, table, 1, CancelVerifyFingerHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 384, 394);

    g_imgSignFinger = GuiCreateImg(cont, &imgYellowFinger);
    lv_obj_align(g_imgSignFinger, LV_ALIGN_BOTTOM_MID, 0, -178);

    g_labelSignFailed = GuiCreateIllustrateLabel(cont, _("scan_qr_code_sign_fingerprint_verify_fingerprint_failed"));
    lv_obj_set_style_text_color(g_labelSignFailed, RED_COLOR, LV_PART_MAIN);
    lv_obj_add_flag(g_labelSignFailed, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(g_labelSignFailed, LV_ALIGN_TOP_MID, 0, 670);
    lv_obj_set_style_text_align(g_labelSignFailed, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_t *arc = GuiCreateArc(cont);
    lv_obj_align(arc, LV_ALIGN_BOTTOM_MID, 0, -154);
    FpRecognize(RECOGNIZE_OPEN_SIGN);
}

void FingerSignHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        bool en = lv_obj_has_state(g_fpSingerSwitch, LV_STATE_CHECKED);
        if (en) {
            lv_obj_clear_state(g_fpSingerSwitch, LV_STATE_CHECKED);
            UpdateFingerSignFlag(GetCurrentAccountIndex(), !en);
            SetFingerManagerInfoToSE();
            lv_event_send(g_fpSingerSwitch, LV_EVENT_VALUE_CHANGED, NULL);
        } else {
            GuiShowKeyboardHandler(e);
        }
    }
}

void GuiSettingDealFingerRecognize(void *param)
{
    if (g_verifyFingerCont == NULL) {
        return;
    }
    uint8_t errCode = *(uint8_t *)param;
    if (errCode == FP_SUCCESS_CODE) {
        g_fpEnSignCnt = 0;
        GuiClearQrcodeSignCnt();
        lv_img_set_src(g_imgSignFinger, &imgGreenFinger);
        lv_obj_add_flag(g_labelSignFailed, LV_OBJ_FLAG_HIDDEN);
        UpdateFingerSignFlag(GetCurrentAccountIndex(), true);
        g_fpRecognizeTimer = lv_timer_create(RecognizeSuccussHandler, 500, NULL);
    } else {
        g_fpEnSignCnt++;
        lv_obj_clear_flag(g_labelSignFailed, LV_OBJ_FLAG_HIDDEN);
        lv_img_set_src(g_imgSignFinger, &imgRedFinger);
        if (g_fpEnSignCnt >= FINGERPRINT_EN_SING_ERR_TIMES) {
            GUI_DEL_OBJ(g_verifyFingerCont)
            ClearSecretCache();
            GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
        } else {
            g_fpRecognizeTimer = lv_timer_create(RecognizeFailHandler, 1000, NULL);
            FpRecognize(RECOGNIZE_OPEN_SIGN);
        }
    }
}

void GuiWalletFingerAddFpWidget(lv_obj_t *parent, bool success)
{
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    lv_obj_t *img = NULL;
    lv_obj_t *label = NULL;
    lv_obj_t *btn = NULL;
    lv_obj_t *desc = NULL;

    if (success) {
        img = GuiCreateImg(parent, &imgSuccess);
        label = GuiCreateLittleTitleLabel(parent, _("fingerprint_add_success"));
        btn = GuiCreateTextBtn(parent, _("Done"));
    } else {
        img = GuiCreateImg(parent, &imgFailed);
        label = GuiCreateLittleTitleLabel(parent, _("fingerprint_add_failed"));
        btn = GuiCreateTextBtn(parent, _("try_again"));
        desc = GuiCreateNoticeLabel(parent, _("fingerprint_add_failed_use_another"));
        lv_obj_set_style_text_align(desc, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    }
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 180);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 284);
    if (desc != NULL) {
        lv_obj_align_to(desc, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);
    }
    lv_obj_set_size(btn, 408, 66);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_style_bg_color(btn, ORANGE_COLOR, LV_PART_MAIN);
    if (success) {
        if (g_firstAddFingerFlag == true) {
            g_firstAddFingerFlag = false;
            lv_obj_add_event_cb(btn, FirstAddFingerToManagerView, LV_EVENT_CLICKED, NULL);
        } else {
            lv_obj_add_event_cb(btn, AddFingerToManagerView, LV_EVENT_CLICKED, NULL);
        }
    } else {
        lv_obj_add_event_cb(btn, ClearFingerErrorStateView, LV_EVENT_CLICKED, NULL);
    }
}

void GuiFingerAddDestruct(void *obj, void *param)
{
    g_fpAddCont = NULL;
    GUI_DEL_OBJ(g_imgFinger)
    GUI_DEL_OBJ(g_arcProgress)
    GUI_DEL_OBJ(g_fpRegLabel)
}

void GuiSettingFingerRegisterSuccess(void *param)
{
    if (g_fpAddCont == NULL) {
        return;
    }
    uint8_t step = *(uint8_t *)param;
    lv_arc_set_value(g_arcProgress, step);
    lv_img_set_src(g_imgFinger, &imgYellowFinger);
    lv_obj_set_style_arc_color(g_arcProgress, ORANGE_COLOR, LV_PART_INDICATOR);
    lv_label_set_text(g_fpRegLabel, "");
    if (step == 8) {
        SetPageLockScreen(false);
        static uint16_t signal = SIG_FINGER_REGISTER_ADD_SUCCESS;
        GuiShowKeyboard(&signal, true, StopAddNewFingerHandler);
    }
}

void GuiSettingFingerRegisterFail(void *param)
{
    if (g_fpAddCont == NULL) {
        return;
    }
    if (param == NULL) {
        uint8_t walletIndex = DEVICE_SETTING_FINGER_ADD_OUT_LIMIT;
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &walletIndex, sizeof(walletIndex));
    } else {
        uint8_t errCode = *(uint8_t *)param;
        const char *text = NULL;
        lv_img_set_src(g_imgFinger, &imgRedFinger);
        printf("errCode = %#x\n", errCode);
        lv_obj_set_style_arc_color(g_arcProgress, RED_COLOR, LV_PART_INDICATOR);
        if (errCode == 0x93) { // finger registered
            text = _("fingerprint_add_failed_duplicate");
        } else {
            text = (char *)GetFpErrorMessage(errCode);
        }
        lv_label_set_text(g_fpRegLabel, text);
    }
}

void GuiWalletFingerOpenUnlock(void)
{
    lv_obj_add_state(g_fpUnlockSwitch, LV_STATE_CHECKED);
    UpdateFingerUnlockFlag(true);
    lv_event_send(g_fpUnlockSwitch, LV_EVENT_VALUE_CHANGED, NULL);
}

void GuiWalletFingerAddWidget(lv_obj_t *parent)
{
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    g_fpAddCont = parent;
    lv_obj_t *label = NULL;
    label = GuiCreateTitleLabel(parent, _("fingerprint_add"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 12);

    label = GuiCreateNoticeLabel(parent, _("fingerprint_add_desc"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 72);

    g_imgFinger = GuiCreateImg(parent, &imgWhiteFinger);
    lv_obj_align(g_imgFinger, LV_ALIGN_BOTTOM_MID, 0, -264);

    g_arcProgress = GuiCreateArc(parent);
    lv_arc_set_range(g_arcProgress, 0, 8);
    lv_obj_align(g_arcProgress, LV_ALIGN_BOTTOM_MID, 0, -240);

    g_fpRegLabel = GuiCreateIllustrateLabel(parent, "");
    lv_obj_set_style_text_color(g_fpRegLabel, RED_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_align(g_fpRegLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(g_fpRegLabel, LV_ALIGN_BOTTOM_MID, 0, -156);

    for (int i = 0; i < 3; i++) {
        if (GetFingerRegisteredStatus(i) == 0) {
            RegisterFp(i);
            break;
        }
    }
#ifdef COMPILE_SIMULATOR
    label = GuiCreateTextLabel(parent, "Register complete");
    GuiButton_t table[1] = {
        {
            .obj = label,
            .align = LV_ALIGN_BOTTOM_MID,
            .position = {0, -15},
        }
    };
    static uint32_t walletSetting = DEVICE_SETTING_FINGER_ADD_SUCCESS;
    lv_obj_t *button = NULL;
    button = GuiCreateButton(parent, 408, 66, table, 1, WalletSettingHandler, &walletSetting);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -24);
#endif
}

void GuiWalletFingerManagerWidget(lv_obj_t *parent)
{
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    lv_obj_t *container = GuiCreateContainerWithParent(parent, 480, 656);
    lv_obj_set_align(container, LV_ALIGN_BOTTOM_MID);
    g_fingerManagerContainer = container;
    GuiFingerMangerStructureCb(NULL, NULL);
}

void GuiFingerMangerStructureCb(void *obj, void *param)
{
    lv_obj_t *label = NULL;
    lv_obj_t *imgArrow = NULL;
    lv_obj_t *button = NULL;
    lv_obj_t *container = g_fingerManagerContainer;
    uint32_t curPositionY = 0;
    g_fpEnSignCnt = 0;

    static uint32_t g_curDealFingerIndex[3] = {0, 1, 2};
    static uint32_t walletSetting[3] = {
        DEVICE_SETTING_FINGER_ADD_ENTER,
        DEVICE_SETTING_FINGER_DELETE,
        DEVICE_SETTING_FINGER_SET_PATTERN
    };
    static uint16_t walletUnlock = SIG_FINGER_SET_UNLOCK;
    static uint16_t walletSign = SIG_FINGER_SET_SIGN_TRANSITIONS;

    if (lv_obj_get_child_cnt(container) > 0) {
        lv_obj_clean(container);
    }
    label = GuiCreateTextLabel(container, _("fingerprint_unlock_device"));
    g_fpUnlockSwitch = GuiCreateSwitch(container);

    if (GetFingerUnlockFlag() == 1) {
        lv_obj_add_state(g_fpUnlockSwitch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(g_fpUnlockSwitch, LV_STATE_CHECKED);
    }
    lv_obj_clear_flag(g_fpUnlockSwitch, LV_OBJ_FLAG_CLICKABLE);

    GuiButton_t table[] = {
        {
            .obj = label,
            .align = LV_ALIGN_LEFT_MID,
            .position = {24, 0},
        },
        {
            .obj = g_fpUnlockSwitch,
            .align = LV_ALIGN_LEFT_MID,
            .position = {376, 0},
        },
    };
    button = GuiCreateButton(container, 456, 84, table, NUMBER_OF_ARRAYS(table), FingerUnlockDeviceHandler, &walletUnlock);
    if (GetRegisteredFingerNum() == 0) {
        lv_obj_clear_flag(button, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(button, LV_OPA_10, LV_PART_MAIN);
    }
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, curPositionY);
    curPositionY += 96;

    label = GuiCreateTextLabel(container, _("fingerprint_sign_tx"));
    g_fpSingerSwitch = GuiCreateSwitch(container);
    if (GetFingerSignFlag() == 1) {
        lv_obj_add_state(g_fpSingerSwitch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(g_fpSingerSwitch, LV_STATE_CHECKED);
    }
    lv_obj_clear_flag(g_fpSingerSwitch, LV_OBJ_FLAG_CLICKABLE);
    table[0].obj = label;
    table[1].obj = g_fpSingerSwitch;
    button = GuiCreateButton(container, 456, 84, table, NUMBER_OF_ARRAYS(table), FingerSignHandler, &walletSign);
    if (GetRegisteredFingerNum() == 0) {
        lv_obj_clear_flag(button, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(button, LV_OPA_10, LV_PART_MAIN);
    }
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, curPositionY);
    curPositionY += 96;

    // bool en = lv_obj_has_state(g_fpSingerSwitch, LV_STATE_CHECKED);
    // if (en) {
    //     label = GuiCreateTextLabel(container, "Pattern Verification");
    //     imgArrow = GuiCreateImg(container, &imgArrowRight);
    //     table[0].obj = label;
    //     table[1].obj = imgArrow;
    //     table[1].position.x = 396;
    //     button = GuiCreateButton(container, 456, 84, table, 2, WalletSettingHandler, &walletSetting[2]);
    //     lv_obj_align(button, LV_ALIGN_DEFAULT, 12, curPositionY);
    //     curPositionY += 84;
    //     curPositionY += 12;
    // }

    lv_obj_t *line = GuiCreateDividerLine(container);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, curPositionY);
    curPositionY += 12;

    uint8_t fpRegisteredNum = 0;
    uint8_t textId = 0;
    for (int i = 0; i < 3; i++) {
        textId = GetFingerRegisteredStatus(i);
        if (textId != 0) {
            printf("text id = %d\n", textId);
            fpRegisteredNum++;
            label = GuiCreateTextLabel(container, "");
            lv_label_set_text_fmt(label, _("fingerprint_nth"), textId);
            imgArrow = GuiCreateImg(container, &imgArrowRight);

            table[0].obj = label;
            table[1].obj = imgArrow;
            table[1].position.x = 396;

            button = GuiCreateButton(container, 456, 84, table, 2, FingerButtonHandler, &g_curDealFingerIndex[i]);
            lv_obj_align(button, LV_ALIGN_DEFAULT, 12, curPositionY);
            curPositionY += 96;
        }
    }
    if (fpRegisteredNum < 3) {
        label = GuiCreateTextLabel(container, _("fingerprint_add_btn"));
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
        table[0].obj = label;
        button = GuiCreateButton(container, 456, 84, table, 1, WalletSettingHandler, &walletSetting[0]);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 12, curPositionY);
    } else {
        label = GuiCreateNoticeLabel(container, _("fingerprint_up_to_3"));
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, curPositionY);
    }
}

void GuiWalletFingerDeleteWidget(lv_obj_t *parent)
{
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    lv_obj_t *imgFinger = GuiCreateImg(parent, &imgYellowFinger);
    lv_obj_align(imgFinger, LV_ALIGN_BOTTOM_MID, 0, -548);

    char buf[BUFFER_SIZE_32] = {0};
    snprintf_s(buf, BUFFER_SIZE_32, _("fingerprint_nth"), GetFingerRegisteredStatus(g_deleteFingerIndex));
    lv_obj_t *label = GuiCreateLittleTitleLabel(parent, buf);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -476);

    label = GuiCreateTextLabel(parent, _("fingerprint_remove"));
    lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
    GuiButton_t table[] = {
        {
            .obj = label,
            .align = LV_ALIGN_BOTTOM_MID,
            .position = {0, -15},
        }
    };
    lv_obj_t *button = GuiCreateButton(parent, 282, 66, table, NUMBER_OF_ARRAYS(table), FingerDeleteDialogsHandler, &g_deleteFingerIndex);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -64);
}

void GuiWalletSetFingerPassCodeWidget(lv_obj_t *parent)
{
    static uint32_t walletSetting[2] = {
        SIG_FINGER_FINGER_SETTING,
        SIG_SETTING_CHANGE_PASSWORD
    };
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    lv_obj_t *label = GuiCreateTextLabel(parent, _("fingerprint_passcode_fingerprint_setting"));
    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRight);

    GuiButton_t table[2] = {
        {
            .obj = label,
            .align = LV_ALIGN_DEFAULT,
            .position = {24, 22},
        },
        {
            .obj = imgArrow,
            .align = LV_ALIGN_DEFAULT,
            .position = {411, 32},
        },
    };
    lv_obj_t *button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table),
                                       GuiShowKeyboardHandler, &walletSetting[0]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 144 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 240 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateTextLabel(parent, _("fingerprint_passcode_reset_passcode"));
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    table[0].obj = label;
    table[1].obj = imgArrow;
    button = GuiCreateButton(parent, 456, 84, table, 2, GuiShowKeyboardHandler, &walletSetting[1]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 253 - GUI_MAIN_AREA_OFFSET);
}

/* STATIC FUNC */
static void ClearFingerErrorStateView(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
        lv_arc_set_value(g_arcProgress, 0);
        lv_img_set_src(g_imgFinger, &imgWhiteFinger);
        lv_obj_set_style_arc_color(g_arcProgress, WHITE_COLOR, LV_PART_INDICATOR);
        lv_label_set_text(g_fpRegLabel, "");
        for (int i = 0; i < 3; i++) {
            if (GetFingerRegisteredStatus(i) == 0) {
                RegisterFp(i);
                break;
            }
        }
    }
}

static void FirstAddFingerToManagerView(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiSettingCloseToTargetTileView(2);
        uint8_t walletIndex = DEVICE_SETTING_FINGER_MANAGER;
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &walletIndex, sizeof(walletIndex));
    }
}

static void AddFingerToManagerView(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiSettingCloseToTargetTileView(3);
    }
}

static void FingerButtonHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        uint8_t *fingerIndex = lv_event_get_user_data(e);
        g_deleteFingerIndex = *fingerIndex;
        uint8_t walletIndex = DEVICE_SETTING_FINGER_DELETE;
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &walletIndex, sizeof(walletIndex));
    }
}

static void RecognizeSuccussHandler(lv_timer_t *timer)
{
    GUI_DEL_OBJ(g_verifyFingerCont)
    lv_obj_add_state(g_fpSingerSwitch, LV_STATE_CHECKED);
    lv_event_send(g_fpSingerSwitch, LV_EVENT_VALUE_CHANGED, NULL);
    lv_timer_del(g_fpRecognizeTimer);
    g_fpRecognizeTimer = NULL;
}

static void RecognizeFailHandler(lv_timer_t *timer)
{
    if (g_verifyFingerCont != NULL) {
        lv_img_set_src(g_imgSignFinger, &imgYellowFinger);
        lv_obj_add_flag(g_labelSignFailed, LV_OBJ_FLAG_HIDDEN);
    }
    lv_timer_del(timer);
    g_fpRecognizeTimer = NULL;
}

static void FingerUnlockDeviceHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        bool en = lv_obj_has_state(g_fpUnlockSwitch, LV_STATE_CHECKED);
        if (en) {
            lv_obj_clear_state(g_fpUnlockSwitch, LV_STATE_CHECKED);
            UpdateFingerUnlockFlag(!en);
            lv_event_send(g_fpUnlockSwitch, LV_EVENT_VALUE_CHANGED, NULL);
        } else {
            GuiShowKeyboardHandler(e);
        }
    }
}

static void FingerDeleteDialogsHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        uint8_t *fingerIndex = lv_event_get_user_data(e);
        char buf[50] = {0};
        lv_snprintf(buf, sizeof(buf), _("fingerprint_nth_remove_desc"), GetFingerRegisteredStatus(*fingerIndex));
        g_fpDeleteCont = GuiCreateResultHintbox(386, &imgWarn, _("fingerprint_nth_remove_title"),
                                                buf, _("Cancel"), DARK_GRAY_COLOR, _("fingerprint_remove_confirm"), RED_COLOR);
        lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_fpDeleteCont);
        lv_obj_add_event_cb(leftBtn, FingerCancelDeleteHandler, LV_EVENT_CLICKED, g_fpDeleteCont);

        lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_fpDeleteCont);
        lv_obj_add_event_cb(rightBtn, FingerDeleteHandler, LV_EVENT_CLICKED, g_fpDeleteCont);
    }
}

static void FingerDeleteHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        DeleteFp(g_deleteFingerIndex);
        GUI_DEL_OBJ(g_fpDeleteCont)
    }
}

static void FingerCancelDeleteHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_fpDeleteCont)
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    }
}