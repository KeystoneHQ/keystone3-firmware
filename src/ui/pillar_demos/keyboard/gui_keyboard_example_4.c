#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "gui.h"
#include "gui_keyboard.h"
#include "gui_create_wallet_widgets.h"
#include "bip39_english.h"

static MnemonicKeyBoard_t *g_keyBoard;
static lv_obj_t *g_cont;
#define MNEMONIC_WORD_CNT 24

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

static void UpdateListHandler(lv_event_t * e)
{
    char mnemonicList[MNEMONIC_WORD_CNT][10];
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        for (int i = 0; i < MNEMONIC_WORD_CNT; i++) {
            strcpy(mnemonicList[i], wordlist[lv_rand(0, 2047)]);
        }
        GuiUpdateMnemonicKeyBoard(g_keyBoard, mnemonicList, false);
    }
}

void gui_keyboard_example_4(void)
{
    char mnemonicList[MNEMONIC_WORD_CNT][10];
    for (int i = 0; i < MNEMONIC_WORD_CNT; i++) {
        strcpy(mnemonicList[i], wordlist[i]);
    }
    g_cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    g_keyBoard = GuiCreateMnemonicKeyBoard(g_cont, NULL,
                                           KEY_STONE_MNEMONIC_24, mnemonicList);
//    lv_btnmatrix_set_btn_ctrl(g_keyBoard->btnm, 1, LV_BTNMATRIX_CTRL_RECOLOR);
    for (int i = 0; i < MNEMONIC_WORD_CNT; i++) {
        strcpy(mnemonicList[i], wordlist[lv_rand(0, 2047)]);
    }
    GuiUpdateMnemonicKeyBoard(g_keyBoard, mnemonicList, false);
#if 0
    lv_btnmatrix_clear_btn_ctrl_all(g_keyBoard->btnm, 1);
    lv_btnmatrix_set_btn_ctrl(g_keyBoard->btnm, 1, LV_BTNMATRIX_CTRL_DISABLED | LV_BTNMATRIX_CTRL_CLICK_TRIG);

    lv_obj_t * btn1 = lv_btn_create(g_cont);
    lv_obj_add_event_cb(btn1, UpdateListHandler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_BOTTOM_LEFT, 45, -40);
    lv_obj_t *label = GuiCreateLabel(btn1, "update");
#endif
}
