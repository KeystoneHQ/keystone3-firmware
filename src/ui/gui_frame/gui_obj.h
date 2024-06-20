#ifndef _GUI_OBJ_H
#define _GUI_OBJ_H

#include "gui.h"

typedef int32_t(*GuiEventProcessFunc)(void *self, uint16_t usEvent, void *param, uint16_t usLen);

#define ITEM_ENUM(x) x,
#define ITEM_STR(x) #x,

#define SCREEN_LIST(add)        \
    add(SCREEN_INIT)            \
    add(SCREEN_LOCK)            \
    add(SCREEN_HOME)            \
    add(SCREEN_SETUP)           \
    add(CREATE_WALLET)          \
    add(CREATE_SHARE)           \
    add(IMPORT_SHARE)           \
    add(SINGLE_PHRASE)          \
    add(IMPORT_SINGLE_PHRASE)   \
    add(CONNECT_WALLET)         \
    add(SCREEN_SETTING)         \
    add(SCREEN_QRCODE)          \
    add(SCREEN_PASSPHRASE)      \
    add(SCREEN_BITCOIN_RECEIVE) \
    add(SCREEN_ETHEREUM_RECEIVE)\
    add(SCREEN_STANDARD_RECEIVE)\
    add(SCREEN_EXPORT_PUBKEY)   \
    add(SCREEN_FORGET_PASSCODE) \
    add(SCREEN_LOCK_DEVICE)     \
    add(SCREEN_FIRMWARE_UPDATE) \
    add(SCREEN_WEB_AUTH)        \
    add(SCREEN_PURPOSE)         \
    add(SCREEN_SYSTEM_SETTING)  \
    add(SCREEN_WEB_AUTH_RESULT) \
    add(SCREEN_ABOUT)           \
    add(SCREEN_ABOUT_KEYSTONE)  \
    add(SCREEN_ABOUT_TERMS)     \
    add(SCREEN_ABOUT_INFO)      \
    add(SCREEN_WIPE_DEVICE)     \
    add(SCREEN_WALLET_TUTORIAL) \
    add(SCREEN_SELF_DESTRUCT)   \
    add(SCREEN_INACTIVE)        \
    add(SCREEN_DISPLAY)         \
    add(SCREEN_TUTORIAL)        \
    add(SCREEN_CONNECTION)      \
    add(SCREEN_MULTI_ACCOUNTS_RECEIVE)          \
    add(SCREEN_KEY_DERIVATION_REQUEST)          \
    add(SCREEN_SCAN)                            \
    add(SCREEN_TRANSACTION_DETAIL)              \
    add(SCREEN_TRANSACTION_SIGNATURE)           \
    add(SCREEN_USB_TRANSPORT)                   \
    add(SCREEN_DICE_ROLLS)                      \
    add(SCREEN_DEVICE_PUB_KEY)                  \
    add(SCREEN_DEVICE_UPDATE_SUCCESS)           \
    add(SCREEN_BTC_WALLET_PROFILE)              \
    add(SCREEN_MULTI_SIG_IMPORT_WALLET_INFO)    \
    add(SCREEN_MULTISIG_WALLET_EXPORT) \
    add(SCREEN_CREATE_MULTI)                    \
    add(SCREEN_MANAGE_MULTI_SIG)                \
    add(SCREEN_MULTISIG_SELECT_IMPORT_METHOD)   \
    add(SCREEN_MULTISIG_READ_SDCARD)            \
    add(SCREEN_TON_MNEMONIC_HINT)               \
    add(SCREEN_MULTISIG_TRANSACTION_SIGNATURE)

typedef enum {
    SCREEN_INVALID = -1,
    SCREEN_LIST(ITEM_ENUM)
    SCREEN_TOTAL,
} SCREEN_ID_ENUM;

typedef struct GUI_VIEW_ {
    SCREEN_ID_ENUM id;
    bool isActive;
    bool optimization;
    GuiEventProcessFunc pEvtHandler;
    struct GUI_VIEW_ *previous;
} GUI_VIEW;

void *GuiCreateContainerWithParent(lv_obj_t *parent, int w, int h);
void *GuiCreateWhiteOpa12Container(lv_obj_t *parent, int w, int h);
void *GuiCreateLabelWithFont(lv_obj_t *parent, const char *text, const lv_font_t *font);
void *GuiCreateLabelWithFontScroll(lv_obj_t *parent, const char *text, const lv_font_t *font, uint16_t width);
void *GuiCreateLabelWithFontAndTextColor(lv_obj_t *parent, const char *text, const lv_font_t *font, int color);
void *GuiCreateNoticeLabel(lv_obj_t *parent, const char *text);
void *GuiCreateImg(lv_obj_t *parent, const void *src);
void *GuiCreateScaleImg(lv_obj_t *parent, const void *src, int scale);
void *GuiCreateCheckBoxWithFont(lv_obj_t *parent, const char *text, bool single, const lv_font_t *font);
void *GuiCreateBtnWithFont(lv_obj_t *parent, const char *text, const lv_font_t *font);
void *GuiCreateLine(lv_obj_t *parent, lv_point_t linePoints[], uint16_t pointNum);
void *GuiCreateDividerLine(lv_obj_t *parent);
void *GuiCreateCircleAroundAnimation(lv_obj_t *parent, int w);
void GuiStopCircleAroundAnimation(void);
void GuiSetAngle(void* img, int32_t v);
void *GuiCreateConfirmSlider(lv_obj_t *parent, lv_event_cb_t cb);
void *GuiCreateStatusCoinButton(lv_obj_t *parent, const char *text, const void *src);
void *GuiCreateTileView(lv_obj_t *parent);
void *GuiCreateAnimView(lv_obj_t *parent, uint16_t animHeight);
void *GuiCreateArc(lv_obj_t *parent);
void *GuiCreateSwitch(lv_obj_t *parent);
void *GuiCreateBtn(lv_obj_t *parent, const char *text);
void *GuiCreateTextBtn(lv_obj_t *parent, const char *text);
void *GuiCreateAdaptButton(lv_obj_t *parent, const char *text);
void GuiAlignToPrevObj(lv_obj_t *obj, lv_align_t align, int16_t x, int16_t y);
void GuiAddObjFlag(void *obj, lv_obj_flag_t flag);
void GuiClearObjFlag(void *obj, lv_obj_flag_t flag);
void *GuiCreateSpacer(void *parent, uint16_t height);
void *GuiCreateSelectPathCheckBox(lv_obj_t *parent);

#define GuiCreateContainer(w, h) GuiCreateContainerWithParent(lv_scr_act(), w, h)
#define GuiCreateSingleCheckBox(parent, text) GuiCreateCheckBoxWithFont(parent, text, true, g_defIllustrateFont)
#define GuiCreateMultiCheckBox(parent, text) GuiCreateCheckBoxWithFont(parent, text, false, g_defIllustrateFont)
#define GuiCreateTitleLabel(parent, text) GuiCreateLabelWithFont(parent, text, g_defTitleFont)
#define GuiCreateScrollTitleLabel(parent, text) GuiCreateLabelWithFontScroll(parent, text, g_defTitleFont, 408)
#define GuiCreateLittleTitleLabel(parent, text) GuiCreateLabelWithFont(parent, text, g_defLittleTitleFont)
#define GuiCreateScrollLittleTitleLabel(parent, text, width) GuiCreateLabelWithFontScroll(parent, text, g_defLittleTitleFont, width)
#define GuiCreateTextLabel(parent, text) GuiCreateLabelWithFont(parent, text, g_defTextFont)
#define GuiCreateScrollTextLabel(parent, text, width) GuiCreateLabelWithFontScroll(parent, text, g_defTextFont, width)
#define GuiCreateIllustrateLabel(parent, text) GuiCreateLabelWithFont(parent, text, g_defIllustrateFont)
#define GuiCreateScrollIllustrateLabel(parent, text, width) GuiCreateLabelWithFontScroll(parent, text, g_defIllustrateFont, width)
#define GuiCreateColorIllustrateLabel(parent, text, color) GuiCreateLabelWithFontAndTextColor(parent, text, g_defIllustrateFont, color)
#define GuiCreateOrangeIllustrateLabel(parent, text) GuiCreateColorIllustrateLabel(parent, text, 0xF5870A)
#define GuiCreateBoldIllustrateLabel(parent, text) GuiCreateLabelWithFont(parent, text, g_defBoldIllustratFont)

#endif /* _GUI_OBJ_H */
