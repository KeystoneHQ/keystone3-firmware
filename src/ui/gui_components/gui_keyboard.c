
#include "gui.h"
#include "gui_obj.h"
#include "gui_create_wallet_widgets.h"
#include "gui_keyboard.h"
#include "user_memory.h"
#include "gui_hintbox.h"
#include "gui_views.h"
#include "gui_lock_widgets.h"
#include "gui_letter_tree.h"
#include "string.h"
#include "motor_manager.h"

#pragma GCC optimize ("O0")

#define LV_KB_BTN(width)                                    LV_BTNMATRIX_CTRL_POPOVER | LV_BTNMATRIX_CTRL_NO_REPEAT | width
#define LV_KB_RECOLOR_BTN(width)                            LV_BTNMATRIX_CTRL_RECOLOR | LV_BTNMATRIX_CTRL_NO_REPEAT | width
#define MNEMONIC_KB_12WORD_HEIGHT                           290
#define MNEMONIC_KB_18WORD_HEIGHT                           462
#define MNEMONIC_KB_20WORD_HEIGHT                           540
#define MNEMONIC_KB_24WORD_HEIGHT                           618
#define MNEMONIC_KB_33WORD_HEIGHT                           852
#define MNEMONIC_KB_12WORD_CONT_HEIGHT                      480
#define MNEMONIC_KB_24WORD_CONT_HEIGHT                      734
#define MNEMONIC_KB_33WORD_CONT_HEIGHT                      484
#define MNEMONIC_KB_CONT_WIDTH                              408
#define LETTER_KB_HEIGHT                                    180
#define LETTER_KB_CONT_HEIGHT                               242
#define DEFAULT_KB_HEIGHT                                   310
#define DEFAULT_KB_CONT_HEIGHT                              310
#define DEFAULT_KB_CONT_WIDTH                               480

static void KbTextAreaHandler(lv_event_t *e);

static const char *g_numBtnmMap[] = {
    "1", "2", "3", "\n",
    "4", "5", "6", "\n",
    "7", "8", "9", "\n",
    " ", "0", USR_SYMBOL_DELETE, '\0'
};

static lv_btnmatrix_ctrl_t g_numBtnmMapCtrl[] = {
    4, 4, 4,
    4, 4, 4,
    4, 4, 4,
    LV_BTNMATRIX_CTRL_HIDDEN | 4, 4, 4
};

static lv_btnmatrix_ctrl_t g_numBtnm12MapCtrl[] = {
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
};

static lv_btnmatrix_ctrl_t g_numBtnm18MapCtrl[] = {
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
};

static lv_btnmatrix_ctrl_t g_numBtnm20MapCtrl[] = {
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_BTNMATRIX_CTRL_HIDDEN | LV_KB_RECOLOR_BTN(4),
};

static lv_btnmatrix_ctrl_t g_numBtnm24MapCtrl[] = {
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
};

static lv_btnmatrix_ctrl_t g_numBtnm33MapCtrl[] = {
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4),
    LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4), LV_KB_RECOLOR_BTN(4)
};

static const char *g_normalNumBtnmMap[] = {
    "1", "2", "3", "\n",
    "4", "5", "6", "\n",
    "7", "8", "9", "\n",
    "-", "0", LV_SYMBOL_OK, '\0'
};

static char *g_emojiBtnmMap[] = {
    "1", "2", "3", "4", "\n",
    "5", "6", "7", "8", "\n",
    "9", "10", "11", "12", "\n",
    "13", "14", "15", "16", '\0'
};

static const char *g_selectSliceBtnmMap[] = {
    "2", "3", "4", "5", "6", "\n",
    "7", "8", "9", "10", "11", "\n",
    "12", "13", "14", "15", "16", "\0",
};

static const char *g_selectSlice2BtnmMap[] = {"2", "\0"};
static const char *g_selectSlice3BtnmMap[] = {"2", "3", "\0"};
static const char *g_selectSlice4BtnmMap[] = {"2", "3", "4", "\0"};
static const char *g_selectSlice5BtnmMap[] = {"2", "3", "4", "5", "\0"};
static const char *g_selectSlice6BtnmMap[] = {"2", "3", "4", "5", "6", "\0"};
static const char *g_selectSlice7BtnmMap[] = {"2", "3", "4", "5", "6", "\n", "7", "\0"};
static const char *g_selectSlice8BtnmMap[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "\0"};
static const char *g_selectSlice9BtnmMap[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "9", "\0"};
static const char *g_selectSlice10BtnmMap[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "9", "10", "\0"};
static const char *g_selectSlice11BtnmMap[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "9", "10", "11", "\0"};
static const char *g_selectSlice12BtnmMap[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "9", "10", "11", "\n", "12", "\0"};
static const char *g_selectSlice13BtnmMap[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "9", "10", "11", "\n", "12", "13", "\0"};
static const char *g_selectSlice14BtnmMap[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "9", "10", "11", "\n", "12", "13", "14", "\0"};
static const char *g_selectSlice15BtnmMap[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "9", "10", "11", "\n", "12", "13", "14", "15", "\0"};

static lv_btnmatrix_ctrl_t g_selectSlice7BtnmMapCtrl[] = {
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_BTNMATRIX_CTRL_HIDDEN | 4, LV_BTNMATRIX_CTRL_HIDDEN | 4, LV_BTNMATRIX_CTRL_HIDDEN | 4, LV_BTNMATRIX_CTRL_HIDDEN | 4
};
#if 0
//static lv_btnmatrix_ctrl_t g_selectSlice2BtnmMapCtrl[] = {LV_KB_BTN(4)};
static lv_btnmatrix_ctrl_t g_selectSlice5BtnmMapCtrl[] = {LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4)};
static lv_btnmatrix_ctrl_t g_selectSlice6BtnmMapCtrl[] = {LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4)};
static lv_btnmatrix_ctrl_t g_selectSlice3BtnmMapCtrl[] = {LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4)};
static lv_btnmatrix_ctrl_t g_selectSlice4BtnmMapCtrl[] = {"2", "3", "4", "\0"};
static lv_btnmatrix_ctrl_t g_selectSlice5BtnmMapCtrl[] = {"2", "3", "4", "5", "\0"};
static lv_btnmatrix_ctrl_t g_selectSlice7BtnmMapCtrl[] = {"2", "3", "4", "5", "6", "\n", "7", "\0"};
static lv_btnmatrix_ctrl_t g_selectSlice8BtnmMapCtrl[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "\0"};
static lv_btnmatrix_ctrl_t g_selectSlice9BtnmMapCtrl[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "9", "\0"};
static lv_btnmatrix_ctrl_t g_selectSlice10BtnmMapCtrl[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "9", "10", "\0"};
static lv_btnmatrix_ctrl_t g_selectSlice11BtnmMapCtrl[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "9", "10", "11", "\0"};
static lv_btnmatrix_ctrl_t g_selectSlice12BtnmMapCtrl[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "9", "10", "11", "\n", "12", "\0"};
static lv_btnmatrix_ctrl_t g_selectSlice13BtnmMapCtrl[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "9", "10", "11", "\n", "12", "13", "\0"};
static lv_btnmatrix_ctrl_t g_selectSlice14BtnmMapCtrl[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "9", "10", "11", "\n", "12", "13", "14", "\0"};
static lv_btnmatrix_ctrl_t g_selectSlice15BtnmMapCtrl[] = {"2", "3", "4", "5", "6", "\n", "7", "8", "9", "10", "11", "\n", "12", "13", "14", "15", "\0"};
#endif
static const char **g_selectSliceVariableBtnmMap[15] = {
    (const char **)g_selectSlice2BtnmMap,
    (const char **)g_selectSlice3BtnmMap,
    (const char **)g_selectSlice4BtnmMap,
    (const char **)g_selectSlice5BtnmMap,
    (const char **)g_selectSlice6BtnmMap,
    (const char **)g_selectSlice7BtnmMap,
    (const char **)g_selectSlice8BtnmMap,
    (const char **)g_selectSlice9BtnmMap,
    (const char **)g_selectSlice10BtnmMap,
    (const char **)g_selectSlice11BtnmMap,
    (const char **)g_selectSlice12BtnmMap,
    (const char **)g_selectSlice13BtnmMap,
    (const char **)g_selectSlice14BtnmMap,
    (const char **)g_selectSlice15BtnmMap,
    (const char **)g_selectSliceBtnmMap,
};

typedef struct {
    const lv_btnmatrix_ctrl_t *btnMatrixCtl;
    uint16_t size;
} BtnMatrixCtl_t;

static const lv_img_dsc_t *g_emojiMatrix[16] = {
    &emojiBitcoin, &emojiEth,       &emojiLogo,     &emojiAt,
    &emojiSafe,    &emojiFlash,     &emojiAlien,    &emojiHappy,
    &emojiRocket,  &emojiCrown,     &emojiCopper,   &emojiStar,
    &emojiMusic,   &emojiHeart,     &emojiCompass,  &emojiGame,
};

static MnemonicKeyBoard_t *g_importPhraseKb = NULL;
static lv_obj_t *g_walletIcon = NULL;
static lv_obj_t *g_enterProgressLabel = NULL;
static uint8_t g_currEmojiIndex = 0;
static uint8_t g_statusBarEmojiIndex = 0;
char g_wordBuf[3][32];
static char g_wordChange[32];
extern TrieSTPtr rootTree;

static const char *const g_fullKbLcMap[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
    " ", "a", "s", "d", "f", "g", "h", "j", "k", "l", " ", "\n",
    USR_SYMBOL_UPPER, "z", "x", "c", "v", "b", "n", "m", USR_SYMBOL_DELETE, "\n",
    "#@", USR_SYMBOL_SPACE, USR_SYMBOL_KB_NEXT, '\0'
};

static const char *const g_fullKbLcConfirmMap[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
    " ", "a", "s", "d", "f", "g", "h", "j", "k", "l", " ", "\n",
    USR_SYMBOL_UPPER, "z", "x", "c", "v", "b", "n", "m", USR_SYMBOL_DELETE, "\n",
    "#@", USR_SYMBOL_SPACE, USR_SYMBOL_KB_NEXT, '\0'
};

static const char *const g_fullKbUcMap[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",   // 9
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "\n",   // 19
    " ", "A", "S", "D", "F", "G", "H", "J", "K", "L", " ", "\n",  // 30
    USR_SYMBOL_UPPER, "Z", "X", "C", "V", "B", "N", "M", USR_SYMBOL_DELETE, "\n", // 39
    "#@", USR_SYMBOL_SPACE, USR_SYMBOL_KB_NEXT, '\0'
};

static const char *const g_fullKbUcConfirmMap[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",   // 9
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "\n",   // 19
    " ", "A", "S", "D", "F", "G", "H", "J", "K", "L", " ", "\n",  // 30
    USR_SYMBOL_UPPER, "Z", "X", "C", "V", "B", "N", "M", USR_SYMBOL_DELETE, "\n", // 39
    "#@", USR_SYMBOL_SPACE, USR_SYMBOL_CONFIRM, '\0'
};

static const char *const g_symbolMap[] = {
    "[", "]", "{", "}", "#", "%", "^", "*", "+", "=", "\n",
    "_", "\\", "|", "~", "<", ">", "€", "£", "¥", "·", "\n",
    " ", "-", "/", ":", ";", "(", ")", "$", "&", "\"", " ", "\n",
    " ", "`", ".", ",", "?", "!", "'", "@", USR_SYMBOL_DELETE, "\n",
    "abc", USR_SYMBOL_SPACE, USR_SYMBOL_KB_NEXT, '\0'
};

const static uint8_t g_KeyMatchTable[] = {
    11, 26, 24, 13,  2, 14, 15,         // a b c d e f g
    16,  7, 17, 18, 19, 28, 27,         // h i j k l m n
    8,  9,  0,      3,  12, 4,          // o p q   r s t
    6, 25,  1,     23,  5, 22, 29, 21   // u v w   x y z check del
};

static const char *const g_letterLcMap[] = {
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
    " ", "a", "s", "d", "f", "g", "h", "j", "k", "l", " ", "\n",  // 20
    USR_SYMBOL_DELETE, "z", "x", "c", "v", "b", "n", "m", USR_SYMBOL_KB_NEXT, "\0"
};

static const char *const g_letterConfirmMap[] = {
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
    " ", "a", "s", "d", "f", "g", "h", "j", "k", "l", " ", "\n",  // 20
    USR_SYMBOL_DELETE, "z", "x", "c", "v", "b", "n", "m", USR_SYMBOL_CHECK, "\0"
};

static lv_btnmatrix_ctrl_t g_fullCtrlMap[] = {
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_BTNMATRIX_CTRL_HIDDEN | 2, LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),  LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_BTNMATRIX_CTRL_HIDDEN | 2,
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(6),
    LV_BTNMATRIX_CTRL_CUSTOM_1 | LV_KB_BTN(2), LV_KB_BTN(6), LV_KB_BTN(2) | LV_BTNMATRIX_CTRL_NO_REPEAT,
    // LV_BTNMATRIX_CTRL_CUSTOM_1 | LV_KB_BTN(2), LV_KB_BTN(6), LV_KB_BTN(2) | LV_BTNMATRIX_CTRL_DISABLED
};

static const lv_btnmatrix_ctrl_t g_fullCtrlBak[] = {
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_BTNMATRIX_CTRL_HIDDEN | 2, LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),  LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_BTNMATRIX_CTRL_HIDDEN | 2,
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(6),
    LV_KB_BTN(2), LV_KB_BTN(6), LV_KB_BTN(2) | LV_BTNMATRIX_CTRL_DISABLED | LV_BTNMATRIX_CTRL_NO_REPEAT
};

static lv_btnmatrix_ctrl_t g_symbolCtrlMap[] = {
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_BTNMATRIX_CTRL_HIDDEN | 2, LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),  LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_BTNMATRIX_CTRL_HIDDEN | 1,
    LV_BTNMATRIX_CTRL_HIDDEN | 5, LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(6),
    LV_KB_BTN(2), LV_KB_BTN(6), LV_KB_BTN(2) | LV_BTNMATRIX_CTRL_CHECKED
};

static lv_btnmatrix_ctrl_t g_letterCtrlMap[] = {
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_BTNMATRIX_CTRL_HIDDEN | 2, LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),  LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_BTNMATRIX_CTRL_HIDDEN | 1,
    LV_KB_BTN(5), LV_KB_BTN(4), LV_KB_BTN(4) | LV_BTNMATRIX_CTRL_DISABLED, LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(5) | LV_BTNMATRIX_CTRL_CHECKED | LV_BTNMATRIX_CTRL_DISABLED
};

static const lv_btnmatrix_ctrl_t g_letterCtrlMapBak[] = {
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_BTNMATRIX_CTRL_HIDDEN | 2, LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),  LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_BTNMATRIX_CTRL_HIDDEN | 1,
    LV_KB_BTN(5), LV_KB_BTN(4), LV_KB_BTN(4) | LV_BTNMATRIX_CTRL_DISABLED, LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(5) | LV_BTNMATRIX_CTRL_CHECKED | LV_BTNMATRIX_CTRL_DISABLED
};

static lv_btnmatrix_ctrl_t g_passPhraseSymbolCtrlMap[] = {
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4) | LV_BTNMATRIX_CTRL_DISABLED, LV_KB_BTN(4) | LV_BTNMATRIX_CTRL_DISABLED, LV_KB_BTN(4) | LV_BTNMATRIX_CTRL_DISABLED, LV_KB_BTN(4) | LV_BTNMATRIX_CTRL_DISABLED,
    LV_BTNMATRIX_CTRL_HIDDEN | 2, LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),  LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_BTNMATRIX_CTRL_HIDDEN | 1,
    LV_BTNMATRIX_CTRL_HIDDEN | 5, LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(6),
    LV_KB_BTN(2), LV_KB_BTN(6), LV_KB_BTN(2) | LV_BTNMATRIX_CTRL_CHECKED
};

#if 0
static const lv_btnmatrix_ctrl_t g_mnemonic12CtrlMap[] = {
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4)
};

static const lv_btnmatrix_ctrl_t g_mnemonic24CtrlMap[] = {
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4),
    LV_KB_BTN(4), LV_KB_BTN(4), LV_KB_BTN(4)
};
#endif

static const char **g_kbConfirmMap[2] = {
    (const char **)g_fullKbLcConfirmMap,
    (const char **)g_fullKbUcConfirmMap,
};

static char **g_kbMap[4] = {
    (char **)g_fullKbLcMap,
    (char **)g_fullKbUcMap,
    (char **)g_symbolMap,
    (char **)g_letterLcMap,
};

static lv_btnmatrix_ctrl_t *g_kbCtrl[4] = {
    g_fullCtrlMap,
    g_fullCtrlMap,
    g_symbolCtrlMap,
    g_letterCtrlMap,
};

static const BtnMatrixCtl_t g_kbCtrlBak[4] = {
    {g_fullCtrlBak, sizeof(g_fullCtrlBak)},
    {g_fullCtrlBak, sizeof(g_fullCtrlBak)},
    {g_symbolCtrlMap, sizeof(g_symbolCtrlMap)},
    {g_letterCtrlMapBak, sizeof(g_letterCtrlMapBak)},
};

static void EmojiDrawEventHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *bgCont = lv_event_get_user_data(e);
    if (code == LV_EVENT_DRAW_PART_BEGIN) {
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
        if (dsc->class_p == &lv_btnmatrix_class && dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
            dsc->rect_dsc->radius = 30;
            dsc->rect_dsc->bg_color = DARK_BG_COLOR;
            dsc->rect_dsc->shadow_width = 0;
            dsc->rect_dsc->border_width = 1;
            dsc->rect_dsc->outline_width = 0;
            dsc->rect_dsc->outline_color = WHITE_COLOR;
            uint32_t currentId = lv_btnmatrix_get_selected_btn(obj);
            if (currentId == dsc->id) {
                dsc->rect_dsc->border_color = ORANGE_COLOR;
                if (g_walletIcon != NULL) {
                    lv_img_set_src(g_walletIcon, g_emojiMatrix[dsc->id]);
                }
                g_currEmojiIndex = currentId;
            } else {
                dsc->rect_dsc->border_opa = LV_OPA_10 + LV_OPA_2;
                dsc->rect_dsc->border_color = WHITE_COLOR;
            }
        }
    } else if (code == LV_EVENT_DRAW_PART_END) {
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);

        if (dsc->class_p == &lv_btnmatrix_class && dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
            lv_img_header_t header;
            lv_res_t res = lv_img_decoder_get_info(g_emojiMatrix[dsc->id], &header);
            if (res != LV_RES_OK) return;

            lv_area_t a;
            a.x1 = dsc->draw_area->x1 + (lv_area_get_width(dsc->draw_area) - header.w) / 2;
            a.x2 = a.x1 + header.w - 1;
            a.y1 = dsc->draw_area->y1 + (lv_area_get_height(dsc->draw_area) - header.h) / 2;
            a.y2 = a.y1 + header.h - 1;

            lv_draw_img_dsc_t img_draw_dsc;
            lv_draw_img_dsc_init(&img_draw_dsc);
            img_draw_dsc.recolor = lv_color_black();
            lv_draw_img(dsc->draw_ctx, &img_draw_dsc, &a, g_emojiMatrix[dsc->id]);
        }
    } else if (code == LV_EVENT_CLICKED) {
        lv_obj_del(bgCont);
        Vibrate(SLIGHT);
    }
}

void GuiSetEmojiIconIndex(uint8_t index)
{
    if (GUI_KEYBOARD_EMOJI_CANCEL_NEW_INDEX == index) {
        index = g_statusBarEmojiIndex;
    } else if (GUI_KEYBOARD_EMOJI_NEW_INDEX == index) {
        index = 0;
    }
    g_currEmojiIndex = index;
}

uint8_t GuiSearchIconIndex(lv_obj_t *icon)
{
    const void *img = lv_img_get_src(icon);
    for (uint8_t i = 0; i < sizeof(g_emojiMatrix) / sizeof(g_emojiMatrix[0]); i++) {
        if (g_emojiMatrix[i] == img) {
            return i;
        }
    }
    return 0;
}

uint8_t GuiGetEmojiIconIndex(void)
{
    return g_currEmojiIndex;
}

const lv_img_dsc_t *GuiGetEmojiIconImg(void)
{
    return g_emojiMatrix[g_currEmojiIndex];
}

void SetStatusBarEmojiIndex(uint8_t index)
{
    g_statusBarEmojiIndex = index;
}

void *GuiCreateEmojiKeyBoard(lv_obj_t *parent, lv_obj_t *icon)
{
    g_walletIcon = icon;
    lv_obj_t *hintbox = GuiCreateHintBox(parent, 480, 534, true);
    lv_obj_add_event_cb(lv_obj_get_child(hintbox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label = GuiCreateNoticeLabel(hintbox, "Pick an icon for your wallet");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 296);

    lv_obj_t *img = GuiCreateImg(hintbox, &imgClose);
    lv_obj_align(img, LV_ALIGN_TOP_RIGHT, -36, 293);
    lv_obj_add_event_cb(img, CloseCurrentParentHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *btnm = lv_btnmatrix_create(hintbox);
    lv_btnmatrix_set_map(btnm, (const char **)g_emojiBtnmMap);
    lv_obj_set_size(btnm, 440, 440);
    lv_obj_align(btnm, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(btnm, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_color(btnm, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_width(btnm, 0, 0);
    lv_btnmatrix_set_selected_btn(btnm, g_currEmojiIndex);
    lv_obj_set_style_pad_gap(btnm, 24, LV_PART_MAIN);
    lv_obj_add_event_cb(btnm, EmojiDrawEventHandler, LV_EVENT_ALL, hintbox);
    return hintbox;
}

void *GuiCreateNumKeyboard(lv_obj_t *parent, lv_event_cb_t cb, NUM_KEYBOARD_ENUM numMode, void *param)
{
    uint16_t kbHeight = 310;
    uint16_t kbWidth = 480;
    lv_obj_t *btnm = lv_btnmatrix_create(parent);
    switch (numMode) {
    case NUM_KEYBOARD_PIN:
        lv_obj_add_style(btnm, &g_numBtnmStyle, LV_PART_ITEMS);
        lv_btnmatrix_set_map(btnm, (const char **)g_numBtnmMap);
        lv_obj_align(btnm, LV_ALIGN_TOP_MID, 0, 490 - GUI_MAIN_AREA_OFFSET);
        lv_obj_set_style_bg_color(btnm, DARK_BG_COLOR, LV_PART_MAIN);
        lv_btnmatrix_set_ctrl_map(btnm, g_numBtnmMapCtrl);
        break;
    case NUM_KEYBOARD_SLICE:
        kbHeight = 204;
        kbWidth = 408;

        lv_obj_add_style(btnm, &g_numShareStyle, LV_PART_ITEMS);
        lv_btnmatrix_set_map(btnm, (const char **)g_selectSliceBtnmMap);
        lv_obj_set_style_bg_color(btnm, BLACK_COLOR, LV_PART_MAIN);
        break;
    case NUM_KEYBOARD_NORMAL:
        lv_obj_add_style(btnm, &g_numBtnmStyle, LV_PART_ITEMS);
        lv_btnmatrix_set_map(btnm, (const char **)g_normalNumBtnmMap);
        lv_obj_set_style_bg_color(btnm, DARK_BG_COLOR, LV_PART_MAIN);
        lv_btnmatrix_set_btn_ctrl(btnm, 11, LV_BTNMATRIX_CTRL_CHECKED);

        lv_obj_add_style(btnm, &g_numBtnmDisabledStyle, LV_PART_ITEMS | LV_STATE_DISABLED);
        lv_obj_add_style(btnm, &g_numBtnmCheckedStyle, LV_PART_ITEMS | LV_STATE_CHECKED);
        break;
    default:
        lv_obj_del(btnm);
        return NULL;
    }
    lv_obj_set_size(btnm, kbWidth, kbHeight);
    lv_obj_set_style_text_font(btnm, &openSansButton, LV_PART_MAIN);
    lv_obj_set_style_border_width(btnm, 0, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(btnm, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btnm, 8, LV_PART_MAIN);
    lv_obj_set_style_radius(btnm, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(btnm, 4, LV_PART_MAIN);
#if 0
    lv_obj_set_style_bg_color(btnm, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_add_style(btnm, &g_numBtnmStyle, LV_PART_ITEMS);
#endif

    lv_obj_add_event_cb(btnm, cb, LV_EVENT_ALL, param);

    return btnm;
}

void GuiUpdateSsbKeyBoard(lv_obj_t *btnm, uint8_t memberCnt)
{
    lv_btnmatrix_set_map(btnm, (const char **)g_selectSliceVariableBtnmMap[memberCnt - 2]);
    if (memberCnt < 6) {
        lv_obj_set_size(btnm, 81 * (memberCnt - 1), 82);
    } else if (memberCnt == 6) {
        lv_obj_set_size(btnm, 408, 82);
    } else if (memberCnt <= 11) {
        lv_obj_set_size(btnm, 408, 2 * 60 + 18);
    } else {
        lv_obj_set_size(btnm, 408, 3 * 60 + 12 * 2);
    }

    if (memberCnt == 7) {
        lv_btnmatrix_set_ctrl_map(btnm, g_selectSlice7BtnmMapCtrl);
    }
}

void GuiDeleteKeyBoard(KeyBoard_t *kb)
{
    if (kb != NULL) {
        lv_obj_del(kb->ta);
        lv_obj_del(kb->cont);
        for (int i = 0; i < 3; i++) {
            if (kb->associateLabel[i] != NULL) {
                lv_obj_del(kb->associateLabel[i]);
            }
        }
        SRAM_FREE(kb);
    }
}

void GuiSetKeyBoardMinTaLen(KeyBoard_t *keyBoard, uint8_t len)
{
    keyBoard->taMinLen = len;
    UpdateFullKeyBoard("", keyBoard);
}

void *GuiCreateKeyBoard(lv_obj_t *parent, lv_event_cb_t cb, lv_keyboard_user_mode_t keyMode, void *param)
{
    uint16_t contHeight = DEFAULT_KB_CONT_HEIGHT;
    uint16_t kbHeight = DEFAULT_KB_HEIGHT;
    uint16_t contWidth = DEFAULT_KB_CONT_WIDTH;
    KeyBoard_t *keyBoard = SRAM_MALLOC(sizeof(KeyBoard_t));
    keyBoard->ta = NULL;
    for (int i = 0; i < 3; i++) {
        keyBoard->associateLabel[i] = NULL;
    }
    switch (keyMode) {
    case KEY_STONE_LETTER:
        contHeight = LETTER_KB_CONT_HEIGHT;
        kbHeight = LETTER_KB_HEIGHT;
        break;
    default:
        break;
    }
    keyBoard->cont = GuiCreateContainerWithParent(parent, contWidth, contHeight);
    keyBoard->mode = keyMode;
    lv_obj_set_align(keyBoard->cont, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_color(keyBoard->cont, DARK_BG_COLOR, LV_PART_MAIN);
    keyBoard->kb = lv_keyboard_create(keyBoard->cont);
    lv_obj_set_style_text_font(keyBoard->kb, &openSansButton, 0);
    lv_keyboard_set_map(keyBoard->kb, keyMode, (const char **)g_kbMap[keyMode - KEY_STONE_FULL_L],
                        g_kbCtrl[keyMode - KEY_STONE_FULL_L]);
    lv_keyboard_set_mode(keyBoard->kb, keyMode);
    lv_obj_set_size(keyBoard->kb, contWidth, kbHeight);
    lv_obj_set_style_bg_color(keyBoard->kb, DARK_BG_COLOR, 0);
    lv_obj_add_event_cb(keyBoard->kb, cb, LV_EVENT_ALL, param);
    return keyBoard;
}

void *GuiCreateFullKeyBoard(lv_obj_t *parent, lv_event_cb_t kbCb, lv_keyboard_user_mode_t keyMode, void *param)
{
    KeyBoard_t *keyBoard = GuiCreateKeyBoard(parent, kbCb, keyMode, param);
    lv_obj_t *textArea = lv_textarea_create(parent);
    lv_obj_add_event_cb(textArea, KbTextAreaHandler, LV_EVENT_ALL, keyBoard);
    lv_obj_set_style_text_font(textArea, &openSansButton, 0);
    if (GuiDarkMode()) {
        lv_obj_set_style_text_color(textArea, WHITE_COLOR, 0);
    } else {
    }
    lv_obj_set_size(textArea, 0, 0);
    lv_obj_align(textArea, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_bg_color(textArea, BLACK_COLOR, 0);
    lv_obj_set_style_text_opa(textArea, 0, 0);
    lv_obj_clear_flag(textArea, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(textArea, 0, LV_PART_MAIN);
    lv_keyboard_set_textarea(keyBoard->kb, textArea);
    keyBoard->ta = textArea;
    GuiSetKeyBoardMinTaLen(keyBoard, 6);
    return keyBoard;
}

void GuiSetFullKeyBoardConfirm(KeyBoard_t *keyBoard, bool en)
{
    if (keyBoard->mode != KEY_STONE_FULL_L || keyBoard->mode != KEY_STONE_FULL_U) {
        return;
    }
    if (!en) {
        lv_keyboard_set_map(keyBoard->kb, keyBoard->mode, (const char **)g_kbConfirmMap[keyBoard->mode - KEY_STONE_FULL_L],
                            g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L]);
    } else {
        lv_keyboard_set_map(keyBoard->kb, keyBoard->mode, (const char **)g_kbConfirmMap[keyBoard->mode - KEY_STONE_FULL_L],
                            g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L]);
    }
    lv_keyboard_set_mode(keyBoard->kb, keyBoard->mode);
}

void GuiSetEnterProgressLabel(lv_obj_t *label)
{
    g_enterProgressLabel = label;
}

void GuiClearEnterProgressLabel(void)
{
    lv_obj_set_style_text_color(g_enterProgressLabel, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(g_enterProgressLabel, LV_OPA_56, LV_PART_MAIN);
    lv_label_set_text(g_enterProgressLabel, "0/16");
}

void GuiDelEnterProgressLabel(void)
{
    g_enterProgressLabel = NULL;
}

void GuiSetFullKeyBoardTa(KeyBoard_t *keyBoard, lv_obj_t *ta)
{
    lv_keyboard_set_textarea(keyBoard->kb, ta);
    // lv_obj_add_event_cb(ta, KbTextAreaHandler, LV_EVENT_ALL, keyBoard);
    keyBoard->ta = ta;
}

void GuiConfirmFullKeyBoard(KeyBoard_t *keyBoard)
{
    char **tempMap = g_kbMap[keyBoard->mode - KEY_STONE_FULL_L];
    g_kbMap[keyBoard->mode - KEY_STONE_FULL_L] = (char **)g_kbConfirmMap[keyBoard->mode - KEY_STONE_FULL_L];
    lv_keyboard_set_map(keyBoard->kb, keyBoard->mode, (const char **)g_kbMap[keyBoard->mode - KEY_STONE_FULL_L],
                        g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L]);
    lv_keyboard_set_mode(keyBoard->kb, keyBoard->mode);
    g_kbMap[keyBoard->mode - KEY_STONE_FULL_L] = tempMap;
}

static void MnemonicDrawHandler(lv_event_t * e)
{
}

void *GuiCreateMnemonicKeyBoard(lv_obj_t *parent,
                                lv_event_cb_t kbCb, lv_keyboard_user_mode_t keyMode, char *mnemonic)
{
    uint16_t contHeight = MNEMONIC_KB_12WORD_CONT_HEIGHT;
    uint16_t kbHeight = MNEMONIC_KB_12WORD_HEIGHT;
    MnemonicKeyBoard_t *mkb = SRAM_MALLOC(sizeof(MnemonicKeyBoard_t));
    if (mkb == NULL) {
        return NULL;
    }

    switch (keyMode) {
    case KEY_STONE_MNEMONIC_12:
        mkb->wordCnt = 12;
        break;
    case KEY_STONE_MNEMONIC_18:
        mkb->wordCnt = 18;
        kbHeight = MNEMONIC_KB_18WORD_HEIGHT;
        contHeight = MNEMONIC_KB_18WORD_HEIGHT - 110;
        break;
    case KEY_STONE_MNEMONIC_20:
        mkb->wordCnt = 20;
        kbHeight = MNEMONIC_KB_20WORD_HEIGHT;
        contHeight = MNEMONIC_KB_20WORD_HEIGHT;
        break;
    case KEY_STONE_MNEMONIC_24:
        mkb->wordCnt = 24;
        kbHeight = MNEMONIC_KB_24WORD_HEIGHT;
        break;
    case KEY_STONE_MNEMONIC_33:
        mkb->wordCnt = 33;
        kbHeight = MNEMONIC_KB_33WORD_HEIGHT;
        contHeight = MNEMONIC_KB_33WORD_CONT_HEIGHT;
        break;
    default:
        SRAM_FREE(mkb);
        return NULL;
    }
    for (int i = 0; i < 33 * 4 / 3; i++) {
        mkb->mnemonicWord[i] = SRAM_MALLOC(24);
        strcpy(mkb->mnemonicWord[i], "\n");
    }

    for (int i = 0, j = 0; i < mkb->wordCnt; j++, i += 3) {
        for (int k = i; k < i + 3; k++) {
            sprintf(mkb->mnemonicWord[k + j], "%d\n", k + 1);
        }
    }
    strcpy(mkb->mnemonicWord[(mkb->wordCnt + 1) / 3 * 4 - 1], "\0");

    mkb->cont = GuiCreateContainerWithParent(parent, 408, contHeight - 110);
    lv_obj_set_align(mkb->cont, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(mkb->cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(mkb->cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(mkb->cont, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scrollbar_mode(mkb->cont, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *btnm = lv_btnmatrix_create(mkb->cont);
    lv_obj_set_size(btnm, MNEMONIC_KB_CONT_WIDTH, kbHeight);
    lv_obj_set_style_text_font(btnm, &openSans_20, 0);
    lv_btnmatrix_set_map(btnm, (const char **)mkb->mnemonicWord);
    lv_obj_align(btnm, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_border_width(btnm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(btnm, 0, 0);
    lv_obj_set_style_pad_all(btnm, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btnm, 0, 0);
    lv_obj_set_style_bg_color(btnm, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_add_event_cb(btnm, kbCb ? kbCb : MnemonicDrawHandler, LV_EVENT_ALL, mkb);
    lv_obj_add_style(btnm, &g_mnemonicDarkStyle, LV_PART_ITEMS);
    lv_obj_set_style_pad_gap(btnm, 4, LV_PART_MAIN);
    lv_obj_set_style_text_align(btnm, LV_TEXT_ALIGN_CENTER, 0);
    if (keyMode == KEY_STONE_MNEMONIC_12) {
        lv_btnmatrix_set_ctrl_map(btnm, g_numBtnm12MapCtrl);
    } else if (keyMode == KEY_STONE_MNEMONIC_18) {
        lv_btnmatrix_set_ctrl_map(btnm, g_numBtnm18MapCtrl);
    } else if (keyMode == KEY_STONE_MNEMONIC_20) {
        lv_btnmatrix_set_ctrl_map(btnm, g_numBtnm20MapCtrl);
    } else if (keyMode == KEY_STONE_MNEMONIC_24) {
        lv_btnmatrix_set_ctrl_map(btnm, g_numBtnm24MapCtrl);
    } else if (keyMode == KEY_STONE_MNEMONIC_33) {
        lv_btnmatrix_set_ctrl_map(btnm, g_numBtnm33MapCtrl);
    }
    // lv_obj_t *bottomCont = GuiCreateContainerWithParent(mkb->cont, 408, 50);
    // lv_obj_align_to(bottomCont, btnm, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    mkb->btnm = btnm;
    mkb->currentSlice = 0;
    mkb->currentId = 0;
    mkb->threShold = 0xff;
    mkb->titleLabel = NULL;
    mkb->descLabel = NULL;
    mkb->stepLabel = NULL;

    return mkb;
}

void ClearMnemonicKeyboard(MnemonicKeyBoard_t *mkb, uint8_t *inputId)
{
    *inputId = 0;
    lv_btnmatrix_set_selected_btn(mkb->btnm, *inputId);
    GuiClearMnemonicKeyBoard(mkb);
    lv_obj_scroll_to_y(mkb->cont, 0, LV_ANIM_ON);
}

void GuiClearMnemonicKeyBoard(MnemonicKeyBoard_t *mnemonicKeyBoard)
{
    for (int i = 0; i < 44; i++) {
        memset(mnemonicKeyBoard->mnemonicWord[i], 0, 20);
    }
    GuiUpdateMnemonicKeyBoard(mnemonicKeyBoard, (char *)NULL, false);
}

void GuiDelMnemonicKeyBoard(MnemonicKeyBoard_t * mnemonicKeyBoard)
{
    // TODO DEL
    for (int i = 0; i < 44; i++) {
        // SRAM_FREE(mnemonicKeyBoard->mnemonicWord[i]);
    }
    if (g_importPhraseKb != NULL) {
        lv_obj_del(g_importPhraseKb->btnm);
        g_importPhraseKb->btnm = NULL;
        g_importPhraseKb = NULL;
    }
}

void GuiUpdateMnemonicKeyBoard(MnemonicKeyBoard_t *mnemonicKeyBoard, char *mnemonic, bool confirm)
{
    char *find = mnemonic, *tail, word[10];
    uint16_t contHeight = MNEMONIC_KB_12WORD_CONT_HEIGHT;
    uint16_t kbHeight = MNEMONIC_KB_12WORD_HEIGHT;
    for (int i = 0; i < (mnemonicKeyBoard->wordCnt) * 4 / 3; i++) {
        strcpy(mnemonicKeyBoard->mnemonicWord[i], "\n");
    }
    for (int i = 0, j = 0; i < mnemonicKeyBoard->wordCnt; j++, i += 3) {
        for (int k = i; k < i + 3; k++) {
            if (mnemonic == NULL) {
                sprintf(mnemonicKeyBoard->mnemonicWord[k + j], "%d\n", k + 1);
                continue;
            }
            if (mnemonicKeyBoard->wordCnt == 20 && k >= 18) {
                break;
            }
            if (k == mnemonicKeyBoard->wordCnt - 1) {
                tail = strchr(find, 0);
            } else {
                tail = strchr(find, ' ');
            }
            memcpy(word, find, tail - find);
            word[tail - find] = 0;
            find = tail + 1;
            if (confirm) {
                sprintf(mnemonicKeyBoard->mnemonicWord[k + j], "-\n%s", word);
            } else {
                sprintf(mnemonicKeyBoard->mnemonicWord[k + j], "%d\n%s", k + 1, word);
            }
        }
    }
    strcpy(mnemonicKeyBoard->mnemonicWord[(mnemonicKeyBoard->wordCnt + 1) / 3 * 4 - 1], "\0");
    lv_btnmatrix_set_map(mnemonicKeyBoard->btnm, (const char **)mnemonicKeyBoard->mnemonicWord);
    switch (mnemonicKeyBoard->wordCnt) {
    case 12:
        lv_btnmatrix_set_ctrl_map(mnemonicKeyBoard->btnm, g_numBtnm12MapCtrl);
        break;
    case 18:
        lv_btnmatrix_set_ctrl_map(mnemonicKeyBoard->btnm, g_numBtnm18MapCtrl);
        kbHeight = MNEMONIC_KB_18WORD_HEIGHT;
        contHeight = MNEMONIC_KB_18WORD_HEIGHT - 110;
        break;
    case 20:
        kbHeight = MNEMONIC_KB_20WORD_HEIGHT;
        contHeight = MNEMONIC_KB_20WORD_HEIGHT - 110;
        lv_btnmatrix_set_ctrl_map(mnemonicKeyBoard->btnm, g_numBtnm20MapCtrl);
        break;
    case 24:
        kbHeight = MNEMONIC_KB_24WORD_HEIGHT;
        contHeight += 150;
        lv_btnmatrix_set_ctrl_map(mnemonicKeyBoard->btnm, g_numBtnm24MapCtrl);
        break;
    case 33:
        lv_btnmatrix_set_ctrl_map(mnemonicKeyBoard->btnm, g_numBtnm33MapCtrl);
        kbHeight = MNEMONIC_KB_33WORD_HEIGHT;
        contHeight += 150;
        break;
    }
    lv_obj_set_size(mnemonicKeyBoard->btnm, MNEMONIC_KB_CONT_WIDTH, kbHeight);
    // lv_obj_set_size(mnemonicKeyBoard->cont, MNEMONIC_KB_CONT_WIDTH, contHeight);
}

void GuiConfirmMnemonicKeyBoard(MnemonicKeyBoard_t *mnemonicKeyBoard,
                                char *mnemonic, int n, int num, int dig)
{
    char *find = mnemonic, *tail, word[10];
    for (int i = 0, j = 0; i < mnemonicKeyBoard->wordCnt; j++, i += 3) {
        for (int k = i; k < i + 3; k++) {
            if (k == mnemonicKeyBoard->wordCnt - 1) {
                tail = strchr(find, 0);
            } else {
                tail = strchr(find, ' ');
            }
            memcpy(word, find, tail - find);
            word[tail - find] = 0;
            find = tail + 1;
            if (k == n) {
                if (dig == 1) {
                    sprintf(mnemonicKeyBoard->mnemonicWord[k + j], "%d\n%s", num, word);
                } else {
                    sprintf(mnemonicKeyBoard->mnemonicWord[k + j], "-\n%s", word);
                }
            }
        }
    }

    lv_btnmatrix_set_map(mnemonicKeyBoard->btnm, (const char **)mnemonicKeyBoard->mnemonicWord);
}

void GuiInputMnemonicKeyBoard(MnemonicKeyBoard_t* inputMnemonicKeyBoard, char *word, int n, int dig)
{
    for (int i = 0, j = 0; i < inputMnemonicKeyBoard->wordCnt; j++, i += 3) {
        for (int k = i; k < i + 3; k++) {
            if (k == n) {
                if (dig == 1) {
                    sprintf(inputMnemonicKeyBoard->mnemonicWord[k + j], "%d\n%s", k + j + 1 - k / 3, word);
                } else {
                    sprintf(inputMnemonicKeyBoard->mnemonicWord[k + j], "%d\n", k + j + 1 - k / 3);
                }
            }
        }
    }

    lv_btnmatrix_set_map(inputMnemonicKeyBoard->btnm, (const char**)inputMnemonicKeyBoard->mnemonicWord);
}

volatile static int g_letterConfirm = 0;
void GuiSetLetterBoardConfirm(KeyBoard_t *keyBoard, int en)
{
    g_letterConfirm = en;
    GuiKeyBoardSetMode(keyBoard);
}

void GuiSetLetterBoardNext(KeyBoard_t *keyBoard)
{
    g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L][g_KeyMatchTable[26]] &= ~LV_BTNMATRIX_CTRL_DISABLED; // check
    GuiKeyBoardSetMode(keyBoard);
}

void GuiUpdatePassPhraseKb(KeyBoard_t *keyBoard)
{
    lv_keyboard_set_map(keyBoard->kb, KEY_STONE_SYMBOL, (const char **)g_symbolMap,
                        g_passPhraseSymbolCtrlMap);
    lv_keyboard_set_mode(keyBoard->kb, keyBoard->mode);
}

void GuiKeyBoardSetMode(KeyBoard_t *keyBoard)
{
    if (keyBoard->mode == KEY_STONE_LETTER) {
        if (g_letterConfirm != 0) {
            g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L][g_KeyMatchTable[26]] &= ~LV_BTNMATRIX_CTRL_DISABLED; // check
            lv_keyboard_set_map(keyBoard->kb, KEY_STONE_LETTER, (const char **)g_letterConfirmMap,
                                g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L]);
        } else {
            lv_keyboard_set_map(keyBoard->kb, KEY_STONE_LETTER, (const char **)g_kbMap[keyBoard->mode - KEY_STONE_FULL_L],
                                g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L]);
        }
    } else {
        lv_keyboard_set_map(keyBoard->kb, keyBoard->mode, (const char **)g_kbMap[keyBoard->mode - KEY_STONE_FULL_L],
                            g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L]);
    }
    lv_keyboard_set_mode(keyBoard->kb, keyBoard->mode);
}

void GuiKeyBoardLetterUpdate(KeyBoard_t *keyBoard, uint8_t *enable)
{
    for (int i = 0; i < 26 + 2; i++) {
        g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L][g_KeyMatchTable[i]] &= ~LV_BTNMATRIX_CTRL_DISABLED;
        g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L][g_KeyMatchTable[i]] |= LV_BTNMATRIX_CTRL_DISABLED & enable[i];
    }
    g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L][g_KeyMatchTable[26]] |= LV_BTNMATRIX_CTRL_DISABLED & enable[26];
    g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L][g_KeyMatchTable[27]] |= LV_BTNMATRIX_CTRL_DISABLED & enable[27];
}

void GuiKeyBoardRestoreDefault(KeyBoard_t *keyBoard)
{
    // todo keyboard?
    memcpy(g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L], g_kbCtrlBak[keyBoard->mode - KEY_STONE_FULL_L].btnMatrixCtl,
           g_kbCtrlBak[keyBoard->mode - KEY_STONE_FULL_L].size);
}

void GuiClearKeyBoard(KeyBoard_t *keyBoard)
{
    GuiKeyBoardRestoreDefault(keyBoard);
    GuiKeyBoardSetMode(keyBoard);
    lv_textarea_set_text(keyBoard->ta, "");
    for (int i = 0; i < 3; i++) {
        memset(g_wordBuf[i], 0, sizeof(g_wordBuf[i]));
        lv_label_set_text(keyBoard->associateLabel[i], "");
    }
}

// When the letter mnemonic is entered as proof, the keyboard will be disabled and all keyboard letters will be marked as disabled.
bool GuiLetterKbStatusError(void)
{
    for (int i = 0; i < NUMBER_OF_ARRAYS(g_letterCtrlMap); i++) {
        if (i == 10 || i == 20 || i == 21 || i == 29) {
            continue;
        }
        if ((g_letterCtrlMap[i] & LV_BTNMATRIX_CTRL_DISABLED) != LV_BTNMATRIX_CTRL_DISABLED) {
            return false;
        }
    }

    memcpy(g_kbCtrl[KEY_STONE_LETTER - KEY_STONE_FULL_L], g_kbCtrlBak[KEY_STONE_LETTER - KEY_STONE_FULL_L].btnMatrixCtl,
           g_kbCtrlBak[KEY_STONE_LETTER - KEY_STONE_FULL_L].size);
    GuiEmitSignal(GUI_EVENT_UPDATE_KEYBOARD, NULL, 0);

    return true;
}

bool GuiSingleWordsWhole(const char *text)
{
    int wordcnt = searchTrie(rootTree, text);
    if (wordcnt == 1) {
        if (strcmp(text, g_wordBuf[0]) == 0) {
            return true;
        }
        return false;
    }
    return false;
}

bool GuiWordsWhole(const char* text)
{
    int wordcnt = searchTrie(rootTree, text);
    if (wordcnt > 0) {
        if (!strcmp(text, g_wordBuf[0])) {
            return true;
        }
        return false;
    }
    return false;
}

void UpdateFullKeyBoard(const char *str, KeyBoard_t *keyBoard)
{
    // if (keyBoard->mode != KEY_STONE_FULL_L && keyBoard->mode != KEY_STONE_FULL_U) {
    //     return;
    // }

    // if (strlen(str) >= keyBoard->taMinLen) {
    //     g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L][42] = LV_BTNMATRIX_CTRL_CHECKED | 2;
    // } else {
    //     g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L][42] = LV_BTNMATRIX_CTRL_HIDDEN | 2;
    //     // g_kbCtrl[keyBoard->mode - KEY_STONE_FULL_L][42] = LV_BTNMATRIX_CTRL_DISABLED | 2;
    // }
    // GuiKeyBoardSetMode(keyBoard);
}

void UpdateKeyBoard(TrieSTPtr root, const char *str, KeyBoard_t *keyBoard)
{
    if (keyBoard->mode != KEY_STONE_LETTER || root == NULL) {
        return;
    }
    GuiKeyBoardRestoreDefault(keyBoard);
    if (strlen(str) == 0) {
        GuiKeyBoardSetMode(keyBoard);
        return;
    }
    TrieSTPtr tmp = rootTree;
    int i = 0;
    uint8_t enable[CHAR_LENGTH + 2] = {0};
    while (str[i] != '\0') {
        if (tmp->next[str[i] - 'a'] != NULL) {
            tmp = tmp->next[str[i] - 'a'];
        }
        i++;
    }

    for (int j = 0; j <= 'z' - 'a'; j++) {
        if (tmp->next[j] == NULL) {
            enable[j] = LV_BTNMATRIX_CTRL_DISABLED;
        }
    }

    if (searchTrie(rootTree, str) != 1) {
        if (!g_letterConfirm) {
            enable[26] = LV_BTNMATRIX_CTRL_DISABLED;
        }
    }

    if (strlen(str) < 3) {
        enable[26] = LV_BTNMATRIX_CTRL_DISABLED;
    }

    if (strlen(str) == 0) {
        // enable[27] = LV_BTNMATRIX_CTRL_DISABLED;
    }

    GuiKeyBoardLetterUpdate(keyBoard, enable);
    GuiKeyBoardSetMode(keyBoard);
}

static void LetterKbAssociateHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    KeyBoard_t *keyBoard = lv_event_get_user_data(e);
    char *text = lv_label_get_text(lv_obj_get_child(lv_event_get_target(e), 0));
    char buf[1] = {0};
    if (code == LV_EVENT_CLICKED) {
        strcpy(g_wordChange, text);
        if (g_importPhraseKb != NULL) {
            lv_event_send(g_importPhraseKb->btnm, LV_EVENT_READY, g_wordChange);
        }
        lv_textarea_set_text(keyBoard->ta, "");
        for (int i = 0; i < 3; i++) {
            lv_label_set_text(keyBoard->associateLabel[i], "");
            memset(g_wordBuf[i], 0, sizeof(g_wordBuf[0]));
        }
        UpdateKeyBoard(rootTree, buf, keyBoard);
    }
}

static void CloseLetterKbHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    KeyBoard_t *keyBoard = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        if (keyBoard->mode == KEY_STONE_LETTER) {
            if (g_importPhraseKb != NULL) {
                lv_event_send(g_importPhraseKb->btnm, KEY_STONE_KEYBOARD_HIDDEN, NULL);
            }
        }
        lv_obj_add_flag(keyBoard->cont, LV_OBJ_FLAG_HIDDEN);
    }
}

void UpdateAssociateLabel(KeyBoard_t *keyBoard, const char *currText)
{
    char endBuf[8] = { 0 };
    if (strlen(currText) >= 3) {
        for (int i = 0; i < 3; i++) {
            memset(g_wordBuf[i], 0, sizeof(g_wordBuf[i]));
            lv_label_set_text(keyBoard->associateLabel[i], "");
        }
        int wordcnt = searchTrie(rootTree, currText);
        if (wordcnt == 1) {
            strcpy(endBuf, g_wordBuf[0]);
            lv_label_set_text(keyBoard->associateLabel[0], endBuf);
        } else while (wordcnt--) {
                lv_label_set_text(keyBoard->associateLabel[wordcnt], g_wordBuf[wordcnt]);
                strcpy(endBuf, g_wordBuf[wordcnt]);
            }
    }
}

void ClearKbCache(void)
{
    for (int i = 0; i < 3; i++) {
        memset(g_wordBuf[i], 0, sizeof(g_wordBuf[i]));
    }
}

char *GuiGetTrueWord(const lv_obj_t *obj, uint16_t btn_id)
{
    const char *tempText = lv_btnmatrix_get_btn_text(obj, btn_id);
    char *trueText = strstr(tempText, "\n") + 1;
    if (trueText != NULL) {
        if (trueText != NULL) {
            if (strstr(trueText, "#0 ")) {
                trueText += strlen("#0 ");
                tempText = strstr(trueText, "#");
                if (tempText == strstr(trueText, "#")) {
                    trueText[tempText - trueText] = '\0';
                }
            }
        }
    }
    return trueText;
}

void KbTextAreaHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    uint8_t taLen = strlen(lv_textarea_get_text(ta));
    KeyBoard_t *keyBoard = lv_event_get_user_data(e);
    const char *currText = lv_textarea_get_text(ta);
    if (code == LV_EVENT_VALUE_CHANGED) {
        // Vibrate(SLIGHT);
        if (keyBoard == NULL) {
            return;
        }
        if (keyBoard->mode == KEY_STONE_LETTER) {
            if (g_importPhraseKb != NULL) {
                lv_event_send(g_importPhraseKb->btnm, KEY_STONE_KEYBOARD_VALUE_CHANGE, (void *)currText);
            }
            UpdateKeyBoard(rootTree, currText, keyBoard);
        } else {
            if (keyBoard->mode == KEY_STONE_FULL_L || keyBoard->mode == KEY_STONE_FULL_U) {
                UpdateFullKeyBoard(currText, keyBoard);
            }
            if (g_enterProgressLabel != NULL) {
                if (taLen >= 16) {
                    lv_obj_set_style_text_color(g_enterProgressLabel, RED_COLOR, LV_PART_MAIN);
                    lv_obj_set_style_text_opa(g_enterProgressLabel, LV_OPA_100, LV_PART_MAIN);
                }
                lv_label_set_text_fmt(g_enterProgressLabel, "%d/16", taLen);
            }
        }
    } else if (code == LV_EVENT_READY) {
        if (keyBoard->mode == KEY_STONE_LETTER) {
            if (g_importPhraseKb != NULL) {
                lv_event_send(g_importPhraseKb->btnm, LV_EVENT_READY, (void *)currText);
            }
        } else {
            UpdateFullKeyBoard("", keyBoard);
        }
    } else if (code == LV_EVENT_CANCEL) {
        Vibrate(SLIGHT);
        if (keyBoard->mode == KEY_STONE_LETTER) {
            if (strlen(currText) < 3) {
                for (int i = 0; i < 3; i++) {
                    memset(g_wordBuf[i], 0, sizeof(g_wordBuf[i]));
                    lv_label_set_text(keyBoard->associateLabel[i], "");
                }
            }
            if (g_importPhraseKb != NULL) {
                lv_event_send(g_importPhraseKb->btnm, LV_EVENT_CANCEL, (void *)currText);
            }
            UpdateKeyBoard(rootTree, currText, keyBoard);
        } else {
            // if (keyBoard->mode == KEY_STONE_FULL_L || keyBoard->mode == KEY_STONE_FULL_U) {
            //     UpdateFullKeyBoard(currText, keyBoard);
            // }

            if (g_enterProgressLabel != NULL) {
                lv_label_set_text_fmt(g_enterProgressLabel, "%d/16", taLen);
                lv_obj_set_style_text_color(g_enterProgressLabel, WHITE_COLOR, LV_PART_MAIN);
                lv_obj_set_style_text_opa(g_enterProgressLabel, LV_OPA_56, LV_PART_MAIN);
            }
        }
    } else if (code == KEY_STONE_KEYBOARD_CHANGE) {
        Vibrate(SLIGHT);
        lv_keyboard_user_mode_t *keyMode = lv_event_get_param(e);
        keyBoard->mode = *keyMode;
        GuiKeyBoardSetMode(keyBoard);
    } else if (code == LV_EVENT_CLICKED) {
        Vibrate(SLIGHT);
    }
}

void *GuiCreateLetterKeyBoard(lv_obj_t *parent, lv_event_cb_t cb, bool bip39, void *param)
{
    static lv_point_t linePoints[2] = {{0, 0}, {0, 40}};

    if (param != NULL) {
        g_importPhraseKb = param;
        g_importPhraseKb->btnm = ((MnemonicKeyBoard_t*)param)->btnm;
    }

    UpdateRootTree(bip39);

    KeyBoard_t *keyBoard = GuiCreateKeyBoard(parent, cb, KEY_STONE_LETTER, NULL);
    lv_obj_align(keyBoard->kb, LV_ALIGN_TOP_MID, 0, 50);

    lv_obj_t *cont = GuiCreateContainerWithParent(keyBoard->cont, 408, 50);
    lv_obj_set_style_bg_color(cont, DARK_BG_COLOR, 0);
    lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 2, 5);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);

    lv_obj_t *btn = lv_btn_create(cont);
    lv_obj_align_to(btn, cont, LV_ALIGN_LEFT_MID, 7, 0);
    lv_obj_set_height(btn, 40);
    lv_obj_set_style_bg_opa(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_t *label1 = GuiCreateLittleTitleLabel(btn, "");
    lv_obj_set_align(label1, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(label1, WHITE_COLOR, 0);
    lv_obj_add_event_cb(btn, LetterKbAssociateHandler, LV_EVENT_ALL, keyBoard);
    keyBoard->associateLabel[0] = label1;
    lv_obj_t *line = GuiCreateLine(cont, linePoints, 2);
    lv_obj_align_to(line, btn, LV_ALIGN_OUT_RIGHT_MID, 5, 15);

    btn = lv_btn_create(cont);
    lv_obj_align_to(btn, line, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    lv_obj_set_height(btn, 40);
    lv_obj_set_style_bg_opa(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_t *label2 = GuiCreateLittleTitleLabel(btn, "");
    lv_obj_set_align(label2, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(label2, WHITE_COLOR, 0);
    lv_obj_add_event_cb(btn, LetterKbAssociateHandler, LV_EVENT_ALL, keyBoard);
    keyBoard->associateLabel[1] = label2;
    line = GuiCreateLine(cont, linePoints, 2);
    lv_obj_align_to(line, btn, LV_ALIGN_OUT_RIGHT_MID, 5, 15);

    btn = lv_btn_create(cont);
    lv_obj_align_to(btn, line, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    lv_obj_set_height(btn, 40);
    lv_obj_set_style_bg_opa(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_t *label3 = GuiCreateLittleTitleLabel(btn, "");
    lv_obj_set_align(label3, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(label3, WHITE_COLOR, 0);
    lv_obj_add_event_cb(btn, LetterKbAssociateHandler, LV_EVENT_ALL, keyBoard);
    keyBoard->associateLabel[2] = label3;

    btn = GuiCreateBtn(keyBoard->cont, "");
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, 0, 2);
    lv_obj_set_size(btn, 70, 50);
    lv_obj_t *img = GuiCreateImg(btn, &imgArrowDownS);
    lv_obj_align(img, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_radius(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, CloseLetterKbHandler, LV_EVENT_ALL, keyBoard);

    lv_obj_t *ta = lv_textarea_create(keyBoard->cont);
    lv_obj_set_size(ta, 0, 0);
    lv_obj_align(ta, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_add_event_cb(ta, KbTextAreaHandler, LV_EVENT_ALL, keyBoard);
    lv_obj_set_style_bg_color(ta, DARK_BG_COLOR, 0);
    lv_obj_set_style_text_opa(ta, 0, 0);
    keyBoard->ta = ta;

    lv_keyboard_set_textarea(keyBoard->kb, ta);
    lv_textarea_set_max_length(ta, 12);

    return keyBoard;
}

