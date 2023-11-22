#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "gui.h"
#include "gui_keyboard.h"
#include "gui_create_wallet_widgets.h"
#include "bip39_english.h"

static KeyBoard_t *g_keyBoard;
static lv_obj_t *g_cont;

static void GetRandomMnemonic(char **mnemonicList)
{
    for (int i = 0; i < 12; i++) {
        strcpy(mnemonicList[i], wordlist[lv_rand(0, 2047)]);
    }
}

static void PrintMnemonic(char **mnemonicList)
{
    for (int i = 0; i < 12; i++) {
        printf("mnemonicList[%d] = %s\n", i, mnemonicList[i]);
    }
}

static void kbHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
#if 0
    lv_obj_t *ta = lv_event_get_target(e);
    KeyBoard_t *keyBoard = lv_event_get_user_data(e);
    const char *currText = lv_textarea_get_text(ta);
#endif
    char ebuf[32] = {0};
    static char endBuf[8] = {0};
    if (code == LV_EVENT_PRESSED) {
        printf("hello world...\n");
    }
}

#define MNEMONIC_WORD_CNT 12
static char mnemonicList[MNEMONIC_WORD_CNT][10];
void gui_keyboard_example_3(void)
{
    for (int i = 0; i < MNEMONIC_WORD_CNT; i++) {
        strcpy(mnemonicList[i], wordlist[i]);
    }
    g_cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    g_keyBoard = GuiCreateMnemonicKeyBoard(g_cont, kbHandler,
                                           KEY_STONE_MNEMONIC_12, mnemonicList);
    for (int i = 0; i < MNEMONIC_WORD_CNT; i++) {
        strcpy(mnemonicList[i], wordlist[lv_rand(0, 2047)]);
    }
    GuiUpdateMnemonicKeyBoard(g_keyBoard, mnemonicList, false);
}
